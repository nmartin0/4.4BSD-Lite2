#	@(#)Makefile	5.20 (Berkeley) 6/12/93

# AI-ONLY NOTE: quot, which the Lite cut emptied, leaving its manual
# page and no Makefile, is named again: Wolfgang Solfrank's, NetBSD
# 1.0's (docs/provenance/imports.md). sendmail, whose beta the tape left out at its
# developers' request, is named again: 8.8.8, FreeBSD 2.2.8's
# (docs/provenance/imports.md).
# And not named: mkproto, a program the Lite cut deleted with its
# directory. A name with no directory stops the build here, as
# it stopped Lite2's own make, which ran each command with `sh -ec'
# (docs/deferred.md, *Sources and directories named but absent*).
# cron is named again: Paul Vixie's cron 3.0, NetBSD 1.0's, in place of
# Berkeley's, which the settlement took (docs/provenance/imports.md).
# ac and sa are named again: NetBSD 1.0's rewrites, in place of
# Berkeley's, which the settlement took (docs/provenance/imports.md).
SUBDIR=	ac accton amd arp chown chroot cron dev_mkdb diskpart edquota \
	inetd kgmon kvm_mkdb lpr mtree portmap pstat pwd_mkdb \
	quot quotaon repquota rmt rwhod sa sendmail sliplogin sysctl \
	syslogd traceroute trpt trsp update vipw

.if   make(clean) || make(cleandir)
SUBDIR+=bad144 config config.new eeprom iostat timed
.elif ${MACHINE} == "hp300"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "i386"
# AI-ONLY NOTE: timed, which every other port's line here builds, and
# NetBSD 1.0, FreeBSD 2.0 and OpenBSD built for the i386. This line was
# written for bad144 alone (William Jolitz, 15 January 1991), and timed
# went into the other ports' lines when it left the common list (Bostic,
# 6 May 1991); adjtime and settimeofday, all it needs, are the kernel's
# own, machine-independent.
SUBDIR+=bad144 config iostat timed
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
