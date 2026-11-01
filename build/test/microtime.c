/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * AI-ONLY NOTE: a test, not part of the system; run by
 * build/test/ktest.sh as /sbin/init. It exercises microtime, NetBSD
 * 1.0's microtime.s (docs/provenance/microtime.md), through the only
 * door a program has to it, gettimeofday, in a tight loop.
 *
 * It asks four things of the answers, taken as one 64-bit number of
 * microseconds: that none is smaller than the one before (a tick that
 * has been counted by the 8254 but not yet added to `time' makes a
 * wrong microtime answer a tick early, and then put the clock back
 * when the tick is added); that tv_usec is always under a million;
 * that the loop crosses a hundred tick boundaries or more, the
 * boundary being where the bug lives; and that the answers are finer
 * than a tick, more than 100 distinct values falling inside one 10 ms
 * tick. The largest step is printed, not judged: a stall of the
 * emulator's host shows in it and says nothing about the kernel. A
 * microtime that returns `time' plus a whole tick, which is what this
 * port had, passes the first two and fails the last.
 *
 * It prints counts, never times, so that the expected output is the
 * same on every machine. QEMU's clock is emulated, and what a pass
 * shows is in docs/provenance/microtime.md.
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

static void say(const char *s) { write(2, s, strlen(s)); }

#define NCALLS	3000000L	/* at most */
#define SPAN	(2 * 1000000LL)	/* run for two seconds of the kernel's own clock */
#define TICK	10000LL		/* hz is 100 */

int
main(int argc, char **argv)
{
	struct timeval tv;
	long long now, last = 0, first = 0, step, maxstep = 0;
	long long tickbase, distinct = 0, bestdistinct = 0, boundaries = 0;
	long back = 0, badusec = 0, n;
	long long worst = 0;
	char b[160];

	open("/dev/console", O_RDWR); dup2(0, 1); dup2(0, 2);
	say("T begin\n");

	tickbase = -1;
	for (n = 0; n < NCALLS; n++) {
		if (gettimeofday(&tv, 0) < 0) {
			say("T gettimeofday failed\n");
			break;
		}
		if (tv.tv_usec < 0 || tv.tv_usec >= 1000000)
			badusec++;
		now = (long long)tv.tv_sec * 1000000 + tv.tv_usec;
		if (n == 0) {
			first = now;
		} else {
			step = now - last;
			if (step < 0) {
				back++;
				if (-step > worst)
					worst = -step;
			} else if (step > maxstep)
				maxstep = step;
		}
		/*
		 * count the values inside each ten-millisecond window of
		 * the answers, windows starting at the first answer; keep
		 * the best window's count. The values of a window are
		 * distinct if each differs from the one before it.
		 */
		if (n == 0 || now / TICK != tickbase) {
			if (distinct > bestdistinct)
				bestdistinct = distinct;
			distinct = 0;
			tickbase = now / TICK;
			boundaries++;
		}
		if (n == 0 || now != last)
			distinct++;
		last = now;
		if (now - first >= SPAN)
			break;
	}
	if (distinct > bestdistinct)
		bestdistinct = distinct;

	sprintf(b, "T the clock never went backwards: %s\n", back == 0 ? "yes" : "NO");
	say(b);
	if (back) {
		sprintf(b, "  backwards %ld times, worst %lld us\n", back, worst);
		write(2, b, strlen(b));	/* not a T line: the want file has no number */
	}
	sprintf(b, "T tv_usec always under a million: %s\n", badusec == 0 ? "yes" : "NO");
	say(b);
	sprintf(b, "T crossed 100 or more ticks: %s\n", boundaries >= 100 ? "yes" : "NO");
	say(b);
	sprintf(b, "T over 100 distinct values in one tick: %s\n", bestdistinct > 100 ? "yes" : "NO");
	say(b);
	sprintf(b, "  calls %ld, ticks %lld, maxstep %lld us, best window %lld\n",
	    n, boundaries, maxstep, bestdistinct);
	write(2, b, strlen(b));
	say("T end\n");
	reboot(RB_HALT);
	for (;;)
		;
}
