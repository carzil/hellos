AS=gcc -m32 -c -g -mgeneral-regs-only
CC=gcc -m32 -g -mgeneral-regs-only -mno-red-zone -std=gnu99 -ffreestanding -Wall -Wextra -fno-pie
LD=gcc -m32 -fno-pic
OBJCOPY=objcopy

build:
	$(AS) boot.s -o boot.o
	$(CC) -c kernel.c -o kernel.o
	$(CC) -c idt.c    -o idt.o
	$(CC) -c gdt.c    -o gdt.o
	$(AS) -c gdt.s    -o gdt_asm.o
	$(CC) -c acpi.c   -o acpi.o
	$(CC) -c apic.c   -o apic.o
	$(CC) -c vga.c    -o vga.o
	$(CC) -c paging.c -o paging.o
	$(LD) -T linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib boot.o paging.o acpi.o gdt_asm.o gdt.o idt.o kernel.o vga.o apic.o -lgcc
	$(OBJCOPY) --only-keep-debug kernel.bin kernel.sym
	$(OBJCOPY) --strip-debug kernel.bin

clean:
	rm -f *.o
	rm -f kernel.bin

.PHONY: build clean
