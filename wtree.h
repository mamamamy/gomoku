#ifndef WTREE_H
#define WTREE_H

#include "board.h"

typedef struct wtree wtree;
typedef struct wtree_entry wtree_entry;

struct wtree_entry {
  board bd; // key
  int pos; // value
  wtree_entry *next;
};

struct wtree {
  wtree_entry **buckets;
  uint64_t item_num;
  uint64_t bucket_num;
};

void wtree_init(wtree *wt);
void wtree_free(wtree *wt);
void wtree_insert(wtree *wt, board *bd, int pos);
void wtree_erase(wtree *wt, board *bd);
int wtree_find(wtree *wt, board *bd);

#endif
