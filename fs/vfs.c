
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

#include "std/string.h"
#include "franklin/fs/vfs.h"
#include "ramfs.h"


void *kalloc(int);

struct vfsops ram_ops;

void init_rootfs()
{
  vfslist.first = ram_ops;
  init_lock(&vfslist.lock);
};

struct vfs *rootfs;


/*
 find vnode that corresponds to path
 vfslist.lock has to be held

 return: locked vnode or NULL
*/
struct vnode*
lookup(struct componentname *path)
{

  struct vfs *vfs;
  struct vnode *parent, *vn;
  size_t i;
  char *str = path->nm;

  if (*str++ == '/') {
    vfs = rootfs;
  }
  vfs_root(vfs, &vn);

  // enter loop with parent->lock held
  // loop until pathname string has ended
  for (;;) {
    parent = vn;
    
    path->nm = str;
    str = strchr(str, '/');
    i = str - path->nm;
    if (*str == '/')
      str++; // skip '/'
    
    // end of pathname string
    if ((path->len = i) == 0)
      return parent;

    if (parent->type != VDIR)
      panic("lookup, parent->type");

    // return vn locked
    int z = vfs_lookup(parent, &vn, path);
    release(&parent->lock);

    if (z < 0)
      return NULL; 

    // if vnode is a mountpoint, get the root
    // vnode of the mounted filesystem
    // and release the lock on the mountpoint vnode
    if (vn->mountedhere) {
      vfs = vn->mountedhere;
      vfs_close(vn);
      vfs_root(vfs, &vn);
    }
  }
  
  return vn;
};



/*
  Lookup a vnode in a directory
  - vnode is returned locked
*/
int
vfs_lookup(struct vnode *vdir, struct vnode **vpp,
	   struct componentname *path)
{
  int z = vdir->ops->lookup(vdir, vpp, path);
  acquire(&(*vpp)->lock);
  return z;
}

/*
  Get root vnode of a mounted filesystem
  returns it locked
*/
int
vfs_root(struct vfs *vfs, struct vnode **root)
{
  vfs->ops->root(vfs, root);
  acquire(&(*root)->lock);
}


int
vfs_mount(char *mntpoint, const char *fstype)
{
  
  struct vnode *mntvnode; // vnode corresponding to mntpoint string
  struct vfsops *vfsops;
  struct vfs *vfs;
  struct componentname path;

  acquire(&vfslist.lock);

  for (vfsops = &vfslist.first; vfsops; vfsops = vfsops->next) {
    if (strcmp(vfsops->name, fstype) == 0)
      break;
  };

  if (vfsops == NULL) {
    panic("vfs_mount: no vfsops");
  }


  vfs = kalloc(sizeof *vfs);
  vfs->ops = vfsops;
  
  if (mntpoint) {
    path.nm = strdup(mntpoint);
    path.len = strlen(mntpoint);

    // return mountpoint vnode locked
    mntvnode = lookup(&path);
    
    if (mntvnode == NULL) {
      print(path.nm);
      panic("\n mount: mnt vnode NULL");      
    }
    if (mntvnode->type != VDIR)
      panic("mount: mountpoint type");
    
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

  release(&vfslist.lock);
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
  struct componentname cname = {
				  .nm = strdup(parent),
				  .len = strlen(parent),
  };
  
  vdir = lookup(&cname);
  if (vdir == NULL) {
    print("vfs_create: parent not found.. ");
    panic(parent);
  }
    
  if (vdir->type != VDIR) {
    print("vfs_create: type");
    return -1;
  }

  vdir->ops->create(vdir, vpp, name, type);
  
  vfs_close(vdir);
}

int
vfs_mkdir(const char *parent, const char *name, struct vnode **vpp, enum vtype type)
{
  vfs_create(parent, name, vpp, type);
}


int
vfs_vget(struct vfs *vfs, struct vnode **vpp, ino_t ino)
{
  if (vfs)
    vfs->ops->vget(vfs, vpp, ino);
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
    panic("vfs_close, lock");

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

