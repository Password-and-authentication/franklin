#include "franklin/switch.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"



extern void switc(stack*, stack*, uint32_t*);
void scheduler(void);
stack *c;
stack *contex;

void scheduler(void);


void allocproc() {
  
}

void init_scheduler() {
  c = palloc(1);
  contex = palloc(1);
  c->rip = scheduler;
}

void scheduler() {


  for (;;) {
    /* print("ee"); */
    switc(&c, contex, EOI);
  }
}


/*
 * - isr_stub pushes the trap number on the stack,
 * - and jmps to 'alltraps'
 * - 
 * -'alltraps' saves all registers on the kernel stack, and aligns the stack
 * - and it will call trap, with the RSP in RDI
 *
 * - 'trap' calls 'apic_timer' which will call 'switc'
 * 
 * - 'switc' will save the RIP and RBP of the current thread
 * - then it will store the RSP in the thread's stack
 * - then it replaces RSP with the new thread's stack pointer
 * 
 * - if the thread is new, the new stack will only contain the RIP,
 * - which points to the entry point of the thread
 * 
 * - if the thread has run previously, the stack will contain:
 * - the RIP (last instruction), trapframe for 'trap' and 'alltraps'
 * - and all the saved registers
 * -
 */

