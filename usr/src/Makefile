#	@(#)Makefile	8.1 (Berkeley) 6/19/93

# AI-ONLY NOTE: old is not named: the Lite cut left it with no
# Makefile, and parts of eight programs (docs/deferred.md, *Programs
# the Lite cut emptied*). kerberosIV is built only with KERBEROS, as
# the Kerberos parts of login, su, rlogind and the rest are
# (docs/deferred.md, *Kerberos*); NetBSD 1.0's top-level Makefile
# guards its line in `build' the same way, with `.if exists'.
SUBDIR=	bin contrib games include lib libexec sbin \
	share usr.bin usr.sbin
.if defined(KERBEROS)
SUBDIR+=kerberosIV
.endif

# AI-ONLY NOTE: `cd ${.CURDIR}/...' and ${MAKE}, as FreeBSD 2.0's
# top-level Makefile writes its `build' and `afterinstall'; and the
# two makes run here, `cd ${.CURDIR}' first, as FreeBSD 3.0's
# Makefile.inc1 runs its own. Berkeley's said `cd include' and `make',
# which work only when make runs in the source directory itself.
# Under an object directory prefix, as make.sh builds, this
# directory has one too -- the parent of every other -- and a command
# runs there, where there is no Makefile.
afterinstall:
	(cd ${.CURDIR}/share/man && ${MAKE} makedb)

build:
	(cd ${.CURDIR}/include && ${MAKE} install)
	cd ${.CURDIR} && ${MAKE} cleandir
	(cd ${.CURDIR}/lib && ${MAKE} depend && ${MAKE} && ${MAKE} install)
.if defined(KERBEROS)
	(cd ${.CURDIR}/kerberosIV && ${MAKE} depend && ${MAKE} && ${MAKE} install)
.endif
	cd ${.CURDIR} && ${MAKE} depend && ${MAKE} && ${MAKE} install
	
.include <bsd.subdir.mk>
