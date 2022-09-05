#ifndef _PROC_
#define _PROC_

#define NPROC 255

enum procstate;
struct proc;
struct proc *get_current_proc();
struct proc *set_current_proc(struct proc*);
static struct proc *getproc(enum procstate, uint8_t*);
void startproc(struct proc*);
void allocproc(void (*)());
void scheduler(void);

enum procstate {
		UNUSED,
		RUNNABLE,
		RUNNING,
};


struct proc {
  regs_t *regs;
  stack *stack;
  enum procstate state;
  uint32_t pid;
  uint64_t *pml4;
};


#endif
