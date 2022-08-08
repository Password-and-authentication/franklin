#include <stdint.h>
#include "../ACPI//acpi.h"



void apic() {


    uint32_t *lapic;
    uint8_t NMI;
    acpi(&lapic, &NMI);
    


}


void walk_madt(MADT *madt, uint8_t *NMI) {
    
    uint8_t lapicId[100], x = 1;
    for (int i = 0; i < madt->h.length;) {
        if (madt->entry[i] == 0)
            lapicId[x++] = madt->entry[i + 3];
        if (madt->entry[i] == 4)
            *NMI = madt->entry[i + 5];
        i += madt->entry[i + 1];
    }
}