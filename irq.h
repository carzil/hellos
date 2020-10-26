#pragma once

static inline void disable_irq() {
    asm volatile ("cli");
}

static inline void enable_irq() {
    asm volatile ("sti");
}
