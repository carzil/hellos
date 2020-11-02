#include <stddef.h>
#include <stdint.h>

#include "idt.h"
#include "apic.h"
#include "syscall.h"

struct idt_entry {
    uint16_t base_lo;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define ENTRY(base, sel, flg) {\
    .base_lo  = ((uint32_t)(base)) & 0xffff, \
    .base_hi  = ((uint32_t)(base) >> 16) & 0xffff, \
    .selector = (sel), \
    .zero     = 0, \
    .flags    = flg, \
}


struct idt_entry IDT[256];
struct idt_ptr IDTPTR;

static void idt_register(uint8_t pos, uint32_t base, uint16_t sel, uint8_t flags) {
    IDT[pos].base_lo = base & 0xFFFF;
    IDT[pos].base_hi = (base >> 16) & 0xFFFF;

    IDT[pos].selector = sel;
    IDT[pos].zero = 0;
    IDT[pos].flags = flags;
}

#define REGISTER_IRQ(n, entry, sel, flags) \
    extern char __##entry[]; \
    idt_register(n, (uint32_t)__##entry, sel, (flags) | 0b10000000);

void dummy_irq(struct regs* frame) {
    (void)frame;
}

void spurious_irq(struct regs* frame) {
    (void)frame;
    apic_eoi();
}

#define GATE_INTERRUPT 0b1110
#define GATE_TRAP      0b1111
#define DPL_USER       0b01100000

void init_idt() {
    IDTPTR.limit = sizeof(IDT) - 1;
    IDTPTR.base = (uint32_t)&IDT;

    // For flags param see https://wiki.osdev.org/IDT#Structure_IA-32.
    REGISTER_IRQ(14,  pagefault_irq, 0x08, GATE_INTERRUPT);
    REGISTER_IRQ(32,  timer_irq,     0x08, GATE_INTERRUPT);
    REGISTER_IRQ(39,  spurious_irq,  0x08, GATE_INTERRUPT);
    REGISTER_IRQ(40,  keyboard_irq,  0x08, GATE_INTERRUPT);
    REGISTER_IRQ(128, syscall_irq,   0x08, GATE_TRAP | DPL_USER);

    asm volatile (
        "lidt (%0)\n"
        :
        : "r"(&IDTPTR)
        :
    );
}
