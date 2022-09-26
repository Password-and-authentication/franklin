#include "franklin/spinlock.h"
#include "d.h"
#include "franklin/69.h"
#include <stdatomic.h>

void
acquire(lock* lock)
{

  while (atomic_exchange(lock, 1))
    ;
}

void
release(lock* lock)
{
  uint32_t val = 0;
  __atomic_store(lock, &val, __ATOMIC_RELEASE);
}

int
trylock(lock* lock)
{
  int v;
  __atomic_store(&v, lock, __ATOMIC_RELAXED);
  return v;
}

void
init_lock(lock* lock)
{
  *lock = 0;
}

void
init_and_acquire(lock* lock)
{
  init_lock(lock);
  acquire(lock);
}
