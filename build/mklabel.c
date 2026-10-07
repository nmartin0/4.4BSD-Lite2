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
#include <dkbad.h>

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
	struct dkbad bad;
	char sec[512];
	FILE *f;
	int i;
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
	 * AI-ONLY NOTE: the `secs - 32' is not slack. isa/wd.c:753
	 * reads the bad-sector table at d_secperunit - d_nsectors, so
	 * it lives on the last track; a partition running to the end
	 * of the disk has mkfs write over it, and the next open then
	 * prints `format error in bad-sector file' and fails with
	 * ENXIO. Measured, by doing exactly that.
	 *
	 * d_nsectors is 32 here, so the filesystem ends at 40928 on a
	 * 40960-sector disk and the table has its track to itself.
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

	/*
	 * AI-ONLY NOTE: the bad-sector table, which this driver
	 * requires rather than tolerates. isa/wd.c:765-776 reads the
	 * last track, checks bt_mbz is 0 and bt_flag is 0x4321, and
	 * on anything else prints `format error in bad-sector file'
	 * and fails the open with ENXIO. So an image without one
	 * cannot be mounted, however good its label and filesystem.
	 *
	 * This is what bad144(8) writes. An empty table is a bt_csn
	 * of zero, bt_mbz zero, the magic, and a first entry with
	 * bt_cyl of -1, which is the terminator isa/wd.c:462 scans
	 * for.
	 *
	 * wd.c writes it at d_secperunit - d_nsectors + i for even i
	 * up to 10, so the five copies bad144 makes; one at the first
	 * of those is enough to be found.
	 */
	memset(&bad, 0, sizeof bad);
	bad.bt_csn = 0;
	bad.bt_mbz = 0;
	bad.bt_flag = 0x4321;			/* DKBAD_MAGIC, wd.c:766 */
	bad.bt_bad[0].bt_cyl = 0xffff;		/* -1, the terminator */
	bad.bt_bad[0].bt_trksec = 0xffff;
	memset(sec, 0, sizeof sec);
	memcpy(sec, &bad, sizeof bad);
	for (i = 0; i < 10; i += 2) {
		fseek(f, (long)(secs - l.d_nsectors + i) * 512L, SEEK_SET);
		fwrite(sec, 1, 512, f);
	}
	fclose(f);
	printf("label: %lu sectors, LABELSECTOR %d LABELOFFSET %d, sizeof %zu\n",
	    secs, LABELSECTOR, LABELOFFSET, sizeof l);
	printf("bad144 table at sectors %lu..%lu\n",
	    secs - l.d_nsectors, secs - l.d_nsectors + 8);
	return 0;
}
