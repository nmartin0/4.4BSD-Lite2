<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# `test` gets three-argument `-a`/`-o` wrong

plan-4.5.md stage 2, item 5; the sweep's #504 (`audit-bestfit.md`
item 9).

## The defect

`usr/src/bin/test/test.c`, `main()`, the three-argument case
(`% test arg1 op arg2`):

```c
case 3:
	if (IS_BANG(argv[1])) {
		...
	} else {
		ret_val = posix_binary_op(&argv[1]);
		if (ret_val >= 0)
			return (ret_val);
	}
	break;
```

`posix_binary_op()` looks the middle argument up in `binary_op[]`,
which lists `-a` and `-o` alongside the real two-operand comparisons
(`=`, `-eq`, and so on). For those, `op_argflag[op]` is 0 -- neither
`OP_INT` nor `OP_STRING` -- so the function's `if (c == OP_INT) {...}
else {...}` falls into the string branch and stores the two raw,
unevaluated `argv` pointers into `struct value`'s union. `expr_operator()`'s
`OR1`/`AND1` case then does `*sp = *(sp + 1)` -- copying the *second*
operand's union (still holding a string pointer) over the first -- on
the understanding, stated in its own comment, that both operands have
already been evaluated as truth values by the parser. They have not
been: `posix_binary_op()` bypasses the parser entirely for this three-
argument form. `posix_binary_op()` then reads `v[0].u.num` back out of
that same union and compares it to zero -- reading the second
argument's pointer as if it were an integer result.

A real pointer is essentially never zero, so `v[0].u.num == 0` is
false, `posix_binary_op()` returns false (0 in test(1)'s own sense,
i.e. *success*), and `test arg1 -a arg2` / `test arg1 -o arg2` returns
true for every three-argument `-a`/`-o` expression, regardless of
whether `arg1` or `arg2` is empty -- confirmed directly (see below):
`test "" -a ""` is wrongly true; so is `test "" -a "x"`, `test "x" -a
""`, and `test "" -o ""`.

## The fit (first canonical fit) and the alternatives

NetBSD's and OpenBSD's `test(1)` (checked at OpenBSD's own 1995-10-18
import, which came by way of NetBSD) is a different implementation
entirely -- a lexer/parser (`t_lex()`, `oexpr()`/`aexpr()`/`binop()`)
descended from the AT&T/System V `test`, not this tree's
`posix_unary_op()`/`posix_binary_op()` structure. Its own three-
argument special case (`argc == 4` there, since it does not decrement
`argc` the way this tree does) only takes the shortcut when the lexed
middle token's type is `BINOP`; `-a`/`-o` lex to a different token
type there, so they are excluded from the shortcut by construction,
not by a later fix. There is nothing to import from either donor:
they were never exposed to this particular bug, having a different
design.

FreeBSD shares this tree's exact structure (confirmed:
`posix_binary_op`, `op_argflag`, `andor_op` are all present in
FreeBSD's `bin/test/test.c` too) and carries the same bug, introduced
with the 4.3BSD-Reno/4.4BSD-era rewrite this tree also has. FreeBSD's
fix, commit `9d633f68d7` (Steve Price, 1996-12-25, "Fix handling of -o
and -a operators in the 3 argument case", submitted by Tom Rush):

```diff
-		} else {
+		} else if (lookup_op(argv[2], andor_op) < 0) {
 			ret_val = posix_binary_op(&argv[1]);
```

one line, excluding `-a`/`-o` from the three-argument shortcut so
execution falls through to the general operator-precedence parser
below, which already handles `-a`/`-o` correctly (with short-circuit
evaluation, per the comment just above that parser). This is the same
shape as this tree's existing case-4 fix (`andor_op`, cited in
`operators.c`/`operators.h`, FreeBSD's earlier 1994 fix for `! arg1 op
arg2`) -- the two fixes are by different FreeBSD contributors, two
years apart, for the twin three- and four-argument forms of the same
mistake.

## The fix

The same one line, in this tree's naming (identical to FreeBSD's,
since the surrounding code is already identical): `} else if
(lookup_op(argv[2], andor_op) < 0) {`, with an AI-ONLY NOTE explaining
the union-reinterpretation mechanism above.

## The test

`test(1)` is a plain host program with no device or kernel dependency,
so `build/test/test_3arg.sh` builds it with the host's own `cc` (not
the cross toolchain, not `build/kernel.sh`) and runs it directly,
checking the exit status of all eight three-argument `-a`/`-o`
combinations of an empty and a non-empty string, plus three ordinary
binary comparisons (`=`, `-eq`) to confirm the fix's `lookup_op` guard
doesn't disturb cases it isn't meant to touch.

| expression | pre-fix | fixed |
|---|---|---|
| `"" -a ""` | true (wrong) | false |
| `"" -a "x"` | true (wrong) | false |
| `"x" -a ""` | true (wrong) | false |
| `"x" -a "y"` | true | true |
| `"" -o ""` | true (wrong) | false |
| `"" -o "x"` | true | true |
| `"x" -o ""` | true | true |
| `"x" -o "y"` | true | true |
| `abc = abc` | true | true (unaffected) |
| `abc = xyz` | false | false (unaffected) |
| `1 -eq 1` | true | true (unaffected) |

Run against the pre-fix binary (`git show HEAD:usr/src/bin/test/test.c`,
built the same way), all four cases that should be false come back
true; against the fixed binary, all eleven match `test_3arg.want`
exactly. So the test fails without the fix and passes with it.

Regression: the cross build (`cd usr/src/bin/test && sh
../../../../build/make.sh` -- `build/make.sh` builds from the current
directory, as its own header documents) compiles clean, no new
warnings; the three "unaffected" cases above confirm ordinary
comparisons still take the same `posix_binary_op()` path as before.

## What is not established

- The four-argument `! arg1 op arg2` case already carries its own,
  earlier `andor_op` guard (FreeBSD 1994, already in this tree before
  this patch); not re-examined here beyond confirming it still
  compiles and the existing cited AI-ONLY NOTE for it is accurate.
- Parenthesized or longer expressions using `-a`/`-o` (five or more
  arguments) go through the general parser unconditionally already and
  were not affected by this bug; not separately re-tested here since
  they never called `posix_binary_op()`.
- Real NetBSD/OpenBSD history was checked only at one point each (the
  1995-10-18 OpenBSD import); not traced further, since the design
  difference found there is structural, not a matter of a later fix
  one might have missed.
