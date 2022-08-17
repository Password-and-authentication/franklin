#include "franklin/time.h"
#include "franklin/io.h"
#include "franklin/apic.h"
#include "franklin/interrupt.h"
#include "franklin/spinlock.h"


void sleep(int us) {
  acquire(&spinlock);
  PIT_COUNTER = us;

  while (PIT_COUNTER > 0)
      ;

  release(&spinlock);
}

static int ticks;

void configure_timer(unsigned int* lapic, int ms) {
  write32(lapic, DIVIDE_REG, 0); // divide by 2
  write32(lapic, INITCOUNT, ~0);

  sleep(ms); // sleep for 1 ms

  write32(lapic, TIMER_REG, 1 << 16); // stop timer
  ticks = ~0 - *read32(lapic, CURRENTCOUNT);
}

void init_pit(int hz) {
  
  int divisor = 1193180 / hz;
  out(0x43, 0b110100);
  out(0x40, divisor & 0xFF);
  out(0x40, divisor >> 8);
}
