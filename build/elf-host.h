/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * This file deliberately carries no SPDX licence identifier: the
 * licence for this tree's own scripts is not settled yet, and the tag
 * is kept out rather than written into a sentence, where a scanner
 * would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
 * cover the Berkeley material, not this file.
 *
 * AI-ONLY NOTE: the host side of building usr.sbin/crunch/crunchide,
 * for build/sysroot.sh, as build/fs-host.h is for mkfs.c.
 *
 * crunchide's ELF back end, exec_elf32.c, is written against
 * FreeBSD 3.0's <elf.h>, which spells the ELF structures twice: by
 * their System V names, Elf32_Ehdr and the rest, and through
 * <sys/elf_generic.h> by size-free aliases, Elf_Ehdr, ELF_ST_INFO,
 * for the word size the machine header chose. The host's <elf.h> has
 * the System V names and not the aliases, nor FreeBSD's IS_ELF. So
 * this includes the host's and defines, for 32 bits, the aliases
 * exec_elf32.c uses -- each to the name <sys/elf_generic.h> makes it
 * by __ElfN and __ELFN -- and IS_ELF as FreeBSD 3.0's
 * <sys/elf_common.h> has it. __RCSID, which glibc's <sys/cdefs.h>
 * lacks, records a revision string and nothing else.
 *
 * Included with -include, so it is seen before anything else.
 * Nothing here is copied from another system beyond those names.
 */
#ifndef _ELF_HOST_H_
#define _ELF_HOST_H_

#include <elf.h>

#define	Elf_Ehdr	Elf32_Ehdr
#define	Elf_Shdr	Elf32_Shdr
#define	Elf_Sym		Elf32_Sym
#define	Elf_Rel		Elf32_Rel
#define	Elf_Rela	Elf32_Rela
#define	Elf_Word	Elf32_Word
#define	Elf_Off		Elf32_Off

#define	ELF_R_SYM	ELF32_R_SYM
#define	ELF_R_TYPE	ELF32_R_TYPE
#define	ELF_ST_BIND	ELF32_ST_BIND
#define	ELF_ST_TYPE	ELF32_ST_TYPE
#define	ELF_ST_INFO	ELF32_ST_INFO

#define	IS_ELF(ehdr)	((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
			 (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
			 (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
			 (ehdr).e_ident[EI_MAG3] == ELFMAG3)

#define	__RCSID(s)

#endif /* !_ELF_HOST_H_ */
