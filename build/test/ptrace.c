/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/ktest.sh as /sbin/init. It exercises ptrace(2) as it now
 * is, NetBSD 1.0's sys_process.c with PT_WRITE_U confined by
 * ptrace_write_u (docs/provenance/ptrace-firstfit.md): PT_TRACE_ME and PT_ATTACH,
 * PT_READ_D and PT_WRITE_D, PT_CONTINUE, PT_DETACH, the refusals for a
 * process that is not traced, and the user area: PT_READ_U works, as
 * gdb needs; PT_WRITE_U may change an ordinary register, as gdb needs,
 * but not the code segment or the protected flags, nor anything
 * outside the register frame and the FPU save area.
 * (Revised by the integrator from the worker's first version, which
 * expected NetBSD 1.1's removal of both requests.)
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/param.h>
#include <sys/user.h>
#include <machine/frame.h>
#include <machine/vmparam.h>
#include <stddef.h>

static void say(const char *s) { write(2, s, strlen(s)); }

static volatile int cell = 0x1234;	/* the tracee's, at the same address in both */
static volatile int flag;

static int
req(int r, pid_t pid, caddr_t addr, int data, int *err)
{
	int v;

	errno = 0;
	v = ptrace(r, pid, addr, data);
	*err = errno;
	return v;
}

int
main(int argc, char **argv)
{
	char b[160];
	int st, e, v, stops;
	pid_t c;

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	say("T begin\n");

	/* PT_TRACE_ME: the child stops itself, the parent reads and writes it */
	c = fork();
	if (c == 0) {
		ptrace(PT_TRACE_ME, 0, 0, 0);
		kill(getpid(), SIGSTOP);
		_exit(cell & 0xff);
	}
	waitpid(c, &st, 0);
	sprintf(b, "T trace-me: stopped=%d signal=%d\n", WIFSTOPPED(st) != 0,
	    WIFSTOPPED(st) ? WSTOPSIG(st) : -1);
	say(b);
	v = req(PT_READ_D, c, (caddr_t)&cell, 0, &e);
	sprintf(b, "T read_d: value=%#x errno=%d\n", v, e);
	say(b);
	v = req(PT_WRITE_D, c, (caddr_t)&cell, 0x55, &e);
	sprintf(b, "T write_d: result=%d errno=%d\n", v, e);
	say(b);
	v = req(PT_READ_D, c, (caddr_t)&cell, 0, &e);
	sprintf(b, "T read_d after write: value=%#x errno=%d\n", v, e);
	say(b);
	v = req(PT_READ_D, c, (caddr_t)0, 0, &e);
	sprintf(b, "T read_d at address 0: refused=%d\n", e != 0);
	say(b);

	/* the user area: readable; writable only in the ordinary registers */
	{
		static const unsigned long bad[] = { 0, 0x100, 0x1000,
		    0x2000, 0x10000, 0xfffffffc, 3 };
		unsigned long regs, fr;
		int i, n = sizeof bad / sizeof bad[0];

		regs = (unsigned long)req(PT_READ_U, c, (caddr_t)offsetof(struct user,
		    u_kproc.kp_proc.p_md.md_regs), 0, &e);
		fr = regs - USRSTACK;
		sprintf(b, "T read_u md_regs: errno=%d in-u-area=%d\n", e,
		    fr < UPAGES * NBPG);
		say(b);
		v = req(PT_WRITE_U, c, (caddr_t)(fr + offsetof(struct trapframe,
		    tf_esi)), 0x5a5a5a5a, &e);
		sprintf(b, "T write_u esi: result=%d errno=%d\n", v, e);
		say(b);
		v = req(PT_READ_U, c, (caddr_t)(fr + offsetof(struct trapframe,
		    tf_esi)), 0, &e);
		sprintf(b, "T read_u esi: same=%d errno=%d\n", v == 0x5a5a5a5a, e);
		say(b);
		v = req(PT_WRITE_U, c, (caddr_t)(fr + offsetof(struct trapframe,
		    tf_cs)), 0x8, &e);
		sprintf(b, "T write_u cs = kernel's: refused=%d errno=%d\n",
		    v != 0 || e != 0, e);
		say(b);
		v = req(PT_READ_U, c, (caddr_t)(fr + offsetof(struct trapframe,
		    tf_eflags)), 0, &e);
		v = req(PT_WRITE_U, c, (caddr_t)(fr + offsetof(struct trapframe,
		    tf_eflags)), v | 0x3000, &e);
		sprintf(b, "T write_u eflags with IOPL 3: refused=%d errno=%d\n",
		    v != 0 || e != 0, e);
		say(b);
		for (i = 0; i < n; i++) {
			v = req(PT_WRITE_U, c, (caddr_t)bad[i], 0x41414141, &e);
			sprintf(b, "T write_u offset %#lx: refused=%d errno=%d\n",
			    bad[i], v != 0 || e != 0, e);
			say(b);
		}
	}

	/* the tracee must be unharmed by the refused writes */
	v = req(PT_CONTINUE, c, (caddr_t)1, 0, &e);
	sprintf(b, "T continue: result=%d errno=%d\n", v, e);
	say(b);
	waitpid(c, &st, 0);
	sprintf(b, "T trace-me child: exited=%d status=%#x\n", WIFEXITED(st) != 0,
	    WIFEXITED(st) ? WEXITSTATUS(st) : -1);
	say(b);

	/* PT_ATTACH to a running process, write its flag, let it finish */
	c = fork();
	if (c == 0) {
		while (!flag)
			;
		_exit(7);
	}
	usleep(200000);
	v = req(PT_READ_D, c, (caddr_t)&flag, 0, &e);
	sprintf(b, "T read_d of a process not traced: errno=%d\n", e);
	say(b);
	v = req(PT_ATTACH, getpid(), 0, 0, &e);
	sprintf(b, "T attach to self: result=%d errno=%d\n", v, e);
	say(b);
	v = req(PT_ATTACH, c, 0, 0, &e);
	sprintf(b, "T attach: result=%d errno=%d\n", v, e);
	say(b);
	waitpid(c, &st, 0);
	sprintf(b, "T attach: stopped=%d signal=%d\n", WIFSTOPPED(st) != 0,
	    WIFSTOPPED(st) ? WSTOPSIG(st) : -1);
	say(b);
	v = req(PT_ATTACH, c, 0, 0, &e);
	sprintf(b, "T attach again: result=%d errno=%d\n", v, e);
	say(b);
	v = req(PT_WRITE_D, c, (caddr_t)&flag, 1, &e);
	sprintf(b, "T write_d flag: result=%d errno=%d\n", v, e);
	say(b);
	v = req(PT_WRITE_U, c, (caddr_t)0x1000, 0, &e);
	sprintf(b, "T write_u on the attached process: refused=%d errno=%d\n",
	    v != 0 || e != 0, e);
	say(b);
	v = req(PT_READ_U, c, (caddr_t)0, 0, &e);
	sprintf(b, "T read_u on the attached process: errno=%d\n", e);
	say(b);
	v = req(PT_CONTINUE, c, (caddr_t)1, 0, &e);
	sprintf(b, "T continue: result=%d errno=%d\n", v, e);
	say(b);
	/* the stop PT_ATTACH asked for may be reported a second time */
	stops = 0;
	for (;;) {
		waitpid(c, &st, 0);
		if (!WIFSTOPPED(st) || ++stops > 5)
			break;
		req(PT_CONTINUE, c, (caddr_t)1, 0, &e);
	}
	sprintf(b, "T attached child: exited=%d status=%d\n", WIFEXITED(st) != 0,
	    WIFEXITED(st) ? WEXITSTATUS(st) : -1);
	say(b);

	/*
	 * PT_DETACH after PT_TRACE_ME: the child keeps its parent. This
	 * checks the detach path; it is not the test of a fix (NetBSD 1.0
	 * gives the same answer, since pfind(0) finds nothing).
	 */
	c = fork();
	if (c == 0) {
		pid_t pp = getppid();

		ptrace(PT_TRACE_ME, 0, 0, 0);
		kill(getpid(), SIGSTOP);
		_exit(getppid() == pp ? 3 : 4);
	}
	waitpid(c, &st, 0);
	v = req(PT_DETACH, c, (caddr_t)1, 0, &e);
	sprintf(b, "T detach: result=%d errno=%d\n", v, e);
	say(b);
	v = waitpid(c, &st, 0);
	sprintf(b, "T detached child: ours=%d same-parent=%d\n", v == c,
	    v == c && WIFEXITED(st) && WEXITSTATUS(st) == 3);
	say(b);

	/* a request that does not exist */
	v = req(9999, getpid(), 0, 0, &e);
	sprintf(b, "T unknown request: errno=%d\n", e);
	say(b);

	say("T end\n");
	reboot(RB_HALT);
	for (;;)
		;
}
