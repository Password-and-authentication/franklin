#include "franklin/io.h"
#include "franklin/69.h"



void out(char port, char buffer) {
    asm("out %%al, %%dx" :: "a"(buffer), "d"(port));
}

char in(char port) {
    char buffer;
    asm volatile("in %%dx, %%al" : "=a"(buffer) : "d"(port));
    return buffer;
}
