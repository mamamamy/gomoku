#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define BOARD_SIZE 15

typedef struct board board;

struct board {
  uint32_t b[8];
  uint32_t w[8];
};

void board_init(board *bd);
void board_put_black(board *bd, int pos);
void board_put_white(board *bd, int pos);
void board_remove_black(board *bd, int pos);
void board_remove_white(board *bd, int pos);
void board_remove_piece(board *bd, int pos);
int board_has_black(const board *bd, int pos);
int board_has_white(const board *bd, int pos);
int board_has_piece(const board *bd, int pos);
void board_flip_horizontal(board* bd);
void board_rotate_clockwise_90(board* bd);
uint64_t board_hash(const board *bd);
int board_equal(const board *a, const board *b);

void board_to_xy(int pos, int *x, int *y);
int board_to_pos(int x, int y);
int board_is_valid_xy(int x, int y);
int board_is_valid_pos(int pos);

#endif
