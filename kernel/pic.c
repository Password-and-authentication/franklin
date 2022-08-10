#include "io.h"



void unmask_irq(char irq) {

    char imr = in(0x21);
    imr ^= (1 << 1);
    out(0x21, imr);
}



void pic_remap(int offset) {
    char mask = in(0x21);

    out(0x20, 0x10 | 0x01); // start initalization

    out(0x21, 0x20); // vector offset

    out(0x21, 0x01);

    out(0x21, mask);
}