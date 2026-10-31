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
 * test for lib/libc/stdlib/strtoimax.c and strtoumax.c's "0x with no
 * hex digit" fix (docs/provenance/smaller-fixes.md). Both functions
 * are pure string/arithmetic code with no OS dependency at all, so
 * this #includes both real source files directly and calls the real
 * functions -- not a copy that can drift. They're renamed with
 * #define (my_strtoimax/my_strtoumax) only to avoid clashing with
 * the host libc's own strtoimax/strtoumax, which this file doesn't
 * otherwise use.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#define strtoimax my_strtoimax
#include "../../usr/src/lib/libc/stdlib/strtoimax.c"
#undef strtoimax

#define strtoumax my_strtoumax
#include "../../usr/src/lib/libc/stdlib/strtoumax.c"
#undef strtoumax

static int fails;

static void
check_signed(const char *label, const char *input, int base,
    intmax_t want_val, int want_errno, int want_consumed)
{
	char *endptr;
	intmax_t got;

	errno = 0;
	got = my_strtoimax(input, &endptr, base);
	printf("T %s: val=%jd errno=%d consumed=%d\n", label, (intmax_t)got,
	    errno, (int)(endptr - input));
	if (got != want_val || errno != want_errno ||
	    (int)(endptr - input) != want_consumed) {
		printf("T   FAIL: want val=%jd errno=%d consumed=%d\n",
		    (intmax_t)want_val, want_errno, want_consumed);
		fails = 1;
	}
}

static void
check_unsigned(const char *label, const char *input, int base,
    uintmax_t want_val, int want_errno, int want_consumed)
{
	char *endptr;
	uintmax_t got;

	errno = 0;
	got = my_strtoumax(input, &endptr, base);
	printf("T %s: val=%ju errno=%d consumed=%d\n", label, (uintmax_t)got,
	    errno, (int)(endptr - input));
	if (got != want_val || errno != want_errno ||
	    (int)(endptr - input) != want_consumed) {
		printf("T   FAIL: want val=%ju errno=%d consumed=%d\n",
		    (uintmax_t)want_val, want_errno, want_consumed);
		fails = 1;
	}
}

int
main(void)
{
	/*
	 * The bug: "0xg" (base 0) used to consume "0x" unconditionally,
	 * then fail entirely on 'g' (not a hex digit) -- EINVAL, endptr
	 * back at the very start, discarding the valid leading "0".
	 * Fixed: base falls back to octal, "0" is accepted, endptr
	 * lands right after it (consumed == 1), no error.
	 */
	check_signed("imax \"0xg\" base 0", "0xg", 0, 0, 0, 1);
	check_unsigned("umax \"0xg\" base 0", "0xg", 0, 0, 0, 1);

	/* Same shape, explicit base 16: falls through to "0" is itself
	 * a valid (if trivial) base-16 digit, then 'x' stops the scan. */
	check_signed("imax \"0xg\" base 16", "0xg", 16, 0, 0, 1);

	/* A real hex number must still work exactly as before. */
	check_signed("imax \"0x1A\" base 0", "0x1A", 0, 0x1A, 0, 4);
	check_unsigned("umax \"0xFF\" base 0", "0xFF", 0, 0xFF, 0, 4);

	/* "0x" with nothing at all after it: same fallback-to-octal
	 * shape, consumed == 1 (just the leading "0"). */
	check_signed("imax \"0x\" alone, base 0", "0x", 0, 0, 0, 1);

	/* Plain octal and decimal numbers, unaffected by this fix. */
	check_signed("imax \"017\" base 0 (octal)", "017", 0, 15, 0, 3);
	check_signed("imax \"17\" base 0 (decimal)", "17", 0, 17, 0, 2);

	/* A real "0x" prefix followed by trailing non-hex text: the
	 * hex number itself is still parsed in full, and only the
	 * trailing text is left unconsumed. */
	check_signed("imax \"0x2Fxyz\" base 0", "0x2Fxyz", 0, 0x2F, 0, 4);

	if (fails)
		printf("T FAIL\n");
	else
		printf("T all checks passed\n");
	return fails;
}
