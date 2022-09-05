#ifndef _ASM_
#define _ASM_

#include "d.h"

#define asm __asm__



static void out(uint8_t port, uint8_t buffer) {
    asm("out %%al, %%dx" :: "a"(buffer), "d"(port));
}

static uint8_t in(uint8_t port) {
    uint8_t buffer;
    asm volatile("in %%dx, %%al" : "=a"(buffer) : "d"(port));
    return buffer;
}

/* static void lgdt(void *gdtr) { */
  /* asm("lgdt %0" :: "m"(*gdtr)); */
/* } */

/* struct idtr; */
/* static void lidt(struct idtr *idtr) { */
  /* asm("lidt %0":: "m"(*idtr)); */
/* } */





static void swapgs() {
  asm("swapgs":::"memory");
}

#define MSR_GS 0xc0000102

static uint64_t rdmsr(uint32_t msr) {

  uint32_t a, d;
  asm("rdmsr" : "=a"(a), "=d"(d) :"c"(msr));
  return (uint64_t)d << 32 | a;
}


static void wrmsr(uint32_t msr, uint64_t val) {

  asm("wrmsr" :: "a"((uint32_t)val), "d"(val >> 32), "c"(msr));
}

static void ltr(uint16_t tr) {
  asm("ltr %0" :: "m"(tr));
}



#endif

