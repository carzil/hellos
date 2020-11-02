#pragma once
#include <stdint.h>

struct regs {
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

static inline void disable_irq() {
    asm volatile ("cli");
}

static inline void enable_irq() {
    asm volatile ("sti");
}

static inline int is_userspace(struct regs* regs) {
    return (regs->cs & 0b11) == 0b11;
}
