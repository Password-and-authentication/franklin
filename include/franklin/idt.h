
#define IDT_MAX_DESC 256




typedef struct {
    uint16_t isr_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t isr_mid;
    uint32_t isr_high;
    uint32_t zero;
} __attribute__((packed)) idt_entry;


typedef struct {
    uint16_t size;
    uint64_t base;
} __attribute__((packed)) idtr_t;


__attribute__((noreturn))
void exception_handler(uint64_t);

void timerh(uint64_t);


extern void *isr_table[];
idtr_t idtr;
volatile int countdown;
int x;

void init_idt(void);
void load_idt(void);
void new_irq(char, void(*)(void));
void set_idt_entry(uint8_t, void(*)(void), uint8_t);
void isr_apic_timer(void);
void isr_kbd(void);

