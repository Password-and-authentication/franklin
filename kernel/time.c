#include <stdint.h>
#include "asm/x86.h"
#include "franklin/time.h"
#include "franklin/apic.h"
#include "franklin/interrupt.h"
#include "franklin/spinlock.h"
#include "franklin/io.h"


void sleep(int us) {
  acquire(&spinlock);
  PIT_COUNTER = us;

  while (PIT_COUNTER > 0)
      ;

  release(&spinlock);
}


int configure_timer(unsigned int* lapic) {
  write32(lapic, DIVIDE_REG, 0); // divide by 2 (NOTE: dividor is only used so the counter have a smaller value)
  write32(lapic, INITCOUNT, ~0);

  sleep(1); // 1 ms

  write32(lapic, TIMER_REG, 1 << 16); // stop timer
  return ~0 - (*read32(lapic, CURRENTCOUNT));
}

void init_pit(int hz) {
  
  int divisor = 1193180 / hz;
  out(0x43, 0b110100);
  out(0x40, divisor & 0xFF);
  out(0x40, divisor >> 8);
}
