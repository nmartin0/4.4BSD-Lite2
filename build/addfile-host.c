/*
 * AI-ONLY NOTE: a host tool that puts /sbin/init into the filesystem
 * build/mkfs-host.c has made, so the kernel has something to exec.
 * The tree is not touched: this links against this tree's own
 * sbin/newfs/mkfs.c and drives the helpers that file already has.
 *
 * Why it exists. The build host has no UFS support -- /proc/filesystems
 * lists none -- and no tool that writes this format, so there is no
 * way to put a file into the image once newfs has run. 4.4BSD solved
 * this with a distribution tape. This project has a build host and a
 * disk image, and nothing in between.
 *
 * It is not a general tool and does not pretend to be. It adds one
 * directory and one file, using exactly the sequence mkfs.c's own
 * fsinit() uses for `/' and `lost+found':
 *
 *	node.di_mode  = IFDIR or IFREG
 *	node.di_size  = the size
 *	node.di_db[n] = alloc(size, mode)
 *	wtfs(fsbtodb(&sblock, node.di_db[n]), size, data)
 *	iput(&node, ino)
 *
 * alloc() and iput() do the bookkeeping, which is why no bitmap
 * arithmetic appears here: alloc marks the blocks used in the
 * cylinder group and decrements cs_nbfree, and iput sets the inode
 * bit with setbit(cg_inosused(&acg), ino), decrements cs_nifree in
 * the group, the superblock and fscs[0], and writes the group back.
 * Both are mkfs.c's, both non-static.
 *
 * The offset. mkfs.c's wtfs and rdfs seek from the descriptor's
 * origin, because newfs(8) is handed a partition device where sector
 * 0 is the partition's first sector. So the partition is copied out
 * to a file of its own, modified there, and copied back -- the same
 * arrangement build/mkfs-host.c uses, and for the same reason.
 *
 * MUST BE BUILT -m32. struct fs and struct dinode go to disk as
 * written and time_t is `long', four bytes on the i386 target and
 * eight on an x86-64 host; the size is asserted at startup.
 *
 * Build and use, after mklabel and mkfs-host:
 *
 *	cc -m32 -w -include build/fs-host.h -I usr/src/sbin/newfs \
 *	   -I <private include dir> -o addfile-host \
 *	   build/addfile-host.c usr/src/sbin/newfs/mkfs.c
 *	./addfile-host disk.img /path/to/init
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>
#include <sys/disklabel.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#define	say(...)	do { printf(__VA_ARGS__); fflush(stdout); } while (0)

/*
 * mkfs.c's, which this tool drives rather than duplicates. sblock and
 * acg are macros over these unions there, so the unions are what is
 * declared.
 */
extern union {
	struct fs fs;
	char pad[SBSIZE];
} fsun;
#define	sblock	fsun.fs

extern struct dinode node;
extern char buf[MAXBSIZE];
extern int fsi, fso;
extern struct csum *fscs;
daddr_t alloc();
void iput();
void wtfs();
void rdfs();

/* The globals mkfs.c reads. Only those its helpers touch matter here. */
int	mfs = 0, Nflag = 0, Oflag = 0;
int	fssize, ntracks, nsectors, nphyssectors, secpercyl, sectorsize;
int	rpm, interleave, trackskew = -1, headswitch, trackseek;
int	fsize = 0, bsize = 0, cpg = 16, cpgflg = 0;
int	minfree = MINFREE, opt = DEFAULTOPT, density, maxcontig = 0;
int	rotdelay = 4, maxbpg, nrpos = 8, bbsize = BBSIZE, sbsize = SBSIZE;
u_long	memleft;
caddr_t	membase;

void
rewritelabel(char *s, int fd, struct disklabel *lp)
{
}

/*
 * The next free inodes. mkfs.c creates only ROOTINO, which is 2 at
 * ufs/ufs/dinode.h:47: its lost+found is inside `#ifdef LOSTDIR' and
 * sbin/newfs/Makefile does not define LOSTDIR, so LOSTFOUNDINO is
 * never referenced in a compiled branch and inode 3 is free.
 *
 * There is no allocator here because there is no need for one. The
 * filesystem is seconds old, nothing else has written to it, and this
 * tool adds exactly two inodes.
 */
static ino_t nextino = 3;	/* ROOTINO is 2; see above */

