#ifndef _IO_
#define _IO_

#include <stdint.h>

void write32(uint64_t, uint32_t, uint32_t);
uint32_t* read32(uint64_t, uint32_t);

void
test2(void);

#endif
