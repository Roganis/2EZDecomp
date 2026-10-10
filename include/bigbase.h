#ifndef BIGBASE_H
#define BIGBASE_H

#include "kprofile.h"
#include "icom.h"


/* One of the two record kinds in .data (see src/classinfo.cpp): the object
   record the engine's pools allocate from. Screens register the ones they need
   with themselves through Obj401780::addInfo @0x401850, which is what threads
   the three links below; m4017a0 @0x4017a0 then hands each record its slice of
   the screen's one allocation, and EzPooled::operator new @0x4016a0 bumps the
   cursor through it. The records sit 0x3c apart in .data.

   The engine has one name for this and the display model has another - the
   records ARE the draw groups (addInfo's `flags & 2` list is the one s1 ticks
   and its `& 4` list the one s1 draws), so DrawGroup is the same struct. */
struct DrawGroup
{
    unsigned int objectSize; /* 0x00 */
    unsigned int count;      /* 0x04 */
    unsigned int recFlags;   /* 0x08 - bit 1 = ticked, bit 2 = drawn */
    DrawGroup *nextOwned;    /* 0x0c - the link the screen's own list uses */
    DrawGroup *nextTick;     /* 0x10 - the link s1's first walk follows */
    DrawGroup *nextDraw;     /* 0x14 - the link its second walk follows */
    int        live;         /* 0x18 - how many operator new has handed out */
    char      *poolBase;     /* 0x1c */
    char      *poolEnd;      /* 0x20 */
    struct DrawNode *nodes;  /* 0x24 - the first object out of the pool */
    struct DrawNode *last;   /* 0x28 - the most recent one */
    char       _pad2c[0x3c - 0x2c];
};

typedef DrawGroup EzObjectInfo;

/* Every pooled engine object allocates through the same class-specific operator
   new @0x4016a0: it takes the record most recently published at 0x530b74, hands
   out the next slot from that record's pool (the bounds m4017a0 @0x4017a0 set up)
   and logs if the pool is exhausted. Modeled as one empty base so there is a
   single definition of it, exactly as in the original. */
struct EzPooled
{
    static void *__stdcall operator new(unsigned int n); /* 0x4016a0 - the
                       engine's allocator is __stdcall: it pops its own argument */
};

/* The record the next pooled allocation comes from. */
extern EzObjectInfo *g_objectRecord;            /* 0x530b74 */


/* The engine's huge base class - ~0x420000 bytes, constructed at 0x40c180
   (which zeroes 0x4200f4/0x420100/0x420104/0x420208/0x42026c/0x420270).
   Named after that ctor. Its members live past the image base, which is what
   used to trip reccmp's displacement symbolization; fixed by
   reccmp-patches/04-displacement-not-an-address.patch. */
/* One entry of the two 128-entry tables the engine object opens with. tick
   @0x40cc90 and applyOutputs @0x40c940 between them name all five words: an
   entry is driven by one of three sources, and source 1 reads a hardware byte
   and runs a four-state press tracker whose state applyOutputs then resolves
   onto the output channel (0/1 = released, so release the output; 2/3 = pressed,
   so drive it, and 2 stamps the time it happened). Channel -1 means "no output". */
struct IoSlot
{
    int source;   /* 0x00 - 1 = the hardware tracker, 0 and 3 = two handlers */
    int bit;      /* 0x04 - which byte of keyState source 1 watches */
    int state;    /* 0x08 */
    int since;    /* 0x0c - the time state 2 was entered */
    int channel;  /* 0x10 */
};

/* One recorded input event: when it happened (relative to the run's base time)
   and which state the entry went to. SIX bytes - packed, or the int would pad it
   to eight - and 20 channels x 0x4650 of them is exactly the 0x20F580-byte
   buffer clearBuffers wipes. The stride is what makes cl address the buffer with
   `lea reg,[reg+reg*2]` and a *2 scale. */
#pragma pack(push, 2)
struct IoEvent
{
    int  time;      /* 0x00 */
    char state;     /* 0x04 */
    char consumed;  /* 0x05 - set by the replay pass so an event fires once */
};
#pragma pack(pop)

/* One recording channel: the entry index it follows, and the slot in the SECOND
   table that its replayed events drive. */
struct IoChannel
{
    int key;      /* 0x00 */
    int slot;     /* 0x04 */
    int f8;
    int fc;
};

