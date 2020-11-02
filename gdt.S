    .section .text
    .global load_gdt
    .type load_gdt, @function
load_gdt:
    movl    4(%esp), %eax
    lgdt	(%eax)

    # 0x10 is the data segment selector.
    movw	$0x10, %ax
    movw	%ax, %ds
    movw	%ax, %es
    movw	%ax, %fs
    movw	%ax, %gs
    movw	%ax, %ss

    # Load TSS.
    movw    $0x2b, %ax
    ltr     %ax

    ljmp	$0x08, $.flush  # Change CS.
.flush:
    ret
