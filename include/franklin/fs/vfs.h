#ifndef _VFS_
#define _VFS_

#include "../spinlock.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>



#define VROOT 0x1

#define MNT_ROOTFS 0x1

;struct vnode;

int vfs_close(struct vnode *);

struct nameidata {
  struct vnode *vdir;
  struct vnode *vn;

  char *name;
  size_t len;
};


struct componentname {
  char *nm;
  size_t len;
};


enum vtype {VNON, VREG, VDIR, VLNK, VSOCK, VBLK, VCHR};

struct vnodeops;
struct vfsops;
struct vfs;


struct vnode {
  struct vfs *vfs;
  struct vfs *mountedhere;
  struct vnodeops *ops;
  enum vtype type;
  int refcount;
  uint16_t flags;
  lock lock;
  void *data;
};


struct vnodeops {
  int (*open)();
  int (*read)();
  int (*write)();
  int (*remove)();
  int (*lookup)();
  int (*create)();
  int (*mkdir)();
  int (*rmdir)();
  int (*close)();
  int (*inactive)();
  int (*symlink)();
  int (*readlink)();
  int (*readdir)();
  int (*link)();
  
};


struct vfs {
  struct vfs *next;
  struct vnode *mountpoint;
  struct vfsops *ops;
  int flags;
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
