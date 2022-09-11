/* slab plan

 a slab takes up 1 page and it has
 PGSIZE / blocksize amount of blocks

 the MIN blocksize is 8 bytes, otherwise
 the pointer to the next block will get overwritten

 the slab has a linked list of free blocks
 when the slab is made, the freelist has all the blocks

 when a block is allocated, the free block
 is removed from the linked list

 when a block is freed, it is added back
 to the linked list

 all the slabs are in a slab linked list

 when allocating a block, the correct slab
 is found by iterating the linked list and
 comparing the slab block size, with the
 allocation size

*/


#include "franklin/mmu.h"
#include <stddef.h>



static struct slab* new_slab(size_t);
static struct block* slab_alloc(struct slab*);


struct block {
  struct block *next;
};

struct slab {
  struct slab *next;
  size_t size;
  void *start;

  struct block *freelist;
};

static struct slab *slab_head;

static struct slab* new_slab(size_t size) {

  struct slab *slab;
  struct block *blk;
  size_t i = 1;

  slab = P2V((uintptr_t)palloc(1));
  slab->size = size;
  
  slab->next = slab_head;
  slab_head = slab;

  blk = (void*)slab + sizeof(struct slab);
  slab->freelist = blk;

  for (; blk < (void*)slab + PGSIZE; blk = blk->next, ++i)
    blk->next = (void*)slab->freelist + (i * size);


  // slab->freelist = 085c020
  // slab-

  return slab;
}

void init_slab() {
  slab_head = new_slab(8);
}

static struct block* slab_alloc(struct slab *slab) {
  struct block *ret;
  
  ret = slab->freelist;
  slab->freelist = slab->freelist->next;
  return ret;
};

void* kalloc(size_t size) {
  struct slab *slab;

  for (slab = slab_head; slab; slab = slab->next) {
    if (slab->size == size)
      return (void*)slab_alloc(slab);
  }
  slab = new_slab(size);

  return (void*)slab_alloc(slab);
}

void free(void *ptr) {
  struct slab* slab;
  
  for (slab = slab_head; slab; slab = slab->next) {
    if (slab > ptr || ptr > (void*)slab + PGSIZE)
      continue;
    
    slab->freelist->next = slab->freelist;
    slab->freelist = ptr;
  }
};


