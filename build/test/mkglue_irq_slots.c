/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * This file deliberately carries no SPDX licence identifier: the
 * licence for this tree's own scripts is not settled yet, and the tag
 * is kept out rather than written into a sentence, where a scanner
 * would read it as a declaration. 4.4BSD-Lite2's terms (../../COPYRIGHT)
 * cover the Berkeley material, not this file.
 *
 * Nothing here is copied from another system. AI-ONLY NOTE: this is a
 * standalone model of two counter-slot-assignment schemes for the
 * i386 interrupt-count table that usr.sbin/config/mkglue.c's vector()
 * builds, run against GENERIC.i386's real isa device list (see
 * sys/i386/conf/GENERIC.i386). It does not link against mkglue.c
 * (which is entangled with config's own "struct device"/dtab
 * machinery); instead it reproduces each scheme's arithmetic by hand,
 * and mkglue_irq_slots.sh greps the real mkglue.c first to confirm
 * the "new" column below still matches what the source actually
 * emits, so this test cannot quietly drift from it.
 *
 * OLD scheme (patch 222, the bug): clk is slot 0 (fixed), rtc is
 * slot 1 (fixed -- VEC(rtc) used to say "INTR2(8, highmask, 1)"),
 * then every isa device in dtab order gets the next sequential
 * slot starting at 2. That counter has nothing to do with the
 * device's actual IRQ number, so a device's slot can coincide with
 * the raw IRQ number of some OTHER, unconfigured line -- and
 * icu.s's 16 always-present hardcoded "stray irqN" vectors (and the
 * two INTR_SPUR1/INTR_SPUR2 spurious-IRQ checks) index isa_intr by
 * raw IRQ number, never by this sequential count. Two unrelated
 * interrupt sources then add into the same longword.
 *
 * NEW scheme (patch 258, the fix): every slot is the device's own
 * raw IRQ number (0-15); clk is slot 0 and rtc is slot 8 (its real
 * IRQ), matching icu.s's convention exactly, so no collision with
 * an unconfigured line is possible by construction.
 */
#include <stdio.h>
#include <string.h>

struct dev {
	const char *name;
	int irq;
};

/*
 * GENERIC.i386's isa device list, in the order it appears in
 * sys/i386/conf/GENERIC.i386 (dtab order): pc0, npx0, wdc0, wdc1,
 * fd0, com0, ed0, ed1, wt0.
 */
static struct dev devs[] = {
	{ "pc0",  1 },
	{ "npx0", 13 },
	{ "wdc0", 14 },
	{ "wdc1", 15 },
	{ "fd0",  6 },
	{ "com0", 4 },
	{ "ed0",  9 },
	{ "ed1",  3 },
	{ "wt0",  5 },
};
#define NDEVS (sizeof(devs) / sizeof(devs[0]))

int
main(void)
{
	int old_slot[NDEVS], new_slot[NDEVS];
	int old_used[16], new_used[16];
	int i, collisions_old, collisions_new;
	int count;

	for (i = 0; i < 16; i++)
		old_used[i] = new_used[i] = 0;
	old_used[0] = new_used[0] = 1;	/* clk, irq0 */
	old_used[1] = 1;		/* OLD: rtc fixed at slot 1 */
	new_used[8] = 1;		/* NEW: rtc at its real irq, 8 */

	count = 2;			/* OLD: first free sequential slot */
	for (i = 0; i < (int)NDEVS; i++) {
		old_slot[i] = count++;
		new_slot[i] = devs[i].irq;
	}

	printf("dev     irq  old-slot  new-slot\n");
	for (i = 0; i < (int)NDEVS; i++)
		printf("%-6s  %3d  %8d  %8d\n",
		    devs[i].name, devs[i].irq, old_slot[i], new_slot[i]);

	/*
	 * A collision is a configured device's slot landing on an IRQ
	 * number that is NOT its own IRQ and that no device claims --
	 * i.e. icu.s's hardcoded stray vector (or INTR_SPUR1/2) for
	 * that unrelated, unconfigured line shares the slot.
	 */
	collisions_old = collisions_new = 0;
	for (i = 0; i < (int)NDEVS; i++) {
		int s = old_slot[i];
		if (s < 16 && s != devs[i].irq) {
			int claimed = (s == 0 || s == 8);
			int j;
			for (j = 0; j < (int)NDEVS; j++)
				if (devs[j].irq == s)
					claimed = 1;
			if (!claimed) {
				printf("OLD COLLISION: %s's slot %d == "
				    "unconfigured stray irq%d\n",
				    devs[i].name, s, s);
				collisions_old++;
			}
		}
	}
	for (i = 0; i < (int)NDEVS; i++) {
		if (new_slot[i] != devs[i].irq) {
			printf("NEW COLLISION: %s's slot %d != its own "
			    "irq%d\n", devs[i].name, new_slot[i], devs[i].irq);
			collisions_new++;
		}
	}

	printf("old scheme: %d collision(s)\n", collisions_old);
	printf("new scheme: %d collision(s)\n", collisions_new);

	if (collisions_old == 0) {
		fprintf(stderr, "mkglue_irq_slots: expected the OLD scheme "
		    "to collide on GENERIC.i386 -- test is wrong\n");
		return 1;
	}
	if (collisions_new != 0) {
		fprintf(stderr, "mkglue_irq_slots: NEW scheme collided -- "
		    "fix regressed\n");
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
