#	@(#)Makefile	8.1 (Berkeley) 6/4/93

LIB=	util
# AI-ONLY NOTE: -I/sys removed. It names the kernel source of the
# running system, which on a Linux build host is sysfs; these five
# files include only ordinary headers, so nothing wanted it. NetBSD
# 1.0, NetBSD 1.1 and OpenBSD 1996 all carry this Makefile without it;
# FreeBSD 2.0.5 keeps it.
CFLAGS+=-DLIBC_SCCS
SRCS=	login.c login_tty.c logout.c logwtmp.c pty.c

.include <bsd.lib.mk>
