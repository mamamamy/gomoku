#ifndef SCACHE_H
#define SCACHE_H

#include "board.h"
#include "rwlock.h"
#include "spinlock.h"

#include <stdint.h>
#include <stdatomic.h>

typedef struct scache scache;
typedef struct scache_entry scache_entry;

struct scache_entry {
  board bd;
  scache_entry *next;
};

struct scache {
  scache_entry **buckets;
  atomic_uint_fast64_t item_num;
  uint64_t bucket_num;
  spinlock *bucket_locks;
  rwlock lock; // Read & Write shared, Resize & Clear exclusive
};

void scache_init(scache *sc);
void scache_free(scache *sc);
void scache_clear(scache *sc);
void scache_insert(scache *sc, const board *bd);
int scache_exist(scache *sc, const board *bd);

#endif
