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

void ramfs_mount(struct vfs *vfs) {
  static struct ramvfs ram;
  static struct ramnode root;
  ram.root = &root; 
  
  vfs->data = &ram; //vfs private pointer to ram mount struct
}

static const struct vfsops ram_ops = {
			 .name = "ramfs",
			 .mount = ramfs_mount,
};

struct vfsops *vfslist = &ram_ops;



