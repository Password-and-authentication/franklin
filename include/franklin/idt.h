
#define IDT_MAX_DESC 256




typedef struct {
    unsigned short isr_low;
    unsigned short selector;
    unsigned char ist;
    unsigned char attributes;
    unsigned short isr_mid;
    unsigned int isr_high;
    unsigned int zero;
} __attribute__((packed)) idt_entry;


typedef struct {
    unsigned short size;
    unsigned long base;
} __attribute__((packed)) idtr_t;


__attribute__((noreturn))
void exception_handler(unsigned long);

void timerh(unsigned long);


extern void *isr_table[];
idtr_t idtr;


void init_idt(void);
void load_idt(void);
void new_irq(unsigned char, void(*)(void));
void set_idt_entry(unsigned char, void(*)(void), unsigned char);
void isr_apic_timer(void);
void isr_kbd(void);

