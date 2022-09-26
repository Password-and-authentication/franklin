#ifndef _SWITCH_
#define _SWITCH_

#include <stdint.h>

struct regs
{
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
  uint64_t rsp;
  uint64_t ss;
} __attribute__((packed));

struct stack
{
  uint64_t rbp;
  uint64_t rbx;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rip;
} __attribute__((packed));

typedef struct
{
  uint64_t rsp;
} thread;

extern void
switc(struct stack**, struct stack*);

#endif
