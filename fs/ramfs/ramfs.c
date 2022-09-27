#include <stddef.h>
#include <sys/errno.h>
#include <sys/types.h>

#include "franklin/fs/dirent.h"
#include "franklin/fs/vfs.h"
#include "franklin/spinlock.h"
#include "ramfs.h"

#include "std/fcntl.h"
#include "std/string.h"

static const struct vnodeops ramfs_vnode_ops;

void* kalloc(size_t);

static int
ramfs_create(struct vnode*, struct vnode**, struct componentnam*, enum vtype);
static int
ramfs_mkdir(struct vnode*, struct vnode**, struct componentnam*);
static int
ramfs_symlink(struct vnode*, const char*, struct componentnam*);
static int
ramfs_readlink(struct vnode*, struct componentnam*);
static int
ramfs_vget(struct vfs*, struct vnode**, ino_t);
static int
ramfs_write(struct vnode* vn, void* buf, off_t offset, size_t count);
static int
ramfs_read(struct vnode* vn, void* buf, off_t offset, size_t count);
static int
ramfs_remove(struct vnode*, struct vnode*, struct componentnam*);
static struct ramdentry*
ramfs_dir_lookup(struct ramnode* dir, const char*);
static int
ramfs_readdir(struct vnode* vdir, void* buf, size_t* count, off_t* offset);

static struct ramnode*
ramfs_alloc_node(struct ramvfs*, struct ramnode*, enum vtype, const char*);
static int
ramfs_alloc_file(struct vnode*,
                 struct vnode**,
                 struct componentnam* cnp,
                 enum vtype,
                 const char*);
static int
ramfs_dir_attach(struct ramnode*, struct ramdentry*);
static int
ramfs_dir_detach(struct ramnode*, struct ramdentry*);
static int
ramfs_alloc_dentry(struct ramnode*, struct ramdentry**, const char*);
static int
ramfs_vget(struct vfs* vfs, struct vnode** vnode, ino_t ino);
static int
ramfs_lookup(struct vnode*, struct vnode**, struct componentnam*);
static int
ramfs_free_dentry(struct ramdentry*);
static int
ramfs_free_node(struct ramvfs*, struct ramnode*);

/* mount ramfs
 - doesn't create a dentry for the root
 - initialize root ramnode and root mount struct
 - vfs is locked so no need to lock ramvfs
*/
static int
ramfs_mount(struct vfs* vfs)
{
  struct ramvfs* ram = kalloc(sizeof *ram);
  struct vnode* vn;
  struct ramnode* root;

  // init root ramnode
  root = ramfs_alloc_node(ram, NULL, VDIR, NULL);

  // root's parent points to itself
  // so it has a linkcount of 2
  root->linkcount++;

  root->prev = NULL;
  root->next = NULL;
  ram->root = root;
  ram->nodes = root;

  vfs->data = ram;
  return 0;
}

/*
  Unmount the mounted filesystem by destroying
  all the nodes and dentries in the filesystem
*/
static int
ramfs_unmount(struct vfs* vfs)
{
  struct ramvfs* ram = vfs->data;
  struct ramnode *node, *node2;
  struct ramdentry* de;

  // first destroy all dentries
  for (node = ram->nodes; node; node = node->next) {
    node->vnode = NULL;
    if (node->type != VDIR)
      continue;

    for (de = node->dir.dentry; de; de = de->next) {
      node2 = de->node;
      if (node2)
        node2->vnode = NULL;
      ramfs_dir_detach(node, de);
      ramfs_free_dentry(de);
    }
  }
  // now destroy all nodes
  for (node = ram->nodes; node; node = node->next) {
    ramfs_free_node(ram, node);
  }
  kfree(ram);
  return 0;
}

