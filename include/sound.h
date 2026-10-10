#ifndef SOUND_H
#define SOUND_H

/* Virtual interface of the object receiving the scaled level. Only the vtable
   layout matters here (never instantiated); setLevel is slot 8 -> vtable+0x20.
   The object behind it is the KSample (Obj40D4E0, vtable @0x488320,
   include/obj40d4e0.h): slot 4 = v4 @0x40d640 PLAY (arg = loop flag; the
   DirectSound buffer's Play(0,0,flags)), slot 5 = v5 @0x40d6a0 STOP AND
   REWIND, slot 6 = put16 @0x40d290 -> buffer SetPan (-10000..10000), slot 8
   = put15 @0x40d2e0 -> buffer SetVolume (-10000..0, hundredths of dB - which
   is why setLevel's argument is `q2 - 5000`). */
struct IVolTarget
{
    virtual void vf00();
    virtual void vf01();
    virtual void vf02();
    virtual void vf03();
    virtual void vf04(int v);  /* slot 4 -> vtable + 0x10 : play(loop) */
    virtual void vf05();       /* slot 5 -> vtable + 0x14 : stop + rewind */
    virtual void setPan(int v);     /* slot 6 -> vtable + 0x18 */
    virtual void vf07();
    virtual void setLevel(int v);   /* slot 8 -> vtable + 0x20 */
};

/* Provisional sound-parameter block passed to the volume/pan setters.
   Only the touched offsets are named. */
struct SoundParam
{
    int         field_0x00;   /* 0x00 : the lane's KIND, written by the stage
                                 setups through SlotClock::setKind - 0 an
                                 auto lane, 1 the player's own, 2 the
                                 player's under autoplay (g_1b5f2e0).
                                 applyEvent's `!= 1` guard and reseedLane's
                                 `== 0` early-out read it that way; a busy
                                 flag would never be 2. */
    char        _pad4[0x0c - 4];
    int         field_0x0c;   /* 0x0c : the bound sample's id (m40fbf0) */
    int         field_0x10;   /* 0x10 : volume value */
    int         field_0x14;   /* 0x14 : pan value */
    int         field_0x18;   /* 0x18 */
    int         field_0x1c;   /* 0x1c */
    int         field_0x20;   /* 0x20 : pan position */
    IVolTarget *field_0x24;   /* 0x24 : target object */
};

/* One queued event, as KEZPlayer::applyEvent @0x40fc50 reads it: a kind at
   +0x04 selecting one of eight arms, and an eight-byte payload at +0x0c that
   each arm reads differently. */
struct SoundEvent
{
    char           _pad0[4];
    unsigned char  kind;      /* 0x04 */
    char           _pad5[0x0c - 5];
    union
    {
        float          fvalue;      /* 0x0c - kinds 5 and 7 */
        unsigned char  bvalue;      /* 0x0c - kind 4 */
        struct                      /* kind 1 reads four fields out of it */
        {
            unsigned short _pad0c;
            unsigned short w0e;     /* 0x0e */
            unsigned char  b10;     /* 0x10 */
            unsigned char  b11;     /* 0x11 */
            unsigned char  b12;     /* 0x12 */
        } n;
    } v;
};

/* (The class owning the volume setter @0x40f4a0, modelled here as Obj40F4A0,
   is KEZPlayer: its this+0x726c falls inside the 0x727c that class's factory
   allocates, and @0x40fc50 drives both setters with the same `this`. Moved to
   include/ksongplayer.h.) */

#endif /* SOUND_H */
