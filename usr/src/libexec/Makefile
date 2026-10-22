#	@(#)Makefile	8.1 (Berkeley) 6/4/93

# AI-ONLY NOTE: kpasswdd is Kerberos itself, and built only with
# KERBEROS, as the Kerberos parts of login, su, rlogind and the rest
# are (docs/deferred.md, *Kerberos*).
# AI-ONLY NOTE: atrun, which Lite2's /etc/crontab runs from /usr/libexec:
# NetBSD 1.0's, Thomas Koenig's and David Parsons's, built here as NetBSD
# builds it. Berkeley's lived in usr.bin/at/atrun and went with the
# settlement (docs/provenance/imports.md).
SUBDIR=	atrun bugfiler comsat fingerd ftpd getNAME getty lfs_cleanerd \
	mail.local makekey rexecd rlogind rshd talkd telnetd tftpd uucpd
.if defined(KERBEROS)
SUBDIR+=kpasswdd
.endif

.if	${MACHINE} == "hp300"
SUBDIR+=rbootd
.elif	${MACHINE} == "luna68k"
SUBDIR+=rbootd
.endif

.include <bsd.subdir.mk>
