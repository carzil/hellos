#include "ext2.h"
#include "panic.h"
#include "ata.h"
#include "common.h"
#include "errno.h"

struct ext2_fs root_ext2;

static inline uint32_t ext2_block_size(struct ext2_fs* fs) {
    return 1024 << fs->superblock.block_size_log;
}

static int ext2_read_block(struct ext2_fs* fs, uint32_t block, void* buf) {
    uint32_t sectors_per_block = ext2_block_size(fs) / 512;
    memset(buf, '\0', ext2_block_size(fs));
    return ata_read(buf, sectors_per_block * block, sectors_per_block);
}

static int ext2_read_inode(struct ext2_fs* fs, uint32_t inode, struct ext2_inode* buf) {
    uint32_t group = (inode - 1) / fs->superblock.inodes_per_block_group;
    uint32_t inode_index = (inode - 1) % fs->superblock.inodes_per_block_group;
    uint32_t inode_size = 128;
    if (fs->superblock.rev >= 1) {
        inode_size = fs->superblock.extended.inode_size_bytes;
    }
    uint32_t block = fs->bgd[group].inode_table + (inode_index * inode_size) / ext2_block_size(fs);
    int ret = ext2_read_block(fs, block, fs->tmp);
    if (ret < 0) {
        return ret;
    }
    uint32_t pos = (inode_index * inode_size) % ext2_block_size(fs);
    memcpy(buf, (uint8_t*)fs->tmp + pos, sizeof(*buf));
    return 0;
}

static const char* skip_slashes(const char* name) {
    const char* res = name;
    while (*res == '/') {
        res++;
    }
    return res;
}

static int compare_dirent_name(const char* path, struct ext2_dir_entry_head* head) {
    size_t pos = 0;
    while (pos < head->name_len) {
        if (path[pos] == '/' || path[pos] != head->name[pos]) {
            return 0;
        }
        pos++;
    }
    return 1;
}

static uint32_t ext2_max_blocks(struct ext2_fs* fs, struct ext2_inode* info) {
    return info->i_blocks / (2 << fs->superblock.block_size_log);
}

int ext2_find_inode(struct ext2_fs* fs, const char* path, struct ext2_inode* info, uint32_t* curr_inode) {
    if (path[0] != '/') {
        return -EINVAL;
    }
    path = skip_slashes(path);

    *curr_inode = 2;

    while (*path) {
        // Locate current inode.
        int ret = ext2_read_inode(fs, *curr_inode, info);
        if (ret < 0) {
            return ret;
        }

        uint32_t max_blocks = ext2_max_blocks(fs, info);
        int found = 0;
        for (uint32_t i = 0; i < max_blocks && !found; i++) {
            ret = ext2_read_block(fs, info->direct_ptr[i], fs->tmp);
            if (ret < 0) {
                return ret;
            }

            struct ext2_dir_entry_head* head = fs->tmp;
            while (head < fs->tmp + ext2_block_size(fs)) {
                if (head->inode) {
                    if (compare_dirent_name(path, head)) {
                        found = 1;
                        *curr_inode = head->inode;
                        path += head->name_len;
                        path = skip_slashes(path);
                        break;
                    }
                }
                head = (uint8_t*)head + head->rec_len;
            }
        }

        if (!found) {
            return -ENOENT;
        }
    }

    return ext2_read_inode(fs, *curr_inode, info);
}

int ext2_read(struct file* file, size_t offset, void* buf, size_t sz) {
    uint32_t inode = (uint32_t)file->priv;
    struct ext2_fs* fs = (struct ext2_fs*)file->fs;
    struct ext2_inode info;
    int ret = ext2_read_inode(fs, inode, &info);
    if (ret < 0) {
        return ret;
    }

    uint32_t bs = ext2_block_size(fs);
    BUG_ON(offset % bs != 0);
    BUG_ON(sz > ARRAY_SIZE(info.direct_ptr) * bs);
    uint32_t max_blocks = ext2_max_blocks(fs, &info);
    uint32_t block = offset / bs;
    while (sz > bs && block < max_blocks) {
        ret = ext2_read_block(fs, info.direct_ptr[block], buf);
        if (ret < 0) {
            return ret;
        }
        block++;
        buf += bs;
    }

    if (sz > 0 && block < max_blocks) {
        void* tmp = kalloc();
        ret = ext2_read_block(fs, info.direct_ptr[block], tmp);
        memcpy(buf, tmp, sz);
        kfree(tmp);
    }

    return 0;
}

int ext2_close(struct file* f) {
    return 0;
}

struct file_operations ext2_file_ops = {
    .read = ext2_read,
    .close = ext2_close,
};

int ext2_open(void* priv, struct file* file, const char* name, int flags) {
    struct ext2_fs* fs = (struct ext2_fs*)priv;
    struct ext2_inode info;
    uint32_t inode = 0;
    int ret = ext2_find_inode(fs, name, &info, &inode);
    if (ret < 0) {
        return ret;
    }

    if (info.mode & EXT2_S_IFDIR) {
        return -EISDIR;
    }

    file->priv = inode;
    file->ops = &ext2_file_ops;
    return 0;
}

static int ext2_init(struct ext2_fs* fs) {
    // Superblock is always located at 1024 byte offset (sector LBA = 2).
    ata_read(&fs->superblock, 2, 2);
    if (fs->superblock.signature != EXT2_SIGNATURE) {
        panic("invalid signature for ext2: 0x%x", (uint32_t)fs->superblock.signature);
    }

    uint32_t sectors_per_block = (1024 << fs->superblock.block_size_log) / 512;
    uint32_t groups = fs->superblock.total_blocks / fs->superblock.blocks_per_group;
    BUG_ON(PAGE_SIZE / sizeof(struct ext2_block_group_descriptor) < groups);
    BUG_ON(PAGE_SIZE < ext2_block_size(fs));
    fs->bgd = kalloc();
    fs->tmp = kalloc();
    int ret = ext2_read_block(fs, 1, fs->bgd);
    if (ret < 0) {
        return ret;
    }
    printk("found valid ext2, rev=%d.%d, sectors_per_block=%d, groups=%d\n", fs->superblock.rev, fs->superblock.rev_minor, sectors_per_block, groups);

    root_ext2.ops.open = ext2_open;
    vfs_set_root(&root_ext2.ops);

    return 0;
}

void init_ext2() {
    BUG_ON(sizeof(struct ext2_superblock) != 1024);
    BUG_ON(sizeof(struct ext2_inode) < 128);
    BUG_ON(sizeof(struct ext2_block_group_descriptor) != 32);
    ext2_init(&root_ext2);
}
