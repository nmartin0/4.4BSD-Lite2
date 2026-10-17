#Build for this system: hunt.
#
# AI-ONLY NOTE: hunt, built from Berkeley's own sources with the
# settings its Makefile chooses -- DEFS_BSD44 and its GAME_PARAM --
# and installed where that Makefile installs it: hunt in /usr/games,
# huntd in /usr/libexec, and their pages, formatted, in
# /usr/contrib/man/cat6, under ${DESTDIR}, which it does not know.
#
# Not by that Makefile itself, which builds among the sources: its
# rule for the driver's objects compiles $*.c, which only the source
# directory has, so it cannot build in an object directory as
# contrib/ builds here. Its programs, objects and CFLAGS are the same
# below; sys.mk's .c.o finds the sources by .PATH.

BINOWN?=	bin
BINGRP?=	bin

.PATH:		${.CURDIR}

GAME_PARAM=	-DRANDOM -DREFLECT -DMONITOR -DOOZE -DFLY -DVOLCANO -DBOOTS \
		-DOTTO
DEFS_BSD44=	-DINTERNET -DLOG -DBSD_RELEASE=44 -DSIGNAL_TYPE=void
CFLAGS=		-O2 ${GAME_PARAM} ${DEFS_BSD44}

DOBJ=		answer.o driver.o draw.o execute.o expl.o makemaze.o \
		shots.o terminal.o extern.o \
		faketalk.o ctl.o ctl_transact.o get_names.o
POBJ=		hunt.o connect.o playit.o otto.o
HDR=		hunt.h bsd.h talk_ctl.h

all: hunt huntd
	nroff -man ${.CURDIR}/hunt.6 > hunt.0
	nroff -man ${.CURDIR}/huntd.6 > huntd.0

hunt: ${POBJ} pathname.o
	${CC} -o hunt ${POBJ} pathname.o -lcurses -ltermcap

huntd: ${DOBJ} pathname.o
	${CC} -o huntd ${DOBJ} pathname.o

${POBJ} ${DOBJ}: ${HDR}

pathname.o: pathname.c
	${CC} ${CFLAGS} -DHUNTD=\"/usr/libexec/huntd\" -c ${.ALLSRC:M*.c}

install:
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 huntd \
	    ${DESTDIR}/usr/libexec/huntd
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 hunt \
	    ${DESTDIR}/usr/games/hunt
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 hunt.0 huntd.0 \
	    ${DESTDIR}/usr/contrib/man/cat6

clean cleandir:
	rm -f hunt huntd *.o *.0

depend lint obj tags:
