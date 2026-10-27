/*
 * Copyright (c) 1998 Robert Nordier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */

/*
 * $FreeBSD$
 */

/*
 * AI-ONLY NOTE: FreeBSD 4.1's boot2.c, shrink-wrapped for this tree.
 * docs/provenance/imports.md (kind D) has why this release.
 *
 * Cut, because this tree has no use for it:
 *   - a.out kernels. This tree's kernel is ELF; boot2 reads its
 *     PT_LOAD segments, one or two -- `ld -n' makes one, and 4.1
 *     assumed two.
 *   - struct bootinfo, the BIOS memory and disk-geometry probes that
 *     filled it, and the symbol table loaded after the kernel for it.
 *     This kernel reads none of them; it sizes memory itself, and
 *     ps and kvm_mkdb read /vmunix. ddb's symbols are loaded instead
 *     as NetBSD's boot loads them, below (load).
 *   - the third stage, /boot/loader, tried before the kernel.
 *   - DOS slices. Lite2's wd driver reads its label from sector 1 of
 *     the disk and knows no partition table, so boot2 does the same.
 *     boot1 still finds boot2 through the one-entry table at the end
 *     of its sector, as on a FreeBSD disk used whole.
 *   - the ad and da disk types and the CD flag; the -c, -C, -g and -v
 *     options, which set boot flags this kernel does not have. Of
 *     <sys/reboot.h>'s, -a, -s, -r and -d reach the kernel; -h, -D and
 *     -P choose boot2's own console and are not passed on. A raw
 *     partition without a label is no longer booted.
 *
 * Changed:
 *   - /vmunix for /kernel, and the banner.
 *   - MAKEBOOTDEV with wd 0 and fd 2, this tree's block majors, which
 *     i386/i386/autoconf.c's setroot decodes, and no slice; a device
 *     is named wd(0,a), with no slice field.
 *   - this tree's headers: <sys/exec_elf.h>'s names, IS_ELF and
 *     roundup2 spelled out, uint32_t and uint8_t from u_int32_t and
 *     u_int8_t, and dinode.h before fs.h, which needs it here.
 */

#include <sys/param.h>
#include <sys/reboot.h>
#include <sys/disklabel.h>
#include <sys/dirent.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <stdarg.h>

#include <sys/exec_elf.h>

#include <btxv86.h>

#include "lib.h"

#define IS_ELF(eh)	((eh).e_ident[0] == 0x7f && (eh).e_ident[1] == 'E' && \
			 (eh).e_ident[2] == 'L' && (eh).e_ident[3] == 'F')
#define roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))

#define RBX_ASKNAME	0x0	/* -a */
#define RBX_SINGLE	0x1	/* -s */
#define RBX_DFLTROOT	0x5	/* -r */
#define RBX_KDB 	0x6	/* -d */
#define RBX_SERIAL	0xc	/* -h */
#define RBX_DUAL	0x1d	/* -D */
#define RBX_PROBEKBD	0x1e	/* -P */

#define RBX_MASK	(1 << RBX_ASKNAME | 1 << RBX_SINGLE | \
			 1 << RBX_DFLTROOT | 1 << RBX_KDB)

#define PATH_CONFIG	"/boot.config"
#define PATH_KERNEL	"/vmunix"

#define ARGS		0x900
#define NOPT		7
#define BSIZEMAX	8192
#define NDEV		2
#define V86_CY(x)	((x) & 1)
#define V86_ZR(x)	((x) & 0x40)

#define DRV_HARD	0x80
#define DRV_MASK	0x7f

#define TYPE_WD		0
#define TYPE_FD		1

extern uint32_t _end;

static const char optstr[NOPT] = "DhadPrs";
static const unsigned char flags[NOPT] = {
    RBX_DUAL,
    RBX_SERIAL,
    RBX_ASKNAME,
    RBX_KDB,
    RBX_PROBEKBD,
    RBX_DFLTROOT,
    RBX_SINGLE
};

static const char *const dev_nm[] = {"wd", "fd"};
static const unsigned dev_maj[] = {0, 2};

static struct dsk {
    unsigned drive;
    unsigned type;
    unsigned unit;
    unsigned part;
    unsigned start;
    int meta;
} dsk;
static char cmd[512];
static char kname[1024];
static uint32_t opts;
static int ls;
static uint32_t fs_off;
static uint8_t ioctrl = 0x1;

