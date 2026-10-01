#	@(#)Makefile	8.1 (Berkeley) 6/5/93

PROG=	lfs_cleanerd
# AI-ONLY NOTE: no -I for the LFS sources. The programs here include
# <ufs/lfs/lfs.h>, which an -I naming .../sys/ufs/lfs cannot resolve:
# that would need .../sys/ufs/lfs/ufs/lfs/lfs.h, which exists in no
# tree. The header reaches them from the installed
# /usr/include/ufs/lfs/lfs.h, and lfs_cksum.c, the source .PATH pulls
# in, includes only <sys/types.h>. The flag resolved nothing in
# Berkeley's tree either. .PATH is what finds the sources and stays.
# OpenBSD 1996 deletes the -I from all three of these Makefiles and
# keeps .PATH; FreeBSD 2.0.5 and NetBSD 1.0 keep the dead flag.
CFLAGS+=-I${.CURDIR} -DDIAGNOSTIC
MAN8=	lfs_cleanerd.0
SRCS=	cleanerd.c lfs_cksum.c library.c misc.c print.c

.PATH:	${.CURDIR}/../../sys/ufs/lfs

.include <bsd.prog.mk>
