# AI-ONLY NOTE: new (2026-10-27), for GDB 4.16's own readline, GNU
# readline 2.0, in contrib/gdb-4.16/readline, whose Makefile.in lists
# the objects. FreeBSD 3.0 links gdb with its own libreadline
# (lib/libreadline) and NetBSD 1.3 builds the dist's readline in
# gnu/lib/readline into a library its gdb links; this tree has no
# readline anywhere else, and gdb is its only user. Made as NetBSD
# 1.3's gnu/lib/readline/Makefile makes its library, which is never
# installed (its `libinstall:' is empty), and as FreeBSD 3.0's
# bfd/Makefile here.
LIB=	readline

GDBDIR=	${.CURDIR}/../../../../contrib/gdb-4.16
.PATH:	${GDBDIR}/readline

SRCS=	readline.c vi_mode.c funmap.c keymaps.c parens.c search.c \
	rltty.c complete.c bind.c isearch.c display.c signals.c \
	tilde.c history.c xmalloc.c

CFLAGS+= -I. -I${.CURDIR} -I${GDBDIR}/readline -I${GDBDIR}/include

# readline's configure links sysdep.h to the sysdep-*.h for the system
# (sysdep-norm.h is the one for any not named); this is that, made here.
CLEANFILES+=	sysdep.h
${SRCS:R:S/$/.o/g} .depend: sysdep.h
sysdep.h:	sysdep-norm.h
	ln -sf ${.ALLSRC} ${.TARGET}

NOPROFILE=no
NOPIC=no
NOMAN=

install:
	@echo -n

.include <bsd.lib.mk>