void exit(int);
static void load(const char *);
static int parse(char *);
static ino_t lookup(const char *);
static int xfsread(ino_t, void *, size_t);
static ssize_t fsread(ino_t, void *, size_t);
static int dskread(void *, unsigned, unsigned);
static int printf(const char *,...);
static int putchar(int);
static void *memcpy(void *, const void *, size_t);
static void *malloc(size_t);
static int drvread(void *, unsigned, unsigned);
static int keyhit(unsigned);
static int xputc(int);
static int xgetc(int);
static int getc(int);

static inline void
readfile(const char *fname, void *buf, size_t size)
{
    ino_t ino;

    if ((ino = lookup(fname)))
	fsread(ino, buf, size);
}

static inline int
strcmp(const char *s1, const char *s2)
{
    for (; *s1 == *s2 && *s1; s1++, s2++);
    return (u_char)*s1 - (u_char)*s2;
}

static inline int
fsfind(const char *name, ino_t * ino)
{
    char buf[DEV_BSIZE];
    struct dirent *d;
    char *s;
    ssize_t n;

    fs_off = 0;
    while ((n = fsread(*ino, buf, DEV_BSIZE)) > 0)
	for (s = buf; s < buf + DEV_BSIZE;) {
	    d = (void *)s;
	    if (ls)
		printf("%s ", d->d_name);
	    else if (!strcmp(name, d->d_name)) {
		*ino = d->d_fileno;
		return d->d_type;
	    }
	    s += d->d_reclen;
	}
    if (n != -1 && ls)
	putchar('\n');
    return 0;
}

static inline int
getchar(void)
{
    int c;

    c = xgetc(0);
    if (c == '\r')
	c = '\n';
    return c;
}

static inline void
getstr(char *str, int size)
{
    char *s;
    int c;

    s = str;
    do {
	switch (c = getchar()) {
	case 0:
	    break;
	case '\b':
	case '\177':
	    if (s > str) {
		s--;
		putchar('\b');
		putchar(' ');
	    } else
		c = 0;
	    break;
	case '\n':
	    *s = 0;
	    break;
	default:
	    if (s - str < size - 1)
		*s++ = c;
	}
	if (c)
	    putchar(c);
    } while (c != '\n');
}

static inline void
putc(int c)
{
    v86.addr = 0x10;
    v86.eax = 0xe00 | (c & 0xff);
    v86.ebx = 0x7;
    v86int();
}

int
main(void)
{
    int autoboot;

    v86.ctl = V86_FLAGS;
    dsk.drive = *(uint8_t *)PTOV(ARGS);
    dsk.type = dsk.drive & DRV_HARD ? TYPE_WD : TYPE_FD;
    dsk.unit = dsk.drive & DRV_MASK;
    autoboot = 1;
    readfile(PATH_CONFIG, cmd, sizeof(cmd));
    if (*cmd) {
	printf("%s: %s", PATH_CONFIG, cmd);
	if (parse(cmd))
	    autoboot = 0;
	*cmd = 0;
    }
    if (autoboot && !*kname)
	memcpy(kname, PATH_KERNEL, sizeof(PATH_KERNEL));
    for (;;) {
	printf(" \n>> 4.4BSD/i386 BOOT\n"
	       "Default: %u:%s(%u,%c)%s\n"
	       "boot: ",
	       dsk.drive & DRV_MASK, dev_nm[dsk.type], dsk.unit,
	       'a' + dsk.part, kname);
	if (ioctrl & 0x2)
	    sio_flush();
	if (!autoboot || keyhit(0x5a))
	    getstr(cmd, sizeof(cmd));
	else
	    putchar('\n');
	autoboot = 0;
	if (parse(cmd))
	    putchar('\a'); 
	else
	    load(kname);
    }
}

/* XXX - Needed for btxld to link the boot2 binary; do not remove. */
void
exit(int x)
{
}

