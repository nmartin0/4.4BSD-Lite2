/*
 * AI-ONLY NOTE: a host driver for this tree's sbin/newfs/mkfs.c, so a
 * filesystem can be written onto an image file from the build host.
 * The tree is not touched: mkfs.c compiles exactly as it stands and
 * everything the host needs is here.
 *
 * Why this exists. newfs(8) builds and links against this tree's own
 * libc, whose syscall stubs are 4.4BSD's, so the resulting binary
 * segfaults inside write() the moment it runs on Linux -- measured,
 * in __sflush from the first fprintf. And newfs.c asks the kernel for
 * the disk label with ioctl(DIOCGDINFO), which Linux has no answer
 * for. mkfs.c itself does no ioctl at all: it is 1265 lines of format
 * arithmetic and writes, and is portable as written.
 *
 * So this replaces newfs.c, not mkfs.c. It supplies the two dozen
 * globals mkfs() reads, opens the image, reads the label that
 * build/mklabel.c wrote at LABELSECTOR rather than asking an ioctl
 * for it, and calls mkfs().
 *
 * MUST BE BUILT -m32, and it refuses to run otherwise. mkfs.c writes
 * struct fs straight to disk, and that structure is fixed-width by
 * Berkeley's design -- 55 int32_t, some smaller ints -- except for
 * one field, fs_time, which is time_t, which is `long' at
 * i386/include/ansi.h:52. Measured against this tree's headers:
 *
 *	64-bit host   sizeof(struct fs) 1392   fs_magic at 1380
 *	32-bit host   sizeof(struct fs) 1380   fs_magic at 1372
 *
 * Twelve bytes out, and the magic number eight bytes from where the
 * kernel looks for it. A 64-bit build would write a superblock that
 * fails the magic check with no indication why, so the size is
 * asserted at startup the way mklabel.c prints its own.
 *
 * Build and use:
 *	cc -m32 -w -I usr/src/sbin/newfs \
 *	   -I $L2_BUILD/root/usr/include -o mkfs-host \
 *	   build/mkfs-host.c usr/src/sbin/newfs/mkfs.c
 *	./mkfs-host disk.img
 *
 * after build/mklabel.c has written the label, since the geometry and
 * the a partition are read back from it.
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>
#include <sys/disklabel.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * The globals mkfs() reads. Values are newfs(8)'s own defaults, from
 * sbin/newfs/newfs.c, except where the label supplies them.
 */
int	mfs = 0;
int	Nflag = 0;
int	Oflag = 0;
int	fssize;
int	ntracks;
int	nsectors;
int	nphyssectors;
int	secpercyl;
int	sectorsize;
int	rpm;
int	interleave;
int	trackskew = -1;
int	headswitch;
int	trackseek;
int	fsize = 0;
int	bsize = 0;
int	cpg = 16;
int	cpgflg = 0;
int	minfree = MINFREE;
int	opt = DEFAULTOPT;
int	density;
int	maxcontig = 0;
int	rotdelay = 4;
int	maxbpg;
int	nrpos = 8;
int	bbsize = BBSIZE;
int	sbsize = SBSIZE;
u_long	memleft;
caddr_t	membase;

/*
 * fsi and fso are mkfs.c's own, at its :119 -- it declares them
 * rather than taking them from newfs.c, so they are not repeated
 * here. mkfs() is passed the descriptors anyway and assigns them.
 */
extern int fsi, fso;

/* newfs.c's, which mkfs.c calls and which does nothing here. */
void
rewritelabel(char *s, int fd, register struct disklabel *lp)
{
}

int
main(int argc, char **argv)
{
	struct disklabel l;
	struct partition *pp;
	char sec[512], buf[65536], tmp[1024];
	int fd, pfd, n;

	if (sizeof(struct fs) != 1380) {
		fprintf(stderr,
		    "mkfs-host: sizeof(struct fs) is %d, want 1380\n",
		    (int)sizeof(struct fs));
		fprintf(stderr, "mkfs-host: build with -m32\n");
		return (1);
	}
	if (argc != 2) {
		fprintf(stderr, "usage: mkfs-host image\n");
		return (1);
	}
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror(argv[1]);
		return (1);
	}
	/* The label, where mklabel.c put it, in place of DIOCGDINFO. */
	if (lseek(fd, (off_t)LABELSECTOR * 512, SEEK_SET) < 0 ||
	    read(fd, sec, sizeof sec) != sizeof sec) {
		perror("read label");
		return (1);
	}
	memcpy(&l, sec + LABELOFFSET, sizeof l);
	if (l.d_magic != DISKMAGIC) {
		fprintf(stderr, "mkfs-host: no disk label on %s\n", argv[1]);
		fprintf(stderr, "mkfs-host: run mklabel on it first\n");
		return (1);
	}

	pp = &l.d_partitions[0];		/* the a partition */
	sectorsize = l.d_secsize;
	nsectors = l.d_nsectors;
	ntracks = l.d_ntracks;
	secpercyl = l.d_secpercyl;
	nphyssectors = l.d_nsectors;
	rpm = l.d_rpm;
	interleave = l.d_interleave;
	headswitch = l.d_headswitch;
	trackseek = l.d_trkseek;
	if (trackskew == -1)
		trackskew = l.d_trackskew;
	fssize = pp->p_size;
	fsize = pp->p_fsize ? pp->p_fsize : 1024;
	bsize = pp->p_frag ? pp->p_fsize * pp->p_frag : 8192;
	density = 4 * fsize;
	maxbpg = MAXBLKPG(bsize);

	/*
	 * AI-ONLY NOTE: mkfs() writes relative to the descriptor it is
	 * given, because newfs(8) is handed a partition device --
	 * /dev/rwd0a -- where sector 0 is the partition's first
	 * sector. Handing it the whole image put the superblock at
	 * absolute sector 16 rather than at the a partition's start,
	 * and the kernel read zeros there and returned EINVAL from
	 * ffs_vfsops.c:432.
	 *
	 * So the filesystem is built in a file of exactly p_size
	 * sectors and then copied into the image at p_offset. That
	 * leaves mkfs.c alone, which is the point, and is explicit
	 * about an offset that newfs(8) gets from the device node.
	 */
	snprintf(tmp, sizeof tmp, "%s.part", argv[1]);
	if ((pfd = open(tmp, O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0) {
		perror(tmp);
		return (1);
	}
	if (ftruncate(pfd, (off_t)fssize * sectorsize) < 0) {
		perror("ftruncate");
		return (1);
	}
	fsi = fso = pfd;
	printf("mkfs-host: %s partition a, %d sectors of %d bytes at %d\n",
	    argv[1], fssize, sectorsize, pp->p_offset);
	printf("mkfs-host: bsize %d fsize %d cpg %d, sizeof(struct fs) %d\n",
	    bsize, fsize, cpg, (int)sizeof(struct fs));
	mkfs(pp, tmp, fsi, fso);

	/* Copy it into place, at the partition's offset. */
	if (lseek(pfd, (off_t)0, SEEK_SET) < 0 ||
	    lseek(fd, (off_t)pp->p_offset * sectorsize, SEEK_SET) < 0) {
		perror("lseek");
		return (1);
	}
	while ((n = read(pfd, buf, sizeof buf)) > 0)
		if (write(fd, buf, n) != n) {
			perror("write");
			return (1);
		}
	close(pfd);
	close(fd);
	unlink(tmp);
	printf("mkfs-host: copied to %s at sector %d\n",
	    argv[1], pp->p_offset);
	return (0);
}