/*
  Write to file 'vn' from the buffer 'buf'
  for 'count' amount of bytes, starting at offset 'offset'
  - file type has to be VREG
*/
static int
ramfs_write(struct vnode* vn, void* buf, off_t offset, size_t count)
{
  struct ramnode* node = vn->data;
  void* newdata;
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
ramfs_read(struct vnode* vn, void* buf, off_t offset, size_t count)
{
  struct ramnode* node = vn->data;
  int error = 0;

  acquire(&node->ramlock);

  if (offset + count > node->reg.size) {
    error = -EINVAL;
    goto fail;
  }
  if (node->reg.data == NULL) {
    goto fail;
  }

  memcpy(buf, node->reg.data, count);
fail:
  release(&node->ramlock);
  return 0;
}

static int
ramfs_open(struct vnode* vn)
{}

/*
  Remove a file from a directory by

*/
static int
ramfs_remove(struct vnode* vdir, struct vnode* vn, struct componentnam* cnp)
{
  struct ramnode *parent = vdir->data, *node = vn->data;
  struct ramdentry* de;

  if (vn->type == VDIR)
    return -EISDIR;
  if (vdir->type != VDIR)
    return -ENOTDIR;

  de = ramfs_dir_lookup(parent, cnp->name);
  if (de == NULL || de->node != node)
    panic("ramfs_remove: parent");

  /* Remove the dentry from the directory
   * and decrement the link count
   * Note: the node referred by it will not be
   * destroyed until the vnode is destroyed
   */
  ramfs_dir_detach(parent, de);
  ramfs_free_dentry(de);
  return 0;
}

/*
  Remove a directory
*/
static int
ramfs_rmdir(struct vnode* vdir, struct vnode* vpp, struct componentnam* cnp)
{
  struct ramnode *node = vpp->data, *parent = vdir->data;
  struct ramdentry* de;
  int error;

  if (cnp->len == 2 && cnp->name[0] == '.' && cnp->name[1] == '.')
    return -ENOTEMPTY;

  acquire(&node->ramlock);

  if (node->dir.parent != vdir->data)
    panic("ramfs_rmdir: parent");

  if (node->dir.dentry) {
    error = -ENOTEMPTY;
    goto fail;
  }
  // decrement the linkcount for the virtual '.' entry
  node->linkcount--;
  release(&node->ramlock);

  de = ramfs_dir_lookup(parent, cnp->name);
  if (de == NULL || de->node != node)
    panic("ramfs_rmdir: node");

  // remove dentry from the parent directory
  ramfs_dir_detach(parent, de);

  // delete the dentry
  ramfs_free_dentry(de);
  return 0;
fail:
  release(&node->ramlock);
  return error;
}

static int
ramfs_free_dentry(struct ramdentry* dentry)
{
  struct ramnode* node = dentry->node;
  acquire(&node->ramlock);

  if (node->linkcount <= 0)
    panic("ramfs_free_dentry: linkcount <= 0");

  node->linkcount--;

  kfree(dentry->name);
  kfree(dentry);
  release(&node->ramlock);
  return 0;
}

/*
  Detach dentry from a directory's list of dentries
*/
static int
ramfs_dir_detach(struct ramnode* parent, struct ramdentry* de)
{
  acquire(&parent->ramlock);
  struct ramdentry* d;

  parent->linkcount--;

  if (parent->dir.dentry == de) {
    parent->dir.dentry = parent->dir.dentry->next;
  } else {
    d = parent->dir.dentry;
    while (d->next != de) {
      d = d->next;
    }
    d->next = d->next->next;
  }

  release(&parent->ramlock);
  return 0;
}

/*
 Create a file in vdir directory
 with name nm
*/
static int
ramfs_create(struct vnode* vdir,
             struct vnode** vpp,
             struct componentnam* cnp,
             enum vtype type)
{
  int error;
  error = ramfs_alloc_file(vdir, vpp, cnp, type, NULL);
  return error;
};

static int
ramfs_mkdir(struct vnode* vdir, struct vnode** vpp, struct componentnam* cnp)
{
  return ramfs_alloc_file(vdir, vpp, cnp, VDIR, NULL);
}

/*
 Allocate a new file of type type
 and add it to the parent directory dir
*/
static int
ramfs_alloc_file(struct vnode* vdir,
                 struct vnode** vpp,
                 struct componentnam* cnp,
                 enum vtype type,
                 const char* target)
{
  struct ramvfs* ram = vdir->vfs->data;
  struct ramnode *node, *parent = vdir->data;
  struct ramdentry* de;
  struct vnode* vn;

  if (vdir->type != VDIR || (target && type != VLNK))
    return -EINVAL;

  node = ramfs_alloc_node(ram, parent, type, target);

  // alloc a dentry for the ramnode
  ramfs_alloc_dentry(node, &de, cnp->name);

  // add dentry to the parent's list
  // of dentries
  ramfs_dir_attach(parent, de);

  ramfs_vget(vdir->vfs, &vn, (ino_t)node);
  *vpp = vn;
  return 0;
}

/*
  Make dirent out of ramnode
*/
static int
ramfs_make_dirent(struct ramnode* node, struct dirent* d, const char* name)
{
  strcpy(d->name, name);
  d->reclen = get_reclen(d);
  d->ino = (ino_t)node;
  return 0;
}

/*
  Read entries from directory vdir

  - amount of bytes that were read are returned in count
*/
static int
ramfs_readdir(struct vnode* vdir, void* buf, size_t* count, off_t* offset)
{
  size_t resid = *count;
  struct ramnode* parent = vdir->data;
  struct ramdentry* de;
  struct dirent d;
  int error = 0;

  acquire(&parent->ramlock);

  if (*offset == SEQ_DOT) {
    ramfs_make_dirent(parent, &d, ".");
    memcpy(buf, &d, d.reclen);
    buf = (char*)buf + d.reclen;

    *offset = SEQ_DOTDOT;
    resid -= d.reclen;
  }
  if (*offset == SEQ_DOTDOT) {
    ramfs_make_dirent(parent->dir.parent, &d, "..");
    memcpy(buf, &d, d.reclen);
    buf = (char*)buf + d.reclen;

    de = parent->dir.dentry;
    *offset = de ? de->seq : SEQ_EOF;
    resid -= d.reclen;
  }

  if (*offset == SEQ_EOF)
    goto done;

  // get dentry corresponding to offset
  for (de = parent->dir.dentry; de; de = de->next)
    if (de->seq == *offset)
      break;

  if (de == NULL) {
    error = EINVAL;
    goto done;
  }

  do {
    ramfs_make_dirent(de->node, &d, de->name);
    d.type = de->node->type;
    switch (de->node->type) {
      case VBLK:
        d.type = DT_BLK;
        break;
      case VREG:
        d.type = DT_REG;
      case VDIR:
        d.type = DT_DIR;
        break;
      case VSOCK:
        d.type = DT_SOCK;
        break;
      case VLNK:
        d.type = DT_LNK;
        break;
      case VCHR:
        d.type = DT_CHR;
        break;
      default:
        panic("ramfs_readdir: unknown type");
        break;
    }

    memcpy(buf, &d, d.reclen);
    buf = (char*)buf + d.reclen;
    *offset = de->seq;
  } while ((resid -= d.reclen) && (de = de->next));

  // amount of bytes that were read
done:
  *count -= resid;
  release(&parent->ramlock);
  return error;
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
ramfs_alloc_node(struct ramvfs* ram,
                 struct ramnode* dir,
                 enum vtype type,
                 const char* link)
{
  struct ramnode* node = kalloc(sizeof *node);
  init_and_acquire(&node->ramlock);

  // add node to the list of all
  // ramnodes in the *ram filesystem
  if (ram->nodes)
    ram->nodes->prev = node;
  node->next = ram->nodes;
  ram->nodes = node;
  node->prev = NULL;

  node->type = type;
  node->vnode = NULL;
  node->linkcount = 0;

  switch (node->type) {
    case VDIR:
      node->dir.parent = (dir == NULL) ? node : dir;
      node->dir.lastseq = SEQ_START;

      node->linkcount++; // extra link for the virtual '.' entry
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
  Free the node and its contents
  in: locked

*/
static int
ramfs_free_node(struct ramvfs* ram, struct ramnode* node)
{
  // remove from the list of all nodes
  if (node->next != NULL)
    node->next->prev = node->prev;
  node->prev = node->next;

  switch (node->type) {
    case VREG:
      kfree(node->reg.data);
      break;
    case VLNK:
      kfree(node->lnk.link);
      break;
    default:
      break;
  }
  kfree(node);
}

/*
  allocates a dentry for a ramnode
*/
static int
ramfs_alloc_dentry(struct ramnode* node,
                   struct ramdentry** de,
                   const char* name)
{
  struct ramdentry* dentry = kalloc(sizeof *dentry);
  acquire(&node->ramlock);

  dentry->name = strdup(name);
  dentry->node = node;
  dentry->next = NULL;
  node->linkcount++;

  release(&node->ramlock);
  *de = dentry;
  return 0;
}

/*
  add dentry to the parent ramnode
*/
static int
ramfs_dir_attach(struct ramnode* parent, struct ramdentry* dentry)
{
  int error = 0;
  acquire(&parent->ramlock);
  if (parent->type != VDIR) {
    error = -EINVAL;
    goto fail;
  }
  dentry->seq = parent->dir.lastseq++;

  dentry->next = parent->dir.dentry;
  parent->dir.dentry = dentry;

  parent->linkcount++;
fail:
  release(&parent->ramlock);
  return error;
}

/*
  (hard link)
  Link the vnode vn to the name name,
  in the directory vdir

  - Allocate a dentry for the link
    and add it to the parent's list of dentries

  (A dentry is a link, which points at a file)
*/
static int
ramfs_link(struct vnode* vdir, struct vnode* vn, struct componentnam* cnp)
{
  struct ramnode* parent = vdir->data;
  struct ramdentry* de;

  ramfs_alloc_dentry(vn->data, &de, cnp->name);
  ramfs_dir_attach(parent, de);
  return 0;
}

/*
  Create a symbolic link that points at 'path'
*/
static int
ramfs_symlink(struct vnode* vdir, const char* target, struct componentnam* cnp)
{
  struct vnode* vpp;
  return ramfs_alloc_file(vdir, &vpp, cnp, VLNK, target);
}

static int
ramfs_readlink(struct vnode* vn, struct componentnam* cnp)
{
  struct ramnode* node = vn->data;

  acquire(&node->ramlock);
  strcpy(cnp->name, node->lnk.link);
  cnp->len = strlen(node->lnk.link);
  release(&node->ramlock);
  return 0;
}

/*
  Get root vnode of the mounted filesystem
*/
static int
ramfs_root(struct vfs* vfs, struct vnode** vnode)
{
  struct ramvfs* rootvfs = vfs->data;
  return ramfs_vget(vfs, vnode, (ino_t)rootvfs->root);
}

// get vnode corresponding to the ramnode 'ino'
static int
ramfs_vget(struct vfs* vfs, struct vnode** vnode, ino_t ino)
{
  struct ramnode* node = (struct ramnode*)ino;
  struct vnode* vn;
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

    if (node->type == VDIR) {
      vn->flags |= node->dir.parent == node ? VROOT : 0;
    }

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
ramfs_lookup(struct vnode* vdir, struct vnode** vpp, struct componentnam* cnp)
{
  struct ramnode *parent, *node = vdir->data;
  struct ramdentry* entry;
  int error = 0;

  acquire(&node->ramlock);
  if (vdir->type != VDIR) {
    error = -ENOTDIR;
    goto done;
  }

  if (strcmp(cnp->name, ".") == 0) {
    *vpp = vdir;
    vref_locked(vdir);
    goto done;
  }

  if (strcmp(cnp->name, "..") == 0) {
    parent = node->dir.parent;
    ramfs_vget(vdir->vfs, vpp, (ino_t)parent);
    goto done;
  }

  for (entry = node->dir.dentry; entry; entry = entry->next) {
    if (strncmp(entry->name, cnp->name, cnp->len) == 0) {
      vdir->vfs->ops->vget(vdir->vfs, vpp, (ino_t)entry->node);
      goto done;
    }
  };
  *vpp = NULL;
  error = -ENOENT;
done:
  release(&node->ramlock);
  return error;
};

static struct ramdentry*
ramfs_dir_lookup(struct ramnode* dir, const char* name)
{
  struct ramdentry* de;

  acquire(&dir->ramlock);
  for (de = dir->dir.dentry; de; de = de->next)
    if (strcmp(de->name, name) == 0)
      break;

  release(&dir->ramlock);
  return de;
}

static int
ramfs_close(struct vnode* vn)
{
  (void)vn;
}

/*
  vnode vn is not used anymore
  if the file has been deleted, delete the file contents
*/
static int
ramfs_inactive(struct vnode* vn)
{
  struct ramnode* node = vn->data;
  acquire(&node->ramlock);
  node->vnode = NULL;
  if (node->linkcount == 0) {
    ramfs_free_node(vn->vfs->data, node);
  }
  release(&node->ramlock);
}

static const struct vnodeops ramfs_vnode_ops = {
  .lookup = ramfs_lookup,
  .open = ramfs_open,
  .read = ramfs_read,
  .write = ramfs_write,
  .create = ramfs_create,
  .remove = ramfs_remove,
  .mkdir = ramfs_mkdir,
  .close = ramfs_close,
  .inactive = ramfs_inactive,
  .readlink = ramfs_readlink,
  .symlink = ramfs_symlink,
  .readdir = ramfs_readdir,
  .link = ramfs_link,
  .rmdir = ramfs_rmdir,
};

const struct vfsops ram_ops = {
  .name = "ramfs",
  .mount = ramfs_mount,
  .unmount = ramfs_unmount,
  .root = ramfs_root,
  .vget = ramfs_vget,
  /* .lookup = ramfs_lookup, */
};

void
ramfs_t()
{
  off_t offset = 0;
  char buf[4096];
  struct dirent* d = buf;
  struct vnode *vn, *v, *root;
  struct ramnode* node;
  struct ramdentry* de;
  int r;
  struct nameidata nd;

  vfs_mkdir("/mnt", &v);
  r = vfs_mkdir("/", &v);
  if (r != -EEXIST)
    panic("eexits");

  vfs_mkdir("/first", &v);
  vfs_mount("/first", "ramfs");
  vfs_mkdir("/first/a", &v);
  vfs_mkdir("/first/b", &v);
  vfs_mkdir("/first/c", &v);
  vfs_unmount("/first");

  r = vfs_unmount("/");
  if (r != -EINVAL)
    panic("einval");
  r = vfs_unmount("/mnt");

  /* vfs_mkdir("", &vn); */
  vfs_mkdir("/mine", &v);
  vfs_rmdir("/.");
  vfs_mount("/mine", "ramfs");
  vfs_mkdir("/mine/con", &v);
  node = v->data;
  if (node->linkcount != 2)
    panic("lincount");
  r = vfs_rmdir("/mine/con");
  if (r != 0)
    panic("minecon");

  vfs_mkdir("/hh", &v);
  printdir(v);

  r = vfs_rmdir("/hh");
  if (r != 0)
    panic("rm,dir");
  r = vfs_rmdir("/sex");
  if (r != -ENOENT)
    panic("enoent");
  vfs_open("/main.c", &v, VREG, O_CREATE);
  r = vfs_rmdir("/main.c");
  if (r != -ENOTDIR)
    panic("enotdri");
  r = vfs_open("/main.c", &v, VREG, 0);
  if (r != 0)
    panic("oepen");
  r = vfs_open("/pussy", &v, VREG, 0);
  if (r != -ENOENT)
    panic("pussY");

  vfs_unlink("/.");
  vfs_mkdir("/../../../../../../lmao", &vn);
  vfs_mkdir("/a", &vn);
  vfs_mkdir("/a/b", &vn);
  vfs_symlink("/a/b/l", "/a");
  namei("/a/b/l/b/l/b/l", &nd);
  r = vfs_link("/lmao/lol", "/a");
  if (r != -EISDIR)
    panic("EISDIR");
  r = namei("/lmao/lol/..", &nd);
  if (r != -ENOENT)
    panic("ENOENT");
  vfs_link("/hahahahah", "/");

  vfs_open("/", &root, 0, 0);
  vfs_close(root);
  vfs_symlink("/777", "/../../../../../../..");
  vfs_mkdir("/777/lmao", &vn);
  vfs_open("/", &root, 0, 0);
  if (root->refcount != 2)
    panic("root");
  vfs_close(root);
  vfs_symlink("/fornite", "/777");
  vfs_mkdir("/fornite/SEX", &vn);
  vfs_mkdir("/fornite/SEX/nice", &vn);
  print("\n");
  printdir(root);
  r = vfs_mkdir("/lmao", &vn);
  if (r != -EEXIST)
    panic("/lmao");
  vfs_mkdir("/lmao/nice", &vn);
  r = vfs_mkdir("/lmao/nice", &vn);
  if (r != -EEXIST)
    panic("mkdir");
  vfs_mkdir("/whale", &vn);
  /* vfs_mkdir("", &vn); */

  r = vfs_open("/", &root, 0, 0);
  if (r < 0 || (root->flags & VROOT) == 0)
    panic("error1");
  vfs_close(root);
  r = vfs_mount("/", "ramfs");
  if (r < 0)
    panic("error2");
  r = vfs_mount("/", "ramfs");
  if (r != -EBUSY)
    panic("mountpoint busy");
  r = vfs_mkdir("/mnt", &vn);
  if (r != -EEXIST)
    panic("error4");
  r = vfs_mount("/mnt", "ramfs");
  if (r < 0)
    panic("mtn mount");
  r = vfs_open("/mnt", &v, 0, 0);
  r = vfs_mkdir("/mnt/bruno", &v);
  if (v->refcount > 1)
    panic("refcount");
  r = vfs_mkdir("/mnt/bruno/mars", &vn);
  if (v->refcount > 1)
    panic("refcount");
  r = vfs_unmount("/mnt");
  r = vfs_open("/mnt", &vn, 0, 0);
  if (vn == v || vn->mountedhere)
    panic("unmount");
  offset = 0;
  r = vfs_readdir(root, d, 1000, &offset);
  if (r == 0)
    panic("error5");
  if (strcmp(d->name, "mnt") != 0)
    ;
  r = vfs_open("/main.c", &vn, VREG, O_CREATE);
  vput(vn);
  if (r < 0)
    panic("create");
  r = vfs_create("/main.c/error", &vn, VREG);
  if (r != -ENOTDIR)
    panic("create dir");
  vfs_mkdir("/pe", &v);

  vfs_open("/", &vn, 0, 0);
  if (vn->refcount != 2)
    panic("refocunt");
  vfs_close(vn);
  r = vfs_rmdir("/mnt/bruno");
  if (r != -ENOENT)
    panic("enotdir");
  r = vfs_rmdir("/mnt");
  if (r != 0)
    panic("rmdir");
  r = vfs_rmdir("/whale");
  if (r != 0)
    panic("rmdir");
  r = vfs_rmdir("/lmao");
  if (r != -ENOTEMPTY)
    panic("rmdir");
  r = vfs_unlink("/main.c");
  vfs_mkdir("/lmo", &v);

  /* vfs_open("/main.cc", &v, VREG, O_CREATE); */
  /* vfs_mkdir("/lmoa/tmp", &v); */
  /* vfs_open("/keeper", &v, VREG, O_CREATE); */
  /* vfs_close(v); */
  /* r = vfs_rmdir("/keeper"); */
  /* if (r != -ENOTDIR) */
  /* panic("NOTDIR"); */
  /* vfs_unlink("/keeper"); */
  /* vfs_rmdir("/tmp"); */
  /* vfs_unlink("/777"); */
  /* vfs_unlink("/fornite"); */
  /* vfs_unlink("/lmoa"); */
  /* vfs_rmdir("/SEX/nice"); */

  /* r = vfs_rmdir("/SEX"); */
  /* /\* if (r != -ENOTEMPT´) *\/ */
  /* /\* panic("ENOTEMPTY"); *\/ */
  /* r = vfs_unlink("/lmao"); */
  /* if (r != -EISDIR) */
  /*   panic("ESIDIR"); */
  /* r = vfs_rmdir("/lmao"); */
  /* if (r != -ENOTEMPTY) */
  /*   panic("ENOTMEPT)"); */
  /* r = vfs_unlink("/."); */
  /* if (r != -EBUSY) */
  /*   panic("EPERM"); */
  /* vfs_open("/", &vn); */
  /* /\* printdir(vn); *\/ */
}
