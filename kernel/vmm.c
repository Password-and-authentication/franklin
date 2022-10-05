#include "d.h"
#include "franklin/69.h"
#include "franklin/misc.h"
#include "franklin/mmu.h"
#include "limine.h"
#include "mm/vm.h"
#include "std/string.h"
#include <errno.h>
#include <stddef.h>

#define PTE_ADDR_MASK 0x000ffffffffff000
#define PTE_GET_ADDR(value) (value & PTE_ADDR_MASK)

uint64_t
V2P(uint64_t V)
{
  return V - HHDM_OFFSET;
}

uint64_t
P2V(uint64_t P)
{
  return P + HHDM_OFFSET;
}

void
init_vmm()
{
  /* PML4E = (pml4_t*)P2V((uint64_t)palloc(1)); */
  /* memzero((uint8_t*)PML4E, PGSIZE); */
  /* test(); */
}

typedef char symbol[];

// from linker.ld
extern symbol text_start_addr, text_end_addr, rodata_start_addr,
  rodata_end_addr, data_start_addr, data_end_addr;

// 0xfd000000
// 0x300000

uint64_t*
get_next_level(uint64_t* top_level, size_t idx, bool alloc)
{
  if (top_level[idx] & PTE_PRESENT) {
    return P2V((uint64_t)PTE_GET_ADDR(top_level[idx]));
  }

  if (!alloc) {
    return NULL;
  }

  uint64_t* entry = palloc(1);
  memset(P2V(entry), 0, PGSIZE);
  top_level[idx] = (uint64_t)entry | PTE_PRESENT | PTE_RW | PTE_USER;
  return P2V(entry);
}

struct vm_map* kernel_vm_map;

static volatile struct limine_kernel_address_request kaddr_request = {
  .id = LIMINE_KERNEL_ADDRESS_REQUEST,
  .revision = 0,
};

void
init_vm(uint64_t lol)
{
  struct limine_kernel_address_response* kaddr = kaddr_request.response;
  uint64_t* top_level;
  uintptr_t vaddr, paddr;
  kernel_vm_map = kalloc(sizeof(struct vm_map));

  kernel_vm_map->top_level = top_level = P2V(palloc(1));
  memset(top_level, 0, PGSIZE);

  uintptr_t datastart = ALIGN_DOWN((uintptr_t)data_start_addr, PGSIZE),
            textstart = ALIGN_DOWN((uintptr_t)text_start_addr, PGSIZE),
            rodatastart = ALIGN_DOWN((uintptr_t)rodata_start_addr, PGSIZE),
            dataend = ALIGN_UP((uintptr_t)data_end_addr, PGSIZE),
            textend = ALIGN_UP((uintptr_t)text_end_addr, PGSIZE),
            rodataend = ALIGN_UP((uintptr_t)rodata_end_addr, PGSIZE);

  /* map higher half */
  for (int i = 256; i < 512; ++i) {
    get_next_level(top_level, i, true);
  }

  /* map the text segment of the kernel */
  for (vaddr = textstart; vaddr < textend; vaddr += PGSIZE) {
    paddr = vaddr - kaddr->virtual_base + kaddr->physical_base;
    mappage2(top_level, vaddr, paddr, PTE_PRESENT | PTE_USER);
  }

  /* map the rodata segment of the kernel */
  for (vaddr = rodatastart; vaddr < rodataend; vaddr += PGSIZE) {
    paddr = vaddr - kaddr->virtual_base + kaddr->physical_base;
    mappage2(top_level, vaddr, paddr, PTE_PRESENT | PTE_NX | PTE_USER);
  }

  /* map the data segment of the kernel */
  for (vaddr = datastart; vaddr < dataend; vaddr += PGSIZE) {
    paddr = vaddr - kaddr->virtual_base + kaddr->physical_base;
    mappage2(top_level, vaddr, paddr, PTE_PRESENT | PTE_NX | PTE_RW | PTE_USER);
  }

  /* map lower half */
  for (vaddr = 0x1000; vaddr < 0x100000000 / 2; vaddr += PGSIZE) {
    mappage2(top_level, vaddr, vaddr, PTE_PRESENT | PTE_RW | PTE_USER);

    mappage2(top_level,
             vaddr + HHDM_OFFSET,
             vaddr,
             PTE_PRESENT | PTE_NX | PTE_RW | PTE_USER);
  }

  /* struct limine_memmap_response* memmap = memmap_request.response; */
  /* struct limine_memmap_entry* entry; */
  /* uintptr_t base, top; */

  /* for (int i = 0; i < memmap->entry_count; ++i) { */
  /* entry = memmap->entries[i]; */

  /* base = ALIGN_DOWN(entry->base, PGSIZE); */
  /* top = ALIGN_UP(entry->base + entry->length, PGSIZE); */
  /* if (top <= 0x100000000 / 2) { */
  /* continue; */
  /* } */

  /* for (int j = base; j < top; j += PGSIZE) { */
  /* if (j < 0x100000000 / 2) { */
  /* continue; */
  /* } */
  /* mappage2(top_level, j, j, PTE_PRESENT | PTE_RW); */
  /* } */
  /* } */
}

