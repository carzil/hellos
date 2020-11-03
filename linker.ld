ENTRY(_start_early)

#include "defs.h"

SECTIONS
{
    . = 1M;

    .multiboot : {
        KEEP(*(.multiboot))
    }

    .boot.text : {
        *(.boot.text)
    }

    .boot.bss : {
        *(.boot.bss)
    }

    . += KERNEL_HIGH;
    . = ALIGN(0x1000);

    PROVIDE(KERNEL_START = .);
    .text : AT(ADDR(.text) - KERNEL_HIGH) {
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_HIGH) {
        *(.rodata)
    }

    .data : AT(ADDR(.data) - KERNEL_HIGH) {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - KERNEL_HIGH) {
        *(COMMON)
        *(.bss)
    }
    PROVIDE(KERNEL_END = .);

    . += 4M;
    . = ALIGN(0x1000);
    PROVIDE(USERSPACE_START = .);
    .userspace.text : AT(ADDR(.userspace.text) - KERNEL_HIGH) {
        *(.userspace.text)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note.gnu.*)
        *(.dynamic)
    }
}
