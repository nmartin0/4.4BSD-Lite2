#	@(#)Makefile	8.1 (Berkeley) 6/4/93

# AI-ONLY NOTE: changed (2026-10-25) -- built to be run by inetd, as
# inetd.conf's (commented) uucpd line runs it: NetBSD 1.0's Makefile
# has CFLAGS+= -DBSDINETD, and FreeBSD 2.0's. Without it uucpd is the
# old stand-alone daemon, which listened only under BSD4_2.
CFLAGS+=-DBSDINETD
PROG=	uucpd
NOMAN=	noman

.include <bsd.prog.mk>
