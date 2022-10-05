#include "franklin/proc.h"
#include "asm/x86.h"
#include "d.h"
#include "franklin/apic.h"
#include "franklin/fs/vfs.h"
#include "franklin/gdt.h"
#include "franklin/misc.h"
#include "franklin/mmu.h"
#include "franklin/switch.h"
#include "mm/vm.h"
#include "std/string.h"
#include <elf.h>
#include <stdint.h>

void
copyargs(uintptr_t* stack[], const char* argv[], struct auxval* aux);

extern void
ret(void);

#define STACKSIZE 0x40000

int
replace_proc(struct proc* proc, struct vm_map* vmap, void (*entry)());

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
  return (struct proc*)rdmsr(MSR_GS);
}

/* void */
/* load_elf(struct ) */
/* {} */

struct vm_map;
extern struct vm_map* kernel_vm_map;

void
loadelf(const char* name, struct vm_map* vmap, struct auxval* aux)
{
  Elf64_Ehdr elf;
  Elf64_Phdr phdr;
  struct vnode* vn;
  /* struct vm_map_entry* map_entry; */
  uint64_t flags, pagecount, paddr;
  size_t i;

  vfs_open(name, &vn, 0, 0);
  vfs_read(vn, &elf, 0, sizeof elf);
  ;

  for (i = 0; i < elf.e_phnum; ++i) {
    vfs_read(vn, &phdr, (off_t)(elf.e_phoff + (i * sizeof phdr)), sizeof phdr);

    switch (phdr.p_type) {
      case PT_LOAD: {
        /* map_entry = kalloc(sizeof *map_entry); */
        pagecount = DIV_ROUNDUP(phdr.p_memsz, PGSIZE);

        paddr = (uintptr_t)palloc(pagecount);

        /* map_entry->start = phdr.p_vaddr; */
        /* map_entry->end = phdr.p_vaddr + phdr.p_memsz; */

        flags = PTE_PRESENT | PTE_USER;
        if (phdr.p_flags & PF_W) {
          flags |= PTE_RW;
        }
        if ((phdr.p_flags & PF_X) == 0) {
          flags |= PTE_NX;
        }

        // map the segment
        mappages(
          vmap->top_level, phdr.p_vaddr, paddr, pagecount * PGSIZE, flags);

        // read it into memory
        vfs_read(vn, (void*)P2V(paddr), (off_t)phdr.p_offset, phdr.p_filesz);
        break;
      }
      case PT_PHDR: {
        aux->at_phdr = phdr.p_vaddr;
      };
    };
  }
  aux->at_entry = elf.e_entry;
  aux->at_phent = elf.e_phentsize;
  aux->at_phnum = elf.e_phnum;
  return 0;
}

void
exec(const char* name, const char* argv[])
{
  struct proc* p;
  struct vm_map *vmap, *oldvmap;
  struct auxval aux;
  uintptr_t *stack, *stacktop;
  uintptr_t stackva = 0x70000000000;

  p = get_current_proc();

  vmap = newvm_map();

  loadelf(name, vmap, &aux);

  // allocate and map the stack at 0x7000000000
  stack = palloc(STACKSIZE / PGSIZE);
  mappages(vmap->top_level,
           (uintptr_t)stackva,
           stack,
           STACKSIZE,
           PTE_PRESENT | PTE_RW | PTE_USER);

  p->regs->rsp = (uintptr_t)stackva + STACKSIZE;
  p->regs->rip = aux.at_entry;

  p->regs->cs = 0x43;

  // sdm: on far ret, if the target CPL == 3, ss cant be 0
  p->regs->ss = 0x3b;
  p->regs->eflags = 0x202;

  oldvmap = p->vmap;
  p->vmap = vmap;

  stack = (uintptr_t*)P2V((uintptr_t)stack + STACKSIZE);
  stacktop = stack;
  copyargs(&stack, argv, &aux);

  p->regs->rsp -= (uintptr_t)stacktop - (uintptr_t)stack;

  struct tss* tss = kalloc(sizeof *tss);

  switchvm(vmap);

  tss->rsp0 = P2V(palloc(1) + PGSIZE);
  load_tss(tss);

  /* destroyvm(oldvmap); */

  // TODO: exec is a system call, so this is not needed,
  // it should just return to the trap
  startproc(p);
}

/*
  Copy args to the stack by first copying the actual strings to the stack
  and then placing pointers to those strings on the stack
*/
void
copyargs(uintptr_t* stack[], const char* argv[], struct auxval* aux)
{
  size_t argc, length, i;
  uintptr_t oldrsp = (uintptr_t)*stack;

  for (argc = 0; argv[argc]; argc++) {
    length = strlen(argv[argc]);
    *stack = (uintptr_t*)((char*)*stack - length - 1);
    strcpy((char*)*stack, argv[argc]);
  }

  *--(*stack) = 0, *--(*stack);
  *stack -= 2, (*stack)[0] = AT_ENTRY, (*stack)[1] = aux->at_entry;
  *stack -= 2, (*stack)[0] = AT_PHDR, (*stack)[1] = aux->at_phdr;
  *stack -= 2, (*stack)[0] = AT_PHENT, (*stack)[1] = aux->at_phent;
  *stack -= 2, (*stack)[0] = AT_PHNUM, (*stack)[1] = aux->at_phnum;

  *--(*stack) = 0;
  *stack -= argc;
  for (i = 0; i < argc; ++i) {
    oldrsp -= (uintptr_t)strlen(argv[i]) + 1;
    (*stack)[i] = oldrsp;
  }
  *--(*stack) = argc;
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

  stack -= sizeof(struct regs);

  p->regs = (struct regs*)stack;
  p->regs->rip = (uintptr_t)entry;
  p->regs->cs = 0x28;
  p->regs->eflags = 0x202;

  stack -= sizeof(struct stack);
  p->stack = (struct stack*)stack;

  p->regs->rsp = stack - 0x200;
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
