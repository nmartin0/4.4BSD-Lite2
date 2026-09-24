#	@(#)Makefile	8.1 (Berkeley) 6/4/93

LIB=compat
CFLAGS+=-DLIBC_SCCS -DSYSLIBC_SCCS
AINC=	-I${.CURDIR}/../libc/${MACHINE}

.PATH:	${.CURDIR}/4.1/${MACHINE} ${.CURDIR}/4.1 \
	${.CURDIR}/4.3/${MACHINE} ${.CURDIR}/4.3 \
	${.CURDIR}/4.4/${MACHINE} ${.CURDIR}/4.4 \
	${.CURDIR}/regexp

# compat 4.1 sources
# AI-ONLY NOTE: this tree ships none of the 4.1 sources -- lib/compat/4.1
# holds five manual pages and no code -- so none is named here. NetBSD
# 1.0 and OpenBSD 1996 name gtty.c, ftime.c and stty.c, the three they
# have, and comment the rest out; the same practice, applied to what is
# in this tree.
SRCS=

.if (${MACHINE} == "vax")
SRCS+=	reset.s
.endif

MAN3+=	ftime.0 getpw.0 stty.0 vlimit.0 vtimes.0

MLINKS+=stty.3 gtty.3

# compat 4.3 sources
# AI-ONLY NOTE: ecvt.c, gcvt.c, sibuf.c, sobuf.c and strout.c are not in
# this tree and are not named. regex.c is here now, imported from
# NetBSD 1.0, because usr.bin/more and usr.bin/rdist call re_comp and
# re_exec and nothing else in this tree defines them; 4.4BSD-Lite2
# ships its manual page, 4.3/re_comp.3, and no code. NetBSD 1.0 names
# cfree.c, lsearch.c, regex.c and rexec.c here, which is now the same
# list; OpenBSD 1996 drops cfree.c as well.
#
# regex.c and regexp/regerror.c both define regerror. NetBSD 1.0 builds
# both into this library too: an archive may hold two definitions, and
# a link fails only if something pulls in both members.
SRCS+=	cfree.c lsearch.c regex.c rexec.c

.if	(${MACHINE} == "hp300" || ${MACHINE} == "luna68k")
SRCS+=	insque.s remque.s
.elif	(${MACHINE} == "i386")
SRCS+=	insque.c remque.c
.elif	(${MACHINE} == "mips")
SRCS+=	insque.c remque.c
.elif	(${MACHINE} == "sparc")
SRCS+=	insque.c remque.c
.elif	(${MACHINE} == "tahoe")
SRCS+=	nargs.s insque.s remque.s
.elif	(${MACHINE} == "vax")
SRCS+=	nargs.s insque.s remque.s
.endif

MAN3+=	ecvt.0 insque.0 lsearch.0 re_comp.0 rexec.0

MLINKS+=ecvt.3 fcvt.3 ecvt.3 gcvt.3
MLINKS+=insque.3 remque.3
MLINKS+=re_comp.3 re_exec.3

# compat 4.4 sources
SRCS+=	cuserid.c

# regexp sources
SRCS+=	regerror.c regexp.c regsub.c

MAN3+=	regexp.0

MLINKS+=regexp.3 regcomp.3 regexp.3 regexec.3 regexp.3 regsub.3 \
	regexp.3 regerror.3

.include <bsd.lib.mk>
