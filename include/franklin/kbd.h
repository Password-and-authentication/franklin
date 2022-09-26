#ifndef _KBD_
#define _KBD_

#include <stdint.h>

void
init_kbd(void);
uint8_t
check_ps2(void);

void
key_press(void);
void setconfb(uint8_t);
uint8_t
getconfb(void);
void
disdev(void);
void
testps2control(void);
void testps2port(uint8_t);
uint8_t
gettype(void);
uint8_t
getscancode(void);

#endif
