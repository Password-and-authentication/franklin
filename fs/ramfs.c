#include "franklin/fs/vfs.h"
#include "franklin/fs/ramfs.h"

struct vnode;
struct vfs;
struct vfsops;



void *kalloc(int);

int
ramfs_mount(struct vfs *vfs)
{
  
  struct ramvfs *ram = kalloc(sizeof(struct ramvfs));
  struct ramnode *root = kalloc(sizeof(struct ramnode));

  vfs->data = (void*)ram;

  root->type = VDIR;
  ram->root = root;
}


int
ramfs_root(struct vfs *vfs, struct vnode **vnode)
{
  struct ramvfs *rootvfs = vfs->data;
  *vnode = rootvfs->root;
}


void
ramfs_lookup(struct vnode *vdir, struct vnode **vpp, const char *target)
{

  if (vdir->type != VDIR)
    return;

  struct ramnode *ram = vdir->data;
  struct ramdentry *entry;
  
  for (entry = ram->dir.dentry; entry; entry = entry->next) {
    if (strcmp(entry->name, target) == 0) {
      *vpp = entry;
      return;
    }
  };
};



static const struct vfsops ram_ops = {
			 .name = "ramfs",
			 .mount = ramfs_mount,
			 .root = ramfs_root,
			 /* .lookup = ramfs_lookup, */
};

struct vfsops vfslist = ram_ops;



