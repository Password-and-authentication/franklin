#ifndef _PIC_
#define _PIC_

#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1 0b000010000
#define ICW2 0b1

void unmask_irq(uint8_t);
void pic_remap(uint8_t);

#endif
