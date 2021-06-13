# Simple Operating System

Core OS for QEMU/x86 with minimal functionality to study OS concepts. 

Key Features:

- monolithic kernel
- multiboot loading
- APIC/LAPIC support
- DMA storage support
- ext2 VFS support

This project is open source under the terms of [MIT License](./LICENSE). You can freely redistribute and use this code, but do not forget about your School or University rules on copyright and Honor Code while submit your homeworks.

Build Instructions:

Kernel build requires modern GCC toolchain with 32-bit x86 build environment
and grub tools.

Just try run GNU Make within project directory. You can override tools to use
(in case if different versions installed) by passing `make` command line
arguments.

Example:

```
> make CC=gcc-10 AS=gcc-10 LD=gcc-10 GRUB_MKRESCUE=grub2-mkrescue
```



