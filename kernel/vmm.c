#include "mmu.h"
#include "limine.h"
#include "defs.h"




void init_vmm() {
    uint64_t addr;
    PML4E = palloc(1);
    mappage(0, 0, KFLAGS);
    unmappage(0);
    test();    
}   

int mappage(uint64_t vaddr, uint64_t paddr, uint8_t flags) {
    uint64_t *PDPTE, *PDE, *PTE;
    Table tablearr[4] = {{PML4E, 39}, {PDPTE, 30}, {PDE, 21}, {PTE, 12}};
    uint16_t index;
    uintptr_t addr;
    for (int i = 0; i < 3; ++i) {
        index = (vaddr >> (tablearr[i].shift)) & 0x1FF;
        tablearr[i + 1].table = newentry(&tablearr[i].table[index], 0, flags);
    }
    index = (vaddr >> (tablearr[3].shift)) & 0x1FF;
    newentry(&tablearr[3].table[index], 0, flags);


    return 1;
}

// get PTE and set present flag to 0 and free page from physical memory
void unmappage(uint64_t vaddr) {
    pte_t *pte = getpte(vaddr);
    __asm__ volatile("invlpg %0" : : "m" (*(char*)vaddr));
    uintptr_t addr = ((uintptr_t)*pte >> PAGE_SHIFT);
    *pte &= (0 << PRESENT);
    freepg((void*)P2V(addr), 1);
}

// 1. if PFN is already in use, error 
// 2. get current PTE and replace the address part of its bits
void remappage(uint64_t vaddr, int pfn) {
    if (!isfree(pfn))
        panic("panic: remappage, pfn: is not free\n");
    pte_t *pte = getpte(vaddr);
    uintptr_t addr = *pte >> PAGE_SHIFT;
    freepg((void*)P2V(addr), 1);
    *pte |= (pfn * PGSIZE) << 12;
}

void memzero(char* mem, int n) {
    for (int i = 0; i < n; ++i) {
        mem[i] = 0;
    }
}


pte_t *getpte(uint64_t vaddr) {
    uintptr_t addr;
    uint16_t index = vaddr >> 39;
    if ((PML4E[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PML4E not in use\n");
    addr = PML4E[index] >> PAGE_SHIFT;
    pdpte_t *PDPTE = P2V(addr);
    index = (vaddr >> 30) & 0x1FF;
    if ((PDPTE[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PDPTE not in use\n");
    addr = PDPTE[index] >> PAGE_SHIFT;
    pde_t *PDE = P2V(addr);
    index = (vaddr >> 21) & 0x1FF;
    if ((PDE[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PDE not in use\n");
    addr = PDE[index] >> PAGE_SHIFT;
    pte_t *PTE = P2V(addr);
    index = (vaddr >> 12) & 0x1FF;
    if ((PTE[index] & PRESENT) == 0)
        panic("ERROR: getpte(), PTE not in use\n");
    return &PTE[index];
}

// set new entry in the table_entry table
// returns the new entry
uint64_t* newentry(uint64_t *table_entry, uint64_t paddr, uint8_t flags) {
    if (*table_entry & PRESENT)
        goto noalloc;    
        
    uint64_t *page;
    if (paddr != 0)
        page = pallocaddr(1, paddr);
    else
        page = palloc(1);
    paddr = V2P(page);
    *table_entry = ((uintptr_t)paddr << PAGE_SHIFT);
    *table_entry |= flags;
    memzero((char*)page, PGSIZE);
    noalloc:
    paddr = P2V((*table_entry >> PAGE_SHIFT));
    return (uint64_t*) paddr;
}



void test() {
    mappage(0, 0, KFLAGS);
    unmappage(0);
    mappage(10 * PGSIZE, 0, KFLAGS);
    unmappage(10 * PGSIZE);
    for (int i = 0; i < 10; ++i) {
        mappage(i * PGSIZE, 0, KFLAGS);
    }
    for (int i = 0; i < 10; ++i) {
        unmappage(i * PGSIZE);
    }

    for (int i = 1000; i < 1010; ++i) {
        mappage(i * PGSIZE, 0, KFLAGS);
        unmappage(i * PGSIZE);
    }
        // mappage(0x2000, 0x1000, KFLAGS);
    // mappage(0x3000, PGSIZE * PGSIZE, KFLAGS);
    // mappage(3000 * PGSIZE, 1000 * PGSIZE, KFLAGS);
    // mappage(5000* PGSIZE, 6000 * PGSIZE, KFLAGS);
    // unmappage(1000 * PGSIZE);
    // unmappage(5000 * PGSIZE);
    // unmappage(3000 * PGSIZE);
    // unmappage(0x3000);
    for (int i = 1000; i < (1000 + 10); ++i) {
        mappage(i * PGSIZE, 0, KFLAGS);
    }

    for (int i = 1000; i < (10 + 1000); ++i) {
        unmappage(i * PGSIZE);
    }

    // for (int i = 0; i < 1000; ++i) {
    //     mappage(i * PGSIZE, 0, KFLAGS);
    // }

    // for (int i = 5000; i < 10000; ++i) {
    //     mappage(i * PGSIZE, 0, KFLAGS);
    // }
    // for (int i = 5000; i < 10000; ++i) {
    //     unmappage(i * PGSIZE);
    // }
    // mappage(10000* PGSIZE, 0, KFLAGS);
    // mappage(19990 * PGSIZE, 0, KFLAGS);

}