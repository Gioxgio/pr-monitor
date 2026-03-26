#include "ui.h"
#include "macros.h"
#include "pr.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void init(void);
static int count_lines_init_offsets(PRList *, int *);
static void init_buffer(WINDOW *, PRList *, int);
static int print_repo(WINDOW *, const char *, int);
static int print_pr(WINDOW *, PR *, bool, int);
static void print_field(WINDOW *, int, char *, char *, int);
static int open_url(char *);

// TODO Add border
// If content smaller than terminal then obrder smaller than terminal
int run_ui(PRList *list) {
    init();

    int selected = 0;
    int scroll_offset = 0;
    int *pr_offsets = malloc(sizeof(*pr_offsets) * list->count);
    int total_lines = count_lines_init_offsets(list, pr_offsets);
    WINDOW *pad = newpad(total_lines + 1, COLS);

    init_buffer(pad, list, selected);
    refresh();

    while (1) {

        prefresh(pad, scroll_offset, 0, 0, 0, LINES - 1, COLS - 1);

        int ch = getch();
        int old_selected = selected;

        if (ch == 'q' || ch == 'Q' || ch == 27) {
            break;
        }
        if (ch == KEY_UP || ch == 'k') {
            if (selected > 0)
                selected--;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (selected < (int)list->count - 1)
                selected++;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            open_url(list->prs[selected].url);
            continue;
        }

        if (old_selected != selected) {
            int old_pos = pr_offsets[old_selected];
            print_pr(pad, &list->prs[old_selected], false, old_pos);
            int curr_pos = pr_offsets[selected];
            print_pr(pad, &list->prs[selected], true, curr_pos);

            if (curr_pos < scroll_offset) {
                scroll_offset = curr_pos - 1;
            } else if (curr_pos + 4 > scroll_offset + LINES) {
                scroll_offset = curr_pos + 5 - LINES;
            }
        }
    }

    delwin(pad);
    endwin();
    return 0;
}

static void init() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (!has_colors()) {
        return;
    }

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Url
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Repo
    init_pair(3, 245, COLOR_BLACK);          // Dimmed text
    init_pair(5, COLOR_GREEN, COLOR_BLACK);  // Approved PRs
    init_pair(6, COLOR_RED, COLOR_BLACK);    // Changes requested PRs
}

static int count_lines_init_offsets(PRList *list, int *pr_offests) {
    int line = 0;
    char *prev_repo = NULL;

    for (size_t i = 0; i < list->count; i++) {
        PR *pr = &list->prs[i];
        if (!prev_repo || strcmp(prev_repo, pr->repository) != 0) {
            prev_repo = pr->repository;
            line += 2;
        }
        pr_offests[i] = line;
        line += 4;
    }

    return line;
}

static void init_buffer(WINDOW *pad, PRList *list, int selected) {
    int line = 0;
    char *prev_repo = NULL;

    for (size_t i = 0; i < list->count; i++) {
        PR *pr = &list->prs[i];
        if (!prev_repo || strcmp(prev_repo, pr->repository) != 0) {
            line++;
            prev_repo = list->prs[i].repository;
            line = print_repo(pad, prev_repo, line);
        }
        line = print_pr(pad, &list->prs[i], (i == (size_t)selected), line);
    }
}

static int print_repo(WINDOW *win, const char *repo, int line_num) {
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, line_num++, 0, "📦 %s", repo);
    wattroff(win, COLOR_PAIR(2));
    return line_num;
}

static int print_pr(WINDOW *win, PR *pr, bool selected, int line_num) {
    if (selected) {
        wattron(win, A_BOLD);
        mvwprintw(win, line_num++, 0, "> #%d %s", pr->number, pr->title);
        wattroff(win, A_BOLD);
    } else {
        mvwprintw(win, line_num++, 0, "  #%d %s", pr->number, pr->title);
    }

    int review_color = 0;
    if (!pr->review_decision) {
        review_color = 3;
    } else if (strcmp(pr->review_decision, "REVIEW_REQUIRED") == 0) {
        review_color = 2;
    } else if (strcmp(pr->review_decision, "APPROVED") == 0) {
        review_color = 5;
    } else if (strcmp(pr->review_decision, "CHANGES_REQUESTED") == 0) {
        review_color = 6;
    }
    char *review_value = get_review(pr);
    print_field(win, line_num++, "    ├─ review: ", review_value, review_color);
    print_field(win, line_num++, "    ├─ author: ", pr->author, 0);
    print_field(win, line_num++, "    └─ url:    ", pr->url, 1);

    free(review_value);
    return line_num;
}

static void print_field(WINDOW *win, int line_num, char *label, char *value,
                        int color_pair) {
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, line_num, 0, "%s", label);
    wattroff(win, COLOR_PAIR(3));

    int col = strlen(label);
    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, line_num, col, "%s", value);
    wattroff(win, COLOR_PAIR(color_pair));
}

static int open_url(char *url) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "xdg-open '%s' &", url);
    return system(cmd);
}
