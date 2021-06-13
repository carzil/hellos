CC=gcc
AS=gcc
LD=gcc
GRUB_MKRESCUE=grub-mkrescue

ASFLAGS=-m32 -c -g -mgeneral-regs-only
CCFLAGS=-m32 -g -mgeneral-regs-only -mno-red-zone -std=gnu99 -ffreestanding -fno-pie -Wall -Wextra -static-libgcc -Wno-int-conversion -Wno-implicit-function-declaration -Wno-unused-function -Wno-discarded-qualifiers -Wno-unused-parameter -Wno-incompatible-pointer-types
LDFLAGS=-m32 -fno-pic -Wl,-static -Wl,-Bsymbolic -nostartfiles
OBJCOPY=objcopy
SHELL := /bin/bash

C_SOURCES   := $(shell ls *.c)
ASM_SOURCES := $(shell ls *.S)
C_OBJS      := $(C_SOURCES:.c=.c.o)
ASM_OBJS    := $(ASM_SOURCES:.S=.S.o)

image: build
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub
	cp kernel.bin isodir/boot
	$(GRUB_MKRESCUE) -o kernel.iso isodir
	rm -rf isodir

build: .c-depend .S-depend $(C_OBJS) $(ASM_OBJS) linker.ld
	$(LD) $(LDFLAGS) -T <(cpp -P -E linker.ld) -o kernel.bin -ffreestanding -O2 -nostdlib -lgcc $(ASM_OBJS) $(C_OBJS)
	$(OBJCOPY) --only-keep-debug kernel.bin kernel.sym
	$(OBJCOPY) --strip-debug kernel.bin

%.c.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.S.o: %.S
	$(AS) $(CCFLAGS) -c $< -o $@

.c-depend: $(C_SOURCES)
	rm -f ./.depend.c
	$(CC) $(CFLAGS) -MM $^ > ./.c-depend
	sed -i -E 's/^(.*)\.o/\1.c.o/g' .c-depend

.S-depend: $(ASM_SOURCES)
	rm -f ./.depend.S
	$(CC) $(CFLAGS) -MM $^ > ./.S-depend
	sed -i -E 's/^(.*)\.o/\1.S.o/g' .S-depend

clean:
	rm -f *.o
	rm -f .*-depend
	rm -f kernel.bin
	rm -f kernel.sym

include .c-depend
include .S-depend

.PHONY: build clean
