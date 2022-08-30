#ifndef _GDT_
#define _GDT_

#include <stdint.h>

void init_gdt(void);
uint16_t init_tss(void);


typedef struct gdt_desc_struct{
  uint16_t segment_limit;
  uint8_t base_addr[3];
  uint8_t attributes_1;
  uint8_t segment_limit2: 4;
  uint8_t attributes_2: 4;
  uint8_t base_addr2;
} __attribute__((packed))gdt_desc;


typedef struct {
  gdt_desc desc;
  uint32_t base_high;
} __attribute__((packed))tss_desc_t;



#endif
