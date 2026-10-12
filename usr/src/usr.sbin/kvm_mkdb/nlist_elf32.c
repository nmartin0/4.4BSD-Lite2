/*	$NetBSD: nlist_elf32.c,v 1.4.2.1 1997/10/23 20:11:35 mellon Exp $	*/

/*
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou
 *	for the NetBSD Project.
 * 4. The name of the author may not be used to endorse or promote products
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
 * AI-ONLY NOTE: NetBSD 1.3's usr.sbin/kvm_mkdb/nlist_elf32.c,
 * revision 1.4.2.1, so that kvm_mkdb reads an ELF kernel's symbol
 * table: before it, /etc/rc's kvm_mkdb said `/vmunix: bad magic
 * number' on every boot.
 *
 * Shrink-wrapped as lib/libc/gen/nlist_elf32.c is, from the same
 * release: the 32-bit names written out where ELFSIZE and the ELFNAME
 * macros made them, NetBSD 1.3's i386 machine cases in this tree's
 * sys/exec_elf.h spelling, and its format list in nlist.c rather than
 * extern.h. NetBSD's kvm_mkdb.c has punt(), which removes the
 * temporary database before exiting; this tree's does not, so each
 * `warn(...); punt();' is err(1, ...) and badfmt is errx(1, ...),
 * the exits Berkeley's own nlist.c here makes. name is char *, not
 * const char *, to match this tree's create_knlist. The reading is
 * unchanged: the whole file mapped from offset 0, the header, section
 * header table and symbol and string sections checked against its
 * size, each symbol entered under its name with
 * an underscore added, as a.out named it, and the version string
 * entered under VRS_KEY. One line differs in what it does: a kernel
 * with no symbol table at all left symshdrp NULL and was dereferenced;
 * it is now `stripped', with the test the libc file of the same
 * release makes, `symshdrp == NULL ||'.
 */

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <a.out.h>
#include <db.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#include <sys/exec_elf.h>

typedef struct nlist NLIST;
#define	_strx	n_un.n_strx
#define	_name	n_un.n_name

#define	badfmt(str)	errx(1, "%s: %s: %s", kfile, str, strerror(EFTYPE))

#define	check(off, size)	((off < 0) || (off + size > mappedsize))
#define	BAD			do { rv = -1; goto out; } while (0)
#define	BADUNMAP		do { rv = -1; goto unmap; } while (0)

static char *kfile;

