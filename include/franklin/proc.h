#ifndef _PROC_
#define _PROC_


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
};


#endif
