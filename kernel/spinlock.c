#include "franklin/69.h"
#include "franklin/spinlock.h"



void acquire(lock *lock) {

    uint32_t expected = 0;
    while (__atomic_compare_exchange_n(lock, &expected, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED) == 0)
        ;
    return;
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