struct Obj40C180
{
    /* vftable 0x488254 - three slots, ALL of them the CRT's _purecall, so this
       class is abstract and the concrete subsystem object (the static at
       0x170dff0 that EZ2AC::v2 publishes) overrides every slot. It IS pinned
       now (src/obj40c180.cpp): _purecall has a LIBRARY annotation, so the
       slots resolve, and the class's own constructor @0x40c180 stores that
       vftable, which needs the entity to exist.
       init() @0x40cc30 calls slot 0 first and tail-jumps slot 1. */
    /* The three slots, named by the concrete class's overrides (vftable
       0x48cf50, CabinetInput below) - nothing calls them directly. */
    virtual void resetCoins() = 0;     /* slot 0 */
    virtual void bindButtons() = 0;    /* slot 1 */
    virtual void serviceCoins() = 0;   /* slot 2 */

    /* Seven zero stores after the vptr - and every one of the seven fields was
       already named by the methods. */
    Obj40C180();           /* 0x40c180 */
    /* NOT virtual (the vftable has no destructor slot): restore the vptr,
       resetFlags, tail-jump releaseAll - shutdown()'s body. Only the
       unreferenced ~CabinetInput reaches it. */
    ~Obj40C180();          /* 0x40cdc0 (src/obj40c180.cpp) */

    int  f4;           /* 0x000004 - cleared by init() */
    IoSlot slotsA[128];  /* 0x000008 */
    IoSlot slotsB[128];  /* 0x000a08 */
    /* Everything from here to 0x4200e8 is one contiguous run that
       clearBuffers() @0x40ca20 zeroes end to end. */
    /* 20 channels x 18000 six-byte events. m40ca90 @0x40ca90 appends to it,
       using arrA[channel] as the write cursor. */
    IoEvent events[20][0x4650];   /* 0x001408 */
    /* The second buffer is the same shape, and m40cb20 @0x40cb20 REPLAYS it:
       events whose time has come are pushed into the second slot table. */
    IoEvent replay[20][0x4650];   /* 0x210988 */
    IoChannel channels[20];       /* 0x41ff08 - what maps an entry to a channel */
    int  arrA[20];       /* 0x420048 - cleared together with arrB */
    int  arrB[20];       /* 0x420098 */
    int  f4200e8;   /* 0x4200e8 */
    int  f4200ec;   /* 0x4200ec */
    int  f4200f0;   /* 0x4200f0 */
    int  f4200f4;   /* 0x4200f4 - zero when there is no I/O board, and every
                       port access in the class is gated on it */
    int  cursorX;   /* 0x4200f8 - the mouse in client coordinates */
    int  cursorY;   /* 0x4200fc */
    /* DirectInput: the interface and its two devices, brought up by createInput
       @0x40c250 and torn down by releaseAll @0x40c320 (unacquire, slot 8, then
       release, slot 2, then the interface itself). */
    IDirectInputA *dinput;         /* 0x420100 */
    IDirectInputDeviceA *keyboard; /* 0x420104 - GUID_SysKeyboard */
    unsigned char keys[0x100];     /* 0x420108 - the keyboard state m40c390 polls */
    IDirectInputDeviceA *mouse;    /* 0x420208 - GUID_SysMouse */
    /* DIMOUSESTATE, filled by m40c430: three axes and four button bytes. */
    int mouseX;                    /* 0x42020c */
    int mouseY;                    /* 0x420210 */
    int mouseZ;                    /* 0x420214 */
    /* The button bytes tick's source-1 entries watch, bit 7 = pressed. The first
       four ARE the mouse's (this is the tail of the DIMOUSESTATE above); 4..6 are
       the I/O board's ports 0x101/0x102/0x106, inverted by m40c4b0. */
    unsigned char keyState[8];     /* 0x420218 */
    /* The I/O board's two button ports (0x103 and 0x104), latched twice so the
       tick can tell edges from levels. Volatile, like the event cursor below:
       the input state is shared with whoever polls the board, and it is what
       keeps resetInputState's four latch stores in source order. */
    volatile int port103;   /* 0x420220 */
    volatile int port104;   /* 0x420224 */
    volatile int port103b;  /* 0x420228 */
    volatile int port104b;  /* 0x42022c */
    /* The two analog encoders (turntables), read by m40c4b0 @0x40c4b0: this
       tick's movement, the running total, and the last five movements per side. */
    int  delta[2];  /* 0x420230 */
    int  accum[2];  /* 0x420238 */
    int  hist[2][5];/* 0x420240 - hist[1] is the 0x420254 half */
    int  limit;     /* 0x420268 - a running total past this is thrown away */
    /* The I/O board's event counter (port 0x105) and our cursor into it:
       pending() is the difference modulo 0x100. */
    int  head;      /* 0x42026c */
    int  tail;      /* 0x420270 */
    int  flags[4];  /* 0x420274 .. 0x420280 */
    int  have;      /* 0x420284 */
    int  f420288;   /* 0x420288 */
    int  f42028c;   /* 0x42028c */
    int  need;      /* 0x420290 - the .ini's "CreditCoins" */
    /* Two flags TestModeDirector's teardown latches on the way into the
       operator test menu. */
    int  testMode1; /* 0x420294 */
    int  testMode2; /* 0x420298 */
    int  f42029c;   /* 0x42029c */

