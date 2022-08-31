#include "franklin/io.h"
#include "d.h"
#include "franklin/69.h"




void write32(unsigned long base, unsigned int reg, unsigned int val) {
  *(volatile unsigned int*)(base + reg) = val;
}

unsigned int* read32(unsigned long base, unsigned int reg) {
  return (unsigned int*)(base + reg);
}


