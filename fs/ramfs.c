#include <stddef.h>
#include <sys/errno.h>
#include <sys/types.h>

#include "std/string.h"
#include "franklin/spinlock.h"
#include "franklin/uio.h"
#include "franklin/fs/vfs.h"
#include "ramfs.h"


static const struct vnodeops ramfs_vnode_ops;

void *kalloc(size_t);


static int ramfs_create(struct vnode*, struct vnode**, const char*, enum vtype);
static int ramfs_mkdir(struct vnode*, struct vnode**, const char*, enum vtype);
static int ramfs_symlink(struct vnode*, const char*, const char*);
static int ramfs_readlink(struct vnode*, struct uio*);
static int ramfs_vget(struct vfs*, struct vnode**, ino_t);
static int ramfs_write(struct vnode *vn, void *buf, off_t offset, size_t count);
static int ramfs_read(struct vnode *vn, void *buf, off_t offset, size_t count);
static int ramfs_remove(struct vnode*, const char*);
static struct ramdentry* ramfs_dir_lookup(struct ramnode *dir, struct componentname*);



void ramfs_t() {

  struct vnode *vn, *v, *vv, *new, *w1, *w2, *w3, *w4;



  vfs_mkdir("/", "yes", &vn, VDIR);
  vfs_mkdir("/yes", "yes", &vn, VDIR);
  vfs_mount("/yes/yes", "ramfs");
  vfs_mkdir("/yes", "main.c", &vn, VDIR);
  vfs_mount("/yes/main.c", "ramfs");

  /* vfs_create("/yes", "nice.c", &vn, VREG); */
  struct componentname nam = {
			       .nm = "/yes/..",
			       .len = 4,
  };
  vfs_create("/yes/", "hahahahahahhahahaha", &vn, VDIR);

  
  /* vv = lookup(&nam); */
  vfs_create("/", "nice", &vv, VDIR);

  rootfs->ops->root(rootfs, &vv);
  

  vfs_symlink("/", "/yes", "symlink");

  vfs_create("/symlink/lol", "nice.c", &vn, VREG);
  
  struct uio uio = {
		    .offset = 0,
		    .buf = kalloc(10),
  };

  rootfs->ops->root(rootfs, &vn);
  struct ramnode *rn = vn->data;
  struct ramdentry *de;

  
  for (de = rn->dir.dentry; de; de = de->next) {
    if (strncmp(de->name, "yes", 3) == 0)
      break;
  }
  rn = de->node;
  /* for (de = rn->dir.dentry; de; de = de->next) { */
    /* print("\n"); */
    /* print(de->name); */
    /* print("\n"); */
  /* } */

  vfs_create("/yes", "ella.c", &vn, VREG);
  char buf[1000] = "#include <stdio.h>";
  int len = strlen(buf);
  ramfs_write(vn, buf, 0, len);

  
  vfs_create("/", "grinding", &vn, VDIR);
  vfs_create("/grinding", "main.c", &vn, VDIR); 
  vfs_open("/yes/ella.c", &vn, 0,0 );
  w1 = vn;
  vfs_close(vn);
  vfs_symlink("/grinding", "/", "linkbitch");
  vfs_symlink("/grinding/linkbitch", "grinding", "unna");
  
  ramfs_read(vn, buf, 0, len); // LOCK BUG
  
  print(buf);
  print("\n");

  rootfs->ops->root(rootfs, &vn);
  ramfs_remove(vn, "unna");

  vfs_open("/grinding/main.c", &vn, 0, 0);

  /* print(buf); */
}

static struct ramnode* ramfs_alloc_node(struct ramvfs*, struct ramnode*, enum vtype, const char*);
static int ramfs_alloc_file(struct vnode*, struct vnode **, const char *, enum vtype, const char*);
static void ramfs_dir_attach(struct ramnode*, struct ramdentry*);
static int ramfs_dir_detach(struct ramnode*, struct ramdentry*);
static int ramfs_alloc_dentry(struct ramnode*, struct ramdentry **, const char *);
static int ramfs_vget(struct vfs *vfs, struct vnode **vnode, ino_t ino);
static int ramfs_lookup(struct vnode*, struct vnode**, struct componentname*);
static int ramfs_free_dentry(struct ramdentry*);


/* mount ramfs
 - doesn't create a dentry for the root
 - initialize root ramnode and root mount struct
 - vfs is locked so no need to lock ramvfs
*/
static int
ramfs_mount(struct vfs *vfs)
{
  struct ramvfs *ram = kalloc(sizeof *ram);
  struct ramnode *root;

  // init root ramnode
  root = ramfs_alloc_node(ram, NULL, VDIR, NULL);
  ram->root = root;

  vfs->data = ram;
}


