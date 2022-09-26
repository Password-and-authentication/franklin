#ifndef _DIRENT_
#define _DIRENT_

#include "string.h"
#include "vfs.h"

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12

struct dirent {
  ino_t ino;
  int reclen;
  char type;
  char name[256];
};

// get size of dirent
static inline uint16_t get_reclen(struct dirent *d) {
  return ((char *)&d->name + strlen(d->name) + 1) - (char *)d;
}

#endif
