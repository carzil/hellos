# Simple Operating System

Core OS for QEMU/x86 with minimal functionality to study OS concepts. 

Key Features:

- monolithic kernel
- multiboot loading
- APIC/LAPIC support
- DMA storage support
- ext2 VFS support

This project is open source under the terms of [MIT License](./LICENSE). You can freely redistribute and use this code, but do not forget about your School or University rules on copyright and Honor Code while submit your homeworks.

## Build Instructions:

### Linux Host

Kernel build requires modern GCC toolchain with 32-bit x86 build environment
and grub tools.

Just try run GNU Make within project directory. You can override tools to use
(in case if different versions installed) by passing `make` command line
arguments.

Example:

```
make CC=gcc-10 AS=gcc-10 LD=gcc-10 GRUB_MKRESCUE=grub2-mkrescue
```

### MacOS Host

Mac OS build requires ELF-related cross toolchain and GNU version of `sed`.

```
brew tap nativeos/i386-elf-toolchain 
brew install i386-elf-binutils i386-elf-gcc i386-elf-grub xorriso gnu-sed
```

To build image, run:

```
make CC=i386-elf-gcc AS=i386-elf-gcc LD=i386-elf-gcc SED=gsed OBJCOPY=i386-elf-objcopy
```

## Userspace init process

Trivial userspace init process program located in `./user/init`. Use `make` command to build and place built file into `/bin/` directory of `ext2` file system.

```
# prepare root file system
mkdir -p ./root_fs/bin
cp ./user/init/init ./root_fs/bin/

# create raw ext2 disk image from root directory
genext2fs -d ./root_fs -b 10000 -B 4096 c.img
```

## Launch it using QEMU

Command line options to start HellOS using qemu:

```
qemu-system-i386 -hda c.img -cdrom kernel.iso -boot d
```

