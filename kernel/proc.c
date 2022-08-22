#include "franklin/switch.h"
#include "franklin/apic.h"



extern void switc(stack*, stack*, uint32_t*);
extern stack *c;
extern stack *contex;

void helo() {


  for (;;) {
    /* print("ee"); */
    switc(&c, contex, EOI);
  }
}
