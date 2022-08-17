#include "franklin/mmu.h"


static unsigned int tss[3]; // only using RSP0

static struct {
  unsigned short segment_limit;
  unsigned short base_1;
  unsigned char base_2;
  unsigned short attributes_1;
  unsigned int segment_limit_2: 4;
  unsigned int attributes_2: 4;
  unsigned char base_3;
  unsigned int base_4;
} TSSdesc;


void init_tss() {

  tss[1] = palloc(1); // allocate a page for the RSP0
  unsigned int size = sizeof(unsigned int) * 3;
  /* TSSdesc.segment_limit = (unsigned short)size; */
  /* TSSdesc.segment_limit_2 = (size >> 16) & 0b111;  */
  
  /* TSSdesc.base_1 = (unsigned short)tss; */
  /* TSSdesc.base_2 = (unsigned char)(tss >> 16); */
  /* TSSdesc.base_3 = (unsigned char)(tss >> 24); */
  /* TSSdesc.base_4 = (unsigned int)(tss >> 32); */
}
