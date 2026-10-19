PATH=/sbin:/usr/sbin:/bin:/usr/bin
export PATH

BLOCKSIZE=1k
export BLOCKSIZE

#export TERM for single user shells.
# AI-ONLY NOTE: given a value first, as FreeBSD 2.2's dot.profile gives
# it (TERM=${TERM:-cons25}): exported unset, TERM reached single user's
# programs as an empty name, which no terminal has, and nvi's ex would
# not start. pc3 is this console in Lite2's termcap, "IBM PC 386BSD
# Console".
TERM=${TERM:-pc3}
export TERM

echo 'erase ^H, kill ^U, intr ^C status ^T'
stty erase '^H' kill '^U' intr '^C' status '^T' crt

umask 022

echo "Don't login as root, use the su command."
