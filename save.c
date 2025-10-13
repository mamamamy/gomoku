#include "save.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define WTREE_FILE_PATH "data/wtree_%"PRIu64".bin"
#define WTREE_FILE_PATH_MAX_LEN 36

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

static int board8_has_black(const board8 *bd, int pos) {
  return bd->data[pos / 8] & (1 << (pos % 8));
}

static int board8_has_white(const board8 *bd, int pos) {
  return bd->data[(pos / 8) + 32] & (1 << (pos % 8));
}

static void board_to_board8(const board *bd, board8 *bd8) {
  board8_init(bd8);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board_has_black(bd, pos)) {
      board8_put_black(bd8, pos);
    } else if (board_has_white(bd, pos)) {
      board8_put_white(bd8, pos);
    }
  }
}

static void board8_to_board(const board8 *bd8, board *bd) {
  board_init(bd);
  for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
    if (board8_has_black(bd8, pos)) {
      board_put_black(bd, pos);
    } else if (board8_has_white(bd8, pos)) {
      board_put_white(bd, pos);
    }
  }
}

static int wtree_entry_save_to_file(const wtree_entry *entry, void *user_data) {
  FILE *f = (FILE *)user_data;
  board8 bd8;
  board_to_board8(&entry->bd, &bd8);
  size_t wn;
  wn = fwrite(bd8.data, sizeof(uint8_t), 64, f);
  if (wn != 64) {
    fprintf(stderr, "Write failed\n");
    exit(EXIT_FAILURE);
  }
  uint8_t pos_uint8 = (uint8_t)(entry->pos);
  wn = fwrite(&pos_uint8, sizeof(uint8_t), 1, f);
  if (wn != 1) {
    fprintf(stderr, "Write failed\n");
    exit(EXIT_FAILURE);
  }
  return 1;
}

void save_to_file(wtree *wt, uint64_t id) {
  char filename[WTREE_FILE_PATH_MAX_LEN];
  snprintf(filename, WTREE_FILE_PATH_MAX_LEN, WTREE_FILE_PATH, id);
  FILE* f = fopen(filename, "wb");
  if (f == NULL) {
    fprintf(stderr, "Failed to open the file%s\n", filename);
    exit(EXIT_FAILURE);
  }
  wtree_foreach(wt, wtree_entry_save_to_file, f);
  fflush(f);
  fclose(f);
}

void load_from_file(wtree *wt, uint64_t id) {
  wtree_clear(wt);
  char filename[WTREE_FILE_PATH_MAX_LEN];
  snprintf(filename, WTREE_FILE_PATH_MAX_LEN, WTREE_FILE_PATH, id);
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "Failed to open the file%s\n", filename);
    exit(EXIT_FAILURE);
  }
  board bd;
  board8 bd8;
  size_t rn;
  uint8_t pos_uint8;
  int pos;
  for (;;) {
    rn = fread(bd8.data, sizeof(uint8_t), 64, f);
    if (rn == 0 && feof(f)) {
      break;
    }
    if (rn != 64) {
      fprintf(stderr, "Read failed\n");
      exit(EXIT_FAILURE);
    }
    rn = fread(&pos_uint8, sizeof(uint8_t), 1, f);
    if (rn != 1) {
      fprintf(stderr, "Read failed\n");
      exit(EXIT_FAILURE);
    }
    board8_to_board(&bd8, &bd);
    pos = pos_uint8;
    wtree_insert(wt, &bd, pos);
  }
  fclose(f);
}
