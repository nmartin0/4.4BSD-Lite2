#	@(#)Makefile	8.1 (Berkeley) 6/18/93

PROG=	dumplfs
# AI-ONLY NOTE: no -I for the LFS sources. The programs here include
# <ufs/lfs/lfs.h>, which an -I naming .../sys/ufs/lfs cannot resolve:
# that would need .../sys/ufs/lfs/ufs/lfs/lfs.h, which exists in no
# tree. The header reaches them from the installed
# /usr/include/ufs/lfs/lfs.h, and lfs_cksum.c, the source .PATH pulls
# in, includes only <sys/types.h>. The flag resolved nothing in
# Berkeley's tree either. .PATH is what finds the sources and stays.
# OpenBSD 1996 deletes the -I from all three of these Makefiles and
# keeps .PATH; FreeBSD 2.0.5 and NetBSD 1.0 keep the dead flag.
SRCS=	dumplfs.c lfs_cksum.c misc.c
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the
# kernel source of the running system -- on a Linux build host,
# sysfs. FreeBSD 2.0.5 and OpenBSD 1996 both write this .PATH that
# way; NetBSD 1.0 writes ${DESTDIR}/sys. That holds for .PATH only:
# the note this replaces said the same of a -I line, which neither
# of them writes.
.PATH:	${.CURDIR}/../../sys/ufs/lfs
MAN8=	dumplfs.0

.include <bsd.prog.mk>
