#include "franklin/defs.h"
#include "franklin/69.h"
#include "franklin/spinlock.h"


void panic(char *s) {
    print(s);

    asm("cli; hlt");
};
