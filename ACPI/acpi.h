

#define MADT_CODE 0x43495041
#define EOI_REG 0xB0
#define SPURIOUS_VECTOR 0xF0
#define TPR_REG 0x80
#define TIMER_REG 0x320
#define DIVIDE_REG 0x3E0
#define INITCOUNT 0x380
#define LINT0 0x350
#define LINT1 0x360


uint32_t *EOI;



typedef struct {
    uint64_t signature;
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdtaddr;
    uint32_t length;
    uint64_t xsdtaddr;
    uint8_t xchecksum;
    char zero[3];
} __attribute__((packed))RSDP;

typedef struct {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    uint64_t OEMTABLEid;
    uint32_t OEMrevision;
    uint32_t creatorId;
    uint32_t creatorrevision;
} __attribute__((packed))defaultheader;


typedef struct {
    defaultheader h;
    uint32_t entry[];
} __attribute__((packed))RSDT;

typedef struct {
    defaultheader h;
    uint32_t lapic;
    uint32_t flags;
    char entry[];
} __attribute__((packed))MADT;



void init_apic(uint32_t*);
void init_timer(uint32_t*);

// typedef struct {
//     uint32_t signature;
//     uint32_t length;
//     uint8_t revision;
//     uint8_t checksum;
//     char OEMID[6];
//     uint64_t OEMTABLEid;
//     uint32_t OEMrevision;
//     uint32_t creatorId;
//     uint32_t creatorrevision;
//     uint64_t *entry;
// } __attribute__((packed))XSDT;