/*
  Write to file 'vn' from the buffer 'buf'
  for 'count' amount of bytes, starting at offset 'offset'
  - file type has to be VREG
*/
static int
ramfs_write(struct vnode *vn, void *buf, off_t offset, size_t count)
{
  struct ramnode *node = vn->data;
  void *newdata;
  size_t newsize;
  
  acquire(&node->ramlock);

  if (offset + count >= node->reg.size) {
    newsize = node->reg.size + 1; // incase size is 0
    while (offset + count >= newsize)
      newsize *= 2;
    newdata = kalloc(newsize);

    memcpy(newdata, node->reg.data, node->reg.size);
    if (node->reg.data)
      kfree(node->reg.data);

    node->reg.data = newdata;
    node->reg.size = newsize;
  }
  memcpy((char*)node->reg.data + offset, buf, count);

  release(&node->ramlock);
  return 0;
}


static int
ramfs_read(struct vnode *vn, void *buf, off_t offset, size_t count)
{
  struct ramnode *node = vn->data;
  
  acquire(&node->ramlock);

  if (offset + count > node->reg.size)
    return -EINVAL;
  if (node->reg.data == NULL)
    return 0;
  
  memcpy(buf, node->reg.data, count);
  release(&node->ramlock);
  return 0;
}

static int
ramfs_open(struct vnode *vn)
{
  
}


static int
ramfs_remove(struct vnode *vdir, const char *name)
{

  struct vnode *vn;
  struct ramnode *parent = vdir->data;
  struct ramdentry *de;
  struct componentname cname;
  
  if (vdir->type != VDIR || parent == NULL)
    return -EINVAL;

  cname.len = strlen(name);
  cname.nm = strdup(name);

  de = ramfs_dir_lookup(parent, &cname);
  if (de == NULL)
    return -ENOENT;

  ramfs_dir_detach(parent, de);
  ramfs_free_dentry(de);
}


static int
ramfs_free_dentry(struct ramdentry *dentry)
{
  struct ramnode *node = dentry->node;

  acquire(&node->ramlock);

  if (node->linkcount <= 0)
    panic("ramfs_free_dentry: linkcount <= 0");
  
  node->linkcount--;
  if (node->linkcount == 0) {
    kfree(dentry->name);
    kfree(node);
  } else
    release(&node->ramlock);

}


static int
ramfs_dir_detach(struct ramnode *parent, struct ramdentry *de)
{
  acquire(&parent->ramlock);
  struct ramdentry *d = parent->dir.dentry;

  if (d == de) {
    d = d->next;
  } else {
    while (d->next != de)
      d = d->next;

    d->next = de->next;
  }

  parent->dir.dentry = d;

  release(&parent->ramlock);
  return 0;
}

/*
 Create a file in vdir directory
 with name nm
*/
static int
ramfs_create(struct vnode *vdir, struct vnode **vpp, const char *name, enum vtype type)
{
  /* if (type != VREG || type != VREG) */
    /* panic("ramfs_create: type"); */
  ramfs_alloc_file(vdir, vpp, name, type, NULL);
  return 0;
};



static int
ramfs_mkdir(struct vnode *vdir, struct vnode **vpp, const char *name, enum vtype type)
{

  if (type != VDIR)
    return -EINVAL;

  ramfs_alloc_file(vdir, vpp, name, type, NULL);
  return 0;
}

/*
 Allocate a new file of type type
 and add it to the parent directory dir
*/
static int
ramfs_alloc_file(struct vnode *vdir, struct vnode **vpp, const char *name, enum vtype type,
		 const char *target)
{
  if (vdir->type != VDIR || (target && type != VLNK))
    return -EINVAL;
  
  struct ramvfs *ram = vdir->vfs->data;
  struct ramnode *node, *parent = vdir->data;
  struct ramdentry *de;
  struct vnode *vn;

  node = ramfs_alloc_node(ram, parent, type, target);

  
  // alloc a dentry for the ramnode
  ramfs_alloc_dentry(node, &de, name); 

  // add dentry to the parent's list
  // of dentries
  ramfs_dir_attach(parent, de);
  
  ramfs_vget(vdir->vfs, &vn, (ino_t)node);
  *vpp = vn;
}


/* 
   Creates new ramnode and returns it locked

   - if type is VDIR,
     dir points at the parent (unless root)

   - adds the node to the list of all nodes
     in the filesystem (ram)

   - (DOESN'T ALLOC A DENTRY)
 */
static struct ramnode*
ramfs_alloc_node(struct ramvfs *ram, struct ramnode *dir, enum vtype type, const char *link)
{
  
  struct ramnode *node = kalloc(sizeof *node);
  init_and_acquire(&node->ramlock);

  // add node to the list of all
  // ramnodes in the *ram filesystem
  node->nextnode = ram->nodes;
  ram->nodes = node;
  
  node->type = type;
  node->vnode = NULL;
  node->linkcount = 0;

  switch (node->type) {
  case VDIR:
    node->dir.parent = (dir == NULL) ? node : dir;
    break;
  case VLNK:
    node->lnk.link = strdup(link);
    break;
  case VREG:
    node->reg.size = 0;
    node->reg.data = NULL;
    break;
  }

  release(&node->ramlock);
  return node;
}


/*
  allocates a dentry for a ramnode
*/
static int
ramfs_alloc_dentry(struct ramnode *node, struct ramdentry **de, const char *name)
{
  
  struct ramdentry *dentry = kalloc(sizeof *dentry);
  acquire(&node->ramlock);

  dentry->name = strdup(name);
  dentry->node = node;;
  dentry->next = NULL;
  node->linkcount++;
  
  release(&node->ramlock);
  *de = dentry;
  return 0;
}


