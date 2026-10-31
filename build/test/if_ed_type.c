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
 * unit test for if_ed.c's SMC8216/SMC8216T type decision
 * (docs/provenance/if_ed-assign.md). QEMU's NIC emulation is ne2000,
 * not the WD790/SMC8216 family this code identifies, so there is no
 * way to reach this path by booting a kernel here; the decision is
 * lifted out verbatim (both guarded `if' statements, copy-pasted from
 * the real source, not retyped) into a small harness that supplies
 * `sc->type' and checks what each statement decides, for both the
 * pre-fix (`=') and fixed (`==') forms.
 */
#include <stdio.h>
#include <string.h>

#define ED_TYPE_SMC8216T	0x2a
#define ED_TYPE_SMC8216C	0x2b

struct sc_s { int type; const char *type_str; };

/*
 * Verbatim from the tree, before the fix (usr/src/sys/i386/isa/
 * if_ed.c, ed_probe_WD80x3()). Both guards: the 16K-memory report
 * and the nested 8K-memory (ED_WD790_RAR_SZ8) report.
 */
static void
decide_pre(struct sc_s *sc)
{
	if (sc->type = ED_TYPE_SMC8216C) {
		sc->type_str = "SMC8216/SMC8216C";
	} else {
		sc->type_str = "SMC8216T";
	}
}

static void
decide_pre_8k(struct sc_s *sc)
{
	if (sc->type = ED_TYPE_SMC8216C) {
		sc->type_str = "SMC8416C/SMC8416BT";
	} else {
		sc->type_str = "SMC8416T";
	}
}

/* Verbatim from the tree, after the fix. */
static void
decide_post(struct sc_s *sc)
{
	if (sc->type == ED_TYPE_SMC8216C) {
		sc->type_str = "SMC8216/SMC8216C";
	} else {
		sc->type_str = "SMC8216T";
	}
}

static void
decide_post_8k(struct sc_s *sc)
{
	if (sc->type == ED_TYPE_SMC8216C) {
		sc->type_str = "SMC8416C/SMC8416BT";
	} else {
		sc->type_str = "SMC8416T";
	}
}

static int fails;

static void
check(const char *who, void (*decide)(struct sc_s *), int type_in,
    int want_type_out, const char *want_str)
{
	struct sc_s sc;

	sc.type = type_in;
	sc.type_str = "?";
	decide(&sc);
	printf("T %s(type=0x%02x): type_str=\"%s\" type_out=0x%02x\n",
	    who, type_in, sc.type_str, sc.type);
	if (sc.type != want_type_out || strcmp(sc.type_str, want_str) != 0) {
		printf("T %s MISMATCH: wanted type_str=\"%s\" type_out=0x%02x\n",
		    who, want_str, want_type_out);
		fails++;
	}
}

int
main(void)
{
	printf("T begin\n");

	/* The actual board type is read from hardware into sc->type
	 * beforehand; this switch case is reached for either 8216C or
	 * 8216T, and the two boards are told apart afterward. */

	/* pre-fix: `=' always takes the true branch and overwrites
	 * sc->type, so an SMC8216T board (0x2a) is misreported *and*
	 * silently turned into an SMC8216C (0x2b) in sc->type. */
	check("pre-fix, SMC8216T board ", decide_pre, ED_TYPE_SMC8216T,
	    ED_TYPE_SMC8216C, "SMC8216/SMC8216C");
	check("pre-fix, SMC8216C board ", decide_pre, ED_TYPE_SMC8216C,
	    ED_TYPE_SMC8216C, "SMC8216/SMC8216C");

	/* fixed: `==' leaves sc->type alone and reports each correctly. */
	check("fixed,   SMC8216T board ", decide_post, ED_TYPE_SMC8216T,
	    ED_TYPE_SMC8216T, "SMC8216T");
	check("fixed,   SMC8216C board ", decide_post, ED_TYPE_SMC8216C,
	    ED_TYPE_SMC8216C, "SMC8216/SMC8216C");

	/* the second guard (8K-memory variant): same pattern, own strings. */
	check("pre-fix, SMC8416T board ", decide_pre_8k, ED_TYPE_SMC8216T,
	    ED_TYPE_SMC8216C, "SMC8416C/SMC8416BT");
	check("pre-fix, SMC8416C board ", decide_pre_8k, ED_TYPE_SMC8216C,
	    ED_TYPE_SMC8216C, "SMC8416C/SMC8416BT");
	check("fixed,   SMC8416T board ", decide_post_8k, ED_TYPE_SMC8216T,
	    ED_TYPE_SMC8216T, "SMC8416T");
	check("fixed,   SMC8416C board ", decide_post_8k, ED_TYPE_SMC8216C,
	    ED_TYPE_SMC8216C, "SMC8416C/SMC8416BT");

	printf("T %s\n", fails ? "FAIL" : "all checks passed");
	printf("T end\n");
	return (fails != 0);
}
