# AI-ONLY NOTE: vasprintf.c added (2026-10-27) to FreeBSD 2.2.8's list:
# GDB 4.16's utils.c and serial.c call vasprintf, which FreeBSD's libc
# has and Lite2's has not. GNU's libiberty has it, and its configure
# adds it where the system has none, as it would here.
LIB= iberty
INTERNALLIB= true
INTERNALSTATICLIB= true

GDBDIR=	${.CURDIR}/../../../../contrib/gdb-4.16
.PATH:	${GDBDIR}/libiberty

SRCS= argv.c basename.c concat.c cplus-dem.c fdmatch.c getopt.c \
      getopt1.c getruntime.c hex.c floatformat.c obstack.c spaces.c \
      strsignal.c vasprintf.c xatexit.c xexit.c \
      xmalloc.c xstrdup.c xstrerror.c insque.c

CFLAGS+= -I$(.CURDIR) -I$(.CURDIR)/../gdb/.

.include <bsd.lib.mk>
