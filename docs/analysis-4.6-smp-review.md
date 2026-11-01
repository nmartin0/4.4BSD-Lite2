<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# Three analyses reviewed: what 4.6BSD should lean towards, and what it should drop

AI-ONLY NOTE. Written 2026-11-01 at the maintainer's request. It reviews
three documents the maintainer had other agents write on 2026-10-31.
They are not in this tree, and this file does not copy them into it:

- **`analysis-4.6-smp-primitives.md`**: SMP built from Lite2's own
  primitives. Here, "the SMP note".
- **`analysis-4.6-mach-primitives.md`**: what CMU Mach and OSF MK offer
  the same plan. Here, "the Mach note".
- **`Report.md`**: how much of the posixcheck suite depends on kernel
  design. Here, "the suite report".

This is a judgment, not a decision, and it changes no plan. As the
maintainer asked, nothing here is added to `roadmap-4.6.md`,
`plan-4.6.md` or `guide-4.6.md`. Each "lean" or "drop" below is
measured against 4.6BSD's own rules (section 1) and gives its reason.
Where a decision belongs to the maintainer, section 7 says so.

What was done for it:

- **Read in full:** each of the three, twice; `design-4.6.md`;
  `guide-4.6.md`; `plan-4.6.md`; `analysis-4.6-continuations.md`
  sections 3, 4, 5, 11 and 13; `roadmap-4.6.md` *Kernel primitives* and
  *SMP from FreeBSD*.
- **Checked by script in this tree:** every claim about our tree that
  could be checked (section 2).
- **Checked in donors:** NetBSD 1.5's `kern_synch.c`; OpenBSD's current
  `proc.h`, `cpu.h` and `mutex.h`.
