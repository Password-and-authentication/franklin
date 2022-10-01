#ifndef _VM_
#define _VM_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct vmspace
{};

struct vm_map
{
  struct vm_map_entry* vmap;
};

struct vm_map_entry
{
  struct vm_map_entry* next;
  int start, end;
  off_t offset;
  void* obj;
};

#endif
