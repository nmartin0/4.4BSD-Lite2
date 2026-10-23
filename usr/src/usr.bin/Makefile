#	@(#)Makefile	8.3 (Berkeley) 1/7/94

# AI-ONLY NOTE: awk, which 4.4BSD-Lite's lists had dropped with the
# program itself; it is back as Kernighan's own -- see awk/Makefile.
# And five not named that the Lite cut emptied, leaving a manual page
# or two and no Makefile: diff (diff and diff3), ex, file, sort and
# uucp. contrib/ has programs that could stand for some; until one is
# chosen, nothing is built or installed by these names
# (docs/deferred.md, *Programs the Lite cut emptied*). For diff one is:
# GNU diff 2.6, the system's diff, diff3 and sdiff, from
# contrib/diff-2.6, as in NetBSD 1.0 and FreeBSD 2.0; and for sort:
# Berkeley's own new sort, from contrib/sort. And grep, which is here
# and not named: James Woods' egrep, which hands -v, -b, -w, -x and -y
# to the old AT&T greps the Lite cut removed; the system's grep, egrep
# and fgrep are GNU grep 2.0, NetBSD 1.1's, from contrib/grep-2.0, as
# in NetBSD and FreeBSD 2.0. And ar, nm,
# ranlib, size, strip, strings, gprof and gcore, which read or write
# a.out only, are not built for the i386, whose programs are ELF:
# binutils 2.11.2's stand in (contrib/binutils-2.11.2), as OpenBSD
# builds its own ar, ranlib, strip and strings only `.if
# ${ELF_TOOLCHAIN:L} == "no"' and NetBSD from 1.4 used binutils'.
# gcore NetBSD and OpenBSD dropped. ld likewise, below.
# And not named: bc, dc, deroff, diction, graph, learn,
# plot, ptx, spell, spline, struct, units and xsend, programs the Lite
# cut deleted with their directories. A name with no directory stops
# the build here, as it stopped Lite2's own make, which ran each
# command with `sh -ec' (docs/deferred.md, *Sources and directories
# named but absent*). crontab, which Berkeley's cron had not, is
# Vixie cron's, built from usr.sbin/cron, as NetBSD 1.0 has it; and at
# is named again, Thomas Koenig's, NetBSD 1.0's (docs/provenance/imports.md).
# compress is named again: Berkeley's, as 4.4BSD-Lite had it, which
# Lite2 left out (docs/provenance/imports.md).
SUBDIR=	apply apropos at awk banner basename bdes biff cal calendar \
	cap_mkdb checknr chflags chpass cksum col colcrt colrm column \
	comm compress cpp crontab ctags cut dirname du \
	env error expand false find finger fmt fold fpr from \
	fsplit fstat ftp head hexdump id indent \
	join jot kdump ktrace lam last lastcomm leave locate \
	lock logger login logname lorder m4 mail make man mesg mkdep \
	mkfifo mklocale mkstr more msgs mt netstat nfsstat nice nohup \
	pagesize passwd paste patch pr printenv printf quota \
	rdist renice rev rlogin rs rsh ruptime rwho sccs \
	script sed shar showmount soelim split \
	su talk tcopy tee telnet tftp time tip \
	tn3270 touch tput tr true tset tsort tty ul uname unexpand \
	unifdef uniq unvis users uudecode uuencode vacation \
	vgrind vis w wall wc what whatis whereis who whois window write \
	xargs xinstall xstr yacc yes

.if	${MACHINE} != "i386"
SUBDIR+=ar gcore gprof nm ranlib size strings strip
.endif

# Cmp, look and tail all use mmap, so new-VM only.
# F77 and pascal are VAX/Tahoe only.
.if	${MACHINE} == "hp300"
SUBDIR+=cmp ld look systat tail vmstat
.elif	${MACHINE} == "i386"
SUBDIR+=cmp look systat tail vmstat
.elif	${MACHINE} == "luna68k"
SUBDIR+=cmp ld look systat tail vmstat
.elif	${MACHINE} == "mips"
SUBDIR+=cmp look systat tail vmstat
.elif	${MACHINE} == "sparc"
SUBDIR+=cmp ld look tail vmstat.sparc
.elif	${MACHINE} == "tahoe"
SUBDIR+=f77 pascal systat vmstat
.elif	${MACHINE} == "vax"
SUBDIR+=f77 pascal systat vmstat
.endif

.include <bsd.subdir.mk>
