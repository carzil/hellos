#include "pci.h"
#include "io.h"

#define PCI_ADDRESS_PORT        0xcf8
#define PCI_DATA_PORT           0xcfc


uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address = 0;
    address |= (uint32_t)bus << 16;
    address |= (uint32_t)dev << 11;
    address |= (uint32_t)func << 8;
    address |= offset & 0xfc;
    address |= 1 << 31;

    outl(PCI_ADDRESS_PORT, address);
    return inl(PCI_DATA_PORT);
}

void pci_check_device(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t id = pci_read(bus, dev, func, 0x00);

    if (id != 0xffffffff) {
        printk("found PCI device, bus=0x%x, dev=0x%x, func=0x%d, id=0x%x\n", bus, dev, func, id);
    }
}

void pci_print_devices() {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            uint32_t reg = pci_read(bus, dev, 0, 0x0c);
            uint8_t header_type = (reg >> 16) & 0xff;
            pci_check_device(bus, dev, 0);
            if (header_type & 0x80) {
                // Multifunction device.
                for (uint16_t func = 1; func < 8; func++) {
                    pci_check_device(bus, dev, func);
                }
            }
        }
    }
}
