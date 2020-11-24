CC=gcc -c -m32 -g -mgeneral-regs-only -std=gnu99 -ffreestanding -fno-pie -Wall -Wextra -static-libgcc -O2
LD=gcc -m32 -fno-pic -Wl,-static -Wl,-Bsymbolic -nostartfiles -nostdlib -ffreestanding

build:
	$(CC) main.c -o main.o
	$(LD) -T linker.ld main.o -o init
