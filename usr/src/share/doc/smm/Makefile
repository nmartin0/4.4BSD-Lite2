#	@(#)Makefile	8.2 (Berkeley) 5/10/94

# The following modules do not build/install:
# 10.named, 13.amd

BINDIR=	/usr/share/doc/smm
FILES=	Makefile Title
SUBDIR=	01.setup 02.config 03.fsck 04.quotas 05.fastfs 06.nfs 07.lpd \
	08.sendmailop 09.sendmail 11.timedop 12.timed 14.uucpimpl \
	15.uucpnet 16.security 17.password 18.net 19.perl

Title.ps: ${FILES}
	groff -ms Title > ${.TARGET}

# AI-ONLY NOTE: from ${.CURDIR}, as games/fortune/datfiles installs
# its files: the build runs this in an object directory, which this
# directory has as the parent of its subdirectories' (build/make.sh).
beforeinstall:
	(cd ${.CURDIR} && install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${FILES} ${DESTDIR}${BINDIR})

.include <bsd.subdir.mk>
