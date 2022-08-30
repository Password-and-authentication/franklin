#ifndef _ASM_
#define _ASM_


#define asm __asm__



static void out(char port, char buffer) {
    asm("out %%al, %%dx" :: "a"(buffer), "d"(port));
}

static char in(char port) {
    char buffer;
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

static void rdmsr(uint64_t *val) {
  uint32_t edx, eax;
  asm volatile(
      "mov $0xc0000102, %%rcx;"
      "rdmsr;"
      : "=a"(eax), "=d"(edx)
	       );
  *val = ((uint64_t)edx << 32) | eax;

}

static void wrmsr(uint64_t val) {
  asm(
      "mov %0, %%edx;"
      "mov %1, %%eax;"
      "mov $0xc0000102, %%rcx;"
      "wrmsr;"
      ::
       "r"((uint32_t)(val >> 32)),
       "r"((uint32_t)val)
      : "rdx", "rax", "rcx"
      );
}

static void ltr(uint16_t tr) {
  asm("ltr %0" :: "m"(tr));
}



#endif

