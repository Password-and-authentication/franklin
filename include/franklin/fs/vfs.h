#ifndef _VFS_
#define _VFS_

#include "../spinlock.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define VROOT 0x1

#define MNT_ROOTFS 0x1

;
struct vnode;

int
vfs_close(struct vnode*);
;
struct vnode;

struct vnode* rootvn;

int
vfs_close(struct vnode*);

#define ISSYMLINK 0x1
#define FOLLOW 0x2

struct nameidata
{

  struct vnode* vdir;
  struct vnode* vn;
  struct vnode* root;
  char* next;

  int flags;

  struct componentnam
  {
    char* name;
    size_t len;
  } cn;
};

struct componentname
{
  char* nm;
  size_t len;
};

enum vtype
{
  VNON,
  VREG,
  VDIR,
  VLNK,
  VSOCK,
  VBLK,
  VCHR
};

struct vnodeops;
struct vfsops;
struct vfs;

struct vnode
{
  struct vfs* vfs;
  struct vfs* mountedhere;
  struct vnodeops* ops;
  enum vtype type;
  int refcount;
  uint16_t flags;
  lock lock;
  void* data;
};

/*
  Called when copying a vnode pointer
  (pointers are copied when the pointer needs to
   outlive where it came from)
*/
static inline vref(struct vnode* vn)
{
  acquire(&vn->lock);
  vn->refcount++;
  release(&vn->lock);
}

static inline vref_locked(struct vnode* vn)
{
  vn->refcount++;
}

static inline vput(struct vnode* vn)
{
  vfs_close(vn);
}

static inline vrele(struct vnode* vn)
{
  vput(vn);
}

struct vnodeops
{
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

struct vfs
{
  struct vfs* next;
  struct vnode* mountpoint;
  struct vfsops* ops;
  int flags;
  void* data;
};

struct vfsops
{
  const char* name;
  struct vfsops* next;
  int (*mount)();
  int (*unmount)();
  int (*root)();
  int (*vget)();
};

extern struct vfs* rootfs;
struct vfs* mountedlist;
struct vfs* head;

struct
{
  struct vfsops first;
  lock lock;
} vfslist;

#endif
