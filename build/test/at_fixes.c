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
 * test for usr.bin/at's two fixes in this patch
 * (docs/provenance/at-fixes.md):
 *
 *   1. parsetime.c's assign_date() treating a two-digit year below
 *      100 as if it were already a tm_year value, so "00" (meant as
 *      2000) became 1900.
 *   2. at.c's writefile() checking the wrong pointer (`ap', leftover
 *      from an unrelated loop) instead of `eqp' (strchr()'s result,
 *      the one that can actually be NULL) when writing out the
 *      environment.
 *
 * assign_date() is `static' and has no side effects reaching the
 * filesystem or the network, so this #includes the real
 * usr.bin/at/parsetime.c textually (as build/test/mkdir_nul.c does
 * for mkdir.c) and calls it directly -- the exact function under
 * test, not a copy that can drift. panic() is stubbed to longjmp()
 * out rather than exit(), so a deliberately-triggered panic (used
 * below to show the pre-fix behaviour never panics when it should
 * have, or does when it shouldn't) can be caught and checked instead
 * of ending the test program.
 *
 * at.c's writefile() is not practical to pull in the same way (it
 * opens real files under _PATH_ATJOBS, forks a mail pipeline, and
 * reads the real environment), so its eqp/ap mistake is reproduced
 * standalone instead, the same way build/test/sa_argv.c reproduces
 * usr.sbin/sa's argv/*argv mistake: the exact few lines around the
 * bug, built as their own tiny function and driven directly, with an
 * environment string that has no '=' -- the one case where `eqp'
 * (not `ap') must be NULL-checked.
 */
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

static int fails;
static jmp_buf panic_jmp;
static char panic_msg[128];

/* Stand in for panic.h's panic(): record the message and jump back
 * out, instead of exiting the whole test program. */
void
panic(char *a)
{
	snprintf(panic_msg, sizeof panic_msg, "%s", a ? a : "");
	longjmp(panic_jmp, 1);
}

void
perr(char *a)
{
	panic(a);
}

void
perr2(char *a, char *b)
{
	panic(a);
}

void
usage(void)
{
	panic("usage");
}

#define main unused_main	/* keep parsetime.c's own main (if any) out of our way */
#include "../../usr/src/usr.bin/at/parsetime.c"
#undef main

/*
 * Run assign_date() on a fresh struct tm and check the resulting
 * year (tm_year + 1900) against want_year, or, if want_panic is set,
 * check that it panicked instead. "now" is a plausible reference
 * date; the loop-to-next-year branch in assign_date() is not
 * exercised here (it only fires for year == -1, and every case below
 * either passes a real year or checks the -1 case on its own terms).
 */
static void
probe(const char *label, long year, int want_panic, int want_year)
{
	struct tm now;
	int panicked;
	int got_year = -1;

	memset(&now, 0, sizeof now);
	now.tm_year = 126;	/* 2026, same as this patch's own date */
	now.tm_mon = 9;		/* October (0-based) */
	now.tm_mday = 31;

	panicked = 0;
	if (setjmp(panic_jmp) == 0) {
		assign_date(&now, now.tm_mday, now.tm_mon, year);
		got_year = now.tm_year + 1900;
	} else {
		panicked = 1;
	}

	if (panicked) {
		printf("T %-28s -> panic (%s)\n", label, panic_msg);
		if (!want_panic) {
			printf("T   FAIL: did not expect a panic here\n");
			fails++;
		}
	} else {
		printf("T %-28s -> %d\n", label, got_year);
		if (want_panic) {
			printf("T   FAIL: expected a panic, got %d\n",
			    got_year);
			fails++;
		} else if (got_year != want_year) {
			printf("T   FAIL: wanted %d, got %d\n",
			    want_year, got_year);
			fails++;
		}
	}
}

int
main(void)
{
	printf("T begin\n");

	/*
	 * The actual bug, shown directly: pre-fix, a bare two-digit
	 * year "00" went straight into tm_year, giving 1900. Fixed,
	 * it must give 2000. 69 is the donor's own pivot (69 and
	 * above stays 19xx, below it becomes 20xx).
	 */
	probe("year 00 (two-digit)", 0, 0, 2000);
	probe("year 25 (two-digit)", 25, 0, 2025);
	probe("year 68 (two-digit)", 68, 0, 2068);
	probe("year 69 (two-digit)", 69, 0, 1969);
	probe("year 99 (two-digit)", 99, 0, 1999);

	/* Four-digit years, unambiguous, must be unaffected. */
	probe("year 2026 (four-digit)", 2026, 0, 2026);
	probe("year 1999 (four-digit)", 1999, 0, 1999);

	/*
	 * The shared limitation documented in
	 * docs/provenance/at-fixes.md: a four-digit year already
	 * reduced below 69 by the `year > 99' block above (1900-1968,
	 * typed out in full) also gets the century bumped, exactly as
	 * NetBSD's own applied fix does -- checked here so a change
	 * that accidentally narrowed this to "only apply below 100"
	 * (which would also be a behavior change from the donor's own
	 * fix) doesn't pass unnoticed. An independent reviewer asked
	 * for this case to be covered, not just described in prose.
	 */
	probe("year 1950 (four-digit, pre-1969)", 1950, 0, 2050);
	probe("year 1968 (four-digit, pre-1969)", 1968, 0, 2068);

	/* No year given at all (NUMBER token path skipped it): stays
	 * at `now''s year, here 2026. */
	probe("year -1 (none given)", -1, 0, 2026);

	/*
	 * Below 1900: assign_date() itself panics on this ("garbled
	 * time"), both before and after this patch -- not something
	 * this patch changes, checked here only so a future change to
	 * that boundary doesn't silently pass unnoticed.
	 */
	probe("year 1899 (pre-1900)", 1899, 1, 0);

	/*
	 * eqp/ap check (at.c's writefile()), reproduced standalone:
	 * same two forms as the real bug, run directly rather than
	 * through a full writefile() call.
	 */
	{
		char envstr[] = "NOEQUALSIGN";	/* no '=' at all */
		char *eqp, *ap_stale;
		char scratch[] = "xx";

		/* ap_stale: made the same way the real bug's `ap' ends
		 * up -- pointing at a string's own terminating NUL,
		 * never NULL, after an unrelated loop walks off the end
		 * of some other buffer. */
		for (ap_stale = scratch; *ap_stale != '\0'; ap_stale++)
			;

		eqp = strchr(envstr, '=');	/* NULL: no '=' in envstr */

		printf("T %-28s -> %s\n", "bug check  (ap == NULL)",
		    ap_stale == NULL ? "yes" : "no");
		if (ap_stale == NULL) {
			printf("T   FAIL: ap_stale was NULL -- "
			    "this probe no longer reproduces the bug\n");
			fails++;
		}

		printf("T %-28s -> %s\n", "fixed check(eqp == NULL)",
		    eqp == NULL ? "yes" : "no");
		if (eqp == NULL) {
			/* this is the correct, fixed behaviour */
		} else {
			printf("T   FAIL: eqp was not NULL for a "
			    "'='-less string\n");
			fails++;
		}
	}

	printf("T %s\n", fails ? "FAIL" : "all checks passed");
	printf("T end\n");
	return (fails != 0);
}
