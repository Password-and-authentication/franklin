#ifndef _ACPI_
#define _ACPI_



#define MADT_C 0x43495041
#define HPET_CODE 0x54455048
#define MADT_CODE 0x43495041
#define FADT_CODE 0x50434146




typedef struct {
    unsigned long signature;
    unsigned char checksum;
    unsigned char OEMID[6];
    unsigned char revision;
    unsigned int rsdtaddr;
    unsigned int length;
    unsigned long xsdtaddr;
    unsigned char xchecksum;
    unsigned char zero[3];
} __attribute__((packed))RSDP;

typedef struct {
    unsigned int signature;
    unsigned int length;
    unsigned char revision;
    unsigned char checksum;
    unsigned char OEMID[6];
    unsigned long OEMTABLEid;
    unsigned int OEMrevision;
    unsigned int creatorId;
    unsigned int creatorrevision;
} __attribute__((packed))defaultheader;


typedef struct {
    defaultheader h;
    unsigned int entry[];
} __attribute__((packed))RSDT;

typedef struct {
    defaultheader h;
    unsigned int lapic;
    unsigned int flags;
    unsigned char entry[];
} __attribute__((packed))MADT;




typedef struct {
    defaultheader h;
    unsigned int FIRMWARE_CONTROL;
    unsigned int DSDT;
    unsigned char zero;
    unsigned char PMprofile;
    unsigned short SCI_INT;
    unsigned int SMI_CMD;
    unsigned char ACPI_ENABLE;
    unsigned char S4BIOS_REQ;
    unsigned char PSTATE_CNT;
    unsigned int PM1a_EVT_BLK;
    unsigned int PM1b_EVT_BLK;
    unsigned int PM1a_CNT_BLK;
    unsigned int PM1b_CNT_BLK;
    unsigned int PM2_CNT_BLK;
    unsigned int PM_TMR_BLK;
    unsigned int GPEO_BLK;
    unsigned int GPE1_BLK;
    unsigned char PM1_EVT_LEN;
    unsigned char PM1_CNT_LEN;
    unsigned char PM2_CNT_LEN;
    unsigned char PM_TMR_LEN;
    unsigned char GPE0_BLK_LEN;
    unsigned char GPE1_BLK_LEN;
    unsigned char GPE1_BASE;
    unsigned char CST_CNT;
    unsigned short P_LVL2_LAT;
    unsigned short P_LVL3_LAT;
    unsigned short FLUSH_SIZE;
    unsigned short FLUSH_STRIDE;
    unsigned char DUTY_OFFSET;
    unsigned char DUTY_WIDTH;
    unsigned char DAY_ALRM;
    unsigned char MON_ALRM;
    unsigned char CENTURY;
    unsigned short IAPC_BOOT_ARCH;



} __attribute__((packed)) FADT;


unsigned int *lapicc;
RSDT *rsdt;


void *get_acpi_sdt(unsigned long);
void init_acpi(void);
void acpi(unsigned int**, unsigned char*);


#endif

// typedef struct {
//     unsigned int signature;
//     unsigned int length;
//     unsigned char revision;
//     unsigned char checksum;
//     unsigned char OEMID[6];
//     unsigned long OEMTABLEid;
//     unsigned int OEMrevision;
//     unsigned int creatorId;
//     unsigned int creatorrevision;
//     unsigned long *entry;
// } __attribute__((packed))XSDT;
