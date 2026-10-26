# Makefile for cu
# $Id: Makefile,v 1.3 1995/08/24 05:19:28 jtc Exp $

BINDIR=		$(bindir)
BINOWN=		$(owner)
# AI-ONLY NOTE: changed (2026-10-25) -- the dialer group alone may run
# cu, as it alone may run tip (usr.bin/tip/Makefile, 4510), and as
# Lite2's cu, tip under another name, was run: OpenBSD's Makefile
# (de Raadt, 3 September 1996, "like tip, group dialer access only").
# NetBSD 1.1's installs it 4555.
BINGRP=		dialer
BINMODE=	4554

PROG=		cu
SRCS=		cu.c prot.c log.c chat.c conn.c copy.c
LDADD+=		$(LIBUNIX) $(LIBUUCONF) $(LIBUUCP)
DPADD+=		$(LIBUNIX) $(LIBUUCONF) $(LIBUUCP)
CFLAGS+=	-I$(.CURDIR)/../common_sources\
		-DVERSION=\"$(VERSION)\"

.include <bsd.prog.mk>
.PATH: $(.CURDIR)/../common_sources