- **Not checked:** OSF MK and Mach source (not readable from this
  workspace; the Mach note's claims about them are taken as stated);
  DragonFly's source; anything built or run.

The SMP and Mach notes refer to three more notes that are not in this
tree either: `analysis-4.6-percpu.md`, `analysis-4.6-vm-buildout.md` and
`analysis-4.6-radixvm-design.md`. Their claims were not checked.

## 1. The yardstick

**4.6BSD's goal is POSIX** (`design-4.6.md` 1): POSIX.1-2024 with C17,
the System V ABI, and a usable system on current hardware. SMP is
admitted under rule 2, because it makes the system fast "*without
changing what a POSIX program sees*". So SMP serves the goal; it is not
the goal.

The rules that decide between the ideas below:

- **Rule 1, deviate from Lite2 as little as possible.** Lite2's code and
  design are the default.
- **Rule 1a, best fit** (4.6BSD's rule; 4.5BSD keeps first fit). Six
  points: smallest change to Lite2; same semantics as 4.4 ("its idioms,
  its interfaces, its locking model"); complete; fewest foreign
  dependencies; clean provenance; a continuation, not a redesign. The
  source order (Lite2, 386BSD, the other BSDs, Mach, Minix, our own) is
  where to look. Best fit chooses among what that order admits, so Mach
  can win on fit.
- **No original works** (`provenance/precedent.md`): "This project no
  longer writes function bodies at all". A missing body is imported from
  a donor and cut down, not written.
- **Rule 5:** C17 atomics rather than per-port assembly.
- **`design-4.6.md` 6, driver Tier C:** "mutexes, reader/writer locks,
  the kernel lock, task queues" come "with the SMP decision", from
  OpenBSD's plateau. The modern drivers 4.6BSD imports call these.
- **`design-4.6.md` 9, item 6:** "threads and SMP decided together after
  a survey of the precedents back to the earliest."

**How 4.6BSD differs from 4.5BSD here.** 4.5BSD restores. Its rule is
the earliest continuation that closes a gap, and it has no SMP and no
threads. 4.6BSD continues 4.4 forward. It may take any donor's code at
any date, provided the code is still a continuation of the 4.4 design
and fits it best. So for 4.6BSD the question about each idea is not
"who did this first" but "which form changes Lite2 least and keeps its
meaning".

**A decision this tree does not yet record.** The SMP note says the
maintainer has decided: no big kernel lock, a preemptive kernel, and
DragonFly-style tokens where they make sense. This tree's
`plan-4.6.md` still lists "big-lock SMP first" among the decisions
taken, and its stage 5.3 and `guide-4.6.md` 4 still give OpenBSD's 2004
kernel lock as the route. This review takes the maintainer's newer
decision as given. The plan was not changed, as asked (section 7).

## 2. What the tree says, checked

### 2.1 Confirmed

- **`lockmgr` is Mach's lock.** `kern/kern_lock.c` credits Avadis
  Tevanian, Michael Wayne Young and the Mach project.
- **The VM calls Mach's wait primitives.** `vm/vm_glue.c` lines 530-577
  emulate `assert_wait`, `thread_block`, `thread_sleep` and
  `thread_wakeup` for one CPU.
- **Lite2 states its own rule** (`i386/include/param.h`): "uniprocessors
  will always run to completion or a sleep. It is an error to hold one
  of these locks while a process is sleeping." Debug kernels check it in
  `mi_switch` (`kern_synch.c`, `p->p_simple_locks`).
- **Sleep has 128 hashed queues** (`kern_synch.c:261-266`), with `tsleep`
  at line 291 and `wakeup` at line 506.
- **`need_resched` and `aston` set global flags**
  (`i386/include/cpu.h:135,158`).
- **Only two kernel processes exist besides `init`:** the swapper and
  the page daemon (`init_main.c`).
- **`test_and_set` is called but defined nowhere** in the tree.

### 2.2 Not as the notes say: Lite2's multiprocessor path has never compiled

Both notes describe Lite2's multiprocessor lock code as present and
needing only an atomic test-and-set. On the i386 it cannot have been
built in either configuration:

- **Without `DEBUG`:** `simple_lock_try` in `i386/include/param.h`
  ends `return (!test_and_set(&lkp->lock_data))` with no semicolon
  (line 252). That code is compiled only when `NCPUS > 1`.
- **With `DEBUG`:** `sys/lock.h` declares `_simple_lock` and its
  fellows. `kern_lock.c` defines them only under
  `#if defined(DEBUG) && NCPUS == 1` (line 442). A `DEBUG` kernel with
  `NCPUS > 1` would not link.
- **`NCPUS` is set to 1** in `i386/include/param.h:44`.
- **`lockmgr`'s spin (`PAUSE`, `kern_lock.c:56-77`)** busy-reads the
  lock with no barrier. Under C17 that read needs an atomic load, or the
  compiler may hoist it out of the loop.

The same missing semicolon is in the hp300, luna68k, mips, news3400,
pmax and sparc `param.h` files (found by the fact-check), so no Lite2
port can have built this path.

So "turn on the locks Lite2 already has" is right in direction, but its
first step is to make the multiprocessor path compile at all. The size
of the rot is unknown until that is tried (section 6, step 1).

### 2.3 Counts, and their scopes

The notes do not state how they counted. Two of their figures
reproduce once header files are included: `lockmgr` 25 (`.c` and `.h`),
and the Mach note's 23 uses of the four Mach wait names (14 calls in
`.c` files, 5 macro uses in headers, 4 definitions in `vm_glue.c`). The
others did not reproduce under any scope tried. Counts here are calls
or uses in `.c` files:

| Claim in the note | Counted here |
|---|---|
| `lockmgr` 25 calls | 19 in the whole kernel; 16 in `kern`, `ufs` and `vm` |
| `VOP_LOCK`/`vn_lock` about 240 | 110 in the whole kernel |
| `spl` 762 calls | 2,037 in the whole kernel; 343 in `kern`, `ufs` and `vm` |
| `curproc` 208 uses | 344 in the whole kernel; 117 in `kern`, `ufs` and `vm` |
| `cnt` 106 uses | 144 `cnt.` uses in the whole kernel |
| about 250 `simple_lock` calls | 100 `simple_lock`, 108 `simple_unlock` and 1 `simple_lock_try` in `kern`, `ufs` and `vm` |
| about 120 "busy, then sleep" sites | 126 `tsleep` calls in the whole kernel. This is not an upper bound: the VM's `PG_BUSY` sites sleep through `PAGE_ASSERT_WAIT` and `thread_block`, not `tsleep` |
| (Mach note) the VM uses Mach's wait calls 23 times | 20 occurrences of the four names in `.c` files, 2 of them in comments; 23 with the header macros, as above |

None of this changes a conclusion. **Planning should not use these
numbers until they are recounted with the scope written down.**

### 2.4 The roadmap already reviewed Mach, and reached the opposite verdict

`roadmap-4.6.md`, *Kernel primitives*, "And Mach, looked at
(2026-10-18)", read three Mach lines (CMU's MK83A, where the Mach note
read MK84; Utah's Mach 4; OSF MK as DR3) and concluded: "None fits better than OpenBSD's line". Its reasons:

- MK83's and Mach 4's `lock_t` is what `lockmgr` replaced, "without its
  wait message, priority, timeout or signals";
- "OSFMK's mutex sleeps where OpenBSD's mtx spins at an spl";
- OSF MK's `lock_t` "lost the recursion vnode locking needs";
- "everything in it rests on Mach threads, `assert_wait` and
  `thread_block`'s continuations, which Lite2's processes and u-area
  cannot carry".

It kept OSF MK's lock debugging as worth having, and ddb, which it
credits as "CMU Mach's code, taken from NetBSD or OpenBSD".

**The Mach note does not cite this review or answer it.** The two can
be reconciled, and section 3 does so. The 2026-10-18 review asked
whether OSF MK's lock *code* could replace OpenBSD's as a whole, and it
cannot. The Mach note is right about several of its *designs*, and two
of those are already shapes Lite2 has. Taken as designs, or as small
pieces of code, they survive every objection the roadmap made.

### 2.5 When NetBSD's interlocked sleep arrived

The SMP note marks NetBSD's `ltsleep` "(1999) ... not re-checked".
Checked: it is **not** in NetBSD 1.4. It **is** in NetBSD 1.5's
`kern/kern_synch.c` (rev 1.78.2.5, 2000), under "Copyright (c) 1999,
2000 The NetBSD Foundation" beside the Regents' notice. Its comment says
the interlock "is held until the scheduler_slock is held", and the
`PNORELOCK` flag is there. So it is an spl-era BSD continuation of
`tsleep` itself. (The Foundation notice is the file's, not `ltsleep`'s:
the netbsd-1-4 branch's `kern_synch.c`, rev 1.57.2.3 of 2000-04-30,
carries the same notice and no `ltsleep`. The donor repository's
history is grafted, so the date `ltsleep` reached NetBSD's trunk was
not found.)

## 3. Lean towards

Each item gives the idea, where it comes from, why it fits the rules,
and its conditions.

### L1. Finish Lite2's own lock design; import no lock family

- **What:** `simple_lock` made real on C17 atomics; `lockmgr` as it is;
  the VFS and VM interlocks used as Lite2 wrote them.
- **Why:** rule 1 at its purest. Lite2 shipped a multiprocessor lock
  design and switched it off. Finishing it is a smaller change, with the
  same semantics, than any donor's lock package. Both notes agree on
  this, and it is their strongest common ground.
- **Condition:** section 2.2. The first patch makes `NCPUS > 1` compile
  with a C17 `atomic_flag` test-and-set, still on one CPU, and boots.
  That one patch shows how much rot there is before anything is
  designed on top of it.

### L2. An interlocked sleep in `tsleep`'s own form; the VM keeps its Mach names on top

Both notes want a sleep that releases a lock without losing a wakeup.
They differ on the form:

- **The SMP note:** an interlocked `tsleep`, after NetBSD's `ltsleep`.
- **The Mach note:** Mach's two-phase wait made real. `assert_wait`
  queues the thread, the lock is released, `thread_block` gives up the
  processor. The "busy, then sleep" sites use the one-call macro
  `thread_sleep_simple_lock`, and `tsleep` is rebuilt as `assert_wait`,
  a timeout and `thread_block`, "its interface and its callers" not
  changing.

At a call site the two look alike: one call that sleeps and releases a
lock. The difference is underneath. **Lean: `tsleep` kept as it is, with
an interlock added, in NetBSD 1.5's `ltsleep` form; Lite2's VM names
implemented on top of it in `vm_glue.c`.** The reasons, against rule 1a
and the precedent rule:

- **Smallest change, and no body written.** `ltsleep` is `tsleep` with
  one more argument, and NetBSD 1.5 has its body, a BSD continuation of
  `tsleep` that can be imported. The Mach form rebuilds `tsleep`'s body
  on new primitives. No donor has that `tsleep`, so it would be our own
  writing of a central function, which `precedent.md` rules out.
- **Same semantics as 4.4.** `tsleep` carries a wake priority, `PCATCH`
  (signals, `EINTR` and `ERESTART`), a timeout and a wait message. Mach's
  `assert_wait(event, interruptible)` carries the counterpart of
  `PCATCH` and nothing else. Rebuilding `tsleep` on it means keeping the
  priority, timeout and message somewhere new, which is where a quiet
  change in behaviour would hide. `ltsleep` keeps them where they are.
- **It covers Lite2's Mach shape.** Lite2's own
  `thread_sleep(event, lock, ruptible)` *is* an interlocked sleep. It
  becomes `ltsleep(event, PVM | PNORELOCK, "thrd_sleep", 0, lock)`.
  **`PNORELOCK` is required:** `ltsleep` otherwise takes the lock back
  before returning, and Lite2's callers retake it themselves
  (`vm_pageout.c`, the scan loop; the `VM_WAIT` macro in
  `vm_pageout.h`), so without it they would deadlock once locks are
  real. (An earlier draft of this note omitted the flag; the fact-check
  found it.) `assert_wait` and `thread_block`, which the VM uses in a
  handful of places, stay in `vm_glue.c` on the real queues.
- **Source order.** BSD code comes before Mach. Best fit could still
  choose Mach, but here Mach does not fit better.
- **Condition:** NetBSD 1.5's `kern_synch.c` needs its ledger row first.
  `ltsleep` assumes one global `scheduler_slock`, a big-lock-era shape.
  With per-CPU run queues (L3), the interlock is released once the sleep
  queue's own lock is held, which is the SMP note's "a lock per queue".

**What is kept from the Mach note here is its reason**: release the
lock only when the wakeup can no longer be missed. That is its best
insight, and `ltsleep` is that insight in `tsleep`'s form.

### L3. The per-CPU structure, with Lite2's names kept by macros

- **What:** OpenBSD's `struct cpu_info` and `curcpu()`, with Lite2's
  globals moved to where OpenBSD keeps them and their names kept by
  macros:
  - `curproc` becomes `ci_curproc`;
  - `cpl` becomes the per-CPU `ci_ilevel`;
  - `curpriority` goes into the per-CPU scheduler state
    (`spc_curpriority`);
  - `astpending` becomes per process, not per CPU
    (`p->p_md.md_astpending`, set by `aston(p)`).
  `need_resched` marks the right CPU, and `aston` the right process;
  if that process runs on another CPU, that CPU is sent an
  interprocessor interrupt.
- **Why:** it is the smallest change that makes per-CPU state possible,
  and every line of code that says `curproc` stays as it is. OpenBSD
  does exactly this today: `ci_curproc`, with `curproc` as a macro.
- **Fits with L10.** In OpenBSD's split, `struct proc` is the thread
  ("List of all threads") and `struct process` is the process. So
  `curproc` stays the right name after threads arrive, and this work
  is not done twice.

### L4. Spin locks for data shared with interrupt handlers: OpenBSD's `mtx` at an IPL

The Mach note's best new distinction is the job of OSF MK's
`usimple_lock`, separate from `simple_lock`:

- `usimple_lock` guards data that an interrupt handler also touches;
- `simple_lock` guards data shared only between CPUs.

**Lean: give that job to OpenBSD's `mtx`, initialised with its
interrupt level (`mtx_init(&m, IPL_NET)`), not to a new `usimple_lock`.**

- **`design-4.6.md` 6 has already chosen it.** `mtx` is Tier C. Of the
  five OpenBSD drivers that section sampled, three call `mtx_enter`
  today (`ahci` 4 times, `nvme` 4, `em` 2) and two do not (`com`,
  `xhci`); at the 2014 plateau there are likely fewer. A different name
  would mean rewriting those calls in each imported driver, or a shim. Both fail "fewest foreign dependencies",
  and a shim breaks the maintainer's no-translation-layer preference.
- **The level is part of the lock.** With `usimple_lock`, each caller
  must remember to raise the right `spl` first. With `mtx`, the lock
  raises it. That is harder to get wrong, and it keeps Lite2's `spl`
  meaning ("these interrupts are blocked on this CPU").
- **It is the BSD continuation.** `mtx` came with OpenBSD's SMP in 2004,
  and OpenBSD continues 4.4 to about 2014
  (`analysis-4.6-continuations.md` 4).

So the Mach note's distinction is kept, under OpenBSD's name and with
the level built in.

### L5. Tokens, for process-context subsystems, as the SMP note frames them

**This is the best idea in either note.** A token is Lite2's own rule,
"run to completion or a sleep", kept for one subsystem on many CPUs:

- held only while the thread runs;
- given up in `mi_switch`, exactly where Lite2 already checks for held
  locks;
- taken back in `tsleep` before it returns.

Code written to the 4.4 rule needs no audit for "what might sleep under
a lock", because a sleep releases the token, as it did on one CPU. A
thread blocked on a `lockmgr` lock holds no tokens, so tokens and
`lockmgr` cannot deadlock against each other.

**Lean, with two conditions the note does not state:**

1. **Tokens are for process context only.** An interrupt handler that is
   not a thread cannot block. If it spun on a token, it could deadlock
   against the holder it interrupted on the same CPU, or wait out a
   long hold. DragonFly takes tokens only in interrupt *threads*. So any subsystem whose data an interrupt touches
   needs a second answer:
   - the network's soft interrupt, `ipintr` and the socket buffers it
     fills;
   - `biodone` and the buffer queues;
   - the tty input path.

   The answer is either an `mtx` at the right level around that data
   (L4), or moving that path into a kernel thread (L9), which can wait
   for a token. **This must be decided per subsystem before that
   subsystem gets a token.** Sockets are on the note's token list, and
   `ipintr` appends to socket buffers at `splnet`, so the network is the
   first place it bites.
2. **"No big lock" should be said honestly.** One token per subsystem
   is a coarse lock, a "medium lock". Subsystems in 4.4 are porous:
   - `namei` runs through the VFS, the buffer cache and a driver;
   - a Unix-domain socket binds to a vnode;
   - `exit` touches the process lists, the VM, the VFS and the tty.

   Threads will often hold several tokens at once. DragonFly's rule, all
   released on block and all retaken on wake, avoids deadlock but can
   bounce between contending threads. The token list per subsystem
   should be drawn with that in mind, and contention measured
   (section 6, step 7).

### L6. Lock debugging per CPU, as OSF MK keeps it

- **What:** a per-CPU count of held spin locks (OSF MK's
  `simple_lock_count`), with a "none held" assertion before a thread
  blocks. This is Lite2's `p_simple_locks` check moved from the process
  to the CPU, where it belongs once a CPU can hold a lock its process is
  not running under. Also OSF MK's per-processor stack of held locks,
  and a spin that times out into the debugger.
- **Why:** the 2026-10-18 roadmap review recommended the same thing.
  It is debugging, not semantics, so it changes nothing a program sees.
  Lite2 already has the hook.
- **As code or as idea:** as an idea written in Lite2's names. The OSF
  code carries ETAP and Mach-thread assumptions (D4 below).

### L7. Preemption as a per-CPU level, switched on last

- **What:** OSF MK's `preemption_level` per CPU, raised by any spin lock
  and by any raised `spl`, with preemption taken at return from an
  interrupt when the level is zero and a reschedule is pending. This is
  the rule both notes give.
- **The condition neither note states as a rule:** preemption ends
  Lite2's "run to completion or a sleep" on one CPU as much as SMP does
  on many. Two processes in the kernel on the same CPU then interleave
  anywhere. **Every shared structure touched from process context must
  be under a token or a lock before preemption is switched on.** The
  SMP note's order puts tokens at step 4 and preemption at step 6, which
  respects this. It should be a written rule, with a check, not a
  consequence of the order.
- **The precedent is thin.** MK84 has no preemption. OSF MK's is the
  `MACH_RT` configuration, real-time work under 148 conditionals, and
  its production use is "not checked" by the note itself. OpenBSD, the
  driver plateau, has no kernel preemption. DragonFly's preempts only
  into interrupt threads, and is a redesign.
- **This collides with the precedent rule.** "This project no longer
  writes function bodies at all." Preemption at interrupt return, the
  level checks and the switch path have no donor that continues 4.4.
  Either a donor is found and measured (OSF MK's `MACH_RT` code is the
  nearest candidate), or the maintainer makes an explicit exception.
  It should not happen by drift (section 7).

### L8. OSF MK's TLB shootdown algorithm, taken as a piece

- **What:** the shootdown of Black, Rashid, Golub and Hill (1989) as
  OSF MK's `intel/pmap.c` implements it:
  - each pmap records which CPUs are using it;
  - a CPU with interrupts blocked leaves the active set and is brought
    up to date when it returns;
  - only the CPUs that need it are signalled.

  Also its written lock rule: never more than one pmap lock and one
  pv-list lock at a time.
- **Why it beats own writing:** `analysis-4.6-continuations.md` 11,
  item 2, expected the multiprocessor pmap to be our own writing. OSF
  MK's is written to the pmap interface Lite2 has, from the Mach line
  Lite2 descends from, under a permissive notice. That fits "no original
  works" better than writing one.
- **Why only a piece, not the whole pmap:** see D7.

### L9. Kernel threads before anything that needs them

FreeBSD 4.0's `kern_kthread.c` (144 lines, `roadmap-4.6.md`) is the
measured candidate. L5's per-subsystem answer, per-CPU work, and any
interrupt path moved out of interrupt context all need it. Small, a
continuation of Lite2's `fork1` and processes, and early.

### L10. Decide threads and SMP together, on OpenBSD's split

`design-4.6.md` 9 already says to decide them together. The suite
report shows why it matters (L11), and the SMP note's own plan needs it:

- tokens are a list "in `struct proc`";
- the per-CPU structure names the running `struct proc`.

**Lean: OpenBSD's rthreads split, `struct proc` the thread and
`struct process` the process** (`plan-4.6.md` 3.3 already names it).
Its advantage for SMP is that Lite2's `struct proc` stays the unit the
scheduler, `tsleep`, `mi_switch` and the token list work on. So L3, L5
and L7 are written once, before threads arrive, and stay right
afterwards.

### L11. What the suite report adds that the SMP notes miss

The report's own sums check: 1,315 + 1,461 + 565 + 197 + 3,397 + 1,955
= 8,890 assertions; 1,040 modules; 25,443 requirements. Its kernel
total of 31% and its 40% for the kernel and what rests on it also
check. (`design-4.6.md` 4 says "about 8,600"; the suite has grown.)

Four things in it bear on SMP and threads:

1. **Process-wide stop.** The report lists it among the BSDs' wrong
   behaviours. It is "single-threading": stopping every other thread of
   a process for `exec`, `exit`, a core dump or a stop signal. Under SMP
   that is an interprocessor request to each CPU running one of them,
   and a wait until each has stopped. **Neither SMP note mentions it.**
   It should be designed with L3, L10 and the interprocessor interrupts,
   not after them.
2. **CPU-time clocks** (`CLOCK_PROCESS_CPUTIME_ID`,
   `CLOCK_THREAD_CPUTIME_ID`). These need runtime kept per thread at
   each switch. Lite2 keeps `p_rtime` in `mi_switch` using `microtime`,
   so they depend on a correct per-CPU `microtime`. 4.5BSD's stage-4
   `microtime` work (#686) is the first link in that chain.
3. **`ps`, `fuser` and `ipcs` need a view of process and IPC state.**
   `guide-4.6.md` 7 keeps the kvm-based `ps`. kvm reads kernel
   structures directly, so the thread and per-CPU layout fixes what
   `libkvm` must read. Choose them together, as the report says.
4. **The harness needs the kernel first.** `pcexec` uses `fork`,
   `execve`, `setsid`, `setpgid`, `kill`, `alarm`, `sigaction`,
   `waitpid`, `setrlimit`, the `setuid` family, `pipe`, `dup2`, `umask`,
   `mkdtemp` and `clock_gettime`. A search of this tree finds the
   first group, but no `clock_gettime` and no `mkdtemp` (nor
   `posix_spawn`, `waitid` or `pthread_create`). `plan-4.6.md` puts the
   harness in stage 2 and `clock_gettime` at stage 3.4. **So either
   `clock_gettime` (with `CLOCK_REALTIME` and `CLOCK_MONOTONIC`) and
   `mkdtemp` move ahead of the harness, or the harness gets a stated,
   temporary substitute.** This is the most immediately actionable
   finding in the three documents.

The report's largest fact is not about the kernel at all: **about 60%
of assertions test libc, headers, the shell and utilities.** That
supports `design-4.6.md` 9's "cheapest and widest first", and it means
the score will move most before SMP is touched. SMP serves POSIX by
speed (rule 2). It is not on the critical path to conformance, and
threads are.

### L12. The scalable commutativity rule, once SMP runs

`guide-4.6.md` 5 already names Clements's rule and the Commuter tool as
a test method beside the suite. Once tokens are in, it is the way to
find where the implementation serialises where POSIX does not require
it. That is the measurement section 6, step 7 needs.

## 4. Drop

### D1. OSF MK's sleeping `mutex`

`lockmgr` and tokens cover its uses in 4.4 code, and OpenBSD's `mtx`
(L4) covers spinning. A third kind of lock adds a choice at every call
site and gains nothing. The 2026-10-18 review reached the same
conclusion.

### D2. `usimple_lock` as a separate name and discipline

Its job is kept as `mtx` at an IPL (L4), which the drivers 4.6BSD
imports already call.

### D3. Mach's thread model: continuations, `thread_act`, tasks, IPC

`thread_block(continuation)` lets a Mach thread give up its kernel
stack. A Lite2 process with its u-area cannot do that, as the 2026-10-18
review found. The two-phase wait needs no continuation (OSF's own macro
passes 0), and L2 keeps only its reason. Both notes already put ports,
tasks, `thread_act` and RPC outside the plan, rightly.

### D4. OSF MK's lock instrumentation (ETAP) and its shared run queue per processor set

The Mach note drops both, rightly. ETAP is large and optional; L6
keeps its useful core as an idea. Mach's main run queue is shared per
processor set, with only bound threads on per-processor queues. That is
less per-CPU than OpenBSD's 2009 form of the 4.4 scheduler, which is
the continuation (`analysis-4.6-continuations.md` 4). Mach's hand-off
of the next thread to an idle processor can be kept as an idea beside
OpenBSD's queues.

### D5. Spreading `assert_wait`/`thread_block` to the BSD sleep sites

That is the part of the Mach note's section 3 to drop, for L2's
reasons:

- it changes the 4.4 idiom at every "busy, then sleep" site;
- it moves `tsleep`'s priority, signal and timeout handling somewhere
  new;
- `ltsleep` gives the same guarantee in one call.

The Mach names stay where Lite2 already has them, in the VM.

### D6. Moving every interrupt into a thread, and making `spl` a no-op

This is FreeBSD 5's SMPng (BSDi's design, which FreeBSD took) and
DragonFly's model. `analysis-4.6-continuations.md` 4 rates both FreeBSD 5
and DragonFly as redesigns, and they end Lite2's
`spl` meaning. A kernel thread for a particular path that must wait for
a token (L5, condition 1) is different. That is one path moved, and it
keeps `spl` everywhere else.

### D7. Deciding the i386 pmap before the PAE question is settled

The Mach note proposes OSF MK's whole `intel/pmap.c` as the i386
multiprocessor pmap. Whether that is right depends on a decision not yet
taken:

- **The hardware no-execute bit exists only in PAE mode on the i386,**
  whose page tables are a different format (OpenBSD's `pmapae.c`:
  `PG_NX` is bit 63). OSF MK's pmap has no PAE.
- **But W^X does not need PAE.** OpenBSD's non-PAE i386 `pmap.c`
  enforces execute limits through the code-segment limit (`pm_hiexec`,
  `pmap_exec_fixup`). That was OpenBSD's i386 W^X before PAE.
  `guide-4.6.md` 4 says the same: "NX by honouring `VM_PROT_EXECUTE` in
  the pmap". (An earlier draft of this note said no-execute needed PAE
  outright; the fact-check corrected it.)
- **PAE is also what gives the i386 more than 4 GB.** `plan-4.6.md` 5.2
  lists `paddr_t`, PAE, the no-execute bit, W^X and ASLR together,
  without making one depend on another.

So: **if 4.6BSD's i386 is to have PAE,** importing OSF MK's whole pmap
means rewriting its page-table code soon after, the same work done
twice, and only L8's pieces should be taken. **If it is not,** the whole
pmap is a fair candidate, measured against FreeBSD 3.x, NetBSD and
OpenBSD as rule 1a requires. (The Mach note's own "Not checked" list
admits it has not measured how much of the 3,943 lines is Paragon,
memory-object or `pmap_attribute` code.) What to drop is choosing the
file before that question is answered.

**A related question, not a correction.** `analysis-4.6-continuations.md`
11, item 2 rules the amd64, multiprocessor and PAE pmaps "own writing,
kind C". They are written for Lite2's pmap interface by following
OpenBSD's and NetBSD's as the reference, because every UVM call in a
donor's pmap must be rewritten. That ruling predates the precedent
rule's "no longer writes function bodies at all". Whether a donor's pmap
with each VM call rewritten is an adaptation (kind B or D) or own
writing (kind C) is now a question for the maintainer (section 7).

### D8. The notes' size estimates and counts, as planning numbers

The SMP note's "1,500 to 3,000 lines" counts edits at known points. It
leaves out the real cost, which is reading each of the `spl` and
`tsleep` sites to learn what it protects. Its counts could not be
reproduced (section 2.3). Keep both notes as design arguments. Measure
again, with the method written down, before anything is scheduled from
their numbers.

### D9. A gap rather than an error: `spl` and shared data

The SMP note says `spl` "still means 'no interrupts on this CPU', which
is all a per-CPU structure needs". That is right about per-CPU data, and
it is all the note claims there. What the note does not say is what
becomes of the many `spl` calls in Lite2 that protect *shared* data:

- the buffer queues at `splbio`;
- mbufs at `splimp`;
- the tty queues at `spltty`;
- the routing table and protocol control blocks at `splnet`.

On many CPUs, an interrupt handler on another CPU can reach that data
while this CPU's `spl` is raised. So shared data needs `mtx` at the
level (L4), or a token with the interrupt path moved to a thread (L5).
**What to drop is any plan that treats `spl` per CPU as finishing the
job.** The `spl` sites have to be read one by one. (An earlier draft
listed this as an error in the note; the fact-check pointed out the
note does not make that claim.)

## 5. Corrections to the three documents

| Document | Says | Correction |
|---|---|---|
| SMP note 1, 2.1 | Lite2's multiprocessor simple locks exist; "only the test-and-set itself is missing" | They have never compiled on any Lite2 port in either configuration (2.2) |
| SMP note 2.2 | `ltsleep` 1999, not re-checked | in NetBSD 1.5 (2000) and not in 1.4; its trunk date not found (2.5) |
| SMP note and Mach note | counts of calls | two reproduce with headers included; the rest not, as given (2.3) |
| Mach note 1-3, 8 | Mach's primitives are the cleanest fit | does not engage `roadmap-4.6.md`'s 2026-10-18 review, which found the opposite for the code. Reconciled in L2, L4, L6, L8 and D1-D5 |
| Mach note 6, 8 | the i386 MP pmap can be OSF MK's | only if the i386 is to have no PAE; otherwise its algorithm, not the file (D7) |
| Suite report | (nothing found wrong) | every sum checks. Its "Doc", the architecture document on the proc/thread split, is not in this tree |

And corrections to this note's own first draft, found by the
fact-check: `thread_sleep` needs `PNORELOCK` (L2); the Mach note's sites
use one macro, not three statements (L2); no-execute does not need PAE
for W^X (D7); not every imported driver calls `mtx` (L4);
`continuations` 11 is a deliberate ruling, not an omission (D7); the
`spl` point is a gap, not an error (D9); OpenBSD keeps `astpending` per
process (L3).

## 6. An order, if the leans are taken

The SMP note's order (atomics, per-CPU, interlocked sleep, tokens,
processors, preemption, finer locks) is sound. With the leans above it
becomes:

