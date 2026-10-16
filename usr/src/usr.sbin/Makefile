#	@(#)Makefile	5.20 (Berkeley) 6/12/93

# AI-ONLY NOTE: three not named that the Lite cut emptied, leaving a
# manual page or a README and no Makefile: ac, quot and sendmail
# (docs/deferred.md, *Programs the Lite cut emptied*).
# And not named: cron, mkproto and sa, programs the Lite cut deleted
# with their directories. A name with no directory stops the build
# here, as it stopped Lite2's own make, which ran each command with
# `sh -ec' (docs/deferred.md, *Sources and directories named but
# absent*).
SUBDIR=	accton amd arp chown chroot dev_mkdb diskpart edquota \
	inetd kgmon kvm_mkdb lpr mtree portmap pstat pwd_mkdb \
	quotaon repquota rmt rwhod sliplogin sysctl \
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