static void
load(const char *fname)
{
    union {
	Elf32_Ehdr eh;
    } hdr;
    Elf32_Phdr ep[2];
    Elf32_Shdr *shp;
    Elf32_Ehdr *elf;
    caddr_t p, ssym;
    ino_t ino;
    uint32_t addr, kend, off, esym;
    int i, j;

    if (!(ino = lookup(fname))) {
	if (!ls)
	    printf("No %s\n", fname);
	return;
    }
    if (xfsread(ino, &hdr, sizeof(hdr)))
	return;
    if (!IS_ELF(hdr.eh)) {
	printf("Invalid %s\n", "format");
	return;
    }
    fs_off = hdr.eh.e_phoff;
    for (j = i = 0; i < hdr.eh.e_phnum && j < 2; i++) {
	if (xfsread(ino, ep + j, sizeof(ep[0])))
	    return;
	if (ep[j].p_type == Elf32_pt_load)
	    j++;
    }
    for (i = 0; i < j; i++) {
	p = PTOV(ep[i].p_paddr & 0xffffff);
	fs_off = ep[i].p_offset;
	if (xfsread(ino, p, ep[i].p_filesz))
	    return;
    }
    /*
     * AI-ONLY NOTE: added (2026-10-26) -- with -d, the kernel's symbols
     * for ddb: its ELF header, its section headers, and its symbol and
     * string tables, loaded after the kernel's bss, the copied header
     * made to describe them where they lie, and the end passed to the
     * kernel as the fifth word, esym, which locore.s takes; 0 without
     * -d. This is NetBSD 1.3's alpha boot, stand/common/loadfile.c
     * elf_exec(), from "Copy the ELF and section headers" to "Frob the
     * copied ELF header", the layout NetBSD 1.3's ddb/db_elf.c reads;
     * written with boot2's fs_off and xfsread for its lseek and read,
     * and the kernel's end, its ffp_save, taken from the segments
     * loaded above. Its progress printing is left out, as boot2
     * prints none. Loading only for -d is Berkeley's, the condition
     * vax/stand/boot.c and tahoe/stand/boot.c load symbols under,
     * `howto & RB_KDB'.
     */
    esym = 0;
    if (opts & 1 << RBX_KDB) {
	kend = 0;
	for (i = 0; i < j; i++)
	    if (kend < (ep[i].p_paddr & 0xffffff) + ep[i].p_memsz)
		kend = (ep[i].p_paddr & 0xffffff) + ep[i].p_memsz;
	/*
	 * Copy the ELF and section headers.
	 */
	p = ssym = PTOV(roundup(kend, sizeof(long)));
	memcpy(p, &hdr.eh, sizeof(Elf32_Ehdr));
	p += sizeof(Elf32_Ehdr);
	fs_off = hdr.eh.e_shoff;
	if (xfsread(ino, p, hdr.eh.e_shnum * sizeof(Elf32_Shdr)))
	    return;
	shp = (Elf32_Shdr *)p;
	p += roundup((hdr.eh.e_shnum * sizeof(Elf32_Shdr)), sizeof(long));

	/*
	 * Now load the symbol sections themselves.  Make sure the
	 * sections are aligned.
	 */
	off = roundup((sizeof(Elf32_Ehdr) + (hdr.eh.e_shnum *
	    sizeof(Elf32_Shdr))), sizeof(long));
	for (i = 0; i < hdr.eh.e_shnum; i++) {
	    if (shp[i].sh_type == Elf_sht_symtab ||
		shp[i].sh_type == Elf_sht_strtab) {
		fs_off = shp[i].sh_offset;
		if (xfsread(ino, p, shp[i].sh_size))
		    return;
		p += roundup(shp[i].sh_size, sizeof(long));
		shp[i].sh_offset = off;
		off += roundup(shp[i].sh_size, sizeof(long));
	    }
	}
	esym = VTOP(p);

	/*
	 * Frob the copied ELF header to give information relative
	 * to ssym.
	 */
	elf = (Elf32_Ehdr *)ssym;
	elf->e_phoff = 0;
	elf->e_shoff = sizeof(Elf32_Ehdr);
	elf->e_phentsize = 0;
	elf->e_phnum = 0;
    }
    addr = hdr.eh.e_entry & 0xffffff;
    __exec((caddr_t)addr, opts & RBX_MASK,
	   MAKEBOOTDEV(dev_maj[dsk.type], 0, 0, dsk.unit, dsk.part), 0, esym);
}

