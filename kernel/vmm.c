#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {


    pgdir = palloc(1);
    mappages(pgdir, 1);

}



void mappages(pde_t *pgdir, int n) {
    pte_t tblentry;
    pde_t *entry = &pgdir[0];
    if (*entry == 0) {
        pte_t *pagetable = pgdirentry(entry);
        char *s = palloc(n);
        uintptr_t addr = (uintptr_t) s - HHDM_OFFSET;
        pagetable[0] = (addr << 12);
        pagetable[0] |= KFLAGS;
    }

}


pte_t* pgdirentry(pde_t* entry) {

    pte_t *pagetable = palloc(1);
    uintptr_t addr = (uintptr_t) pagetable >> (64 - 20); 
    *entry = (addr << 12);
    *entry |= KFLAGS;
};