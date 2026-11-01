/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/ktest.sh as /sbin/init (model: execve.c). It tests the
 * maximum stack size, MAXSSIZ in <machine/vmparam.h>
 * (docs/provenance/stack-limit.md). Raising MAXDSIZ to 128 MB for the
 * C++ compiler also raised the stack's maximum, because Lite2 defines
 * MAXSSIZ as MAXDSIZ; this shows what the maximum is and that a stack
 * cannot grow past it.
 *
 * As init it reads getrlimit(RLIMIT_STACK)'s maximum, then for each case
 * forks a child that sets the soft limit to that maximum and execs this
 * program as a "grow" role, which touches a byte a given number
 * of megabytes below its stack pointer. A stack used within the maximum must
 * work and one that goes past it must die with SIGSEGV. The last case
 * is the root's, which may set a limit above the maximum (the kernel
 * caps it at the data maximum only) and then exec.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/reboot.h>

static void say(const char *s) { write(2, s, strlen(s)); }

/*
 * touch a byte mb megabytes below this frame, which is stack the process
 * has not used: if the kernel counts it inside the limit it is made
 * (QEMU has no swap and little memory, so one page is touched, not all
 * the pages down to it)
 */
static void
grow(int mb)
{
	char b[80];
	volatile char here;
	volatile char *p = &here - ((long)mb << 20);

	*p = 1;
	sprintf(b, "T grew %d MB\n", mb);
	say(b);
	_exit(0);
}

/* fork; the child sets the soft limit to cur, hard to max; exec the grower */
static void
try(const char *label, int mb, quad_t cur, quad_t max)
{
	char b[160], num[16];
	char *av[4];
	struct rlimit r;
	int st, e = 0, p[2];
	pid_t c;

	pipe(p);
	c = fork();
	if (c == 0) {
		close(p[0]);
		r.rlim_cur = cur;
		r.rlim_max = max;
		if (setrlimit(RLIMIT_STACK, &r) < 0) {
			e = errno;
			write(p[1], &e, sizeof e);
			_exit(98);
		}
		sprintf(num, "%d", mb);
		av[0] = "init"; av[1] = "grow"; av[2] = num; av[3] = 0;
		execv("/sbin/init", av);
		e = errno;
		write(p[1], &e, sizeof e);
		_exit(99);
	}
	close(p[1]);
	if (read(p[0], &e, sizeof e) != sizeof e)
		e = 0;
	close(p[0]);
	waitpid(c, &st, 0);
	if (WIFSIGNALED(st))
		sprintf(b, "T %s: errno=%d, killed by signal %d\n", label, e, WTERMSIG(st));
	else
		sprintf(b, "T %s: errno=%d, exit %d\n", label, e, WEXITSTATUS(st));
	say(b);
}

int
main(int argc, char **argv)
{
	struct rlimit r;
	char b[160];
	int mb;

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	if (argc > 2 && strcmp(argv[1], "grow") == 0)
		grow(atoi(argv[2]));
	say("T begin\n");
	getrlimit(RLIMIT_STACK, &r);
	sprintf(b, "T default stack limit: %lu KB\n", (unsigned long)(r.rlim_cur / 1024));
	say(b);
	sprintf(b, "T stack maximum: %lu MB\n", (unsigned long)(r.rlim_max >> 20));
	say(b);
	mb = (int)(r.rlim_max >> 20);

	sprintf(b, "within the maximum, %d MB", mb - 1);
	try(b, mb - 1, r.rlim_max, r.rlim_max);
	sprintf(b, "past the maximum, %d MB", mb + 1);
	try(b, mb + 1, r.rlim_max, r.rlim_max);
	try("past the old maximum, 129 MB", 129, r.rlim_max, r.rlim_max);
	/*
	 * the root asks for 128 MB: the kernel caps it at the maximum
	 * (maxsmap), and the exec must not die. (Revised by the integrator:
	 * the cap is now read back, so this line tests it.)
	 */
	r.rlim_cur = r.rlim_max = 128LL << 20;
	setrlimit(RLIMIT_STACK, &r);
	getrlimit(RLIMIT_STACK, &r);
	sprintf(b, "T root asks for 128 MB, gets: %lu MB, maximum %lu MB\n",
	    (unsigned long)(r.rlim_cur >> 20), (unsigned long)(r.rlim_max >> 20));
	say(b);
	try("root, limit 128 MB, grow 1 MB", 1, 128LL << 20, 128LL << 20);
	say("T end\n");
	reboot(RB_HALT);
	_exit(0);
}
