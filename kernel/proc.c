#include "franklin/switch.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"



extern void switc(stack*, stack*, uint32_t*);
void scheduler(void);
stack *c;
stack *contex;

void scheduler(void);



void init_scheduler() {
  c = palloc(1);
  c->rip = scheduler;
}


void scheduler() {


  for (;;) {
    print("ee");
    switc(&c, contex, EOI);
  }
}
