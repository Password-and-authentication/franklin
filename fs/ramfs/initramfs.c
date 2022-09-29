#include "../limine/limine.h"
#include "franklin/fs/vfs.h"
#include "franklin/misc.h"
#include <stddef.h>

static volatile struct limine_module_request module_request = {
  .id = LIMINE_MODULE_REQUEST,
  .revision = 0,
};

/*
  A tarball is a file that contains other files,

  Each file has a tar header with a size of 512 bytes
  The length of a file is rounded up to 512 bytes
*/
struct tar
{
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char type;
  char link[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
};

#define TAR_HEADER_SIZE 512

#define TAR_FILETYPE_REG '0'
#define TAR_FILETYPE_HARDLINK '1'
#define TAR_FILETYPE_SYMLINK '2'
#define TAR_FILETYPE_DIR '3'

// Some of the tar header fields are in octal because
// a long time ago the char size was 6 and not 8
static inline uint64_t
oct2int(const char* oct, size_t len)
{
  uint64_t val = 0;
  while (*oct && len--) {
    val = val * 8 + (*oct++ - '0');
  }
  return val;
}

/*
  The first module is a tarball that contains different files
  initramfs() goes through the tarball and copies the files into the vfs
*/
void
initramfs(void)
{
  struct limine_module_response* modules = module_request.response;
  struct limine_file* module;
  struct tar* file;
  struct vnode* vn;
  uint64_t size;
  char name[100];

  if (modules == NULL || modules->module_count == 0)
    panic("initramfs: module");

  module = modules->modules[0];

  file = module->address;

  while (strncmp(file->magic, "ustar", 5) == 0) {

    name[0] = '/';
    strcpy(name + 1, file->name);
    size = oct2int(file->size, sizeof file->size);

    switch (file->type) {
      case TAR_FILETYPE_REG: {
        vfs_create(name, &vn, VREG);
        if (vn == NULL) {
          panic("initramfs: file create");
        }
        vfs_write(vn, (char*)file + TAR_HEADER_SIZE, 0, size);
      };
      case TAR_FILETYPE_SYMLINK: {
      };
      case TAR_FILETYPE_DIR: {
      }
    }
    vfs_close(vn);

    file = (char*)file + TAR_HEADER_SIZE + ALIGN_UP(size, TAR_HEADER_SIZE);
  }
}
