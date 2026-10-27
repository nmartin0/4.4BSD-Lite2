/*	$NetBSD: exec_elf.h,v 1.1 1995/06/22 21:31:03 fvdl Exp $	*/

/*
 * Copyright (c) 1994 Christos Zoulas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * AI-ONLY NOTE: imported from NetBSD 1.1, sys/sys/exec_elf.h, revision
 * 1.1 of 22 June 1995, which is the earliest NetBSD release carrying a
 * working i386 ELF loader. 4.4BSD-Lite2 has no ELF definitions at all
 * outside contrib/gdb-4.7.LBL's bfd, and this tree's own binaries are
 * ELF -- GNU ld has no i386 a.out emulation, as
 * docs/provenance/csu-elf.md records for lib/csu.
 *
 * 153 of its 162 lines are taken. The nine dropped are prototypes for
 * exec_elf_makecmds and elf_copyargs, which belong to NetBSD's exec
 * framework -- struct exec_package and the vmcmd set -- and this tree
 * does not have that framework and is not taking it. What remains is
 * the on-disk format alone: Elf32_Ehdr, Elf32_Phdr, the e_ident magic,
 * and the machine, type and program-header constants, which are the
 * ELF specification and not anyone's design. (An earlier form of this
 * note also listed Elf32_Shdr and SHT_ constants; revision 1.1 has
 * neither. They arrive below, from a later revision.)
 *
 * Three-clause BSD, Christos Zoulas 1994. No Regents notice, so
 * nothing here is Berkeley's.
 */

#ifndef _SYS_EXEC_ELF_H_
#define _SYS_EXEC_ELF_H_

#include <machine/types.h>

typedef u_int32_t Elf32_Addr;
typedef u_int32_t Elf32_Off;
typedef int32_t   Elf32_Sword;
typedef u_int32_t Elf32_Word;
typedef u_int16_t Elf32_Half;


#define ELF_IDSIZE	16

enum Elf32_e_type {
	Elf32_et_none = 0,
	Elf32_et_rel,
	Elf32_et_exec,
	Elf32_et_dyn,
	Elf32_et_core,
	Elf32_et_num
};

enum Elf32_e_machine {
	Elf32_em_none = 0,
	Elf32_em_m32,
	Elf32_em_sparc,
	Elf32_em_386,
	Elf32_em_68k,
	Elf32_em_88k,
	Elf32_em_486,
	Elf32_em_860,
	Elf32_em_mips,
	Elf32_em_num
};

typedef struct {
	unsigned char	e_ident[ELF_IDSIZE];	/* Id bytes */
	Elf32_Half	e_type;			/* file type */
	Elf32_Half	e_machine;		/* machine type */
	Elf32_Word	e_version;		/* version number */
	Elf32_Addr	e_entry;		/* entry point */
	Elf32_Off	e_phoff;		/* Program hdr offset */
	Elf32_Off	e_shoff;		/* Section hdr offset */
	Elf32_Word	e_flags;		/* Processor flags */
	Elf32_Half      e_ehsize;		/* sizeof ehdr */
	Elf32_Half      e_phentsize;		/* Program header entry size */
	Elf32_Half      e_phnum;		/* Number of program headers */
	Elf32_Half      e_shentsize;		/* Section header entry size */
	Elf32_Half      e_shnum;		/* Number of section headers */
	Elf32_Half      e_shstrndx;		/* String table index */
} Elf32_Ehdr;


enum Elf32_p_pf {
	Elf32_pf_r = 4,
	Elf32_pf_w = 2,
	Elf32_pf_x = 1
};

enum Elf32_p_pt {
	Elf32_pt_nulli = 0,		/* Program header table entry unused */
	Elf32_pt_load = 1,		/* Loadable program segment */
	Elf32_pt_dynamic = 2,		/* Dynamic linking information */
	Elf32_pt_interp = 3,		/* Program interpreter */
	Elf32_pt_note = 4,		/* Auxiliary information */
	Elf32_pt_shlib = 5,		/* Reserved, unspecified semantics */
	Elf32_pt_phdr = 6,		/* Entry for header table itself */
	Elf32_pt_loproc = 0x70000000,	/* Processor-specific */
	Elf32_pt_hiproc = 0x7FFFFFFF	/* Processor-specific */

};

typedef struct {
	Elf32_Word	p_type;		/* entry type */
	Elf32_Off	p_offset;	/* offset */
	Elf32_Addr	p_vaddr;	/* virtual address */
	Elf32_Addr	p_paddr;	/* physical address */
	Elf32_Word	p_filesz;	/* file size */
	Elf32_Word	p_memsz;	/* memory size */
	Elf32_Word	p_flags;	/* flags */
	Elf32_Word	p_align;	/* memory & file alignment */
} Elf32_Phdr;

#define Elf32_e_ident "\177ELF"
#define Elf32_e_siz (sizeof(Elf32_e_ident) - 1)

/*
 * AI-ONLY NOTE: section headers and symbols, for nlist(3) and
 * kvm_mkdb(8) to read a kernel's symbol table -- revision 1.1 above
 * describes only what exec needs. Taken from NetBSD 1.3's revision of
 * this same file, 1.11 of 10 June 1997, which is the revision
 * NetBSD 1.3's nlist_elf32.c was written against: Elf_Byte, the
 * section-type enumeration, Elf32_Shdr, Elf32_Sym, and the symbol
 * binding and type enumerations with their two macros. Their 64-bit
 * twins, the section flags, the special section indices and the
 * relocation, dynamic and note structures are left there.
 */
