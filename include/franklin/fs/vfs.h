#ifndef _VFS_
#define _VFS_

#include <stdint.h>
#include <stddef.h>
typedef uintptr_t ino_t;



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

struct vfs *rootfs;
struct vfs *mountedlist;
struct vfs *head;
struct vfsops vfslist;


#endif
