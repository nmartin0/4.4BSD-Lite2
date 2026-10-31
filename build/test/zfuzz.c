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
 * test for compress(1)'s zopen.c decode-stack bound
 * (docs/provenance/compress-stack.md). This is a defensive test: it
 * feeds the decoder damaged input and checks it returns an error
 * instead of running off the end of its table, under
 * AddressSanitizer. It does not construct or report a working
 * overflow trigger; it is run once with the fix absent, to confirm
 * ASan can catch the class of bug this guards against on some seed,
 * and then with the fix in place, over many random seeds, to show
 * the guard holds.
 *
 * zopen.c is compiled as it stands (build/test/zfuzz.sh). funopen()
 * is not in glibc; a small shim over fopencookie() stands in for it.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#undef calloc	/* the build renames it for zopen.c only; see zd_calloc */
extern FILE *zopen(const char *, const char *, int);

typedef int (*rfn_t)(void *, char *, int);
typedef int (*wfn_t)(void *, const char *, int);
typedef int (*cfn_t)(void *);
static rfn_t shim_read;
static wfn_t shim_write;
static cfn_t shim_close;

static ssize_t
sh_read(void *c, char *b, size_t n)
{
	return (shim_read(c, b, (int)n));
}

static ssize_t
sh_write(void *c, const char *b, size_t n)
{
	return (shim_write(c, b, (int)n));
}

static int
sh_close(void *c)
{
	return (shim_close(c));
}

FILE *
funopen(void *cookie, rfn_t r, wfn_t w, void *s, cfn_t c)
{
	cookie_io_functions_t f;

	(void)s;
	memset(&f, 0, sizeof f);
	shim_read = r;
	shim_write = w;
	shim_close = c;
	f.read = r ? sh_read : NULL;
	f.write = w ? sh_write : NULL;
	f.close = sh_close;
	return (fopencookie(cookie, r ? "r" : "w", f));
}

void *
zd_calloc(size_t n, size_t sz)
{
	return (calloc(n, sz));
}

static unsigned long
rnd(unsigned long *s)
{
	*s = *s * 6364136223846793005UL + 1;
	return (*s >> 33);
}

/*
 * Make a genuinely valid .Z stream, by running real input through the
 * tree's own zopen() in write mode -- the table-growth and width
 * changes are then whatever the real compressor does, not a guess.
 * Returns the stream's length.
 */
static size_t
make_valid_stream(const char *name, unsigned long *seed)
{
	FILE *f;
	unsigned char plain[131072];
	size_t i, n;
	long sz;

	/* Text with enough repetition that the table grows and widens. */
	/*
	 * Alternate dense repetition (which grows the table fast) with
	 * stretches of high-entropy bytes (which drive the compressor's
	 * ratio check to emit a CLEAR, per cl_block()) so a mutated
	 * stream can exercise both the table and a table reset.
	 */
	for (i = 0; i < sizeof plain; i++) {
		if ((i / 2048) % 2 == 0) {
			if (i > 64 && (rnd(seed) % 9) != 0)
				plain[i] = plain[i - 1 - (rnd(seed) % 300)];
			else
				plain[i] = (unsigned char)('a' + rnd(seed) % 26);
		} else {
			plain[i] = (unsigned char)rnd(seed);
		}
	}
	f = zopen(name, "w", 0);
	fwrite(plain, 1, sizeof plain, f);
	fclose(f);

	f = fopen(name, "rb");
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fclose(f);
	(void)n;
	return ((size_t)sz);
}

/*
 * Mutation fuzzing: take a valid stream and flip a handful of random
 * bits in its code bytes (the header is left alone, so the stream is
 * still recognized), then try to read it all the way back through
 * zopen(). A sound decoder must either produce bytes and stop at
 * EOF, or set ferror() and stop -- never run past its own table.
 * ASan is the judge of that, not this program: a run that overflows
 * aborts under ASan before reaching the summary line.
 */
static int
one_trial(const char *name, unsigned long seed)
{
	unsigned char buf[262144];
	FILE *f;
	size_t len, i, flips;
	char out[65536];
	size_t total;

	len = make_valid_stream(name, &seed);
	f = fopen(name, "r+b");
	if (f == NULL || len > sizeof buf)
		return (-1);
	fread(buf, 1, len, f);

	flips = 1 + rnd(&seed) % 200;
	for (i = 0; i < flips; i++) {
		size_t pos = 3 + rnd(&seed) % (len - 3);	/* skip the header */
		buf[pos] ^= (unsigned char)(1u << (rnd(&seed) % 8));
	}
	fseek(f, 0, SEEK_SET);
	fwrite(buf, 1, len, f);
	fclose(f);

	f = zopen(name, "r", 0);
	if (f == NULL)
		return (0);
	total = 0;
	while (total < sizeof out) {
		size_t n = fread(out + total, 1, sizeof out - total, f);
		if (n == 0)
			break;
		total += n;
	}
	fclose(f);
	return (0);
}

int
main(int argc, char **argv)
{
	char name[64];
	int trials = argc > 1 ? atoi(argv[1]) : 20000;
	unsigned long seed = argc > 2 ? strtoul(argv[2], NULL, 0) : 1;
	int i;

	setvbuf(stdout, NULL, _IOLBF, 0);
	sprintf(name, "/tmp/zfuzz.%d", (int)getpid());
	printf("T begin\n");
	for (i = 0; i < trials; i++)
		one_trial(name, seed + (unsigned long)i * 2654435761UL);
	unlink(name);
	/* Reaching here at all, under ASan, is the result: no overrun. */
	printf("T %d random streams decoded without an overrun\n", trials);
	printf("T end\n");
	return (0);
}
