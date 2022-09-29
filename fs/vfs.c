
/*
   mount plan

   vfs_mount() takes in the mount point string
   and the file system type string,

   1. find the correct vfsops struct for
      the file system type by iterating
      the vfsops linked list and comparing
      vfsops->name == fstype

   2. find the vnode corresponding to the
      mntpoint string with vnode->lookup()

   3. allocate a vfs struct and set vfs->ops = vfsops

   4. set vnode->mountedhere = vfs and,
      add vfs to the vfs linked list
      then call vfs_mount()

   5. vfs_mount() will allocate a root inode
      and a fs specific mount structure,
      and set mount->root = root

*/

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "franklin/fs/dirent.h"
#include "ramfs/ramfs.h"

#include "franklin/fs/vfs.h"
#include "std/fcntl.h"
#include "std/string.h"

int
lookup(struct nameidata*);

void*
kalloc(int);

struct vfsops ram_ops;

void
init_rootfs()
{
  vfslist.first = ram_ops;
  init_lock(&vfslist.lock);
};

struct vfs* rootfs;
struct vnode* rootvn;

void
init_rootvn()
{
  if (rootfs)
    rootfs->ops->root(rootfs, &rootvn);
}

/*
  Convert name to a vnode

  overall outline of namei:

  1. get starting directory of lookup (root or cwd)
  2. call lookup routine
  3. if result is a symbolic link concat the link
     with the remaining path and goto step 1

  Note: caller has to vput() ndp->vdir
*/
int
namei(const char* name, struct nameidata* ndp)
{
  struct vfs* vfs;
  struct vnode* vn;
  struct componentnam* cnp = &ndp->cn;
  struct componentnam sym;
  char* bufptr;
  char* savepath;
  int error;

  cnp->name = strdup(name);
  cnp->len = strlen(name);
  ndp->vn = NULL;
  savepath = cnp->name;

  /* ndp->vdir = current->cwd; */
  int x = 0;

  for (;;) {
    if (*cnp->name == '/') {
      if (x++)
        vput(vn);
      vfs = rootfs;
      vfs->ops->root(vfs, &vn);
      while (*cnp->name == '/')
        cnp->name++;
    }
    ndp->vdir = vn;
    if ((error = lookup(ndp)) != 0)
      goto fail;
    if ((ndp->flags & ISSYMLINK) == 0)
      goto fail;

    vn = ndp->vn;

    // Concat the symbolic link and the path together
    sym.name = kalloc(512);
    vn->ops->readlink(vn, &sym);
    strcpy(sym.name + sym.len, cnp->name + cnp->len);
    cnp->name = sym.name;

    kfree(savepath);
    savepath = sym.name;

    // symbolic link vnode is not needed anymore
    vput(vn);

    vn = ndp->vdir;
  }
fail:
  return error;
}

/*
  Lookup a vnode in the file system

  overall outline of lookup:

  1. get the next component of the pathname
     if the pathname is null, return
     if .. and vnode is VROOT, get vnode in the other filesystem
  2. call VOP_LOOKUP()
       if the resulting vnode is a mountpoint,
       get root of the mounted fs
       if the resulting vnode is a symbolic link, return
  3. goto step1

   (symbolic links are always followed except
    if it's the last component, then the FOLLOW flags is checked)
*/
int
lookup(struct nameidata* ndp)
{
  char* cp;
  struct vfs* vfs;
  struct componentnam* cnp = &ndp->cn;
  struct vnode *root, *vdir2, *vdir = ndp->vdir;
  int error;

  ndp->flags &= ~ISSYMLINK;

  for (;;) {
    for (cp = cnp->name; *cp && *cp != '/'; cp++)
      ;
    cnp->len = cp - cnp->name;
    ndp->next = cp; // save ptr to next component

    if (*cnp->name == 0) {
      ndp->vn = ndp->vdir;
      vref(ndp->vn);
      return 0;
    }
    // if root, ignore and goto next name
    // if root of a filesystem, get the mountpoint vnode of the mounted
    // filesystem
    if (strncmp(cnp->name, "..", 2) == 0) {
      for (;;) {
        if (vdir == rootvn) {
          ndp->vdir = ndp->vn = rootvn;
          vref(vdir);
          goto nextname;
        }
        acquire(&vdir->lock);
        if ((vdir->flags & VROOT) == 0) {
          release(&vdir->lock);
          break;
        }
        vdir2 = vdir;
        vdir = vdir->vfs->mountpoint;
        vrele(vdir2);
        vref(vdir);
      }
    }

    ndp->vdir = vdir;
    acquire(&vdir->lock);
    error = vdir->ops->lookup(vdir, &ndp->vn, cnp);
    release(&vdir->lock);

    if (error != 0) {
      goto fail;
    }
    vdir = ndp->vn;
    acquire(&vdir->lock);

    /* if crossing mountpoints, get the root vnode
     * of the mounted filesystem
     */
    while (vdir->type == VDIR && (vfs = vdir->mountedhere)) {
      vrele(vdir);
      error = vfs->ops->root(vfs, &vdir);
      if (error != 0)
        return error;
      acquire(&vdir->lock);
      ndp->vn = vdir; // incase this was the last component
    }

    // check for symbolic link
    if (vdir->type == VLNK && (ndp->flags & FOLLOW || *ndp->next == '/')) {
      ndp->flags |= ISSYMLINK;
      // dont call vput(ndp->vdir), since it's needed in namei
      return 0;
    }
    // get next name
  nextname:
    if (*ndp->next == 0)
      break;
    cnp->name = ndp->next;
    while (*cnp->name == '/') {
      cnp->name++;
    }
    // not needed anymore
    vput(ndp->vdir);

    release(&vdir->lock);
  }
  release(&vdir->lock);
  return 0;
fail:
  /* vput(vdir); */
  return error;
}

