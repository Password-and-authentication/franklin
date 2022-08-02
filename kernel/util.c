#include <stdint.h>
#include "defs.h"
#include "limine.h"

static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
};


uintptr_t gethhdm() {
    struct limine_hhdm_response *hhdm_res = hhdm_req.response;
    return hhdm_res->offset;
} 
