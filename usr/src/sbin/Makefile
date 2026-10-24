#	@(#)Makefile	8.5 (Berkeley) 3/31/94

# AI-ONLY NOTE: not named: icheck and ncheck, programs the Lite cut
# deleted with their directories, which no BSD of the period had
# either (docs/deferred.md). fsdb is named again: John Kohl's, NetBSD
# 1.1's, in place of Berkeley's, which the Lite cut took
# (docs/provenance/imports.md). A name with no directory stops
# the build here, as it stopped Lite2's own make, which ran each
# command with `sh -ec' (docs/deferred.md, *Sources and directories
# named but absent*).
SUBDIR=	XNSrouted badsect clri disklabel dmesg dump dumpfs dumplfs fastboot \
	fsck fsdb ifconfig init mknod mount mount_cd9660 mount_fdesc \
	mount_kernfs mount_lfs mount_nfs mount_null mount_portal \
	mount_procfs mount_umap mount_union mountd newfs newlfs nfsd \
	nfsiod nologin ping quotacheck reboot restore route routed savecore \
	scsiformat shutdown slattach startslip swapon tunefs umount

.include <bsd.subdir.mk>
