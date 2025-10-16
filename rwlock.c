#include "rwlock.h"

void rwlock_init(rwlock *lock) {
  spinlock_init(&lock->lock);
  lock->read_count = 0;
  lock->write_count = 0;
  lock->write_locked = 0;
}

void rwlock_rdlock(rwlock *lock) {
  for (;;) {
    spinlock_lock(&lock->lock);
    if (lock->write_count <= 0) {
      ++lock->read_count;
      spinlock_unlock(&lock->lock);
      break;
    }
    spinlock_unlock(&lock->lock);
    __builtin_ia32_pause();
  }
}

void rwlock_wrlock(rwlock *lock) {
  spinlock_lock(&lock->lock);
  ++lock->write_count;
  if (lock->read_count <= 0 && !lock->write_locked) {
    lock->write_locked = 1;
    spinlock_unlock(&lock->lock);
    return;
  }
  spinlock_unlock(&lock->lock);
  for (;;) {
    spinlock_lock(&lock->lock);
    if (lock->read_count <= 0 && !lock->write_locked) {
      lock->write_locked = 1;
      spinlock_unlock(&lock->lock);
      break;
    }
    spinlock_unlock(&lock->lock);
    __builtin_ia32_pause();
  }
}

void rwlock_unlock(rwlock *lock) {
  spinlock_lock(&lock->lock);
  if (lock->read_count > 0) {
    --lock->read_count;
  } else {
    --lock->write_count;
    lock->write_locked = 0;
  }
  spinlock_unlock(&lock->lock);
}
