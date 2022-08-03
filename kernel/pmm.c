#include "limine.h"
#include "mmu.h"
#include "defs.h"



void togglepage(int page) {
    bitmap[page / 64] ^= 1 << (page % 64);
};
uint8_t isfree(int page) {
    return ((bitmap[page / 64] & (((1 << (page % 64))))) == 0);
}


void *palloc(int size) {
    int page = 0, p = 0;
    int x;
    while (1) {

        if (isfree(page)) {

            // check that there are 'size' amount of pages free 
            for (p = page; p < (size + page); ++p) {
                if (!isfree(p))
                    break;
            }

            // mark the pages as used
            if (p == size + page) {
                for (p = page; p < (size + page); ++p) 
                    togglepage(p);
                return (void*) HHDM_OFFSET + (page * PGSIZE);
            }
        };
        page++;
    };
}   


void *pallocaddr(int size, uint64_t paddr) {
    
    int pfn = paddr / PGSIZE;
    for (int i = pfn; i < pfn + size; ++i) {
        if (!isfree(i))
            panic("panic: pallocaddr, page is not free\n");
        togglepage(i);
    };
    return (void*)P2V(paddr);
}


void freepg(void *addr, int length) {
    int page = ((uint64_t)addr - (uint64_t)HHDM_OFFSET) / PGSIZE;

    do {
        togglepage(page);
    } while(--length && page++);
}

void initbmap(struct limine_memmap_response *memmap) {
    uint64_t bitmapsz = getmemsz(memmap);
    struct limine_memmap_entry *entry = getentry(memmap, bitmapsz);

    bitmap = (uint64_t)HHDM_OFFSET + entry->base;
    entry->base += bitmapsz;
    entry->length -= bitmapsz;
    for (int i = 0; i < memmap->entry_count; ++i) {
        setentry(memmap->entries[i]);
    }

}


void setentry(struct limine_memmap_entry *entry) {
    uint_t j = entry->base;
    int inuse = (entry->type == 0) ? 0 : 1;

    for (int i = 0; i < (entry->length / PGSIZE); i++, j++) {
        bitmap[j / 64] = inuse << (j % 64) | bitmap[j / 64];
    }
}


uint64_t getmemsz(struct limine_memmap_response* memmap) {
    uint64_t size = memmap->entries[14]->base + memmap->entries[14]->length;

    size /= PGSIZE;
    size /= 8;
    size /= PGSIZE;
    return size;
}

struct limine_memmap_entry* getentry(struct limine_memmap_response *memmap, uint64_t mapsize) {

    int i = 0;
    while (1) {
        if (memmap->entries[i]->type == 0 && memmap->entries[i]->length > mapsize)
            return memmap->entries[i];
        i++;
    }
    return 0;
}
