#pragma once

enum {
  VFS_TYPE_FILE,
  VFS_TYPE_DIR
};
struct vfs_stat {
  int type;
};
