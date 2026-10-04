/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz and Don Ahn.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)clock.c	7.2 (Berkeley) 5/12/91
 *	from NetBSD: Id: clock.c,v 1.6 1993/05/22 08:01:07 cgd Exp 
 *
 *	@(#)clock.c	8.1 (Berkeley) 6/11/93
 *
 */

/*
 * Primitive clock interrupt routines.
 */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <machine/segments.h>
#include <i386/isa/icu.h>
#include <i386/isa/isa.h>
#include <i386/isa/rtc.h>

/* these should go elsewere (timerreg.h) but to avoid admin overhead... */
/*
 * Macros for specifying values to be written into a mode register.
 */
#define TIMER_CNTR0     (IO_TIMER1 + 0) /* timer 0 counter port */
#define TIMER_CNTR1     (IO_TIMER1 + 1) /* timer 1 counter port */
#define TIMER_CNTR2     (IO_TIMER1 + 2) /* timer 2 counter port */
#define TIMER_MODE      (IO_TIMER1 + 3) /* timer mode port */
#define         TIMER_SEL0      0x00    /* select counter 0 */
#define         TIMER_SEL1      0x40    /* select counter 1 */
#define         TIMER_SEL2      0x80    /* select counter 2 */
#define         TIMER_INTTC     0x00    /* mode 0, intr on terminal cnt */
#define         TIMER_ONESHOT   0x02    /* mode 1, one shot */
#define         TIMER_RATEGEN   0x04    /* mode 2, rate generator */
#define         TIMER_SQWAVE    0x06    /* mode 3, square wave */
#define         TIMER_SWSTROBE  0x08    /* mode 4, s/w triggered strobe */
#define         TIMER_HWSTROBE  0x0a    /* mode 5, h/w triggered strobe */
#define         TIMER_LATCH     0x00    /* latch counter for reading */
#define         TIMER_LSB       0x10    /* r/w counter LSB */
#define         TIMER_MSB       0x20    /* r/w counter MSB */
#define         TIMER_16BIT     0x30    /* r/w counter 16 bits, LSB first */
#define         TIMER_BCD       0x01    /* count in BCD */

#define DAYST 119
#define DAYEN 303

#ifndef	XTALSPEED
#define XTALSPEED 1193182
#endif

startrtclock() {
	int s;

	/*
	 * AI-ONLY NOTE: findcpuspeed(), spinwait() and delaycount are
	 * gone from this file, and the call that stood here with them.
	 *
	 * They existed for one reader: the DELAY macro in
	 * machine/param.h computed delaycount * n / 1000 and spun that
	 * many times. The macro now calls delay(), which watches the
	 * counter, so delaycount had no reader and this call spent a
	 * millisecond of every boot computing a number nothing used.
	 * Nothing else in this tree touched any of the three.
	 *
	 * FreeBSD 2.0.5's clock.c has none of them: it removed them
	 * when it wrote its DELAY against the counter. NetBSD 1.0 and
	 * OpenBSD 1996 both kept theirs after adding delay(), and in
	 * both the only caller of spinwait is findcpuspeed and the
	 * only reader of delaycount is spinwait -- their fd.c, wd.c,
	 * pccons.c and machdep.c touch neither. They left it standing;
	 * FreeBSD did not, and this follows FreeBSD.
	 */
	/* initialize 8253 clock */
	outb(TIMER_MODE, TIMER_SEL0|TIMER_RATEGEN|TIMER_16BIT);

	/* Correct rounding will buy us a better precision in timekeeping */
	outb (IO_TIMER1, (XTALSPEED+hz/2)/hz);
	outb (IO_TIMER1, ((XTALSPEED+hz/2)/hz)/256);

	/* initialize brain-dead battery powered clock */
	outb (IO_RTC, RTC_STATUSA);
	outb (IO_RTC+1, 0x26);
	outb (IO_RTC, RTC_STATUSB);
	outb (IO_RTC+1, 2);

	outb (IO_RTC, RTC_DIAG);
	if (s = inb (IO_RTC+1))
		printf("RTC BIOS diagnostic error %b\n", s, RTCDG_BITS);
	outb (IO_RTC, RTC_DIAG);
	outb (IO_RTC+1, 0);
}




/* convert 2 digit BCD number */
bcd(i)
int i;
{
	return ((i/16)*10 + (i%16));
}

/* convert years to seconds (from 1970) */
unsigned long
ytos(y)
int y;
{
	int i;
	unsigned long ret;

	ret = 0;
	for(i = 1970; i < y; i++) {
		if (i % 4) ret += 365*24*60*60;
		else ret += 366*24*60*60;
	}
	return ret;
}

/* convert months to seconds */
unsigned long
mtos(m,leap)
int m,leap;
{
	int i;
	unsigned long ret;

	ret = 0;
	for(i=1;i<m;i++) {
		switch(i){
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			ret += 31*24*60*60; break;
		case 4: case 6: case 9: case 11:
			ret += 30*24*60*60; break;
		case 2:
			if (leap) ret += 29*24*60*60;
			else ret += 28*24*60*60;
		}
	}
	return ret;
}


/*
 * Initialize the time of day register, based on the time base which is, e.g.
 * from a filesystem.
 */
inittodr(base)
	time_t base;
{
	unsigned long sec;
	int leap,day_week,t,yd;
	int sa,s;

	/* do we have a realtime clock present? (otherwise we loop below) */
	sa = rtcin(RTC_STATUSA);
	if (sa == 0xff || sa == 0) return;

	/* ready for a read? */
	while ((sa&RTCSA_TUP) == RTCSA_TUP)
		sa = rtcin(RTC_STATUSA);

	sec = bcd(rtcin(RTC_YEAR)) + 1900;
	if (sec < 1970)
		sec += 100;
	leap = !(sec % 4); sec = ytos(sec); /* year    */
	yd = mtos(bcd(rtcin(RTC_MONTH)),leap); sec += yd;	/* month   */
	t = (bcd(rtcin(RTC_DAY))-1) * 24*60*60; sec += t; yd += t; /* date    */
	day_week = rtcin(RTC_WDAY);				/* day     */
	sec += bcd(rtcin(RTC_HRS)) * 60*60;			/* hour    */
	sec += bcd(rtcin(RTC_MIN)) * 60;			/* minutes */
	sec += bcd(rtcin(RTC_SEC));				/* seconds */

	/* XXX off by one? Need to calculate DST on SUNDAY */
	/* Perhaps we should have the RTC hold GMT time to save */
	/* us the bother of converting. */
	yd = yd / (24*60*60);
	if ((yd >= DAYST) && ( yd <= DAYEN)) {
		sec -= 60*60;
	}
	sec += tz.tz_minuteswest * 60;

	time.tv_sec = sec;
}

#ifdef garbage
/*
 * Initialze the time of day register, based on the time base which is, e.g.
 * from a filesystem.
 */
test_inittodr(base)
	time_t base;
{

	outb(IO_RTC,9); /* year    */
	printf("%d ",bcd(inb(IO_RTC+1)));
	outb(IO_RTC,8); /* month   */
	printf("%d ",bcd(inb(IO_RTC+1)));
	outb(IO_RTC,7); /* day     */
	printf("%d ",bcd(inb(IO_RTC+1)));
	outb(IO_RTC,4); /* hour    */
	printf("%d ",bcd(inb(IO_RTC+1)));
	outb(IO_RTC,2); /* minutes */
	printf("%d ",bcd(inb(IO_RTC+1)));
	outb(IO_RTC,0); /* seconds */
	printf("%d\n",bcd(inb(IO_RTC+1)));

	time.tv_sec = base;
}
#endif

/*
 * Restart the clock.
 */
resettodr()
{
}

/*
 * Wire clock interrupt in.
 */
#define V(s)	__CONCAT(V, s)
extern V(clk)();
enablertclock() {
	INTREN(IRQ0);
	setidt(ICU_OFFSET+0, &V(clk), SDT_SYS386IGT, SEL_KPL);
	splnone();
}

/*
 * AI-ONLY NOTE: gettick and delay, which this port did not have.
 *
 * gettick latches counter 0 before reading it and does so with
 * interrupts off. findcpuspeed above read the counter with two bare
 * inb's while it was still running, so the low and high bytes need
 * not belong to the same value; measured under QEMU that read
 * disagreed with a latched one by one tick usually and by
 * twenty-five once. NetBSD 1.0's clock.c:147 is this function,
 * TIMER_SEL0|TIMER_LATCH and all; FreeBSD 2.0.5 calls its getit.
 *
 * delay waits n microseconds by watching the counter rather than by
 * counting loop iterations. What it replaces, the DELAY macro in
 * machine/param.h, computed delaycount * n / 1000 in a signed int,
 * which overflows: measured on this port's own calibration under
 * QEMU, delaycount came out 271473 and the product passes INT_MAX
 * above about 7900 microseconds, where N goes negative and the loop
 * exits at once. Six of the eight DELAY call sites in this
 * configuration ask for more than that, com.c's 100 millisecond wait
 * and pccons.c's four second one among them.
 *
 * The scaling is NetBSD 1.0's and FreeBSD 2.0.5's non-assembly path,
 * which both write identically and FreeBSD comments as "without
 * using floating point and without any avoidable overflows": split n
 * into seconds and microseconds and sum four partial products, none
 * of which can overflow. NetBSD also has a __GNUC__ path using mul
 * and div in inline assembly for a 64-bit intermediate; it is not
 * taken here, the decomposition being arithmetic a reader can check
 * and this tree having been bitten once already by hand-written
 * register constraints (see lib/libc/arch/i386/gen/ldexp.c).
 *
 * Verified on a multiboot harness under QEMU before being written
 * here: the scaling is exact -- 1193 ticks for 1000 us and 4772728
 * for 4000000, against TIMER_FREQ of 1193182 -- and the poll loop
 * was traced iteration by iteration at 10 ms, the counter descending
 * and the remaining count converging. delay(1000) measured 979 to
 * 982 us consistently; delay(100) measured 100 to 125, the spread
 * being FreeBSD's guessed 20 us of setup overhead, which is a fifth
 * of that request.
 *
 * The n -= 20 is theirs and is that guess. hz is read rather than
 * assumed so the wrap arithmetic follows whatever startrtclock
 * loaded.
 */
gettick()
{
	unsigned char lo, hi;
	int s;

	/*
	 * Don't want someone changing the counter while we're here.
	 * Both donors write disable_intr() and enable_intr() around
	 * this; neither exists in this tree, whose own way of shutting
	 * interrupts out of a short sequence is splhigh and splx --
	 * isa/com.c:651 and isa/if_we.c:308 among others. splhigh
	 * raises the priority to mask everything, which is what the
	 * latch needs: the two reads must not be separated.
	 */
	s = splhigh();
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_LATCH);
	lo = inb(TIMER_CNTR0);
	hi = inb(TIMER_CNTR0);
	splx(s);
	return ((hi << 8) | lo);
}

