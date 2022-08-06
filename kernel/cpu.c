#include "limine.h"
#include "cpu.h"
#include "../69.h"




volatile static struct limine_smp_request smp_req = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

void init_cpu() {
    struct limine_smp_response *smp = smp_req.response;

    smp->cpus[0]->goto_address = cpu;

}




void cpu(struct limine_smp_info *info) {
    int x = 10;
    int y = 100;
    int p = y;
    asm("hlt");
}