ssize_t
vfs_readdir(struct vnode* vdir, void* buf, size_t nbytes, off_t* offset)
{
  size_t n = nbytes;
  int error;
  acquire(&vdir->lock);
  if (vdir->type != VDIR) {
    n = -EINVAL;
    goto fail;
  }

  vdir->ops->readdir(vdir, buf, &n, offset);
fail:
  release(&vdir->lock);
  return n;
}

int
vfs_link(const char* name, const char* target)
{
  struct nameidata nd;
  struct vnode *vdir, *vn;
  int error;
  if ((error = namei(target, &nd)) != 0)
    return error;
  vn = nd.vn;
  acquire(&vn->lock);
  if (vn->type == VDIR) {
    error = -EISDIR;
    goto fail;
  }
  vput(nd.vdir);
  if ((error = namei(name, &nd)) != -ENOENT)
    return error;
  vdir = nd.vdir;
  acquire(&vdir->lock);
  error = vdir->ops->link(vdir, vn, &nd.cn);
  release(&vdir->lock);
fail:
  if (nd.vdir)
    vput(nd.vdir);
  vrele(vn);
  return error;
}

int
vfs_readlink_locked(struct vnode* vn, char* linkbuf)
{
  int error = 0;
  if (vn->type != VLNK)
    return -EINVAL;
  error = vn->ops->readlink(vn, linkbuf);
  return error;
}

/*
  Read the symbolic link in vnode vn
*/
int
vfs_readlink(struct vnode* vn, char* linkbuf)
{
  int error = 0;
  acquire(&vn->lock);

  if (vn->type != VLNK) {
    error = -EINVAL;
    goto fail;
  }
  error = vn->ops->readlink(vn, linkbuf);
fail:
  release(&vn->lock);
  return error;
}

/*
  Create a symbolick link that points at 'link'
*/
int
vfs_symlink(const char* name, const char* target)
{
  int error;
  struct nameidata nd;
  struct vnode* vdir;
  if ((error = namei(name, &nd)) != -ENOENT) {
    if (nd.vdir)
      vput(nd.vdir);
    if (nd.vn)
      vput(nd.vn);
    return error;
  }
  vdir = nd.vdir;
  acquire(&vdir->lock);
  vdir->ops->symlink(vdir, target, &nd.cn);
  vrele(vdir);
  return error;
}

/*
  Get root vnode of a mounted filesystem
  returns it locked
*/
int
vfs_root(struct vfs* vfs, struct vnode** root)
{
  int error = vfs->ops->root(vfs, root);
  acquire(&(*root)->lock);
  return error;
}

