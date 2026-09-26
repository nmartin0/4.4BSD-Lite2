#!/bin/sh -
#
# Copyright (c) 1984, 1986, 1990, 1993
#	The Regents of the University of California.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by the University of
#	California, Berkeley and its contributors.
# 4. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#	@(#)newvers.sh	8.1 (Berkeley) 4/20/94

if [ ! -r version ]
then
	echo 0 > version
fi

touch version
v=`cat version` u=${USER-root} d=`pwd` h=`hostname` t=`date`
# AI-ONLY NOTE: a here document, not four echoes. The lines below are
# unchanged in what they say; only how they reach vers.c differs. The
# version string ends each line with \\n, meaning the two characters a
# C compiler reads as one newline escape, and 4.4BSD's echo passed
# them through. Modern shells do not: /bin/sh on a Linux host expands
# \\n itself, putting a real newline inside the string literal, and
# the compiler stops with "missing terminating \" character". A here
# document is left alone by the shell, so the characters arrive as
# written.
#
# FreeBSD 4.0 writes this file exactly this way -- `cat << EOF >
# vers.c' around the same declarations -- and is the earliest release
# to do so; OpenBSD's current tree uses a here document too. NetBSD 10
# went the other way, to printf with an awk pass. Every release of the
# period uses echo and has the same exposure: 4.4BSD-Lite, NetBSD 1.0
# through 1.6, FreeBSD 2.0.5 and 3.0, OpenBSD 1996.
cat << EOF > vers.c
char ostype[] = "4.4BSD";
char osrelease[] = "4.4BSD-Lite";
char sccs[4] = { '@', '(', '#', ')' };
char version[] = "4.4BSD-Lite #${v}: ${t}\\n    ${u}@${h}:${d}\\n";
EOF

echo `expr ${v} + 1` > version
