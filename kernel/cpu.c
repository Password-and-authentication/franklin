#include <stdint.h>

#include "d.h"
#include "franklin/acpi.h"
#include "franklin/apic.h"
#include "franklin/cpu.h"
#include "franklin/fs/vfs.h"
#include "franklin/idt.h"
#include "franklin/mmu.h"
#include "franklin/spinlock.h"
#include "limine.h"

#include "asm/x86.h"

static volatile struct limine_smp_request smp_req = {
  .id = LIMINE_SMP_REQUEST,
  .revision = 0,
};

void
init_cpu()
{
  struct limine_smp_response* smp = smp_req.response;

  smp->cpus[1]->goto_address = cpu;
}

void
load_gdt(void);
extern uint16_t
init_tss(void);

void
cpu(struct limine_smp_info* info)
{
  asm("cli");
  uint16_t tr;
  MADT* madt;

  load_idt();
  load_gdt();
  tr = init_tss();
  ltr(tr);

  madt = get_acpi_sdt(MADT_C);
  init_apic((uint32_t*)((uint64_t)HHDM_OFFSET + madt->lapic));

  uint8_t* l = palloc(100);
  freepg(P2V((uintptr_t)l), 100);

  /* ramfs_t(); */

  init_proc(1);
  asm("sti");
  for (;;)
    ;
}
