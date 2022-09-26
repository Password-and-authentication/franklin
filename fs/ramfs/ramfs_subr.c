#include "ramfs.h"

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
