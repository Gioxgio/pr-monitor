#ifndef PR_H
#define PR_H

#include <stddef.h>

typedef struct {
    int number;
    char *title;
    char *url;
    char *author;
    char *repository;
    char *review_decision;
    char *reviewer;
} PR;

typedef struct {
    PR *prs;
    size_t count;
} PRList;

void pr_free(PR *pr);
void prlist_free(PRList *list);
const char *get_review_emoji(const char *decision);
const char *get_review_text(const char *decision);
const char *get_review_color(const char *decision);

#endif
