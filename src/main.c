#include <stdio.h>
#include <locale.h>
#include "pr.h"
#include "json_parser.h"
#include "ui.h"

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    
    const char *filename = "response.json";
    
    if (argc > 1) {
        filename = argv[1];
    }
    
    PRList *list = parse_prs_from_file(filename);
    if (!list) {
        fprintf(stderr, "Failed to parse PRs from %s\n", filename);
        return 1;
    }
    
    printf("Loaded %zu PRs\n", list->count);
    
    int result = run_ui(list);
    
    prlist_free(list);
    return result;
}
