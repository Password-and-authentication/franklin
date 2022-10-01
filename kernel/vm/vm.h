#ifndef _VM_
#define _VM_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct vmspace
{};

struct vm_map
{
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
