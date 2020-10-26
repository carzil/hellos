ENTRY(_start_early)

KERNEL_HIGH = 0x80000000;

SECTIONS
{
    . = 0x00100000;

    .multiboot : {
        KEEP(*(.multiboot))
    }

    .boot.text : {
        *(.boot.text)
    }

	. += KERNEL_HIGH;

    PROVIDE(KERNEL_START = .);
	.text ALIGN(0x1000) : AT(ADDR(.text) - KERNEL_HIGH) {
		*(.text)
	}

	.rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_HIGH) {
		*(.rodata)
	}

	.data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_HIGH) {
		*(.data)
	}

	.bss ALIGN(0x1000) : AT(ADDR(.bss) - KERNEL_HIGH) {
		*(COMMON)
		*(.bss)
	}
    PROVIDE(KERNEL_END = .);

    /DISCARD/ : {
		*(.eh_frame)
        *(.note.gnu.*)
	}
}
