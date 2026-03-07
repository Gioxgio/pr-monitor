#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE_EXTENDED

#include "ui.h"
#include "pr.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_field(WINDOW *win, int *line_num, const char *label,
                        const char *value, int color_pair) {
  wattron(win, A_DIM | COLOR_PAIR(3));
  mvwprintw(win, (*line_num)++, 0, "%s", label);
  wattroff(win, A_DIM | COLOR_PAIR(3));

  int col = strlen(label);
  if (color_pair == 4) {
    wattron(win, A_DIM | COLOR_PAIR(3));
  } else if (color_pair > 0) {
    wattron(win, COLOR_PAIR(color_pair));
  }
  mvwprintw(win, (*line_num - 1), col, "%s", value);
  if (color_pair == 4) {
    wattroff(win, A_DIM | COLOR_PAIR(3));
  } else if (color_pair > 0) {
    wattroff(win, COLOR_PAIR(color_pair));
  }
}

static void print_pr(WINDOW *win, PR *pr, int y, int selected, int *line_num) {
  if (selected) {
    wattron(win, A_BOLD);
    mvwprintw(win, (*line_num)++, 0, "> #%d %s", pr->number, pr->title);
    wattroff(win, A_BOLD);
  } else {
    mvwprintw(win, (*line_num)++, 0, "  #%d %s", pr->number, pr->title);
  }

  const char *emoji = get_review_emoji(pr->review_decision);
  const char *review_text = get_review_text(pr->review_decision);

  int review_color = 0;
  if (!pr->review_decision) {
    review_color = 4;
  } else if (strcmp(pr->review_decision, "REVIEW_REQUIRED") == 0) {
    review_color = 2;
  } else if (strcmp(pr->review_decision, "APPROVED") == 0) {
    review_color = 5;
  } else if (strcmp(pr->review_decision, "CHANGES_REQUESTED") == 0) {
    review_color = 6;
  }

  char review_value[64];
  if (pr->reviewer) {
    snprintf(review_value, sizeof(review_value), "%s %s (by: %s)", emoji,
             review_text, pr->reviewer);
  } else {
    snprintf(review_value, sizeof(review_value), "%s %s", emoji, review_text);
  }
  print_field(win, line_num, "    ├─ review: ", review_value, review_color);

  print_field(win, line_num, "    ├─ author: ", pr->author, 0);
  print_field(win, line_num, "    └─ url:    ", pr->url, 1);
}

static void print_repo(WINDOW *win, const char *repo, int y, int selected,
                       int *line_num) {
  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, (*line_num)++, 0, "📦 %s", repo);
  wattroff(win, COLOR_PAIR(2));
}

static int open_url(const char *url) {
  char cmd[1024];
  snprintf(cmd, sizeof(cmd), "xdg-open '%s' &", url);
  return system(cmd);
}

int run_ui(PRList *list) {
  initscr();
  raw();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  if (has_colors()) {
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    if (COLORS >= 256) {
      init_pair(3, 245, COLOR_BLACK);
    } else {
      init_pair(3, COLOR_WHITE, COLOR_BLACK);
    }
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_RED, COLOR_BLACK);
  }

  int selected = 0;
  int ch;

  while (1) {
    clear();

    int height, width;
    getmaxyx(stdscr, height, width);

    int line = 1;
    char *current_repo = NULL;
    int pr_idx = 0;

    for (size_t i = 0; i < list->count; i++) {
      PR *pr = &list->prs[i];

      if (!current_repo || strcmp(current_repo, pr->repository) != 0) {
        if (current_repo != NULL) {
          line++;
        }
        free(current_repo);
        current_repo = strdup(pr->repository);
        print_repo(stdscr, current_repo, line, (int)i == selected, &line);
      }

      int is_selected = (int)i == selected;
      print_pr(stdscr, pr, line, is_selected, &line);
      pr_idx++;
    }

    free(current_repo);

    refresh();

    ch = getch();

    if (ch == 'q' || ch == 'Q') {
      break;
    } else if (ch == KEY_UP) {
      if (selected > 0)
        selected--;
    } else if (ch == KEY_DOWN) {
      if (selected < (int)list->count - 1)
        selected++;
    } else if (ch == '\n' || ch == KEY_ENTER) {
      endwin();
      open_url(list->prs[selected].url);
      printf("Opening: %s\n", list->prs[selected].url);
      return 0;
    }
  }

  endwin();
  return 0;
}
