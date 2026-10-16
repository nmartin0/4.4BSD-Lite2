#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	phantasia
SRCS=	main.c fight.c io.c interplayer.c gamesupport.c misc.c phantglobs.c
DPADD=	${LIBM} ${LIBCURSES} ${LIBTERM} ${LIBCOMPAT}
LDADD=	-lm -lcurses -ltermlib -lcompat
HIDEGAME=hidegame
MAN6=	phantasia.0
CLEANFILES+=map setup setup.o gold lastdead mess monsters motd characs \
	scoreboard void

all: setup phantasia ${MAN6}

# AI-ONLY NOTE: setup built with ${HOSTCC}, for the machine doing the
# build, which runs it: as adventure's setup is here, and as FreeBSD
# 4.0 builds this one among its build-tools. It writes the game's
# files where it runs, by their names alone (pathnames.h, _SPATH_),
# and they are installed under ${DESTDIR}, as FreeBSD 2.0's Makefile
# installs them; Berkeley's setup wrote /var/games/phantasia on the
# machine running it, whatever the DESTDIR.
setup: setup.c phantglobs.c monsters.asc
	${HOSTCC} -I${.CURDIR} -o ${.TARGET} ${.CURDIR}/setup.c \
	    ${.CURDIR}/phantglobs.c -lm

beforeinstall: 
	./setup -m ${.CURDIR}/monsters.asc
	install -c -m 660 -o games -g bin gold ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin lastdead ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin mess ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin monsters ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin motd ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin characs ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin scoreboard ${DESTDIR}/var/games/phantasia
	install -c -m 660 -o games -g bin void ${DESTDIR}/var/games/phantasia

# Make Phantasia map.  Change the map commands reflect your installation.
# PLOTDEVICE is used for plotting the map.  Change as appropriate.

map: map.c
	${CC} -O ${.CURDIR}/map.c -lplot -o ${.TARGET}
	./map | plot > /dev/tty

phantasia.0: phantasia.6
	tbl ${.CURDIR}/phantasia.6 | nroff -man > ${.TARGET}

.include <bsd.prog.mk>
