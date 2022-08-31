#ifndef _PROC_
#define _PROC_

struct proc;
struct proc *get_current_proc();
struct proc *set_current_proc(struct proc*);
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
};


#endif
