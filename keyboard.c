#include "irq.h"
#include "vga.h"

void keyboard_irq(struct regs* regs) {
    (void)regs;
    // ...
    apic_eoi();
}
