#include "io.h"



void unmask_irq(char irq) {

    char imr = in(0x21);
    imr ^= (1 << 1);
    out(0x21, imr);
}


#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1 0b000010000
#define ICW2 0b1


void pic_remap(int offset) {
    char mask = in(PIC1_DATA); // mask gets cleared on initalization
    char mask2 = in(PIC2_DATA);

    out(PIC1_COMMAND, ICW1 | 0x01);     // initalization
    out(PIC2_COMMAND, ICW1 | 0x01);


    out(PIC1_DATA, offset);     // offset in idt
    out(PIC2_DATA, offset);

    out(PIC1_DATA, 0b100);  // slave at IRQ2
    out(PIC2_DATA, 0b10);

    out(PIC1_DATA, 0x01); // set mode to 8086
    out(PIC2_DATA, 0x01);

    out(PIC1_DATA, mask);
    out(PIC2_DATA, mask2);
}