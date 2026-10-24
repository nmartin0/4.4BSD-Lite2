LIBUNIX!=cd $(.CURDIR)/../libunix; \
	printf "xxx:\n\techo \$${.OBJDIR}/libunix.a\n" | ${MAKE} -r -s -f - xxx

LIBUUCONF!=cd $(.CURDIR)/../libuuconf; \
	printf "xxx:\n\techo \$${.OBJDIR}/libuuconf.a\n" | ${MAKE} -r -s -f - xxx

LIBUUCP!=cd $(.CURDIR)/../libuucp; \
	printf "xxx:\n\techo \$${.OBJDIR}/libuucp.a\n" | ${MAKE} -r -s -f - xxx

VERSION=	1.06.1
owner=		uucp
bindir=		/usr/bin
sbindir=	/usr/libexec/uucp

# The directory to look in for new style configuration files (when
# using HAVE_TAYLOR_CONFIG).
newconfigdir=	/etc/uucp

# The directory to look in for BNU (when using HAVE_BNU_CONFIG) or
# V2 (when using HAVE_V2_CONFIG) style configuration files.
oldconfigdir=	/etc/uucp

# AI-ONLY NOTE: removed (2026-10-24) -- NetBSD's -lgnumalloc for every
# program, as OpenBSD removed it (Thorsten Lockert, 20 August 1996) and
# NetBSD later (1999): this system has no libgnumalloc, and libc's
# malloc serves, as it does contrib/diff-2.6 and grep-2.0.
# this is sick, but much easier than putting in the individual programs'
# makefiles.
#.if defined(PROG)
## use gnu malloc
#LDADD+=	-lgnumalloc
#DPADD+= /usr/lib/libgnumalloc.a
#.endif
