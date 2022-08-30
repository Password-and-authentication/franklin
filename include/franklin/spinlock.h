#ifndef _SPIN_
#define _SPIN_ 1

#include <stdint.h>

typedef uint32_t lock;
uint32_t spinlock;

void acquire(lock*);
void release(lock*);
void init_lock(lock*);
uint8_t trylock(lock*);

#endif
