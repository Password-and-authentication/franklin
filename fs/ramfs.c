#include <stddef.h>

#include "franklin/fs/vfs.h"
#include "ramfs.h"



static const struct vnodeops ramfs_vnode_ops;

void *kalloc(int);

static int ramfs_create(struct vnode*, struct vnode**, const char*, enum vtype);


void ramfs_t() {

  struct vnode *vn, *v, *vv, *new, *w1, *w2, *w3, *w4;
  rootfs->ops->root(rootfs, &vn);
  ramfs_create(vn, &v, "lmao", VDIR);
  ramfs_create(v, &vv, "main.c", VREG);
  ramfs_create(vn, &new, "newdir", VDIR);
    
  ramfs_create(new, &w1, "main.c", VREG);
  ramfs_create(new, &w2, "main.h", VREG);
  ramfs_create(new, &w3, "main.o", VREG);
  ramfs_create(new, &w4, "makemake", VREG);
  ramfs_create(new, &w4, "print", VDIR);


  ramfs_create(vn, &w1, "nice", VDIR);
  ramfs_create(w1, &w2, "nicee", VDIR);
  ramfs_create(w2, &w4, "main.c", VREG);
  ramfs_create(w2, &w4, "kernel.o", VREG);
  ramfs_create(w2, &w4, "GNUmakefile", VREG);
  /* ramfs_create(w2, &w4, "kernel.c", VREG); */
  /* ramfs_create(w2, &w1, "nice", VDIR); */

  vfs_mount("/nice", "ramfs");
  vfs_mount("/nice/nicee", "ramfs");
}

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
  ram->root = root;

  vfs->data = ram;
}

int ramfs_alloc_file(struct vnode *vdir, struct vnode **vpp, const char *name, enum vtype type);

void testt() {


  /* ramfs_alloc_file(); */
};


/*
 Create a file in vdir directory
 with name nm
*/
static int
ramfs_create(struct vnode *vdir, struct vnode **vpp, const char *name, enum vtype type)
{
  ramfs_alloc_file(vdir, vpp, name, type);
};


static int
ramfs_mkdir(struct vnode *vdir, struct vnode *vpp, const char *name, enum vtype type)
{
  if (type != VDIR)
    panic("panic: ramfs_mkdir type");

  ramfs_alloc_file(vdir, vpp, name, type);
}





/*
 Allocate a new file of type type
 and add it to the parent directory dir
*/
int
ramfs_alloc_file(struct vnode *vdir, struct vnode **vpp, const char *name, enum vtype type)
{
  
  if (vdir->type != VDIR)
    panic("panic: ramfs_ alloc_file");
  
  struct ramvfs *ram = vdir->vfs->data;
  struct ramnode *node, *parent = vdir->data;
  struct ramdentry *de;
  struct vnode *vn;

  node = ramfs_alloc_node(ram, parent, type);

  // alloc a dentry for the ramnode
  ramfs_alloc_dentry(node, &de, name);

  // add dentry to the parent's list
  // of dentries
  ramfs_dir_attach(parent, de);
  
  vdir->vfs->ops->vget(vdir->vfs, &vn, node);
  *vpp = vn;
}


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
  
  *de = dentry;
  return 0;
}

/*
  add dentry to the parent ramnode
*/
void
ramfs_dir_attach(struct ramnode *parent, struct ramdentry *dentry)
{
  dentry->next = parent->dir.dentry;
  parent->dir.dentry = dentry;
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
ramfs_lookup(struct vnode *vdir, struct vnode **vpp, struct componentname *path)
{

  struct ramnode *node = vdir->data;
  struct ramdentry *entry;

  if (vdir->type != VDIR)
    return 0;

  if (strncmp(path->nm, "..", path->len) == 0) {
    *vpp = node->dir.parent->vnode;
    return 0;
  }
  
  for (entry = node->dir.dentry; entry; entry = entry->next) {
    if (strncmp(entry->name, path->nm, path->len) == 0) {
      vdir->vfs->ops->vget(vdir->vfs, vpp, entry->node);
      return 0;
    }
  };
  return -1;
};


int
ramfs_close(struct vnode *vn)
{
  // unimplemented for ramfs  
}

int
ramfs_inactive(struct vnode *vn) {

  struct ramnode *node = vn->data;
  
  node->vnode = NULL;
}


static const struct vnodeops ramfs_vnode_ops = {
					 .lookup = ramfs_lookup,
					 .create = ramfs_create,
					 .mkdir = ramfs_mkdir,
					 .close = vfs_close,
};


const struct vfsops ram_ops = {
			 .name = "ramfs",
			 .mount = ramfs_mount,
			 .root = ramfs_root,
			 .vget = ramfs_vget,
			 /* .lookup = ramfs_lookup, */
};




