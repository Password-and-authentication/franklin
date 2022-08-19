#include "franklin/mmu.h"
#include "franklin/69.h"


static struct {
  unsigned short size;
  unsigned long addr;
} __attribute__((packed))gdtr;

typedef struct gdt_desc_struct{
  unsigned short segment_limit;
  unsigned char base_addr[3];
  unsigned char attributes_1;
  unsigned char segment_limit2: 4;
  unsigned char attributes_2: 4;
  unsigned char base_addr2;
} __attribute__((packed))gdt_desc;

__attribute__((aligned(0x8)))
static gdt_desc *gdt;

void new_descriptor(int, int);


void memcpy(void *dest, const void *src, int n) {

  char *d = dest;
  const char *s = src;
  while (n--) {
    *d++ = *s++;
  }
}

typedef struct {
  gdt_desc desc;
  unsigned int base_high;
} __attribute__((packed))tss_desc_t;


static unsigned int tss[3];
tss_desc_t init_tss(void);

void init_gdt() {
  gdt_desc userdata, usercode;
  gdt = (gdt_desc*)P2V(palloc(1));

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

  gdtr.addr = (unsigned long)gdt;
  gdtr.size = PGSIZE;

  tss_desc_t *tss_p = (tss_desc_t*)&gdt[++i];
  *tss_p = init_tss();

  unsigned short tr = i << 3;
  asm("lgdt %0" :: "m"(gdtr));
  asm("ltr %0" :: "a"(tr));
}



tss_desc_t init_tss() {
  tss_desc_t tss_desc;
  unsigned long rsp0 = P2V(palloc(1));
  tss[1] = rsp0 & 0xFFFFFFFF;
  tss[2] = rsp0 >> 32;
  
  tss_desc.desc.segment_limit = 0x67;
  tss_desc.desc.segment_limit2 = 0;
  tss_desc.desc.base_addr[0] = (char)tss;
  tss_desc.desc.base_addr[1] = (char)((unsigned long)tss >> 8);
  tss_desc.desc.base_addr[2] = (char)((unsigned long)tss >> 16);
  tss_desc.base_high = (unsigned int)((unsigned long)tss >> 32);

  tss_desc.desc.attributes_1 = 9 | 1 << 7;
  tss_desc.desc.attributes_2 = 0;
  return tss_desc;
}
