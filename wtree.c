#include "wtree.h"

#include <stdlib.h>

#define wtree_malloc malloc
#define wtree_free free

#define WTREE_BUKCET_NUM 16

static void wtree_resize(wtree *wt) {
  uint64_t new_bucket_num = wt->bucket_num << 1;
  wtree_entry **new_buckets = wtree_malloc(sizeof(new_buckets) * new_bucket_num);
  for (uint64_t i = 0; i < new_bucket_num; ++i) {
    new_buckets[i] = NULL;
  }
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wtree_entry *entry = wt->buckets[i];
    wtree_entry *next;
    while (entry != NULL) {
      next = entry->next;
      uint64_t bucket_index = board_hash(&entry->bd) % new_bucket_num;
      entry->next = new_buckets[bucket_index];
      if (new_buckets[bucket_index] == NULL) {
        new_buckets[bucket_index] = entry;
      }
      entry = next;
    }
  }
  wtree_free(wt->buckets);
  wt->bucket_num = new_bucket_num;
  wt->buckets = new_buckets;
}

void wtree_init(wtree *wt) {
  wt->bucket_num = WTREE_BUKCET_NUM;
  wt->buckets = wtree_malloc(sizeof(wt->buckets) * wt->bucket_num);
  wt->item_num = 0;
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wt->buckets[i] = NULL;
  }
}

void wtree_clear(wtree *wt) {
  wtree_free(wt->buckets);
}

void wtree_insert(wtree *wt, board *bd, int pos) {
  if (wt->item_num >= wt->bucket_num) {
    wtree_resize(wt);
  }
  uint64_t bucket_index = board_hash(bd) % wt->bucket_num;
  wtree_entry *entry = wt->buckets[bucket_index];
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      entry->pos = pos;
      return;
    }
    entry = entry->next;
  }
  entry = wtree_malloc(sizeof(entry));
  entry->bd = *bd;
  entry->next = wt->buckets[bucket_index];
  if (wt->buckets[bucket_index] == NULL) {
    wt->buckets[bucket_index] = entry;
  }
}

void wtree_erase(wtree *wt, board *bd) {
  uint64_t bucket_index = board_hash(bd) % wt->bucket_num;
  wtree_entry *entry = wt->buckets[bucket_index];
  wtree_entry *prev = NULL;
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      if (prev != NULL) {
        prev->next = entry->next;
      } else {
        wt->buckets[bucket_index] = entry->next;
      }
      wtree_free(entry);
    }
    prev = entry;
    entry = entry->next;
  }
}

int wtree_find(wtree *wt, board *bd) {
  uint64_t bucket_index = board_hash(bd) % wt->bucket_num;
  wtree_entry *entry = wt->buckets[bucket_index];
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      return entry->pos;
    }
    entry = entry->next;
  }
  return -1;
}

void wtree_foreach(wtree *wt, wtree_foreach_callback callback, void *user_data) {
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wtree_entry *entry = wt->buckets[i];
    while (entry != NULL) {
      int ok = callback(entry, user_data);
      if (!ok) {
        return;
      }
      entry = entry->next;
    }
  }
}
