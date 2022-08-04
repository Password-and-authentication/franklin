#include "defs.h"


void panic(char *s) {
    print(s);

    __asm__ volatile("hlt");
};