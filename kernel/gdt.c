#include <stdint.h>
#include "franklin/mmu.h"
#include "franklin/69.h"
#include "franklin/gdt.h"
#include "franklin/string.h"


static tss_desc_t init_tss(void);

__attribute__((aligned(0x8)))
static gdt_desc *gdt;

static unsigned short tr; // task register
static struct {
  unsigned short size;
  uint64_t addr;
} __attribute__((packed))gdtr;

void load_gdt() {
  asm("lgdt %0" :: "m"(gdtr));
  asm("ltr %0" :: "a"(tr));
}

void init_gdt() {
  gdt_desc userdata, usercode;
  
  gdt = (gdt_desc*)P2V((uint64_t)palloc(1));
  asm volatile("sgdt %0" : "=m"(gdtr));
  
  memcpy(gdt, (const void*)gdtr.addr, gdtr.size);
  
  int i = gdtr.size / sizeof(long); 
  
  /* flags = data segment, Privilege level 3, S bit and P bit set */
  userdata.attributes_1 = 2 | (3 << 5) | (1 << 7) | (1 << 4);
  userdata.attributes_2 = 0;
  gdt[++i] = userdata;

  /* flags = code segment, Privilege level 3, S bit and P bit set */
  usercode.attributes_1 = 8 | (3 << 5) | (1 << 7) | (1 << 4);
  usercode.attributes_2 = (1 << 1);
  gdt[++i] = usercode;

  gdtr.addr = (uint64_t)gdt;
  gdtr.size = PGSIZE;

  tss_desc_t *tss_p = (tss_desc_t*)&gdt[++i];
  *tss_p = init_tss();
  
  tr = i << 3;
  load_gdt();
}


static tss_desc_t init_tss() {

  uint64_t rsp0 = P2V((uint64_t)palloc(1));

  struct {
    uint32_t zero;
    uint64_t rsp0;
  } __attribute__((packed)) tss =  {
				    .rsp0 = rsp0,
  };



  tss_desc_t tss_desc =  {
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





