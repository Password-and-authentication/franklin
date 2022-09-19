#include <stdint.h>
#include <stddef.h>
#include "d.h"



void *kalloc(size_t);

size_t
strlen(char *s)
{
    size_t n = 0;
    while (++n && s[n]);
    return n;
}

int
strcmp(char *s, char *s2)
{

  size_t len, i;
  if ((len = strlen(s)) != strlen(s2))
    return -1;
  
  for (i = 0; i < len; ++i)
    if (s[i] != s2[i])
      return -1;
  return 0;
};

int
strncmp(char *s, char *s2, size_t n)
{

  for (size_t i = 0; i < n; ++i)
    if (s[i] != s2[i])
      return -1;
  
  return 0;
}

char *
strchr(char *s, int c) {

  do {
    if (*s == c)
      return s;
  } while (*s++);
  
  return s - 1;
}


int
strcpy(char *s, char *s2) {

  size_t j, i = strlen(s2);
  for (j = 0; j < i; ++j)
    s[j] = s2[j];
  s[j] = 0;
}

char *
strdup(const char *s) {

  size_t i, len = strlen(s);
  char *ss = kalloc(len + 1); // account null byte
  
  for (i = 0; i < len; ++i)
    ss[i] = s[i];
  ss[i] = 0;

  return ss;
}



int memcpy(void *dest, const void *src, size_t n) {

  char *d = dest;
  const char *s = src;
  while (n--) {
    *d++ = *s++;
  }
}

void memzero(uint8_t* mem, size_t n) {
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
