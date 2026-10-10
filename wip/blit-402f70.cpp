/* KGraphics::m402f70 @0x402f70 - IN-PROGRESS wip map. 2,428 bytes,
   ret 0x14 (five stack args - the m401d80/m4038f0 family signature:
   (KTexture **frames, int count, VideoBlit *d, float xoff, float
   yoff)). Its unannotated GUARD twin sits at ~0x403920 (the m4038f0
   shape, calling here) - reconstruct together.

   THE MAP:
     head    - one-time lazy init gated on bit 0 of 0x530c28: fills the
               vertex block at 0x530ba8 (three-and-a-half quads of
               {x, y, z=1, rhw=0.5, diffuse=-1, spec=0} plus the
               trailing pair at 0x530c20) and sets the init bit.
     4030a7  - bind: m4080c0(0, frames[(int)d->f48]), m4080c0(1, 0),
               m408130(d->f70).
     4030c5+ - the vertex math: fistpl-through-control-word colour
               packing ((int)d->f64 << 8 ...), corners from d->f8/fc/
               f10/f14/f58/f64, ~470 instructions of xmm/x87 mix over
               the vertex block, ending in the draw. 24.5% similar to
               m401d80 - the same family, not an edit.

   DERIVED (tick 2):
   - The block at 0x530ba8 is FOUR 32-byte vertices {x, y, z, rhw,
     diffuse, specular, u, v} - the lazy init writes z=1, rhw=0.5,
     diffuse=-1, spec=0 and the uv corners (0,1),(0,0),(1,1),(1,0);
     bit 0 of 0x530c28 is the done flag.
   - Colour: ((int)d->f64 << 24) | ((int)d->f58 << 16) |
     ((int)d->f5c << 8) | (int)d->f60, each via the fistpl control-word
     block, splatted into all four vertices' diffuse.
   - The frame rect (d->f88 nonzero): x/y scale into the sub-rect
     d->f8c..f98 (lerp the corners by f8/fc/f10/f14).
   - uv flips: u picks d->f28/f2c by comiss (min/max), v picks
     d->f3c/f40, sources f28..f44; the two mirror arms (g_d36cb0)
     write the four corners rotated by d->f54 through the shared trig
     tables (sin negated for negative angles), centred on
     (d->f0 + xoff + ?, d->f4 + yoff + ?).
   - Tail: two device slot-0x50 calls (guarded), then the screen-flip
     pass (every y = H - y off d/g?) and
     device->DrawPrimitiveUP(TRIANGLESTRIP, FVF 0x1c4, 0x530ba8, 4, 1)
     via device slot 0x64.
   Next: transcribe the two mirror arms' corner math and the slot-0x50
   pair, then write src/kgraphicsblit2.cpp. */
