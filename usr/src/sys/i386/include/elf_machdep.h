/*	$NetBSD: elf_machdep.h,v 1.2 1997/10/11 19:11:10 christos Exp $	*/

/*
 * AI-ONLY NOTE: added (2026-10-26) -- NetBSD 1.3's i386 elf_machdep.h,
 * which <sys/exec_elf.h> includes, for ddb's db_elf.c: the machine
 * types an i386 ELF file may carry. Taken: ELF32_MACHDEP_ID_CASES, its
 * two labels spelt as this tree's <sys/exec_elf.h> names them,
 * Elf32_em_386 and Elf32_em_486 (NetBSD 1.1's revision 1.1), where
 * NetBSD 1.3's revision 1.11 calls them Elf_em_386 and Elf_em_486.
 * Not taken: ELF32_MACHDEP_ENDIANNESS, whose Elf_ed_2lsb this tree's
 * header has not, and the ELF64 pair, there being no 64-bit types.
 * Neither is used by anything here.
 */

#define	ELF32_MACHDEP_ID_CASES						\
		case Elf32_em_386:					\
		case Elf32_em_486:					\
			break;
