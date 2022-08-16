#ifndef _SPIN_
#define _SPIN_ 1


typedef uint32_t lock;
uint32_t spinlock;

void acquire(lock*);
void release(lock*);
void init_lock(lock*);
int trylock(lock*);

#endif