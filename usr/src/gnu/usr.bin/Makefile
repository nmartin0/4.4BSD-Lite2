#	$Id$

# AI-ONLY NOTE: new, after FreeBSD 2.2.8's gnu/usr.bin/Makefile, whose
# list this cuts to gdb, the one GNU program built here from a makefile
# of the BSD's own; the others of this tree's GNU programs build from
# contrib/. For the i386 only, as this tree has the one port and gdb's
# nm.h, tm.h and xm.h here are its.
.if ${MACHINE} == "i386"
SUBDIR= gdb
.endif

.include <bsd.subdir.mk>
