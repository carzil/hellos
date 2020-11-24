#include <stdint.h>

#define __NR_WAIT  0
#define __NR_PRINT 1

static inline uint32_t syscall1(uint32_t nr, uint32_t arg) {
    uint32_t ret = 0;
    asm volatile (
        "mov %1, %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : "g"(nr), "b"(arg)
    );
    return 0;
}
