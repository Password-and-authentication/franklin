
#include <stdint.h>


#define PGSIZE 4096

typedef uint64_t pde_t;
typedef uint64_t pte_t;
typedef uint64_t pml4_t;
typedef uint64_t pdpte_t;



uint64_t *bitmap;

#define PRESENT     0
#define RW          1
#define USER        2
#define PWT         3
#define PCD         4
#define ACCESSED    5
#define DIRTY       6
#define PG_SIZE     7
#define PDE_ADDR    12

#define KFLAGS 0x5

pde_t *pgdir;
pml4_t *pml4e;



uint64_t *newentry(uint64_t*);

void mappage(uint64_t);
void init_vmm(void);
void mappages(pde_t*, int);
void newpte(pte_t*);
pte_t *newpde(pte_t*);
void newdirentry(pde_t*, int);
pte_t* pgdirentry(pde_t*);



void* palloc(int);
void freepg(void*, int);

void initbmap(struct limine_memmap_response*);
void setentry(struct limine_memmap_entry *);
uint64_t getmemsz(struct limine_memmap_response*);
struct limine_memmap_entry* getentry(struct limine_memmap_response *, uint64_t);
