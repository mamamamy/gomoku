#include "vct.h"

#include "bitmap256.h"

#define VCT_RANGE 2
#define VCT_MAX_DEPTH 13

#define VCT_PATTERN_NONE 0
#define VCT_PATTERN_FIVE_IN_A_ROW (1 << 0)
#define VCT_PATTERN_LIVE_FOUR (1 << 1)
#define VCT_PATTERN_BROKEN_FOUR (1 << 2)
#define VCT_PATTERN_LIVE_THREE (1 << 3)

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

// Five in a row
// X * O O O O O * X

// Live four
// X _ O O O O _ X

// Broken four
// X _ O O O O X
// X * O _ O O O * X
// X * O O _ O O * X
// X * O O O _ O * X
// X O O O O _ X

// Live three
// X _ _ O O O _ X
// X _ O _ O O _ X
// X _ O O _ O _ X
// X _ O O O _ _ X

static int check_pattern(board *bd, int pos) {
  int is_black = board_has_black(bd, pos);
  int is_white = board_has_white(bd, pos);
  if (!is_black && !is_white) {
    return VCT_PATTERN_NONE;
  }
  int x, y;
  board_to_xy(pos, &x, &y);
  static const int dirs[4][2] = {
    {-1, 0},  // Left
    {0, -1},  // Up
    {-1, -1}, // Up Left
    {1, -1},  // Up Right
  };
  int result = VCT_PATTERN_NONE;
  for (int i = 0; i < 4; ++i) {
    int count = 1;
    int begin_x = x;
    int begin_y = y;
    int end_x = x;
    int end_y = y;
    int broken_begin = 0;
    int broken_end = 0;
    for (int j = 1; j < 5; ++j) {
      begin_x += dirs[i][0];
      begin_y += dirs[i][1];
      if (!board_is_valid_xy(begin_x, begin_y)) {
        broken_begin = 1;
        break;
      }
      int tmp_pos = board_to_pos(begin_x, begin_y);
      if (
        (is_black && !board_has_black(bd, tmp_pos)) ||
        (is_white && !board_has_white(bd, tmp_pos))
      ) {
        broken_begin = board_has_piece(bd, tmp_pos);
        break;
      }
      ++count;
    }
    for (int j = 1; j < 5; ++j) {
      end_x -= dirs[i][0];
      end_y -= dirs[i][1];
      if (!board_is_valid_xy(end_x, end_y)) {
        broken_end = 1;
        break;
      }
      int tmp_pos = board_to_pos(end_x, end_y);
      if (
        (is_black && !board_has_black(bd, tmp_pos)) ||
        (is_white && !board_has_white(bd, tmp_pos))
      ) {
        broken_end = board_has_piece(bd, tmp_pos);
        break;
      }
      ++count;
    }
    // X * O O O O O * X
    if (count >= 5) {
      // X * O O O O O * X
      result |= VCT_PATTERN_FIVE_IN_A_ROW;
      continue;
    }
    // X _ O O O O _ X
    // X _ O O O O X
    // X O O O O _ X
    if (count >= 4) {
      if (broken_begin && broken_end) {
        // X O O O O X
        continue;
      }
      if (!broken_begin && !broken_end) {
        // X _ O O O O _ X
        result |= VCT_PATTERN_LIVE_FOUR;
        continue;
      }
      // X _ O O O O X
      // X O O O O _ X
      result |= VCT_PATTERN_BROKEN_FOUR;
      continue;
    }
    // X * O _ O O O * X
    // X * O O O _ O * X
    // X _ _ O O O _ X
    // X _ O O O _ _ X
    if (count >= 3) {
      if (broken_begin && broken_end) {
        // X O O O X
        continue;
      }
      if (!broken_begin && !broken_end) {
        int new_x, new_y, new_pos;
        new_x = begin_x + dirs[i][0];
        new_y = begin_y + dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O O _ X
          goto label_count3_check_end;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ _ O O O _ X
          result |= VCT_PATTERN_LIVE_THREE;
          goto label_count3_check_end;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X * O _ O O O _ X
          result |= VCT_PATTERN_BROKEN_FOUR;
          goto label_count3_check_end;
        }
label_count3_check_end:
        new_x = end_x - dirs[i][0];
        new_y = end_y - dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O O _ X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ O O O _ _ X
          result |= VCT_PATTERN_LIVE_THREE;
          continue;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X _ O O O _ O * X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X _ O O O _ X
        continue;
      }
      if (broken_begin) {
        int new_x, new_y, new_pos;
        new_x = end_x - dirs[i][0];
        new_y = end_y - dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O O O _ X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X O O O _ O * X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X O O O _ X
        continue;
      }
      if (broken_end) {
        int new_x, new_y, new_pos;
        new_x = begin_x + dirs[i][0];
        new_y = begin_y + dirs[i][1];
        new_pos = board_to_pos(new_x, new_y);
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O O X
          continue;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X * O _ O O O X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X _ O O O X
        continue;
      }
      continue;
    }
    // X O O _ O O X
    // X * O O _ O O * X
    // X _ O _ O O _ X
    // X _ O O _ O _ X
    else if (count >= 2) {
      if (broken_begin && broken_end) {
        // X O O X
        continue;
      }
      if (!broken_begin && !broken_end) {
        int new_x, new_y, new_pos;
        new_x = begin_x + dirs[i][0];
        new_y = begin_y + dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O _ X
          goto label_count2_check_end;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O O _ X
          goto label_count2_check_end;
        }
        new_x += dirs[i][0];
        new_y += dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ O O _ X
          goto label_count2_check_end;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ O _ O O _ X
          result |= VCT_PATTERN_LIVE_THREE;
          goto label_count2_check_end;
        } else if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X * O O _ O O _ X
          result |= VCT_PATTERN_BROKEN_FOUR;
          goto label_count2_check_end;
        }
