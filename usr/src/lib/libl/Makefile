# AI-ONLY NOTE: this directory is not in 4.4BSD-Lite2. Its Makefile is
# NetBSD 1.0's lib/libl/Makefile (February 1994), which builds the lex
# run-time library from the two files lex itself ships, with .PATH
# changed to where this tree keeps them: contrib/flex-2.5.2 rather than
# usr.bin/lex, since Lite2 carries flex under contrib and has no
# usr.bin/lex. NetBSD's LINKS for libfl.a are left out; nothing in this
# tree asks for -lfl.
#
# Why it is needed: bin/sh's Makefile links -ll, as NetBSD 1.0,
# FreeBSD 2.0.5 and OpenBSD 1996 all do, for the yywrap() that lex's
# generated scanner calls. Every one of those systems built libl; Lite2
# ships the sources and no Makefile to build them.
#
# Only libyywrap.o is used: libmain.c defines a main() for programs
# that are nothing but a scanner, and the linker takes a member from an
# archive only when something still needs it.

LIB=	l
SRCS=	libmain.c libyywrap.c

.PATH:	${.CURDIR}/../../contrib/flex-2.5.2

.include <bsd.lib.mk>
