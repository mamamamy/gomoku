#include "save.h"

typedef struct board8 board8;

struct board8 {
  uint8_t data[64];
};

static void board8_init(board8 *bd) {
  for (int i = 0; i < 64; ++i) {
    bd->data[i] = 0;
  }
}

static void board8_put_black(board8 *bd, int pos) {
  bd->data[pos / 8] |= 1 << (pos % 8);
}

static void board8_put_white(board8 *bd, int pos) {
  bd->data[(pos / 8) + 32] |= 1 << (pos % 8);
}

static int board8_has_black(board8 *bd, int pos) {
  return bd->data[pos / 8] & (1 << (pos % 8));
}

static int board8_has_white(board8 *bd, int pos) {
  return bd->data[(pos / 8) + 32] & (1 << (pos % 8));
}

static void board_to_board8(board *bd, board8 *bd8) {
  board8_init(bd8);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board_has_black(bd, pos)) {
      board8_put_black(bd8, pos);
    } else if (board_has_white(bd, pos)) {
      board8_put_white(bd8, pos);
    }
  }
}

static void board8_to_board(board8 *bd8, board *bd) {
  board_init(bd);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board8_has_black(bd8, pos)) {
      board_put_black(bd, pos);
    } else if (board8_has_white(bd8, pos)) {
      board_put_white(bd, pos);
    }
  }
}
