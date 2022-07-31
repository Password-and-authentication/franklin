
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
void exception_handler(void);

void init_idt(void);


