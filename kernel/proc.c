#include "franklin/proc.h"
#include "asm/x86.h"
#include "d.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"
#include "franklin/switch.h"
#include "mm/vm.h"
#include <elf.h>
#include <stdint.h>

struct vm_map*
newvm_map(void);

static struct proc* curproc;

static struct
{
  uint32_t lock;
  struct proc proc[NPROC];
} ptable;

void
init_plock()
{
  ptable.lock = 0;
}

void
init_proc(int i)
{
  set_current_proc(&ptable.proc[i]);
  startproc(&ptable.proc[i]);
}

void
acq()
{
  acquire(&ptable.lock);
}

void
r()
{
  release(&ptable.lock);
}

struct proc*
set_current_proc(struct proc* p)
{
  wrmsr(MSR_GS, (uint64_t)p);
  return p;
}

struct proc*
get_current_proc(void)
{
  return rdmsr(MSR_GS);
}

struct elf
{
  int magic;
};

extern struct vm_map* kernel_vm_map;

void
exec(const char* name)
{
  Elf64_Ehdr elf;
  Elf64_Phdr* phdr;
  struct proc* p;
  struct vm_map* vmap;
  struct vm_map_entry *map_entry, *map_entry_prev;
  struct vnode* vn;
  int error, count;

  vfs_open(name, &vn, 0, 0);

  vfs_read(vn, &elf, 0, sizeof elf);

  vmap = newvm_map();

  count = elf.e_phentsize * elf.e_phnum;
  phdr = kalloc(count);
  vfs_read(vn, phdr, elf.e_phoff, count);

  map_entry = kalloc(sizeof *map_entry);
  phdr++;

  // map the text segment
  uintptr_t* paddr = palloc(1);
  map_entry->start = phdr->p_vaddr;
  map_entry->end = phdr->p_vaddr + phdr->p_memsz;
  map_entry->offset = phdr->p_offset;

  mappage2(
    vmap->top_level, phdr->p_vaddr, paddr, PTE_PRESENT | PTE_RW | PTE_USER);
  // 4, 5, 4, 4, 4

  vfs_read(vn, P2V(paddr), phdr->p_offset, phdr->p_filesz);

  p = allocproc(elf.e_entry);

  p->vmap = vmap;

  switchvm(p->vmap);

  startproc(p);
}

void
startproc(struct proc* p)
{
  struct proc* current;
  acq();

  current = set_current_proc(p);
  current->state = RUNNING;

  // this useless stack needs to be "saved" somewhere
  struct stack* discard;
  switc(&discard, p->stack);
}

struct proc*
allocproc(void (*entry)())
{
  uint8_t *stack, index = 0;
  static uint32_t nextpid = 0;
  struct proc* p;

  acquire(&ptable.lock);
  p = getproc(UNUSED, &index);
  release(&ptable.lock);

  if (p == 0)
    return 0;

  p->pid = ++nextpid;

  uint64_t a;
  stack = (uint8_t*)P2V((uintptr_t)palloc(1));
  stack += PGSIZE;
  a = stack;
  extern void ret(void);

  stack -= sizeof(struct regs);

  p->regs = (struct regs*)stack;
  p->regs->rip = (uintptr_t)entry;
  p->regs->cs = 0x28;
  p->regs->eflags = 0x92;

  stack -= sizeof(struct stack);
  p->stack = (struct stack*)stack;

  p->stack->rip = (uintptr_t)ret;

  return p;
}

void
scheduler()
{

  struct proc *p, *prev, *current;
  static uint8_t i = 2; // note: this is static

  // find next runnable process
  for (;;) {
    if ((p = getproc(RUNNABLE, &i)))
      break;
    i = 0;
    // let other CPUs gain entry to scheduler
    /* release(&ptable.lock); */
    /* acquire(&ptable.lock); */
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

static struct proc*
getproc(enum procstate state, uint8_t* index)
{

  while (*index < NPROC) {
    if (ptable.proc[*index].state == state)
      return &ptable.proc[*index];
    (*index)++;
  }
  return 0;
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
