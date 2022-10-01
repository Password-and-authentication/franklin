#include "vm.h"

void* kalloc(size_t);

void
vm_alloc(struct vm_map* map, uintptr_t vaddr, size_t size)
{
  struct vm_map_entry* entry;

  entry = kalloc(sizeof *entry);

  entry->start = vaddr;
  entry->end = vaddr + size;

  entry->next = map->entries;
  map->entries = entry;
}
