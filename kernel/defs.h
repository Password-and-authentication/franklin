
typedef unsigned int uint_t;
#define NULL 0

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
// string.c

int strlen(char *);
int itoa(int, char *);

// print

void print(void*);


__attribute__((noreturn))
void panic(char*);