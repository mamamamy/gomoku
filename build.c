#include "board.h"
#include "vct.h"
#include "wtree.h"
#include "save.h"
#include "bitmap256.h"

#include <stdio.h>

#define TEST_BLACK_RANGE 3

static int check_win(board *bd, int pos) {
  int is_black = board_has_black(bd, pos);
  int is_white = board_has_white(bd, pos);
  if (!is_black && !is_white) {
    return 0;
  }
  int x, y;
  board_to_xy(pos, &x, &y);
  static const int dirs[4][2] = {
    {-1, 0},  // Left
    {0, -1},  // Up
    {-1, -1}, // Up Left
    {1, -1},  // Up Right
  };
  for (int i = 0; i < 4; ++i) {
    int count = 1;
    int tmp_x, tmp_y;
    tmp_x = x;
    tmp_y = y;
    for (int j = 1; j < 5; ++j) {
      tmp_x += dirs[i][0];
      tmp_y += dirs[i][1];
      if (!board_is_valid_xy(tmp_x, tmp_y)) {
        break;
      }
      int tmp_pos = board_to_pos(tmp_x, tmp_y);
      if (
        (is_black && !board_has_black(bd, tmp_pos)) ||
        (is_white && !board_has_white(bd, tmp_pos))
      ) {
        break;
      }
      ++count;
    }
    tmp_x = x;
    tmp_y = y;
    for (int j = 1; j < 5; ++j) {
      tmp_x -= dirs[i][0];
      tmp_y -= dirs[i][1];
      if (!board_is_valid_xy(tmp_x, tmp_y)) {
        break;
      }
      int tmp_pos = board_to_pos(tmp_x, tmp_y);
      if (
        (is_black && !board_has_black(bd, tmp_pos)) ||
        (is_white && !board_has_white(bd, tmp_pos))
      ) {
        break;
      }
      ++count;
    }
    if (count >= 5) {
      return 1;
    }
  }
  return 0;
}

static int test_white(wtree *wt, board *bd, int curr_depth, int max_depth);
static int test_black(wtree *wt, board *bd, int curr_depth, int max_depth);

static int test_white(wtree *wt, board *bd, int curr_depth, int max_depth) {
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    board_put_white(bd, pos);
    if (wtree_find(wt, bd) != -1) {
      board_remove_white(bd, pos);
      continue;
    }
    if (check_win(bd, pos)) {
      board_remove_white(bd, pos);
      return pos;
    }
    test_black(wt, bd, curr_depth + 1, max_depth);
    board_remove_white(bd, pos);
  }
  return -1;
}

static int test_black(wtree *wt, board *bd, int curr_depth, int max_depth) {
  bitmap256 checked;
  bitmap256_init(&checked);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (!board_has_piece(bd, pos)) {
      continue;
    }
    int x, y;
    board_to_xy(pos, &x, &y);
    int min_x = max(x - TEST_BLACK_RANGE, 0);
    int min_y = max(y - TEST_BLACK_RANGE, 0);
    int max_x = min(x + TEST_BLACK_RANGE, BOARD_SIZE - 1);
    int max_y = min(y + TEST_BLACK_RANGE, BOARD_SIZE - 1);
    int pattern;
    for (int new_x = min_x; new_x <= max_x; ++new_x) {
      for (int new_y = min_y; new_y <= max_y; ++new_y) {
        int new_pos = board_to_pos(new_x, new_y);
        if (bitmap256_test(&checked, new_pos)) {
          continue;
        }
        bitmap256_set(&checked, new_pos);
        if (board_has_piece(bd, new_pos)) {
          continue;
        }
        board_put_black(bd, new_pos);
        int result = test_white(wt, bd, curr_depth + 1, max_depth);
        board_remove_black(bd, new_pos);
      }
    }
  }
  return -1;
}

void main_while() {
  printf("Build started\n");
  wtree wt;
  wtree_init(&wt);
  board bd;
  board_init(&bd);
  wtree_insert(&wt, &bd, 112);
  board_put_black(&bd, 112);
  for (int iter_depth = 3; iter_depth <= 3; iter_depth += 2) {
    printf("Current max depth: %d\n", iter_depth);
    test_white(&wt, &bd, 2, iter_depth);
    save_to_file(&wt, iter_depth);
  }
  wtree_free(&wt);
  printf("Build completed\n");
}

int main() {
  main_while();
  return 0;
}
