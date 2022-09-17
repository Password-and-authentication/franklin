#ifndef _VFS_
#define _VFS_

#include "../spinlock.h"

#include <stdint.h>
#include <stddef.h>
typedef uintptr_t ino_t;


struct vnode;
int vfs_close(struct vnode *);


struct componentname {
  char *nm;
  size_t len;
};


enum vtype {VNON, VREG, VDIR, VSYM};

struct vnodeops;
struct vfsops;
struct vfs;


struct vnode {
  struct vfs *vfs;
  struct vfs *mountedhere;
  struct vnodeops *ops;
  enum vtype type;
  int refcount;
  void *data;
};



struct vnodeops {
  int (*open)();
  int (*lookup)();
  int (*create)();
  int (*mkdir)();
  int (*close)();
  int (*inactive)();
};


struct vfs {
  struct vfs *next;
  struct vnode *mountpoint;
  struct vfsops *ops;
  void *data;
};

struct vfsops {
  const char *name;
  struct vfsops *next;
  int (*mount)();
  int (*unmount)();
  int (*root)();
  int (*vget)();
};

extern struct vfs *rootfs;
struct vfs *mountedlist;
struct vfs *head;

struct {
  struct vfsops first;
  lock lock;
} vfslist;



#endif
