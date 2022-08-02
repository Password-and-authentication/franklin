#include "limine.h"
#include "mmu.h"
#include "defs.h"


static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

#define setpage(p, x) (bitmap[p / 64] = x << (p % 64) | bitmap[p / 64])
#define isfree(p) ((bitmap[p / 64] & ((1 << (p % 64)))) == 0)

void *palloc(int size) {
    struct limine_hhdm_response *hhdm_res = hhdm_req.response;
    int page = PGSIZE, p = 0;
    while (1) {

        // if free 
        if (isfree(page)) {

            // check that there are 'size' amount of pages free 
            for (p = page; p < (size + page); ++p) {
                if (!isfree(p))
                    break;
            }

            // mark the pages as used
            if (p == size + page) {
                for (p = page; p < (size + page); ++p) 
                    setpage(p, 1);
                return (void*) hhdm_res + (page * PGSIZE);
            }
        };
        page++;
    };
}   

void initbmap(struct limine_memmap_response *memmap) {
    uint64_t bitmapsz = getmemsz(memmap);
    struct limine_memmap_entry *entry = getentry(memmap, bitmapsz);
    struct limine_hhdm_response *hhdm_res = hhdm_req.response;

    bitmap = (uint64_t) hhdm_res->offset + entry->base;
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
