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

int board_has_black(board *bd, int pos) {
  return bd->b[pos / 32] & (1 << (pos % 32));
}

int board_has_white(board *bd, int pos) {
  return bd->w[pos / 32] & (1 << (pos % 32));
}

int board_has_piece(board *bd, int pos) {
  return board_has_black(bd, pos) || board_has_white(bd, pos);
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
