# 0x40df20 / 0x40df90 — the subsystem's mutex-guarded table scan

Decoded 2026-08-17, not yet written. Callers are @0x40e6e9, 0x40e70e,
0x40e72b, 0x40e73b — all inside EzSubsystem's own range, and the lock is at
`this+0xa008`, which is exactly `SubsystemBackend::lockHandle`. So these are
`SubsystemBackend` methods.

## 0x40df20 — find the next occupied slot after `*cur`

`__thiscall`, one stack argument (`ret 4`), the argument is an `int *`.

    lock(this->lockHandle)
    i = *cur + 1
    r = 0
    if (i < 0x800) {
        e = &entries[i]
        do { r = e->f0; if (r) { *cur = i; break; } i++; e++; } while (i < 0x800)
    }
    unlock(this->lockHandle)
    return r

The entry stride is **0x14** and the table starts at `this+4`, 0x800 entries —
so it spans 4 .. 0xa004, which lands `fa004` and `lockHandle` exactly where
the existing model already puts them. The current `int buf[0x2800]` plus
`fa000` covers the same bytes without describing them; **carving the table out
is a layout-neutral refinement, but check every user of `buf` first** — the
cookbook's rule about adding fields to a verified struct applies.

The unlock is DUPLICATED: the loop-exhausted path at 0x40df63 and the
hit/skip path at 0x40df79 each have their own copy of
`unlock; pop edi; mov eax,esi; pop esi; ret 4`. Expect the duplicate-tail
treatment from CLAUDE.md if a single `return` does not reproduce it.

## The blocker: two imports this dump cannot resolve

The lock and unlock are `call dword ptr ds:0x48709c` and `...0x4870a0`, and
**both IAT slots hold RVAs outside the dumped file** — 0x33e3208 and
0x33e321e against a 0xb7000-byte image. The packer did not rebuild these two,
so the names cannot be read out of the file the way every other import can.
This is the first unresolvable IAT slot found in this tree; other Win32 calls
resolve normally, so it is specific to these slots rather than general.

What the evidence still supports: the two name RVAs are **0x16 apart**, and
0x16 = 2 (hint) + 19 (name) + 1 (nul). `WaitForSingleObject` is exactly 19
characters, and the call is `push -1; push handle` — the INFINITE wait — so
the first slot is almost certainly it. The second takes the handle alone and
runs immediately after the guarded region, which fits `ReleaseMutex`, but the
gap gives no length for the second name so that one is a guess.

**Settle it empirically rather than by reasoning**: reccmp resolves Win32 IAT
calls automatically, so writing the function against `WaitForSingleObject` and
`ReleaseMutex` and reading the diff will confirm or refute both in one build.

## 0x40df90

Not yet disassembled — same caller set, presumably the companion (first slot,
or release-by-value). Do it alongside.
