#include "limine.h"
#include "franklin/69.h"
#include "franklin/mmu.h"



static struct {
  unsigned short size;
  unsigned long addr;
}__attribute__((packed)) gdtr;

static unsigned long *gdt;

void init_tss() {

  asm volatile("sgdt %0" : "=m"(gdtr) :: "memory");
  gdt = (unsigned long*)gdtr.addr;

}
