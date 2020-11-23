#pragma once
#include <stddef.h>
#include <stdint.h>

#include "pci.h"
#include "sched.h"

struct prdt_entry {
    uint32_t buf_addr;
    uint16_t byte_count;
    uint16_t mark;
};

struct ata_dev {
    uint16_t io_base;
    uint16_t io_ctrl_base;
    uint16_t bar4;
    void* dma_buf;
    struct prdt_entry* prdt;
};

#define ATA_REG(dev, offset) ((dev)->io_base + offset)
#define ATA_DATA_REG(dev)       ATA_REG(dev, 0)
#define ATA_ERR_REG(dev)        ATA_REG(dev, 1)
#define ATA_FEAT_REG(dev)       ATA_REG(dev, 1)
#define ATA_SECCOUNT_REG(dev)   ATA_REG(dev, 2)
#define ATA_LBA_LO_REG(dev)     ATA_REG(dev, 3)
#define ATA_LBA_MID_REG(dev)    ATA_REG(dev, 4)
#define ATA_LBA_HI_REG(dev)     ATA_REG(dev, 5)
#define ATA_DRIVE_REG(dev)      ATA_REG(dev, 6)
#define ATA_COMMAND_REG(dev)    ATA_REG(dev, 7)
#define ATA_STATUS_REG(dev)     ATA_REG(dev, 7)
#define ATA_ALT_STATUS_REG(dev) ((dev)->io_ctrl_base + 0)
#define ATA_CONTROL_REG(dev)    ((dev)->io_ctrl_base + 0)

#define BMR_COMMAND_REG(dev)    ((dev)->bar4)
#define BMR_STATUS_REG(dev)     ((dev)->bar4 + 2)
#define BMR_PRDT_REG(dev)       ((dev)->bar4 + 4)

#define ATA_ERR_AMNF     (1 << 0)
#define ATA_ERR_TKZNF    (1 << 1)
#define ATA_ERR_ABRT     (1 << 2)
#define ATA_ERR_MCR      (1 << 3)
#define ATA_ERR_IDNF     (1 << 4)
#define ATA_ERR_MC       (1 << 5)
#define ATA_ERR_UNC      (1 << 6)
#define ATA_ERR_BBK	     (1 << 7)

#define ATA_STATUS_ERR   (1 << 0)
#define ATA_STATUS_DRQ   (1 << 3)
#define ATA_STATUS_BSY   (1 << 7)

#define ATA_CMD_IDENTIFY      0xec
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_READ_DMA      0xc8

#define ATA_SELECT_MASTER 0xa0

void ata_init();
int ata_read(void* buf, int lba, int seccount);
void ata_tick();
