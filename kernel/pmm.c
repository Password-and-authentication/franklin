#include "d.h"
#include "franklin/bitmap.h"
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

int g = 10;

void
freepg(uint64_t addr, uint32_t length)
{
  uint32_t page = addr / PGSIZE;

  /* acquire(&spinlock); */
  do {
    if (bitmap_test(bitmap, page) == 0)
      g = 69;
    bitmap_reset(bitmap, page);
  } while (--length && page++);
  /* release(&spinlock); */
}

// 0xfd000000
// 0x300000

void*
palloc(size_t pages)
{
  size_t p = 0, i;
  /* acquire(&spinlock); */

  for (;;) {
    for (i = p; i < p + pages; ++i)
      if (bitmap_test(bitmap, i))
        break;

    if (i == p + pages) {
      for (i = p; i < p + pages; ++i) {
        bitmap_set(bitmap, i);
      }
      break;
    }
    p++;
  }
  /* release(&spinlock); */
  return p * PGSIZE;
}

/*
  Initialize bitmap by iterating over it 3 times

  Iterations:
  1. get the highest usable memory address
  2. find a memory area where to allocate the bitmap
  3. free all usable memory areas on bitmap
*/
void
initbmap()
{
  struct limine_memmap_response* memmap = memmap_request.response;
  struct limine_memmap_entry* entry;
  size_t i, j, bitmap_size;
  uintptr_t highest_addr = 0;

  for (i = 0; i < memmap->entry_count; i++) {
    entry = memmap->entries[i];

    if (entry->type != 0)
      continue;
    if (highest_addr < entry->base + entry->length)
      highest_addr = entry->base + entry->length;
  }
  bitmap_size = (highest_addr / PGSIZE) / 8;

  for (i = 0; i < memmap->entry_count; ++i) {
    entry = memmap->entries[i];
    if (entry->type != 0)
      continue;
    if (entry->length >= bitmap_size) {
      bitmap = (char*)P2V(entry->base);

      // init bitmap by setting every page non free
      memset(bitmap, 0xff, bitmap_size);

      entry->length -= bitmap_size;
      entry->base += bitmap_size;
      break;
    }
  }

  for (i = 0; i < memmap->entry_count; ++i) {
    entry = memmap->entries[i];
    if (entry->type != 0)
      continue;
    for (j = 0; j < entry->length; j += PGSIZE) {
      bitmap_reset(bitmap, (entry->base + j) / PGSIZE);
    }
  }
}