static int
parse(char *arg)
{
    char *p, *q;
    int drv, c, i;

    while ((c = *arg++)) {
	if (c == ' ' || c == '\t' || c == '\n')
	    continue;
	for (p = arg; *p && *p != '\n' && *p != ' ' && *p != '\t'; p++);
	if (*p)
	    *p++ = 0;
	if (c == '-') {
	    while ((c = *arg++)) {
		for (i = 0; c != optstr[i]; i++)
		    if (i == NOPT - 1)
			return -1;
		opts ^= 1 << flags[i];
	    }
	    if (opts & 1 << RBX_PROBEKBD) {
		i = *(uint8_t *)PTOV(0x496) & 0x10;
		printf("Keyboard: %s\n", i ? "yes" : "no");
		if (!i)
		    opts |= 1 << RBX_DUAL | 1 << RBX_SERIAL;
		opts &= ~(1 << RBX_PROBEKBD);
	    }
	    ioctrl = opts & 1 << RBX_DUAL ? 0x3 :
		     opts & 1 << RBX_SERIAL ? 0x2 : 0x1;
	    if (ioctrl & 0x2)
	        sio_init();
	} else {
	    for (q = arg--; *q && *q != '('; q++);
	    if (*q) {
		drv = -1;
		if (arg[1] == ':') {
		    if (*arg < '0' || *arg > '9')
			return -1;
		    drv = *arg - '0';
		    arg += 2;
		}
		if (q - arg != 2)
		    return -1;
		for (i = 0; arg[0] != dev_nm[i][0] ||
			    arg[1] != dev_nm[i][1]; i++)
		    if (i == NDEV - 1)
			return -1;
		dsk.type = i;
		arg += 3;
		if (arg[1] != ',' || *arg < '0' || *arg > '9')
		    return -1;
		dsk.unit = *arg - '0';
		arg += 2;
		if (arg[1] != ')' || *arg < 'a' || *arg > 'h')
		    return -1;
		dsk.part = *arg - 'a';
		arg += 2;
		if (drv == -1)
		    drv = dsk.unit;
		dsk.drive = (dsk.type == TYPE_WD ? DRV_HARD : 0) + drv;
		dsk.meta = 0;
		fsread(0, NULL, 0);
	    }
	    if ((i = p - arg - !*(p - 1))) {
		if (i >= sizeof(kname))
		    return -1;
		memcpy(kname, arg, i + 1);
	    }
	}
	arg = p;
    }
    return 0;
}

static ino_t
lookup(const char *path)
{
    char name[MAXNAMLEN + 1];
    const char *s;
    ino_t ino;
    ssize_t n;
    int dt;

    ino = ROOTINO;
    dt = DT_DIR;
    for (;;) {
	if (*path == '/')
	    path++;
	if (!*path)
	    break;
	for (s = path; *s && *s != '/'; s++);
	if ((n = s - path) > MAXNAMLEN)
	    return 0;
	ls = *path == '?' && n == 1 && !*s;
	memcpy(name, path, n);
	name[n] = 0;
	if ((dt = fsfind(name, &ino)) <= 0)
	    break;
	path = s;
    }
    return dt == DT_REG ? ino : 0;
}
static int
xfsread(ino_t inode, void *buf, size_t nbyte)
{
    if (fsread(inode, buf, nbyte) != nbyte) {
	printf("Invalid %s\n", "format");
	return -1;
    }
    return 0;
}

static ssize_t
fsread(ino_t inode, void *buf, size_t nbyte)
{
    static struct fs fs;
    static struct dinode din;
    static char *blkbuf;
    static ufs_daddr_t *indbuf;
    static ino_t inomap;
    static ufs_daddr_t blkmap, indmap;
    static unsigned fsblks;
    char *s;
    ufs_daddr_t lbn, addr;
    size_t n, nb, off;

    if (!dsk.meta) {
	if (!blkbuf)
	    blkbuf = malloc(BSIZEMAX);
	inomap = 0;
	if (dskread(blkbuf, SBOFF / DEV_BSIZE, SBSIZE / DEV_BSIZE))
	    return -1;
	memcpy(&fs, blkbuf, sizeof(fs));
	if (fs.fs_magic != FS_MAGIC) {
	    printf("Not ufs\n");
	    return -1;
	}
	fsblks = fs.fs_bsize >> DEV_BSHIFT;
	dsk.meta++;
    }
    if (!inode)
	return 0;
    if (inomap != inode) {
	if (dskread(blkbuf, fsbtodb(&fs, ino_to_fsba(&fs, inode)),
		    fsblks))
	    return -1;
	din = ((struct dinode *)blkbuf)[inode % INOPB(&fs)];
	inomap = inode;
	fs_off = 0;
	blkmap = indmap = 0;
    }
    s = buf;
    if (nbyte > (n = din.di_size - fs_off))
	nbyte = n;
    nb = nbyte;
    while (nb) {
	lbn = lblkno(&fs, fs_off);
	if (lbn < NDADDR)
	    addr = din.di_db[lbn];
	else {
	    if (indmap != din.di_ib[0]) {
		if (!indbuf)
		    indbuf = malloc(BSIZEMAX);
		if (dskread(indbuf, fsbtodb(&fs, din.di_ib[0]),
			    fsblks))
		    return -1;
		indmap = din.di_ib[0];
	    }
	    addr = indbuf[(lbn - NDADDR) % NINDIR(&fs)];
	}
	n = dblksize(&fs, &din, lbn);
	if (blkmap != addr) {
	    if (dskread(blkbuf, fsbtodb(&fs, addr), n >> DEV_BSHIFT))
		return -1;
	    blkmap = addr;
	}
	off = blkoff(&fs, fs_off);
	n -= off;
	if (n > nb)
	    n = nb;
	memcpy(s, blkbuf + off, n);
	s += n;
	fs_off += n;
	nb -= n;
    }
    return nbyte;
}

