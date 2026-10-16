/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * This file deliberately carries no SPDX licence identifier: the
 * licence for this tree's own scripts is not settled yet, and the tag
 * is kept out rather than written into a sentence, where a scanner
 * would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
 * cover the Berkeley material, not this file.
 *
 * AI-ONLY NOTE: the host side of building usr.bin/cap_mkdb, for
 * build/sysroot.sh, as build/elf-host.h is for crunchide. Two
 * functions.
 *
 * cap_mkdb reads its input through lib/libc/gen/getcap.c, whose
 * cgetnext() calls fgetln(3), a 4.4BSD stdio function glibc does not
 * have. This is fgetln as its manual page describes it, written over
 * POSIX getline(3): the next line, its length counting the newline
 * if there is one, no NUL added, valid until the next call. One
 * buffer serves every stream, which is all cap_mkdb asks: it reads
 * one file at a time.
 *
 * And fclose, under the name getcap_fclose, which sysroot.sh defines
 * fclose to while compiling getcap.c: at the end of each file
 * cgetnext() calls fclose(pfp) and then ferror(pfp), and at the end of
 * the last, cgetclose(), which closes pfp again -- as Lite2's getcap.c
 * does, and as FreeBSD's, NetBSD's and OpenBSD's of the 1990s still
 * did. On 4.4BSD that is harmless: fclose frees the buffer and marks
 * the FILE unused, and the FILE, one of a table stdio keeps, is still
 * there to be asked. glibc frees the FILE, and the second fclose
 * aborts the program. So the close is put off until the next one: a
 * FILE closed here stays open, and answerable to ferror(), until
 * another is closed, and closing the same one twice does nothing.
 *
 * Nothing here is copied from another system.
 */
#include <sys/types.h>
#include <stdio.h>

char *
fgetln(FILE *fp, size_t *lenp)
{
	static char *buf;
	static size_t size;
	ssize_t n;

	if ((n = getline(&buf, &size, fp)) <= 0)
		return (NULL);
	*lenp = (size_t)n;
	return (buf);
}

int
getcap_fclose(FILE *fp)
{
	static FILE *last;
	FILE *prev;

	if (fp == last)
		return (0);
	prev = last;
	last = fp;
	if (fflush(fp) == EOF)
		return (EOF);
	return (prev != NULL ? fclose(prev) : 0);
}
