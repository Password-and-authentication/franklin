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

  printl("hello", 5);

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

  test_slab();
  init_rootfs();

  vfs_mount(0, "ramfs");
  init_rootvn();

  /* vfs_mount("/lmaooooo/haha/fuck", "ramfs"); */

  MADT* madt = get_acpi_sdt(MADT_C);
  walk_madt(madt); // get info about MADT table
  init_gdt();

  /* print("\nwhat we gon' do tomorrow?\n"); */
  /* print("i got an idea.. , lets FUCK!\n\n"); */
  init_interrupt();

  /* vfs_mount("/", "ramfs"); */

  init_pit(1000); // 1000 hz, 1000 IRQ0's in a second
  asm("sti");

  /* sets LAPIC registers and starts the LAPIC timer (the first CPU will also
   * configure it) */
  /* init_apic((uint32_t*)((uintptr_t)madt->lapic + HHDM_OFFSET)); */
#define _shit_(x) x
#define __shit_(...)

  _shit_(("lol", "lol"));
  __shit_("lol", "lol");

  /* wrmsr(MSR_GS, 100); */

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

  uint64_t *pagetables, *new, *lol, *lol2, *lol3, *toplevel;
  extern struct vm_map* kernel_vm_map;
  static int l = 69;

  uint64_t* va2pte();

  // 0xfd000000
  // 0x300000
  asm volatile("mov %%cr3, %0" : "=r"(toplevel));

  toplevel = P2V(toplevel);
  lol = va2pte(toplevel, 0xfd000000, 0);

  init_vm();

  /* new = kernel_vm_map->top_level; */

  /* lol2 = va2pte(new, &lol, 0); */

  /* asm("mov %0, %%cr3" ::"r"(V2P(new))); */

  /* l = 0; */
  /* lol = 0; */

  /* exec("/exe"); */

  /* void init_proc(); */
  /* init_proc(0); */

  for (;;)
    asm("hlt");
}

void
thread3()
{
  asm("sti");
  r();
  static int h;

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

  r();

  for (;;) {
    /* print("1\n"); */
  }
}
