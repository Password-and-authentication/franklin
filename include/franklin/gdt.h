

void init_gdt(void);


typedef struct gdt_desc_struct{
  unsigned short segment_limit;
  unsigned char base_addr[3];
  unsigned char attributes_1;
  unsigned char segment_limit2: 4;
  unsigned char attributes_2: 4;
  unsigned char base_addr2;
} __attribute__((packed))gdt_desc;


typedef struct {
  gdt_desc desc;
  unsigned int base_high;
} __attribute__((packed))tss_desc_t;



