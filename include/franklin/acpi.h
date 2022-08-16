#ifndef _ACPI_
#define _ACPI_



#define MADT_C 0x43495041
#define HPET_CODE 0x54455048
#define MADT_CODE 0x43495041
#define FADT_CODE 0x50434146




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




typedef struct {
    defaultheader h;
    uint32_t FIRMWARE_CONTROL;
    uint32_t DSDT;
    char zero;
    char PMprofile;
    uint16_t SCI_INT;
    uint32_t SMI_CMD;
    char ACPI_ENABLE;
    char S4BIOS_REQ;
    char PSTATE_CNT;
    uint32_t PM1a_EVT_BLK;
    uint32_t PM1b_EVT_BLK;
    uint32_t PM1a_CNT_BLK;
    uint32_t PM1b_CNT_BLK;
    uint32_t PM2_CNT_BLK;
    uint32_t PM_TMR_BLK;
    uint32_t GPEO_BLK;
    uint32_t GPE1_BLK;
    char PM1_EVT_LEN;
    char PM1_CNT_LEN;
    char PM2_CNT_LEN;
    char PM_TMR_LEN;
    char GPE0_BLK_LEN;
    char GPE1_BLK_LEN;
    char GPE1_BASE;
    char CST_CNT;
    uint16_t P_LVL2_LAT;
    uint16_t P_LVL3_LAT;
    uint16_t FLUSH_SIZE;
    uint16_t FLUSH_STRIDE;
    char DUTY_OFFSET;
    char DUTY_WIDTH;
    char DAY_ALRM;
    char MON_ALRM;
    char CENTURY;
    uint16_t IAPC_BOOT_ARCH;



} __attribute__((packed)) FADT;


uint32_t *lapicc;
RSDT *rsdt;


void *get_acpi_sdt(unsigned long);
void init_acpi(void);
void acpi(unsigned int**, char*);


#endif

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
