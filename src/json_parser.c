#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "pr.h"
#include "json_parser.h"

static char *get_string(json_object *obj, const char *key) {
    json_object *tmp;
    if (json_object_object_get_ex(obj, key, &tmp)) {
        const char *s = json_object_get_string(tmp);
        return s ? strdup(s) : NULL;
    }
    return NULL;
}

static char *get_nested_string(json_object *obj, const char *parent, const char *child) {
    json_object *parent_obj;
    if (!json_object_object_get_ex(obj, parent, &parent_obj)) return NULL;
    return get_string(parent_obj, child);
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

        json_object *num;
        if (json_object_object_get_ex(item, "number", &num)) {
            pr->number = json_object_get_int(num);
        }

        pr->title = get_string(item, "title");
        pr->url = get_string(item, "url");
        pr->repository = get_nested_string(item, "repository", "name");
        pr->author = get_nested_string(item, "author", "login");
        pr->review_decision = get_string(item, "reviewDecision");

        json_object *reviews;
        if (json_object_object_get_ex(item, "latestReviews", &reviews)) {
            json_object *nodes;
            if (json_object_object_get_ex(reviews, "nodes", &nodes)) {
                int review_count = json_object_array_length(nodes);
                if (review_count > 0) {
                    json_object *review = json_object_array_get_idx(nodes, 0);
                    pr->reviewer = get_nested_string(review, "author", "login");
                }
            }
        }
    }

    json_object_put(root);
    return list;
}
