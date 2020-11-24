#include "syscall.h"

void _start() {
    for (;;) {
        syscall1(__NR_WAIT, 50);
        syscall1(__NR_PRINT, 1);
        syscall1(__NR_WAIT, 50);
        syscall1(__NR_PRINT, 2);
        syscall1(__NR_WAIT, 50);
        syscall1(__NR_PRINT, 3);
    }
}
