#	@(#)Makefile	8.4 (Berkeley) 6/10/97

PROG=	makemap
# AI-ONLY NOTE: the manual page by its formatted page's name, as
# Lite2's bsd.man.mk installs it, where FreeBSD 2.2's names the source.
MAN8=	makemap.0
CFLAGS+=-I${.CURDIR}/../src -DNEWDB -DNOT_SENDMAIL

SRCS=	makemap.c safefile.c

safefile.c: ${.CURDIR}/../src/safefile.c
	ln -s ${.CURDIR}/../src/safefile.c

.include "../../Makefile.inc"
.include <bsd.prog.mk>
