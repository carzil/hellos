#include "vfs.h"
#include "common.h"
#include "errno.h"

struct fs_operations* root_fs = NULL;

int vfs_read(struct file* f, void* buf, size_t sz) {
    int ret = vfs_read_at(f, f->pos, buf, sz);
    if (ret > 0) {
        f->pos += ret;
    }
    return ret;
}

int vfs_read_at(struct file* f, size_t offset, void* buf, size_t sz) {
    if (!(f->flags & VFS_FILE_READABLE)) {
        return -EBADF;
    }
    return f->ops->read(f, offset, buf, sz);
}

int vfs_open(struct file* f, const char* name, int flags) {
    f->flags = flags;
    f->fs = root_fs;
    return root_fs->open(root_fs, f, name, flags);
}

int vfs_close(struct file* f) {
    return f->ops->close(f);
}

void vfs_set_root(struct fs_operations* fs) {
    root_fs = fs;
}

void vfs_init() {
}

