#include "pci.h"
#include "io.h"
#include "panic.h"

#define PCI_ADDRESS_PORT        0xcf8
#define PCI_DATA_PORT           0xcfc

#define PCI_ADDRESS(bus, dev, func, offset) (((uint32_t)(bus) << 16) | ((uint32_t)(dev) << 11) | ((uint32_t)(func) << 8) | ((uint32_t)(offset) & 0xfc) | (1 << 31))


static uint32_t pci_read(struct pci_dev* dev, uint8_t offset) {
    outl(PCI_ADDRESS_PORT, PCI_ADDRESS(dev->bus, dev->dev, dev->func, offset));
    return inl(PCI_DATA_PORT);
}

static void pci_write(struct pci_dev* dev, uint8_t offset, uint32_t value) {
    outl(PCI_ADDRESS_PORT, PCI_ADDRESS(dev->bus, dev->dev, dev->func, offset));
    outl(PCI_DATA_PORT, value);
}

static int pci_check_and_init_device(struct pci_dev* dev, uint8_t target_class, uint8_t target_subclass) {
    uint32_t id = pci_read(dev, 0x00);

    if (id == 0xffffffff) {
        return 0;
    }

    uint32_t class_and_subclass = pci_read(dev, 0x08);
    uint8_t class = (class_and_subclass >> 24) & 0xff;
    uint8_t subclass = (class_and_subclass >> 16) & 0xff;
    if (class != target_class || subclass != target_subclass) {
        return 0;
    }

    printk("found PCI device, bus=0x%x, dev=0x%x, func=0x%d, id=0x%x\n", dev->bus, dev->dev, dev->func, id);

    uint32_t status = pci_read(dev, 0x04);
    status >>= 16;
    if (status & (1 << 4)) {
        uint32_t cap_ptr = pci_read(dev, 0x34) & 0xff;
        for (;;) {
            uint32_t cap_header = pci_read(dev, cap_ptr);
            uint32_t cap_type = cap_header & 0xff;
            switch (cap_type) {
            case 0x05:
                dev->flags |= PCI_FLAG_MSI_CAPABLE;
                break;
            }

            cap_ptr = (cap_header >> 8) & 0xff;
            if (cap_ptr == 0) {
                break;
            }
        }
    }

    return 1;
}

int pci_find_device(struct pci_dev* pci_dev, uint8_t class, uint8_t subclass) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            pci_dev->bus = bus;
            pci_dev->dev = dev;
            pci_dev->func = 0;

            uint32_t reg = pci_read(pci_dev, 0x0c);
            uint8_t header_type = (reg >> 16) & 0xff;
            if (pci_check_and_init_device(pci_dev, class, subclass)) {
                return 1;
            }
            if (header_type & 0x80) {
                // Multifunction device.
                for (uint16_t func = 1; func < 8; func++) {
                    pci_dev->func = func;
                    if (pci_check_and_init_device(pci_dev, class, subclass)) {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}
