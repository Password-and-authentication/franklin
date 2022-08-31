#include "franklin/69.h"
#include "d.h"
#include "franklin/spinlock.h"
#include <stdatomic.h>



void acquire(lock *lock) {

  while (atomic_exchange(lock, 1))
    ;
}

void release(lock *lock) {
    uint32_t val = 0;
    __atomic_store(lock, &val, __ATOMIC_RELEASE);
}

// int trylock(lock *lock) {
//     return (__sync_val_compare_and_swap(lock, 0, 1) == 1);
// }

void init_lock(lock *lock) {
    *lock = 0;
}
