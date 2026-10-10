# `.gds` — the mode descriptor, and where lane order actually comes from

Read out of the binary 2026-08-09, chasing the last open item in the port's
"must be exact" column: *which chart track sits in which visual lane?*

**The answer is that it is not code at all.** Lane order is data, one plain-text
`.gds` file per mode, and the game reads it at load time. Nothing has to be
reverse-engineered to get it right — only parsed.

## The chain

`Judge` and the per-lane input handler index everything by a **lane number**,
and the lane number indexes a per-player `TrackList` at

    director + 0x380 + player * 0x1c4

modelled in `include/players.h` (`count` at +4, `end` at +8, `first` at +0xc,
then `TrackEntry items[]` of 0x10 bytes from +0x10). `items[i].handle` is the
chart track, and `i` is the lane. Three independent confirmations:

* the game-start setup @0x4305a0 walks `items[]` in index order, feeding each
  handle to `judge->m4264d0(handle, 1)` and `chart->find(handle, ...)`;
* the swap helper @0x430500 addresses entries by index (it looks two up by
  *key* — 15 and 23 — and swaps their handles, which is the lane-swap option);
* the two-player sync at 0x421405 either `rep movsl`s a whole 0x1c4-byte list
  onto the next player's, or copies just the handles at `+0x18 + i*0x10`.

`claimTracks` @0x422ca0 and `applyTracks` @0x422bf0 only *consume* the list.
What fills it is:

    int __thiscall loadSlotSection(KParser *p, int player)   ; @0x420f60

    list = this + 0x380 + player*0x1c4
    readInt(&list->count, p)                     ; @0x456270
    list->end = list->count;  list->first = 0
    e = &list->items[0].handle                   ; list + 0x18
    for (i = 0; i < list->count; i++) {
        token(p, 2); token(p, 0xc); token(p, 6)  ; @0x413940 - "TrackN", "=", "{"
        readPair(&e[-2], 2, p)                   ; @0x456670 - Key=a,b
        readInt(e, p)                            ; @0x456270 - SongTrack
        token(p, 7)                              ; "}"
        e[1] = e[0]                              ; fc = handle
        e += 4                                   ; next entry
    }

and its caller is the `.gds` reader @0x421190, a `KParser` loop that dispatches
on section name: `General`, `Slot`*n* (via `strncmp(tok, "Slot", 4)` then
`atoi(rest) - 1` as the player index), `Background`, `Panel`, `Gauge`,
`ShowCredit`, `JudgmentLink`, `InGameSongSelect`.

**The entries are read SEQUENTIALLY.** The `N` in `TrackN` is consumed as a
token and never used as an index, so a file's *order* is the lane order — not
the numbering in the labels.

## The format

Plain text, CRLF, **not encrypted** (unlike `.ez`/`.ezi`/`.ini`). One file per
mode under `system/<mode>/`, 23 of them in Final EX. Brace-structured rather
than INI-flat, which is why the game tokenizes it instead of splitting lines:

    [General]
    NumberOfSlot=2
    ...

    [Slot1]
    NumberOfTrack=7
    Track1 =
    {
        Key=15,16
        SongTrack=10
    }
    Track2 =
    {
        Key=10,-1
        SongTrack=3
    }
    ...

`[Slot1]`/`[Slot2]` are the two players. Per entry:

| file | struct | meaning |
| --- | --- | --- |
| `Key` first value | `TrackEntry.key` | input channel; `-1` for none |
| `Key` second value | `TrackEntry.f4` | a second channel for the same lane |
| `SongTrack` | `TrackEntry.handle` | the chart track this lane plays |
| — | `TrackEntry.fc` | initialised to `handle` |

The two-key entries are the turntable: `Key=15,16` is scratch-up and
scratch-down mapped to one lane. That is also what the swap helper's keys 15
and 23 are — player 1's and player 2's scratch.

**This hands the port the key→lane map for free**, which was listed separately
as a Block 3 problem.

## The lane order itself

`[Slot1]`, in file order, for every shipped mode:

    5keymix, 5RadioMix, ScratchMix, STREETMix,
    STREETMix1st, rubyMix              10 3 4 5 6 7 11
    7StreetMix, RadioMix               10 3 4 5 6 7 11 8 9
    ClubMix, ez2catch, ez2dancer,
    10radiomix                         10 3 4 5 6 7 11 12 13 14 15 16 19
    SpaceMix, 14radiomix               10 3 4 5 6 7 8 9 12 13 14 15 16 17 18 19
    andromedamix                       10 3 4 5 6 7 11 8 9 12 13 20 14 15 16 17 18 19
    7StreetMix_2p                      12 13 20 14 15 16 17 18 19

The shape is consistent: **the scratch (track 10) is lane 0**, then the keys
3..7, then the pedal (11), then the extra ranges. The port's `ez2/mode.c`
derived the correct *sets* empirically from the library but sorted them
ascending, which puts the scratch in the middle of the row and the pedal beside
it — right set, wrong order, on every mode.

`7StreetMix_2p` is a genuinely different layout rather than a reordering, which
is the concrete form of the port's existing note that CV2Mix has no single
layout.

## What the port should do with this

Parse it, do not tabulate it. The file ships with the game the user already
supplies, so reading `system/<mode>/*.gds` at run time keeps the
bring-your-own-exe rule intact and is correct by construction — where a
hardcoded table would be both a copy of the game's data and a thing to keep in
sync. This is PORTING.md 4c ("formats must become parsers, not struct
overlays") applied to a file nobody had noticed was a format.