1. **Make `NCPUS > 1` compile on one CPU:** the semicolon, the `DEBUG`
   definitions, a C17 `atomic_flag` test-and-set, an atomic load in
   `PAUSE`. Boot it. This measures the rot (2.2).
2. **The per-CPU structure,** with Lite2's names kept as macros (L3).
3. **`ltsleep`,** sleep-queue locks, and the VM's `thread_sleep` on it
   with `PNORELOCK` (L2). Each "busy, then sleep" site converted as it is read.
4. **Kernel threads (L9).** Threads and the `struct process` split
   decided with this step (L10), including how a process is stopped
   whole (L11, item 1).
5. **`mtx` at an IPL** for data shared with interrupts (L4), and
   tokens for process-context subsystems (L5). Each subsystem's
   interrupt path decided before it gets its token.
6. **Processors started, interprocessor interrupts, per-CPU run queues,
   callouts and clocks, and the shootdown (L8).** Per-CPU callouts need
   `untimeout` to find, and stop, a callout that another CPU may be
   running at that moment.
7. **Lock debugging (L6)** throughout. Contention measured with the
   commutativity method (L12). Subsystems split from one token only
   where the measurement says so.
8. **Preemption last (L7),** after a check that every process-context
   shared structure is under a token or a lock.

And independent of all of it, and earlier: `clock_gettime` and
`mkdtemp` ahead of the conformance harness (L11, item 4).

