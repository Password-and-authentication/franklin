#include "d.h"
#include "franklin/mmu.h"
#include "franklin/spinlock.h"
#include "limine.h"
#include <stdbool.h>
#include <strings.h>

volatile struct limine_memmap_request memmap_request = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0,
};

void*
palloc(uint32_t size)
{
  if (size >= MAXPG)
    panic("panic: palloc, size too big");
  acquire(&spinlock);
  uint32_t page = 0, p = 0;

  while (1) {
    if (page >= MAXPG)
      panic("panic: out of pages\n");
    // if true all pages are inuse
    if (bitmap[page / 64] == INT64_MAX) {
      page++;
      continue;
    }
    if (isfree(page)) {

      // if value is zero all 64 pages are free
      if (bitmap[page / 64] == 0)
        goto setpages;

      // check that there are 'size' amount of pages free
      for (p = page; p < (size + page); ++p) {
        if (!isfree(p))
          break;
      }

      // mark the pages as used
      if (p == size + page) {
      setpages:
        for (p = page; p < (size + page); ++p)
          togglepage(p);
        goto releaselock;
      }
    };
    page++;
  };
releaselock:
  release(&spinlock);
  return ((uint64_t)(page * PGSIZE));
}

void*
pallocaddr(uint32_t size, uint64_t paddr)
{

  uint32_t pfn = paddr / PGSIZE;

  acquire(&spinlock);
  for (uint32_t i = pfn; i < pfn + size; ++i) {
    if (!isfree(i))
      panic("panic: pallocaddr, page is not free\n");
    togglepage(i);
  };
  release(&spinlock);
  return paddr;
}

void
freepg(uint64_t addr, uint32_t length)
{
  uint32_t page = addr / PGSIZE;

  acquire(&spinlock);
  do {
    if (isfree(page))
      panic("freepg: page already free");
    togglepage(page);
  } while (--length && page++);
  release(&spinlock);
}

void
initbmap()
{
  struct limine_memmap_response* memmap = memmap_request.response;
  uint64_t bitmapsz = getmemsz(memmap);
  struct limine_memmap_entry* entry = getentry(memmap, bitmapsz);

  bitmap = (uint64_t*)P2V(entry->base);
  for (uint32_t i = 0; i < bitmapsz; ++i)
    togglepage(entry->base / PGSIZE + i);

  entry->base += bitmapsz;
  entry->length -= bitmapsz;
  for (uint32_t i = 0; i < memmap->entry_count; ++i) {
    if (memmap->entries[i]->type != 0)
      setentry(memmap->entries[i]);
  }
}

void
setentry(struct limine_memmap_entry* entry)
{
  uint32_t page = entry->base / PGSIZE;
  acquire(&spinlock);
  for (uint32_t i = 0; i < (entry->length / PGSIZE); i++, page++) {
    togglepage(page);
  }
  release(&spinlock);
}

uint64_t
getmemsz(struct limine_memmap_response* memmap)
{
  uint64_t size = memmap->entries[14]->base + memmap->entries[14]->length;

  MAXPG = size / PGSIZE;
  size /= PGSIZE;
  size /= 8;
  size /= PGSIZE;
  return size;
}

struct limine_memmap_entry*
getentry(struct limine_memmap_response* memmap, uint64_t mapsize)
{

  uint32_t i = 0;
  while (1) {
    if (memmap->entries[i]->type == 0 && memmap->entries[i]->length > mapsize)
      return memmap->entries[i];
    i++;
  }
  return 0;
}
