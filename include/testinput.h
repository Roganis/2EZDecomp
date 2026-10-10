#ifndef TESTINPUT_H
#define TESTINPUT_H

#include "panels.h"      /* EzObject, KRuntimeClass, DrawGroup */
#include "ezscreen.h"    /* EzPooled, g_objectRecord */
#include "systemmenu.h"   /* SystemMenu, TestPageSink */

/* One drawn button marker of the input test screen: which switch it watches,
   where on the 640x480 screen it sits, and the two ARGB colours it is drawn
   with (lit and dim - the dim one is the lit one at half alpha with the same
   hue, e.g. 0xffff0000 / 0x80800000). */
struct TestInputItem
{
    int          kind;   /* 0x00 - the input index this marker reports */
    float        x;      /* 0x04 */
    float        y;      /* 0x08 */
    unsigned int lit;    /* 0x0c */
    unsigned int dim;    /* 0x10 */
};

/* TestInput - the "INPUT TEST" page of the operator test menu. Named by its own
   KRuntimeClass @0x4af22c and sized by its object record @0x4af23c: 0x1b8
   bytes, count 2, flags 6.

   Its constructor @0x4754b0 is nothing but twenty appends to the table below,
   which is why 0x1b8 == 0x14 + 21 * sizeof(TestInputItem) exactly. m_count is
   NOT initialised there - the record's pool hands out zeroed storage - and it
   is re-read from memory before every field store, because each store is
   through a pointer that could alias it. */
struct TestInput : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x4753b0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x4753c0 */
    virtual void           update();              /* 0x475bc0 */
    virtual void           update2(int a, int b); /* 0x4753e0 */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    TestInput();                               /* 0x4754b0 */
    ~TestInput();                              /* 0x4753d0 */

    void drawItem(TestInputItem *item);        /* 0x475a90 */


    int  m_count;                              /* 0x10 */
    TestInputItem m_items[21];                 /* 0x14 .. 0x1b8 */

    __forceinline void add(int kind, float x, float y,
                           unsigned int lit, unsigned int dim)
    {
        m_items[m_count].kind = kind;
        m_items[m_count].x = x;
        m_items[m_count].y = y;
        m_items[m_count].lit = lit;
        m_items[m_count].dim = dim;
        m_count++;
    }
};

/* The interface the three test tones are driven through: a 17-slot vtable of
   which the page uses play/stop/setVolume/setPan, a status word whose bit 0
   says "already playing", and one more setter at slot 12. Never instantiated,
   so the unused slots stay declared-only. */
struct TestTone
{
    virtual void q0();
    virtual void q1();
    virtual void q2();
    virtual void q3();
    virtual void play(int a);        /* 0x10 */
    virtual void stop();             /* 0x14 */
    virtual void setVolume(int v);   /* 0x18 */
    virtual void q7();
    virtual void setPan(int v);      /* 0x20 */
    virtual void q9();
    virtual void q10();
    virtual void q11();
    virtual void q12(int a);         /* 0x30 */
    virtual void q13();
    virtual void q14();
    virtual void q15();
    virtual int  status();           /* 0x40 - bit 0 = playing */
    virtual void tick();             /* 0x44 - the pump, as on GFSoundSlots */
};

struct MixerControl;   /* src/mixer.cpp - setValue @0x412a10 */

/* What TestSoundSink::onMenu writes through its OWN `this`: everything the
   page owns, 0x10 lower. */
struct TestSoundSinkFields
{
    char          _pad0[4];
    TestTone     *m_all;          /* sink+0x04 = page+0x14 */
    TestTone     *m_bass;         /* sink+0x08 */
    TestTone     *m_treble;       /* sink+0x0c */
    int           _pad10;
    MixerControl *m_line;         /* sink+0x14 - addControl(0, 2) */
    MixerControl *m_cd;           /* sink+0x18 - addControl(0, 1) */
    MixerControl *m_master;       /* sink+0x1c - addControl(0, 0) */
    MixerControl *m_wave;         /* sink+0x20 - addControl(1, 0) */
    SystemMenu   *m_menu;         /* sink+0x24 */
    int           m_volume;       /* sink+0x28 */
    int           m_adjustVolume; /* sink+0x2c */
};

TestInput *createTestInput();                  /* 0x475d20 */

/* TestSound - the test menu's SOUND page, and the fourth class in the binary
   with a second polymorphic base at +0x10 (see include/directors.h). Its sink's
   single virtual is `ret 0xc`, so THREE arguments - a different interface from
   the warning screens' two-argument one, which is why each gets its own type.

   Named by its KRuntimeClass @0x4afbb8, sized 0x40 by its record @0x4afbc8. */
struct __declspec(novtable) TestSoundSink : TestPageSink
{
    virtual void onMenu(int a, int what, int c); /* 0x476950 */
};

struct TestSound : EzObject, TestSoundSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x476890 */
    virtual DrawGroup     *getObjectInfo();    /* 0x4768a0 */
    virtual void           update();           /* 0x402880 (STUB, folded) */
    virtual void           update2(int, int);  /* 0x476910 (STUB) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    TestSound();                               /* 0x477000 */
    ~TestSound();                              /* 0x4768b0 */

    void buildMenu();                          /* 0x476e40 (STUB) */

    /* Three test tones, loaded by name; only the first is freed again. */
    TestTone *m_all;           /* 0x14 - handed back to the sound subsystem */
    TestTone *m_bass;          /* 0x18 */
    TestTone *m_treble;        /* 0x1c */
    int   _pad20;              /* 0x20 */
    /* The four mixer controls the volume row drives, in the order the sink
       creates them - the same four Obj401780::s2 saves and forces. */
    MixerControl *m_line;      /* 0x24 - addControl(0, 2) */
    MixerControl *m_cd;        /* 0x28 - addControl(0, 1) */
    MixerControl *m_master;    /* 0x2c - addControl(0, 0) */
    MixerControl *m_wave;      /* 0x30 - addControl(1, 0) */
    SystemMenu *m_menu;        /* 0x34 */
    int   m_volume;            /* 0x38 - GameVolume, clamped to 0..1 */
    int   m_adjustVolume;      /* 0x3c - named by its own log line */
};

TestSound *createTestSound();                  /* 0x477110 */

/* TestVideo - the test menu's VIDEO page: a grid and a colour-bar image the
   Service button flips between, drawn full-screen with the page's title and
   footer over it.

   Named by its KRuntimeClass @0x4afc24 and sized 0x1c by its record @0x4afc34,
   which is exactly the two textures its constructor loads plus the index into
   them. It is the only page in the family with a real (if empty) destructor
   AND a factory of its own @0x477430. */
struct TestVideo : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x477170 */
    virtual DrawGroup     *getObjectInfo();    /* 0x477180 */
    virtual void           update();           /* 0x477260 */
    virtual void           update2(int a, int b); /* 0x4771a0 */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    TestVideo();                               /* 0x477390 */
    ~TestVideo();                              /* 0x477190 */

    int              m_index;                  /* 0x10 - which image is up */
    struct KTexture *m_tex[2];                 /* 0x14 - grid, colour bars */
};

TestVideo *createTestVideo();                  /* 0x477430 */

#endif /* TESTINPUT_H */