static int
dskread(void *buf, unsigned lba, unsigned nblk)
{
    static char *sec;
    struct disklabel *d;

    if (!dsk.meta) {
	if (!sec)
	    sec = malloc(DEV_BSIZE);
	dsk.start = 0;
	if (drvread(sec, LABELSECTOR, 1))
	    return -1;
	d = (void *)(sec + LABELOFFSET);
	if (d->d_magic != DISKMAGIC || d->d_magic2 != DISKMAGIC) {
	    printf("Invalid %s\n", "label");
	    return -1;
	}
	if (dsk.part >= d->d_npartitions ||
	    !d->d_partitions[dsk.part].p_size) {
	    printf("Invalid %s\n", "partition");
	    return -1;
	}
	dsk.start = d->d_partitions[dsk.part].p_offset;
    }
    return drvread(buf, dsk.start + lba, nblk);
}

static int
printf(const char *fmt,...)
{
    static const char digits[16] = "0123456789abcdef";
    va_list ap;
    char buf[10];
    char *s;
    unsigned r, u;
    int c;

    va_start(ap, fmt);
    while ((c = *fmt++)) {
	if (c == '%') {
	    c = *fmt++;
	    switch (c) {
	    case 'c':
		putchar(va_arg(ap, int));
		continue;
	    case 's':
		for (s = va_arg(ap, char *); *s; s++)
		    putchar(*s);
		continue;
	    case 'u':
	    case 'x':
		r = c == 'u' ? 10U : 16U;
		u = va_arg(ap, unsigned);
		s = buf;
		do
		    *s++ = digits[u % r];
		while (u /= r);
		while (--s >= buf)
		    putchar(*s);
		continue;
	    }
	}
	putchar(c);
    }
    va_end(ap);
    return 0;
}

static int
putchar(int c)
{
    if (c == '\n')
	xputc('\r');
    return xputc(c);
}

static void *
memcpy(void *dst, const void *src, size_t size)
{
    const char *s;
    char *d;

    for (d = dst, s = src; size; size--)
	*d++ = *s++;
    return dst;
}

static void *
malloc(size_t size)
{
    static uint32_t next;
    void *p;

    if (!next)
	next = roundup2(__base + _end, 0x10000) - __base;
    p = (void *)next;
    next += size;
    return p;
}

static int
drvread(void *buf, unsigned lba, unsigned nblk)
{
    static unsigned c = 0x2d5c7c2f;

    printf("%c\b", c = c << 8 | c >> 24);
    v86.ctl = V86_ADDR | V86_CALLF | V86_FLAGS;
    v86.addr = 0x704;		/* call to xread in boot1 */
    v86.es = VTOPSEG(buf);
    v86.eax = lba;
    v86.ebx = VTOPOFF(buf);
    v86.ecx = lba >> 16;
    v86.edx = nblk << 8 | dsk.drive;
    v86int();
    v86.ctl = V86_FLAGS;
    if (V86_CY(v86.efl)) {
	printf("Disk error 0x%x (lba=0x%x)\n", v86.eax >> 8 & 0xff,
	       lba);
	return -1;
    }
    return 0;
}

static int
keyhit(unsigned ticks)
{
    uint32_t t0, t1;

    t0 = 0;
    for (;;) {
	if (xgetc(1))
	    return 1;
	t1 = *(uint32_t *)PTOV(0x46c);
	if (!t0)
	    t0 = t1;
	if (t1 < t0 || t1 >= t0 + ticks)
	    return 0;
    }
}

static int
xputc(int c)
{
    if (ioctrl & 0x1)
	putc(c);
    if (ioctrl & 0x2)
	sio_putc(c);
    return c;
}

static int
xgetc(int fn)
{
    for (;;) {
	if (ioctrl & 0x1 && getc(1))
	    return fn ? 1 : getc(0);
	if (ioctrl & 0x2 && sio_ischar())
	    return fn ? 1 : sio_getc();
	if (fn)
	    return 0;
    }
}

static int
getc(int fn)
{
    v86.addr = 0x16;
    v86.eax = fn << 8;
    v86int();
    return fn == 0 ? v86.eax & 0xff : !V86_ZR(v86.efl);
}
