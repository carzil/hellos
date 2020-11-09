#pragma once

#include "defs.h"
#include "printk.h"

static inline uint32_t inl(uint16_t port) {
    uint32_t data;
    asm volatile ("in %1, %0" : "=a" (data) : "d" (port));
    return data;
}

static inline void outl(uint16_t port, uint32_t data) {
    asm volatile ("out %0, %1" : : "a" (data), "d" (port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile ("in %1, %0" : "=a" (data) : "d" (port));
    return data;
}

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile ("out %0, %1" : : "a" (data), "d" (port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t data;
    asm volatile ("in %1, %0" : "=a" (data) : "d" (port));
    return data;
}

static inline void outw(uint16_t port, uint16_t data) {
    asm volatile ("out %0, %1" : : "a" (data), "d" (port));
}
