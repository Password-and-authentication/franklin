#include <stdint.h>
#include <stddef.h>

#include "d.h"
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


#include "franklin/fs/vfs.h"






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

static uint64_t ar[3] = {3, 3, 4};

void kmain(void) {
  asm("cli");

  struct limine_memmap_response *memmap = memmap_request.response;
  initbmap(memmap);


  
  init_lock(&spinlock);
  init_vmm();
  init_acpi(); // set global variable RSDT

  void thread1();
  void thread2();
  void thread3();
  allocproc(thread1);
  allocproc(thread2);
  allocproc(thread3);
  init_plock();


  unsigned int x = 70;
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
	  
  test_slab();
  init_rootfs();

  
  vfs_mount(0, "ramfs");

  testt();

  /* vfs_mount("/newdir", "ramfs"); */

  
  
  MADT *madt = get_acpi_sdt(MADT_C);
  walk_madt(madt); // get info about MADT table
  init_gdt();

  print("\nwhat we gon' do tomorrow?\n");
  print("i got an idea.. , lets FUCK!\n\n");
  init_interrupt();

  /* vfs_mount("/", "ramfs"); */


  init_pit(1000); // 1000 hz, 1000 IRQ0's in a second
  asm("sti");


    /* sets LAPIC registers and starts the LAPIC timer (the first CPU will also configure it) */
  init_apic((uint32_t*)((uintptr_t)madt->lapic + HHDM_OFFSET));

  
  /* wrmsr(MSR_GS, 100); */

  /* init_cpu(); // init 2nd CPU, (init_apic() gets called here aswell) */


  ramfs_t();

 

  void init_proc();
  init_proc(0);


    
  for(;;)
    asm ("hlt");
}


void thread3() {
  asm("sti");
  r();
  static int h;


  for (;;) {

  }
}

void thread2() {
  asm("sti");
  r();
  static int x;

  

  for (;;) {

  }
}

void thread1() {
  
    // interrupts get disabled on trap entry
  asm("sti");
  r();

  for(;;) {
    /* print("1\n"); */
  }



}





