#include <stdint.h>
#include "../69.h"
#include "spinlock.h"



void acquire(lock *lock) {

    while (__sync_val_compare_and_swap(lock, 0, 1) == 1)
        ;
    return;
}

void release(lock *lock) {
    *lock = 0;
}

void init_lock(lock *lock) {
    *lock = 0;
}