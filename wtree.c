#include "wtree.h"

#include <stdlib.h>

#define wtree_memory_alloc malloc
#define wtree_memory_free free

#define WTREE_BUCKET_NUM 16

static void wtree_resize(wtree *wt) {
  uint64_t new_bucket_num = wt->bucket_num << 1;
  wtree_entry **new_buckets = wtree_memory_alloc(sizeof(wtree_entry *) * new_bucket_num);
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
      new_buckets[bucket_index] = entry;
      entry = next;
    }
  }
  wtree_memory_free(wt->buckets);
  wt->bucket_num = new_bucket_num;
  wt->buckets = new_buckets;
}

void wtree_init(wtree *wt) {
  wt->bucket_num = WTREE_BUCKET_NUM;
  wt->buckets = wtree_memory_alloc(sizeof(wtree_entry *) * wt->bucket_num);
  wt->item_num = 0;
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wt->buckets[i] = NULL;
  }
  rwlock_init(&wt->lock);
}

void wtree_free(wtree *wt) {
  wtree_clear(wt);
  wtree_memory_free(wt->buckets);
}

void wtree_clear(wtree *wt) {
  rwlock_wrlock(&wt->lock);
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wtree_entry *entry = wt->buckets[i];
    wtree_entry *next;
    while (entry != NULL) {
      next = entry->next;
      wtree_memory_free(entry);
      entry = next;
    }
    wt->buckets[i] = NULL;
  }
  wt->item_num = 0;
  rwlock_unlock(&wt->lock);
}

uint64_t wtree_size(wtree *wt) {
  rwlock_rdlock(&wt->lock);
  uint64_t size = wt->item_num;
  rwlock_unlock(&wt->lock);
  return size;
}

void wtree_insert(wtree *wt, const board *bd, int pos) {
  rwlock_wrlock(&wt->lock);
  if (wt->item_num >= wt->bucket_num) {
    wtree_resize(wt);
  }
  uint64_t bucket_index = board_hash(bd) % wt->bucket_num;
  wtree_entry *entry = wt->buckets[bucket_index];
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      entry->pos = pos;
      rwlock_unlock(&wt->lock);
      return;
    }
    entry = entry->next;
  }
  entry = wtree_memory_alloc(sizeof(wtree_entry));
  entry->bd = *bd;
  entry->pos = pos;
  entry->next = wt->buckets[bucket_index];
  wt->buckets[bucket_index] = entry;
  ++wt->item_num;
  rwlock_unlock(&wt->lock);
}

void wtree_erase(wtree *wt, const board *bd) {
  rwlock_wrlock(&wt->lock);
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
      wtree_memory_free(entry);
      --wt->item_num;
      rwlock_unlock(&wt->lock);
      return;
    }
    prev = entry;
    entry = entry->next;
  }
  rwlock_unlock(&wt->lock);
}

int wtree_find(wtree *wt, const board *bd) {
  rwlock_rdlock(&wt->lock);
  uint64_t bucket_index = board_hash(bd) % wt->bucket_num;
  wtree_entry *entry = wt->buckets[bucket_index];
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      int pos = entry->pos;
      rwlock_unlock(&wt->lock);
      return pos;
    }
    entry = entry->next;
  }
  rwlock_unlock(&wt->lock);
  return -1;
}

void wtree_foreach(wtree *wt, wtree_foreach_callback callback, void *user_data) {
  rwlock_rdlock(&wt->lock);
  for (uint64_t i = 0; i < wt->bucket_num; ++i) {
    wtree_entry *entry = wt->buckets[i];
    while (entry != NULL) {
      int ok = callback(entry, user_data);
      if (!ok) {
        rwlock_unlock(&wt->lock);
        return;
      }
      entry = entry->next;
    }
  }
  rwlock_unlock(&wt->lock);
}
