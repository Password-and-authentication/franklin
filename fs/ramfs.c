#include <stddef.h>

#include "franklin/fs/vfs.h"
#include "ramfs.h"



static const struct vnodeops ramfs_vnode_ops;

void *kalloc(int);

static struct ramnode* ramfs_alloc_node(struct ramvfs*, struct ramnode*, enum vtype);


/* mount ramfs
 - doesn't create a dentry for the root
 - initialize root ramnode and root mount struct
 
 */
int
ramfs_mount(struct vfs *vfs)
{
  struct ramvfs *ram = kalloc(sizeof *ram);
  struct ramnode *root;

  // init root ramnode
  root = ramfs_alloc_node(ram, NULL, VDIR);

  vfs->data = ram;
}

void testt() {
  
  struct ramvfs *ram = rootfs->data;
  struct ramnode *root, *c;
  struct vnode *vn, *v;
  root = ramfs_alloc_node(ram, NULL, VDIR);
  ram->root = root;

  struct ramdentry *de, *dc;
  ramfs_alloc_dentry(root, &de, "..");
  
  c = ramfs_alloc_node(ram, root, VREG);
  ramfs_alloc_dentry(c, &dc, "lmao");

  ramfs_dir_attach(de, dc);
  
  rootfs->ops->vget(rootfs, &vn, (ino_t)root);
  rootfs->ops->vget(rootfs, &v, (ino_t)c);

  ramfs_lookup(vn, &v, "lmao");
};


/* 
   create new ramnode

   - if type is VDIR,
     dir points at the parent (unless root)

   - adds the node to the list of all nodes
     in the filesystem (ram)

   - (DOESN'T ALLOC A DENTRY)
 */
static struct ramnode*
ramfs_alloc_node(struct ramvfs *ram, struct ramnode *dir, enum vtype type)
{
  
  struct ramnode *node = kalloc(sizeof *node);

  // add node to the list of all
  // ramnodes in the *ram filesystem
  node->nextnode = ram->nodes;
  ram->nodes = node;
  
  node->type = type;
  node->vnode = NULL;

  switch (node->type) {
  case VDIR:
    node->dir.parent = (dir == NULL) ? node : dir;
    break;
  case VREG:
    break;
  }

  return node;
}

/*
  allocates a dentry for a ramnode
*/
int
ramfs_alloc_dentry(struct ramnode *node, struct ramdentry **de, const char *name)
{
  
  struct ramdentry *dentry = kalloc(sizeof *dentry);

  dentry->name = strdup(name);
  dentry->node = node;
  dentry->next = NULL;

  if (node->type == VDIR)
    node->dir.dentry = dentry;
  
  *de = dentry;
  return 0;
}

/*
  add dentry to the parent
*/
void
ramfs_dir_attach(struct ramdentry *parent, struct ramdentry *dentry)
{
  dentry->next = parent->next;
  parent->next = dentry;
}

/* tmpfs create plan
   
   tmpfs(vp, nm, va, e, m, vpp, c)

   vp = dir vnode
   nm = new file name (dentry)
   va = vattr
   vpp = results

   alloc new vnode, ramnode and dentry
 */

int
ramfs_create(struct vnode *vp, struct vnode **vpp, const char *nm, enum vtype type)
{

  if (vp->type != VDIR)
    return 0;

  struct ramnode *node;
  node = ramfs_alloc_node(vp->vfs, vp->data, type);

  vp->vfs->ops->vget(vp->vfs, vpp, (ino_t)node);
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


/*
  ramfs_lookup

  - lookup the vnode corresponding
    to target in the directory vdir
*/
int
ramfs_lookup(struct vnode *vdir, struct vnode **vpp, const char *target)
{

  if (vdir->type != VDIR)
    return 0;

  struct ramnode *ram = vdir->data;
  struct ramdentry *entry;
  
  for (entry = ram->dir.dentry; entry; entry = entry->next) {
    if (strcmp(entry->name, target) == 0) {
      vdir->vfs->ops->vget(vdir->vfs , vpp, entry->node);
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




