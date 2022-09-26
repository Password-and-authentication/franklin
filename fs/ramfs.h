#ifndef _RAMFS_
#define _RAMFS_


#include "../include/franklin/spinlock.h"
#include <stddef.h>


#define SEQ_DOT 0
#define SEQ_DOTDOT 1
#define SEQ_EOF 2
#define SEQ_START 3

struct ramvfs {
  struct ramnode *root;
  struct ramnode *nodes; // list of all ram nodes
};


// /link main.c


struct ramdentry {
  struct ramdentry *next; // list of dentries in the parent dir
  struct ramnode *node;

  // sequence number is used to identify a dentry in a directory
  uint64_t seq;
  char *name;
};


struct ramnode {
  struct ramnode *prev, *next;

  struct vnode *vnode; // may be null
  
  enum vtype type;
  lock ramlock;
  int linkcount;

  union {
    struct {
      struct ramdentry *dentry;
      struct ramnode *parent;
      uint64_t lastseq;
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
