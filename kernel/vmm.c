#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {


    pml4e = palloc(1);
    mappage(0x1);
    mappage(0x1000);
}   


typedef struct {
    uint64_t *table;
    uint8_t shift;
} Table;

void mappage(uint64_t vaddr) {
    pdpte_t *PDPTE;
    pde_t *pgdir;
    pte_t *pagetable;
    Table ttablearr[4] = {{pml4e, 39}, {PDPTE, 30}, {pgdir, 21}, {pagetable, 12}};
    uint8_t shiftarr[4] = {39, 30, 21, 12};
    uint64_t *tablearr[4] = {pml4e, PDPTE, pgdir, pagetable};
    uint16_t index;
    uintptr_t addr;
    for (int i = 0; i < 4; ++i) {
        index = (vaddr >> (ttablearr[i].shift)) & 0x1FF;
        if ((ttablearr[i].table[i] & 1) == 0) {
            if (i == 3)
                newentry(&ttablearr[i].table[index]);
            else
                ttablearr[i + 1].table = newentry(&ttablearr[i].table[index]);
        } else {
            if (i == 3) {
                print("page already in use\n");
                break;
            }
            addr = (ttablearr[i].table[index] >> 12);
            ttablearr[i + 1].table = addr + HHDM_OFFSET;
        }
    }
}


uint64_t* newentry(uint64_t *table_entry) {
    uintptr_t paddr;
    uint64_t *page = palloc(1);
    paddr = (uintptr_t)page - HHDM_OFFSET;
    *table_entry = ((uintptr_t)paddr << 12);
    *table_entry |= 1;
    return page;
}








// void mappages(pde_t *pgdir, int n) {
//     pte_t *pagetable;
//     pde_t *page;
//     uintptr_t addr;
//     int i = 0;
//     while (1) {
//         page = &pgdir[i++];

//         if ((*page & 1) == 0) {
//             pagetable = newpde(page);
//             for (int i = 0; i < n; ++i) {
//                 newpte(&pagetable[i]);
//             }
//             return;
//         }

//         addr = (uintptr_t) *page >> 12;
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

// pte_t *newpde(pde_t *page) {
//     pte_t *pagedir = palloc(1);
//     uintptr_t addr = (uintptr_t)pagedir - HHDM_OFFSET;
//     *page = (addr << 12);
//     *page |= KFLAGS;
//     return pagedir;
// }