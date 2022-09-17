
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

void init_rootfs() {
  vfslist = ram_ops;
  
};

char *
strsep(const char **path, const char *ch)
{
  size_t i;
  char *s;
  
  for (i = 0; *path[i] != *ch; ++i)
    ;
  if (i == 0)
    return 0;
  
  s = strdup(*path);
  *path += i;
  return s;
}


struct vnode*
lookup(struct componentname *path) {

  struct vfs *vfs;
  struct vnode *parent, *vn;
  size_t i;

  if (*path->nm++ == '/') {
    vfs = rootfs;
  }
  vfs->ops->root(vfs, &parent);

  for (;;) {
    for (i = 0; path->nm[i] && path->nm[i] != '/'; ++i)
    ;
    if (i == 0)
      return vn;
    path->len = i;
    parent->ops->lookup(parent, &vn, path);
    
    if (vn->type != VDIR)
      return NULL;
    
    parent = vn;
    path->nm += path->len + 1;
  }
  
  return vn;
};

void
solve() {

}

int
vfs_mount(char *mntpoint, const char *fstype) {

  struct vnode *mntvnode; // vnode corresponding to mntpoint string
  struct vfsops *vfsops;
  struct vfs *vfs;
  struct componentname *path = kalloc(sizeof *path);
  
  if (mntpoint) {
    path->nm = strdup(mntpoint);
    path->len = strlen(mntpoint);
  }
  
  
  for (vfsops = &vfslist; vfsops; vfsops = vfsops->next) {
    if (strcmp(vfsops->name, fstype) == 0)
      break;
  };
  if (vfsops == NULL)
    return;

  vfs = kalloc(sizeof *vfs);
  vfs->ops = vfsops;
  path->nm = "/newdir/lmao.lol";

  if (mntpoint) {
    mntvnode = lookup(path);
    if (mntvnode == NULL)
      return;
    
    vfs->mountpoint = mntvnode;
    mntvnode->mountedhere = vfs;
  } else {
    rootfs = vfs;
    rootfs->next = NULL;
  }

  // add vfs to the vfs list
  vfs->next = mountedlist;
  mountedlist = vfs;

  vfs->ops->mount(vfs);
}



struct vnode*
getnewvnode(void) {

  struct vnode *vnode = kalloc(sizeof *vnode);

  vnode->type = VNON;
  vnode->refcount = 0;
  vnode->data = NULL;
  return vnode;
};


void
vfs_root(struct vfs *vfs) {
  return vfs->ops->root();
};




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

