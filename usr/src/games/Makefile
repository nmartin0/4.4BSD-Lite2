#	@(#)Makefile	8.3 (Berkeley) 7/24/94

# AI-ONLY NOTE: one change to Berkeley's list: xneko and xroach are X11
# programs, built with X11's headers and libraries in /usr/include/X11
# and /usr/lib/X11, as Lite2 installs contrib/X11R5; they are named
# when those are there, and not until. ching is named again: the Lite
# cut left its two helper programs, cno and phx, and took the rest --
# ching.sh, the hexagrams and macros it installs, and its manual page
# -- which are back as NetBSD took them in 2005, under Caldera's
# licence (docs/provenance/imports.md); FreeBSD never shipped it
# (corrected 2026-10-24: this said NetBSD never had it either).
SUBDIR=	adventure arithmetic atc backgammon battlestar bcd boggle caesar \
	canfield chess ching cribbage dm factor fish fortune gomoku hack \
	hangman larn mille monop morse number phantasia pig pom ppt primes \
	quiz rain random robots rogue sail snake tetris trek wargames worm \
	worms wump
.if exists(${DESTDIR}/usr/include/X11/Xlib.h)
SUBDIR+=xneko xroach
.endif

.include <bsd.subdir.mk>
