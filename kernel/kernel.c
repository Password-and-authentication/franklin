#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "defs.h"
#include "idt.h"


static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};
int x = 10;

void print(struct limine_terminal *terminal, char *s) {
    terminal_request.response->write(terminal, s, strlen(s));
}
 
void kmain(void) {
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        __asm__ volatile("hlt");
    }
    struct limine_terminal *terminal = terminal_request.response->terminals[0];



    init_idt();
    
    print(terminal, (x == 10) ? "10" : "100");

 
    __asm__ volatile("hlt");
}


