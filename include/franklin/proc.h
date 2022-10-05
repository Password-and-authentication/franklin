#ifndef _PROC_
#define _PROC_

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
struct proc*
allocproc(void (*)());
void
scheduler(void);

struct auxval
{
  uint64_t at_entry;
  uint64_t at_phdr;
  uint64_t at_phent;
  uint64_t at_phnum;
};

enum procstate
{
  UNUSED,
  RUNNABLE,
  RUNNING,
};

struct proc
{
  struct regs* regs;
  struct stack* stack; // kernel stack
  enum procstate state;
  uint32_t pid;

  struct vm_map* vmap;
};

#endif