int
vfs_mount(char* mntpoint, const char* fstype)
{

  struct vnode* mntvnode; // vnode corresponding to mntpoint string
  struct vfsops* vfsops;
  struct vfs* vfs;
  struct nameidata n;
  int error;

  acquire(&vfslist.lock);

  for (vfsops = &vfslist.first; vfsops; vfsops = vfsops->next) {
    if (strcmp(vfsops->name, fstype) == 0)
      break;
  };
  if (vfsops == NULL) {
    error = -ENOENT;
    goto fail;
  }

  vfs = kalloc(sizeof *vfs);
  vfs->ops = vfsops;

  if (mntpoint) {

    if ((error = namei(mntpoint, &n)) != 0)
      goto fail2;
    mntvnode = n.vn;
    acquire(&mntvnode->lock);

    if (mntvnode->type != VDIR) {
      error = -ENOTDIR;
      goto fail2;
    }
    if (mntvnode->mountedhere) {
      error = -EBUSY;
      goto fail2;
    }
    vfs->mountpoint = mntvnode;
    mntvnode->mountedhere = vfs;

    vrele(mntvnode);
    vput(n.vdir);
  } else {
    vfs->flags |= MNT_ROOTFS;
    rootfs = vfs;
    rootfs->next = NULL;
  }

  // add vfs to the vfs list
  vfs->next = mountedlist;
  mountedlist = vfs;

  error = vfs->ops->mount(vfs);
fail:
  release(&vfslist.lock);
  return error;
fail2:
  if (n.vn)
    vput(n.vn);
  if (n.vdir)
    vput(n.vdir);
  release(&vfslist.lock);
  return error;
}

int
vfs_unmount(const char* name)
{
  int error;
  struct nameidata n;
  struct vnode *mntvnode, *vn;
  struct vfs* vfs;
  if ((error = namei(name, &n)) != 0)
    goto fail;
  vn = n.vn;
  vfs = vn->vfs;

  acquire(&vn->lock);
  if ((vn->flags & VROOT) == 0) {
    vrele(vn);
    return -EINVAL;
  }
  // can't unmount root filesystem
  if (vfs->flags & MNT_ROOTFS) {
    vrele(vn);
    return -EINVAL;
  }
  vrele(vn);

  error = vfs->ops->unmount(vfs);
  if (error != 0)
    return error;

  mntvnode = vfs->mountpoint;
  acquire(&mntvnode->lock);
  mntvnode->mountedhere = NULL;
  release(&mntvnode->lock);

  vput(n.vdir);
  return 0;
fail:
  if (n.vn)
    vput(n.vn);
  if (n.vdir)
    vput(n.vdir);
  return error;
}

int
vfs_unlink(const char* name)
{
  struct nameidata nd;
  struct vnode *vn, *vdir;
  int error;
  if ((error = namei(name, &nd)) != 0) {
    if (nd.vn)
      vput(nd.vn);
    if (nd.vdir)
      vput(nd.vdir);
    return error;
  }
  vn = nd.vn;
  vdir = nd.vdir;
  acquire(&vn->lock);
  if (vdir != vn)
    acquire(&vdir->lock);
  if (vn->flags & VROOT) {
    error = -EBUSY;
  } else {
    error = vdir->ops->remove(vdir, vn, &nd.cn);
  }

  // equal if name was for example "/."
  if (vdir != vn)
    vrele(vdir);
  else
    vput(vdir);
  vrele(vn);
  return error;
}

int
vfs_read(struct vnode* vn, void* buf, off_t offset, size_t count)
{
  int error;
  acquire(&vn->lock);
  if (vn->type == VDIR) {
    error = -EISDIR;
    goto fail;
  }
  error = vn->ops->read(vn, buf, offset, count);
fail:
  release(&vn->lock);
  return error;
}

int
vfs_write(struct vnode* vn, void* buf, off_t offset, size_t count)
{
  int error;
  acquire(&vn->lock);
  if (vn->type == VDIR) {
    error = -EISDIR;
    goto fail;
  }
  error = vn->ops->write(vn, buf, offset, count);
fail:
  release(&vn->lock);
  return error;
}

int
vfs_open(const char* name, struct vnode** vpp, enum vtype type, int flags)
{
  struct nameidata nd;
  struct vnode *vn, *vdir;
  int error;
  if (flags & O_CREATE) {
    if ((error = namei(name, &nd)) != -ENOENT) {
      if (nd.vdir)
        vput(nd.vdir);
      if (nd.vn)
        vput(nd.vn);
      return error;
    }
    vdir = nd.vdir;
    acquire(&vdir->lock);
    error = vdir->ops->create(vdir, &nd.vn, &nd.cn, type);
    vrele(vdir);
    if (error != 0)
      return error;
    *vpp = nd.vn;
  } else {
    error = namei(name, &nd);
    if (nd.vdir)
      vput(nd.vdir);
    if (error == 0)
      *vpp = nd.vn;
  }
  return error;
}

