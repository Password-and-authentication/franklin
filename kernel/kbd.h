


void init_kbd(void);
int check_ps2(void);

char kbd_us[];
void key_press(void);
void setconfb(char);
char getconfb(void);
void disdev(void);
void testps2control(void);
void testps2port(int);
char gettype(void);
char getscancode(void);