#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>
#include "kcrt.h"
#include "kbase.h"

void Log(const char *fmt, ...);

/* Per-control WINMM wrapper view: { HMIXEROBJ @0x00; control id @0x04 }.
   Instances live inside the 0x1c-byte records KSNDMixer heap-allocates. */
class MixerControl
{
public:
    HMIXEROBJ m_hmx;        /* 0x00 */
    DWORD     m_controlId;  /* 0x04 */

    DWORD getValue();
    void  setValue(DWORD value);
};

/* The 4-byte details local must be a separate address-taken local assigned
   from the parameter — cl overlaps it into the dead parameter's home slot,
   producing the original's [esp+0x1c] round-trip; pointing paDetails at the
   parameter directly loses those two instructions. */
// FUNCTION: EZ2AC_UNPACKED 0x00412a10
void MixerControl::setValue(DWORD value)
{
    MIXERCONTROLDETAILS mcd;
    MIXERCONTROLDETAILS_UNSIGNED details;

    details.dwValue = value;
    mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mcd.dwControlID = m_controlId;
    mcd.cChannels = 1;
    mcd.cMultipleItems = 0;
    mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    mcd.paDetails = &details;
    mixerSetControlDetails(m_hmx, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
}

// FUNCTION: EZ2AC_UNPACKED 0x004129c0
DWORD MixerControl::getValue()
{
    MIXERCONTROLDETAILS mcd;
    MIXERCONTROLDETAILS_UNSIGNED value;

    mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mcd.dwControlID = m_controlId;
    mcd.cChannels = 1;
    mcd.cMultipleItems = 0;
    mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    mcd.paDetails = &value;
    return mixerGetControlDetailsA(m_hmx, &mcd, MIXER_GETCONTROLDETAILSF_VALUE) ? 0 : value.dwValue;
}

/* KSNDMixer ("KSNDMixer(ksndInitMixer)" log strings): the mixer-device owner.
   Derives from the novtable engine base (kbase.h); 4-slot vtable @0x488af4 =
   {getClassDesc, deleting dtor, init, close}. Owns up to 32 heap-allocated
   0x1c-byte control records (addControl @0x412af0, unmatched, mallocs them;
   MixerControl views point inside them). Formerly modeled as Obj412C60 in
   lists.cpp before init/close/ctor revealed the class. */
// VTABLE: EZ2AC_UNPACKED 0x00488af4
class KSNDMixer : public KBase
{
public:
    HMIXER m_hmixer;    /* 0x04 */
    void  *m_slots[32]; /* 0x08 .. 0x88 */
    int    m_count;     /* 0x88 */

    KSNDMixer();                  /* 0x412d10 */
    virtual void *getClassDesc(); /* slot 0 @0x401a90 */
    virtual ~KSNDMixer();         /* slot 1: ??_G @0x412d90, ??1 @0x412d60 */
    virtual int  init();          /* slot 2 @0x412a60 */
    virtual void close();         /* slot 3 @0x412d40 */

    int   findFreeSlot();    /* 0x412ac0 */
    void *addControl(int dst, int type); /* 0x412af0 */
    int   remove(void *key); /* 0x412c60 */
    int   clear();           /* 0x412cc0 */
};

/* The 0x1c-byte heap record addControl fills; MixerControl views its first
   two fields. */
struct MixerRecord
{
    HMIXER hmx;       /* 0x00 */
    DWORD  controlId; /* 0x04 */
    int    dst;       /* 0x08 */
    int    type;      /* 0x0c */
    DWORD  min;       /* 0x10 */
    DWORD  max;       /* 0x14 */
    DWORD  steps;     /* 0x18 */
};

/* Destination/control lookup tables (contents derived: WINMM SDK constants). */
// GLOBAL: EZ2AC_UNPACKED 0x004a84c0
DWORD g_mixerLineTypes[8] =
{
    MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
    MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT,
    MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
    MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER,
    MIXERLINE_COMPONENTTYPE_SRC_LINE,
    MIXERLINE_COMPONENTTYPE_SRC_DIGITAL,
    MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY,
    MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED
};

// GLOBAL: EZ2AC_UNPACKED 0x004a84e0
DWORD g_mixerControlTypes[14] =
{
    MIXERCONTROL_CONTROLTYPE_VOLUME,
    MIXERCONTROL_CONTROLTYPE_BASS,
    MIXERCONTROL_CONTROLTYPE_TREBLE,
    MIXERCONTROL_CONTROLTYPE_EQUALIZER,
    MIXERCONTROL_CONTROLTYPE_FADER,
    MIXERCONTROL_CONTROLTYPE_PAN,
    MIXERCONTROL_CONTROLTYPE_QSOUNDPAN,
    MIXERCONTROL_CONTROLTYPE_SLIDER,
    MIXERCONTROL_CONTROLTYPE_MUTE,
    MIXERCONTROL_CONTROLTYPE_MONO,
    MIXERCONTROL_CONTROLTYPE_ONOFF,
    MIXERCONTROL_CONTROLTYPE_LOUDNESS,
    MIXERCONTROL_CONTROLTYPE_BOOLEAN,
    MIXERCONTROL_CONTROLTYPE_CUSTOM
};

// FUNCTION: EZ2AC_UNPACKED 0x00412d10
KSNDMixer::KSNDMixer()
{
    m_hmixer = 0;
    memset(m_slots, 0, sizeof(m_slots));
    m_count = 0;
}

/* The class-registry descriptor returned by slot 0 (placeholder storage;
   formerly get_4a50f4 in globals.cpp before the owner was identified). */
// GLOBAL: EZ2AC_UNPACKED 0x004a50f4
static int g_4a50f4;

// FUNCTION: EZ2AC_UNPACKED 0x00401a90
void *KSNDMixer::getClassDesc()
{
    return &g_4a50f4;
}

/* 0x412d90 is the compiler-generated scalar deleting destructor (vtable
   slot 1: call ??1 @0x412d60, conditional operator delete -> free via
   kcrt.h). Target it by name. */
// FUNCTION: EZ2AC_UNPACKED 0x00412d90
// KSNDMixer::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00412a60
int KSNDMixer::init()
{
    if (mixerGetNumDevs() == 0) {
        // STRING: EZ2AC_UNPACKED 0x00488ac0
        Log("KSNDMixer(ksndInitMixer) : There's no mixer device");
        return 0;
    }
    if (mixerOpen(&m_hmixer, 0, 0, 0, 0) != MMSYSERR_NOERROR) {
        // STRING: EZ2AC_UNPACKED 0x00488a84
        Log("KSNDMixer(ksndInitMixer) : Error on opening mixer device");
        return 0;
    }
    memset(m_slots, 0, sizeof(m_slots));
    m_count = 0;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00412ac0
int KSNDMixer::findFreeSlot()
{
    if (m_count < 32) {
        for (int i = 0; i < 32; i++) {
            if (m_slots[i] == 0)
                return i;
        }
    }
    return -1;
}

/* Open a mixer control for destination line dst / control type: allocate a
   MixerRecord, resolve the line and control via WINMM, store it in a free
   slot. The bounds/slot failures share one return-0 tail (fail, no esi pop)
   while the malloc/API failures share a second one (fail2, after esi goes
   live for the record pointer) — the two-goto shape is what shrink-wraps
   push esi below the early checks. */
// FUNCTION: EZ2AC_UNPACKED 0x00412af0
void *KSNDMixer::addControl(int dst, int type)
{
    MIXERLINEA ml;
    MIXERLINECONTROLSA mlc;
    MIXERCONTROLA mc;
    MixerRecord *p;
    int slot;

    if (dst < 0 || dst > 7)
        goto fail;
    if (type < 0 || type > 13)
        goto fail;

    slot = findFreeSlot();
    if (slot == -1)
        goto fail;

    p = new MixerRecord;
    if (p == 0)
        goto fail2;

    ml.cbStruct = sizeof(MIXERLINEA);
    ml.dwComponentType = g_mixerLineTypes[dst];
    if (mixerGetLineInfoA((HMIXEROBJ)m_hmixer, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR)
    {
        delete p;
        goto fail2;
    }

    mlc.cbStruct = sizeof(MIXERLINECONTROLSA);
    mlc.dwLineID = ml.dwLineID;
    mlc.dwControlType = g_mixerControlTypes[type];
    mlc.cControls = 1;
    mlc.cbmxctrl = sizeof(MIXERCONTROLA);
    mlc.pamxctrl = &mc;
    if (mixerGetLineControlsA((HMIXEROBJ)m_hmixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
    {
        delete p;
        goto fail2;
    }

    p->hmx = m_hmixer;
    p->controlId = mc.dwControlID;
    p->dst = dst;
    p->type = type;
    p->min = mc.Bounds.dwMinimum;
    p->max = mc.Bounds.dwMaximum;
    p->steps = mc.Metrics.cSteps;
    m_slots[slot] = p;
    m_count++;
    return p;

fail2:
    return 0;
fail:
    return 0;
}

static int findSlot32(void **arr, void *key)
{
    int i = 0;
    void **p = arr;
    do
    {
        if (*p == key)
            return i;
        i++;
        p++;
    } while (i < 0x20);
    return -1;
}

/* Find a stored record, free it, null the slot, decrement the live count. */
// FUNCTION: EZ2AC_UNPACKED 0x00412c60
int KSNDMixer::remove(void *key)
{
    if (key == 0)
        return 0;

    int i = findSlot32(m_slots, key);
    if (i == -1)
        return 0;

    operator delete(key);
    m_slots[i] = 0;
    m_count--;
    return 1;
}

/* close() calls clear(), which is defined AFTER it (caller-first, so the
   call stays a real call). */
// FUNCTION: EZ2AC_UNPACKED 0x00412d40
void KSNDMixer::close()
{
    clear();
    if (m_hmixer)
    {
        mixerClose(m_hmixer);
        m_hmixer = 0;
    }
}

/* Free every non-null slot, zero the count and the whole array. */
// FUNCTION: EZ2AC_UNPACKED 0x00412cc0
int KSNDMixer::clear()
{
    for (int i = 0; i < 0x20; i++)
    {
        if (m_slots[i])
            operator delete(m_slots[i]);
    }
    m_count = 0;
    memset(m_slots, 0, sizeof(m_slots));
    return 1;
}

/* The real destructor: close() (defined above) gets devirtualized AND
   inlined here — the original body is exactly close's code (its clear()
   call survives because clear has a loop), then the auto-generated tail
   jmp to the base dtor thunk. */
// FUNCTION: EZ2AC_UNPACKED 0x00412d60
KSNDMixer::~KSNDMixer()
{
    close();
}
