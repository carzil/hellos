#include "syscall.h"
#include "panic.h"
#include "common.h"
#include "errno.h"
#include "timer.h"
#include "sched.h"
#include "bug.h"
#include "ata.h"

uint32_t syscall_wait(struct regs* regs) {
    int ticks = regs->ebx;
    if (ticks <= 0) {
        return -EINVAL;
    }

    current->ticks_remaining = ticks;
    current->state = TASK_WAITING;
    reschedule();
    return 0;
}

uint32_t syscall_print(struct regs* regs) {
    switch (regs->ebx) {
    case 1:
        terminal_writestring("1");
        break;
    case 2:
        terminal_writestring("2");
        break;
    case 3:
        terminal_writestring("3");
        break;
    case 4:
        terminal_writestring("4");
        break;
    default:
        terminal_writestring("x");
        break;
    }
    return 0;
}

uint32_t syscall_read(struct regs* regs) {
    uint16_t sector[4 * 256];
    return ata_read(&sector, 12, 4);
}

syscall_fn syscall_table[] = {
    [0] = syscall_wait,
    [1] = syscall_print,
    [2] = syscall_read,
};


void syscall_irq(struct regs* regs) {
    BUG_ON(!is_userspace(regs));
    BUG_ON_NULL(current);

    if (regs->eax >= ARRAY_SIZE(syscall_table)) {
        regs->eax = -ENOSYS;
        return;
    }

    regs->eax = syscall_table[regs->eax](regs);
}