## 7. For the maintainer

These are the maintainer's to decide, not this review's:

1. **Record the SMP decision** (no big lock, preemptive, tokens) in
   `design-4.6.md` or `plan-4.6.md`. They still say "big-lock SMP
   first". Not changed here, as asked.
2. **The interrupt-context model,** per subsystem: `mtx` at an IPL, or
   the interrupt path moved to a thread (L5, D6).
3. **PAE and no-execute on the i386** (`plan-4.6.md` D7). This decides
   whether OSF MK's pmap is a whole-file candidate or a source of one
   algorithm (D7).
4. **Threads on OpenBSD's split** (L10), decided with SMP, as
   `design-4.6.md` 9 already says.
5. **`clock_gettime` and `mkdtemp` before the harness,** or a stated
   temporary substitute (L11, item 4).
6. **Preemption and the precedent rule** (L7): find and measure a
   donor, or make an explicit exception to "no longer writes function
   bodies".
7. **Donor pmaps with their VM calls rewritten:** an adaptation (kind B
   or D), or own writing (kind C) as `analysis-4.6-continuations.md` 11
   rules (D7).
8. **Whether to link the `nmartin0/osfmk7.3` repository** to this
   workspace, so that OSF MK 7.3 and DR3 can be compared. This review,
   the Mach note and `analysis-4.6-continuations.md` 13 all rest on DR3.

