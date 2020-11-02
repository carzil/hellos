#pragma once

#include <stdint.h>
#include <stddef.h>

struct tss {
    uint32_t link;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved0;
    uint32_t reserved1[21];
    uint16_t reserved2;
    uint16_t iopb;
} __attribute__((packed));

extern struct tss TSS;

void init_gdt();
void reload_tss();
