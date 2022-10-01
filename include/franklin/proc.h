#ifndef _PROC_
#define _PROC_

#include "../../kernel/vm/vm.h"
#include "mmu.h"
#include "switch.h"
#include <stddef.h>
#include <stdint.h>

#define NPROC 255

enum procstate;
struct proc;
struct regs;
struct stack;

struct proc*
get_current_proc();
struct proc*
set_current_proc(struct proc*);
static struct proc*
getproc(enum procstate, uint8_t*);
void
startproc(struct proc*);
void
allocproc(void (*)());
void
scheduler(void);

enum procstate
{
  UNUSED,
  RUNNABLE,
  RUNNING,
};

struct proc
{
  struct regs* regs;
  struct stack* stack;
  enum procstate state;
  uint32_t pid;

  struct vm_map* vmap;
  uint64_t* pagetables;
};

#endif
