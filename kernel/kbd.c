#include "franklin/69.h"
#include "franklin/kbd.h"
#include "franklin/io.h"
#include "franklin/defs.h"
#include "franklin/acpi.h"

static uint8_t kbd_us[127] = {
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, '\t', 0, 0, 0, 0, 0, 0, 0, 'q',
		    '1', 0, 0, 0, 'z', 's', 'a', 'w', '2',
		    0, 0, 'c', 'x', 'd', 'e', '4', '3',
		    0, 0, ' ', 'v', 'f', 't', 'r', '5',
		    0, 0, 'n', 'b', 'h', 'g', 'y', '6',
		    0, 0, 0, 'm', 'j', 'u', '7', '8',
		    0, 0, ',', 'k', 'i', 'o', '0', '9',
		    0, 0, '.', '/', 'l', ';', 'p', '-',
		    0, 0, 0, 0, 0, '[', '=', 0, 0, 0,
		    0, '\n',
};


static uint8_t key_release = 0;

void kbd_press() {
    uint8_t keycode = in(0x60);
    uint8_t character[2] = {
		       kbd_us[keycode],
		       0
    };


    static uint8_t flags;
    void print_char(uint8_t*, uint8_t);
    if (keycode == 0xF0)
      key_release = 1;
    if (keycode == 0x12)
      flags ^= 2;
    if (!key_release) {
      if (keycode == 0x58)
	flags ^= 1;
      else if (keycode != 0x12)
	print_char(character, flags);
    }
    if (keycode != 0xF0 && key_release)
      key_release = 0;
    
    out(0x20, 0x20); // eoi
    return;
}


void print_char(uint8_t *ch, uint8_t flags) {

  // if caps lock
  if (flags & 1 || flags & 2)
    ch[0] -= 32;
  print(ch);
}



void init_kbd() {
    
    if (check_ps2() == 0)
        return;
    
    uint8_t buffer, config;

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

void setconfb(uint8_t config) {
    out(0x64, 0x60); 
    uint8_t buffer = in(0x64);
    out(0x60, config);
}

uint8_t getconfb() {
    out(0x64, 0x20);
    uint8_t buffer = in(0x64);
    return in(0x60);
}

uint8_t getscancode() {
    out(0x60, 0xF0); // command to get/set scan code set
    uint8_t buffer = in(0x60);
    out(0x60, 0); // get scan code set
    buffer = in(0x60); // 0xFA "ACK"
    return in(0x60);
}

uint8_t gettype() {
    out(0x60, 0xF5); // disable scanning of device
    uint8_t buffer = in(0x60);

    out(0x60, 0xF2); // identify command
    buffer = in(0x60);

    return in(0x60);
}

void testps2port(uint8_t port) {
    if (port == 1)
        out(0x64, 0xAB);
    else 
        out(0x64, 0xA9);
    uint8_t buffer = in(0x64);
    buffer = in(0x60);
    if (buffer != 0x0)
        panic("ERROR: testps2port");
}


void testps2control() {
    out(0x64, 0xAA);
    uint8_t buffer = in(0x64);
    buffer = in(0x60); // should be 0x55
    if (buffer != 0x55)
        panic("ERROR: testps2control");
}

void disdev() {
    out(0x64, 0xAD);
    out(0x64, 0xA7);
}

uint8_t check_ps2() {
    FADT *fadt = get_acpi_sdt(FADT_CODE);

    return (fadt->IAPC_BOOT_ARCH & (1 << 1));
}