int
create_knlist_elf32(name, db)
	char *name;
	DB *db;
{
	struct stat st;
	struct nlist nbuf;
	DBT key, data;
	char *mappedfile, *symname, *fsymname, *tmpcp, *strtab;
	size_t mappedsize, symnamesize, fsymnamesize;
	Elf32_Ehdr *ehdrp;
	Elf32_Shdr *shdrp, *symshdrp, *symstrshdrp;
	Elf32_Sym *symp;
	Elf32_Off shdr_off;
	Elf32_Word shdr_size;
	Elf32_Half nshdr;
	unsigned long i, nsyms;
	int fd, rv;

	rv = -1;
#ifdef __GNUC__
	/* fix compiler warnings */
	symshdrp = NULL;
	symstrshdrp = NULL;
#endif

	/*
	 * Open and map the whole file.  If we can't open/stat it,
	 * something bad is going on so we punt.
	 */
	kfile = name;
	if ((fd = open(name, O_RDONLY, 0)) < 0)
		err(1, "%s", kfile);
	if (fstat(fd, &st) < 0)
		err(1, "%s", kfile);
	if (st.st_size > SIZE_T_MAX)
		BAD;

	/*
	 * Map the file in its entirety.
	 */
	mappedsize = st.st_size;
	mappedfile = mmap(NULL, mappedsize, PROT_READ, 0, fd, 0);
	if (mappedfile == (char *)-1)
		BAD;

	/*
	 * Make sure we can access the executable's header
	 * directly, and make sure the recognize the executable
	 * as an ELF binary.
	 */
	if (check(0, sizeof *ehdrp))
		BADUNMAP;
	ehdrp = (Elf32_Ehdr *)&mappedfile[0];

	if (memcmp(ehdrp->e_ident, Elf32_e_ident, Elf32_e_siz))
		BADUNMAP;

	switch (ehdrp->e_machine) {
	case Elf32_em_386:
	case Elf32_em_486:
		break;

	default:
		BADUNMAP;
	}

	/*
	 * We've recognized it as an ELF binary.  From here
	 * on out, all errors are fatal.
	 */

	/*
	 * Find the symbol list and string table.
	 */
	nshdr = ehdrp->e_shnum;
	shdr_off = ehdrp->e_shoff;
	shdr_size = ehdrp->e_shentsize * nshdr;

	if (check(shdr_off, shdr_size) ||
	    (sizeof *shdrp != ehdrp->e_shentsize))
		badfmt("bogus section header table");
	shdrp = (Elf32_Shdr *)&mappedfile[shdr_off];

	for (i = 0; i < nshdr; i++) {
		if (shdrp[i].sh_type == Elf_sht_symtab) {
			symshdrp = &shdrp[i];
			symstrshdrp = &shdrp[shdrp[i].sh_link];
		}
	}

	if (symshdrp == NULL || symshdrp->sh_offset == 0)
		badfmt("stripped");
	if (check(symshdrp->sh_offset, symshdrp->sh_size))
		badfmt("bogus symbol section header");
	if (check(symstrshdrp->sh_offset, symstrshdrp->sh_size))
		badfmt("bogus symbol string section header");

	symp = (Elf32_Sym *)&mappedfile[symshdrp->sh_offset];
	nsyms = symshdrp->sh_size / sizeof(*symp);
	strtab = &mappedfile[symstrshdrp->sh_offset];

	/*
	 * Set up the data item, pointing to a nlist structure.
	 * which we fill in for each symbol.
	 */
	data.data = (u_char *)&nbuf;
	data.size = sizeof(nbuf);

	/*
	 * Create a buffer (to be expanded later, if necessary)
	 * to hold symbol names after we've added underscores
	 * to them.
	 */
	symnamesize = 1024;
	if ((symname = malloc(symnamesize)) == NULL)
		err(1, "malloc");

	/*
	 * Read each symbol and enter it into the database.
	 */
	for (i = 0; i < nsyms; i++) {

		/*
		 * No symbol name; ignore this symbol.
		 */
		if (symp[i].st_name == 0)
			continue;

		/*
		 * Find symbol name, copy it (with added underscore) to
		 * temporary buffer, and prepare the database key for
		 * insertion.
		 */
		fsymname = &strtab[symp[i].st_name];
		fsymnamesize = strlen(fsymname) + 1;
		while (symnamesize < fsymnamesize + 1) {
			symnamesize *= 2;
			if ((symname = realloc(symname, symnamesize)) == NULL)
				err(1, "malloc");
		}
		strcpy(symname, "_");
		strcat(symname, fsymname);

		key.data = symname;
		key.size = strlen((char *)key.data);

		/*
		 * Convert the symbol information into an nlist structure,
		 * as best we can.
		 */
		nbuf.n_value = symp[i].st_value;
		switch (ELF_SYM_TYPE(symp[i].st_info)) {
		default:
		case Elf_estt_notype:
			nbuf.n_type = N_UNDF;
			break;
		case Elf_estt_object:
			nbuf.n_type = N_DATA;
			break;
		case Elf_estt_func:
			nbuf.n_type = N_TEXT;
			break;
		case Elf_estt_file:
			nbuf.n_type = N_FN;
			break;
		}
		if (ELF_SYM_BIND(symp[i].st_info) != Elf_estb_local)
			nbuf.n_type |= N_EXT;
		nbuf.n_desc = 0;				/* XXX */
		nbuf.n_other = 0;				/* XXX */

		/*
		 * Enter the symbol into the database.
		 */
		if (db->put(db, &key, &data, 0))
			err(1, "record enter");

		/*
		 * If it's the kernel version string, we've gotta keep
		 * some extra data around.  Under a seperate key,
		 * we enter the first line (i.e. up to the first newline,
		 * with the newline replaced by a NUL to terminate the
		 * entered string) of the version string.
		 */
		if (strcmp((char *)key.data, VRS_SYM) == 0) {
			key.data = (u_char *)VRS_KEY;
			key.size = sizeof(VRS_KEY) - 1;
			/* Find the version string, relative to its section */
			data.data = strdup(&mappedfile[nbuf.n_value -
			    shdrp[symp[i].st_shndx].sh_addr +
			    shdrp[symp[i].st_shndx].sh_offset]);
			/* assumes newline terminates version. */
			if ((tmpcp = strchr(data.data, '\n')) != NULL)
				*tmpcp = '\0';
			data.size = strlen((char *)data.data);

			if (db->put(db, &key, &data, 0))
				err(1, "record enter");

			/* free pointer created by strdup(). */
			free(data.data);

			/* Restore to original values */
			data.data = (u_char *)&nbuf;
			data.size = sizeof(nbuf);
		}
	}

	rv = 0;

unmap:
	munmap(mappedfile, mappedsize);
out:
	return (rv);
}
