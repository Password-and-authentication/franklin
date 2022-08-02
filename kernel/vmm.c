#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {


    pgdir = palloc(1);
    mappages(pgdir, 1);
    mappages(pgdir, 1);
}



void mappages(pde_t *pgdir, int n) {
    pte_t *pagetable;
    pde_t *entry;
    uintptr_t addr;
    int i = 0;
    while (1) {
        entry = &pgdir[i++];

        if ((*entry & 1) == 0) {
            pagetable = newpde(entry);
            for (int i = 0; i < n; ++i) {
                newpte(&pagetable[i]);
            }
            return;
        }

        addr = (uintptr_t) *entry >> 12;
        pagetable = (uintptr_t) addr + HHDM_OFFSET;
        int j = 0;
        while ((*pagetable++ & 1)) 
            ;

        for (int i = 0; i < n; ++i) {
            newpte(&pagetable[i]);
        }
        return;
    }
}



void newpte(pte_t *pte) {
    char *page = palloc(1);
    uintptr_t addr = (uintptr_t)page - HHDM_OFFSET;
    pte[0] = addr << 12;
    pte[0] |= KFLAGS;
}

pte_t *newpde(pde_t *entry) {
    pte_t *pagedir = palloc(1);
    uintptr_t addr = (uintptr_t)pagedir - HHDM_OFFSET;
    *entry = (addr << 12);
    *entry |= KFLAGS;
    return pagedir;
}