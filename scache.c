#include "scache.h"

#include <stdlib.h>

#define SCACHE_BUCKET_NUM 16

static void scache_resize(scache *sc) {
  uint64_t new_bucket_num = sc->bucket_num * 2;
  scache_entry **new_buckets = malloc(sizeof(*new_buckets) * new_bucket_num);
  for (uint64_t i = 0; i < new_bucket_num; ++i) {
    new_buckets[i] = NULL;
  }
  // Move to new bucket
  for (uint64_t i = 0; i < sc->bucket_num; ++i) {
    scache_entry *entry = sc->buckets[i];
    scache_entry *next;
    while (entry != NULL) {
      next = entry->next;
      uint64_t bucket_index = board_hash(&entry->bd) % new_bucket_num;
      entry->next = new_buckets[bucket_index];
      new_buckets[bucket_index] = entry;
      entry = next;
    }
  }
  free(sc->buckets);
  sc->bucket_num = new_bucket_num;
  sc->buckets = new_buckets;
  // Resize and init bucket lock
  free(sc->bucket_locks);
  sc->bucket_locks = malloc(sizeof(*sc->bucket_locks) * sc->bucket_num);
  for (uint64_t i = 0; i < sc->bucket_num; ++i) {
    spinlock_init(&sc->bucket_locks[i]);
  }
}

void scache_init(scache *sc) {
  rwlock_init(&sc->lock);
  sc->bucket_num = SCACHE_BUCKET_NUM;
  sc->buckets = malloc(sizeof(*sc->buckets) * sc->bucket_num);
  sc->bucket_locks = malloc(sizeof(*sc->bucket_locks) * sc->bucket_num);
  for (uint64_t i = 0; i < sc->bucket_num; ++i) {
    sc->buckets[i] = NULL;
    spinlock_init(&sc->bucket_locks[i]);
  }
  atomic_init(&sc->item_num, 0);
}

void scache_free(scache *sc) {
  scache_clear(sc);
  free(sc->buckets);
  free(sc->bucket_locks);
}

void scache_clear(scache *sc) {
  rwlock_wrlock(&sc->lock);
  for (uint64_t i = 0; i < sc->bucket_num; ++i) {
    scache_entry *entry = sc->buckets[i];
    scache_entry *next;
    while (entry != NULL) {
      next = entry->next;
      free(entry);
      entry = next;
    }
    sc->buckets[i] = NULL;
  }
  atomic_store_explicit(&sc->item_num, 0, memory_order_release);
  rwlock_unlock(&sc->lock);
}

void scache_insert(scache *sc, const board *bd) {
  rwlock_rdlock(&sc->lock);
  if (atomic_load_explicit(&sc->item_num, memory_order_acquire) >= sc->bucket_num) {
    rwlock_unlock(&sc->lock);
    // Acquire the resize lock and double-check if resizing is needed
    rwlock_wrlock(&sc->lock);
    if (atomic_load_explicit(&sc->item_num, memory_order_relaxed) >= sc->bucket_num) {
      scache_resize(sc);
    }
    rwlock_unlock(&sc->lock);
    rwlock_rdlock(&sc->lock);
  }
  uint64_t bucket_index = board_hash(bd) % sc->bucket_num;
  // Lock head node
  spinlock_lock(&sc->bucket_locks[bucket_index]);
  scache_entry *entry = sc->buckets[bucket_index];
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      spinlock_unlock(&sc->bucket_locks[bucket_index]);
      rwlock_unlock(&sc->lock);
      return;
    }
    entry = entry->next;
  }
  entry = malloc(sizeof(*entry));
  entry->bd = *bd;
  entry->next = sc->buckets[bucket_index];
  sc->buckets[bucket_index] = entry;
  spinlock_unlock(&sc->bucket_locks[bucket_index]);
  atomic_fetch_add_explicit(&sc->item_num, 1, memory_order_release);
  rwlock_unlock(&sc->lock);
}

int scache_exist(scache *sc, const board *bd) {
  rwlock_rdlock(&sc->lock);
  uint64_t bucket_index = board_hash(bd) % sc->bucket_num;
  // Lock head node
  spinlock_lock(&sc->bucket_locks[bucket_index]);
  scache_entry *entry = sc->buckets[bucket_index];
  while (entry != NULL) {
    if (board_equal(&entry->bd, bd)) {
      spinlock_unlock(&sc->bucket_locks[bucket_index]);
      rwlock_unlock(&sc->lock);
      return 1;
    }
    entry = entry->next;
  }
  spinlock_unlock(&sc->bucket_locks[bucket_index]);
  rwlock_unlock(&sc->lock);
  return 0;
}