typedef u_int8_t  Elf_Byte;

enum Elf_s_sht {
	Elf_sht_null = 0,
	Elf_sht_progbits = 1,
	Elf_sht_symtab = 2,
	Elf_sht_strtab = 3,
	Elf_sht_rela = 4,
	Elf_sht_hash = 5,
	Elf_sht_dynamic = 6,
	Elf_sht_note = 7,
	Elf_sht_nobits = 8,
	Elf_sht_rel =	9,
	Elf_sht_shlib = 10,
	Elf_sht_dynsym = 11,
	Elf_sht_loproc = 0x70000000,
	Elf_sht_hiproc = 0x7FFFFFFF
};

typedef struct {
	Elf32_Word	sh_name;	/* section name */
	Elf32_Word	sh_type;	/* section type */
	Elf32_Word	sh_flags;	/* section flags */
	Elf32_Addr	sh_addr;	/* virtual address */
	Elf32_Off	sh_offset;	/* file offset */
	Elf32_Word	sh_size;	/* section size */
	Elf32_Word	sh_link;	/* link to another */
	Elf32_Word	sh_info;	/* misc info */
	Elf32_Word	sh_addralign;	/* memory alignment */
	Elf32_Word	sh_entsize;	/* table entry size */
} Elf32_Shdr;

typedef struct {
	Elf32_Word	st_name;	/* Symbol name index in str table */
	Elf32_Word	st_value;	/* value of symbol */
	Elf32_Word	st_size;	/* size of symbol */
	Elf_Byte	st_info;	/* type / binding attrs */
	Elf_Byte	st_other;	/* unused */
	Elf32_Half	st_shndx;	/* section index of symbol */
} Elf32_Sym;

enum elf_e_symbol_binding {
	Elf_estb_local,
	Elf_estb_global,
	Elf_estb_weak
};

#define	ELF_SYM_BIND(info)	((enum elf_e_symbol_binding) (((u_int32_t)(info)) >> 4))

enum elf_e_symbol_type {
	Elf_estt_notype,
	Elf_estt_object,
	Elf_estt_func,
	Elf_estt_section,
	Elf_estt_file
};

#define	ELF_SYM_TYPE(info)	((enum elf_e_symbol_type) ((info) & 0x0F))

typedef struct {
	int		au_id;
	unsigned long	au_v;
} AuxInfo;

enum AuxID {
	AUX_null = 0,
	AUX_ignore = 1,
	AUX_execfd = 2,
	AUX_phdr = 3,		/* &phdr[0] */
	AUX_phent = 4,		/* sizeof(phdr[0]) */
	AUX_phnum = 5,		/* # phdr entries */
	AUX_pagesz = 6,		/* PAGESIZE */
	AUX_base = 7,		/* ld.so base addr */
	AUX_flags = 8,		/* processor flags */
	AUX_entry = 9,		/* a.out entry */
	AUX_sun_uid = 2000,	/* euid */
	AUX_sun_ruid = 2001,	/* ruid */
	AUX_sun_gid = 2002,	/* egid */
	AUX_sun_rgid = 2003	/* rgid */
};

#define	ELF_HDR_SIZE	(sizeof(Elf32_Ehdr))

/*
 * AI-ONLY NOTE: added (2026-10-26) -- the machine's own ELF
 * definitions and the size-independent names, for ddb's reader of the
 * kernel's ELF symbols, NetBSD 1.3's ddb/db_elf.c, which is written
 * against them. Taken from NetBSD 1.3's revision of this file, 1.11 of
 * 10 June 1997, its last lines before `#ifdef _KERNEL': the include of
 * <machine/elf_machdep.h>, and of the ELFSIZE == 32 names, those whose
 * Elf32_ types this file has. Left there: Elf_Rel, Elf_RelA, Elf_Dyn,
 * Elf_Note, ELF_R_SYM, ELF_R_TYPE and AuxInfo, whose Elf32_ forms are
 * not here, and the ELFSIZE == 64 arm, there being no Elf64_ types.
 */
#include <machine/elf_machdep.h>

#if defined(ELFSIZE) && (ELFSIZE == 32)
#define	Elf_Ehdr	Elf32_Ehdr
#define	Elf_Phdr	Elf32_Phdr
#define	Elf_Shdr	Elf32_Shdr
#define	Elf_Sym		Elf32_Sym
#define	Elf_Word	Elf32_Word
#define	Elf_Addr	Elf32_Addr
#define	Elf_Off		Elf32_Off

#define	Elf_e_ident	Elf32_e_ident
#define	Elf_e_siz	Elf32_e_siz
#endif

/*
 * AI-ONLY NOTE: KERNEL, where NetBSD 1.1 wrote _KERNEL. This tree
 * compiles the kernel with -DKERNEL, as compile/LINK.i386/Makefile
 * shows, and its own headers test the unprefixed name -- sys/buf.h
 * and sys/proc.h each have one `#ifdef KERNEL' and no `_KERNEL'.
 * NetBSD had moved to the underscored form by 1.1; 4.4BSD-Lite2 had
 * not.
 */
#ifdef KERNEL

#define ELF32_NO_ADDR	((u_long) ~0)

struct elf_args {
        u_long  arg_entry;      /* progran entry point */
        u_long  arg_interp;     /* Interpreter load address */
        u_long  arg_phaddr;     /* program header address */
        u_long  arg_phentsize;  /* Size of program header */
        u_long  arg_phnum;      /* Number of program headers */
};

#endif /* KERNEL */

#endif /* !_SYS_EXEC_ELF_H_ */
