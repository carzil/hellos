#include "exec.h"
#include "vfs.h"
#include "sched.h"
#include "elf.h"
#include "common.h"
#include "errno.h"
#include "defs.h"
#include "paging.h"

int load_image(const char* path) {
    struct file exe;
    memset(&exe, '\0', sizeof(exe));
    int ret = vfs_open(&exe, path, VFS_FILE_READABLE);
    if (ret < 0) {
        return ret;
    }

    void* content = kalloc();
    if (!content) {
        return -ENOMEM;
    }

    ret = vfs_read(&exe, content, PAGE_SIZE);
    if (ret < 0) {
        return ret;
    }

    Elf32_Ehdr* hdr = (Elf32_Ehdr*)content;

    if (memcmp(hdr->e_ident, ELF_MAGIC, sizeof(ELF_MAGIC) - 1)) {
        printk("invalid ELF magic\n");
        return -EACCES;
    }

    for (int i = 0; i < hdr->e_phnum; i++) {
        Elf32_Phdr* phdr = content + hdr->e_phoff + i * hdr->e_phentsize;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        if (phdr->p_filesz > PAGE_SIZE) {
            printk("too big phdr");
            return -EACCES;
        }
        void* page = kalloc();
        ret = vfs_read_at(&exe, phdr->p_offset, page, phdr->p_filesz);
        if (ret < 0) {
            return ret;
        }
        map_continous((void*)phdr->p_vaddr, phdr->p_memsz, virt2phys(page), PT_USER | PT_WRITEABLE);
    }

    void* stack = kalloc();
    map_continous(0x80000, PAGE_SIZE, virt2phys(stack), PT_USER | PT_WRITEABLE);

    current->regs->esp = 0x80000 + PAGE_SIZE;
    current->regs->eip = hdr->e_entry;

    return 0;
}
