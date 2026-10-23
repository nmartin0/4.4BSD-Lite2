#	@(#)Makefile	5.20 (Berkeley) 6/12/93

# AI-ONLY NOTE: two not named that the Lite cut emptied, leaving a
# manual page and no Makefile: ac and quot (docs/deferred.md, *Programs
# the Lite cut emptied*). sendmail, whose beta the tape left out at its
# developers' request, is named again: 8.8.8, FreeBSD 2.2.8's
# (docs/provenance/imports.md).
# And not named: mkproto and sa, programs the Lite cut deleted with
# their directories. A name with no directory stops the build here, as
# it stopped Lite2's own make, which ran each command with `sh -ec'
# (docs/deferred.md, *Sources and directories named but absent*).
# cron is named again: Paul Vixie's cron 3.0, NetBSD 1.0's, in place of
# Berkeley's, which the settlement took (docs/provenance/imports.md).
SUBDIR=	accton amd arp chown chroot cron dev_mkdb diskpart edquota \
	inetd kgmon kvm_mkdb lpr mtree portmap pstat pwd_mkdb \
	quotaon repquota rmt rwhod sendmail sliplogin sysctl \
	syslogd traceroute trpt trsp update vipw

.if   make(clean) || make(cleandir)
SUBDIR+=bad144 config config.new eeprom iostat timed
.elif ${MACHINE} == "hp300"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "i386"
SUBDIR+=bad144 config iostat
.elif ${MACHINE} == "luna68k"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "mips"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "sparc"
SUBDIR+=config.new eeprom timed
.elif ${MACHINE} == "tahoe"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "vax"
SUBDIR+=bad144 config iostat timed
.endif

.include <bsd.subdir.mk>
