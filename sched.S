    .section .text
    .global __switch_to
    .type __switch_to, @function
__switch_to:
    mov     4(%esp), %eax
    mov     %esp, 0(%eax)
    mov     %ebp, 4(%eax)
    mov     %esi, 8(%eax)
    mov     %edi, 12(%eax)
    mov     %ebx, 16(%eax)

    mov     8(%esp), %eax
    mov     0(%eax), %esp
    mov     4(%eax), %ebp
    mov     8(%eax), %esi
    mov     12(%eax), %edi
    mov     16(%eax), %ebx

    ret
