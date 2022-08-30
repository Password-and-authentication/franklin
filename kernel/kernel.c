#include <stdint.h>
#include "limine.h"
#include "franklin/defs.h"
#include "franklin/mmu.h"
#include "franklin/cpu.h"
#include "franklin/idt.h"
#include "franklin/spinlock.h"
#include "franklin/apic.h"
#include "franklin/kbd.h"
#include "franklin/pic.h"
#include "franklin/acpi.h"
#include "franklin/interrupt.h"
#include "franklin/string.h"
#include "franklin/gdt.h"
#include "franklin/switch.h"
#include "franklin/time.h"
#include "franklin/proc.h"
#include "asm/x86.h"





static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};


void print(void* s) {
    struct limine_terminal_response *terminal_res = terminal_request.response;
    struct limine_terminal *terminal = terminal_res->terminals[0];
    acquire(&spinlock);
    terminal_res->write(terminal, s, strlen(s));
    release(&spinlock);
}



void kmain(void) {

  asm("cli");

  struct limine_memmap_response *memmap = memmap_request.response;
  initbmap(memmap);


  
  init_lock(&spinlock);
  init_vmm();
  init_acpi(); // set global variable RSDT

  void testt();
  void another();
  void geex();
  allocproc(testt);
  allocproc(another);
  allocproc(geex);

  
  MADT *madt = get_acpi_sdt(MADT_C);
  walk_madt(madt); // get info about MADT table
  init_gdt();

  print("ii got an idea, lets FUCK!\n");
  init_interrupt();


  init_pit(1000); // 1000 hz, 1000 IRQ0's in a second
  asm("sti");

  /* asm("int $0x21"); */



    /* sets LAPIC registers and starts the LAPIC timer (the first CPU will also configure it) */
  init_apic((unsigned int*)((unsigned long)madt->lapic + HHDM_OFFSET));
    
  /* init_cpu(); // init 2nd CPU, (init_apic() gets called here aswell) */

  
  void startproc(struct proc*);
  extern struct proc ptable[];
  extern struct proc *curproc;
  curproc = &ptable[0];
  asm("mov $10, %rax");

  

  void wrmsr(uint64_t);
  wrmsr((uint64_t)curproc);

  uint64_t lol;
  rdmsr(&lol);
  asm("swapgs");
  asm("swapgs");
  asm("mov $0xc0000102, %rcx");
  asm("rdmsr");
  
  
  
  
  startproc(&ptable[0]);


    
  for(;;)
    asm ("hlt");
}

void ll() {

  asm("mov $0xc0000102, %rcx");
  asm("wrmsr");
}
void geex() {
  asm("sti");

  for (;;) {

  }
}

void another() {
  asm("sti");

  for (;;) {
    /* print("lol\n"); */

  }
}

void testt() {

  // interrupts get disabled on trap entry
  asm("sti");

  for(;;) {
    /* print("ex\n"); */
  }



}





