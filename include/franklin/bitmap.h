#ifndef _BITMAP_
#define _BITMAP_

#include <stdbool.h>
#include <stddef.h>

static inline bool
bitmap_test(void* bitmap, size_t bit)
{
  char* bitmap8 = bitmap;
  return (bitmap8[bit / 8] & (1 << (bit % 8)));
}

static inline void
bitmap_set(void* bitmap, size_t bit)
{
  char* bitmap8 = bitmap;
  bitmap8[bit / 8] |= (1 << (bit % 8));
}

static inline void
bitmap_reset(void* bitmap, size_t bit)
{
  char* bitmap8 = bitmap;
  bitmap8[bit / 8] &= ~(1 << (bit % 8));
}

#endif
