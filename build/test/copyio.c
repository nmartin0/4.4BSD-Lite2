/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system. It runs as /sbin/init
 * on a bare disk (build/test/copyio.sh makes the disk) and asks the
 * kernel to copy data to and from addresses it should refuse and
 * addresses it should accept, printing one `T ...' line for each.
 * The expected lines are in copyio.sh.
 *
 * Why files and pipes are used as they are. An error from copyout in
 * a pipe's read makes soreceive() go round again and sleep for more
 * data, so a read from a pipe into a bad address never returns -- the
 * 4.4BSD code does this for any bad address (docs/deferred.md). The
 * refused copyouts are therefore done on a file, whose read returns
 * the error at once; the refused copyins are done on a pipe's write,
 * which does.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define USRSTACK 0xEFBFE000UL
static void say(const char *s) { write(2, s, strlen(s)); }
static void rep(const char *name, long n, int e)
{
	char b[160];
	sprintf(b, "T %s: ret=%ld errno=%d\n", name, n, e);
	say(b);
}
extern char **environ;
static int f;
/* read from a regular file: an error comes back at once (a socket's read hangs, see notes) */
static long fr(const char *name, void *addr, int len)
{
	long n; int e;
	lseek(f, 0, SEEK_SET);
	errno = 0; n = read(f, addr, len); e = errno;
	rep(name, n, e);
	return n;
}

int main(int argc, char **argv)
{
	int p[2], st; long n; char buf[32];
	char *kaddr = (char *)0xF0100000;
	char *ro, *w;

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	if (argc > 1) {			/* the exec'd child: arguments arrived intact */
		char b[200];
		sprintf(b, "T child: argc=%d argv1=%s argv2=%s env0=%s\n", argc, argv[1], argv[2], environ[0]);
		say(b); _exit(argc == 3 && strcmp(argv[2], "second-argument") == 0 ? 0 : 3);
	}
	say("T begin\n");
	f = open("/sbin/init", O_RDONLY);
	pipe(p);

	/* A: copyout into kernel space */
	fr("A copyout->kernel", kaddr, 6);
	/* B: copyin from kernel space, to a pipe */
	errno = 0; n = write(p[1], kaddr, 16); rep("B copyin<-kernel", n, errno);
	if (n == 16) { char tmp[32]; read(p[0], tmp, 16); }
	/* C: ordinary copies */
	write(p[1], "hello", 5); memset(buf, 0, sizeof buf);
	errno = 0; n = read(p[0], buf, 5); rep("C pipe copy", n, errno);
	say(strcmp(buf, "hello") == 0 ? "T C: data ok\n" : "T C: data BAD\n");
	memset(buf, 0, sizeof buf); n = fr("C2 file copy", buf, 4);
	say(memcmp(buf, "\177ELF", 4) == 0 ? "T C2: data ok\n" : "T C2: data BAD\n");
	/* E: address arithmetic wraps. Not a discriminating case: with the carry
	 * test removed the copy still ends in a hardware fault at 0xFFFFFFF8, and
	 * a wrapping range that starts on a mapped kernel page needs a length no
	 * read(2) passes down. The carry test is defence in depth (copyout.md). */
	fr("E copyout wrap", (char *)0xFFFFFFF8, 16);
	/* F: copyin wrapping (not discriminating, as E) */
	errno = 0; n = write(p[1], (char *)0xFFFFFFF8, 16); rep("F copyin wrap", n, errno);
	/* G: copyin straddling the end of user space */
	errno = 0; n = write(p[1], (char *)USRSTACK - 8, 16); rep("G copyin straddle", n, errno);
	/* J: zero length with a kernel address. Not shown to reach copyout: the
	 * file layer may return 0 first (copyout.md). */
	errno = 0; n = read(f, kaddr, 0); rep("J zero-length", n, errno);
	/* I: untouched anonymous pages are faulted in and written */
	w = mmap(0, 8192, PROT_READ|PROT_WRITE, MAP_ANON, -1, 0);
	n = fr("I copyout across 2 fresh pages", w + 4092, 8);
	say(memcmp(w + 4092, "\177ELF", 4) == 0 ? "T I: data ok\n" : "T I: data BAD\n");
	/* H: a read-only user page must not be written */
	ro = mmap(0, 4096, PROT_READ, MAP_ANON, -1, 0);
	if (ro[0] != 0) say("T H: page not zero\n");	/* touch it: valid, user, read-only */
	fr("H copyout->ro page", ro, 4);
	say(ro[0] == 0 ? "T H: page unchanged\n" : "T H: page WRITTEN\n");
	/* K: ending exactly at USRSTACK is allowed */
	write(p[1], "01234567", 8);
	errno = 0; n = read(p[0], (char *)USRSTACK - 8, 8); rep("K pipe copyout ending at USRSTACK", n, errno);
	/* M, N: copyin ending exactly at USRSTACK is allowed; one byte past is not */
	errno = 0; n = write(p[1], (char *)USRSTACK - 8, 8); rep("M copyin ending at USRSTACK", n, errno);
	if (n == 8) { char tmp[32]; read(p[0], tmp, 8); }
	errno = 0; n = write(p[1], (char *)USRSTACK - 7, 8); rep("N copyin 1 byte past USRSTACK", n, errno);
	/* D: one byte past USRSTACK */
	fr("D copyout 1 byte past USRSTACK", (char *)USRSTACK - 7, 8);
	/* L: execve with arguments and environment, in a child */
	{
		pid_t c = fork();
		if (c == 0) {
			char *av[] = { "init", "first", "second-argument", 0 };
			char *ev[] = { "ENVTEST=yes", 0 };
			execve("/sbin/init", av, ev); _exit(9);
		}
		wait(&st); rep("L child exit status", WEXITSTATUS(st), 0);
	}
	say("T end\n");
	reboot(RB_HALT);
	for (;;) ;
}
