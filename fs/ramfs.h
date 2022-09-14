



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
