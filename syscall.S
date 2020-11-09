    .section .userspace.text, "ax"
    .global userspace_fn
    .type userspace_fn, @function
userspace_fn:
.l:
    # print(1)
    mov $0x1, %eax
    mov $0x1, %ebx
    int $0x80

    # wait(5)
    mov $0x0, %eax
    mov $5, %ebx
    int $0x80

    # print(2)
    mov $0x1, %eax
    mov $0x2, %ebx
    int $0x80

    # read()
    mov $0x2, %eax
    int $0x80

    # print(3)
    mov $0x1, %eax
    mov $0x3, %ebx
    int $0x80

    jmp .l
