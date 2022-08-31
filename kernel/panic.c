#include "franklin/defs.h"
#include "d.h"
#include "franklin/69.h"
#include "franklin/spinlock.h"

void print(char*);


void panic(char *s) {
    print(s);

    asm("cli; hlt");
}
