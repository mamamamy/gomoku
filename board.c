#include "board.h"

void board_init(board *bd) {
  for (int i = 0; i < 8; ++i) {
    bd->b[i] = 0;
    bd->w[i] = 0;
  }
}

void board_put_black(board *bd, int pos) {
  bd->b[pos / 32] |= 1 << (pos % 32);
}

void board_put_white(board *bd, int pos) {
  bd->w[pos / 32] |= 1 << (pos % 32);
}

void board_remove_black(board *bd, int pos) {
  bd->b[pos / 32] &= ~(1 << (pos % 32));
}

void board_remove_white(board *bd, int pos) {
  bd->w[pos / 32] &= ~(1 << (pos % 32));
}

void board_remove_piece(board *bd, int pos) {
  board_remove_black(bd, pos);
  board_remove_white(bd, pos);
}

int board_has_black(const board *bd, int pos) {
  return bd->b[pos / 32] & (1 << (pos % 32));
}

int board_has_white(const board *bd, int pos) {
  return bd->w[pos / 32] & (1 << (pos % 32));
}

int board_has_piece(const board *bd, int pos) {
  return board_has_black(bd, pos) || board_has_white(bd, pos);
}

void board_flip_horizontal(board* bd) {
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = 0; x < BOARD_SIZE / 2; ++x) {
      int mirror_x = BOARD_SIZE - 1 - x;
      int pos_a = board_to_pos(x, y);
      int pos_b = board_to_pos(mirror_x, y);
      int is_black_a = board_has_black(bd, pos_a);
      int is_white_a = board_has_white(bd, pos_a);
      int is_black_b = board_has_black(bd, pos_b);
      int is_white_b = board_has_white(bd, pos_b);
      board_remove_piece(bd, pos_a);
      board_remove_piece(bd, pos_b);
      if (is_black_a) {
        board_put_black(bd, pos_b);
      } else if (is_white_a) {
        board_put_white(bd, pos_b);
      }
      if (is_black_b) {
        board_put_black(bd, pos_a);
      } else if (is_white_b) {
        board_put_white(bd, pos_a);
      }
    }
  }
}

void board_rotate_clockwise_90(board* bd) {
  for (int y = 0; y < BOARD_SIZE / 2; ++y) {
    for (int x = 0; x < (BOARD_SIZE + 1) / 2; ++x) {
      int pos_a = board_to_pos(x, y);
      int pos_b = board_to_pos(y, BOARD_SIZE - 1 - x);
      int pos_c = board_to_pos(BOARD_SIZE - 1 - x, BOARD_SIZE - 1 - y);
      int pos_d = board_to_pos(BOARD_SIZE - 1 - y, x);
      int is_black_a = board_has_black(bd, pos_a);
      int is_white_a = board_has_white(bd, pos_a);
      int is_black_b = board_has_black(bd, pos_b);
      int is_white_b = board_has_white(bd, pos_b);
      int is_black_c = board_has_black(bd, pos_c);
      int is_white_c = board_has_white(bd, pos_c);
      int is_black_d = board_has_black(bd, pos_d);
      int is_white_d = board_has_white(bd, pos_d);
      board_remove_piece(bd, pos_a);
      board_remove_piece(bd, pos_b);
      board_remove_piece(bd, pos_c);
      board_remove_piece(bd, pos_d);
      if (is_black_b) {
        board_put_black(bd, pos_a);
      } else if (is_white_b) {
        board_put_white(bd, pos_a);
      }
      if (is_black_c) {
        board_put_black(bd, pos_b);
      } else if (is_white_c) {
        board_put_white(bd, pos_b);
      }
      if (is_black_d) {
        board_put_black(bd, pos_c);
      } else if (is_white_d) {
        board_put_white(bd, pos_c);
      }
      if (is_black_a) {
        board_put_black(bd, pos_d);
      } else if (is_white_a) {
        board_put_white(bd, pos_d);
      }
    }
  }
}

uint64_t board_hash(const board *bd) {
  uint64_t h = 0xAAAAAAAA;
  for (int i = 0; i < 8; ++i) {
    h = h * 31 + bd->b[i];
    h = h * 31 + bd->w[i];
  }
  return h ^ (h >> 32);
}

int board_equal(const board *a, const board *b) {
  for (int i = 0; i < 8; ++i) {
    if (a->b[i] != b->b[i] || a->w[i] != b->w[i]) {
      return 0;
    }
  }
  return 1;
}

void board_print(const board *bd) {
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = 0; x < BOARD_SIZE; ++x) {
      int pos = board_to_pos(x, y);
      if (board_has_black(bd, pos)) {
        printf("X  ");
      } else if (board_has_white(bd, pos)) {
        printf("O  ");
      } else {
        printf("_  ");
      }
    }
    printf("\n");
  }
  printf("\n");
}

int board_check_has_overlapp(const board *bd) {
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board_has_black(bd, pos) && board_has_white(bd, pos)) {
      return 0;
    }
  }
  return 1;
}

int board_check_win(const board *bd, int pos) {
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

void board_to_xy(int pos, int *x, int *y) {
  *x = pos % BOARD_SIZE;
  *y = pos / BOARD_SIZE;
}

int board_to_pos(int x, int y) {
  return y * BOARD_SIZE + x;
}

int board_is_valid_xy(int x, int y) {
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

int board_is_valid_pos(int pos) {
  return pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE;
}
