# AI-ONLY NOTE: Berkeley's makefile here had three rules -- all, which
# ran "make -f Makefile bsd44c", and clean and install, which ran the
# Makefile's own. Kermit's Makefile builds among the sources, and runs
# ./wart, which it builds with the same compiler as kermit, to make
# ckcpro.c; neither can be done in an object directory with a
# compiler for this system on another machine, as contrib/ builds
# here. So these rules do what bsd44c and install do, with the same
# objects, CFLAGS and LIBS: wart is built with ${HOSTCC}, as
# HOSTCC builds what a build runs on its own machine, and its output
# was compared with the build machine's CFLAGS and with none: the same
# ckcpro.c. kermit is installed as /usr/contrib/bin/kermit, its page,
# formatted, in /usr/contrib/man/cat1, under ${DESTDIR}.

BINOWN?=	bin
BINGRP?=	bin

.PATH:		${.CURDIR}

CFLAGS=		-DBSD44 -DCK_CURSES -DDYNAMIC -DTCPSOCKET -O
LIBS=		-lcurses -ltermcap

OBJS=		ckcmai.o ckutio.o ckufio.o ckcfns.o ckcfn2.o ckcfn3.o \
		ckuxla.o ckcpro.o ckucmd.o ckuus2.o ckuus3.o ckuus4.o \
		ckuus5.o ckuus6.o ckuus7.o ckuusx.o ckuusy.o ckuusr.o \
		ckucon.o ckudia.o ckuscr.o ckcnet.o

all: wermit kermit.0

wermit: ${OBJS}
	${CC} -o wermit ${OBJS} ${LIBS}

wart: ckwart.c
	${HOSTCC} ${CFLAGS} -o wart ${.ALLSRC}

ckcpro.c: ckcpro.w wart
	./wart ${.CURDIR}/ckcpro.w ckcpro.c
ckcpro.o: ckcpro.c
	${CC} ${CFLAGS} -I${.CURDIR} -c ckcpro.c

${OBJS}: ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h ckuxla.h ckcnet.h \
	ckucmd.h ckuusr.h ckuver.h

kermit.0: ckuker.nr
	nroff -man ${.ALLSRC} > kermit.0

install:
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 wermit \
	    ${DESTDIR}/usr/contrib/bin/kermit
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 kermit.0 \
	    ${DESTDIR}/usr/contrib/man/cat1/kermit.0

clean cleandir:
	rm -f wermit wart ckcpro.c *.o kermit.0

depend lint obj tags:
