# $FreeBSD$
#
# AI-ONLY NOTE: FreeBSD 4.1's, for this tree's make, GCC and GNU
# binutils:
#
#   BINDIR	/usr/mdec, where disklabel -B looks, with wdboot and bootwd
#		linked to boot1 and boot2: those are the names it makes
#		from wd0 (sbin/disklabel/disklabel.c). Its two-piece rule
#		-- the first sector from one file, the next 7680 bytes from
#		the other, with the label's sector left as the other's
#		first 512 bytes -- is the layout boot2.ldr reserves.
#   CFLAGS	FreeBSD 4.1's, which its GCC 2.95.2 compiled boot2 with
#		and this tree's GCC 2.95.3 takes as they are, but -elf:
#		FreeBSD's GCC made a.out or ELF and was told which, and
#		this one makes only ELF.
#   dd		bs=7680 conv=sync for obs=7680 conv=osync: GNU dd has no
#		osync, and the two pad the one block the same way.
#   install	written out with install(1), as this tree's
#		bsd.prog.mk does; ${INSTALL} ${COPY} are FreeBSD's.

BINDIR?=	/usr/mdec
BINMODE=	444
NOMAN=		noman
OBJCOPY?=	objcopy
CLEANFILES+=	boot1 boot1.out boot1.o \
		boot2 boot2.ldr boot2.bin boot2.ld boot2.out boot2.o \
		sio.o

# A value of 0x80 enables LBA support.
B1FLAGS=	0x80

BOOT_COMCONSOLE_PORT?= 0x3f8
BOOT_COMCONSOLE_SPEED?= 9600
B2SIOFMT?=	0x3

.if exists(${.OBJDIR}/../btx)
BTX=	${.OBJDIR}/../btx
.else
BTX=	${.CURDIR}/../btx
.endif

ORG1=	0x7c00
ORG2=	0x1000

CFLAGS=	-I${.CURDIR}/../btx/lib -I. \
	-Os -fno-builtin -fforce-addr -fdata-sections \
	-malign-functions=0 -malign-jumps=0 -malign-loops=0 -mrtd \
	-mpreferred-stack-boundary=2 \
	-Wall -Waggregate-return -Wbad-function-cast -Wcast-align \
	-Wmissing-declarations -Wmissing-prototypes -Wnested-externs \
	-Wpointer-arith -Wshadow -Wstrict-prototypes -Wwrite-strings

LDFLAGS=-nostdlib -static -N

all: boot1 boot2

boot1: boot1.out
	${OBJCOPY} -S -O binary boot1.out ${.TARGET}

boot1.out: boot1.o
	${LD} ${LDFLAGS} -e start -Ttext ${ORG1} -o ${.TARGET} boot1.o

boot1.o: boot1.s
	${AS} ${AFLAGS} --defsym FLAGS=${B1FLAGS} ${.IMPSRC} -o ${.TARGET}

boot2: boot2.ldr boot2.bin ${BTX}/btx/btx
	btxld -v -E ${ORG2} -f bin -b ${BTX}/btx/btx -l boot2.ldr \
		-o boot2.ld -P 1 boot2.bin
	@ls -l boot2.ld | awk '{ x = 7680 - $$5; \
		print x " bytes available"; if (x < 0) exit 1 }'
	dd if=boot2.ld of=${.TARGET} bs=7680 conv=sync 2>/dev/null

boot2.ldr:
	dd if=/dev/zero of=${.TARGET} bs=512 count=1 2>/dev/null

boot2.bin: boot2.out
	${OBJCOPY} -S -O binary boot2.out ${.TARGET}

boot2.out: boot2.o sio.o
	${LD} ${LDFLAGS} -Ttext ${ORG2} -o ${.TARGET} \
		${BTX}/lib/crt0.o boot2.o sio.o

sio.o: sio.s
	${AS} ${AFLAGS} --defsym SIOPRT=${BOOT_COMCONSOLE_PORT} \
		--defsym SIOFMT=${B2SIOFMT} \
		--defsym SIOSPD=${BOOT_COMCONSOLE_SPEED} \
		${.IMPSRC} -o ${.TARGET}

install:
	install -c -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
		boot1 ${DESTDIR}${BINDIR}/boot1
	install -c -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
		boot2 ${DESTDIR}${BINDIR}/boot2
	rm -f ${DESTDIR}${BINDIR}/wdboot ${DESTDIR}${BINDIR}/bootwd
	ln ${DESTDIR}${BINDIR}/boot1 ${DESTDIR}${BINDIR}/wdboot
	ln ${DESTDIR}${BINDIR}/boot2 ${DESTDIR}${BINDIR}/bootwd

.include <bsd.prog.mk>
