#include <stdint.h>
#include "franklin/mmu.h"
#include "franklin/69.h"
#include "franklin/gdt.h"
#include "franklin/string.h"


static tss_desc_t alloc_tss(void);




__attribute__((aligned(0x8)))
static gdt_desc *gdt;

static uint8_t gdt_index;
static uint16_t tr; // task register
struct {
  uint16_t size;
  uint64_t addr;
} __attribute__((packed))gdtr;

uint16_t t;
void load_gdt() {
  asm("lgdt %0" :: "m"(gdtr));
  /* asm volatile("str %0" : "=m"(t)); */
  /* asm("ltr %0" :: "a"(tr)); */
}

void init_gdt() {
  gdt_desc userdata, usercode;
  
  gdt = (gdt_desc*)P2V((uint64_t)palloc(1));
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

void ltr(uint16_t tr) {
  asm("ltr %0"::"m"(tr));
}

// return task register
uint16_t init_tss() {
  tss_desc_t *tss_p = (tss_desc_t*)&gdt[++gdt_index];
  *tss_p = alloc_tss();
  return gdt_index << 3;
}


static tss_desc_t alloc_tss() {

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





