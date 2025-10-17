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
#define THREAD_COUNT 20
#define INITIAL_DEPTH 7
#define LOAD_FROM_FILE_ID 0

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static const int POS_ORDER[BOARD_SIZE * BOARD_SIZE] = {
  112, 96, 97, 98, 111, 113, 126, 127, 128, 80, 81, 82, 83, 84, 95,
  99, 110, 114, 125, 129, 140, 141, 142, 143, 144, 64, 65, 66, 67, 68,
  69, 70, 79, 85, 94, 100, 109, 115, 124, 130, 139, 145, 154, 155, 156,
  157, 158, 159, 160, 48, 49, 50, 51, 52, 53, 54, 55, 56, 63, 71,
  78, 86, 93, 101, 108, 116, 123, 131, 138, 146, 153, 161, 168, 169, 170,
  171, 172, 173, 174, 175, 176, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 47, 57, 62, 72, 77, 87, 92, 102, 107, 117, 122, 132, 137,
  147, 152, 162, 167, 177, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
  192, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 31,
  43, 46, 58, 61, 73, 76, 88, 91, 103, 106, 118, 121, 133, 136, 148,
  151, 163, 166, 178, 181, 193, 196, 197, 198, 199, 200, 201, 202, 203, 204,
  205, 206, 207, 208, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 13, 14, 15, 29, 30, 44, 45, 59, 60, 74, 75, 89, 90,
  104, 105, 119, 120, 134, 135, 149, 150, 164, 165, 179, 180, 194, 195, 209,
  210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224
};

static int find_win_pos(wtree *wt, board bd) {
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 4; ++j) {
      int result = wtree_find(wt, &bd);
      if (result != -1) {
        return result;
      }
      if (i == 1 && j == 3) {
        break;
      }
      board_rotate_clockwise_90(&bd);
    }
    if (i == 1) {
      break;
    }
    board_flip_horizontal(&bd);
  }
  return -1;
}

typedef struct test_context test_context;

struct test_context {
  wtree *wt;
  board *bd;
  spinlock save_file_lock;
};

static int test_white(test_context *tc, int curr_depth, int max_depth);
static int test_black(test_context *tc, int curr_depth, int max_depth);

static int test_white(test_context *tc, int curr_depth, int max_depth) {
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board_has_piece(tc->bd, pos)) {
      continue;
    }
    board_put_white(tc->bd, pos);
    if (find_win_pos(tc->wt, *tc->bd) != -1) {
      board_remove_white(tc->bd, pos);
      continue;
    }
    if (vct_check_pattern(tc->bd, pos) & VCT_PATTERN_FIVE_IN_A_ROW) {
      board_remove_white(tc->bd, pos);
      return pos;
    }
    // VCT is only used on the last layer of the white pieces
    if (curr_depth == max_depth - 1 && vct(tc->bd) != -1) {
      board_remove_white(tc->bd, pos);
      continue;
    }
    if (test_black(tc, curr_depth + 1, max_depth) == -1) {
      board_remove_white(tc->bd, pos);
      return pos;
    }
    board_remove_white(tc->bd, pos);
  }
  return -1;
}

static int test_black(test_context *tc, int curr_depth, int max_depth) {
  if (curr_depth >= max_depth) {
    return -1;
  }
  bitmap256 checked;
  bitmap256_init(&checked);
  uint64_t prev_wtree_size = wtree_size(tc->wt);
  int result = find_win_pos(tc->wt, *tc->bd);
  if (result != -1) {
    return result;
  }
  for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
    int pos = POS_ORDER[i];
    if (!board_has_piece(tc->bd, pos)) {
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
        if (board_has_piece(tc->bd, new_pos)) {
          continue;
        }
        // Only check for duplicates when the wtree size changes
        if (wtree_size(tc->wt) > prev_wtree_size) {
          prev_wtree_size = wtree_size(tc->wt);
          int result = find_win_pos(tc->wt, *tc->bd);
          if (result != -1) {
            return result;
          }
        }
        board_put_black(tc->bd, new_pos);
        int result = test_white(tc, curr_depth + 1, max_depth);
        board_remove_black(tc->bd, new_pos);
        if (result == -1) {
          wtree_insert(tc->wt, tc->bd, new_pos);
          uint64_t wt_size = wtree_size(tc->wt);
          printf("Found a winning node\nWin tree size: %"PRIu64"\n", wt_size);
          spinlock_lock(&tc->save_file_lock);
          save_to_file(tc->wt, wt_size);
          spinlock_unlock(&tc->save_file_lock);
          printf("Save to file wtree_%"PRIu64"\n", wt_size);
          return new_pos;
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
  test_context tc;
  tc.wt = ta->wt;
  tc.bd = ta->bd;
  spinlock_init(&tc.save_file_lock);
  test_black(&tc, 3, ta->max_depth);
  printf("Thread %d down\n", ta->id);
  return NULL;
}

void main_while() {
  printf("Build started\n");
  wtree wt;
  wtree_init(&wt);
  // load_from_file(&wt, LOAD_FROM_FILE_ID);
  // printf("Win tree size: %"PRIu64"\n", wtree_size(&wt));
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
    printf("Current max depth: %d\n", iter_depth);
    for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE;) {
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
    }
  }
  wtree_free(&wt);
  printf("Build completed\n");
}

int main() {
  main_while();
  return 0;
}
