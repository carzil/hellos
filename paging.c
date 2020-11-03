#include "paging.h"
#include "common.h"
#include "panic.h"
#include "irq.h"

#include <stdint.h>

uint32_t early_pgdir[1024] __attribute__((aligned(4096)));

struct fl_entry {
    struct fl_entry* next;
};

struct kalloc_head {
    struct fl_entry* freelist_head;
};

struct kalloc_head kalloc_head;

__attribute__((section(".boot.text"))) void setup_paging_early() {
    // 0x0...4Mb identity mapped
    // KERNEL_HIGH...KERNEL_HIGH+4Mb mapped to 0x0...4Mb
    early_pgdir[0] = PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
    early_pgdir[PGDIR_IDX(KERNEL_HIGH)] = PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
}

void init_kalloc_early() {
    void* addr = (void*)ROUNDUP((uint32_t)&KERNEL_END);
    kalloc_head.freelist_head = NULL;
    void* end = KERNEL_HIGH + 4 * (1 << 20);
    while (addr < end) {
        struct fl_entry* entry = (struct fl_entry*)addr;
        entry->next = kalloc_head.freelist_head;
        kalloc_head.freelist_head = entry;
        addr += PAGE_SIZE;
    }
}

void* kalloc() {
    // TODO: spinlock needed here.

    if (!kalloc_head.freelist_head) {
        return NULL;
    }

    void* ptr = kalloc_head.freelist_head;
    kalloc_head.freelist_head = kalloc_head.freelist_head->next;
    return ptr;
}

void kfree(void* p) {
    // ...
}

uint32_t* alloc_page(uint32_t* pgdir, void* addr, int user) {
    uint32_t* page_table = NULL;
    if (pgdir[PGDIR_IDX(addr)] & PT_PRESENT) {
        page_table = phys2virt((void*)ROUNDDOWN(pgdir[PGDIR_IDX(addr)]));
    } else {
        page_table = kalloc();
        if (!page_table) {
            return NULL;
        }
        for (int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
    }

    int flags = PT_PRESENT;
    if (user) {
        flags |= PT_USER;
    }
    pgdir[PGDIR_IDX(addr)] = ((uint32_t)virt2phys(page_table)) | flags;
    return &page_table[PT_IDX(addr)];
}

void map_continous(uint32_t* pgdir, void* addr, size_t size, void* phys_addr, int flags) {
    addr = (void*)ROUNDDOWN((uint32_t)addr);
    phys_addr = (void*)ROUNDDOWN((uint32_t)phys_addr);
    size = ROUNDUP(size);

    while (size > 0) {
        uint32_t* pte = alloc_page(pgdir, addr, flags & PT_USER);
        *pte = ((uint32_t)phys_addr) | PT_PRESENT;
        *pte |= flags;
        addr += PAGE_SIZE;
        phys_addr += PAGE_SIZE;
        size -= PAGE_SIZE;
    }
}

uint32_t* kernel_pgdir = NULL;

void load_cr3(uint32_t* pgdir) {
    asm volatile (
        "mov %0, %%cr3\n"
        :
        : "r"(pgdir)
        : "memory"
    );
}

void init_kernel_paging() {
    kernel_pgdir = kalloc();
    memset(kernel_pgdir, '\0', PAGE_SIZE);
    map_continous(kernel_pgdir, (void*)KERNEL_HIGH, 4 * (1 << 20), 0x0, PT_WRITEABLE);
    map_continous(kernel_pgdir, &USERSPACE_START, 4096, virt2phys(&USERSPACE_START), PT_USER | PT_WRITEABLE);
    load_cr3(virt2phys(kernel_pgdir));
}

void identity_map(void* addr, size_t sz) {
    map_continous(kernel_pgdir, addr, sz, addr, PT_WRITEABLE);
}

uint32_t read_cr2() {
    uint32_t val = 0;
    asm volatile (
        "mov %%cr2, %0\n"
        : "=r"(val)
    );
    return val;
}

void pagefault_irq(struct regs* regs) {
    (void)regs;

    void* addr = (void*)read_cr2();

    if (!is_userspace(regs)) {
        panic("pagefault in kernel space\n    cr2=0x%x, eip=0x%x, err_code=%d", addr, regs->eip, regs->error_code);
    }
}
