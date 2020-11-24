#pragma once

#include <stddef.h>
#include <stdint.h>

struct file;

#define VFS_FILE_READABLE   (1 << 0)
#define VFS_FILE_WRITEABLE  (1 << 1)

struct file_operations {
    int (*read)(struct file* f, size_t offset, void* buf, size_t sz);
    int (*close)(struct file* f);
};

struct fs_operations {
    int (*open)(void* priv, struct file*, const char* name, int flags);
};

struct file {
    struct file_operations* ops;
    void* fs;
    void* priv;
    size_t pos;
    int flags;
};

int vfs_read(struct file* f, void* buf, size_t sz);
int vfs_read_at(struct file* f, size_t offset, void* buf, size_t sz);
int vfs_seek(struct file* f, size_t off);
int vfs_open(struct file*, const char* name, int flags);
void vfs_set_root(struct fs_operations* fs);
