

#define DIV_ROUNDUP(n, d) (((n) + (d)-1) / (d))

#define ALIGN_UP(n, d) (DIV_ROUNDUP(n, d) * d)
