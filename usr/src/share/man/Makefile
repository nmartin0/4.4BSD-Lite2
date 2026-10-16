#	@(#)Makefile	8.2 (Berkeley) 4/16/94

SUBDIR=	man1 man3 man3f man4 man5 man7 man8
SEDF= /usr/share/man/makewhatis.sed

# AI-ONLY NOTE: makewhatis.sed installed from ${.CURDIR}, as
# games/fortune/datfiles installs its files: the build runs this in an
# object directory, which this directory has as the parent of its
# subdirectories' (build/make.sh). And makedb reads the pages under
# ${DESTDIR}, the ones just installed, as FreeBSD 2.0's, NetBSD 1.0's
# and OpenBSD's makedb do, and not the build machine's /usr/share/man.
afterinstall:
	(cd ${.CURDIR} && install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    makewhatis.sed ${DESTDIR}${SEDF})

makedb:
	find ${DESTDIR}/usr/share/man -type f -name '*.0' -print | \
	while read file; \
	do \
		sed -n -f ${DESTDIR}${SEDF} $$file; \
	done | col -b | sort -u > /tmp/whatis.db
	install -o ${BINOWN} -g ${BINGRP} -m 444 /tmp/whatis.db \
	    ${DESTDIR}/usr/share/man

.include <bsd.subdir.mk>
