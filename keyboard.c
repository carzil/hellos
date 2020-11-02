#include "irq.h"
#include "vga.h"

void keyboard_irq(struct regs* regs) {
    (void)regs;
    terminal_writestring_color(".", vga_entry_color(VGA_COLOR_RED, VGA_COLOR_GREEN));
    apic_eoi();
}
