#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "defs.h"
#include "idt.h"
#include "mmu.h"


static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

void print(void* s) {
    struct limine_terminal_response *terminal_res = terminal_request.response;
    struct limine_terminal *terminal = terminal_res->terminals[0];
    terminal_res->write(terminal, s, strlen(s));
}

extern uint64_t *bitmap;

 
void kmain(void) {
    char s[15];

    init_idt();
    struct limine_memmap_response *memmap = memmap_request.response;
    initbmap(memmap);
    int i = 0;

    char *l = malloc(1);
    char *p = malloc(1);
    char *x = malloc(1);
    char *d = malloc(1);


    __asm__ volatile("hlt");
}


