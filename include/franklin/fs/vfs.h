#ifndef _VFS_
#define _VFS_

enum vtype {VNON, VREG, VDIR, VSYM};

struct vnodeops;
struct vfsops;
struct vfs;

struct vnode {

  struct vfs *mountedhere;
  struct vnodeops *ops;
  enum vtype type;
  void *data;
};


struct vnodeops {
  int (*open)();
  int (*lookup)();
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
};

struct vfs *rootfs;
struct vfs *mountedlist;
struct vfs *head;
struct vfsops vfslist;


#endif
