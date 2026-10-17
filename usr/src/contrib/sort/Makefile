#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	sort
SRCS=	append.c fields.c files.c fsort.c init.c msort.c sort.c tmp.c

# AI-ONLY NOTE: installed, by the maintainer's choice, as the system's
# sort, in /usr/bin with its page beside the system's: the Lite cut
# emptied usr.bin/sort, leaving only the old program's page, and this
# is Berkeley's own new sort, Peter McIlroy's of 1993, which Lite2
# ships here off contrib/Makefile's list -- the sort NetBSD's
# usr.bin/sort descends from. bib's invert, among others, runs it.
BINDIR=	/usr/bin
MANDIR=	/usr/share/man/cat

.include <bsd.prog.mk>
