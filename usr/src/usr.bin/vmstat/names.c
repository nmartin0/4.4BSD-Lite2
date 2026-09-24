/*-
 * Copyright (c) 1991, 1993
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
 *
 *	@(#)names.c	8.1 (Berkeley) 6/6/93
 */

#if !defined(hp300) && !defined(tahoe) && !defined(vax) && \
	!defined(luna68k) && !defined(mips)
char *defdrives[] = { 0 };
#endif

/*
 * AI-ONLY NOTE: this file has cases for hp300, tahoe, vax, luna68k,
 * mips and sun, and none for i386, so read_names is undefined and
 * usr.bin/vmstat, usr.bin/vmstat.sparc and usr.bin/systat do not link.
 * Berkeley left the i386 port short here, as it did in lib/libkvm.
 *
 * This is NetBSD 1.0's i386 case, the same stub they give pc532. It
 * returns success and fills nothing, and vmstat.c already has
 * Berkeley's own path for that: a drive whose name is still NULL is
 * labelled ??0, ??1 and so on, and the statistics are reported as
 * usual. defdrives above is already empty for this machine, which is
 * the other half of the same answer.
 *
 * FreeBSD 2.0.5 has a real implementation here instead, by Rodney W.
 * Grimes: it reads namelist[X_DK_NAMES] out of the kernel and gives
 * the drives their true names. It is not taken because it depends on
 * a kernel symbol this tree has not been shown to export, which
 * cannot be checked until a kernel built from this tree runs. If real
 * names are wanted later, that is where to look; the choice is
 * recorded in docs/provenance/missing.md.
 *
 * Neither lineage kept this file: NetBSD replaced it with dkstats.c
 * at 1.2 and FreeBSD dropped it by 3.0, and OpenBSD already had
 * dkstats.c in 1996. The stub is what fits this tree as it stands.
 */
#if defined(i386)
int
read_names()
{
	return 1;
}
#endif

#if defined(hp300) || defined(luna68k)
#if defined(hp300)
#include <hp/dev/device.h>
#else
#include <luna68k/dev/device.h>
#endif

char *defdrives[] = { "sd0", "sd1", "sd2", "rd0", "rd1", "rd2", 0 };

int
read_names()
{
	register char *p;
	register u_long hp;
	static char buf[BUFSIZ];
	struct hp_device hdev;
	struct driver hdrv;
	char name[10];

	hp = namelist[X_HPDINIT].n_value;
	if (hp == 0) {
		(void)fprintf(stderr,
		    "disk init info not in namelist\n");
		return (0);
	}
	p = buf;
	for (;; hp += sizeof hdev) {
		(void)kvm_read(kd, hp, &hdev, sizeof hdev);
		if (hdev.hp_driver == 0)
			break;
		if (hdev.hp_dk < 0 || hdev.hp_alive == 0 ||
		    hdev.hp_cdriver == 0)
			continue;
		(void)kvm_read(kd, (u_long)hdev.hp_driver, &hdrv, sizeof hdrv);
		(void)kvm_read(kd, (u_long)hdrv.d_name, name, sizeof name);
		dr_name[hdev.hp_dk] = p;
		p += sprintf(p, "%s%d", name, hdev.hp_unit) + 1;
	}
	return (1);
}
#endif /* hp300 || luna68k */

#ifdef tahoe
#include <tahoe/vba/vbavar.h>

char *defdrives[] = { "dk0", "dk1", "dk2", 0 };

int
read_names()
{
	register char *p;
	struct vba_device udev, *up;
	struct vba_driver udrv;
	char name[10];
	static char buf[BUFSIZ];

	up = (struct vba_device *)namelist[X_VBDINIT].n_value;
	if (up == 0) {
		(void) fprintf(stderr,
		    "disk init info not in namelist\n");
		return (0);
	}
	p = buf;
	for (;; up += sizeof udev) {
		(void)kvm_read(kd, up, &udev, sizeof udev);
		if (udev.ui_driver == 0)
			break;
		if (udev.ui_dk < 0 || udev.ui_alive == 0)
			continue;
		(void)kvm_read(kd, udev.ui_driver, &udrv, sizeof udrv);
		(void)kvm_read(kd, udrv.ud_dname, name, sizeof name);
		dr_name[udev.ui_dk] = p;
		p += sprintf(p, "%s%d", name, udev.ui_unit);
	}
	return (1);
}
#endif /* tahoe */

