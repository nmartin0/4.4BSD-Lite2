#	@(#)Makefile	8.13 (Berkeley) 4/29/95

# AI-ONLY NOTE: for the i386, binutils 2.11.2, the system's assembler,
# linker and binary tools (binutils-2.11.2/README.BSD), and the
# programs of Berkeley's list below as each is taken up and made to
# build here, one at a time (docs/roadmap.md, *Next: contrib/*). Not
# gcc-2.3.3, gas-1.38 and gdb-4.7.LBL: they make and read a.out, and
# this system is ELF; GCC 2.95.3 and binutils 2.11.2 stand in for the
# first two, and a gdb that reads ELF is to come. And two not on
# Berkeley's list: diff-2.6, GNU's, the system's diff, diff3 and sdiff,
# for the usr.bin/diff the Lite cut emptied; and ed, Berkeley's own
# replacement for the bin/ed the Lite cut deleted.
.if ${MACHINE} == "i386"
SUBDIR=	ansi bib binutils-2.11.2 diff-2.6 ed flex-2.5.2 gawk-2.15.2 \
	gzip-1.2.4 hunt jove-4.14.6 nvi
.else
SUBDIR=	ansi bib bind-4.9.2 flex-2.5.2 gawk-2.15.2 emacs-18.57 gas-1.38 \
	gcc-2.3.3 gdb-4.7.LBL groff-1.08 gzip-1.2.4 hunt jove-4.14.6 \
	kermit-5A.188 libg++-2.3 mh-6.8.3a nvi perl-4.036 rcs-V5.6
.endif

cleandir: clean

obj depend tags: 

.include <bsd.subdir.mk>
