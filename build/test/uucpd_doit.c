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
 * test for libexec/uucpd/uucpd.c's doit() ordering fix
 * (docs/provenance/uucpd-doit.md): chdir(pw->pw_dir) used to run
 * before setuid(pw->pw_uid), so it ran as root and so it always
 * succeeded, even into a directory the uucp user has no permission
 * to enter at all -- the chdir() permission check chdir() is meant
 * to make was bypassed entirely by root's own privilege.
 *
 * doit() itself can't be called directly: it reads a login and
 * password from fd 0, forks nothing of its own, and ends by really
 * calling setuid()/execl() on the real uucico binary -- not
 * something to run inside a test program, and this process is only
 * root while the test harness itself is (see build/test/uucpd_doit.sh).
 * Instead, this reproduces the exact ordering mistake directly, using
 * real chdir()/setuid() calls in forked children against a real
 * scratch directory with real, restrictive permissions -- not a
 * mock -- so what is shown is the actual kernel-enforced permission
 * check, not an assertion about which order two stand-in functions
 * ran in.
 *
 * This only demonstrates anything when actually run as root (uid 0):
 * chdir()'s permission bypass for root is the whole bug, so without
 * root neither order can be told apart. build/test/uucpd_doit.sh
 * checks this and skips cleanly, rather than failing, when not root.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fails;

/*
 * Run one ordering in a forked child (setuid() is one-way, so each
 * ordering needs its own fresh process) and report whether chdir()
 * into `dir' succeeded. chdir_first: true reproduces the bug order
 * (chdir while still root, then drop privilege); false is the fixed
 * order (drop privilege first, then chdir as the unprivileged uid).
 */
static int
try_order(const char *dir, uid_t target_uid, int chdir_first)
{
	pid_t pid;
	int status;
	int fds[2];
	char buf[64];
	ssize_t n;

	if (pipe(fds) == -1)
		err(2, "pipe");
	pid = fork();
	if (pid == -1)
		err(2, "fork");
	if (pid == 0) {
		int chdir_rv, chdir_errno;

		close(fds[0]);
		if (chdir_first) {
			chdir_rv = chdir(dir);
			chdir_errno = errno;
			(void) setuid(target_uid);
		} else {
			if (setuid(target_uid) == -1) {
				/* couldn't drop privilege -- report that
				 * distinctly so it's never read as a pass */
				write(fds[1], "setuid-failed", 13);
				close(fds[1]);
				_exit(0);
			}
			chdir_rv = chdir(dir);
			chdir_errno = errno;
		}
		if (chdir_rv == 0)
			write(fds[1], "ok", 2);
		else
			write(fds[1], strerror(chdir_errno),
			    strlen(strerror(chdir_errno)));
		close(fds[1]);
		_exit(0);
	}
	close(fds[1]);
	n = read(fds[0], buf, sizeof buf - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(fds[0]);
	waitpid(pid, &status, 0);

	printf("T %-38s -> %s\n",
	    chdir_first ? "bug order   (chdir, then setuid)" :
	                  "fixed order (setuid, then chdir)",
	    buf);

	if (strcmp(buf, "setuid-failed") == 0) {
		printf("T   (setuid() to the unprivileged uid itself "
		    "failed -- see below)\n");
		return (-1);
	}
	return (strcmp(buf, "ok") == 0);
}

int
main(void)
{
	char tmpl[] = "/tmp/uucpd_doit_test.XXXXXX";
	char *dir;
	uid_t other_uid = 1;	/* "daemon" on this system; not 0, not us */
	int bug_ok, fixed_ok;

	printf("T begin\n");

	if (getuid() != 0) {
		/*
		 * Not root: chdir()'s permission bypass for root is the
		 * whole bug, so there is nothing to show either way.
		 * build/test/uucpd_doit.sh checks for this before even
		 * building and skips cleanly; this is a second,
		 * belt-and-suspenders check in case the test is run
		 * directly rather than through that script.
		 */
		printf("T not root -- skipping (see uucpd_doit.sh)\n");
		printf("T end\n");
		return (0);
	}

	dir = mkdtemp(tmpl);
	if (dir == NULL) {
		printf("T mkdtemp failed, skipping\n");
		printf("T end\n");
		return (0);
	}
	/*
	 * Owned by root, readable/searchable by no one else at all --
	 * not even "daemon" -- the same shape as a uucp user's home
	 * directory the uucp account itself was somehow locked out of
	 * (permissions changed, moved, whatever); the actual pw_dir
	 * ownership doesn't matter here, only that mode 0700 denies
	 * every uid but the owner (root).
	 */
	if (chmod(dir, 0700) == -1)
		err(2, "chmod");

	bug_ok = try_order(dir, other_uid, 1);
	fixed_ok = try_order(dir, other_uid, 0);

	/*
	 * The property the donor's fix establishes: the bug order lets
	 * chdir() into a directory the target uid has no permission
	 * for at all succeed anyway (bypassed by root), while the
	 * fixed order is subject to the real permission check and
	 * fails for that same uid into that same directory.
	 */
	if (bug_ok != 1) {
		printf("T   FAIL: expected the bug order's chdir() to "
		    "succeed (root bypasses the check); it did not -- "
		    "this probe no longer reproduces the bug\n");
		fails++;
	}
	if (fixed_ok != 0) {
		printf("T   FAIL: expected the fixed order's chdir() to "
		    "fail (permission denied for an unprivileged uid); "
		    "got %d\n", fixed_ok);
		fails++;
	}

	rmdir(dir);

	printf("T %s\n", fails ? "FAIL" : "all checks passed");
	printf("T end\n");
	return (fails != 0);
}
