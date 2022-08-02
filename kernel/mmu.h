
#define PGSIZE 4096


uint64_t *bitmap;


void* palloc(int);
void initbmap(struct limine_memmap_response*);
void setentry(struct limine_memmap_entry *);
uint64_t getmemsz(struct limine_memmap_response*);
struct limine_memmap_entry* getentry(struct limine_memmap_response *, uint64_t);
