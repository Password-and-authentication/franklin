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
 to the linked list, but if the slab
 has no more blocks in use, free the whole slab

 all the slabs are in a slab linked list

 when allocating a block, the correct slab
 is found by iterating the linked list and
 comparing the slab block size, with the
 allocation size AND checking if the slab
 has free blocks

*/


#include "franklin/mmu.h"
#include <stddef.h>



static struct slab* new_slab(size_t);
static struct block* block_alloc(struct slab*);
static void freeslab(struct slab *);
  

// block of memory
struct block {
  struct block *next; 
};

struct slab {
  struct slab *next;
  size_t size; // size of a block
  size_t refcount; // the amount of blocks that are in use

  struct block *freelist;
};

static struct slab *slab_head; // linked list of slabs

static struct slab* new_slab(size_t size) {

  struct slab *slab;
  struct block *blk;
  size_t i = 1;

  // palloc(1) allocates one page, and returns the physical address
  // P2V converts it into a virtual address
  slab = (struct slab*) P2V((uintptr_t)palloc(1));
  slab->size = size;
  slab->refcount = 0;
  
  slab->next = slab_head;
  slab_head = slab;

  blk = (struct block*) ((char*)slab + sizeof(struct slab));
  slab->freelist = blk;

  // setup linked list of free blocks
  for (; (char*)blk < (char*)slab + PGSIZE; i++) {
    blk->next = (struct block*) ((char*)slab->freelist + (i * size));
    blk = blk->next;
  }
    
  return slab;
}

// allocate one block in the slab
static struct block* block_alloc(struct slab *slab) {
  struct block *blk;

  slab->refcount++;
  
  blk = slab->freelist;
  slab->freelist = slab->freelist->next;
  return blk;
};

// allocate 'size' amount of bytes
void* kalloc(size_t size) {
  struct slab *slab;

  // MIN block size is 8
  if (size < 8)
    size = 8;
  
  for (slab = slab_head; slab; slab = slab->next) {
    if (slab->size == size && slab->freelist)
      return (void*)block_alloc(slab);
  }
  // if there is no free slab for the corresponding size,
  // make a new slab
  slab = new_slab(size);

  // and then allocate a block in that slab
  return (void*)block_alloc(slab);
}

// free a block in a slab
void kfree(void *ptr) {
  struct slab* slab;
  
  for (slab = slab_head; slab; slab = slab->next) {
    if ((void*)slab > ptr || (char*)ptr > ((char*)slab + PGSIZE))
      continue;

    // if refcount is already 0, it means the slab was already freed
    // and there can be no blocks in the slab
    if (slab->refcount == 0)
      panic("kfree, double free");
    
    slab->refcount--;
    // free the slab if there are no more blocks in use
    if (slab->refcount == 0)
      freeslab(slab);
    else {
      slab->freelist->next = slab->freelist;
      slab->freelist = ptr;
    }

    return;
  }
};


static void freeslab(struct slab *slab) {
  // free the page starting at address 'slab'
  // V2P converts it to a physical address
  freepg(V2P((uintptr_t)slab), 1);
}


