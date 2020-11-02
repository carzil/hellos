    .section .userspace.text, "ax"
    .global userspace_fn
    .type userspace_fn, @function
userspace_fn:
.l:
    mov $0x0, %eax
    mov $50, %ebx
    int $0x80
    mov $0x1, %eax
    int $0x80
    jmp .l