## 8. Not checked

- **OSF MK's and Mach's source.** Every claim about `kern/lock.h`,
  `sched_prim`, `cpu_data` and `intel/pmap.c` is the Mach note's.
- **DragonFly's tokens.** The quoted definition and
  `lwkt_thread.c:1002` are the SMP note's.
- **NetBSD 1.5's `ltsleep` against the clearance rules.** Its notice
  was read; nothing else was.
- **The three notes the SMP and Mach notes cite** (`percpu`,
  `vm-buildout`, `radixvm-design`); not in this tree.
- **Nothing was built or run.** Section 6, step 1 is the first thing
  that would test any of this.

## Afterwards: decided (2026-11-01)

The maintainer asked for a reality check on the leans above. On a closer
reading, L5's per-subsystem tokens are not the canonical path: every BSD
big lock is already one token (released on sleep, retaken on wake;
checked in OpenBSD 2004 and NetBSD 2.0, and DragonFly renamed its own
`mp_token` in 2010), per-subsystem tokens are DragonFly's and Apple's,
not 4.4's, and Apple's paper reports its split went badly. Lite2's own
code points to its spin locks, `lockmgr` and interlocks. Preemption: 4.4
never intended it. The maintainer decided: no preemption; one kernel lock
as an interim, released on sleep; Lite2's own locks made real underneath
and the conversion path by them; rthreads. Recorded in `plan-4.6.md`.

