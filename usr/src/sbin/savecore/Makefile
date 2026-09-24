#	@(#)Makefile	8.2 (Berkeley) 4/17/94

PROG=	savecore
SRCS=	savecore.c zopen.c
MAN8=	savecore.0
# AI-ONLY NOTE: no .PATH any more. It named usr.bin/compress, which is
# not in this tree: 4.4BSD-Lite and Lite2 both list compress in
# usr.bin's SUBDIR and ship no such directory. zopen.c now sits beside
# savecore.c, which is where OpenBSD keeps its own copy.

.include <bsd.prog.mk>
