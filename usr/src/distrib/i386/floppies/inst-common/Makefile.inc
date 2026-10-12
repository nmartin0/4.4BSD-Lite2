#	$Id: Makefile.inc,v 1.5 1995/05/12 21:16:03 cgd Exp $

# TOP is assumed to be defined by Makefile including this one.

# AI-ONLY NOTE: NetBSD 1.1's, for a host build. NetBSD made the floppy
# on a running NetBSD: a vnode disk, newfs, mount, the lists run into
# the mount point, umount. Here the lists are run into a directory in
# the object directory and makefs, NetBSD's own later answer to the
# same problem and a host tool here as mtree is, makes the filesystem
# from it. The device nodes, which the floppy made by running MAKEDEV
# into it, are in devices.spec, an mtree specification makefs reads:
# making nodes on the build host needs root, and MAKEDEV's groups are
# this tree's, not the host's. mtree and makefs take the tree's own
# etc/ for owner and group names.
#
# The image is the root of the install kernel, not a floppy of its
# own: IMAGE is sized for options MFS_ROOT, FSSIZE in KB, and
# release/write_mfs_in_kernel puts it into the kernel. And the
# crunched binary is linked by the generated makefile run through
# build/make.sh's make, which is how this file is run.

CBIN=		instbin
COMMONDIR=	${TOP}/inst-common
SRCTOP=		${TOP}/../../..

STAGE=		${.OBJDIR}/stage
IMAGE?=		xxx-${REV}.fs
FSSIZE?=	1600
MDEC=		${DESTDIR}/usr/mdec

LISTS=		${COMMONDIR}/list ${.CURDIR}/list
CRUNCHCONF=	${COMMONDIR}/${CBIN}.conf
MTREE=		${COMMONDIR}/mtree.conf
DEVSPEC=	${COMMONDIR}/devices.spec

all: ${CBIN}
	rm -rf ${STAGE}
	mkdir -p ${STAGE}
	mtree -N ${SRCTOP}/etc -def ${MTREE} -p ${STAGE}/ -u >/dev/null
	TOPDIR=${TOP} CURDIR=${.CURDIR} OBJDIR=${.OBJDIR} SRCTOP=${SRCTOP} \
	    MDEC=${MDEC} TARGDIR=${STAGE} sh ${TOP}/runlist.sh ${LISTS}
	rm -f ${IMAGE}
	makefs -t ffs -B le -N ${SRCTOP}/etc -F ${DEVSPEC} \
	    -o version=1,bsize=8192,fsize=1024,density=5120 \
	    -o minfree=0,optimization=space -s ${FSSIZE}k ${IMAGE} ${STAGE}

${CBIN}.conf: ${CRUNCHCONF}
	awk '$$1 == "srcdirs" { for (i = 2; i <= NF; i++) \
	    $$i = "${SRCTOP}/" $$i } { print }' ${CRUNCHCONF} > ${.TARGET}

# The object directory of every program under the srcdirs, made first:
# bmake uses MAKEOBJDIRPREFIX only for a directory that exists, as
# build/make.sh says, and builds in the source directory otherwise.
${CBIN}.mk ${CBIN}.cache ${CBIN}.c: ${CBIN}.conf
	awk '$$1 == "srcdirs" { for (i = 2; i <= NF; i++) print $$i }' \
	    ${CBIN}.conf | while read d; do \
		for p in $$d/*/; do mkdir -p ${MAKEOBJDIRPREFIX}$${p%/}; done; \
	    done
	crunchgen ${CBIN}.conf

${CBIN}: ${CBIN}.mk ${CBIN}.cache ${CBIN}.c
	make -f ${CBIN}.mk all

clean cleandir:
	/bin/rm -rf core ${IMAGE} ${STAGE} ${CBIN} ${CBIN}.conf ${CBIN}.mk \
	    ${CBIN}.cache *.o *.lo *.c

.include <bsd.subdir.mk>
