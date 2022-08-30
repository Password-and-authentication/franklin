#include <stdint.h>


int strlen(char *s) {
    int n = 0;
    while (++n && s[n]);
    return n;
}

void memcpy(void *dest, const void *src, int n) {

  char *d = dest;
  const char *s = src;
  while (n--) {
    *d++ = *s++;
  }
}

void memzero(uint8_t* mem, int n) {
    for (int i = 0; i < n; ++i) {
        mem[i] = 0;
    }
}



void itoa(int x, char* s) {
    int r, i = 0, arr[10];

    do {
        r = x % 10;
        arr[i] = r;
    } while (++i && (x /= 10));
    r = 0;
    while (i--) {
        s[r++] = arr[i] + 48;
    }
}