    void init();           /* 0x40cc30 (src/obj40c180.cpp) */
    void clearBuffers();   /* 0x40ca20 (src/engine.cpp - not visible to init) */
    void createInput();    /* 0x40c250 (STUB, must NOT be visible to init) */
    void resetInputState();/* 0x40c820 (src/obj40c180.cpp, visible to init) */
    void shutdown();       /* 0x40cc80 (src/obj40c180.cpp) */
    void resetFlags();     /* 0x40c8f0 (src/obj40c180.cpp, visible to shutdown) */
    void tick(unsigned int now); /* 0x40cc90 (src/enginetick.cpp) */
    void m40c390();              /* 0x40c390 (STUB) */
    void m40c430();              /* 0x40c430 (STUB) */
    void m40c4b0();              /* 0x40c4b0 (STUB; wip/analogread.cpp) */
    /* The three indexed accessors that used to be modelled as separate
       classes (Obj40C1C0 / Obj40C230 / Obj40C800): their `this`-relative
       addresses are exactly slotsA[i].state, slotsA[i].channel and
       slotsB[i].state, so they are methods of THIS object. Each lives in a TU
       of its own so its caller cannot inline the single load or store. */
    int  getState(int i);                       /* 0x40c1c0 (src/misc.cpp) */
    int  m4191c0();                             /* 0x4191c0 (src/inputscan.cpp) */
    int  m419780();                             /* 0x419780 (src/inputscan.cpp) */
    /* Is any of the seventeen play buttons held? Codes 0x0a..0x19 and then
       0x00, each tested for state 2 (src/inputscan.cpp). */
    int  anyPlayHeld();                         /* 0x4188c0 */
    void setChannel(int i, int channel);        /* 0x40c230 (src/accessors.cpp) */
    int  getStateB(int i);                      /* 0x40c800 (src/accessors.cpp) */
    /* Route the five slots 0x12..0x16 onto output channels 0x18..0x1c, or
       unhook them. */
    void routeOutputs(int on);                  /* 0x418c10 (src/obj40c180.cpp) */
    /* Fill one entry of slotsA and bump the running button count. Its own TU:
       bindButtons reloads `this` from esi before every call, which it would not
       do if it could see this body. */
    int  addButton(int index, int source, int bit, int channel);
                                                /* 0x40c1f0 (src/accessors.cpp) */
    /* The ONLY button map (vtable 0x48cf50 slot 1, called once from init):
       slots 0x00..0x19 and nothing else, so the 0x1d/0x1e long-hold slots
       the menus read are never driven. 26 entries, source 3 (the I/O board's handler)
       for all but two - slot 0 watches keyState bit 0x1b through source 0, and
       slot 3 bit 0x72. None of them is routed to an output. Gated on the
       I/O-board flag like every other port user in the class. */

    /* One output channel at a time (setFlags @0x40c7d0, in src/accessors.cpp,
       is the all-four-words form). */
    void setOutput(int channel, int on);        /* 0x40c780 (src/obj40c180.cpp) */
    /* How long slot i has been in its current state. */
    unsigned int heldFor(int i);                /* 0x40c1d0 (src/obj40c180.cpp) */

    void m40c3d0(IoSlot *e, unsigned int now);  /* 0x40c3d0 (STUB) */
    void m40c6f0(IoSlot *e, unsigned int now);  /* 0x40c6f0 (STUB) */
    void m40ca90(int i, char state, unsigned int now); /* 0x40ca90 (STUB) */
    void m40cb20(unsigned int now);             /* 0x40cb20 (STUB) */
    void releaseAll();     /* 0x40c320 (src/engine.cpp - not visible to it) */
    int  pending();        /* 0x40c760 */
    void setFlags(int on); /* 0x40c7d0 */
    /* Three more one-argument cabinet-state setters, always called with 0
       alongside setFlags on the way out of a screen. */
    /* Bind or unbind three ranges of input slots to their channels
       (src/keychannels.cpp). */
    void m4189e0(int on);  /* 0x4189e0 */
    void m418a50(int on);  /* 0x418a50 */
    void m418af0(int on);  /* 0x418af0 */
    void m418b90(int on);  /* 0x418b90 (STUB) */
    int  consume();        /* 0x418890 */
    void applyOutputs();   /* 0x40c940 */
};

