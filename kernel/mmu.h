
#include <stdint.h>


#define 

typedef uint64_t pde_t;
typedef uint64_t pte_t;



uint64_t *bitmap;
pde_t *pgdir;
pte_t *pgtable;

void init_vmm(void);



void* palloc(int);
void freepg(void*, int);

void initbmap(struct limine_memmap_response*);
void setentry(struct limine_memmap_entry *);
uint64_t getmemsz(struct limine_memmap_response*);
struct limine_memmap_entry* getentry(struct limine_memmap_response *, uint64_t);
