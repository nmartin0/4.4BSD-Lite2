#Build for this system: jove.
#
# AI-ONLY NOTE: jove 4.14.6, as Peter Collinson ported it to BSD/386
# and Berkeley shipped it, built with the settings its Makefile (a
# link to Makefile.bsdi) chooses -- SYSDEFS=-DBSD386, -ltermcap, and
# JOVEHOME /usr/contrib -- and installed where that Makefile installs
# it: jove and teachjove in /usr/contrib/bin; recover, portsrv and kbd
# in /usr/contrib/lib/jove, which etc/mtree/4.4BSD.usr makes, beside
# jove.rc, cmds.doc and teach-jove; and the pages, formatted, in
# /usr/contrib/man/cat1. etc/rc.local already runs recover at boot.
# Under ${DESTDIR}, which that Makefile does not know.
#
# Not by that Makefile itself, which builds among the sources: it
# compiles iproc.c, setmaps.c and teachjove.c by name and reads
# keys.txt and tune.template from the directory it runs in, so it
# cannot build in an object directory as contrib/ builds here. Its
# programs, objects, CFLAGS and generated files are the same below.
#
# setmaps, which writes keys.c, is compiled with LOCALCC, which the
# Makefile provides "for cross compiling Jove ... the local C
# compiler": ${HOSTCC} here. With -ansi: setmaps includes jove's io.h,
# which declares a getline() of its own, and the GNU C library of a
# Linux build machine declares another unless asked for strict ANSI C;
# this system's headers take -ansi as they are. The command table it
# indexes was compared, the build machine's preprocessor against
# this system's: the same 214 commands, in the same order.
#
# cmds.doc is installed as Berkeley shipped it, already formatted;
# the Makefile remakes it with nroff only when doc/cmds.doc.nr is newer.

BINOWN?=	bin
BINGRP?=	bin

.PATH:		${.CURDIR}

TMPDIR=		/var/tmp
RECDIR=		/var/preserve
JOVEHOME=	/usr/contrib
SHAREDIR=	${JOVEHOME}/lib/jove
LIBDIR=		${JOVEHOME}/lib/jove
BINDIR=		${JOVEHOME}/bin
MANDIR=		${JOVEHOME}/man/cat1
DFLTSHELL=	/bin/csh

SYSDEFS=	-DBSD386
CFLAGS=		-O ${SYSDEFS}
LIBS=		-ltermcap
LOCALCC=	${HOSTCC} -ansi
LOCALCFLAGS=	-O ${SYSDEFS}

BASESEG=	funcdefs.o keys.o argcount.o ask.o buf.o ctype.o delete.o \
		disp.o insert.o io.o jove.o loadavg.o marks.o misc.o re.o \
		screen.o tune.o util.o vars.o version.o list.o keymaps.o
OVLAY1=		abbrev.o rec.o paragraph.o fmt.o
OVLAY2=		c.o wind.o fp.o move.o
OVLAY3=		extend.o macros.o
OVLAY4=		iproc.o re1.o
OVLAY5=		proc.o scandir.o term.o case.o
OBJECTS=	${BASESEG} ${OVLAY1} ${OVLAY2} ${OVLAY3} ${OVLAY4} ${OVLAY5}

all: xjove recover teachjove portsrv kbd macvert jove.0 teachjove.0

xjove: ${OBJECTS}
	${CC} -o xjove ${OBJECTS} ${LIBS}
portsrv: portsrv.o
	${CC} -o portsrv portsrv.o ${LIBS}
kbd: kbd.o
	${CC} -o kbd kbd.o ${LIBS}
recover: rectune.h recover.o tune.o
	${CC} -o recover recover.o tune.o ${LIBS}
teachjove: teachjove.o
	${CC} -o teachjove teachjove.o ${LIBS}
macvert: macvert.c
	${CC} ${CFLAGS} -o macvert ${.ALLSRC}