int
main(int argc, char **argv)
{
	struct disklabel l;
	struct partition *pp;
	struct direct *dp;
	struct stat st;
	char sec[512], tmp[1024], *data, *cp;
	off_t partoff;
	ufs_daddr_t *indir, blkno;
	char *dirname, *filepath, *entname;
	char devbuf[64];
	int isdev, devmaj, devmin;
	ino_t dirino, fileino;
	int arg;
	int fd, pfd, ifd, n, i, nblk, bsz;
	time_t utime;

	if (sizeof(struct fs) != 1380) {
		fprintf(stderr, "addfile-host: sizeof(struct fs) is %d, "
		    "want 1380; build with -m32\n", (int)sizeof(struct fs));
		return (1);
	}
	if (argc < 4 || (argc & 1) != 0) {	/* program + image + pairs */
		fprintf(stderr, "usage: addfile-host image dir file "
		    "[dir file ...]\n");
		fprintf(stderr, "  a file may be a path, or a character "
		    "device written as name:c:major:minor\n");
		fprintf(stderr, "  e.g. addfile-host disk.img sbin .../init "
		    "bin .../sh dev console:c:0:0\n");
		return (1);
	}
	time(&utime);

	/* The label, for the partition's offset and size. */
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror(argv[1]);
		return (1);
	}
	if (lseek(fd, (off_t)LABELSECTOR * 512, SEEK_SET) < 0 ||
	    read(fd, sec, sizeof sec) != sizeof sec) {
		perror("read label");
		return (1);
	}
	memcpy(&l, sec + LABELOFFSET, sizeof l);
	if (l.d_magic != DISKMAGIC) {
		fprintf(stderr, "addfile-host: no label on %s\n", argv[1]);
		return (1);
	}
	pp = &l.d_partitions[0];
	sectorsize = l.d_secsize;
	partoff = (off_t)pp->p_offset * sectorsize;

	/* Copy the partition out, so wtfs's offsets are right. */
	snprintf(tmp, sizeof tmp, "%s.part", argv[1]);
	if ((pfd = open(tmp, O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0) {
		perror(tmp);
		return (1);
	}
	if (lseek(fd, partoff, SEEK_SET) < 0) {
		perror("lseek");
		return (1);
	}
	for (n = pp->p_size * sectorsize; n > 0; n -= i) {
		i = read(fd, sec, sizeof sec);
		if (i <= 0)
			break;
		if (write(pfd, sec, i) != i) {
			perror("write");
			return (1);
		}
	}
	fsi = fso = pfd;

	/* The superblock mkfs-host wrote. */
	rdfs((int)(SBOFF / sectorsize), SBSIZE, (char *)&sblock);
	if (sblock.fs_magic != FS_MAGIC) {
		fprintf(stderr, "addfile-host: no filesystem on %s "
		    "partition a; run mkfs-host first\n", argv[1]);
		return (1);
	}
	bsz = sblock.fs_bsize;
	fscs = (struct csum *)calloc(1, sblock.fs_cssize);
	rdfs(fsbtodb(&sblock, sblock.fs_csaddr), sblock.fs_cssize,
	    (char *)fscs);
	say("addfile-host: %s partition a, fs_magic %#x, bsize %d\n",
	    argv[1], sblock.fs_magic, bsz);

	/*
	 * AI-ONLY NOTE: one pass per `dir file' pair on the command
	 * line. Each makes a directory under the root and puts one
	 * file in it, taking the next two free inodes. There is no
	 * allocator because none is needed -- see the note on nextino
	 * above -- and no attempt to reuse a directory that already
	 * exists, because this tool populates an empty filesystem and
	 * nothing else writes to it.
	 */
	for (arg = 2; arg < argc; arg += 2) {
	dirname = argv[arg];
	filepath = argv[arg + 1];
	dirino = nextino++;
	fileino = nextino++;

	/*
	 * AI-ONLY NOTE: a character device node, written
	 * `name:c:major:minor' where a path would go.
	 *
	 * init's first act after forking is setctty(_PATH_CONSOLE),
	 * which opens /dev/console and _exits if it cannot, so an image
	 * with no /dev gets no shell however good the kernel is.
	 *
	 * The numbers are this tree's own. etc/etc.i386/MAKEDEV's `std'
	 * case is
	 *
	 *	mknod console	c 0 0
	 *	mknod tty	c 1 0	; chmod 666
	 *	mknod mem	c 2 0	; chmod 640
	 *	mknod kmem	c 2 1	; chmod 640
	 *	mknod null	c 2 2	; chmod 666
	 *	mknod zero	c 2 12	; chmod 666
	 *	mknod drum	c 4 0	; chmod 640
	 *
	 * which agrees with i386/i386/conf.c's cdevsw, whose slot 0 is
	 * cdev_cn_init(1,cn), the console.
	 *
	 * A device inode carries the device number where a regular one
	 * carries its first block: <ufs/ufs/dinode.h>:102 is
	 * `#define di_rdev di_db[0]', and <sys/types.h>:92 encodes it
	 * as `((x) << 8) | (y)'. di_size and di_blocks stay zero and
	 * nothing is allocated, so the only difference from a regular
	 * file below is that the block loop does not run.
	 */
	isdev = 0;
	if (strchr(filepath, ':') != NULL) {
		char *q;

		strncpy(devbuf, filepath, sizeof devbuf - 1);
		devbuf[sizeof devbuf - 1] = '\0';
		q = strchr(devbuf, ':');
		*q++ = '\0';
		if (*q != 'c' || q[1] != ':' ||
		    (q = strchr(q + 2, ':')) == NULL) {
			fprintf(stderr, "addfile-host: %s: want "
			    "name:c:major:minor, character devices only\n",
			    filepath);
			return (1);
		}
		devmaj = atoi(strchr(devbuf, '\0') + 3);
		devmin = atoi(q + 1);
		isdev = 1;
		entname = devbuf;
		st.st_size = 0;
	} else {
		entname = basename(filepath);
	}

	/* The file to add, read whole; init is small. */
	if (!isdev && stat(filepath, &st) < 0) {
		perror(filepath);
		return (1);
	}
	if (!isdev)
	if ((data = malloc((size_t)st.st_size)) == NULL) {
		perror("malloc");
		return (1);
	}
	/*
	 * AI-ONLY NOTE: ifd, not pfd. pfd is the partition this tool
	 * writes through, opened once above and used by mkfs.c's wtfs
	 * for the whole run; reading the input file through it and
	 * closing it took the filesystem away after the first pair,
	 * and the writes that followed went nowhere.
	 */
	if (!isdev)
	if ((ifd = open(filepath, O_RDONLY)) < 0 ||
	    read(ifd, data, (size_t)st.st_size) != st.st_size) {
		perror(filepath);
		return (1);
	}
	if (!isdev)
		close(ifd);


	/*
	 * The directory: one block holding `.', `..' and the file.
	 */
	memset(buf, 0, (size_t)bsz);
	dp = (struct direct *)buf;
	dp->d_ino = dirino;
	dp->d_type = DT_DIR;
	dp->d_namlen = 1;
	strcpy(dp->d_name, ".");
	dp->d_reclen = DIRSIZ(0, dp);
	cp = (char *)dp + dp->d_reclen;
	dp = (struct direct *)cp;
	dp->d_ino = ROOTINO;
	dp->d_type = DT_DIR;
	dp->d_namlen = 2;
	strcpy(dp->d_name, "..");
	dp->d_reclen = DIRSIZ(0, dp);
	cp = (char *)dp + dp->d_reclen;
	dp = (struct direct *)cp;
	dp->d_ino = fileino;
	dp->d_type = isdev ? DT_CHR : DT_REG;
	dp->d_namlen = strlen(entname);
	strcpy(dp->d_name, entname);
	dp->d_reclen = bsz - (cp - buf);

	memset((char *)&node, 0, sizeof node);
	node.di_atime = node.di_mtime = node.di_ctime = utime;
	node.di_mode = IFDIR | 0755;
	node.di_nlink = 2;
	node.di_size = bsz;
	node.di_db[0] = alloc(bsz, node.di_mode);
	node.di_blocks = btodb(fragroundup(&sblock, bsz));
	wtfs(fsbtodb(&sblock, node.di_db[0]), bsz, buf);
	iput(&node, dirino);
	say("addfile-host: /%s, inode %d, block %d\n",
	    dirname, (int)dirino, node.di_db[0]);

	/*
	 * The file. Twelve direct blocks and one single indirect, which
	 * is as far as this goes -- NDADDR is 12 at ufs/ufs/dinode.h:65
	 * and NINDIR(&sblock) block numbers fit in the indirect, so the
	 * ceiling is (12 + NINDIR) blocks. init is 21 blocks of 8192,
	 * so it needs the indirect and nothing beyond it. The check
	 * below refuses anything larger rather than writing a wrong
	 * inode, which is what the first version of this tool would
	 * have done.
	 */
	nblk = howmany(st.st_size, bsz);
	if (nblk > NDADDR + NINDIR(&sblock)) {
		fprintf(stderr, "addfile-host: %s needs %d blocks, and this "
		    "writes at most %d\n", filepath, nblk,
		    NDADDR + NINDIR(&sblock));
		return (1);
	}
	memset((char *)&node, 0, sizeof node);
	node.di_atime = node.di_mtime = node.di_ctime = utime;
	node.di_nlink = 1;
	node.di_blocks = 0;
	if (isdev) {
		node.di_mode = IFCHR | 0666;
		node.di_size = 0;
		node.di_rdev = (devmaj << 8) | devmin;
		iput(&node, fileino);
		say("addfile-host: /%s/%s, inode %d, char %d/%d\n",
		    dirname, entname, (int)fileino, devmaj, devmin);
		goto rootent;
	}
	node.di_mode = IFREG | 0755;
	node.di_size = st.st_size;
	indir = NULL;
	if (nblk > NDADDR) {
		if ((indir = (ufs_daddr_t *)calloc(1, (size_t)bsz)) == NULL) {
			perror("calloc");
			return (1);
		}
		node.di_ib[0] = alloc(bsz, node.di_mode);
		node.di_blocks += btodb(fragroundup(&sblock, bsz));
	}
	for (i = 0; i < nblk; i++) {
		n = (i == nblk - 1) ? (int)st.st_size - i * bsz : bsz;
		memset(buf, 0, (size_t)bsz);
		memcpy(buf, data + i * bsz, (size_t)n);
		blkno = alloc(bsz, node.di_mode);
		wtfs(fsbtodb(&sblock, blkno), bsz, buf);
		node.di_blocks += btodb(fragroundup(&sblock, bsz));
		if (i < NDADDR)
			node.di_db[i] = blkno;
		else
			indir[i - NDADDR] = blkno;
	}
	if (indir) {
		wtfs(fsbtodb(&sblock, node.di_ib[0]), bsz, (char *)indir);
		free((char *)indir);
	}
	iput(&node, fileino);
	say("addfile-host: /%s/%s, inode %d, %d blocks\n",
	    dirname, basename(filepath), (int)fileino, nblk);

rootent:
	/*
	 * Add the new directory to the root directory. mkfs wrote it as one block
	 * whose last entry's d_reclen runs to the end, so the entry is
	 * split the way ufs_direnter does.
	 */
	{
		struct dinode ibuf[MAXBSIZE / sizeof (struct dinode)];
		struct dinode *dip;
		daddr_t rootblk;
		int rootsz;

		/*
		 * The root inode, addressed the way mkfs.c's own iput
		 * does it: ino_to_fsba for the block and ino_to_fsbo
		 * for the index within it. An earlier version of this
		 * used cgimin and a modulo, which is not the same thing
		 * and read a block of zeros -- the guard below caught
		 * it, reporting a d_reclen of 0.
		 */
		rdfs(fsbtodb(&sblock, ino_to_fsba(&sblock, ROOTINO)),
		    sblock.fs_bsize, (char *)ibuf);
		dip = &ibuf[ino_to_fsbo(&sblock, ROOTINO)];
		rootblk = dip->di_db[0];
		rootsz = dip->di_size;
		node = *dip;
		/*
		 * The root directory is di_size bytes, not fs_bsize:
		 * mkfs builds directories in DIRBLKSIZ chunks and the
		 * root is one of them, 512 bytes. Walking to fs_bsize
		 * ran off the end into zeros and found a d_reclen of 0,
		 * which the guard below reported.
		 */
		rdfs(fsbtodb(&sblock, rootblk), bsz, buf);
		dp = (struct direct *)buf;
		for (;;) {
			if (dp->d_reclen < DIRSIZ(0, dp) ||
			    dp->d_reclen > rootsz) {
				fprintf(stderr, "addfile-host: root directory "
				    "entry has d_reclen %d, which is not a "
				    "usable length\n", dp->d_reclen);
				return (1);
			}
			cp = (char *)dp + dp->d_reclen;
			if (cp >= buf + rootsz)
				break;
			dp = (struct direct *)cp;
		}
		i = DIRSIZ(0, dp);
		if (dp->d_reclen - i < sizeof (struct direct)) {
			fprintf(stderr, "addfile-host: no room in the root "
			    "directory block\n");
			return (1);
		}
		n = dp->d_reclen - i;
		dp->d_reclen = i;
		dp = (struct direct *)((char *)dp + i);
		dp->d_ino = dirino;
		dp->d_type = DT_DIR;
		dp->d_namlen = strlen(dirname);
		strcpy(dp->d_name, dirname);
		dp->d_reclen = n;
		wtfs(fsbtodb(&sblock, rootblk), bsz, buf);
		node.di_nlink++;		/* for the new directory's `..' */
		iput(&node, ROOTINO);
	}
	free(data);
	}

	/* Write the summary back, as mkfs does at its end. */
	wtfs(fsbtodb(&sblock, sblock.fs_csaddr), sblock.fs_cssize,
	    (char *)fscs);
	wtfs((int)(SBOFF / sectorsize), SBSIZE, (char *)&sblock);

	/* And the partition back into the image. */
	if (lseek(pfd, (off_t)0, SEEK_SET) < 0 ||
	    lseek(fd, partoff, SEEK_SET) < 0) {
		perror("lseek");
		return (1);
	}
	while ((n = read(pfd, sec, sizeof sec)) > 0)
		if (write(fd, sec, n) != n) {
			perror("write");
			return (1);
		}
	close(pfd);
	close(fd);
	unlink(tmp);
	say("addfile-host: written back to %s at sector %d\n",
	    argv[1], pp->p_offset);
	return (0);
}
