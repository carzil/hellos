#include "ata.h"
#include "io.h"
#include "panic.h"
#include "irq.h"
#include "apic.h"
#include "sched.h"
#include "common.h"
#include "timer.h"

struct ata_req {
    void* buf;
    int lba;
    int sectors_count;
    struct ata_dev* dev;
    struct ata_req* next;
    struct task* task;
};

struct ata_req* req_queue = NULL;
static struct ata_dev primary_master;

static inline void ata_wait(struct ata_dev* dev) {
    inb(ATA_ALT_STATUS_REG(dev));
    inb(ATA_ALT_STATUS_REG(dev));
    inb(ATA_ALT_STATUS_REG(dev));
    inb(ATA_ALT_STATUS_REG(dev));
}

void ata_issue_read_sectors(struct ata_req* req) {
    BUG_ON(req->sectors_count >= 256);

    outb(ATA_DRIVE_REG(req->dev), ATA_SELECT_MASTER | 0x40 | ((req->lba >> 24) & 0x0f));
    outb(ATA_FEAT_REG(req->dev), 0);
    outb(ATA_SECCOUNT_REG(req->dev), req->sectors_count);
    outb(ATA_LBA_LO_REG(req->dev), req->lba & 0xff);
    outb(ATA_LBA_MID_REG(req->dev), (req->lba >> 8) & 0xff);
    outb(ATA_LBA_HI_REG(req->dev), (req->lba >> 16) & 0xff);
    outb(ATA_COMMAND_REG(req->dev), ATA_CMD_READ_SECTORS);
}

void ata_process_request() {
    BUG_ON_NULL(req_queue);

    struct ata_req* req = req_queue;

    BUG_ON(!req->buf);
    BUG_ON(req->sectors_count == 0);

    uint16_t* buf = req->buf;
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_DATA_REG(req->dev));
    }
    ata_wait(req->dev);
    req->buf += 256 * 2;
    req->sectors_count--;
    if (req->sectors_count == 0) {
        req->task->state = TASK_RUNNING;
        req_queue = req_queue->next;
        if (req_queue) {
            ata_issue_read_sectors(req_queue);
        }
    }
}

void ide_irq1() {
    ata_process_request();
    apic_eoi();
}

void ide_irq2() {
    apic_eoi();
}

uint8_t ata_poll(struct ata_dev* dev) {
    uint8_t drq = 0;
    uint8_t err = 0;
    uint8_t bsy = 1;
    while (bsy || (!drq && !err)) {
        uint8_t status = inb(ATA_ALT_STATUS_REG(dev));
        drq = status & ATA_STATUS_DRQ;
        err = status & ATA_STATUS_ERR;
        bsy = status & ATA_STATUS_BSY;
    }
    if (err) {
        return inb(ATA_ERR_REG(dev));
    }
    return 0;
}

int ata_read(void* buf, int lba, int seccount) {
    BUG_ON(!current);

    struct ata_req req = {
        .buf = buf,
        .lba = lba,
        .sectors_count = seccount,
        .next = NULL,
        .dev = &primary_master,
        .task = current,
    };

    DISABLE_IRQ_BEGIN
        current->state = TASK_DISK_READ;

        if (!req_queue) {
            req_queue = &req;
            ata_issue_read_sectors(&req);
        } else {
            struct ata_req* ptr = req_queue;
            while (ptr->next) {
                ptr = ptr->next;
            }
            ptr->next = &req;
        }
    DISABLE_IRQ_END

    reschedule();
    return 0;
}

static void ata_init_dev(struct ata_dev* dev) {
    // Init primary master only.
    dev->io_base = 0x1f0;
    dev->io_ctrl_base = 0x3f6;
    outb(ATA_DRIVE_REG(dev), ATA_SELECT_MASTER); // Select master.
    outb(ATA_CONTROL_REG(dev), 1 << 1);
    outb(ATA_SECCOUNT_REG(dev), 0);
    outb(ATA_LBA_LO_REG(dev), 0);
    outb(ATA_LBA_MID_REG(dev), 0);
    outb(ATA_LBA_HI_REG(dev), 0);
    outb(ATA_COMMAND_REG(dev), ATA_CMD_IDENTIFY);
    if (inb(ATA_ALT_STATUS_REG(dev)) == 0) {
        panic("cannot init ATA primary master");
    }

    uint8_t lba_lo = inb(ATA_LBA_LO_REG(dev));
    uint8_t lba_hi = inb(ATA_LBA_HI_REG(dev));
    if (lba_lo != 0 || lba_hi != 0) {
        panic("cannot init ATA primary master");
    }

    uint8_t err = ata_poll(dev);
    if (err) {
        panic("error while ATA primary master init");
    }

    for (int i = 0; i < 256; i++) {
        inw(ATA_DATA_REG(dev));
    }
    outb(ATA_CONTROL_REG(dev), 0);
}

void ata_init() {
    struct pci_dev pci_dev;
    if (!pci_find_device(&pci_dev, PCI_CLASS_MASS_STORAGE_CONTROLLER, PCI_SUBCLASS_IDE)) {
        panic("no IDE controller found");
    }

    ata_init_dev(&primary_master);
}

void ata_tick() {
    // ...
}
