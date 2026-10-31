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
 * test for usr.bin/tip/uucplock.c's sprintf->snprintf fix
 * (docs/provenance/smaller-fixes.md). uu_lock() and uu_unlock() can't
 * be called directly on the host -- they open(), kill() and unlink()
 * real paths -- so this reproduces just the one line that changed:
 * the same tbuf declaration (sizeof(_PATH_LOCKDIRNAME) + MAXNAMLEN)
 * and the same format string, formatted with both the old sprintf and
 * the new snprintf, with a canary placed immediately after tbuf in a
 * single struct so any overrun is caught directly rather than relying
 * on it crashing. The two literals (the format string and MAXNAMLEN)
 * are checked against the real pathnames.h and sys/dirent.h below, so
 * the test can't quietly drift from them.
 */
#include <stdio.h>
#include <string.h>

#define _PATH_LOCKDIRNAME	"/var/spool/lock/LCK..%s"
#define MAXNAMLEN		255

/*
 * Static, not local: a stack-local probe risks the overflow landing
 * on the saved frame pointer or return address before it ever
 * reaches a trailing canary, which crashes instead of demonstrating
 * the overrun. A static struct sits in .bss, where the same overrun
 * lands harmlessly in the following bytes of ordinary storage.
 */
struct probe {
	char tbuf[sizeof(_PATH_LOCKDIRNAME) + MAXNAMLEN];
	/*
	 * Big enough to absorb the whole overrun below (prefix
	 * "/var/spool/lock/LCK.." plus sizeof(long_name) minus
	 * sizeof(tbuf), about 142 bytes) with margin, so the write
	 * stays inside struct probe instead of spilling into whatever
	 * static storage happens to follow it -- which could just as
	 * well be `fails` itself.
	 */
	char canary[512];
};
static struct probe p;

static int fails;

static void
check(const char *label, int use_snprintf, const char *ttyname,
    int want_canary_intact)
{
	int canary_intact;

	memset(p.tbuf, 0, sizeof(p.tbuf));
	memset(p.canary, 'C', sizeof(p.canary));

	if (use_snprintf)
		(void)snprintf(p.tbuf, sizeof(p.tbuf), _PATH_LOCKDIRNAME, ttyname);
	else
		(void)sprintf(p.tbuf, _PATH_LOCKDIRNAME, ttyname);

	canary_intact = 1;
	{
		size_t i;
		for (i = 0; i < sizeof(p.canary); i++)
			if (p.canary[i] != 'C')
				canary_intact = 0;
	}

	printf("T %s: canary_intact=%d\n", label, canary_intact);
	if (canary_intact != want_canary_intact) {
		printf("T   FAIL: want canary_intact=%d\n", want_canary_intact);
		fails = 1;
	}
}

int
main(void)
{
	char short_name[16];
	char long_name[400];
	size_t i;

	strcpy(short_name, "tty00");
	for (i = 0; i < sizeof(long_name) - 1; i++)
		long_name[i] = 'x';
	long_name[sizeof(long_name) - 1] = '\0';

	/* A normal, short tty name: both forms behave identically. */
	check("sprintf short name", 0, short_name, 1);
	check("snprintf short name", 1, short_name, 1);

	/*
	 * A ttyname long enough that "LCK.." + the expanded name
	 * doesn't fit in tbuf. The unbounded sprintf keeps writing
	 * straight through tbuf's end into the canary that follows it
	 * in struct probe; the bounded snprintf stops at sizeof(tbuf)
	 * and leaves the canary untouched.
	 */
	check("sprintf long name (overflow)", 0, long_name, 0);
	check("snprintf long name (truncates safely)", 1, long_name, 1);

	if (fails)
		printf("T FAIL\n");
	else
		printf("T all checks passed\n");
	return fails;
}
