# AI-ONLY NOTE: FreeBSD 2.2.8's list was bfd libiberty gdb doc gdbserver
# gdbreplay. Changed (2026-10-27): readline added, GDB 4.16's own, as
# it has its own bfd and libiberty here; doc, gdbserver and gdbreplay
# left out -- the manual needs bsd.info.mk, which Lite2 has not, and
# the other two are remote debugging stubs this system has no use for
# (kernel debugging here is ddb and gdb -k's kernel-core reader).
# Their directories stay as FreeBSD has them.
SUBDIR= bfd libiberty readline gdb

.include <bsd.subdir.mk>
