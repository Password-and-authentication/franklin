#include <stdint.h>
#include "d.h"
#include "franklin/switch.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"
#include "franklin/proc.h"
#include "asm/x86.h"

#define NPROC 255

struct proc *curproc;
struct proc ptable[NPROC];


struct proc* set_current_proc(struct proc *p) {
  wrmsr(MSR_GS, (uint64_t)p);
  swapgs();
  return p;
}



struct proc* get_current_proc() {

  struct proc *p;
  
  swapgs();
  if ((p = (struct proc*)rdmsr(MSR_GS)) == 0) {
    swapgs();
    p = (struct proc*)rdmsr(MSR_GS);
  }
  
  return p;
}

void startproc(struct proc *p) {

  struct proc *current;
  
  current = get_current_proc();
  curproc = current;
  curproc->state = RUNNING;

  // this useless stack needs to be "saved" somewhere
  stack *discard;
  switc(&discard, p->stack);
}

void allocproc(void (*entry)()) {
  static uint32_t nextpid = 0;
  struct proc *p;
  
  p = &ptable[nextpid];
  p->pid = nextpid++;

  // palloc(1) allocates 1 page in phys memory and returns a physical addr
  p->stack = (stack*)P2V((uintptr_t)palloc(1));
  p->stack->rip = (uintptr_t)entry;
    
  p->state = RUNNABLE;
}


void scheduler() {

  struct proc *p, *prev;
  static uint8_t i = 1; // note: this is static

  // find next runnable process
  while ((p = &ptable[i++]) && p->state != RUNNABLE) {
    if (i == NPROC)
      i = 0;
  }
  
  prev = get_current_proc();
  curproc = set_current_proc(p);
  curproc->state = RUNNING;

  // save current stack on the previous process's struct
  // and switch to new process
  switc(&prev->stack, curproc->stack); 
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
 * - 'switc' will save the RIP and RBP of the curproc thread
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

