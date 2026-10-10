# AudienceRatePanel::drawFull @0x419a70 - decode (2026-08-17)

964 bytes, one of three identically-sized spine stubs (0x419a70, 0x419e70,
0x41a270 - almost certainly the same readout on the three panels). Pure
draw-call table: fifteen `drawTex` calls, no register pressure of note, which
is the shape that lands first build.

## Shape

    setBlend(2, 2)

    v = f24                       ; the live value
    y1 = f28 + K(0x48ddf4)
    if (v >= 1000) drawTex(d298[v/1000%10], 269, y1, 0, 0, -1)
                   drawTex(d298[v/100 %10], 282, y1, 0, 0, -1)
                   drawTex(f2c4,            291, y1, 0, 0, -1)   ; separator
                   drawTex(d298[v/10  %10], 300, y1, 0, 0, -1)
                   drawTex(d298[v     %10], 313, y1, 0, 0, -1)

    w = f1c                       ; the second value
    y2 = f28 + K(0x48ddf0)
    if (w < v)                    ; 0x419be3 jge -> the else
        same five with d298 and f2c0 as the separator, colour -1
    else                          ; 0x419d0c
        same five with d248 (a SECOND digit set) and f2c0, colour in eax

Only the thousands digit is gated (`>= 1000`); the other four always draw. The
separator between the hundreds and the tens makes this a rate - hundreds point
tens units - which matches the class name.

## Still to read

Row 3's colour: the calls push `eax`/`edx` where rows 1 and 2 push -1. Read
0x419d0c..0x419d40 for where it is computed.

## Fields

    f1c   the second value            f24   the live value
    f28   the readout's y             f248  digit set B [10]
    f298  digit set A [10]            f2c0  separator for rows 2/3
    f2c4  separator for row 1

## Magic divisors (write the division, cl regenerates these)

    0x10624dd3 sar 6  -> /1000        0x51eb851f sar 5 -> /100
    0x66666667 sar 2  -> /10

## x positions

    269.0 (0x43868000), 282.0 (0x438d0000), 291.0 (0x43918000),
    300.0 (0x43960000), 313.0 (0x439c8000)

## drawSolo @0x419e70 - DONE (100%)

Byte for byte drawFull with the five columns at 311, 324, 333, 342, 355 -
every value, digit set and tint rule identical, x shifted 42 right.

## drawIdle @0x41a270 - NOT the same shape, and BIGGER than the scan says

scan.py reports 964 bytes because the next annotated address is 0x41a640,
but the code runs past it: at 0x41a66e a THIRD number starts, `f20` - the
other side's rate, the one include/panels.h calls "only in a two-up game".
So drawIdle draws f24, then f1c, then f20, each with a two-arm tint pick.

Its geometry differs too - the rows do NOT share one x set:

    row 1 (y + 366):  277, 294, 305, 316, 333
    rows 2/3 (y + 402): 265, 276, 283, 290, 301

Find its real end before writing it (look for the `ret` past 0x41a79d), and
expect three blocks of the drawFull shape rather than two.

## Skipped: 0x432930 (2026-08-17)

390 bytes, 101 instructions, 4 branches - the last game-side candidate in the
branch-density ranking, and characterised but NOT attempted. It is a draw with
two nested picks (an argument against 0x15, then a byte flag) choosing between
the textures at this+0xc0400 and +0xc0404 and between the colours -1,
0xff46e1ff and 0xdcdcdcdc, each pass preceded by its own setBlend - 9/6 for
the additive one, 2/2 for the rest. The two coordinates are arguments plus
0.5f, and the quads are all 64 by 32.

Why skipped: the pushes interleave with `sub esp,8` + movss float stores
across the branches, so scratchpad's push-collector mis-attributes them, and
hand-mapping a four-branch draw has been the session's worst-value work. Use
scratchpad/frame.py on it first if it is picked up.
