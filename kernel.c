#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "idt.h"
#include "gdt.h"
#include "acpi.h"
#include "apic.h"
#include "irq.h"
#include "vga.h"
#include "panic.h"
#include "paging.h"
#include "sched.h"

extern void jump_userspace();

void kernel_main(void) {
    init_gdt();
    init_idt();

    init_kalloc_early();
    init_kernel_paging();

    terminal_initialize();

    struct acpi_sdt* rsdt = acpi_find_rsdt();
    if (!rsdt) {
        panic("RSDT not found!");
    }

    apic_init(rsdt);

    printk("Hell OS is loaded\n");

    scheduler_start();
}
