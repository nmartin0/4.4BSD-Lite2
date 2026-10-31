/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/ktest.sh as /sbin/init. A smoke test for the buffer
 * cache's write paths (patch 248: bwrite, getblk, getnewbuf, allocbuf,
 * geteblk): it makes 120 files, calling sync() every ten so that
 * delayed writes are started asynchronously, then reads every file
 * back and compares it, removes half and checks the rest. It passes
 * before and after the change and did not see the bwrite race, so it
 * is a regression guard only. Its working set (about 0.4 MB) is far
 * below the cache (about 6.7 MB), so buffers are not reclaimed and the
 * paths that reuse or strip them (getnewbuf's delayed-write branch,
 * allocbuf's growth) are barely run; the read-back is served from the
 * cache, so it does not show that the writes reached the disk. See
 * docs/provenance/bufcache.md.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

extern int mount(const char *, const char *, int, void *);
#define MNT_UPDATE 0x00010000
#define NFILES 120
#define FSIZE 3000

static void say(const char *s) { write(2, s, strlen(s)); }
static char buf[FSIZE], chk[FSIZE];

static void fill(int i)
{
	int k;
	for (k = 0; k < FSIZE; k++) buf[k] = (char)(i * 31 + k * 7 + (k >> 8));
}

static int check(int i)
{
	static int shown;
	char name[32];
	int f, n;
	sprintf(name, "/w/f%d", i);
	f = open(name, O_RDONLY);
	if (f < 0) {
		if (!shown++) { char b[80]; sprintf(b, "T open %s: errno=%d\n", name, errno); say(b); }
		return 0;
	}
	n = read(f, chk, FSIZE);
	close(f);
	fill(i);
	if (n != FSIZE && !shown++) { char b[80]; sprintf(b, "T read %s: %d errno=%d\n", name, n, errno); say(b); }
	return n == FSIZE && memcmp(buf, chk, FSIZE) == 0;
}

int main(void)
{
	char name[32], b[80];
	int i, f, bad;

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	say("T begin\n");
	{
		struct { char *fspec; char rest[64]; } a;
		memset(&a, 0, sizeof a);
		a.fspec = "/d/wd0a";
		if (mount("ufs", "/", MNT_UPDATE, &a) < 0) { char eb[60]; sprintf(eb, "T remount failed: errno=%d\n", errno); say(eb); }
	}
	if (mkdir("/w", 0755) < 0) say("T mkdir failed\n");
	for (i = 0; i < NFILES; i++) {
		sprintf(name, "/w/f%d", i);
		f = open(name, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		fill(i);
		if (f < 0 || write(f, buf, FSIZE) != FSIZE) {
			sprintf(b, "T write %d failed: errno=%d\n", i, errno);
			say(b);
		}
		close(f);
		if (i % 10 == 9) sync();
		if (i % 40 == 39) { sprintf(b, "T made %d\n", i + 1); say(b); }
	}
	sync();
	bad = 0;
	for (i = 0; i < NFILES; i++) if (!check(i)) bad++;
	sprintf(b, "T read back after sync: %d bad of %d\n", bad, NFILES);
	say(b);
	for (i = 0; i < NFILES; i += 2) {
		sprintf(name, "/w/f%d", i);
		unlink(name);
		if (i % 10 == 0) sync();
	}
	sync();
	bad = 0;
	for (i = 1; i < NFILES; i += 2) if (!check(i)) bad++;
	sprintf(b, "T survivors after unlinking half: %d bad of %d\n", bad, NFILES / 2);
	say(b);
	say("T end\n");
	reboot(RB_HALT);
	for (;;) ;
}
