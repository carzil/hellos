#include "syscall.h"
#include "panic.h"
#include "common.h"
#include "errno.h"
#include "timer.h"
#include "sched.h"
#include "bug.h"

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
    terminal_writestring(".");
}

syscall_fn syscall_table[] = {
    [0] = syscall_wait,
    [1] = syscall_print,
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
