#	@(#)Makefile	8.1 (Berkeley) 6/19/93

# AI-ONLY NOTE: old is not named: the Lite cut left it with no
# Makefile, and parts of eight programs (docs/deferred.md, *Programs
# the Lite cut emptied*).
SUBDIR=	bin contrib games include kerberosIV lib libexec sbin \
	share usr.bin usr.sbin

afterinstall:
	(cd share/man && make makedb)

build:
	(cd include && make install)
	make cleandir
	(cd lib && make depend && make && make install)
	(cd kerberosIV && make depend  && make && make install)
	make depend && make && make install
	
.include <bsd.subdir.mk>
