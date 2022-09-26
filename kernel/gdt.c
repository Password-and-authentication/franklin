#include "franklin/gdt.h"
#include "asm/x86.h"
#include "d.h"
#include "franklin/mmu.h"
#include "std/string.h"
#include <stdint.h>

__attribute__((aligned(0x8))) static struct gdtdesc* gdt;

static uint8_t gdt_index;
static uint16_t tr; // task register
static struct
{
  uint16_t size;
  uint64_t addr;
} __attribute__((packed)) gdtr;

void
load_gdt()
{
  asm("lgdt %0" ::"m"(gdtr));
}

void
init_gdt()
{
  struct gdtdesc userdata, usercode;

  gdt = (struct gdtdesc*)P2V((uint64_t)palloc(1));
  asm volatile("sgdt %0" : "=m"(gdtr));

  memcpy(gdt, (const void*)gdtr.addr, gdtr.size);

  gdt_index = gdtr.size / sizeof(uint64_t);

  /* flags = data segment, Privilege level 3, S bit and P bit set */
  userdata.attributes_1 = 2 | (3 << 5) | (1 << 7) | (1 << 4);
  userdata.attributes_2 = 0;
  gdt[++gdt_index] = userdata;

  /* flags = code segment, Privilege level 3, S bit and P bit set */
  usercode.attributes_1 = 8 | (3 << 5) | (1 << 7) | (1 << 4);
  usercode.attributes_2 = (1 << 1);
  gdt[++gdt_index] = usercode;

  gdtr.addr = (uint64_t)gdt;
  gdtr.size = PGSIZE;

  load_gdt();
  tr = init_tss();
  ltr(tr);
}

static struct tssdesc
alloc_tss(void);

// return task register
uint16_t
init_tss()
{
  struct tssdesc* tss_p = (struct tssdesc*)&gdt[++gdt_index];
  *tss_p = alloc_tss();
  return gdt_index << 3;
}

static struct tssdesc
alloc_tss()
{

  uint64_t rsp0 = P2V((uint64_t)palloc(1));

  struct
  {
    uint32_t zero;
    uint64_t rsp0;
  } __attribute__((packed)) tss = {
    .rsp0 = rsp0,
  };

  struct tssdesc tss_desc = {
    .desc.segment_limit = 0x67,
    .desc.segment_limit2 = 0,
    .desc.base_addr[0] = (uint8_t)(uint64_t)&tss,
    .desc.base_addr[1] = (uint8_t)((uint64_t)&tss >> 8),
    .desc.base_addr[2] = (uint8_t)((uint64_t)&tss >> 16),
    .base_high = (uint32_t)((uint64_t)&tss >> 32),
    .desc.attributes_1 = 9 | 1 << 7, // 9 = TSS
    .desc.attributes_2 = 0,
  };

  return tss_desc;
}
