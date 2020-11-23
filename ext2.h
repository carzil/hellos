#include "defs.h"
#include "vfs.h"

#include <stddef.h>
#include <stdint.h>

void init_ext2();

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

#define EXT2_SIGNATURE 0xef53

struct ext2_fs {
    struct fs_operations ops;
    struct ext2_superblock superblock;
    struct ext2_block_group_descriptor* bgd;
    void* tmp;
};


#define EXT2_S_IFDIR 0x4000