/* The concrete subsystem object - the static at 0x170dff0 that EZ2AC::v2
   publishes as g_bigInstance - with vftable 0x48cf50 overriding all three
   slots. Its out-of-line constructor and destructor are in the binary but
   nothing references either (the static is built some other way). */
struct CabinetInput : Obj40C180
{
    CabinetInput();                 /* 0x414fc0 */
    /* 0x414fe0 is the class's IMPLICIT destructor emitted out of line - a
       bare `jmp ~Obj40C180` with no vptr store, which a user-declared
       destructor would add (src/cabinetinput.cpp). */

    /* Put the coin state back and re-read the .ini's "CreditCoins". */
    virtual void resetCoins();      /* 0x418700 (src/obj40c180.cpp) */
    /* The ONLY button map, called once from init (see the note above
       addButton). */
    virtual void bindButtons();     /* 0x418c90 (src/obj40c180.cpp) */
    /* The per-frame coin/service tick (src/titledirector.cpp). */
    virtual void serviceCoins();    /* 0x418760 */
};

/* The process-wide instance. */
extern Obj40C180 *g_bigInstance;

/* The engine's screen/Director base class. Identified by EZ2AC::runGame
   @0x417e90: every screen it creates gets a 7-slot vftable whose slot 0 is
   0x4199d0 (armAndSetC, matched in src/misc.cpp) and whose slots 1/2/3/6 are
   shared by all of them, so those are this base's virtuals; slots 4 and 5 are
   overridden per screen (see include/ezscreen.h). run @0x401930 zeroes fields
   8..0x2c, stores the profile at +4 and then drives the screen through slots 2
   and 4 - which is what makes f8/fc (already known from setC @0x401780) fields
   of THIS class.
   setC and armAndSetC MUST stay in different TUs: with setC visible cl inlines
   it and armAndSetC's tail `jmp` disappears. */
struct Obj401780
{
    virtual void armAndSetC(int v); /* slot 0: 0x4199d0 (src/misc.cpp) */
    virtual void s1();              /* slot 1: 0x418620 */
    virtual void s2();              /* slot 2: 0x4184f0 */
    virtual void s3();              /* slot 3: 0x418fa0 */
    virtual void s4();              /* slot 4: per screen */
    virtual void s5();              /* slot 5: per screen */
    virtual void s6();              /* slot 6: 0x455b70 */

    int  run(KProfile *profile);    /* 0x401930 (src/ezscreens.cpp) */
    void setC(int v);               /* 0x401780 (src/accessors.cpp) */
    void addInfo(DrawGroup *g);     /* 0x401850 */
    int  m4017a0();                 /* 0x4017a0 (src/ezscreens.cpp) */
    void m4018c0();                 /* 0x4018c0 (STUB) */
    /* Step a cursor through one group's live instances: null starts at the
       group's head, otherwise follow the node's +8 link. It never touches
       `this` - but it IS a member, and every one of its six call sites loads
       ecx from the screen singleton @0x1b2e844 to prove it. */
    int  nextInstance(DrawGroup *group, struct DrawNode **cur); /* 0x401900 */

    KProfile *f4;      /* 0x04: the profile run() is given (or the app's) */
    int   f8;          /* 0x08: 3 while starting, 1 while running, 0 = leave */
    int   fc;          /* 0x0c: what run() returns */
    DrawGroup *f10;    /* 0x10: every group the screen owns, and 0x14 its tail */
    DrawGroup *f14;    /* 0x14 */
    DrawGroup *f18;    /* 0x18: the groups s1 ticks, and 0x1c its tail */
    DrawGroup *f1c;    /* 0x1c */
    DrawGroup *f20;    /* 0x20: the groups s1 draws, and 0x24 its tail */
    DrawGroup *f24;    /* 0x24 */
    char *f28;         /* 0x28: the one pool allocation, VirtualFree'd on the way out */
    char *f2c;         /* 0x2c: one past its end */
    unsigned int f30;  /* 0x30: timeGetTime() when the screen started */
    unsigned int f34;  /* 0x34..0x40: the four mixer volumes s2 saves before it */
    unsigned int f38;  /*             forces its own, so slot 3 can put them back */
    unsigned int f3c;
    unsigned int f40;
    char  _pad44[0x48 - 0x44];      /* screens are 0x48 bytes (runGame's new) */
};

#endif /* BIGBASE_H */