/*
  add dentry to the parent ramnode
*/
static void
ramfs_dir_attach(struct ramnode *parent, struct ramdentry *dentry)
{
  acquire(&parent->ramlock);
  if (parent->type != VDIR)
    return -EINVAL;
  
  dentry->next = parent->dir.dentry;
  parent->dir.dentry = dentry;
  release(&parent->ramlock);
}


/*
  (hard link)
  Link the vnode vn to the name name,
  in the directory vdir

  - Allocate a dentry for the link 
    and add it to the parent's list of dentries

  - A dentry is a link, which points at a file
*/
static int
ramfs_link(struct vnode *vdir, struct vnode *vn, const char *name)
{
  struct ramnode *parent = vdir->data;
  struct ramdentry *de;
  
  if (vn->type == VDIR || vn->vfs != vdir->vfs)
    return -EINVAL;
  
  ramfs_alloc_dentry(vn->data, &de, name);
  ramfs_dir_attach(parent, de);
}

/*
  Create a symbolic link that points at 'path'
*/
static int
ramfs_symlink(struct vnode *vdir, const char *target, const char *name)
{
  struct vnode *vpp;
  ramfs_alloc_file(vdir, &vpp, name, VLNK, target);
}

static int
ramfs_readlink(struct vnode *vn, struct uio *uio)
{
  struct ramnode *node = vn->data;
  
  if (uio->offset != 0 || node->type != VLNK)
    return -EINVAL;

  strcpy(uio->buf, node->lnk.link);
}


/*
  Get root vnode of the mounted filesystem
*/
static int
ramfs_root(struct vfs *vfs, struct vnode **vnode)
{
  struct ramvfs *rootvfs = vfs->data;
  ramfs_vget(vfs, vnode, (ino_t)rootvfs->root);
  return 0;
}


// get vnode corresponding to the ramnode 'ino'
static int
ramfs_vget(struct vfs *vfs, struct vnode **vnode, ino_t ino)
{
  struct ramnode *node = (struct ramnode*)ino;
  struct vnode *vn;
  acquire(&node->ramlock);
  
  if ((vn = node->vnode)) {
    acquire(&vn->lock);
    vn->refcount++;
    *vnode = vn;
    goto ret;
  } else {
    
    vn = kalloc(sizeof *vn);
    init_and_acquire(&vn->lock);
    
    node->vnode = vn;
    
    vn->mountedhere = NULL;
    vn->ops = &ramfs_vnode_ops;
    vn->type = node->type;
    vn->refcount = 1;
    vn->data = node;
    vn->vfs = vfs;
    *vnode = vn;
  };
 ret:
  release(&vn->lock);
  release(&node->ramlock);
  return 0;
};


/*
  ramfs_lookup

  - lookup the vnode corresponding
    to target in the directory vdir
*/
static int
ramfs_lookup(struct vnode *vdir, struct vnode **vpp, struct componentname *path)
{

  struct ramnode *node = vdir->data;
  struct ramdentry *entry;

  acquire(&node->ramlock);

  if (vdir->type != VDIR)
    return 0;

  if (strncmp(path->nm, "..", path->len) == 0) {
    ramfs_vget(vdir->vfs, vpp, (ino_t)node->dir.parent);
    return 0;
  }
  
  for (entry = node->dir.dentry; entry; entry = entry->next) {
    if (strncmp(entry->name, path->nm, path->len) == 0) {
      
      vdir->vfs->ops->vget(vdir->vfs, vpp, entry->node);
      release(&node->ramlock);
      return 0;
    }
  };
  release(&node->ramlock);
  return -1;
};


static struct ramdentry*
ramfs_dir_lookup(struct ramnode *dir, struct componentname *cname)
{
  struct ramdentry *de;

  acquire(&dir->ramlock);
  for (de = dir->dir.dentry; de; de = de->next)
    if (strncmp(de->name, cname->nm, cname->len) == 0)
      break;

  release(&dir->ramlock);
  return de;
}




static int
ramfs_close(struct vnode *vn)
{
  (void)vn;
}

static int
ramfs_inactive(struct vnode *vn) {

  struct ramnode *node = vn->data;
  acquire(&node->ramlock);
  node->vnode = NULL;
  if (node->linkcount == 0) {

  }
  release(&node->ramlock);
}


static const struct vnodeops ramfs_vnode_ops = {
					 .lookup = ramfs_lookup,
					 .open = ramfs_open,
					 .create = ramfs_create,
					 .mkdir = ramfs_mkdir,
					 .close = ramfs_close,
					 .inactive = ramfs_inactive,
					 .readlink = ramfs_readlink,
					 .symlink = ramfs_symlink,
};


const struct vfsops ram_ops = {
			 .name = "ramfs",
			 .mount = ramfs_mount,
			 .root = ramfs_root,
			 .vget = ramfs_vget,
			 /* .lookup = ramfs_lookup, */
};




