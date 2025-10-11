#include "wtree.h"

#include <stdlib.h>

#define wtree_malloc malloc
#define wtree_free free
#define wtree_realloc realloc

#define WTREE_BUKCET_NUM 16

static void wtree_resize(wtree *wt) {

}

void wtree_init(wtree *wt) {
  wt->bucket_num = WTREE_BUKCET_NUM;
  wt->buckets = wtree_malloc(sizeof(wt->buckets) * wt->bucket_num);
  wt->item_num = 0;
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wt->buckets[i] = NULL;
  }
}

void wtree_free(wtree *wt) {
  wtree_free(wt->buckets);
}

void wtree_insert(wtree *wt, board *bd, int pos) {
  if (wt->item_num >= wt->bucket_num) {
    wtree_resize(wt);
  }
}

void wtree_erase(wtree *wt, board *bd);
int wtree_find(wtree *wt, board *bd);
