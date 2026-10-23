#	@(#)Makefile	8.2 (Berkeley) 9/21/96

PROG=	praliases
# AI-ONLY NOTE: the manual page by its formatted page's name, as
# Lite2's bsd.man.mk installs it, where FreeBSD 2.2's names the source.
MAN8=	praliases.0
CFLAGS+=-I${.CURDIR}/../src -DNEWDB

.include "../../Makefile.inc"
.include <bsd.prog.mk>
