/*
 * AI-ONLY NOTE: the host side of building this tree's sbin/newfs/mkfs.c
 * with the host compiler, for build/mkfs-host.c. The tree is not
 * touched; everything the host needs is here.
 *
 * The problem this solves. mkfs.c wants this tree's <ufs/ffs/fs.h>
 * and <ufs/ufs/dinode.h> for the on-disk format, which must come from
 * here so the layout cannot drift. But it also includes <stdio.h> and
 * <sys/param.h>, and taking those from this tree too does not work:
 * the tree's <stdio.h> declares the three standard streams as
 * `&__sF[n]' and glibc has no __sF, and the tree's <sys/types.h>
 * collides with glibc's <time.h> over clock_t and struct timespec.
 *
 * So the host supplies everything standard and this tree supplies
 * only ufs/ and sys/disklabel.h, with the handful of BSD typedefs and
 * constants those need defined below. Each is the value from the file
 * named beside it, and if one of those files changes this header is
 * wrong and build/mkfs-host.c's size assertion is what will say so.
 *
 * Included with -include, so it is seen before anything else.
 */
#ifndef _FS_HOST_H_
#define _FS_HOST_H_

#include <stdint.h>
#include <sys/types.h>

/* The types ufs/ffs/fs.h and ufs/ufs/dinode.h use. */
typedef int32_t		ufs_daddr_t;	/* sys/types.h:62, daddr_t */
typedef int32_t		bsd_daddr_t;
#define daddr_t		bsd_daddr_t

/*
 * Constants, each from the file named. MAXBSIZE is MAXPHYS at
 * sys/param.h:152, and MAXPHYS is 64K at i386/include/param.h:75 --
 * the i386's value, which is the target being written for.
 */
#define MAXPHYS		(64 * 1024)	/* i386/include/param.h:75 */
#ifndef MAXBSIZE
#define MAXBSIZE	MAXPHYS		/* sys/param.h:152 */
#endif
#ifndef MAXFRAG
#define MAXFRAG		8		/* sys/param.h:153 */
#endif
#ifndef NBBY
#define NBBY		8
#endif
#ifndef howmany
#define howmany(x, y)	(((x) + ((y) - 1)) / (y))
#endif
#ifndef roundup
#define roundup(x, y)	((((x) + ((y) - 1)) / (y)) * (y))
#endif

/*
 * The i386's disk-block arithmetic, from i386/include/param.h:72-73
 * and :129-132. The host's <sys/param.h> has none of this, and the
 * values are the target's: DEV_BSIZE 512, DEV_BSHIFT 9.
 */
#define DEV_BSIZE	512		/* i386/include/param.h:72 */
#define DEV_BSHIFT	9		/* i386/include/param.h:73 */
#define btodb(bytes)	((bytes) >> DEV_BSHIFT)
#define dbtob(db)	((db) << DEV_BSHIFT)

/* newfs.c:121, which mkfs-host.c needs and does not get from mkfs.c. */
#define MAXBLKPG(bsize)	((bsize) / sizeof(daddr_t))

#endif /* _FS_HOST_H_ */
