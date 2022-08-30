#include <stdint.h>
#include "franklin/switch.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"
#include "franklin/proc.h"
#include "asm/x86.h"



struct proc *curproc;
struct proc ptable[20];
static int ptable_index;

static uint8_t swap = 1;
struct proc* set_current_proc(struct proc *p) {
  wrmsr((uint64_t)p);
  if (swap == 0)
    swap = 1;
  swapgs();
  return p;
}

struct proc* get_current_proc() {

  struct proc *p;
  if (swap)
    swapgs(); swap = 0;

  rdmsr(&p);
  
  return p;
}

void startproc(struct proc *p) {
  stack *l;
  struct proc *current;
  current = get_current_proc();
  curproc = current;
  curproc->state = RUNNING;
  switc(&current->stack, p->stack);
}

void allocproc(uintptr_t *entry) {

  struct proc *p;
  
  p = &ptable[ptable_index++];
  
  p->stack = (stack*)P2V((uintptr_t)palloc(1));
  p->stack->rip = (uintptr_t)entry;
  p->state = RUNNABLE;
}




void scheduler() {

  struct proc *p, *prev;
  static uint8_t i = 1;

  while ((p = &ptable[i++]) && p->state != RUNNABLE) {
    if (i == 20)
      i = 0;
  }
  
  prev = get_current_proc();
  curproc = set_current_proc(p);
  curproc->state = RUNNING;
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

