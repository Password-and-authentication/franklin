
#include <sys/types.h>


struct uio {
  off_t offset;
  ssize_t resid;
  void *buf;
};
