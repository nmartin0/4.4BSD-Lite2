#	@(#)Makefile	8.2 (Berkeley) 4/2/94

PROG=	chpass
SRCS=	chpass.c edit.c field.c pw_copy.c pw_scan.c pw_util.c table.c util.c
BINOWN=	root
BINMODE=4555
.PATH:	${.CURDIR}/../../usr.sbin/pwd_mkdb ${.CURDIR}/../../usr.sbin/vipw
CFLAGS+=-I${.CURDIR}/../../usr.sbin/pwd_mkdb -I${.CURDIR}/../../usr.sbin/vipw
LINKS=	${BINDIR}/chpass ${BINDIR}/chfn ${BINDIR}/chpass ${BINDIR}/chsh
MLINKS=	chpass.1 chfn.1 chpass.1 chsh.1

# AI-ONLY NOTE: only when installing onto the running system, which is
# the one /usr/bin/chpass names. Into a DESTDIR -- a root being made
# for another machine -- it would set the flag on the build machine's
# chpass, or fail there, and a file's flags do not travel in the
# distribution's ustar archive anyway; the installer sets this one on
# the new system (distrib/i386/floppies/inst/dot.instutils).
afterinstall:
.if empty(DESTDIR)
	chflags schg /usr/bin/chpass
.endif

.include <bsd.prog.mk>
