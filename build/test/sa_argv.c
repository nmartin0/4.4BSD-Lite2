/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * This file deliberately carries no SPDX licence identifier: the
 * licence for this tree's own tests is not settled yet, and the tag
 * is kept out rather than written into a sentence, where a scanner
 * would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
 * cover the Berkeley material, not this file.
 *
 * Nothing here is copied from another system. AI-ONLY NOTE: a host
 * test for usr.sbin/sa/main.c's two warn("...%s", argv) calls
 * (docs/provenance/sa-argv.md). Reaching those two calls through a
 * real sa(8) run needs a live, privileged accounting file and a
 * forced ftruncate()/close() failure, which isn't practical to stage
 * here, so this reproduces the exact mistake directly: `argv' in
 * that part of main() is `char **', the current position in main's
 * own argv array as the per-file loop advances it (`argc--,
 * argv++'), and %s was printing that pointer's own value instead of
 * the string it points to (`argv[0]', i.e. `*argv'). This test
 * builds that same local setup (an argv array) and captures what
 * libc's own warn() actually writes for both forms, so the
 * difference is observed, not asserted.
 */
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static char buf[4096];

static void
capture(void (*body)(char **), char **argv)
{
	int fds[2];
	pid_t pid;
	ssize_t n;
	size_t total;
	int status;

	if (pipe(fds) == -1)
		err(2, "pipe");
	fflush(stderr);
	pid = fork();
	if (pid == -1)
		err(2, "fork");
	if (pid == 0) {
		close(fds[0]);
		dup2(fds[1], 2);
		close(fds[1]);
		body(argv);
		_exit(0);
	}
	close(fds[1]);
	/* Read to EOF (the child closing its end), not just once: a
	 * single read() can return a short chunk and leave the rest
	 * sitting in the pipe, which made this test flaky at first. */
	total = 0;
	while (total < sizeof buf - 1) {
		n = read(fds[0], buf + total, sizeof buf - 1 - total);
		if (n <= 0)
			break;
		total += (size_t)n;
	}
	buf[total] = '\0';
	close(fds[0]);
	waitpid(pid, &status, 0);
}

/* The bug, exactly as usr.sbin/sa/main.c had it: %s against argv itself. */
static void
bug_form(char **argv)
{
	warn("couldn't truncate %s", argv);
}

/* The fix, exactly as usr.sbin/sa/main.c has it now: %s against *argv. */
static void
fixed_form(char **argv)
{
	warn("couldn't truncate %s", *argv);
}

int
main(void)
{
	/* The same shape as main()'s loop: argv pointing at the file
	 * currently being processed. */
	static char *files[] = { "/var/account/acct", NULL };

	printf("T begin\n");

	/*
	 * The bug form's stderr includes whatever bytes happen to make
	 * up the argv pointer's value, printed as if it were a string --
	 * not reproducible across runs (stack layout, ASLR), so only the
	 * pass/fail check (never finding the real path) is compared
	 * against .want; the raw bytes are shown for a human running
	 * this directly, not asserted on.
	 */
	capture(bug_form, files);
	printf("T bug form   (argv)  stderr shows the path: %s\n",
	    strstr(buf, "/var/account/acct") != NULL ? "yes" : "no");
	fprintf(stderr, "(bug form's actual stderr, for a human: %s)\n", buf);

	capture(fixed_form, files);
	printf("T fixed form (*argv) stderr shows the path: %s\n",
	    strstr(buf, "/var/account/acct") != NULL ? "yes" : "no");

	printf("T end\n");
	return (0);
}
