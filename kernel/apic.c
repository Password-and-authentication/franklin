#include "franklin/acpi.h"
#include "franklin/apic.h"
#include "franklin/69.h"
#include "franklin/io.h"
#include "franklin/defs.h"
#include "franklin/spinlock.h"
#include "franklin/interrupt.h"
#include "franklin/time.h"




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



void init_apic(uint32_t* lapic) {
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



static int ticks;
static int configured;
void init_timer(uint32_t* lapic) {
  
  if (configured) // 1st CPU will configure the timer
    goto startimer;
  
  ticks = configure_timer(lapic, 1);
  configured = 1;  
    
 startimer:
    print("start lapic timer\n");
    write32(lapic, TIMER_REG, 34 | 1 << 17); // vector 34 and periodic mode
    write32(lapic, DIVIDE_REG, 0);
    write32(lapic, INITCOUNT, ticks);
}


void apic_timer() {
  

  *EOI = 0;
}









