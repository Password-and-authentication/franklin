#ifndef _GDT_
#define _GDT_

#include <stdint.h>

void
init_gdt(void);
uint16_t
init_tss(void);

extern struct gdtdesc* gdt;

struct gdtdesc
{
  uint16_t segment_limit;
  uint8_t base_addr[3];
  uint8_t attributes_1;
  uint8_t segment_limit2 : 4;
  uint8_t attributes_2 : 4;
  uint8_t base_addr2;
} __attribute__((packed));

struct tssdesc
{
  struct gdtdesc desc;
  uint32_t base_high;
} __attribute__((packed));

struct tss
{
  uint32_t zero;
  uint64_t rsp0;
} __attribute__((packed));

#endif
