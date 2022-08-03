#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {


    pml4e = palloc(1);
    mappage(0x1);
    mappage(0x1000);
}   



void mappage(uint64_t vaddr) {
    pdpte_t *PDPTE;
    pde_t *pgdir;
    pte_t *pagetable;
    uint8_t shiftarr[4] = {39, 30, 21, 12};
    uint64_t *tablearr[4] = {pml4e, PDPTE, pgdir, pagetable};
    uint16_t index;
    uintptr_t addr;
    for (int i = 0; i < 4; ++i) {
        index = (vaddr >> (shiftarr[i])) & 0x1FF;
        if ((tablearr[i][index] & 1) == 0) {
            if (i == 3)
                newentry(&tablearr[i][index]);
            else
                tablearr[i + 1] = newentry(&tablearr[i][index]);
        } else {
            if (i == 3) {
                print("page already in use\n");
                break;
            }
            addr = (tablearr[i][index] >> 12);
            tablearr[i + 1] = addr + HHDM_OFFSET;
        }
    }
    // if ((pml4e[index] & 1) == 0) {

    //     PDPTE = newentry(&pml4e[index]);

    //     index = vaddr >> 30;
    //     index &= 0x1FF;
    //     pgdir = newentry(&PDPTE[index]);

    //     index = vaddr >> 21;
    //     index &= 0x1FF;
    //     pagetable = newentry(&pgdir[index]);


    //     index = vaddr >> 12;
    //     index &= 0x1FF;
    //     page = (char*) newentry(&pagetable[index]);

    // } else {
        
    //     uintptr_t addr = (pml4e[index] >> 12);
    //     PDPTE = addr + HHDM_OFFSET;

    //     index = vaddr >> 30;
    //     index &= 0x1FF;
    //     if ((PDPTE[index] & 1) == 0) {
    //         pgdir = newentry(&PDPTE[index]);

    //         index = vaddr >> 21;
    //         index &= 0x1FF;

    //         pagetable = newentry(&pgdir[index]);

    //         index = vaddr >> 12;
    //         index &= 0x1FF;

    //         page = (char*) newentry(&pagetable[index]);
    //     } else {
    //         addr = (PDPTE[index] >> 12);
    //         pgdir = addr + HHDM_OFFSET;

    //         index = (vaddr >> 21) & 0x1FF;
    //         if ((pgdir[index] & 1) == 0) {
    //             pagetable = newentry(&pgdir[index]);

    //             index = (vaddr >> 12) & 0x1FF;
    //             page = (char*) newentry(&pagetable[index]);
    //         } else {
    //             addr = (pgdir[index] >> 12);
    //             pagetable = addr + HHDM_OFFSET;

    //             index = (vaddr >> 12) & 0x1FF;
    //             if ((pagetable[index] & 1) == 0) {
    //                 page = (char*) newentry(&pagetable[index]);
    //             } else print("Error page already in use\n");
    //         }
    //     }
    // }
}



void recursive(uint64_t *table, uintptr_t vaddr) {

    uint16_t index;
    if ((table[index] & 1) == 0) {

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