label_count2_check_end:
        new_x = end_x - dirs[i][0];
        new_y = end_y - dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O _ X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O O _ X
          continue;
        }
        new_x -= dirs[i][0];
        new_y -= dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ O O _ O _ X
          result |=  VCT_PATTERN_LIVE_THREE;
          continue;
        } else if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X _ O O _ O O * X
          result |=  VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X _ O O _ X
        continue;
      }
      if (broken_begin) {
        int new_x, new_y, new_pos;
        new_x = end_x - dirs[i][0];
        new_y = end_y - dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O O _ X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X O O _ X
          continue;
        }
        new_x -= dirs[i][0];
        new_y -= dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O O _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X O O _ O O * X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X O O _ O X
        continue;
      }
      if (broken_end) {
        int new_x, new_y, new_pos;
        new_x = begin_x + dirs[i][0];
        new_y = begin_y + dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O O X
          continue;
        }
        new_x += dirs[i][0];
        new_y += dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ O O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X * O O _ O O X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X O _ O O X
        continue;
      }
      continue;
    }
    // X * O _ O O O * X
    // X * O O O _ O * X
    // X _ O _ O O _ X
    // X _ O O _ O _ X
    if (count >= 1) {
      if (broken_begin && broken_end) {
        // X O X
        continue;
      }
      if (!broken_begin && !broken_end) {
        int new_x, new_y, new_pos;
        // 1
        new_x = begin_x + dirs[i][0];
        new_y = begin_y + dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O _ X
          goto label_count1_check_end;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O _ X
          goto label_count1_check_end;
        }
        // 2
        new_x += dirs[i][0];
        new_y += dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ O _ X
          goto label_count1_check_end;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X O _ O _ X
          goto label_count1_check_end;
        }
        // 3
        new_x += dirs[i][0];
        new_y += dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O O _ O _ X
          goto label_count1_check_end;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ O O _ O _ X
          result |= VCT_PATTERN_LIVE_THREE;
          goto label_count1_check_end;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X * O O O _ O _ X
          result |= VCT_PATTERN_BROKEN_FOUR;
          goto label_count1_check_end;
        }
label_count1_check_end:
        // 1
        new_x = end_x - dirs[i][0];
        new_y = end_y - dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O _ X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O _ X
          continue;
        }
        // 2
        new_x -= dirs[i][0];
        new_y -= dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O _ O X
          continue;
        }
        // 3
        new_x -= dirs[i][0];
        new_y -= dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O _ O O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ O _ O O _ X
          result |= VCT_PATTERN_LIVE_THREE;
          continue;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X _ O _ O O O * X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X _ O _ O O X
        continue;
      }
      if (broken_begin) {
        int new_x, new_y, new_pos;
        // 1
        new_x = end_x - dirs[i][0];
        new_y = end_y - dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X O _ X
          continue;
        }
        // 2
        new_x -= dirs[i][0];
        new_y -= dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X O _ O X
          continue;
        }
        // 3
        new_x -= dirs[i][0];
        new_y -= dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ O O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X O _ O O _ X
          continue;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X O _ O O O * X
          result |= VCT_PATTERN_BROKEN_FOUR;
          continue;
        }
        // X O _ O O X
        continue;
      }
      if (broken_end) {
        int new_x, new_y, new_pos;
        // 1
        new_x = begin_x + dirs[i][0];
        new_y = begin_y + dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X _ O X
          continue;
        }
        // 2
        new_x += dirs[i][0];
        new_y += dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (
          (is_black && !board_has_black(bd, new_pos)) ||
          (is_white && !board_has_white(bd, new_pos))
        ) {
          // X O _ O X
          continue;
        }
        // 3
        new_x += dirs[i][0];
        new_y += dirs[i][1];
        if (!board_is_valid_xy(new_x, new_y)) {
          // X O O _ O X
          continue;
        }
        new_pos = board_to_pos(new_x, new_y);
        if (!board_has_piece(bd, new_pos)) {
          // X _ O O _ O X
          continue;
        }
        if (
          (is_black && board_has_black(bd, new_pos)) ||
          (is_white && board_has_white(bd, new_pos))
        ) {
          // X * O O O _ O X
          result |= VCT_PATTERN_BROKEN_FOUR;
        }
        // X O O _ O X
        continue;
      }
      continue;
    }
  }
  return result;
}

