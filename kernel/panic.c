#include "defs.h"
#include "../69.h"

void panic(char *s) {
    print(s);

    asm("hlt");
};