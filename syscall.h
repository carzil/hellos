#pragma once
#include <stdint.h>
#include "irq.h"

typedef uint32_t (*syscall_fn)(struct regs*);
