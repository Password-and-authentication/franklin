/* slab plan

 a slab takes up 1 page and it has
 PGSIZE / blocksize amount of blocks

 the MIN blocksize is 8 bytes, otherwise
 the pointer to the next block will get overwritten

 the slab has a linked list of free blocks
 when the slab is made, the freelist has all the blocks

 the block size is rounded up to a power of 2,
 to reduce the number of slabs, since finding
 the correct slab is O(n)
 

 allocating a block is O(1)

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

#include "d.h"
#include "franklin/mmu.h"
#include "franklin/misc.h"
#include "franklin/spinlock.h"
#include "franklin/list.h"
#include <stddef.h>
#include <stdbool.h>


static struct slab* newslab(size_t);
static struct block* allocblock(struct slab*);
static void freeslab(struct slab *);
uint32_t roundup(uint32_t);



struct large_alloc_metadata {
  size_t pagecount;
};


// block of memory
struct block {
  SLIST_ENTRY(block) blocks;
};

SLIST_HEAD(blocks, block);

struct slab {
  SLIST_ENTRY(slab) slabs;
  size_t size; // size of a block
  size_t refcount; // the amount of blocks that are in use
  int id;

  struct blocks freelist;
};

SLIST_HEAD(slabs, slab);


static struct {
  struct slabs slabs;
  lock lock;
} slabber;


static void *kalloclarge(size_t);
static void kfreelarge(void*);

// allocate 'size' amount of bytes
void*
kalloc(size_t size)
{
  struct slab *slab;
  struct block *block;


  if (size > PGSIZE)
    return kalloclarge(size);
  if (size < sizeof(void*)) // MIN blocksize is 8
    size = sizeof(void*);


  size = (uint32_t)roundup((uint32_t)size);

  acquire(&slabber.lock);

  // loop until a slab is found
  SLIST_FOREACH(slab, &slabber.slabs, slabs) {
    if (slab->size == size && SLIST_FIRST(&slab->freelist)) {
      block = allocblock(slab);
      goto ret;
    }
  }
  // if there is no free slab for the corresponding size,
  // make a new slab and then allocate a block in that slab
  slab = newslab(size);
  block = allocblock(slab);

 ret:
  release(&slabber.lock);
  return (void*)block;
}


// free a block in a slab
void
kfree(void *ptr)
{
  struct slab* slab;
  struct block *block = (struct block*)ptr;
  void* startaddr;

  if (((uintptr_t)ptr & 0xfff) == 0) {
    kfreelarge(ptr);
    return;
  }
  acquire(&slabber.lock);
  
  SLIST_FOREACH(slab, &slabber.slabs, slabs) {

    // startaddr is where the slab starts and
    // where the first block is allocated
    startaddr = (char*)slab - (PGSIZE - sizeof(*slab));
    if (ptr < startaddr || ptr >= (void*)slab)
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
      SLIST_INSERT_HEAD(block, &slab->freelist, blocks);
    }
    break;
  }
  release(&slabber.lock);
};


static void*
kalloclarge(size_t size)
{
  struct large_alloc_metadata *metadata;
  size_t pagecount = DIV_ROUNDUP(size, PGSIZE);

  if ((metadata = palloc(pagecount + 1)) == NULL)
    return NULL;

  metadata = P2V((uintptr_t)metadata);
  metadata->pagecount = pagecount;

  return (char*)metadata + PGSIZE;
}

static void
kfreelarge(void *ptr)
{
  struct large_alloc_metadata *metadata;
  metadata = (char*)ptr - PGSIZE;

  freepg(V2P((uintptr_t)metadata), metadata->pagecount);
}


// allocate new slab and add it to the linked list of slabs
static struct slab*
newslab(size_t size)
{
  char *start;
  struct slab *slab;
  struct block *block;
  size_t i = 1;
  static int id;

  // allocate 1 page for the slab
  start = (char*)P2V((uintptr_t)palloc(1));
  slab = (struct slab*)start;
  
  block = (struct block*)((char*)start + sizeof*slab);
  
  slab->size = size;
  slab->refcount = 0;
  slab->id = id++; // id is for debugging
  
  // set first block to point at allocated memory
  slab->freelist.first = block;

  // set the blocks in the slab's free blocks list
  // check block + size, otherwise the last block
  // will get allocated to the slab struct address
  for (start = (char*)block + size; (char*)block + size < ((char*)slab + PGSIZE); start += size)
    SLIST_INSERT_AFTER((struct block*)start, block, blocks);
  
  // add the slab to the linked list of slabs
  SLIST_INSERT_HEAD(slab, &slabber.slabs, slabs);
  return slab;
}

// allocate one block in the slab
// by removing it from the free blocks list
// LOCK HAS TO BE HELD
static struct block*
allocblock(struct slab *slab)
{
  struct block *block;

  slab->refcount++;
  SLIST_REMOVE_HEAD(block, &slab->freelist, blocks);
  return block;
};



// free slab and remove it from the linked list
static void
freeslab(struct slab *slab)
{
    
  SLIST_REMOVE_ITEM(slab, &slabber.slabs, slab, slabs);
  freepg(V2P((uintptr_t)slab), 1);
}

// round up n to the next power of 2
uint32_t roundup(uint32_t n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}



void
test_slab(void)
{

  char *sex = kalloc(10000);
  kfree(sex);

  char *pussy = kalloc(50000);
  kfree(pussy);
  kalloc(8);
  kalloc(50);
  char *sss = kalloc(16);
  kalloc(32);
  kfree(sss);

  
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
