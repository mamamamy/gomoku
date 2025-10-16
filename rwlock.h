#ifndef RWLOCK_H
#define RWLOCK_H

#include "spinlock.h"

typedef struct rwlock rwlock;

struct rwlock {
  spinlock lock;
  int read_count;
  int write_count; // Wait and write thread count
  int write_locked;
};

void rwlock_init(rwlock *lock);
void rwlock_rdlock(rwlock *lock);
void rwlock_wrlock(rwlock *lock);
void rwlock_unlock(rwlock *lock);

#endif
