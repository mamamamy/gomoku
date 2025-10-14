#include "board.h"
#include "vct.h"

#include <stdio.h>
#include <assert.h>

static void clear_stdin() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

static void vs(board *bd) {
  int pos, x, y;
  assert(board_check_has_overlapp(bd));
  board_print(bd);
  for (;;) {
    printf("Computer is thinking...\n");
    pos = vct(bd);
    if (pos == -1) {
      printf("Black fails to VCT\n");
      break;
    }
    board_to_xy(pos, &x, &y);
    printf("(%x, %x)\n", x, y);
    board_put_black(bd, pos);
    board_print(bd);
    if (vct_check_pattern(bd, pos) & VCT_PATTERN_FIVE_IN_A_ROW) {
      printf("Black win\n");
      break;
    }
    printf("Please input white's position (hex, 0-e), like 7 7 or a 5\n");
    for (;;) {
      int n = scanf("%x %x", &x, &y);
      if (n != 2) {
        printf("Fails to scanf need 2, got %d\n", n);
        clear_stdin();
        continue;
      }
      pos = board_to_pos(x, y);
      if (!board_is_valid_xy(x, y)) {
        printf("Invalid position %x %x\n", x, y);
        continue;
      }
      if (board_has_piece(bd, pos)) {
        printf("(%x, %x) has piece\n", x, y);
        continue;
      }
      break;
    }
    board_put_white(bd, pos);
    board_print(bd);
    if (vct_check_pattern(bd, pos) & VCT_PATTERN_FIVE_IN_A_ROW) {
      printf("White win\n");
      break;
    }
  }
  printf("Press Enter to continue...\n");
  clear_stdin();
  getchar();
}

static void test1() {
  printf("Test1 start\n");
  
  board bd;
  
  board_init(&bd);

  board_put_black(&bd, board_to_pos(7, 7));
  board_put_black(&bd, board_to_pos(8, 7));
  board_put_black(&bd, board_to_pos(8, 6));

  board_put_white(&bd, board_to_pos(0, 0));
  board_put_white(&bd, board_to_pos(0, 1));
  board_put_white(&bd, board_to_pos(0, 2));

  vs(&bd);

  printf("Test1 end\n");
}

static void test2() {
  printf("Test2 start\n");
  board bd;
  
  board_init(&bd);

  board_put_black(&bd, board_to_pos(7, 7));
  board_put_black(&bd, board_to_pos(8, 7));
  board_put_black(&bd, board_to_pos(9, 7));
  board_put_black(&bd, board_to_pos(10, 7));
  board_put_black(&bd, board_to_pos(10, 6));
  board_put_black(&bd, board_to_pos(10, 5));
  board_put_black(&bd, board_to_pos(12, 6));
  board_put_black(&bd, board_to_pos(8, 8));
  board_put_black(&bd, board_to_pos(7, 11));
  board_put_black(&bd, board_to_pos(9, 11));

  board_put_white(&bd, board_to_pos(6, 7));
  board_put_white(&bd, board_to_pos(6, 8));
  board_put_white(&bd, board_to_pos(6, 9));
  board_put_white(&bd, board_to_pos(7, 9));
  board_put_white(&bd, board_to_pos(9, 9));
  board_put_white(&bd, board_to_pos(8, 10));
  board_put_white(&bd, board_to_pos(10, 8));
  board_put_white(&bd, board_to_pos(11, 7));
  board_put_white(&bd, board_to_pos(8, 6));
  board_put_white(&bd, board_to_pos(9, 6));

  vs(&bd);
  
  printf("Test2 end\n");
}

static void test3() {
  printf("Test3 start\n");
  board bd;
  
  board_init(&bd);

  board_put_black(&bd, board_to_pos(7, 7));
  board_put_black(&bd, board_to_pos(7, 8));
  board_put_black(&bd, board_to_pos(7, 9));
  board_put_black(&bd, board_to_pos(8, 9));
  board_put_black(&bd, board_to_pos(9, 8));
  board_put_black(&bd, board_to_pos(8, 6));
  board_put_black(&bd, board_to_pos(9, 5));
  board_put_black(&bd, board_to_pos(10, 6));
  board_put_black(&bd, board_to_pos(5, 7));

  board_put_white(&bd, board_to_pos(7, 6));
  board_put_white(&bd, board_to_pos(6, 6));
  board_put_white(&bd, board_to_pos(4, 6));
  board_put_white(&bd, board_to_pos(9, 6));
  board_put_white(&bd, board_to_pos(10, 4));
  board_put_white(&bd, board_to_pos(8, 7));
  board_put_white(&bd, board_to_pos(10, 8));
  board_put_white(&bd, board_to_pos(7, 10));
  board_put_white(&bd, board_to_pos(5, 9));

  vs(&bd);
  
  printf("Test3 end\n");
}

int main() {
  test1();
  test2();
  test3();

  return 0;
}
