#include <stdint.h>
#include "franklin/acpi.h"
#include "../kernel/limine.h"


static volatile struct limine_rsdp_request rsdp_req = {
						       .id = LIMINE_RSDP_REQUEST,
						       .revision = 0,
};





void init_acpi() {
  struct limine_rsdp_response *rsdp_res = rsdp_req.response;
  RSDP *rsdp = (RSDP*) rsdp_res->address;
  rsdt = (RSDT*)((unsigned long)rsdp->rsdtaddr + HHDM_OFFSET);

  defaultheader *hdr;
  int x = 0;
  for (int i = 36; i < rsdt->h.length; i += 4)
    hdr = (defaultheader*)((uint64_t)rsdt->entry[x++] + HHDM_OFFSET);
}


void* get_acpi_sdt(uint64_t signature) {
  int i = 0;
  defaultheader *hdr;
  while (1) {
    hdr = (defaultheader*)((uint64_t)rsdt->entry[i++] + HHDM_OFFSET);
    if (hdr->signature == signature)
      return (void*)((uint64_t)rsdt->entry[--i] + HHDM_OFFSET);
  }
  return 0;
}
