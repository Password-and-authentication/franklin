#include <stdint.h>
#include "d.h"
#include "franklin/switch.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"
#include "franklin/proc.h"
#include "asm/x86.h"

#define NPROC 255

struct proc *curproc;

struct {
  uint32_t lock;
  struct proc proc[NPROC];
} ptable;

void init_plock() {
  ptable.lock = 0;
}


void init_proc(int i) {
  set_current_proc(&ptable.proc[i]);
  startproc(&ptable.proc[i]);
}

void acq() {
  acquire(&ptable.lock);
}

void r() {
  release(&ptable.lock);
}


struct proc*
set_current_proc(struct proc *p)
{
  wrmsr(MSR_GS, (uint64_t)p);
  return p;
}


struct proc*
get_current_proc(void)
{
  return rdmsr(MSR_GS);
}


void
startproc(struct proc *p)
{

  struct proc *current;
  
  current = set_current_proc(p);
  current->state = RUNNING;

  acq();
  
  // this useless stack needs to be "saved" somewhere
  stack *discard;
  switc(&discard, p->stack);
}


void
allocproc(void (*entry)())
{
  static uint32_t nextpid = 0;
  struct proc *p;
  
  p = &ptable.proc[nextpid];
  p->pid = nextpid++;

  // palloc(1) allocates 1 page in phys memory and returns a physical addr
  p->stack = (stack*)P2V((uintptr_t)palloc(1));
  extern void ret(void);
  p->stack->rip = ret;
  p->regs = p->stack + sizeof(stack);
  p->regs->rip = entry;
  p->stack->rbp = p->regs;
    
  /* p->regs->rip = (uintptr_t)entry; */
    
  p->state = RUNNABLE;
}


void
scheduler()
{
  
  struct proc *p, *prev, *current;
  static uint8_t i = 2; // note: this is static

  // find next runnable process
  for (; p->state != RUNNABLE; p = &ptable.proc[i++]) {
    if (i == NPROC) {
      release(&ptable.lock);
      i = 0;
    }
    if (i == 0)
      acquire(&ptable.lock);
  }
    

  
  prev = get_current_proc();
  current = set_current_proc(p);
  current->state = RUNNING;

  // only switch if next thread is NOT the same as the previous one
  if (prev->pid != current->pid)
    switc(&prev->stack, current->stack);

  // if new thread is same as last one,
  // return from scheduler to trap to interrupt handler to thread
  return;
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

