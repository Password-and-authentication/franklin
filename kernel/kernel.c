#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "defs.h"
#include "idt.h"


static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

void print(char *s) {
    struct limine_terminal *terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, s, strlen(s));
}

int x = 10;
 
void kmain(void) {
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        __asm__ volatile("hlt");
    }
    int y = 100;
    y = 100;
    y = 100;



    init_idt();


    // int yy = 10 / 0;
    
    print("test");

    // yy++;
    // int x = yy;
    // x++;


    // print("ttttt");


    __asm__ volatile("hlt");
}


