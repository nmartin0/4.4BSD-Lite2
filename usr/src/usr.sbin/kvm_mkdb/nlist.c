/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)nlist.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include <sys/param.h>

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

typedef struct nlist NLIST;
#define	_strx	n_un.n_strx
#define	_name	n_un.n_name

#define	badfmt(str)	errx(1, "%s: %s: %s", kfile, str, strerror(EFTYPE))

static void badread __P((int, char *));

static char *kfile;

/*
 * AI-ONLY NOTE: one reader per object format, tried in turn, as
 * NetBSD 1.3's kvm_mkdb does it: Berkeley's a.out reader is now
 * create_knlist_aout, and NetBSD 1.3's ELF reader is
 * create_knlist_elf32, in nlist_elf32.c. The table and create_knlist
 * are NetBSD 1.3's nlist.c, with the two entries written in where its
 * extern.h chooses them per machine, and errx in place of its punt(),
 * which this kvm_mkdb.c does not have -- the same exit Berkeley's
 * badfmt makes. The a.out reader now returns -1 for a file that is
 * not a.out, so the next reader can try, with NetBSD 1.3's
 * nlist_aout.c's two lines; before, it exited on `bad magic number',
 * which every ELF kernel has.
 */
static struct {
	int	(*knlist) __P((char *, DB *));
} knlist_fmts[] = {
	{	create_knlist_aout	},
	{	create_knlist_elf32	},
};

void
create_knlist(name, db)
	char *name;
	DB *db;
{
	int i;

	for (i = 0; i < sizeof(knlist_fmts) / sizeof(knlist_fmts[0]); i++)
		if ((*knlist_fmts[i].knlist)(name, db) != -1)
			return;
	errx(1, "%s: file format not recognized", name);
}

int
create_knlist_aout(name, db)
	char *name;
	DB *db;
{
	register int nsyms;
	struct exec ebuf;
	FILE *fp;
	NLIST nbuf;
	DBT data, key;
	int fd, nr, strsize;
	char *strtab, buf[1024];

	kfile = name;
	if ((fd = open(name, O_RDONLY, 0)) < 0)
		err(1, "%s", name);

	/* Read in exec structure. */
	nr = read(fd, &ebuf, sizeof(struct exec));
	if (nr != sizeof(struct exec))
		return (-1);

	/* Check magic number and symbol count. */
	if (N_BADMAG(ebuf))
		return (-1);
	if (!ebuf.a_syms)
		badfmt("stripped");

	/* Seek to string table. */
	if (lseek(fd, N_STROFF(ebuf), SEEK_SET) == -1)
		badfmt("corrupted string table");

	/* Read in the size of the symbol table. */
	nr = read(fd, (char *)&strsize, sizeof(strsize));
	if (nr != sizeof(strsize))
		badread(nr, "no symbol table");

	/* Read in the string table. */
	strsize -= sizeof(strsize);
	if (!(strtab = malloc(strsize)))
		err(1, NULL);
	if ((nr = read(fd, strtab, strsize)) != strsize)
		badread(nr, "corrupted symbol table");

	/* Seek to symbol table. */
	if (!(fp = fdopen(fd, "r")))
		err(1, "%s", name);
	if (fseek(fp, N_SYMOFF(ebuf), SEEK_SET) == -1)
		err(1, "%s", name);
	
	data.data = (u_char *)&nbuf;
	data.size = sizeof(NLIST);

	/* Read each symbol and enter it into the database. */
	nsyms = ebuf.a_syms / sizeof(struct nlist);
	while (nsyms--) {
		if (fread((char *)&nbuf, sizeof (NLIST), 1, fp) != 1) {
			if (feof(fp))
				badfmt("corrupted symbol table");
			err(1, "%s", name);
		}
		if (!nbuf._strx || nbuf.n_type&N_STAB)
			continue;

		key.data = (u_char *)strtab + nbuf._strx - sizeof(long);
		key.size = strlen((char *)key.data);
		if (db->put(db, &key, &data, 0))
			err(1, "record enter");

		if (strcmp((char *)key.data, VRS_SYM) == 0) {
			long cur_off, voff;
#ifndef KERNTEXTOFF
#define KERNTEXTOFF KERNBASE
#endif
			/*
			 * Calculate offset relative to a normal (non-kernel)
			 * a.out.  KERNTEXTOFF is where the kernel is really
			 * loaded; N_TXTADDR is where a normal file is loaded.
			 * From there, locate file offset in text or data.
			 */
			voff = nbuf.n_value - KERNTEXTOFF + N_TXTADDR(ebuf);
			if ((nbuf.n_type & N_TYPE) == N_TEXT)
				voff += N_TXTOFF(ebuf) - N_TXTADDR(ebuf);
			else
				voff += N_DATOFF(ebuf) - N_DATADDR(ebuf);
			cur_off = ftell(fp);
			if (fseek(fp, voff, SEEK_SET) == -1)
				badfmt("corrupted string table");

			/*
			 * Read version string up to, and including newline.
			 * This code assumes that a newline terminates the
			 * version line.
			 */
			if (fgets(buf, sizeof(buf), fp) == NULL)
				badfmt("corrupted string table");

			key.data = (u_char *)VRS_KEY;
			key.size = sizeof(VRS_KEY) - 1;
			data.data = (u_char *)buf;
			data.size = strlen(buf);
			if (db->put(db, &key, &data, 0))
				err(1, "record enter");

			/* Restore to original values. */
			data.data = (u_char *)&nbuf;
			data.size = sizeof(NLIST);
			if (fseek(fp, cur_off, SEEK_SET) == -1)
				badfmt("corrupted string table");
		}
	}
	(void)fclose(fp);
	return (0);
}

static void
badread(nr, p)
	int nr;
	char *p;
{
	if (nr < 0)
		err(1, "%s", kfile);
	badfmt(p);
}
