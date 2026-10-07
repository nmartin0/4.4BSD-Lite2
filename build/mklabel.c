/*
 * AI-ONLY NOTE: a host tool, not part of the kernel or of the tree's
 * own sbin/disklabel. It writes the smallest disk label wdopen will
 * accept, so the kernel can get past `wd0: bad disk label'.
 *
 * It includes the tree's own <sys/disklabel.h> rather than declaring
 * struct disklabel itself, so the layout cannot drift from the
 * kernel's, and prints sizeof(struct disklabel) and the LABELSECTOR
 * and LABELOFFSET it compiled against as a check on the host's
 * padding. `#define i386' before the include is what selects the
 * i386 arm at that header's :50, which puts the label in sector 1 at
 * offset 0 where every other port uses sector 0 at offset 64.
 *
 * Build and use:
 *	cc -I usr/src/sys/sys -o mklabel build/mklabel.c
 *	dd if=/dev/zero of=disk.img bs=1M count=20
 *	./mklabel disk.img 40960
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define i386 1
#include <disklabel.h>

/* dkcksum, from sys/disklabel.h's own definition */
static u_int16_t cksum(struct disklabel *lp)
{
	u_int16_t *start, *end, sum = 0;
	start = (u_int16_t *)lp;
	end = (u_int16_t *)&lp->d_partitions[lp->d_npartitions];
	while (start < end) sum ^= *start++;
	return (sum);
}

int main(int argc, char **argv)
{
	struct disklabel l;
	char sec[512];
	FILE *f;
	unsigned long secs = strtoul(argv[2], 0, 10);

	memset(&l, 0, sizeof l);
	l.d_magic = DISKMAGIC;
	l.d_magic2 = DISKMAGIC;
	l.d_type = DTYPE_ST506;
	strcpy(l.d_typename, "wd");
	strcpy(l.d_packname, "root");
	l.d_secsize = 512;
	l.d_nsectors = 32;
	l.d_ntracks = 8;
	l.d_ncylinders = secs / (32 * 8);
	l.d_secpercyl = 32 * 8;
	l.d_secperunit = secs;
	l.d_rpm = 3600;
	l.d_interleave = 1;
	l.d_npartitions = 3;
	l.d_bbsize = 8192;
	l.d_sbsize = 8192;
	/*
	 * a: the root filesystem, from cylinder 1 and stopping short
	 * of the last track.
	 *
	 * AI-ONLY NOTE: the `- 32' leaves the last track clear.
	 * isa/wd.c reads a bad144(8) table from d_secperunit -
	 * d_nsectors when the label sets D_BADSECT, so that track is
	 * reserved by convention whether or not a table is there.
	 *
	 * d_flags is left at zero here, so no table is written and
	 * wd.c skips the read. An earlier version of this file wrote
	 * one, because wd.c read it unconditionally and failed the
	 * open without it; that was a defect in the driver, fixed by
	 * `i386: wd reads the bad-sector table only when the label
	 * says to', and writing the table was working around it.
	 */
	l.d_partitions[0].p_offset = 256;
	l.d_partitions[0].p_size = secs - 256 - 32;
	l.d_partitions[0].p_fstype = FS_BSDFFS;
	l.d_partitions[0].p_fsize = 1024;
	l.d_partitions[0].p_frag = 8;
	/* c: the whole disk */
	l.d_partitions[2].p_offset = 0;
	l.d_partitions[2].p_size = secs;
	l.d_partitions[2].p_fstype = FS_UNUSED;
	l.d_checksum = 0;
	l.d_checksum = cksum(&l);

	memset(sec, 0, sizeof sec);
	memcpy(sec + LABELOFFSET, &l, sizeof l);
	if (!(f = fopen(argv[1], "r+b"))) { perror(argv[1]); return 1; }
	fseek(f, LABELSECTOR * 512L, SEEK_SET);
	fwrite(sec, 1, 512, f);

	fclose(f);
	printf("label: %lu sectors, LABELSECTOR %d LABELOFFSET %d, sizeof %zu\n",
	    secs, LABELSECTOR, LABELOFFSET, sizeof l);
	return 0;
}
