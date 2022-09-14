


struct ramvfs {
  struct ramnode *root;
};

struct ramdentry {
  struct ramdentry *next;
  struct ramnode *node;
  
  char *name;
};


struct ramnode {
  struct vnode *vnode;
  enum vtype type;

  union {
    
    struct {
      struct ramdentry *dentry;
      struct ramnode *parent;
    } dir;

    struct {
      
    } reg;
  }
};
