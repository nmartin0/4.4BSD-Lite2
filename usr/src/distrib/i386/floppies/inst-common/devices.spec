#	devices.spec -- the device nodes of the install root
#
# AI-ONLY NOTE: an mtree specification, read by makefs -F (see
# Makefile.inc). It is what `sh MAKEDEV std wd0 wd1' makes, by
# etc/etc.i386/MAKEDEV's own numbers and modes, less std's drum,
# stdin, stdout, stderr and fd/: nothing the install runs opens them.
# NetBSD 1.1 ran MAKEDEV into the floppy instead.

/set type=dir uname=root gname=wheel mode=0755
.
dev
console	type=char mode=0600 device=native,0,0
tty	type=char mode=0666 device=native,1,0
mem	type=char mode=0640 gname=kmem device=native,2,0
kmem	type=char mode=0640 gname=kmem device=native,2,1
null	type=char mode=0666 device=native,2,2
zero	type=char mode=0666 device=native,2,12
klog	type=char mode=0600 device=native,7,0
wd0a	type=block mode=0640 gname=operator device=native,0,0
wd0b	type=block mode=0640 gname=operator device=native,0,1
wd0c	type=block mode=0640 gname=operator device=native,0,2
wd0d	type=block mode=0640 gname=operator device=native,0,3
wd0e	type=block mode=0640 gname=operator device=native,0,4
wd0f	type=block mode=0640 gname=operator device=native,0,5
wd0g	type=block mode=0640 gname=operator device=native,0,6
wd0h	type=block mode=0640 gname=operator device=native,0,7
rwd0a	type=char mode=0640 gname=operator device=native,3,0
rwd0b	type=char mode=0640 gname=operator device=native,3,1
rwd0c	type=char mode=0640 gname=operator device=native,3,2
rwd0d	type=char mode=0640 gname=operator device=native,3,3
rwd0e	type=char mode=0640 gname=operator device=native,3,4
rwd0f	type=char mode=0640 gname=operator device=native,3,5
rwd0g	type=char mode=0640 gname=operator device=native,3,6
rwd0h	type=char mode=0640 gname=operator device=native,3,7
wd1a	type=block mode=0640 gname=operator device=native,0,8
wd1b	type=block mode=0640 gname=operator device=native,0,9
wd1c	type=block mode=0640 gname=operator device=native,0,10
wd1d	type=block mode=0640 gname=operator device=native,0,11
wd1e	type=block mode=0640 gname=operator device=native,0,12
wd1f	type=block mode=0640 gname=operator device=native,0,13
wd1g	type=block mode=0640 gname=operator device=native,0,14
wd1h	type=block mode=0640 gname=operator device=native,0,15
rwd1a	type=char mode=0640 gname=operator device=native,3,8
rwd1b	type=char mode=0640 gname=operator device=native,3,9
rwd1c	type=char mode=0640 gname=operator device=native,3,10
rwd1d	type=char mode=0640 gname=operator device=native,3,11
rwd1e	type=char mode=0640 gname=operator device=native,3,12
rwd1f	type=char mode=0640 gname=operator device=native,3,13
rwd1g	type=char mode=0640 gname=operator device=native,3,14
rwd1h	type=char mode=0640 gname=operator device=native,3,15
..
..
