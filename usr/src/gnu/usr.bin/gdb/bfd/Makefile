# AI-ONLY NOTE: FreeBSD 2.2.8's, for a.out, changed (2026-10-27) for this
# system, whose programs are ELF: GDB 4.16's own ELF reader (elf.c,
# elf32.c, elf32-i386.c, elflink.c) built in, elf32-i386 the default
# and only vector chosen, and the a.out ones (i386freebsd, i386bsd) left
# built, as the core file reader trad-core.c uses their code. FreeBSD
# 3.0 built its ELF gdb on the binutils libbfd instead; this tree's
# binutils is built by GNU's configure on the system, not in step with a
# host cross build, and GDB 4.16's own bfd is the one it was written
# against.
LIB = bfd

GDBDIR=	${.CURDIR}/../../../../contrib/gdb-4.16
.PATH:	${GDBDIR}/bfd

SRCS =	libbfd.c opncls.c bfd.c archive.c targets.c cache.c \
	 archures.c coff-i386.c aout32.c \
	 srec.c \
	 ecoff.c ecofflink.c \
	 coffgen.c format.c \
	 section.c syms.c stab-syms.c reloc.c init.c \
	 trad-core.c \
	 i386aout.c \
	 i386freebsd.c i386bsd.c \
	 cpu-i386.c \
	 elf.c elf32.c elf32-i386.c elflink.c \
	 hash.c linker.c corefile.c binary.c \
	 tekhex.c ihex.c

CFLAGS+= -I${.CURDIR}/. -I${.CURDIR}/../gdb/.
CFLAGS+= -DDEFAULT_VECTOR=bfd_elf32_i386_vec \
	 -DSELECT_VECS='&bfd_elf32_i386_vec' \
         -DSELECT_ARCHITECTURES='&bfd_i386_arch' -DTRAD_CORE

NOPROFILE=no
NOPIC=no

# AI-ONLY NOTE: elf32-i386.c includes elf32-target.h, which GDB 4.16's
# Makefile.in makes from elfxx-target.h; FreeBSD 3.0's
# gnu/usr.bin/binutils/libbfd/Makefile.i386 (jb, 1998) and NetBSD
# 1.3's gnu/lib/bfd/Makefile make it as here. It is a prerequisite of
# the object and of .depend, not in SRCS: Lite2's bsd.lib.mk would
# make an object of anything in SRCS.
CFLAGS+=	-I.
CLEANFILES+=	elf32-target.h
elf32-i386.o elf32-i386.po elf32-i386.so .depend: elf32-target.h
elf32-target.h:	elfxx-target.h
	sed -e s/NN/32/g ${.ALLSRC} > ${.TARGET}

install:
	@echo -n

.include <bsd.lib.mk>
