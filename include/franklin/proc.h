



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
