// TODO Improve handling of this
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#define _XOPEN_SOURCE_EXTENDED

#include "ui.h"
#include "pr.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define private static

private void init();
private void print_repo(WINDOW *win, const char *repo, int *line_num);
private void print_pr(WINDOW *win, PR *pr, bool selected, int *line_num);
private void print_field(WINDOW *, int *, char *, char *, int);
private int open_url(char *);

int run_ui(PRList *list) {
    init();

    size_t selected = 0;
    int ch;

    while (1) {
        int line = 1;
        char *current_repo = NULL;

        for (size_t i = 0; i < list->count; i++) {
            PR *pr = &list->prs[i];

            if (!current_repo || strcmp(current_repo, pr->repository) != 0) {
                if (current_repo != NULL) {
                    line++;
                }
                free(current_repo);
                current_repo = strdup(pr->repository);
                print_repo(stdscr, current_repo, &line);
            }

            bool is_selected = i == selected;
            print_pr(stdscr, pr, is_selected, &line);
        }
        free(current_repo);

        refresh();

        ch = getch();

        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == KEY_UP || ch == 'k' || ch == 'K') {
            if (selected > 0)
                selected--;
        } else if (ch == KEY_DOWN || ch == 'j' || ch == 'J') {
            if (selected < list->count - 1)
                selected++;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            open_url(list->prs[selected].url);
        }
    }

    endwin();
    return 0;
}

private void init() {
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

private void print_repo(WINDOW *win, const char *repo, int *line_num) {
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, (*line_num)++, 0, "📦 %s", repo);
    wattroff(win, COLOR_PAIR(2));
}

private void print_pr(WINDOW *win, PR *pr, bool selected, int *line_num) {
    if (selected) {
        wattron(win, A_BOLD);
        mvwprintw(win, (*line_num)++, 0, "> #%d %s", pr->number, pr->title);
        wattroff(win, A_BOLD);
    } else {
        mvwprintw(win, (*line_num)++, 0, "  #%d %s", pr->number, pr->title);
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
    print_field(win, line_num, "    ├─ review: ", review_value, review_color);

    print_field(win, line_num, "    ├─ author: ", pr->author, 0);
    print_field(win, line_num, "    └─ url:    ", pr->url, 1);

    free(review_value);
}

private void print_field(WINDOW *win, int *line_num, char *label, char *value,
                         int color_pair) {
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, (*line_num)++, 0, "%s", label);
    wattroff(win, COLOR_PAIR(3));

    int col = strlen(label);
    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, (*line_num - 1), col, "%s", value);
    wattroff(win, COLOR_PAIR(color_pair));
}

private int open_url(char *url) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "xdg-open '%s' &", url);
    return system(cmd);
}
