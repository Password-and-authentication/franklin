#ifndef _CPU_
#define _CPU_

void
init_cpu(void);

int
checkx2(void);

struct limine_smp_info;

void
cpu(struct limine_smp_info*);

#endif
