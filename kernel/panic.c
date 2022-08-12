#include "defs.h"
#include "../69.h"
#include "spinlock.h"

void panic(char *s) {
    print(s);

    

    asm("hlt");
};