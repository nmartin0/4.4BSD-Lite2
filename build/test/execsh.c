/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/execsh.sh, which puts this tree's own /bin/sh and ls on
 * the disk beside it (as /bin/sh and /x/ls). Where execve.c runs
 * itself in every role, this runs the system's real programs through
 * exec: a `#!/bin/sh' script that runs ls by command substitution and
 * as a plain command, and ends by exec'ing it; and `sh -c'. It passes
 * on the kernel before patch 262 as well -- it is there to catch a
 * regression in real programs, not to tell the two apart
 * (docs/provenance/exec-stack.md).
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

extern int mount(const char *, const char *, int, void *);
#define MNT_UPDATE 0x00010000
static void say(const char *s) { write(2, s, strlen(s)); }

static void
run(const char *label, const char *path, char **av, char **ev)
{
	char b[160];
	int p[2], st, e;
	pid_t c;

	pipe(p);
	c = fork();
	if (c == 0) {
		close(p[0]);
		execve(path, av, ev);
		e = errno;
		write(p[1], &e, sizeof e);
		_exit(99);
	}
	close(p[1]);
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

static const char script[] =
	"#!/bin/sh\n"
	"echo \"T sh: \\$0=$0 \\$1=$1 \\$#=$#\"\n"
	"echo \"T ls by substitution:\" $(/x/ls /x)\n"
	"/x/ls /nothing-here\n"
	"echo \"T ls of a missing file: status $?\"\n"
	"exec /x/ls -d /x\n";

int
main(int argc, char **argv)
{
	char *av[4], *ev[2], b[120];
	int g;

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	say("T begin\n");
	{
		struct { char *fspec; char rest[64]; } a;
		memset(&a, 0, sizeof a);
		a.fspec = "/d/wd0a";
		if (mount("ufs", "/", MNT_UPDATE, &a) < 0) {
			sprintf(b, "T remount: errno=%d\n", errno);
			say(b);
		}
	}
	g = open("/s", O_WRONLY|O_CREAT|O_TRUNC, 0755);
	write(g, script, sizeof script - 1);
	close(g);
	ev[0] = "PATH=/x"; ev[1] = 0;

	av[0] = "s"; av[1] = "one"; av[2] = 0;
	run("a #!/bin/sh script", "/s", av, ev);
	av[0] = "sh"; av[1] = "-c"; av[2] = "echo \"T sh -c: $#\"; ls -d /bin"; av[3] = 0;
	run("sh -c", "/bin/sh", av, ev);

	say("T end\n");
	reboot(RB_HALT);
	for (;;)
		;
}
