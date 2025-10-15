#include "spinlock.h"

void spinlock_init(spinlock *lock) {
  atomic_init(&lock->data, 0);
}

void spinlock_lock(spinlock *lock) {
  for (;;) {
    if (!atomic_exchange_explicit(&lock->data, 1, memory_order_acquire)) {
      break;
    }
    while (atomic_load_explicit(&lock->data, memory_order_relaxed)) {
      __builtin_ia32_pause();
    }
  }
}

void spinlock_unlock(spinlock *lock) {
  atomic_store_explicit(&lock->data, 0, memory_order_release);
}
