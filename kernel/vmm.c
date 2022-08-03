#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {
    PML4E = palloc(1);
    mappage(0x1);
    mappage(0x1000);
    unmappage(0x1);
    mappage(0x500);
}   

void mappage(uint64_t vaddr) {
    pdpte_t *PDPTE;
    pde_t *PDE;
    pte_t *PTE;
    Table tablearr[4] = {{PML4E, 39}, {PDPTE, 30}, {PDE, 21}, {PTE, 12}};
    uint16_t index;
    uintptr_t addr;
    for (uint8_t i = 0; i < 4; ++i) {
        index = (vaddr >> (tablearr[i].shift)) & 0x1FF;
        if ((tablearr[i].table[index] & 1) == 0) {
            if (i == 3)
                newentry(&tablearr[i].table[index]);
            else
                tablearr[i + 1].table = newentry(&tablearr[i].table[index]);
        } else {
            if (i == 3) {
                print("page already in use\n");
                break;
            }
            addr = (tablearr[i].table[index] >> 12);
            tablearr[i + 1].table = P2V(addr);
        }
    }
}


void unmappage(uint64_t vaddr) {
    pte_t *pte = getpte(vaddr);
    uintptr_t addr = ((uintptr_t)*pte >> 12);
    *pte &= (0 << PRESENT);
    freepg((void*)P2V(addr), 1);
}

pte_t *getpte(uint64_t vaddr) {
    uintptr_t addr;
    uint16_t index = vaddr >> 39;
    if ((PML4E[index] & PRESENT) == 0)
        print("ERROR: unmappage, getpte(), PML4E not in use\n");
    addr = PML4E[index] >> 12;
    pdpte_t *PDPTE = P2V(addr);
    index = (vaddr >> 30) & 0x1FF;
    if ((PDPTE[index] & PRESENT) == 0)
        print("ERROR: unmappage, getpte(), PDPTE not in use\n");
    addr = PDPTE[index] >> 12;
    pde_t *PDE = P2V(addr);
    index = (vaddr >> 21) & 0x1FF;
    if ((PDE[index] & PRESENT) == 0)
        print("ERROR: unmappage, getpte(), PDE not in use\n");
    addr = PDE[index] >> 12;
    pte_t *PTE = P2V(addr);
    index = (vaddr >> 12) & 0x1FF;
    if ((PTE[index] & PRESENT) == 0)
        print("ERROR: unmappage, getpte(), PTE not in use\n");
    return &PTE[index];
}


uint64_t* newentry(uint64_t *table_entry) {
    uintptr_t paddr;
    uint64_t *page = palloc(1);
    paddr = V2P(page);
    *table_entry = ((uintptr_t)paddr << 12);
    *table_entry |= 1;
    return page;
}