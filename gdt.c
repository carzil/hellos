#include "gdt.h"

struct gdt_entry {\
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  limit_high;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry GDT[6];
struct gdt_ptr GDTPTR;
struct tss TSS;

#define ACCESS_PRESENT   0b10000000
#define ACCESS_RING0     0b00000000
#define ACCESS_RING3     0b01100000
#define ACCESS_SEGMENT   0b00010000
#define ACCESS_EXEC      0b00001000
#define ACCESS_RW        0b00000010
#define ACCESS_ACCESSED  0b00000001

static void gdt_register(int32_t idx, uint32_t base, uint32_t limit, uint8_t access) {
    GDT[idx].base_low = (base & 0xFFFF);
    GDT[idx].base_middle = (base >> 16) & 0xFF;
    GDT[idx].base_high = (base >> 24) & 0xFF;

    GDT[idx].limit_low = (limit & 0xFFFF);
    GDT[idx].limit_high = (limit >> 16) & 0x0F;

    GDT[idx].limit_high |= 0b11000000;
    GDT[idx].access = access | ACCESS_PRESENT;
}

void reload_tss() {
    // Clear Busy flag.
    GDT[5].access &= 0b11111101;
    asm volatile (
        "movw   $0x2b, %ax\n"
        "ltrw   %ax\n"
    );
}

void init_gdt() {
    gdt_register(0, 0, 0, 0);
    gdt_register(1, 0, 0xffffffff, ACCESS_RING0 | ACCESS_SEGMENT | ACCESS_EXEC | ACCESS_RW);
    gdt_register(2, 0, 0xffffffff, ACCESS_RING0 | ACCESS_SEGMENT | ACCESS_RW);
    gdt_register(3, 0, 0xffffffff, ACCESS_RING3 | ACCESS_SEGMENT | ACCESS_EXEC | ACCESS_RW);
    gdt_register(4, 0, 0xffffffff, ACCESS_RING3 | ACCESS_SEGMENT | ACCESS_RW);
    gdt_register(5, (uint32_t)&TSS, sizeof(TSS) - 1, ACCESS_EXEC | 1);

    GDTPTR.base = (uint32_t)&GDT;
    GDTPTR.limit = sizeof(GDT) - 1;

    asm volatile (
        "lgdt	(%0)\n"
        "movw   $0x10, %%ax\n"
        "movw   %%ax, %%ds\n"
        "movw   %%ax, %%es\n"
        "movw   %%ax, %%fs\n"
        "movw   %%ax, %%gs\n"
        "movw   %%ax, %%ss\n"
        "ljmp	$0x08, $1f\n"
        "1:\n"
        :
        : "r"(&GDTPTR)
    );

    reload_tss();
}
