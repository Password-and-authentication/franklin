#include "limine.h"
#include "franklin/defs.h"
#include "franklin/cpu.h"
#include "franklin/69.h"
#include "franklin/idt.h"
#include "franklin/spinlock.h"
#include "franklin/apic.h"
#include "franklin/mmu.h"
#include "franklin/acpi.h"


volatile static struct limine_smp_request smp_req = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};


void init_cpu() {
    struct limine_smp_response *smp = smp_req.response;
    
    smp->cpus[1]->goto_address = cpu;
}


void load_gdt(void);
extern int init_tss();

void cpu(struct limine_smp_info *info) {

  uint16_t tr;
  MADT *madt;
  
  load_idt();
  load_gdt();
  tr = init_tss();
  ltr(tr);
  
  
  
  madt = get_acpi_sdt(MADT_C);
  init_apic((unsigned int*)((unsigned long)HHDM_OFFSET + madt->lapic));

  char *l = palloc(100);
  freepg(P2V((uintptr_t)l), 100);

  for(;;);
}
