#include <stdint.h>
#include "../ACPI/acpi.h"
#include "../69.h"
#include "kbd.h"
#include "io.h"






void init_kbd() {
    
    if (check_ps2() == 0)
        return;
    
    char buffer;

    out(0x64, 0xAD); // disable devices
    out(0x64, 0xA7);

    in(0x60); // flush output buffer

    out(0x64, 0x20);
    buffer = in(0x64);
    buffer = in(0x60);

    buffer ^= 0b1000011; // set the configure byte (clear bits 0, 1, 6)

    out(0x64, 0xAA);
    buffer = in(0x64);
    buffer = in(0x60);

    out(0x64, 0xAB); // test first port
    buffer = in(0x64);
    buffer = in(0x60);

    out(0x60, 0xF5); // disable scanning
    buffer = in(0x60);
    out(0x60, 0xF2); // identify command
    buffer = in(0x60);

    buffer = in(0x60); // get type

    out(0x60, 0xF4); //enable keyboard
    out(0x64, 0xAE);
}


int check_ps2() {
    FADT *fadt = get_acpi_sdt(FADT_CODE);

    return (fadt->IAPC_BOOT_ARCH & (1 << 1));
}


