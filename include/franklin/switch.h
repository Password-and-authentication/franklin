#ifndef _SWITCH_
#define _SWITCH_

#include <stdint.h>

typedef struct {
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rax;
  uint64_t rbp;
  uint64_t code;
  /* uint64_t errcode; */
  uint64_t rip;
  uint64_t cs;
  uint64_t eflags;
} __attribute__((packed))regs_t;

typedef struct {
  unsigned long rbp;
  unsigned long rip;
} __attribute__((packed))stack;


typedef struct {
  uint64_t rsp;
} thread;




#endif
