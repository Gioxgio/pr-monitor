#define _POSIX_C_SOURCE 200809L

#include "json_parser.h"
#include "pr.h"
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int get_int(json_object *obj, const char *key) {
    json_object *tmp;
    if (!json_object_object_get_ex(obj, key, &tmp))
        return 0;
    return json_object_get_int(tmp);
}

static char *get_string(json_object *obj, const char *path) {
    char *p = strdup(path);
    char *key = strtok(p, "->");
    while (key) {
        if (!json_object_object_get_ex(obj, key, &obj)) {
            free(p);
            return NULL;
        }
        if ((key = strtok(NULL, "->")) == NULL) {
            free(p);
            const char *s = json_object_get_string(obj);
            return s ? strdup(s) : NULL;
        }
    }
    free(p);
    return NULL;
}

PRList *parse_prs_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(len + 1);
    fread(data, 1, len, fp);
    data[len] = '\0';
    fclose(fp);

    json_object *root = json_tokener_parse(data);
    free(data);

    if (!root) {
        fprintf(stderr, "Failed to parse JSON\n");
        return NULL;
    }

    PRList *list = malloc(sizeof(PRList));
    list->count = json_object_array_length(root);
    list->prs = calloc(list->count, sizeof(PR));

    for (size_t i = 0; i < list->count; i++) {
        json_object *item = json_object_array_get_idx(root, i);
        PR *pr = &list->prs[i];

        pr->number = get_int(item, "number");
        pr->title = get_string(item, "title");
        pr->url = get_string(item, "url");
        pr->repository = get_string(item, "repository->name");
        pr->author = get_string(item, "author->login");
        pr->review_decision = get_string(item, "reviewDecision");

        json_object *reviews;
        if (json_object_object_get_ex(item, "latestReviews", &reviews)) {
            json_object *nodes;
            if (json_object_object_get_ex(reviews, "nodes", &nodes)) {
                int review_count = json_object_array_length(nodes);
                if (review_count > 0) {
                    json_object *review = json_object_array_get_idx(nodes, 0);
                    pr->reviewer = get_string(review, "author->login");
                }
            }
        }
    }

    json_object_put(root);
    return list;
}
