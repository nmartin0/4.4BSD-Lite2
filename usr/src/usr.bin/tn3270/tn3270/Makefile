#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	tn3270
CFLAGS+=-I${.CURDIR} -I.
LDADD+=	-lcurses -ltermcap -ltelnet
# AI-ONLY NOTE: the libraries in ${DESTDIR}, where the rest of this
# tree's Makefiles find them through bsd.prog.mk's LIB* variables;
# /usr/lib here was the build machine's, which is not the system's
# when the build is not native.
DPADD+=	${LIBCURSES} ${LIBTERM} ${DESTDIR}/usr/lib/libtelnet.a
CLEANFILES+= asc_disp.OUT asc_disp.out disp_asc.OUT disp_asc.out TMPfunc.out
.PATH:	${.CURDIR}/../api ${.CURDIR}/../ascii ${.CURDIR}/../ctlr
.PATH:	${.CURDIR}/../general ${.CURDIR}/../sys_curses ${.CURDIR}/../../telnet

MAN1=	tn3270.0

SRCS+=	apilib.c api_bsd.c api_exch.c asc_ebc.c astosc.c dctype.c
SRCS+=	disp_asc.c ebc_disp.c
SRCS+=	map3270.c termin.c
SRCS+=	api.c function.c inbound.c oia.c options.c outbound.c
SRCS+=	genbsubs.c globals.c system.c termout.c
SRCS+=	commands.c main.c network.c ring.c sys_bsd.c telnet.c terminal.c
SRCS+=	tn3270.c utilities.c

# This and the dependency hacks below to make 'depend' target
# work right...

DEPSRCS+= astosc.OUT asc_disp.OUT disp_asc.OUT kbd.OUT
DEPSRCS+= apilib.c api_bsd.c api_exch.c asc_ebc.c dctype.c
DEPSRCS+= ebc_disp.c
DEPSRCS+= map3270.c termin.c
DEPSRCS+= api.c function.c inbound.c oia.c options.c outbound.c
DEPSRCS+= genbsubs.c globals.c system.c termout.c
DEPSRCS+= commands.c main.c network.c ring.c sys_bsd.c telnet.c terminal.c
DEPSRCS+= tn3270.c utilities.c

# AI-ONLY NOTE: the generators are found where make built them, as
# NetBSD 1.1's Makefile finds them (asking make for the tool's
# .OBJDIR), and not at ${.CURDIR}/../tools/X/obj, a symbolic link
# into /usr/obj that only a native build of Berkeley's tree makes.
MKASTOSC!=cd ${.CURDIR}/../tools/mkastosc; \
	printf "xxx:\n\techo \$${.OBJDIR}/mkastosc\n" | ${MAKE} -r -s -f - xxx
MKASTODS!=cd ${.CURDIR}/../tools/mkastods; \
	printf "xxx:\n\techo \$${.OBJDIR}/mkastods\n" | ${MAKE} -r -s -f - xxx
MKDSTOAS!=cd ${.CURDIR}/../tools/mkdstoas; \
	printf "xxx:\n\techo \$${.OBJDIR}/mkdstoas\n" | ${MAKE} -r -s -f - xxx
MKHITS!=cd ${.CURDIR}/../tools/mkhits; \
	printf "xxx:\n\techo \$${.OBJDIR}/mkhits\n" | ${MAKE} -r -s -f - xxx

astosc.o: astosc.OUT
CLEANFILES+= astosc.OUT astosc.out
astosc.OUT: ${.CURDIR}/../ctlr/hostctlr.h ${.CURDIR}/../ctlr/function.h
astosc.OUT: ${.CURDIR}/../ctlr/${KBD} ${MKASTOSC}
	${MKASTOSC} \
	    ${.CURDIR}/../ctlr/hostctlr.h \
	    ${.CURDIR}/../ctlr/function.h < ${.CURDIR}/../ctlr/${KBD} \
	    > ${.TARGET}
	rm -f astosc.out; ln -s astosc.OUT astosc.out

disp_asc.o: asc_disp.OUT disp_asc.OUT
asc_disp.OUT: ${MKASTODS}
	${MKASTODS} > ${.TARGET}
	rm -f asc_disp.out; ln -s asc_disp.OUT asc_disp.out

disp_asc.OUT: ${MKDSTOAS}
	${MKDSTOAS} > ${.TARGET}
	rm -f disp_asc.out; ln -s disp_asc.OUT disp_asc.out

inbound.o: kbd.OUT
CLEANFILES += kbd.OUT kbd.out
kbd.OUT: ${.CURDIR}/../ctlr/hostctlr.h ${.CURDIR}/../ctlr/${KBD}
kbd.OUT: ${MKHITS}
	${CC} ${CFLAGS} -E ${.CURDIR}/../ctlr/function.c > TMPfunc.out
	${MKHITS} ${.CURDIR}/../ctlr/hostctlr.h \
	    TMPfunc.out < ${.CURDIR}/../ctlr/${KBD} > ${.TARGET}
	rm -f kbd.out; ln -s kbd.OUT kbd.out

# astosc.out
# asc_disp.out disp_asc.out
# default.map
# kbd.out

${MKASTOSC}:
	cd ${.CURDIR}/../tools/mkastosc; ${MAKE}
${MKASTODS}:
	cd ${.CURDIR}/../tools/mkastods; ${MAKE}
${MKDSTOAS}:
	cd ${.CURDIR}/../tools/mkdstoas; ${MAKE}
${MKHITS}:
	cd ${.CURDIR}/../tools/mkhits; ${MAKE}

depend: .depend
.depend: ${DEPSRCS}
	mkdep ${MKDEP} ${CFLAGS:M-[ID]*} ${.ALLSRC:M*.c}

.include <../../Makefile.inc>
.include <bsd.prog.mk>