uint64_t*
va2pte(uint64_t*, uint64_t, bool);

/*
  Map size / PGSIZE amount of pages starting from vaddr at the physical address
  paddr
*/
int
mappages(uint64_t* top_level,
         uint64_t vaddr,
         uint64_t paddr,
         size_t size,
         uintptr_t flags)
{
  size_t i, pagecount = size / PGSIZE;

  for (i = 0; i < pagecount; ++i) {
    mappage2(top_level, vaddr + (i * PGSIZE), paddr + (i * PGSIZE), flags);
  }
  return 0;
}

extern int g;

int
mappage2(uint64_t* top_level, uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
  uint64_t* pte = va2pte(top_level, vaddr, true);

  // If page is already mapped
  if (*pte & PTE_PRESENT) {
    return -EINVAL;
  }
  *pte = paddr | flags;
  return 0;
}

void
destroy_level(uint64_t* pml, int start, int end, int level)
{
  // TODO: mapped pages should be free with munmap()
  if (level == 0) {
    freepg(V2P(pml), 1);
    return;
  }

  for (int i = start; i < end; ++i) {
    uint64_t* nextlevel = get_next_level(pml, i, false);
    if (nextlevel == NULL) {
      continue;
    }
    destroy_level(nextlevel, 0, 512, level - 1);
  }
  freepg(V2P(pml), 1);
}

void
destroyvm(struct vm_map* vmap)
{
  // end at 256 to not destroy kernel mappings
  destroy_level(vmap->top_level, 0, 256, 4);
}

void
switchvm(struct vm_map* vmap)
{
  asm("mov %0, %%cr3" ::"r"(V2P(vmap->top_level)));
}

struct vm_map*
newvm_map(void)
{
  struct vm_map* vmap = kalloc(sizeof *vmap);

  vmap->top_level = P2V(palloc(1));

  // copy kernel mappings
  for (int i = 256; i < 512; ++i) {
    vmap->top_level[i] = kernel_vm_map->top_level[i];
  }
  return vmap;
}

uint64_t*
va2pte(uint64_t* top_level, uint64_t vaddr, bool alloc)
{
  uint64_t *pml4 = top_level, *pml3, *pml2, *pml1;
  int idx;

  idx = (vaddr >> 39) & 0x1ff;
  pml3 = get_next_level(pml4, idx, alloc);

  idx = (vaddr >> 30) & 0x1ff;
  pml2 = get_next_level(pml3, idx, alloc);

  idx = (vaddr >> 21) & 0x1ff;
  pml1 = get_next_level(pml2, idx, alloc);

  idx = (vaddr >> 12) & 0x1ff;
  return &pml1[idx];
}
