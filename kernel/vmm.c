#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {


    pml4e = palloc(1);
    mappage(0x1);
}   



void mappage(uint64_t vaddr) {
    uint16_t index = vaddr >> 38;
    if ((pml4e[index] & 0) == 0) {
        pdpte_t *pgdirptrtable = newentry(&pml4e[index]);

        index = vaddr >> 30;
        index &= 0x1FF;
        pde_t *pgdir = newentry(&pgdirptrtable[index]);

        index = vaddr >> 20;
        index &= 0x1FF;
        pte_t *pagetable = newentry(&pgdir[index]);


        index = vaddr >> 12;
        index &= 0x1FF;
        char *page = (char*) newentry(&pagetable[index]);

    } else {
        




    }
}










uint64_t* newentry(uint64_t *table) {
    uintptr_t paddr;
    uint64_t *entry = palloc(1);
    paddr = (uintptr_t)entry - HHDM_OFFSET;
    *table = ((uintptr_t)entry << 12);
    *table |= 1;
    return entry;
}








// void mappages(pde_t *pgdir, int n) {
//     pte_t *pagetable;
//     pde_t *entry;
//     uintptr_t addr;
//     int i = 0;
//     while (1) {
//         entry = &pgdir[i++];

//         if ((*entry & 1) == 0) {
//             pagetable = newpde(entry);
//             for (int i = 0; i < n; ++i) {
//                 newpte(&pagetable[i]);
//             }
//             return;
//         }

//         addr = (uintptr_t) *entry >> 12;
//         pagetable = (uintptr_t) addr + HHDM_OFFSET;
//         int j = 0;
//         while ((*pagetable++ & 1)) 
//             ;

//         for (int i = 0; i < n; ++i) {
//             newpte(&pagetable[i]);
//         }
//         return;
//     }
// }



// void newpte(pte_t *pte) {
//     char *page = palloc(1);
//     uintptr_t addr = (uintptr_t)page - HHDM_OFFSET;
//     pte[0] = addr << 12;
//     pte[0] |= KFLAGS;
// }

// pte_t *newpde(pde_t *entry) {
//     pte_t *pagedir = palloc(1);
//     uintptr_t addr = (uintptr_t)pagedir - HHDM_OFFSET;
//     *entry = (addr << 12);
//     *entry |= KFLAGS;
//     return pagedir;
// }