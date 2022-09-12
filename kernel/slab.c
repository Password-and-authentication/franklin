/* slab plan

 a slab takes up 1 page and it has
 PGSIZE / blocksize amount of blocks

 the MIN blocksize is 8 bytes, otherwise
 the pointer to the next block will get overwritten

 the slab has a linked list of free blocks
 when the slab is made, the freelist has all the blocks

 when a block is allocated, the free block
 is removed from the linked list

 the free block list starts at the BEGINNING
 of the slab to make it cacheline aligned
 and the slab struct is allocated, at
 the end of the slab

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

// TODO: add macro to iterate linked list and check locks


#include "franklin/mmu.h"
#include "franklin/spinlock.h"
#include <stddef.h>
#include <stdbool.h>


static struct slab* new_slab(size_t);
static struct block* block_alloc(struct slab*);
static void freeslab(struct slab *);
  

#define SLIST_ENTRY(type)			\
  struct {					\
    struct type *next;				\
  }

#define SLIST_NEXT(struc, field) ((struc)->field.next)



// block of memory
struct block {
  struct block *next;
};


struct slab {
  struct slab *next;
  size_t size; // size of a block
  size_t refcount; // the amount of blocks that are in use
  int id; 

  struct block *freelist;
};

static struct {
  struct slab *slab_head;
  lock lock;
} slabber;
/* static struct slab *slab_head; // linked list of slabs */



// allocate 'size' amount of bytes
void*
kalloc(size_t size)
{
  struct slab *slab;
  struct block *blk;

  // MIN block size is 8
  if (size < sizeof(void*))
    size = sizeof(void*);

  acquire(&slabber.lock);

  for (slab = slabber.slab_head; slab;
       slab = slab->next) {
    if (slab->size == size && slab->freelist) {
      blk = block_alloc(slab);
      goto ret;
    }
  }
  // if there is no free slab for the corresponding size,
  // make a new slab and then allocate a block in that slab
  slab = new_slab(size);
  blk = block_alloc(slab);

 ret:
  release(&slabber.lock);
  return (void*)blk;
}


// free a block in a slab
void
kfree(void *ptr)
{
  struct slab* slab;
  struct block *blk = (struct block*)ptr;

  acquire(&slabber.lock);
  for (slab = slabber.slab_head; slab;
       slab->next) {
    if ((void*)slab > ptr || (char*)ptr >= ((char*)slab + PGSIZE))
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
      // otherwise add the block to the free blocks list
      blk->next = slab->freelist;
      slab->freelist = blk;
    }
    break;
  }
  release(&slabber.lock);
};


// allocate new slab and add it to the linked list of slabs
static struct slab*
new_slab(size_t size)
{
  char *start;
  struct slab *slab;
  struct block *blk;
  size_t i = 1;
  static int id;

  // allocate 1 page for the slab
  start = (char*)P2V((uintptr_t)palloc(1));
  blk = (struct block*)start;
  
  slab = (struct slab*)(((char*)start + PGSIZE) - sizeof(struct slab));

  
  slab->size = size;
  slab->refcount = 0;
  slab->id = id++; // id is for debugging
  slab->freelist = blk;

  slab->next = slabber.slab_head;


  slabber.slab_head = slab;


  for (; (char*)blk < ((char*)slab - sizeof(struct block)); ++i) {
    blk->next = (struct block*)(start + (i * size));
    blk = blk->next;
  }

    
  return slab;
}

// allocate one block in the slab
// LOCK HAS TO BE HELD
static struct block*
block_alloc(struct slab *slab)
{
  struct block *blk;

  slab->refcount++;
  
  blk = slab->freelist;
  slab->freelist = slab->freelist->next;
  return blk;
};



// free slab and remove it from the linked list
static void
freeslab(struct slab *slab)
{
  struct slab *prev, *slabptr = slabber.slab_head;
  
  while (slabptr != slab) {
    prev = slabptr;    
    slabptr = slabptr->next;
  }
  prev->next = slabptr->next;
  

  freepg(V2P((uintptr_t)slab), 1);
}


void
test_slab(void)
{


  int *in = kalloc(sizeof(int));
  int *in2 = kalloc(sizeof(int));
  *in = 10;
  *in2 = 100;
  
  char *g = kalloc(8);
  kalloc(8);
  kfree(g);
  char *xx = kalloc(16);
  kalloc(32);
  kalloc(16);
  kalloc(32);
  g = kalloc(8);
  kalloc(8);
  kalloc(8);
  kfree(g);
  kfree(g);
  g = kalloc(8);
  kfree(g);

  
  char *f = palloc(1);
  char *ff = palloc(1);
  kfree(xx);
  g = kalloc(8);
  kfree(g);
  char *addr[1000];

  for (int i = 0; i < 1000; ++i)
    addr[i] = kalloc(8);
  for (int i = 0; i < 1000; ++i)
    kfree(addr[i]);

  struct l {
    int x;
    int y;
  };
  struct l *l = kalloc(sizeof(struct l));
  kfree(l);
  int *x = kalloc(1000);
  kfree(x);

  if (*in != 10 || *in2 != 100)
    panic("panic:_ slab test");

  
  
}
