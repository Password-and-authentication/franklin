
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

#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "std/string.h"
#include "franklin/fs/vfs.h"
#include "franklin/uio.h"
#include "ramfs.h"

struct vnode *lookup(struct componentname*);

void *kalloc(int);

struct vfsops ram_ops;

void init_rootfs()
{
  vfslist.first = ram_ops;
  init_lock(&vfslist.lock);
};

struct vfs *rootfs;


struct vnode *
namei(const char *name)
{

  struct vnode *vdir;
  struct componentname cname = {
				.nm = strdup(name),
				.len = strlen(name),
  };
  vdir = lookup(&cname);
  return vdir;
}


/*
 find vnode that corresponds to path
 vfslist.lock has to be held
 
 overall outline of lookup:
 
   1. set the vnode to the starting directory of the lookup (root or cwd)
   2. get the next component out of the full pathname
      if the component is 0, return the vnode
   if .. and crossing mountpoints and on mounted fs, find parent
   3. call the lookup routine of the parent
      if the returned vnode is a symbolic link, 
         call the readlink routine of the vnode and goto step 1
      if the returned vnode is a mountpoint,
         get the root of the mounted filesystem and goto step 2
      else goto step 2

 return: locked vnode or NULL
*/
struct vnode*
lookup(struct componentname *path)
{

  struct vfs *vfs;
  struct vnode *parent, *vn;
  struct uio uio = {
		    .offset = 0,
		    .buf = kalloc(100),
  };
  size_t i;
  char *str = path->nm;

 checkabs:
  if (*str == '/') {
    vfs = rootfs;
    while (*str == '/')
      str++;
  }
  vfs_root(vfs, &vn);

  // enter loop with parent->lock held
  // loop until pathname string has ended
  for (;;) {
    parent = vn;
    
    path->nm = str;
    str = strchr(str, '/');
    i = str - path->nm;
    while (*str == '/')
      *str++;
    
    // end of pathname string
    if ((path->len = i) == 0)
      return parent;

    if (parent->type != VDIR)
      return -ENOTDIR;


    // if .. and crossing mount points and on mounted fs, find parent
    if (strncmp(path->nm, "..", 2) == 0 && parent->flags & VROOT) {
      parent = parent->vfs->mountpoint;
    }
    
    // return vn locked
    int z = vfs_lookup(parent, &vn, path);
    release(&parent->lock);

    if (z < 0)
      return NULL;

    if (vn->type == VLNK) {
      vfs_readlink(vn, &uio);
      str = strdup(uio.buf);
      goto checkabs;
      
    } else if (vn->mountedhere) {
      // if vnode is a mountpoint, get the root
      // vnode of the mounted filesystem
      // and release the lock on the mountpoint vnode
      
      vfs = vn->mountedhere;
      vfs_close(vn);
      vfs_root(vfs, &vn);
    }

  }
  
  return vn;
};


int
vfs_readlink(struct vnode *vn, struct uio *uio)
{
  int error = vn->ops->readlink(vn, uio);
  return error;
}

int      // todo: dir to be type fd
vfs_symlink(const char *dir, const char *link, const char *name)
{

  struct vnode *vdir;
  vdir = namei(dir);
  if (vdir == NULL)
    return 0;
  if (vdir->type != VDIR)
    return -ENOTDIR;
  
  vdir->ops->symlink(vdir, link, name);
  vfs_close(vdir);
  return 0;
}

/*
  Lookup a vnode in a directory
  - vnode is returned locked
*/
int
vfs_lookup(struct vnode *vdir, struct vnode **vpp,
	   struct componentname *path)
{
  int error = vdir->ops->lookup(vdir, vpp, path);
  acquire(&(*vpp)->lock);
  return error;
}

/*
  Get root vnode of a mounted filesystem
  returns it locked
*/
int
vfs_root(struct vfs *vfs, struct vnode **root)
{
  int error = vfs->ops->root(vfs, root);
  acquire(&(*root)->lock);
  return error;
}


int
vfs_mount(char *mntpoint, const char *fstype)
{
  
  struct vnode *mntvnode; // vnode corresponding to mntpoint string
  struct vfsops *vfsops;
  struct vfs *vfs;
  struct componentname path;
  int error;

  acquire(&vfslist.lock);

  for (vfsops = &vfslist.first; vfsops; vfsops = vfsops->next) {
    if (strcmp(vfsops->name, fstype) == 0)
      break;
  };

  if (vfsops == NULL) {
    error = 0;
    goto fail;
  }


  vfs = kalloc(sizeof *vfs);
  vfs->ops = vfsops;
  
  if (mntpoint) {
    path.nm = strdup(mntpoint);
    path.len = strlen(mntpoint);

    // return mountpoint vnode locked
    mntvnode = lookup(&path);
    
    if (mntvnode == NULL) {
      error = 0;
      goto fail;
    }
    if (mntvnode->type != VDIR) {
      error = -ENOTDIR;
      goto fail;
    }

    mntvnode->flags |= VROOT;
    vfs->mountpoint = mntvnode;
    mntvnode->mountedhere = vfs;
    
     // release lock and decrement refcount
    vfs_close(mntvnode);
  } else {
    rootfs = vfs;
    rootfs->next = NULL;
  }

  // add vfs to the vfs list
  vfs->next = mountedlist;
  mountedlist = vfs;

  vfs->ops->mount(vfs);

 fail:
  release(&vfslist.lock);
}


#define O_CREATE 0x200

int    //parent should be type fd
vfs_open(const char *name, struct vnode **vpp,
	 enum vtype type, int flags)
{
  *vpp = namei(name);
  return 0;
}

/*
  Create a file in parent directory
  Returns locked file in **vpp

  - inside the function,
    the parent gets locked and unlocked
*/
int
vfs_create(const char *parent, const char *name, struct vnode **vpp, enum vtype type)
{
  struct vnode *vdir;
  vdir = namei(parent);
  if (vdir == NULL)
    return 0;
    
  if (vdir->type != VDIR)
    return -ENOTDIR;


  vdir->ops->create(vdir, vpp, name, type);
  
  vfs_close(vdir);
  return 0;
}

int
vfs_mkdir(const char *parent, const char *name, struct vnode **vpp, enum vtype type)
{
  int error = vfs_create(parent, name, vpp, type);
  return error;
}


int
vfs_vget(struct vfs *vfs, struct vnode **vpp, ino_t ino)
{
  int error;
  error =vfs->ops->vget(vfs, vpp, ino);
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
vfs_close(struct vnode *vn)
{
  if (trylock(&vn->lock) == 0)
    return -ENOLCK;

  vn->ops->close(vn);
  vn->refcount--;

  if (vn->refcount == 0) {
    vn->ops->inactive(vn);
    kfree(vn);
  } else
    release(&vn->lock);
  
  return 0;
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

