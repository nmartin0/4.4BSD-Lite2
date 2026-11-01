/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/ktest.sh as /sbin/init. It exercises execve as it now is,
 * NetBSD 1.0's exec framework with 1.1's ELF loader
 * (docs/provenance/exec-stack.md), against plan-4.5.md's list for this
 * item -- init, ELF static binaries, `#!' scripts, set-user-ID,
 * argument limits, ps strings, saved IDs -- and the behaviours the
 * change alters: a `#!' line longer than 52 bytes, ETXTBSY for a
 * program open for writing, the zeroth argument of a script's
 * interpreter, and the program's vnode for procfs's `file'.
 *
 * One program, many roles. As init (no arguments) it remounts the root
 * writable, makes its fixtures from its own image, and runs each case
 * in a child: the child makes any change the case needs, execs, and on
 * failure sends errno back through a pipe. A program that ran prints
 * its own `T' line from the role named by its argv[1].
 *
 * Added after independent review (2026-10-31): a stack limit that is
 * not a whole number of pages (the review found NetBSD 1.0's stack
 * setup killed the exec; fixed by NetBSD 1.3's round_page in
 * kern_resource.c), a null argv, an empty argv and environment, the
 * 64- and 65-byte `#!' lines, setpgid on a child that has exec'd
 * (P_EXEC), a traced exec of a set-id program and of one the tracee
 * cannot read, the e_phnum and PT_INTERP refusals, and a signal caught
 * after exec.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ptrace.h>

extern int mount(const char *, const char *, int, void *);
#define MNT_UPDATE 0x00010000

/* this port's, as copyio.c has them; <sys/exec.h>'s structure */
#define USRSTACK 0xEFBFE000UL
struct ps_strings {
	char	**ps_argvstr;
	int	ps_nargvstr;
	char	**ps_envstr;
	int	ps_nenvstr;
};
#define PS_STRINGS ((struct ps_strings *)(USRSTACK - sizeof(struct ps_strings)))

extern char **environ;
static void say(const char *s) { write(2, s, strlen(s)); }

/* ---------------------------------------------------------------- roles */

static volatile int ncaught;
static void caught(int s) { ncaught++; }

static int deep(int n) { volatile char b[4096]; b[0] = n; b[4095] = n; return n ? deep(n - 1) + b[0] : 0; }

static void
role(int argc, char **argv)
{
	char b[400];
	int i;
	char *r = argv[1];

	if (strcmp(r, "args") == 0) {
		struct ps_strings *ps = PS_STRINGS;
		strcpy(b, "T args: argc=");
		sprintf(b + strlen(b), "%d argv=[", argc);
		for (i = 0; i < argc; i++)
			sprintf(b + strlen(b), "%s%s", i ? "|" : "", argv[i]);
		strcat(b, "] env=[");
		for (i = 0; environ[i]; i++)
			sprintf(b + strlen(b), "%s%s", i ? "|" : "", environ[i]);
		strcat(b, "]\n");
		say(b);
		sprintf(b, "T ps_strings: nargv=%d argv=%s nenv=%d envp=%s\n",
		    ps->ps_nargvstr, ps->ps_argvstr == argv ? "same" : "DIFFERENT",
		    ps->ps_nenvstr, ps->ps_envstr == environ ? "same" : "DIFFERENT");
		say(b);
		sprintf(b, "T stack: 300 KB used, deep()=%d\n", deep(75));
		say(b);
		_exit(0);
	}
	if (strcmp(r, "stackover") == 0) {	/* past the 512 KB limit */
		deep(160);
		say("T stackover: survived (wrong)\n");
		_exit(0);
	}
	if (strcmp(r, "role-script") == 0 || strcmp(r, "/s2") == 0) {
		sprintf(b, "T script: argc=%d argv=[", argc);
		for (i = 0; i < argc; i++)
			sprintf(b + strlen(b), "%s%s", i ? "|" : "", argv[i]);
		strcat(b, "]\n");
		say(b);
		_exit(0);
	}
	if (strncmp(r, "role-long-", 10) == 0) {
		sprintf(b, "T long line: argv[1] is %d bytes\n", (int)strlen(r));
		say(b);
		_exit(0);
	}
	if (strcmp(r, "suid") == 0 || strcmp(r, "suidscript") == 0) {
		int u, g;
		sprintf(b, "T %s: ruid=%d euid=%d rgid=%d egid=%d\n", r,
		    (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());
		say(b);
		if (strcmp(r, "suid") == 0) {
			/* the saved ids: drop to the real ones and take the set-id ones back */
			seteuid(getuid());
			u = seteuid(5) == 0 ? (int)geteuid() : -errno;
			setegid(getgid());
			g = setegid(6) == 0 ? (int)getegid() : -errno;
			sprintf(b, "T saved ids: seteuid back -> %d, setegid back -> %d\n", u, g);
			say(b);
		}
		_exit(0);
	}
	if (strcmp(r, "cloexec") == 0) {
		int e10, f11;
		errno = 0; e10 = fcntl(10, F_GETFD) < 0 ? errno : 0;
		f11 = fcntl(11, F_GETFD);
		sprintf(b, "T cloexec: fd 10 errno=%d, fd 11 flags=%d\n", e10, f11);
		say(b);
		_exit(0);
	}
	if (strcmp(r, "sig") == 0) {
		struct sigaction o1, o2;
		sigset_t m;
		sigaction(SIGUSR1, 0, &o1);
		sigaction(SIGUSR2, 0, &o2);
		sigprocmask(SIG_BLOCK, 0, &m);
		sprintf(b, "T sig: usr1=%s usr2=%s hup=%s\n",
		    o1.sa_handler == SIG_DFL ? "default" : "NOT default",
		    o2.sa_handler == SIG_IGN ? "ignored" : "NOT ignored",
		    sigismember(&m, SIGHUP) ? "blocked" : "NOT blocked");
		say(b);
		_exit(0);
	}
	if (strcmp(r, "textvp") == 0) {
		struct stat a, g;
		if (stat("/proc/curproc/file", &a) < 0)
			sprintf(b, "T textvp: /proc/curproc/file errno=%d\n", errno);
		else {
			stat("/good", &g);
			sprintf(b, "T textvp: %s\n", a.st_ino == g.st_ino &&
			    a.st_dev == g.st_dev ? "is /good" : "is NOT /good");
		}
		say(b);
		_exit(0);
	}
	if (strcmp(r, "big") == 0) {
		sprintf(b, "T big: argc=%d, argv[2] %d bytes\n", argc, (int)strlen(argv[2]));
		say(b);
		_exit(0);
	}
	if (strcmp(r, "chain") == 0) {
		int n = atoi(argv[2]);
		char nb[8], *av[4];
		if (n == 0) {
			say("T chain: reached the last of 6 programs\n");
			_exit(0);
		}
		sprintf(nb, "%d", n - 1);
		av[0] = "/good"; av[1] = "chain"; av[2] = nb; av[3] = 0;
		execve("/good", av, environ);
		sprintf(b, "T chain: exec failed at %d, errno=%d\n", n, errno);
		say(b);
		_exit(1);
	}
	if (strcmp(r, "waitpipe") == 0) {	/* blocks until fd 10 has data */
		char c;
		read(10, &c, 1);
		_exit(0);
	}
	if (strcmp(r, "sigdeliver") == 0) {
		struct sigaction sa;
		memset(&sa, 0, sizeof sa);
		sa.sa_handler = caught;
		sigaction(SIGUSR1, &sa, 0);
		kill(getpid(), SIGUSR1);
		sprintf(b, "T sigdeliver: handler ran %d time(s)\n", ncaught);
		say(b);
		_exit(0);
	}
	if (strcmp(r, "vforked") == 0) {
		say("T vforked child ran\n");
		_exit(0);
	}
	sprintf(b, "T unknown role %s\n", r);
	say(b);
	_exit(2);
}

/* ---------------------------------------------------------------- cases */

static void hook_none(void) { }
static void hook_uid7(void) { setgid(7); setuid(7); }
static void
hook_rlim1025k(void)
{
	struct rlimit r;
	getrlimit(RLIMIT_STACK, &r);
	r.rlim_cur = 1025 * 1024;	/* not a whole number of pages */
	setrlimit(RLIMIT_STACK, &r);
}
static void hook_trace_uid7(void) { setgid(7); setuid(7); ptrace(PT_TRACE_ME, 0, 0, 0); }
static void
hook_sig(void)
{
	struct sigaction sa;
	sigset_t m;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = (void (*)(int))hook_none;	/* caught */
	sigaction(SIGUSR1, &sa, 0);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGUSR2, &sa, 0);
	sigemptyset(&m);
	sigaddset(&m, SIGHUP);
	sigprocmask(SIG_BLOCK, &m, 0);
}
static void
hook_fds(void)
{
	int f = open("/good", O_RDONLY);
	dup2(f, 10); fcntl(10, F_SETFD, 1);
	dup2(f, 11); fcntl(11, F_SETFD, 0);
	close(f);
}

static void
run(const char *label, const char *path, char **av, char **ev, void (*hook)(void))
{
	char b[160];
	int p[2], st, e;
	pid_t c;

	pipe(p);
	c = fork();
	if (c == 0) {
		close(p[0]);
		hook();
		execve(path, av, ev);
		e = errno;
		write(p[1], &e, sizeof e);
		_exit(99);
	}
	close(p[1]);
	if (hook == hook_trace_uid7) {	/* stops at exec with SIGTRAP: let it go */
		waitpid(c, &st, 0);
		if (WIFSTOPPED(st)) {
			sprintf(b, "T %s: stopped with signal %d\n", label, WSTOPSIG(st));
			say(b);
			ptrace(PT_CONTINUE, c, (caddr_t)1, 0);
		}
	}
	e = 0;
	if (read(p[0], &e, sizeof e) != sizeof e)
		e = 0;
	close(p[0]);
	waitpid(c, &st, 0);
	if (WIFSIGNALED(st))
		sprintf(b, "T %s: exec errno=%d, killed by signal %d\n", label, e, WTERMSIG(st));
	else
		sprintf(b, "T %s: exec errno=%d, exit %d\n", label, e, WEXITSTATUS(st));
	say(b);
}

static void
put(const char *name, const char *data, int len, int mode)
{
	int g = open(name, O_WRONLY|O_CREAT|O_TRUNC, 0700);
	char b[100];
	if (g < 0 || write(g, data, len) != len) {
		sprintf(b, "T cannot make %s: errno=%d\n", name, errno);
		say(b);
	}
	close(g);
	chmod(name, mode);
}

static char img[300000];
static char big[21000];

int
main(int argc, char **argv)
{
	char *ev[3], *av[6], b[200];
	int n, f;

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	if (argc == 0) {		/* exec'd with an empty argv */
		say("T argc 0: ran\n");
		_exit(0);
	}
	if (argc > 1)
		role(argc, argv);
	say("T begin\n");
	{	/* the root is mounted read-only: make it writable, as elfshort.c does */
		struct { char *fspec; char rest[64]; } a;
		memset(&a, 0, sizeof a);
		a.fspec = "/d/wd0a";
		if (mount("ufs", "/", MNT_UPDATE, &a) < 0) {
			sprintf(b, "T remount: errno=%d\n", errno);
			say(b);
		}
	}
	f = open("/sbin/init", O_RDONLY);
	n = read(f, img, sizeof img);
	close(f);
	if (n <= 0 || n == sizeof img)
		say("T /sbin/init does not fit the test's buffer\n");
	put("/good", img, n, 0755);
	put("/su", img, n, 0755);
	chown("/su", 5, 6);
	chmod("/su", 06755);			/* set-user-ID 5, set-group-ID 6 */
	put("/busy", img, n, 0755);
	put("/s1", "#!/sbin/init role-script\n", 25, 0755);
	put("/s2", "#! /sbin/init\n", 14, 0755);
	/* a 60-byte line: past the 52 bytes the old execve read, inside MAXINTERP 64 */
	put("/s3", "#!/sbin/init role-long-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", 60, 0755);
	/* a 70-byte line: past MAXINTERP */
	put("/s4", "#!/sbin/init role-long-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", 70, 0755);
	put("/s5", "#!/s1\n", 6, 0755);		/* a script whose interpreter is a script */
	put("/s6", "#!/sbin/init suidscript\n", 24, 0755);
	chown("/s6", 5, 6);
	chmod("/s6", 06755);
	put("/s7", "#!/nonexistent\n", 15, 0755);
	put("/junk", "hello, world\n", 13, 0755);
	/* exactly MAXINTERP: 64 bytes with the newline; and 65 */
	put("/s9", "#!/sbin/init role-long-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", 64, 0755);
	put("/s10", "#!/sbin/init role-long-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", 65, 0755);
	put("/unread", img, n, 0711);		/* execute-only: owner 5 below */
	chown("/unread", 5, 6);
	chmod("/unread", 04711);
	{	/* e_phnum (offset 44) 200; and the first program header made PT_INTERP */
		static char m[300000];
		memcpy(m, img, n);
		m[44] = 200; m[45] = 0;
		put("/phnum", m, n, 0755);
		memcpy(m, img, n);
		m[52] = 3; m[53] = m[54] = m[55] = 0;	/* e_phoff is 52; p_type 3 */
		put("/interp", m, n, 0755);
	}
	put("/noexec", img, n, 0644);
	mkdir("/proc", 0755);
	if (mount("procfs", "/proc", 0, (void *)0) < 0) {
		sprintf(b, "T mount procfs: errno=%d\n", errno);
		say(b);
	}

	ev[0] = "A=1"; ev[1] = "B=two"; ev[2] = 0;

	/* ELF static binaries, arguments, environment, ps_strings, the stack */
	av[0] = "/good"; av[1] = "args"; av[2] = "one"; av[3] = "two words"; av[4] = 0;
	run("ELF program", "/good", av, ev, hook_none);
	av[1] = "stackover"; av[2] = 0;
	run("stack past its limit", "/good", av, ev, hook_none);

	/* `#!' scripts */
	av[0] = "s1-argv0"; av[1] = "x"; av[2] = "y"; av[3] = 0;
	run("script with an argument", "/s1", av, ev, hook_none);
	run("script without one", "/s2", av, ev, hook_none);
	run("script, 60-byte line", "/s3", av, ev, hook_none);
	run("script, 70-byte line", "/s4", av, ev, hook_none);
	run("script run by a script", "/s5", av, ev, hook_none);
	run("script, interpreter missing", "/s7", av, ev, hook_none);

	/* refusals */
	av[0] = "x"; av[1] = "args"; av[2] = 0;
	run("not a program", "/junk", av, ev, hook_none);
	run("no execute permission", "/noexec", av, ev, hook_none);
	run("a directory", "/", av, ev, hook_none);
	run("no such file", "/nonexistent", av, ev, hook_none);

	/* set-user-ID, set-group-ID, saved IDs */
	av[0] = "/su"; av[1] = "suid"; av[2] = 0;
	run("set-id program, as uid 7", "/su", av, ev, hook_uid7);
	av[0] = "/s6";
	run("set-id script, as uid 7", "/s6", av, ev, hook_uid7);

	/* argument limits: ARG_MAX is 20480 */
	memset(big, 'a', 16000); big[16000] = 0;
	av[0] = "/good"; av[1] = "big"; av[2] = big; av[3] = 0;
	run("arguments of 16000 bytes", "/good", av, ev, hook_none);
	memset(big, 'a', sizeof big - 1); big[sizeof big - 1] = 0;
	run("arguments over ARG_MAX", "/good", av, ev, hook_none);

	/* what exec keeps and resets */
	av[0] = "/good"; av[1] = "cloexec"; av[2] = 0;
	run("close-on-exec", "/good", av, ev, hook_fds);
	av[1] = "sig";
	run("signals", "/good", av, ev, hook_sig);

	/* a stack limit that is not a whole number of pages */
	av[0] = "/good"; av[1] = "args"; av[2] = "rlim"; av[3] = 0;
	run("stack limit 1025K", "/good", av, ev, hook_rlim1025k);

	/* null argv; an empty argv and environment */
	run("null argv", "/good", 0, ev, hook_none);
	{
		char *none[1];
		none[0] = 0;
		run("empty argv and environment", "/good", none, none, hook_none);
	}

	/* the MAXINTERP boundary */
	av[0] = "x"; av[1] = 0;
	run("script, 64-byte line", "/s9", av, ev, hook_none);
	run("script, 65-byte line", "/s10", av, ev, hook_none);

	/* traced: set-id is not honoured; a program the tracee cannot read */
	av[0] = "/su"; av[1] = "suid"; av[2] = 0;
	run("traced set-id program, as uid 7", "/su", av, ev, hook_trace_uid7);
	av[0] = "/unread";
	run("traced execute-only program, as uid 7", "/unread", av, ev, hook_trace_uid7);

	/* the loader's refusals */
	av[0] = "x"; av[1] = "args"; av[2] = 0;
	run("e_phnum 200", "/phnum", av, ev, hook_none);
	run("a PT_INTERP header", "/interp", av, ev, hook_none);

	/* a signal caught in the new image */
	av[0] = "/good"; av[1] = "sigdeliver"; av[2] = 0;
	run("signal after exec", "/good", av, ev, hook_none);

	/* setpgid on a child that has exec'd (P_EXEC) */
	{
		int q[2], st;
		pid_t c;
		pipe(q);
		c = fork();
		if (c == 0) {
			dup2(q[0], 10);
			av[0] = "/good"; av[1] = "waitpipe"; av[2] = 0;
			execve("/good", av, ev);
			_exit(99);
		}
		sleep(2);		/* let it exec */
		errno = 0;
		sprintf(b, "T setpgid on an exec'd child: errno=%d\n",
		    setpgid(c, c) < 0 ? errno : 0);
		say(b);
		write(q[1], "x", 1);
		waitpid(c, &st, 0);
	}

	/* the program's vnode, as procfs's `file' reads it */
	av[1] = "textvp";
	run("procfs file", "/good", av, ev, hook_none);

	/* a program open for writing; then writing a program that has run */
	f = open("/busy", O_WRONLY);
	av[0] = "/busy"; av[1] = "args"; av[2] = 0;
	run("program open for writing", "/busy", av, ev, hook_none);
	close(f);
	errno = 0;
	f = open("/good", O_WRONLY);
	sprintf(b, "T open for writing a program that has run: errno=%d\n", f < 0 ? errno : 0);
	say(b);
	if (f >= 0)
		close(f);

	/* six programs in one process, each exec'ing the next */
	av[0] = "/good"; av[1] = "chain"; av[2] = "5"; av[3] = 0;
	run("exec chain", "/good", av, ev, hook_none);

	/* vfork: the parent resumes when the child execs */
	{
		pid_t c;
		int st;
		av[0] = "/good"; av[1] = "vforked"; av[2] = 0;
		c = vfork();
		if (c == 0) {
			execve("/good", av, ev);
			_exit(99);
		}
		waitpid(c, &st, 0);
		sprintf(b, "T vfork: parent resumed, child exit %d\n", WEXITSTATUS(st));
		say(b);
	}

	say("T end\n");
	reboot(RB_HALT);
	for (;;)
		;
}
