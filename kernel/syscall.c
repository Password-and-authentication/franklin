#include "franklin/switch.h"

/* void */
/* syscall(struct regs*); */

extern int
exec();

static int (*syscalls[])(void) = {
  exec,
};

void
syscall(struct regs* regs)
{
  int ret, num = regs->rax;

  // BUG: p->regs and regs are not the same
  struct proc* p = get_current_proc();

  ret = syscalls[num]();
}
