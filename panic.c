#include <stdarg.h>

#include "panic.h"
#include "irq.h"

void __panic(const char* location, const char* msg, ...) {
    printk("panic at %s: ", location);

    va_list args;
    va_start(args, msg);
    vprintk(msg, args);
    va_end(args);

    disable_irq();
    for (;;) {
        asm volatile ("hlt");
    }
}
