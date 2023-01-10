#include <stddef.h>
#include <stdint.h>

#include "asm/x86.h"
#include "d.h"
#include "franklin/acpi.h"
#include "franklin/interrupt.h"
#include "std/string.h"

#include "franklin/apic.h"
#include "franklin/cpu.h"

#include "franklin/gdt.h"
#include "franklin/idt.h"
#include "franklin/interrupt.h"
#include "franklin/kbd.h"
#include "franklin/mmu.h"
#include "franklin/pic.h"
#include "franklin/proc.h"
#include "franklin/spinlock.h"

#include "franklin/switch.h"
#include "franklin/time.h"
#include "limine.h"

#include "mm/vm.h"

#include "franklin/fs/vfs.h"

static volatile struct limine_terminal_request terminal_request = {
  .id = LIMINE_TERMINAL_REQUEST,
  .revision = 0
};

void
print(void* s)
{
  struct limine_terminal_response* terminal_res = terminal_request.response;
  struct limine_terminal* terminal = terminal_res->terminals[0];
  acquire(&spinlock);
  terminal_res->write(terminal, s, strlen(s));
  release(&spinlock);
}

void
printl(void* s, size_t len)
{
  /* struct limine_terminal_response* terminal_res = terminal_request.response;
   */
  /* struct limine_terminal* terminal = terminal_res->terminals[0]; */
  acquire(&spinlock);
  /* terminal_res->write(terminal, s, len); */
  release(&spinlock);
}

void
kmain(void)
{
  asm("cli");

  initbmap();
  palloc(1);

  print("E");
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

  int* a[3];
  int x = sizeof(a) / sizeof(a[0]);

  print("U");
  test_slab();
  print("B");

  init_rootfs();

  vfs_mount(0, "ramfs");
  init_rootvn();


  MADT* madt = get_acpi_sdt(MADT_C);
  walk_madt(madt); // get info about MADT table
  init_gdt();

  init_interrupt();


  init_pit(1000); // 1000 hz, 1000 IRQ0's in a second
  asm("sti");

  /* sets LAPIC registers and starts the LAPIC timer (the first CPU will also
   * configure it) */
  /* init_apic((uint32_t*)((uintptr_t)madt->lapic + HHDM_OFFSET)); */
#define _shit_(x) x
#define __shit_(...)


  /* init_cpu(); // init 2nd CPU, (init_apic() gets called here aswell) */

  ramfs_t();
  initramfs();

  struct vnode* v;
  vfs_open("/main.c", &v, 0, 0);
  char buf[1024];
  if (v) {
    vfs_read(v, buf, 0, 100);
    printl(buf, 100);
    vfs_close(v);
  }

  init_vm();

  void init_proc();
  init_proc(0);
  /* asm("cli; hlt"); */

  for (;;)
    asm("hlt");
}

void
thread3()
{
  asm("sti");
  r();
  static int h;

  const char* s[] = { "sex", "lmao" };
  exec("/exe", s);

  for (;;) {
  }
}

void
thread2()
{
  asm("sti");
  r();
  static int x;

  for (;;) {
  }
}

void
thread1()
{

  // interrupts get disabled on trap entry
  asm("sti");
  exec("/exe");

  r();

  for (;;) {
    /* print("1\n"); */
  }
}
