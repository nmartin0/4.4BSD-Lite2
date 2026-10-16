#	@(#)Makefile	8.5 (Berkeley) 3/31/94

# AI-ONLY NOTE: not named: fsdb, icheck and ncheck, programs the Lite
# cut deleted with their directories. A name with no directory stops
# the build here, as it stopped Lite2's own make, which ran each
# command with `sh -ec' (docs/deferred.md, *Sources and directories
# named but absent*).
SUBDIR=	XNSrouted badsect clri disklabel dmesg dump dumpfs dumplfs fastboot \
	fsck ifconfig init mknod mount mount_cd9660 mount_fdesc \
	mount_kernfs mount_lfs mount_nfs mount_null mount_portal \
	mount_procfs mount_umap mount_union mountd newfs newlfs nfsd \
	nfsiod nologin ping quotacheck reboot restore route routed savecore \
	scsiformat shutdown slattach startslip swapon tunefs umount

.include <bsd.subdir.mk>
