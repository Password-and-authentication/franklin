#include "limine.h"
#include "cpu.h"
#include "../69.h"
#include "../kernel/defs.h"
#include "idt.h"



volatile static struct limine_smp_request smp_req = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

void init_cpu() {
    struct limine_smp_response *smp = smp_req.response;
    
    // smp->cpus[1]->goto_address = cpu;
    volatile int x = 10;
    asm("mov $10, %eax");
    smp->cpus[1]->goto_address = cpu;


}




void cpu(struct limine_smp_info *info) {
    load_idt();

    for(;;);
        
}
