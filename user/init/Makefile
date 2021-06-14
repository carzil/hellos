CC=gcc
LD=gcc
CCFLAGS=-c -m32 -g -mgeneral-regs-only -std=gnu99 -ffreestanding -fno-pie -Wall -Wextra -static-libgcc -O2
LDFLAGS=-m32 -fno-pic -Wl,-static -Wl,-Bsymbolic -nostartfiles -nostdlib -ffreestanding

build:
	$(CC) $(CCFLAGS) main.c -o main.o
	$(LD) $(LDFLAGS) -T linker.ld main.o -o init
