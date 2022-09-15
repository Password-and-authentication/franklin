#include <stddef.h>

#include "franklin/fs/vfs.h"
#include "ramfs.h"



static const struct vnodeops ramfs_vnode_ops;

void *kalloc(int);

int
ramfs_mount(struct vfs *vfs)
{
  
  struct ramvfs *ram = kalloc(sizeof *ram);
  struct ramnode *root = kalloc(sizeof *root);
  struct ramdentry *droot = kalloc(sizeof *droot);

  droot->name = "/";
  droot->next = NULL;

  vfs->data = ram;

  root->type = VDIR;
  root->vnode = NULL;
  root->dir.parent = NULL;
  root->dir.dentry = droot;
  
  ram->root = root;
}


int
ramfs_root(struct vfs *vfs, struct vnode **vnode)
{
  struct ramvfs *rootvfs = vfs->data;
  if (rootvfs->root == NULL)
    return -1;
  
  
  vfs->ops->vget(vfs, vnode, rootvfs->root);
  return 0;
}


// get vnode corresponding to the ramnode 'ino'
int
ramfs_vget(struct vfs *vfs, struct vnode **vnode, ino_t ino) {

  struct ramnode *node = (struct ramnode*)ino;

  if (node->vnode) {
    node->vnode->refcount++;
    *vnode = node->vnode;
    return 0;
  } else {
    
    struct vnode *vn = kalloc(sizeof *vn);
    node->vnode = vn;
    
    vn->mountedhere = NULL;
    vn->ops = &ramfs_vnode_ops;
    vn->type = node->type;
    vn->refcount = 1;
    vn->data = node;
    vn->vfs = vfs;

    *vnode = vn;
  };
  return 0;
};


int
ramfs_lookup(struct vnode *vdir, struct vnode **vpp, const char *target)
{

  if (vdir->type != VDIR)
    return 0;

  struct ramnode *ram = vdir->data;
  struct ramdentry *entry;
  
  for (entry = ram->dir.dentry; entry; entry = entry->next) {
    if (strcmp(entry->name, target) == 0) {
      *vpp = entry;
      return 1;
    }
  };
};


static const struct vnodeops ramfs_vnode_ops = {
					 .lookup = ramfs_lookup,
};


const struct vfsops ram_ops = {
			 .name = "ramfs",
			 .mount = ramfs_mount,
			 .root = ramfs_root,
			 .vget = ramfs_vget,
			 /* .lookup = ramfs_lookup, */
};




