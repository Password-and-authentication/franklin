
#include <stdint.h>


#define PGSIZE 4096
uint64_t MAXPG;

#define V2P(V) (uintptr_t)V - HHDM_OFFSET
#define P2V(P) (uintptr_t)P + HHDM_OFFSET

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




// VMM  


pml4_t *PML4E;

typedef struct {
    uint64_t *table;
    uint8_t shift;
} Table;


void test(void);
void init_vmm(void);

int mappage(uint64_t, uint64_t, uint8_t);
void remappage(uint64_t, int);
void unmappage(uint64_t);

pte_t *getpte(uint64_t);
uint64_t *newentry(uint64_t*, uint64_t, uint8_t);
void memzero(char*, int);



// PMM

uint8_t isfree(int);
void togglepage(int);

void* palloc(int);
void* pallocaddr(int, uint64_t);
void freepg(void*, int);

void initbmap(struct limine_memmap_response *);
void setentry(struct limine_memmap_entry *);
uint64_t getmemsz(struct limine_memmap_response*);
struct limine_memmap_entry* getentry(struct limine_memmap_response*, uint64_t);

