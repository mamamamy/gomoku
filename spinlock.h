#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>

typedef struct spinlock spinlock;

struct spinlock {
  atomic_int data;
};

void spinlock_init(spinlock *lock);
void spinlock_lock(spinlock *lock);
void spinlock_unlock(spinlock *lock);

#endif
