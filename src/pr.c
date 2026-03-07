#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pr.h"

#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_DIM "\033[2m"
#define COLOR_RESET "\033[0m"

void pr_free(PR *pr) {
    if (!pr) return;
    free(pr->title);
    free(pr->url);
    free(pr->author);
    free(pr->repository);
    free(pr->review_decision);
    free(pr->reviewer);
}

void prlist_free(PRList *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        pr_free(&list->prs[i]);
    }
    free(list->prs);
    free(list);
}

const char *get_review_emoji(const char *decision) {
    if (!decision) return "⏳";
    if (strcmp(decision, "APPROVED") == 0) return "✅";
    if (strcmp(decision, "CHANGES_REQUESTED") == 0) return "❌";
    if (strcmp(decision, "REVIEW_REQUIRED") == 0) return "⏳";
    return "⏳";
}

const char *get_review_text(const char *decision) {
    if (!decision) return "Pending";
    if (strcmp(decision, "APPROVED") == 0) return "Approved";
    if (strcmp(decision, "CHANGES_REQUESTED") == 0) return "Changes Requested";
    if (strcmp(decision, "REVIEW_REQUIRED") == 0) return "Review Required";
    return "Pending";
}

const char *get_review_color(const char *decision) {
    if (!decision) return COLOR_DIM;
    if (strcmp(decision, "APPROVED") == 0) return COLOR_GREEN;
    if (strcmp(decision, "CHANGES_REQUESTED") == 0) return COLOR_RED;
    if (strcmp(decision, "REVIEW_REQUIRED") == 0) return COLOR_DIM;
    return COLOR_DIM;
}
