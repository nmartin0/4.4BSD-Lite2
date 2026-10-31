/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/ktest.sh as /sbin/init. It checks the ELF loader's read
 * of the program headers (patch 246, kern_exec.c's use of
 * elf_read_from). The loader must refuse a file whose program headers
 * lie beyond the end of the file, with ENOEXEC; before the fix a short
 * read was taken for success and the headers were whatever the kernel's
 * buffer held.
 *
 * It makes /short, the first 52 bytes (the ELF header) of /sbin/init
 * with e_phoff set to 52, the end of the file, and e_phnum 1; and /good,
 * a whole copy of /sbin/init. Execing /good must work; /short must give
 * ENOEXEC; /short2, whose headers begin inside the file but run past
 * its end (e_phoff 40, 12 bytes of the 32 present), the same.
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
extern char **environ;

static void mk(const char *name, int len, int phoff)
{
	static char buf[200000];
	int f = open("/sbin/init", O_RDONLY), g, n;
	n = read(f, buf, sizeof buf);
	close(f);
	if (n <= 0 || n == sizeof buf)
		say("T /sbin/init does not fit the test's buffer\n");
	if (len < 0) len = n;
	if (phoff >= 0) {		/* e_phoff at 28, e_phnum at 44 */
		buf[28] = phoff; buf[29] = buf[30] = buf[31] = 0;
		buf[44] = 1; buf[45] = 0;
	}
	g = open(name, O_WRONLY|O_CREAT|O_TRUNC, 0755);
	if (g < 0 || write(g, buf, len) != len) {
		char b[80];
		sprintf(b, "T cannot make %s: errno=%d\n", name, errno);
		say(b);
	}
	close(g);
}

static void tryexec(const char *label, const char *path)
{
	char b[120], *av[3], *ev[2];
	int st, e;
	int p[2];
	pid_t c;
	pipe(p);
	c = fork();
	if (c == 0) {
		av[0] = (char *)path; av[1] = "ctl"; av[2] = 0; ev[0] = 0;
		execve(path, av, ev);
		e = errno;
		write(p[1], &e, sizeof e);
		_exit(99);
	}
	close(p[1]);
	e = 0;
	if (read(p[0], &e, sizeof e) != sizeof e) e = 0;
	wait(&st);
	sprintf(b, "T %s: exec errno=%d\n", label, e);
	say(b);
}

int main(int argc, char **argv)
{
	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	if (argc > 1) { say("T child ran\n"); _exit(0); }
	say("T begin\n");
	{	/* the root file system is mounted read-only: make it writable */
		/* only fspec matters (0: no new device); the kernel copies
		 * in sizeof(struct ufs_args), possibly more than this */
		struct { char *fspec; char rest[64]; } a;
		char b[80];
		memset(&a, 0, sizeof a);
		if (mount("ufs", "/", MNT_UPDATE, &a) < 0) {
			sprintf(b, "T remount: errno=%d\n", errno);
			say(b);
		}
	}
	mk("/good", -1, -1);
	mk("/short", 52, 52);
	mk("/short2", 52, 40);
	tryexec("good copy", "/good");
	tryexec("headers past the end", "/short");
	tryexec("headers cut by the end", "/short2");
	say("T end\n");
	reboot(RB_HALT);
	for (;;) ;
}
