#pragma once

#include "defs.h"

static inline uint32_t inl(uint16_t port) {
    uint32_t word;
    asm volatile("in %1, %0" : "=a" (word) : "d" (port));
    return word;
}

static inline uint32_t outl(uint16_t port, uint32_t word) {
    asm volatile("out %0, %1" : : "a" (word), "d" (port));
}
