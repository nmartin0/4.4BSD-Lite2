#	$Id: Makefile,v 1.13.2.6 1997/05/02 11:26:03 gj Exp $

# AI-ONLY NOTE: FreeBSD 2.2.8's, changed (2026-10-27) for this system:
# GDB 4.16 is contrib/gdb-4.16; the readline is the one built beside
# the bfd and libiberty, GDB 4.16's own (FreeBSD linked its
# lib/libreadline), and so is its regex, gnu-regex.c, where FreeBSD
# linked -lgnuregex and NetBSD 1.3 compiled it in as here; and there
# are no shared libraries, which this system has none of: solib.c is out
# (see nm.h).
PROG = gdb

GDBDIR=	${.CURDIR}/../../../../contrib/gdb-4.16
.PATH:	${GDBDIR}/gdb
.PATH:	${GDBDIR}/opcodes

BINDIR= /usr/bin
XSRCS = annotate.c blockframe.c breakpoint.c buildsym.c c-lang.c \
	c-typeprint.c c-valprint.c ch-lang.c ch-typeprint.c \
	ch-valprint.c coffread.c command.c complaints.c copying.c core-aout.c \
	corelow.c cp-valprint.c \
	dcache.c dbxread.c demangle.c dwarfread.c \
	elfread.c environ.c eval.c exec.c expprint.c \
	findvar.c fork-child.c i386b-nat.c gdbtypes.c gnu-regex.c \
	i386-tdep.c infcmd.c inflow.c infptrace.c \
	infrun.c inftarg.c language.c \
	m2-lang.c m2-typeprint.c m2-valprint.c main.c maint.c \
	mem-break.c minsyms.c objfiles.c parse.c \
	printcmd.c remote.c remote-utils.c source.c \
	stabsread.c stack.c symfile.c symmisc.c \
	symtab.c target.c thread.c top.c \
	typeprint.c utils.c valarith.c valops.c \
	valprint.c values.c version.c \
	serial.c ser-unix.c ser-tcp.c mdebugread.c \
	c-exp.c f-exp.c m2-exp.c i387-tdep.c \
	kvm-fbsd.c bcache.c \
	corefile.c ch-exp.c f-lang.c scm-exp.c scm-lang.c \
	scm-valprint.c f-typeprint.c f-valprint.c nlmread.c \
	callback.c
XSRCS+= i386-dis.c dis-buf.c disassemble.c
SRCS=	init.c ${XSRCS}

CFLAGS+= -I$(.CURDIR) -I${GDBDIR}/readline -I$(.CURDIR)/../bfd
# use phkmalloc
CFLAGS+= -DNO_MMALLOC
# uncomment the next line if you want to debug gdb
#CFLAGS+= -g

CLEANFILES+=	c-exp.c f-exp.c m2-exp.c init.c y.tab.h init.c-tmp
#beforedepend:	c-exp.c f-exp.c m2-exp.c init.c
.ORDER:	c-exp.c f-exp.c m2-exp.c

.if exists(${.OBJDIR}/../bfd)
LDADD+=   -L${.OBJDIR}/../bfd -lbfd
DPADD+=   ${.OBJDIR}/../bfd/libbfd.a
.else
LDADD+=   -L${.CURDIR}/../bfd/ -lbfd
DPADD+=   ${.CURDIR}/../bfd/libbfd.a
.endif

.if exists(${.OBJDIR}/../readline)
LDADD+=   -L${.OBJDIR}/../readline -lreadline
DPADD+=   ${.OBJDIR}/../readline/libreadline.a
.else
LDADD+=   -L${.CURDIR}/../readline/ -lreadline
DPADD+=   ${.CURDIR}/../readline/libreadline.a
.endif

.if exists(${.OBJDIR}/../libiberty)
LDADD+=   -L${.OBJDIR}/../libiberty -liberty
DPADD+=   ${.OBJDIR}/../libiberty/libiberty.a
.else
LDADD+=   -L${.CURDIR}/../libiberty/ -liberty
DPADD+=   ${.CURDIR}/../libiberty/libiberty.a
.endif

DPADD+= ${LIBTERMCAP}
LDADD+= -ltermcap

.if exists(${.OBJDIR}/../libiberty)
LDADD+=   -L${.OBJDIR}/../libiberty -liberty
DPADD+=   ${.OBJDIR}/../libiberty/libiberty.a
.else
LDADD+=   -L${.CURDIR}/../libiberty/ -liberty
DPADD+=   ${.CURDIR}/../libiberty/libiberty.a
.endif

# We do this by grepping through sources.  If that turns out to be too slow,
# maybe we could just require every .o file to have an initialization routine
# of a given name (remote-udi.o -> _initialize_remote_udi, etc.).
#
# Formatting conventions:  The name of the _initialize_* routines must start
# in column zero, and must not be inside #if.
#
# Note that the set of files with init functions might change, or the names
# of the functions might change, so this files needs to depend on all the
# object files that will be linked into gdb.

init.c: ${XSRCS}
	@echo Making ${.TARGET}
	@rm -f init.c-tmp
	@echo '/* Do not modify this file.  */' >init.c-tmp
	@echo '/* It is created automatically by the Makefile.  */'>>init.c-tmp
	@echo 'void initialize_all_files () {' >>init.c-tmp
	@for i in ${.ALLSRC} ; do \
	  filename=`echo $$i | sed \
	    -e '/^Onindy.c/d' \
	    -e '/^nindy.c/d' \
	    -e '/ttyflush.c/d' \
	    -e '/xdr_ld.c/d' \
	    -e '/xdr_ptrace.c/d' \
	    -e '/xdr_rdb.c/d' \
	    -e '/udr.c/d' \
	    -e '/udip2soc.c/d' \
	    -e '/udi2go32.c/d' \
	    -e '/version.c/d' \
	    -e '/^[a-z0-9A-Z_]*_[SU].c/d' \
	    -e '/[a-z0-9A-Z_]*-exp.tab.c/d'` ; \
	  case $$filename in \
	    "") ;; \
	    *) sed <$$filename >>init.c-tmp -n \
	-e '/^_initialize_[a-z_0-9A-Z]* *(/s/^\([a-z_0-9A-Z]*\).*/  {extern void \1 (); \1 ();}/p' ; ;; \
	  esac ; \
	done
	@echo '}' >>init.c-tmp
	@mv init.c-tmp ${.TARGET}

.PRECIOUS: init.c

.include <bsd.prog.mk>