#ifdef vax
#include <vax/uba/ubavar.h>
#include <vax/mba/mbavar.h>

char *defdrives[] = { "hp0", "hp1", "hp2", 0 };

int
read_names()
{
	register char *p;
	unsigned long mp, up;
	struct mba_device mdev;
	struct mba_driver mdrv;
	struct uba_device udev;
	struct uba_driver udrv;
	char name[10];
	static char buf[BUFSIZ];

	mp = namelist[X_MBDINIT].n_value;
	up = namelist[X_UBDINIT].n_value;
	if (mp == 0 && up == 0) {
		(void)fprintf(stderr,
		    "disk init info not in namelist\n");
		return (0);
	}
	p = buf;
	if (mp)
		for (;; mp += sizeof mdev) {
			(void)kvm_read(kd, mp, &mdev, sizeof mdev);
			if (mdev.mi_driver == 0)
				break;
			if (mdev.mi_dk < 0 || mdev.mi_alive == 0)
				continue;
			(void)kvm_read(kd, mdev.mi_driver, &mdrv, sizeof mdrv);
			(void)kvm_rea(kd, mdrv.md_dname, name, sizeof name);
			dr_name[mdev.mi_dk] = p;
			p += sprintf(p, "%s%d", name, mdev.mi_unit);
		}
	if (up)
		for (;; up += sizeof udev) {
			(void)kvm_read(kd, up, &udev, sizeof udev);
			if (udev.ui_driver == 0)
				break;
			if (udev.ui_dk < 0 || udev.ui_alive == 0)
				continue;
			(void)kvm_read(kd, udev.ui_driver, &udrv, sizeof udrv);
			(void)kvm_read(kd, udrv.ud_dname, name, sizeof name);
			dr_name[udev.ui_dk] = p;
			p += sprintf(p, "%s%d", name, udev.ui_unit);
		}
	return (1);
}
#endif /* vax */

#ifdef sun
#include <sundev/mbvar.h>

int
read_names()
{
	static int once = 0;
	struct mb_device mdev;
	struct mb_driver mdrv;
	short two_char;
	char *cp = (char *) &two_char;
	register struct mb_device *mp;

	mp = (struct mb_device *)namelist[X_MBDINIT].n_value;
	if (mp == 0) {
		(void)fprintf(stderr,
		    "disk init info not in namelist\n");
		return (0);
	}
	for (;; ++mp) {
		(void)kvm_read(kd, mp++, &mdev, sizeof(mdev));
		if (mdev.md_driver == 0)
			break;
		if (mdev.md_dk < 0 || mdev.md_alive == 0)
			continue;
		(void)kvm_read(kd, mdev.md_driver, &mdrv, sizeof(mdrv));
		(void)kvm_read(kd, mdrv.mdr_dname, &two_char, sizeof(two_char));
		(void)sprintf(dr_name[mdev.md_dk],
		    "%c%c%d", cp[0], cp[1], mdev.md_unit);
	}
	return(1);
}
#endif /* sun */

#if defined(mips)
#include <pmax/dev/device.h>

char *defdrives[] = { "rz0", "rz1", "rz2", "rz3", "rz4", "rz5", "rz6", 0 };

int
read_names()
{
	register char *p;
	register u_long sp;
	static char buf[BUFSIZ];
	struct scsi_device sdev;
	struct driver hdrv;
	char name[10];

	sp = namelist[X_SCSI_DINIT].n_value;
	if (sp == 0) {
		(void)fprintf(stderr, "disk init info not in namelist\n");
		return (0);
	}
	p = buf;
	for (;; sp += sizeof sdev) {
		(void)kvm_read(kd, sp, &sdev, sizeof sdev);
		if (sdev.sd_driver == 0)
			break;
		if (sdev.sd_dk < 0 || sdev.sd_alive == 0 ||
		    sdev.sd_cdriver == 0)
			continue;
		(void)kvm_read(kd, (u_long)sdev.sd_driver, &hdrv, sizeof hdrv);
		(void)kvm_read(kd, (u_long)hdrv.d_name, name, sizeof name);
		dr_name[sdev.sd_dk] = p;
		p += sprintf(p, "%s%d", name, sdev.sd_unit) + 1;
	}
	return (1);
}
#endif /* mips */
