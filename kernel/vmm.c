#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {
    uint64_t addr;
    PML4E = palloc(1);
    addr = mappage(20 * PGSIZE, 20 * PGSIZE, KFLAGS);
    remappage(20 * PGSIZE, 21);
    addr = mappage(0x1000, 0, KFLAGS);

    unmappage(20 * PGSIZE);
    addr = mappage(20 * PGSIZE, 20 * PGSIZE, KFLAGS);
    addr = mappage(6900 * PGSIZE, 0, KFLAGS);
    unmappage(addr);
}   

uint64_t mappage(uint64_t vaddr, uint64_t paddr, uint8_t flags) {
    uint64_t *PDPTE, *PDE, *PTE;
    Table tablearr[4] = {{PML4E, 39}, {PDPTE, 30}, {PDE, 21}, {PTE, 12}};
    uint16_t index;
    uintptr_t addr;
    for (uint8_t i = 0; i < 4; ++i) {
        index = (vaddr >> (tablearr[i].shift)) & 0x1FF;
        if ((tablearr[i].table[index] & 1) == 0) {

            // if at the end of array allocate a page
            if (i == 3)
                newentry(&tablearr[i].table[index], paddr, flags);
            else
                tablearr[i + 1].table = newentry(&tablearr[i].table[index], 0, flags);
        } else {
            if (i == 3) {
                panic("panic: mappage, page already in use\n");
                break;
            }
            addr = (tablearr[i].table[index] >> 12);
            tablearr[i + 1].table = P2V(addr);
        }
    }
    return vaddr;
}

// get PTE and set present flag to 0 and free page from physical memory
void unmappage(uint64_t vaddr) {
    pte_t *pte = getpte(vaddr);
    uintptr_t addr = ((uintptr_t)*pte >> 12);
    *pte &= (0 << PRESENT);
    freepg((void*)P2V(addr), 1);
}

// 1. if PFN is already in use, error 
// 2. get current PTE and replace the address part of its bits
void remappage(uint64_t vaddr, int pfn) {
    if (!isfree(pfn))
        panic("panic: remappage, pfn: is not free\n");
    pte_t *pte = getpte(vaddr);
    uintptr_t addr = *pte >> 12;
    freepg((void*)P2V(addr), 1);
    *pte |= (pfn * PGSIZE) << 12;
}

void memset64(char* mem, int n) {
    for (int i = 0; i < n; ++i) {
        mem[i] = 0;
    }
}


pte_t *getpte(uint64_t vaddr) {
    uintptr_t addr;
    uint16_t index = vaddr >> 39;
    if ((PML4E[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PML4E not in use\n");
    addr = PML4E[index] >> 12;
    pdpte_t *PDPTE = P2V(addr);
    index = (vaddr >> 30) & 0x1FF;
    if ((PDPTE[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PDPTE not in use\n");
    addr = PDPTE[index] >> 12;
    pde_t *PDE = P2V(addr);
    index = (vaddr >> 21) & 0x1FF;
    if ((PDE[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PDE not in use\n");
    addr = PDE[index] >> 12;
    pte_t *PTE = P2V(addr);
    index = (vaddr >> 12) & 0x1FF;
    if ((PTE[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PTE not in use\n");
    return &PTE[index];
}

// set new entry in the table_entry table
// returns the new entry
uint64_t* newentry(uint64_t *table_entry, uint64_t paddr, uint8_t flags) {
    uint64_t *page;
    if (paddr != 0)
        page = pallocaddr(1, paddr);
    else
        page = palloc(1);
    paddr = V2P(page);
    *table_entry = ((uintptr_t)paddr << 12);
    *table_entry |= flags;
    memset64((char*)page, PGSIZE);
    return page;
}