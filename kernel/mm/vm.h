#ifndef _VM_
#define _VM_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define PTE_PRESENT (1ull << 0ull)
#define PTE_RW (1ull << 1ull)
#define PTE_USER (1ull << 2ull)
#define PTE_NX (1ull << 63ull)

struct vmspace
{};

struct vm_map
{
  uint64_t* top_level;
  struct vm_map_entry* entries;
};

struct vm_map_entry
{
  struct vm_map_entry* next;
  uintptr_t start, end;
  off_t offset;
  void* obj;
};

void
vm_alloc(struct vm_map*, uintptr_t, size_t);

#endif
