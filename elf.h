#pragma once

#include <stddef.h>
#include <stdint.h>


typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8
#define EI_PAD        9
#define ELF_NIDENT    16
#define ELF_MAGIC     "\177ELF"
#define ELFDATA2LSB   1
#define ELFCLASS32    1
#define PT_LOAD       1

typedef struct elf32_hdr {
  unsigned char  e_ident[ELF_NIDENT];
  Elf32_Half     e_type;
  Elf32_Half     e_machine;
  Elf32_Word     e_version;
  Elf32_Addr     e_entry;  /* Entry point */
  Elf32_Off      e_phoff;
  Elf32_Off      e_shoff;
  Elf32_Word     e_flags;
  Elf32_Half     e_ehsize;
  Elf32_Half     e_phentsize;
  Elf32_Half     e_phnum;
  Elf32_Half     e_shentsize;
  Elf32_Half     e_shnum;
  Elf32_Half     e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	Elf32_Word		p_type;
	Elf32_Off		p_offset;
	Elf32_Addr		p_vaddr;
	Elf32_Addr		p_paddr;
	Elf32_Word		p_filesz;
	Elf32_Word		p_memsz;
	Elf32_Word		p_flags;
	Elf32_Word		p_align;
} Elf32_Phdr;
