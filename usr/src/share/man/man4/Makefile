#	@(#)Makefile	8.2 (Berkeley) 5/19/94

# AI-ONLY NOTE: changed (2026-10-26) -- ddb.0, CMU's page for the
# kernel debugger, as NetBSD 1.0's share/man/man4 has it.
MAN4=	bpf.0 clnp.0 cltp.0 ddb.0 drum.0 esis.0 fd.0 icmp.0 idp.0 inet.0 \
	ip.0 \
	iso.0 lo.0 netintro.0 ns.0 nsip.0 null.0 pty.0 route.0 \
	spp.0 tcp.0 termios.0 tp.0 tty.0 udp.0 unix.0
MLINKS+=fd.4 stderr.4 fd.4 stdin.4 fd.4 stdout.4
MLINKS+=netintro.4 networking.4
SUBDIR=	man4.hp300 man4.i386 man4.sparc man4.tahoe man4.vax

.include <bsd.prog.mk>
