#include "franklin/acpi.h"
#include "franklin/apic.h"
#include "franklin/69.h"
#include "franklin/io.h"
#include "franklin/defs.h"
#include "franklin/spinlock.h"
#include "franklin/interrupt.h"
#include "franklin/time.h"
#include "franklin/switch.h"





static void init_timer(unsigned int*);

// right now its only getting the LINT pin that is connected to NMI
void walk_madt(MADT *madt) {

    char lapicId[100], x = 1;
    for (int i = 0; i < madt->h.length;) {
        if (madt->entry[i] == 0)
            lapicId[x++] = madt->entry[i + 3];
        if (madt->entry[i] == 4)
            NMI_LINT = madt->entry[i + 5];
        i += madt->entry[i + 1];
    }
}



void init_apic(unsigned int* lapic) {
    // set the correct LINT pin for NMI
    if (NMI_LINT == 1) {
      write32(lapic, LINT1, 1 << 10);
    } else {
      write32(lapic, LINT0, 1 << 10);
    };
    EOI = read32(lapic, EOI_REG);
    write32(lapic, TPR_REG, 0);
    write32(lapic, SPURIOUS_VECTOR, 0x1FF);

    init_timer(lapic);
}



static void init_timer(unsigned int* lapic) {
  static int ticks;
  static int configured;
  
  if (configured) // 1st CPU will configure the timer
    goto startimer;
  
  ticks = 100 * configure_timer(lapic); // return ticks in 1ms, multiply by 100 to get 100ms
  configured = 1;  
    
 startimer:
  print("start lapic timer\n");
  write32(lapic, TIMER_REG, 34 | 1 << 17); // vector 34 and periodic mode
  write32(lapic, DIVIDE_REG, 0);
  write32(lapic, INITCOUNT, ticks);
}



void scheduler(void);
void helo(void);

stack cc = {
	     .rip = helo,
};
stack *c;

void ff() {
  c = &cc;
}

extern void switc(stack*, stack*, uint32_t*);


/*
 * - isr_stub pushes the trap number on the stack,
 * - and jmps to 'alltraps'
 * - 
 * -'alltraps' saves all registers on the kernel stack, and aligns the stack
 * - and it will call trap, with the RSP in RDI
 *
 * - 'trap' calls 'apic_timer' which will call 'switc'
 * 
 * - 'switc' will save the RIP and RBP of the current thread
 * - then it will store the RSP in the thread's stack
 * - then it replaces RSP with the new thread's stack pointer
 * 
 * - if the thread is new, the new stack will only contain the RIP,
 * - which points to the entry point of the thread
 * 
 * - if the thread has run previously, the stack will contain:
 * - the RIP (last instruction), trapframe for 'trap' and 'alltraps'
 * - and all the saved registers
 * -
 */


static stack *contex;
void helo() {


  for (;;) {
    print("ee");
    switc(&c, contex, EOI);
  }
}

void apic_timer(regs_t *regs) {

  switc(&contex, c, EOI);

  
}









