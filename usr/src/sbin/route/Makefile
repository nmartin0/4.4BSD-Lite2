#	@(#)Makefile	8.1 (Berkeley) 6/5/93

PROG=	route
MAN8=	route.0
SRCS=	route.c ccitt_addr.c
CFLAGS+=-I.
CLEANFILES+=keywords.h
BINOWN=	root
BINMODE=4555

all: route ${MAN8}

keywords.h: keywords
	sed -e '/^#/d' -e '/^$$/d' ${.CURDIR}/keywords > _keywords.tmp
	tr a-z A-Z < _keywords.tmp | paste _keywords.tmp - | \
	    awk '{ \
		if (NF > 1) \
			printf "#define\tK_%s\t%d\n\t{\"%s\", K_%s},\n", \
			    $$2, NR, $$1, $$2 }' \
	    > ${.TARGET}
	rm -f _keywords.tmp

.include <bsd.prog.mk>

route .depend lint tags: keywords.h
# AI-ONLY NOTE: the object as well. 4.4BSD generated keywords.h during
# `make depend', through the line above; this tree's build runs `make'
# without a depend pass first, so route.c was compiled before the
# header existed: `keywords.h: No such file or directory'. FreeBSD 2.0
# puts keywords.h in SRCS, which its bsd.prog.mk filters out of OBJS
# and this tree's does not, so the dependency is said directly.
route.o: keywords.h