/*
  Create a file in parent directory
  Returns locked file in **vpp

  - inside the function,
    the parent gets locked and unlocked
*/
int
vfs_create(const char* name, struct vnode** vpp, enum vtype type)
{
  int error = vfs_open(name, vpp, type, O_CREATE);
  return error;
}

/*
  Bug: if you do /shit/fuck, and neither of them exist,
       it still tries to create a file
*/

int
vfs_mkdir(const char* name, struct vnode** vpp)
{
  struct nameidata n;
  struct vnode* vdir;
  int error;
  if (*name == 0)
    return -EINVAL;
  if ((error = namei(name, &n)) != -ENOENT) {
    if (n.vdir)
      vput(n.vdir);
    if (n.vn)
      vput(n.vn);
    return error ? error : -EEXIST;
  }
  vdir = n.vdir;
  acquire(&vdir->lock);
  if (vdir->type != VDIR) {
    error = -ENOTDIR;
    goto fail;
  }
  error = vdir->ops->mkdir(vdir, vpp, &n.cn);
fail:
  vrele(vdir);
  return error;
}

int
vfs_rmdir(const char* name)
{
  struct nameidata nd;
  struct vnode *parent, *vdir;
  int error;
  if ((error = namei(name, &nd)) != 0) {
    if (nd.vn)
      vput(nd.vn);
    if (nd.vdir)
      vput(nd.vdir);
    return error;
  }
  vdir = nd.vn;
  parent = nd.vdir;
  acquire(&vdir->lock);

  // "." is not allowed
  if (parent == vdir) {
    error = -EINVAL;
    goto fail;
  } else {
    acquire(&parent->lock);
  }
  if (vdir->type != VDIR) {
    error = -ENOTDIR;
    goto fail;
  }
  // the root of a filesystem cant be deleted
  if (vdir->flags & VROOT) {
    error = -EBUSY;
    goto fail;
  }
  error = parent->ops->rmdir(parent, vdir, &nd.cn);
fail:
  if (parent != vdir)
    vrele(parent);
  else
    vput(parent);
  vrele(vdir);
  return error;
}

/*
  Close a vnode by decrementing it's
  refcount, if the refcount reaches 0
  the vnode will be freed

  in: lock held
  out: lock released
*/
int
vfs_close(struct vnode* vn)
{
  if (trylock(&vn->lock) == 0)
    acquire(&vn->lock);

  vn->ops->close(vn);
  vn->refcount--;

  if (vn->refcount == 0) {
    vn->ops->inactive(vn);
    vn->data = NULL;
    kfree(vn);
  } else
    release(&vn->lock);

  return 0;
}

int
printdir(struct vnode* vdir)
{
  char buf[4096];
  off_t offset = 0;
  struct dirent* d = buf;
  ssize_t count = vfs_readdir(vdir, d, 4096, &offset);
  print("\n");
  for (ssize_t i = 0; i < count; i += d->reclen, d = (char*)d + d->reclen) {
    print(d->name);
    print("\t");
  }
}

/* VFS plan

 Mounting a file system means to make the new filesystem
 available somewhere in the directory tree.
 mounting is done by attaching the root of the new filesystem
 to a mount point, the mount point is a directory in the directory tree



 - file systems are manipulated through the vfs struct

 - each mounted vfs is in a linked list, and the first vfs
   is always the root

 - vfs struct has a private_data field that points at
   file system dependent data

 - mount() takes in the fs type, the mount point and fs specific data


  when mount() is called, the VFS looks up the vnode for the mount point,
  if vfs_mount() succeeds, the vfs is added to the linked list,
  and the vfs_nodecovered is set to the vnode for the mount point

 - the root vnode for a vfs can be obtained with vfs_root()

 - in the vnode struct, the v_vfsp points to the vfs of the vnode

 - if a vnode is a mount point, the vfs_mountedhere points to the vfs
  that is mounted

  the file_system_type struct has a function
  for obtaining a superblock


 - the superblock struct contains:
     - file_system_type struct
     - root dentry
     - list of all inodes
     - pointer to fs specific info

 superblock operations contain functions for getting info
 about an inode struct
    - alloc_inode(), destroy_inode(), read_inode()


 - during system initialization, the register_filesystem()
   is called for every fs that needs to be registered

 - register_filesystem() adds the file_system_type struct
   to the linked list


 linux mount first gets the correct fs struct from the fs list
 then it uses fs->get_sb() to get the superblock of a file system
 then it sets the vfs->sb of the new mount, to the superblock
 then it sets the vfs->root to the root dentry
 of the file system, it gets the root from the superblock


 */
