#include "franklin/69.h"
#include "franklin/kbd.h"
#include "franklin/io.h"
#include "franklin/defs.h"
#include "franklin/acpi.h"

static char kbd_us[127] = {
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'q'
};


static int key_release = 0;
void kbd_press() {
    unsigned char keycode = in(0x60);
    /* print("\n"); */
    
    /* char s[20]; */
    /* itoa(keycode, s); */
    /* print(s); */
    /* print("\n"); */
    
    if (keycode != 0xF0 && !key_release && keycode < 127)
        print(&kbd_us[keycode]);
    if (key_release)
        key_release = 0;
    if (keycode == 0xF0)
        key_release = 1;

    out(0x20, 0x20);
    return;
}



void init_kbd() {
    
    if (check_ps2() == 0)
        return;
    
    char buffer, config;

    disdev();  // disable devices

    in(0x60); // flush output buffer

    config = getconfb();
    setconfb(config & 0b111100); // disable translation and both ports

    /* testps2control(); // commented out because apparently it can reset some settings */
    /* testps2port(1); */

    // for debugging
    buffer = gettype();
    buffer = getscancode();

    config = getconfb();
    setconfb(config | 0b11); // enable both ports

    out(0x60, 0xF4); //enable keyboard
    out(0x64, 0xAE);
}

void setconfb(char config) {
    out(0x64, 0x60); 
    char buffer = in(0x64);
    out(0x60, config);
}

char getconfb() {
    out(0x64, 0x20);
    char buffer = in(0x64);
    return in(0x60);
}

char getscancode() {
    out(0x60, 0xF0); // command to get/set scan code set
    char buffer = in(0x60);
    out(0x60, 0); // get scan code set
    buffer = in(0x60); // 0xFA "ACK"
    return in(0x60);
}

char gettype() {
    out(0x60, 0xF5); // disable scanning of device
    char buffer = in(0x60);

    out(0x60, 0xF2); // identify command
    buffer = in(0x60);

    return in(0x60);
}

void testps2port(int port) {
    if (port == 1)
        out(0x64, 0xAB);
    else 
        out(0x64, 0xA9);
    char buffer = in(0x64);
    buffer = in(0x60);
    if (buffer != 0x0)
        panic("ERROR: testps2port");
}


void testps2control() {
    out(0x64, 0xAA);
    char buffer = in(0x64);
    buffer = in(0x60); // should be 0x55
    if (buffer != 0x55)
        panic("ERROR: testps2control");
}

void disdev() {
    out(0x64, 0xAD);
    out(0x64, 0xA7);
}

int check_ps2() {
    FADT *fadt = get_acpi_sdt(FADT_CODE);

    return (fadt->IAPC_BOOT_ARCH & (1 << 1));
}


