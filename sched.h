#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum state {
    TASK_NOT_ALLOCATED = 0,
    TASK_RUNNING       = 1,
    TASK_WAITING       = 2,
} state_t;

struct context {
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebx;
};

struct task {
    int pid;
    state_t state;
    struct context context;
    struct regs* regs;
    void* kstack;
    int ticks_remaining;
    int exitcode;
};

extern struct task* current;

struct task* task_create();
void scheduler_start();
void scheduler_tick(struct regs* regs);
void reschedule();