delay(n)
	int n;
{
	int limit, tick, otick, sec, usec;

	/*
	 * Read the counter first, so that the rest of the setup
	 * overhead is counted.
	 */
	otick = gettick();

	n -= 20;
	if (n <= 0)
		return;

	/*
	 * (n * XTALSPEED) / 1e6, without floating point and without
	 * any avoidable overflow.
	 */
	sec = n / 1000000;
	usec = n - sec * 1000000;
	n = sec * XTALSPEED
	    + usec * (XTALSPEED / 1000000)
	    + usec * ((XTALSPEED % 1000000) / 1000) / 1000
	    + usec * (XTALSPEED % 1000) / 1000000;

	limit = XTALSPEED / hz;

	while (n > 0) {
		tick = gettick();
		if (tick > otick)
			n -= limit - (tick - otick);
		else
			n -= otick - tick;
		otick = tick;
	}
}

/*
 * AI-ONLY NOTE: cpu_initclocks and setstatclockrate, which
 * kern/kern_clock.c calls and this port did not have. hp300,
 * luna68k, sparc, pmax and news3400 all define both in their own
 * clock.c; this file had the 386BSD names -- startrtclock,
 * enablertclock, spinwait -- and was never connected to the 4.4BSD
 * clock interface.
 *
 * What cpu_initclocks must do is visible in hp300's: settle hz,
 * stathz and profhz, compute tick, then start the hardware.
 * enablertclock above already starts the hardware here, so this sets
 * the rates and calls it. The three donors all register the clock
 * interrupt at run time instead -- intr_establish in NetBSD 1.0,
 * register_intr in FreeBSD 2.0.5, isa_intr_establish in OpenBSD 1996
 * -- which this tree has no equivalent of, its interrupts being
 * wired in by config. So none of their versions could be taken.
 *
 * stathz stays zero: there is no statistics clock on this port.
 * FreeBSD 2.0.5 and OpenBSD 1996 start a second clock on IRQ 8 for
 * one, which is work this does not attempt. kern_clock.c reads
 * stathz == 0 as meaning the hardclock does the statistics, which is
 * what happens here.
 *
 * setstatclockrate is therefore empty, as NetBSD 1.0's i386 one is.
 * hp300's switches between two reload values, having a second clock
 * to switch.
 */
void
cpu_initclocks()
{

	tick = 1000000 / hz;
	enablertclock();
}

void
setstatclockrate(newhz)
	int newhz;
{
}





