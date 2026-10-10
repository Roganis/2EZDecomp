# 0x466da0 — the two-mode judgement dispatcher

Decoded 2026-08-18, not written: no template to copy, three unstubbed
callees, and the owning class is not identified.

## Signature and shape

`__thiscall`, `ret 8` — two stack args: a lane index (ebp) and a mode (eax).

    handle = f1e0->elems[this->f8].items[lane].handle
    st = { -1, -1 }

    if (mode == 2) {
        owner = this->f1ec
        r = owner->f24e0->slot22(handle, 1, 0, &st)
        v = owner->m426700(r)
        this->f1e8->slot32(handle)
        if (v == 0) return
        tailFlag = 1
    } else if (mode == 1) {
        owner = this->f1ec
        r = owner->f24e0->slot22(handle, 1, 0, &st)
        v = owner->m4268c0(r)
        if (v == 0) return
        if (v == 2) this->f1e8->slot33(handle)
        tailFlag = 0
    } else return;

    m466ac0(lane, &st, v, tailFlag);

Note the asymmetry, which looks deliberate rather than accidental: mode 2
calls slot 32 UNCONDITIONALLY and before testing `v`, while mode 1 calls slot
33 only when `v == 2`. So a mode-2 dispatch touches the sink even when it then
returns without dispatching.

## The element addressing needs care

The index arithmetic is the familiar `0x71 * slot` — 0x1c4 bytes — but the
displacement is `+ 0x358 + lane*0x10`, which is past the end of a 0x1c4
element. The only consistent reading is that **`f1e0` points 0x358 BEFORE the
array**: the elements start at `f1e0 + 0x358`, are 0x1c4 apart, and the lane
items are 0x10 apart from the element's own start with the handle at +0.
Model it as a pointer to a struct with `char _pad0[0x358]` and the array after
it, rather than as an array pointer with a bias.

## What blocks it

* **The class.** Its two callers @0x45d51c and 0x45d531 (GF range) reach it as
  `[local+0x130]`, so the owner is not identified either. The fields it uses -
  f8 as a slot id, f1e0/f1e8/f1ec as pointers - resemble PanelSlot, but
  PanelSlot's +0x1ec is a float ("the catch player's life") while this one is
  a pointer to an object with a +0x24e0. Do not assume PanelSlot.
* **Three unstubbed callees**: 0x426700 and 0x4268c0 (both on the f1ec owner,
  taking the slot-22 result) and 0x466ac0 (a four-argument method on this
  class).
* **A sink with slots 22, 32 and 33** (+0x58, +0x80, +0x84) - slot 22 takes
  four arguments and returns something the two judges consume.

Worth doing once the class is pinned; it is on the spine and it is where a
judgement turns into a sink call.
