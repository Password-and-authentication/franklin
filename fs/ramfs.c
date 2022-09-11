#include "franklin/fs/vfs.h"



struct vnode;
struct vfs;
struct vfsops;


struct ramnode {
  struct vnode *vnode;
};

struct ramvfs {
  struct ramnode *root;
};

struct ramdentry {
  char *name;
  struct vnode *vnode;
};

int ramfs_mount(struct vfs *vfs) {
  static struct ramvfs ram;
  static struct ramnode root;
  ram.root = &root; 
  
  vfs->data = &ram; //vfs private pointer to ram mount struct
}

int ramfs_root(struct vfs *vfs, struct vnode **vnode) {
  struct ramvfs *rootvfs = vfs->data;
  *vnode = 0x100;
}


static const struct vfsops ram_ops = {
			 .name = "ramfs",
			 .mount = ramfs_mount,
};

struct vfsops *vfslist = &ram_ops;



