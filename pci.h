#pragma once

#include <stdint.h>
#include <stddef.h>

struct pci_dev {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint32_t flags;
};

#define PCI_FLAG_MSI_CAPABLE (1 << 0)

#define PCI_CLASS_MASS_STORAGE_CONTROLLER  0x01
#define PCI_SUBCLASS_IDE                   0x01
#define PCI_SUBCLASS_ATA                   0x05
#define PCI_SUBCLASS_SATA                  0x06

void pci_print_devices();
int pci_find_device(struct pci_dev* dev, uint8_t class, uint8_t subclass);
void pci_set_irq(struct pci_dev* dev, int irq);
uint32_t pci_read_bar4(struct pci_dev* dev);
void pci_enable_bus_master(struct pci_dev* dev);