static int vct_white(board *bd, int curr_depth, int max_depth, int *has_vct);
static int vct_black(board *bd, int curr_depth, int max_depth, int *has_vct);

static int vct_white(board *bd, int curr_depth, int max_depth, int *has_vct) {
  bitmap256 checked;
  bitmap256_init(&checked);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (!board_has_piece(bd, pos)) {
      continue;
    }
    int x, y;
    board_to_xy(pos, &x, &y);
    int min_x = max(x - VCT_RANGE, 0);
    int min_y = max(y - VCT_RANGE, 0);
    int max_x = min(x + VCT_RANGE, BOARD_SIZE - 1);
    int max_y = min(y + VCT_RANGE, BOARD_SIZE - 1);
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
        board_put_white(bd, new_pos);
        pattern = check_pattern(bd, new_pos);
        board_remove_white(bd, new_pos);
        if (pattern & VCT_PATTERN_FIVE_IN_A_ROW) {
          return new_pos;
        }
        if (pattern & (VCT_PATTERN_BROKEN_FOUR | VCT_PATTERN_LIVE_FOUR)) {
          board_put_white(bd, new_pos);
          int result = vct_black(bd, curr_depth + 1, max_depth, has_vct);
          board_remove_white(bd, new_pos);
          if (result == -1) {
            return new_pos;
          }
        }
        board_put_black(bd, new_pos);
        pattern = check_pattern(bd, new_pos);
        board_remove_black(bd, new_pos);
        if (pattern & (VCT_PATTERN_LIVE_FOUR | VCT_PATTERN_FIVE_IN_A_ROW)) {
          board_put_white(bd, new_pos);
          int result = vct_black(bd, curr_depth + 1, max_depth, has_vct);
          board_remove_white(bd, new_pos);
          if (result == -1) {
            return new_pos;
          }
        }
      }
    }
  }
  return -1;
}

static int vct_black(board *bd, int curr_depth, int max_depth, int *has_vct) {
  bitmap256 checked;
  bitmap256_init(&checked);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (!board_has_piece(bd, pos)) {
      continue;
    }
    int x, y;
    board_to_xy(pos, &x, &y);
    int min_x = max(x - VCT_RANGE, 0);
    int min_y = max(y - VCT_RANGE, 0);
    int max_x = min(x + VCT_RANGE, BOARD_SIZE - 1);
    int max_y = min(y + VCT_RANGE, BOARD_SIZE - 1);
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
        pattern = check_pattern(bd, new_pos);
        board_remove_black(bd, new_pos);
        if (pattern & VCT_PATTERN_FIVE_IN_A_ROW) {
          return new_pos;
        }
        if (pattern != VCT_PATTERN_NONE) {
          if (curr_depth < max_depth) {
            board_put_black(bd, new_pos);
            int result = vct_white(bd, curr_depth + 1, max_depth, has_vct);
            board_remove_black(bd, new_pos);
            if (result == -1) {
              return new_pos;
            }
          } else {
            *has_vct = 1;
          }
        }
        board_put_white(bd, new_pos);
        pattern = check_pattern(bd, new_pos);
        board_remove_white(bd, new_pos);
        if ((pattern & (VCT_PATTERN_LIVE_FOUR | VCT_PATTERN_FIVE_IN_A_ROW)) && curr_depth < max_depth) {
          board_put_black(bd, new_pos);
          int result = vct_white(bd, curr_depth + 1, max_depth, has_vct);
          board_remove_black(bd, new_pos);
          if (result == -1) {
            return new_pos;
          }
        }
      }
    }
  }
  return -1;
}

static int vct_black_iter_deepening(board *bd, int max_depth) {
  for (int iter_depth = 1; iter_depth <= max_depth; iter_depth += 2) {
    int has_vct = 0;
    int result = vct_black(bd, 1, iter_depth, &has_vct);
    if (!has_vct) {
      return -1;
    }
    if (result != -1) {
      return result;
    }
  }
  return -1;
}

int vct(board *bd) {
  return vct_black_iter_deepening(bd, VCT_MAX_DEPTH);
}
