#	@(#)Makefile	8.1 (Berkeley) 7/2/95

PROG=	smrsh
# AI-ONLY NOTE: the manual page by its formatted page's name, as
# Lite2's bsd.man.mk installs it, where FreeBSD 2.2's names the source.
MAN8=	smrsh.0
CFLAGS+=-I${.CURDIR}/../src -DNEWDB

BINDIR=	/usr/libexec
.include <bsd.prog.mk>
