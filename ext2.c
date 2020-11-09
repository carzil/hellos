#include "ext2.h"
#include "panic.h"
#include "ata.h"
#include "common.h"

struct ext2_extended_superblock {
    uint32_t first_inode;
    uint16_t inode_size_bytes;
    uint16_t block_group;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t ro_features;
    uint8_t  fs_id[16];
    uint8_t  volume_name[16];
    uint8_t  last_mountpoint[64];
    uint32_t compression;
    uint8_t  file_preallocate_blocks;
    uint8_t  dir_preallocate_blocks;
    uint16_t unused0;
    uint8_t  journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_dev;
    uint32_t orphan_inodes_head;
    uint8_t  unused1[788];
} __attribute__((packed));

struct ext2_superblock {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t root_reserved_blocks;
    uint32_t blocks_unallocated;
    uint32_t inodes_unallocated;
    uint32_t superblock_block;
    uint32_t block_size_log;
    uint32_t fragment_size_log;
    uint32_t blocks_per_group;
    uint32_t fragments_per_block_group;
    uint32_t inodes_per_block_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t mounts_since_fsck;
    uint16_t mounts_before_fsck;
    uint16_t signature;
    uint16_t fs_state;
    uint16_t error_detection;
    uint16_t rev_minor;
    uint32_t last_fsck_time;
    uint32_t force_fsck_time_interval;
    uint32_t os_id;
    uint32_t rev;
    uint16_t root_uid;
    uint16_t root_gid;
    struct ext2_extended_superblock extended;
} __attribute__((packed));

struct ext2_inode {
    uint16_t mode;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links;
    uint32_t i_blocks;
    uint32_t flags;
    uint32_t os_specific1;
    uint32_t direct_ptr[12];
    uint32_t single_indirect_ptr;
    uint32_t doubly_indirect_ptr;
    uint32_t triple_indirect_ptr;
    uint32_t generation_number;
    uint32_t acl;
    union {
        uint32_t size_hi;
        uint32_t dir_acl;
    };
    uint32_t fragment_block_address;
    uint8_t os_specific2[12];
};

struct ext2_block_group_descriptor {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t unallocated_blocks;
    uint16_t unallocated_inodes;
    uint16_t total_directories;
    uint8_t  unused[14];
} __attribute__((packed));

struct ext2_dir_entry_head {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[0];
} __attribute__((packed));

#define EXT2_INODE_TYPE_FIFO      0x1000
#define EXT2_INODE_TYPE_CHAR_DEV  0x2000
#define EXT2_INODE_TYPE_DIR       0x4000
#define EXT2_INODE_TYPE_BLOCK_DEV 0x6000
#define EXT2_INODE_TYPE_REG_FILE  0x8000
#define EXT2_INODE_TYPE_SYMLINK   0xa000
#define EXT2_INODE_TYPE_SOCKET    0xc000

#define EXT2_SIGNATURE 0xef53

struct ext2_fs {
    struct ext2_superblock superblock;
    struct ext2_block_group_descriptor* bgd;
    void* tmp;
};

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

    struct ext2_inode root_inode;
    ret = ext2_read_inode(fs, 2, &root_inode);
    if (ret < 0) {
        return ret;
    }

    uint32_t max_blocks = root_inode.i_blocks / (2 << fs->superblock.block_size_log);

    for (uint32_t i = 0; i < max_blocks; i++) {
        ret = ext2_read_block(fs, root_inode.direct_ptr[i], fs->tmp);
        if (ret < 0) {
            return ret;
        }

        struct ext2_dir_entry_head* head = fs->tmp;
        while (head < fs->tmp + ext2_block_size(fs)) {
            printk(" * entry name='%s', inode=%d\n", &head->name, head->inode);
            head = (uint8_t*)head + head->rec_len;
        }
    }

    return 0;

}

void init_ext2() {
    BUG_ON(sizeof(struct ext2_superblock) != 1024);
    BUG_ON(sizeof(struct ext2_block_group_descriptor) != 32);
    BUG_ON(sizeof(struct ext2_inode) < 128);
    ext2_init(&root_ext2);
}
