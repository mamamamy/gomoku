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

#endif
