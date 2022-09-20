#ifndef _RAMFS_
#define _RAMFS_


#include "../include/franklin/spinlock.h"
#include <stddef.h>


struct ramvfs {
  struct ramnode *root;
  struct ramnode *nodes; // list of all ram nodes
};


struct ramdentry {
  struct ramdentry *next; // list of dentries in the parent dir
  struct ramnode *node;
  
  char *name;
};


struct ramnode {
  struct vnode *vnode; // may be null
  
  struct ramnode *nextnode; // list of all ramnodes
  enum vtype type;
  lock ramlock;
  int linkcount;

  union {
    struct {
      struct ramdentry *dentry;
      struct ramnode *parent;
    } dir;

    struct {
      char *link;
    } lnk;

    struct {
      void *data;
      size_t size;
    } reg;
  };
};


// vnode 1 access ramnode 1 (dir)
// vnode 2 access ramnode 2 (reg)
// vnode 1 can access ramnode 2 through *nextnode

// 





#endif
