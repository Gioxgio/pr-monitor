#define _POSIX_C_SOURCE 200809L

#include "pr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO obfuscate object structure
// TODO Check if reviewer correcly handles multiple reviewers
void pr_free(PR *pr) {
    if (!pr)
        return;
    free(pr->title);
    free(pr->url);
    free(pr->author);
    free(pr->repository);
    free(pr->review_decision);
    free(pr->reviewer);
}

void prlist_free(PRList *list) {
    if (!list)
        return;
    for (size_t i = 0; i < list->count; i++) {
        pr_free(&list->prs[i]);
    }
    free(list->prs);
    free(list);
}

// TODO improve naming
char *get_review(PR *pr) {
    char *decision = pr->review_decision;
    if (!decision)
        return strdup("⏳ Pending");
    if (strcmp(decision, "REVIEW_REQUIRED") == 0)
        return strdup("⏳ Review required");
    if (strcmp(decision, "CHANGES_REQUESTED") == 0)
        return strdup("❌ Changes requested");
    if (strcmp(decision, "APPROVED") == 0) {
        if (pr->reviewer) {
            char *mask = "✅ Approved (by: %s)";
            int len = snprintf(NULL, 0, mask, pr->reviewer) + 1;
            char *result = malloc(len);
            snprintf(result, len, mask, pr->reviewer);
            return result;
        }
    }
    return strdup("😵‍💫 Unknown");
}

