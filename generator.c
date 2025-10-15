#include "board.h"
#include "vct.h"
#include "wtree.h"
#include "save.h"
#include "bitmap256.h"

#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>

#define CENTER_POSITION 112
#define TEST_BLACK_RANGE 3
#define THREAD_COUNT 16
#define INITIAL_DEPTH 3
#define LOAD_FROM_FILE_ID 1

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static int test_white(wtree *wt, board *bd, int curr_depth, int max_depth);
static int test_black(wtree *wt, board *bd, int curr_depth, int max_depth);

static int test_white(wtree *wt, board *bd, int curr_depth, int max_depth) {
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board_has_piece(bd, pos)) {
      continue;
    }
    board_put_white(bd, pos);
    board tmp_bd = *bd;
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 4; ++j) {
        if (wtree_find(wt, &tmp_bd) != -1) {
          goto label_continue;
        }
        board_rotate_clockwise_90(&tmp_bd);
      }
      if (i == 0) {
        board_flip_horizontal(&tmp_bd);
      }
    }
    if (vct_check_pattern(bd, pos) & VCT_PATTERN_FIVE_IN_A_ROW) {
      board_remove_white(bd, pos);
      return pos;
    }
    if (vct(bd) != -1) {
      goto label_continue;
    }
    if (test_black(wt, bd, curr_depth + 1, max_depth) == -1) {
      board_remove_white(bd, pos);
      return pos;
    }
label_continue:
    board_remove_white(bd, pos);
  }
  return -1;
}

static int test_black(wtree *wt, board *bd, int curr_depth, int max_depth) {
  bitmap256 checked;
  bitmap256_init(&checked);
  board tmp_bd = *bd;
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 4; ++j) {
      int result = wtree_find(wt, &tmp_bd);
      if (result != -1) {
        return result;
      }
      board_rotate_clockwise_90(&tmp_bd);
    }
    if (i == 0) {
      board_flip_horizontal(&tmp_bd);
    }
  }
  uint64_t prev_wtree_size = wtree_size(wt);
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
        // Only check for duplicates when the wtree size changes
        if (wtree_size(wt) > prev_wtree_size) {
          board tmp_bd = *bd;
          for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 4; ++j) {
              int result = wtree_find(wt, &tmp_bd);
              if (result != -1) {
                return result;
              }
              board_rotate_clockwise_90(&tmp_bd);
            }
            if (i == 0) {
              board_flip_horizontal(&tmp_bd);
            }
          }
        }
        if (curr_depth < max_depth) {
          board_put_black(bd, new_pos);
          int result = test_white(wt, bd, curr_depth + 1, max_depth);
          board_remove_black(bd, new_pos);
          if (result == -1) {
            wtree_insert(wt, bd, new_pos);
            printf("Found a winning node\nWin tree size: %"PRIu64"\n", wtree_size(wt));
            return new_pos;
          }
        }
      }
    }
  }
  return -1;
}

typedef struct thread_arg thread_arg;

struct thread_arg {
  wtree *wt;
  board *bd;
  int max_depth;
  int id;
};

void *thread_func(void *arg) {
  thread_arg *ta = (thread_arg *)arg;
  test_black(ta->wt, ta->bd, 3, ta->max_depth);
  printf("Thread %d down\n", ta->id);
  return NULL;
}

void main_while() {
  printf("Build started\n");
  wtree wt;
  wtree_init(&wt);
  // load_from_file(&wt, LOAD_FROM_FILE_ID);
  board bd;
  board_init(&bd);
  wtree_insert(&wt, &bd, CENTER_POSITION);
  pthread_t threads[THREAD_COUNT];
  board boards[THREAD_COUNT];
  thread_arg args[THREAD_COUNT];
  int unsorted_pos[BOARD_SIZE * BOARD_SIZE];
  for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
    unsorted_pos[i] = i;
  }
  srand(time(NULL));
  for (int iter_depth = INITIAL_DEPTH; iter_depth <= BOARD_SIZE * BOARD_SIZE; iter_depth += 2) {
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
      int random_i = rand() % (BOARD_SIZE * BOARD_SIZE);
      int tmp = unsorted_pos[i];
      unsorted_pos[i] = unsorted_pos[random_i];
      unsorted_pos[random_i] = tmp;
    }
    for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE;) {
      printf("Current max depth: %d\n", iter_depth);
      int thread_num = 0;
      for (int i = 0; i < THREAD_COUNT && pos < BOARD_SIZE * BOARD_SIZE; ++i, ++pos) {
        if (unsorted_pos[i] == CENTER_POSITION) {
          continue;
        }
        board_init(&boards[i]);
        board_put_black(&boards[i], CENTER_POSITION);
        board_put_white(&boards[i], unsorted_pos[i]);
        args[i].bd = &boards[i];
        args[i].max_depth = iter_depth;
        args[i].wt = &wt;
        args[i].id = i;
        ++thread_num;
      }
      for (int i = 0; i < thread_num; ++i) {
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
      }
      for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
      }
      printf("Win tree size: %"PRIu64"\n", wtree_size(&wt));
      printf("Save to file wtree_%d.bin\n", iter_depth);
      save_to_file(&wt, iter_depth);
    }
  }
  wtree_free(&wt);
  printf("Build completed\n");
}

int main() {
  main_while();
  return 0;
}
