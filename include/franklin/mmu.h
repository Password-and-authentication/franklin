#ifndef _MMU_
#define _MMU_
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define PGSIZE 4096
uint64_t MAXPG;

uint64_t V2P(uint64_t);
uint64_t P2V(uint64_t);

typedef uint64_t pde_t;
typedef uint64_t pte_t;
typedef uint64_t pml4_t;
typedef uint64_t pdpte_t;

uint64_t *bitmap;

#define PRESENT     0x1
#define RW          1
#define USER        2
#define PWT         3
#define PCD         4
#define ACCESSED    5
#define DIRTY       6
#define PG_SIZE     7
#define PDE_ADDR    12

#define KFLAGS 0x3
#define PAGE_SHIFT 12




// 7 FIGURE SLABBER

void *kalloc(size_t);


// VMM  


pml4_t *PML4E;

typedef struct {
    uint64_t *table;
    uint8_t shift;
} Table;


void test(void);
void init_vmm(void);

uint8_t mappage(uint64_t, uint64_t, uint8_t);
void remappage(uint64_t, int);
void unmappage(uint64_t);

static pte_t *getpte(uint64_t);
uint64_t *newentry(uint64_t*, uint64_t, uint8_t);




// PMM

bool isfree(uint32_t);
void togglepage(uint32_t);

void* palloc(uint32_t);
void* pallocaddr(uint32_t, uint64_t);
void freepg(uint64_t, uint32_t);

void initbmap(struct limine_memmap_response *);
void setentry(struct limine_memmap_entry *);
uint64_t getmemsz(struct limine_memmap_response*);
struct limine_memmap_entry* getentry(struct limine_memmap_response*, uint64_t);

#endif
