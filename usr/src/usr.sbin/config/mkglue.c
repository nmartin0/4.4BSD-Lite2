/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)mkglue.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

/*
 * Make the bus adaptor interrupt glue files.
 */
#include <stdio.h>
#include "config.h"
#include "y.tab.h"
#include <ctype.h>

/*
 * Create the UNIBUS interrupt vector glue file.
 */
ubglue()
{
	register FILE *fp, *gp;
	register struct device *dp, *mp;

	fp = fopen(path("ubglue.s"), "w");
	if (fp == 0) {
		perror(path("ubglue.s"));
		exit(1);
	}
	gp = fopen(path("ubvec.s"), "w");
	if (gp == 0) {
		perror(path("ubvec.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_ubavec(fp, id->id,
						    dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_std(fp, gp);
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_intname(fp, id->id,
							dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_ctrs(fp);
	(void) fclose(fp);
	(void) fclose(gp);
}

static int cntcnt = 0;		/* number of interrupt counters allocated */

/*
 * Print a UNIBUS interrupt vector.
 */
dump_ubavec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	(void) sprintf(v, "%s%d", vector, number);
	fprintf(fp, "\t.globl\t_X%s\n\t.align\t2\n_X%s:\n\tpushr\t$0x3f\n",
	    v, v);
	fprintf(fp, "\tincl\t_fltintrcnt+(4*%d)\n", cntcnt++);
	if (strncmp(vector, "dzx", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdzdma\n\n", number);
	else if (strncmp(vector, "dpx", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdpxdma\n\n", number);
	else if (strncmp(vector, "dpr", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdprdma\n\n", number);
	else {
		if (strncmp(vector, "uur", 3) == 0) {
			fprintf(fp, "#ifdef UUDMA\n");
			fprintf(fp, "\tmovl\t$%d,r0\n\tjsb\tuudma\n", number);
			fprintf(fp, "#endif\n");
		}
		fprintf(fp, "\tpushl\t$%d\n", number);
		fprintf(fp, "\tcalls\t$1,_%s\n\tpopr\t$0x3f\n", vector);
		fprintf(fp, "\tincl\t_cnt+V_INTR\n\trei\n\n");
	}
}

/*
 * Create the VERSAbus interrupt vector glue file.
 */
vbglue()
{
	register FILE *fp, *gp;
	register struct device *dp, *mp;

	fp = fopen(path("vbglue.s"), "w");
	if (fp == 0) {
		perror(path("vbglue.s"));
		exit(1);
	}
	gp = fopen(path("vbvec.s"), "w");
	if (gp == 0) {
		perror(path("vbvec.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		struct idlst *id, *id2;

		mp = dp->d_conn;
		if (mp == 0 || mp == (struct device *)-1 ||
		    eq(mp->d_name, "mba"))
			continue;
		for (id = dp->d_vec; id; id = id->id_next)
			for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
				if (id == id2) {
					dump_vbavec(fp, id->id, dp->d_unit);
					break;
				}
				if (eq(id->id, id2->id))
					break;
			}
	}
	dump_std(fp, gp);
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_intname(fp, id->id,
							dp->d_unit);
						break;
					}
					if (eq(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_ctrs(fp);
	(void) fclose(fp);
	(void) fclose(gp);
}

/*
 * Print a VERSAbus interrupt vector
 */
dump_vbavec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	(void) sprintf(v, "%s%d", vector, number);
	fprintf(fp, "SCBVEC(%s):\n", v);
	fprintf(fp, "\tCHECK_SFE(4)\n");
	fprintf(fp, "\tSAVE_FPSTAT(4)\n");
	fprintf(fp, "\tPUSHR\n");
	fprintf(fp, "\tincl\t_fltintrcnt+(4*%d)\n", cntcnt++);
	fprintf(fp, "\tpushl\t$%d\n", number);
	fprintf(fp, "\tcallf\t$8,_%s\n", vector);
	fprintf(fp, "\tincl\t_cnt+V_INTR\n");
	fprintf(fp, "\tPOPR\n");
	fprintf(fp, "\tREST_FPSTAT\n");
	fprintf(fp, "\trei\n\n");
}

/*
 * HP9000/300 interrupts are auto-vectored.
 * Code is hardwired in locore.s
 */
hpglue() {}

static	char *vaxinames[] = {
	"clock", "cnr", "cnx", "tur", "tux",
	"mba0", "mba1", "mba2", "mba3",
	"uba0", "uba1", "uba2", "uba3"
};
static	char *tahoeinames[] = {
	"clock", "cnr", "cnx", "rmtr", "rmtx", "buserr",
};
static	struct stdintrs {
	char	**si_names;	/* list of standard interrupt names */
	int	si_n;		/* number of such names */
} stdintrs[] = {
	{ vaxinames, sizeof (vaxinames) / sizeof (vaxinames[0]) },
	{ tahoeinames, (sizeof (tahoeinames) / sizeof (tahoeinames[0])) }
};
/*
 * Start the interrupt name table with the names
 * of the standard vectors not directly associated
 * with a bus.  Also, dump the defines needed to
 * reference the associated counters into a separate
 * file which is prepended to locore.s.
 */
dump_std(fp, gp)
	register FILE *fp, *gp;
{
	register struct stdintrs *si = &stdintrs[machine-1];
	register char **cpp;
	register int i;

	fprintf(fp, "\n\t.globl\t_intrnames\n");
	fprintf(fp, "\n\t.globl\t_eintrnames\n");
	fprintf(fp, "\t.data\n");
	fprintf(fp, "_intrnames:\n");
	cpp = si->si_names;
	for (i = 0; i < si->si_n; i++) {
		register char *cp, *tp;
		char buf[80];

		cp = *cpp;
		if (cp[0] == 'i' && cp[1] == 'n' && cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		}
		for (tp = buf; *cp; cp++)
			if (islower(*cp))
				*tp++ = toupper(*cp);
			else
				*tp++ = *cp;
		*tp = '\0';
		fprintf(gp, "#define\tI_%s\t%d\n", buf, i*sizeof (long));
		fprintf(fp, "\t.asciz\t\"%s\"\n", *cpp);
		cpp++;
	}
}

dump_intname(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	register char *cp = vector;

	fprintf(fp, "\t.asciz\t\"");
	/*
	 * Skip any "int" or "intr" in the name.
	 */
	while (*cp)
		if (cp[0] == 'i' && cp[1] == 'n' &&  cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		} else {
			putc(*cp, fp);
			cp++;
		}
	fprintf(fp, "%d\"\n", number);
}

/*
 * Reserve space for the interrupt counters.
 */
dump_ctrs(fp)
	register FILE *fp;
{
	struct stdintrs *si = &stdintrs[machine-1];

	fprintf(fp, "_eintrnames:\n");
	fprintf(fp, "\n\t.globl\t_intrcnt\n");
	fprintf(fp, "\n\t.globl\t_eintrcnt\n");
	fprintf(fp, "\t.align 2\n");
	fprintf(fp, "_intrcnt:\n");
	fprintf(fp, "\t.space\t4 * %d\n", si->si_n);
	fprintf(fp, "_fltintrcnt:\n");
	fprintf(fp, "\t.space\t4 * %d\n", cntcnt);
	fprintf(fp, "_eintrcnt:\n\n");
	fprintf(fp, "\t.text\n");
}

/*
 * Create the ISA interrupt vector glue file.
 */
vector() {
	register FILE *fp, *gp;
	register struct device *dp, *mp;
	int count;
	char names[512];	/* NUL-separated, in counter-slot order */
	int namelen;

	fp = fopen(path("vector.s"), "w");
	if (fp == 0) {
		perror(path("vector.s"));
		exit(1);
	}
	fprintf(fp,"\
/*\n\
 * AT/386\n\
 * Interrupt vector routines\n\
 * Generated by config program\n\
 */ \n\
\n\
#include	\"i386/isa/isa.h\"\n\
#include	\"i386/isa/icu.h\"\n\
\n\
#define	VEC(name)	.align 4; .globl _C_LABEL(V/**/name); _C_LABEL(V/**/name):\n\n");

	fprintf(fp,"\
	.globl	_C_LABEL(clkintr)\n\
VEC(clk)\n\
	INTR1(0, _C_LABEL(highmask), 0)\n\
	call	_C_LABEL(clkintr) \n\
	INTREXIT1\n\n\n");

	/*
	 * AI-ONLY NOTE: the statistics clock's vector, next to the
	 * clock's own, which this file hardcodes the same way. IRQ 8
	 * is the RTC's periodic interrupt (i386/isa/clock.c, rtcintr).
	 * FreeBSD 2.0 wired it at run time with register_intr, which
	 * this tree has no equivalent of; here it is a vector of
	 * config's, at highmask as the clock's is.
	 */
	fprintf(fp,"\
	.globl	_C_LABEL(rtcintr)\n\
VEC(rtc)\n\
	INTR2(8, _C_LABEL(highmask), 1)\n\
	call	_C_LABEL(rtcintr) \n\
	INTREXIT2\n\n\n");

	/*
	 * AI-ONLY NOTE: counter slots. Each vector counts into its own
	 * longword of isa_intr (icu.s) through the last argument of its
	 * INTRn; slot 0 is the clock, slot 1 the statistics clock, the
	 * devices follow from 2. The rtc used to be hardcoded as 8, the
	 * same as the ninth device's. The names go out below as
	 * intrnames, in the same order, for vmstat -i; the layout (a
	 * table of names and a table of longs, each with a label past
	 * its end) is the one vmstat has read since 4.3BSD, and NetBSD
	 * 1.0's vector.s keeps the same four labels, as dummies.
	 */
	namelen = 0;
	strcpy(names, "clk");	namelen += 4;
	strcpy(names + namelen, "rtc");	namelen += 4;
	count=1;
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && /* mp != (struct device *)-1 &&*/
		    eq(mp->d_name, "isa")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						if(dp->d_irq == -1) continue;
			fprintf(fp,"\t.globl _C_LABEL(%s), _C_LABEL(%s%dmask)\n\t.data\n",
				id->id, dp->d_name, dp->d_unit);
			fprintf(fp,"_C_LABEL(%s%dmask):\t.long 0\n\t.text\n",
				dp->d_name, dp->d_unit);
			fprintf(fp,"VEC(%s%d)\n\t%sINTR%d(%d, ",
				dp->d_name, dp->d_unit,
				dp->d_irq == 7 ? "INTR_SPUR1\n\t" :
				dp->d_irq == 15 ? "INTR_SPUR2\n\t" : "",
				dp->d_irq / 8 + 1, dp->d_unit);
					if(eq(dp->d_mask,"null"))
		 				fprintf(fp,"_C_LABEL(%s%dmask), ",
							dp->d_name, dp->d_unit);
					else
		  				fprintf(fp,"_C_LABEL(%smask), ",
							dp->d_mask);
		  	fprintf(fp,"%d)\n\tcall\t_C_LABEL(%s)\n\tINTREXIT%d\n\n\n",
				++count, id->id, (dp->d_irq > 7)?2:1); 
			if (count >= 16 ||
			    namelen + strlen(dp->d_name) + 12 > sizeof names) {
				fprintf(stderr,
				    "config: too many interrupt vectors (16)\n");
				exit(1);
			}
			namelen += sprintf(names + namelen, "%s%d",
			    dp->d_name, dp->d_unit) + 1;
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	/*
	 * intrcnt is the front of isa_intr, as many longs as there are
	 * names; vmstat finds the number of counters from the labels.
	 */
	{
		char *cp;

		fprintf(fp, "\t.globl _C_LABEL(intrnames), _C_LABEL(eintrnames)\n");
		fprintf(fp, "\t.globl _C_LABEL(intrcnt), _C_LABEL(eintrcnt)\n");
		fprintf(fp, "\t.data\n_C_LABEL(intrnames):\n");
		for (cp = names; cp < names + namelen; cp += strlen(cp) + 1)
			fprintf(fp, "\t.asciz\t\"%s\"\n", cp);
		fprintf(fp, "_C_LABEL(eintrnames):\n");
		fprintf(fp, "\t.set\t_C_LABEL(intrcnt), _C_LABEL(isa_intr)\n");
		fprintf(fp, "\t.set\t_C_LABEL(eintrcnt), _C_LABEL(isa_intr) + %d\n",
		    (count + 1) * 4);
		fprintf(fp, "\t.text\n");
	}
	(void) fclose(fp);
}
