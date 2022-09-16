



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
  struct vnode *vnode;
  struct ramnode *nextnode; // list of all ramnodes
  enum vtype type;

  union {
    
    struct {
      struct ramdentry *dentry;
      struct ramnode *parent;
    } dir;

    struct {
      
    } reg;
  };
};