teachjove.o: teachjove.c
	${CC} -c ${CFLAGS} -DTEACHJOVE=\"${SHAREDIR}/teach-jove\" ${.ALLSRC}

setmaps: setmaps.c funcdefs.c
	${LOCALCC} ${LOCALCFLAGS} -I${.CURDIR} -o setmaps \
	    ${.CURDIR}/setmaps.c

keys.c: setmaps keys.txt
	-./setmaps < ${.CURDIR}/keys.txt > keys.c
keys.o: keys.c jove.h
	${CC} ${CFLAGS} -I${.CURDIR} -c keys.c

tune.c: tune.template
	rm -f tune.c
	echo "/* Changes should be made in Makefile, not to this file! */" \
	    > tune.c
	echo "" >> tune.c
	sed -e 's;TMPDIR;${TMPDIR};' \
	    -e 's;LIBDIR;${LIBDIR};' \
	    -e 's;SHAREDIR;${SHAREDIR};' \
	    -e 's;BINDIR;${BINDIR};' \
	    -e 's;SHELL;${DFLTSHELL};' ${.CURDIR}/tune.template >> tune.c
tune.o: tune.c
	${CC} ${CFLAGS} -I${.CURDIR} -c tune.c

rectune.h:
	rm -f rectune.h
	echo "/* Changes should be made in Makefile, not to this file! */" \
	    > rectune.h
	echo "" >> rectune.h
	echo \#define TMP_DIR \"${TMPDIR}\" >> rectune.h
	echo \#define REC_DIR \"${RECDIR}\" >> rectune.h
recover.o: rectune.h
	${CC} ${CFLAGS} -I. -c ${.CURDIR}/recover.c

iproc.o: iproc-ptys.c iproc-pipes.c iproc.c
	${CC} -c ${CFLAGS} ${.CURDIR}/iproc.c

jove.0: doc/jove.nr
	sed -e 's;TMPDIR;${TMPDIR};' \
	    -e 's;LIBDIR;${LIBDIR};' \
	    -e 's;SHAREDIR;${SHAREDIR};' \
	    -e 's;SHELL;${DFLTSHELL};' ${.CURDIR}/doc/jove.nr | \
	    nroff -mandoc > jove.0
teachjove.0: doc/teachjove.nr
	sed -e 's;TMPDIR;${TMPDIR};' \
	    -e 's;LIBDIR;${LIBDIR};' \
	    -e 's;SHAREDIR;${SHAREDIR};' \
	    -e 's;SHELL;${DFLTSHELL};' ${.CURDIR}/doc/teachjove.nr | \
	    nroff -mandoc > teachjove.0

install:
	install -c -o ${BINOWN} -g ${BINGRP} -m 644 \
	    ${.CURDIR}/doc/teach-jove ${DESTDIR}${SHAREDIR}/teach-jove
	install -c -o ${BINOWN} -g ${BINGRP} -m 644 \
	    ${.CURDIR}/doc/cmds.doc ${DESTDIR}${SHAREDIR}/cmds.doc
	install -c -o ${BINOWN} -g ${BINGRP} -m 644 \
	    ${.CURDIR}/doc/system.rc ${DESTDIR}${SHAREDIR}/jove.rc
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 755 portsrv \
	    ${DESTDIR}${LIBDIR}/portsrv
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 755 kbd \
	    ${DESTDIR}${LIBDIR}/kbd
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 755 recover \
	    ${DESTDIR}${LIBDIR}/recover
	install -c -o ${BINOWN} -g ${BINGRP} -m 755 xjove \
	    ${DESTDIR}${BINDIR}/jove
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 755 teachjove \
	    ${DESTDIR}${BINDIR}/teachjove
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 jove.0 teachjove.0 \
	    ${DESTDIR}${MANDIR}

clean cleandir:
	rm -f *.o keys.c tune.c xjove portsrv kbd recover setmaps \
	    teachjove macvert rectune.h *.0

depend lint obj tags:
