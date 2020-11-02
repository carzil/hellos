#define ERRCODE 0
#define NOERRCODE 1

.macro RESTORE_CTX_AND_IRET
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popw    %gs
    popw    %fs
    popw    %es
    popw    %ds

    addl    $4, %esp # Skip error code.
    iret
.endm

.macro IRQ_ENTRY errcode entry
    .section .text
    .extern \entry
    .global __\entry
    .type __\entry, @function
__\entry:
.if \errcode
    pushl $0
.endif
    # First of all, save context on stack.
    pushw   %ds
    pushw   %es
    pushw   %fs
    pushw   %gs
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp

    # Restore kernel data segments.
    movw	$0x10, %ax
    movw	%ax, %ds
    movw	%ax, %es
    movw	%ax, %fs
    movw	%ax, %gs

    pushl   %esp
    call    \entry
    addl    $4, %esp

    RESTORE_CTX_AND_IRET
.endm

IRQ_ENTRY NOERRCODE timer_irq
IRQ_ENTRY NOERRCODE keyboard_irq
IRQ_ENTRY NOERRCODE spurious_irq
IRQ_ENTRY NOERRCODE syscall_irq
IRQ_ENTRY ERRCODE   pagefault_irq


    .section .text
    .global __jump_userspace
    .type __jump_userspace, @function
__jump_userspace:
    RESTORE_CTX_AND_IRET
