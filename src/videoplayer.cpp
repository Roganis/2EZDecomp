/* Obj40B2E0 - the DirectShow video player the backgrounds and directors
   embed. Its constructor @0x40b180 builds the graph: CoInitialize, a
   FilterGraph with an IMediaControl on it, a Sample Grabber filter added to
   the graph as "Sample Grabber" and buffering its samples, plus the
   IMediaSeeking / IVideoWindow / IBasicAudio / IMediaEventEx views; the
   audio starts silent (volume and balance -10000). The window is 640x480 at
   the origin. Everything else is zeroed by hand and f0 = 1. */
#include <windows.h>
#include <objbase.h>
#include "kcrt.h"      /* the array operator new is a real call */
#include "panels.h"
#include "ez2acapp.h"
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "kparser.h"
#include "loader.h"
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                         /* 0x412e60 */
extern void replaceExtension(char *name, char *ext);/* 0x413000 */

/* Only the slots this constructor calls, at their COM vtable indices. */
struct IGraphBuilder
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall AddFilter(struct IBaseFilter *f, const wchar_t *name);   /* slot 3 -> +0xc */
};
struct IBaseFilter
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
};
struct IMediaControl
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetTypeInfoCount(void *n);
    virtual int __stdcall GetTypeInfo(int i, int lcid, void **out);
    virtual int __stdcall GetIDsOfNames(void *iid, void *names, int n, int lcid, void *ids);
    virtual int __stdcall Invoke(int id, void *iid, int lcid, int flags, void *p, void *r, void *e, void *a);
    virtual int __stdcall Run();
    virtual int __stdcall Pause();
    virtual int __stdcall Stop();
    virtual int __stdcall GetState(int timeout, int *state);
    virtual int __stdcall RenderFile(const wchar_t *name);
    virtual int __stdcall AddSourceFilter(const wchar_t *name, void **out);
    virtual int __stdcall get_FilterCollection(void **out);
    virtual int __stdcall get_RegFilterCollection(void **out);
    virtual int __stdcall StopWhenReady();                                   /* slot 15 -> +0x3c */
};
struct IUnknownView
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
};
struct ISampleGrabber
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall SetOneShot(int on);
    virtual int __stdcall SetMediaType(void *mt);
    virtual int __stdcall GetConnectedMediaType(void *mt);
    virtual int __stdcall SetBufferSamples(int on);                          /* slot 6 -> +0x18 */
    virtual int __stdcall GetCurrentBuffer(int *size, void *buf);            /* slot 7 -> +0x1c */
};
/* VIDEOINFOHEADER - only the BITMAPINFOHEADER at +0x30 is reached. */
struct VideoInfoHeader { char _pad0[0x30]; BITMAPINFOHEADER bmi; };
struct IBasicAudio
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetTypeInfoCount(void *n);
    virtual int __stdcall GetTypeInfo(int i, int lcid, void **out);
    virtual int __stdcall GetIDsOfNames(void *iid, void *names, int n, int lcid, void *ids);
    virtual int __stdcall Invoke(int id, void *iid, int lcid, int flags, void *p, void *r, void *e, void *a);
    virtual int __stdcall put_Volume(int v);                                 /* slot 7 -> +0x1c */
    virtual int __stdcall get_Volume(int *v);
    virtual int __stdcall put_Balance(int v);                                /* slot 9 -> +0x24 */
};

/* The GUIDs, as the binary keeps them (placeholder storage; the values are
   IID_IGraphBuilder, CLSID_FilterGraph, IID_IMediaControl, IID_IBaseFilter,
   CLSID_SampleGrabber, IID_IVideoWindow, IID_IBasicAudio, IID_IMediaEventEx,
   IID_ISampleGrabber, IID_IMediaSeeking). */
// GLOBAL: EZ2AC_UNPACKED 0x004a01d0
GUID g_IID_IGraphBuilder;
// GLOBAL: EZ2AC_UNPACKED 0x004a0270
GUID g_CLSID_FilterGraph;
// GLOBAL: EZ2AC_UNPACKED 0x004a0250
GUID g_IID_IMediaControl;
// GLOBAL: EZ2AC_UNPACKED 0x004a01f0
GUID g_IID_IBaseFilter;
// GLOBAL: EZ2AC_UNPACKED 0x004a0200
GUID g_CLSID_SampleGrabber;
// GLOBAL: EZ2AC_UNPACKED 0x004a0220
GUID g_IID_IVideoWindow;
// GLOBAL: EZ2AC_UNPACKED 0x004a0230
GUID g_IID_IBasicAudio;
// GLOBAL: EZ2AC_UNPACKED 0x004a0240
GUID g_IID_IMediaEventEx;
// GLOBAL: EZ2AC_UNPACKED 0x004a0210
GUID g_IID_ISampleGrabber;
// GLOBAL: EZ2AC_UNPACKED 0x004a01e0
GUID g_IID_IMediaSeeking;

// FUNCTION: EZ2AC_UNPACKED 0x0040b180
Obj40B2E0::Obj40B2E0()
{
    x = 0.0f;
    y = 0.0f;
    w = 640.0f;
    f4 = 0;
    f10 = 0;
    f14 = 0;
    f438 = 0;
    f0 = 1;
    f8 = 0;
    fc = 0;
    h = 480.0f;
    f43c = 0;
    f440 = 0;
    f444 = 0;
    format = 0;
    CoInitialize(0);
    CoCreateInstance(g_CLSID_FilterGraph, 0, 3, g_IID_IGraphBuilder, (void **)&graph);
    graph->QueryInterface(&g_IID_IMediaControl, (void **)&control);
    CoCreateInstance(g_CLSID_SampleGrabber, 0, 3, g_IID_IBaseFilter, (void **)&grabberFilter);
    // STRING: EZ2AC_UNPACKED 0x00488170
    graph->AddFilter(grabberFilter, L"Sample Grabber");
    graph->QueryInterface(&g_IID_IMediaSeeking, (void **)&seeking);
    graph->QueryInterface(&g_IID_IVideoWindow, (void **)&window);
    graph->QueryInterface(&g_IID_IBasicAudio, (void **)&audio);
    graph->QueryInterface(&g_IID_IMediaEventEx, (void **)&events);
    grabberFilter->QueryInterface(&g_IID_ISampleGrabber, (void **)&grabber);
    grabber->SetBufferSamples(1);
    audio->put_Balance(-10000);
    audio->put_Volume(-10000);
}

/* The teardown @0x40b2e0: only once a graph was built (f438), stop it and
   Release the eight interfaces, then CoUninitialize (tail-jumped). */
// FUNCTION: EZ2AC_UNPACKED 0x0040b2e0
Obj40B2E0::~Obj40B2E0()
{
    if (f438 != 0)
    {
        control->StopWhenReady();
        ((IUnknownView *)events)->Release();
        ((IUnknownView *)audio)->Release();
        ((IUnknownView *)window)->Release();
        ((IUnknownView *)seeking)->Release();
        grabberFilter->Release();
        ((IUnknownView *)grabber)->Release();
        control->Release();
        graph->Release();
        CoUninitialize();
    }
}

/* Draw the current frame texture - the blend and a white or untinted quad
   depending on f4 - and, while the graph is running and not paused (f0),
   grab the newest sample: malloc a buffer of the last reported size, copy the
   sample into it, lock the texture's surface and StretchDIBits the frame into
   it at 256 or 512 square (the two arms spell their sizes differently, as the
   original does), then release, unlock and free. */
// FUNCTION: EZ2AC_UNPACKED 0x0040b710
void Obj40B2E0::m40b710()
{
    void *dc;

    if (f438 != 0)
    {
        gfx->setBlend(2, 1);
        if (f4 == 1)
            gfx->drawTex(tex, x, y, w, h, 0);
        else
            gfx->drawTex(tex, x, y, w, h, 0xffffff);
        if (f438 != 0 && f0 == 0)
        {
            BITMAPINFO *bmi;
            int n;
            buf = new unsigned char[bufSize];
            grabber->GetCurrentBuffer(&bufSize, buf);
            bmi = (BITMAPINFO *)&format->bmi;
            tex->surface->GetDC(&dc);
            tex->surface->Lock(0, &tex->desc, 0x20, 0);
            n = f10;
            if (n == 0x100)
                StretchDIBits((HDC)dc, 0, 0, 0x100, 0x100, 0, 0, 0x100, 0x100, buf, bmi, 0, SRCCOPY);
            else if (n == 0x200)
                StretchDIBits((HDC)dc, 0, 0, n, n, 0, 0, n, n, buf, bmi, 0, SRCCOPY);
            tex->surface->ReleaseDC(dc);
            tex->surface->Unlock(0);
            delete[] buf;
        }
    }
}

/* Start the graph: stamp the start time, clear the stopped/ended flags, Run,
   and clear the frame counter. */
// FUNCTION: EZ2AC_UNPACKED 0x0040b8b0
void Obj40B2E0::play(unsigned int now)
{
    f448 = now;
    f0 = 0;
    f440 = 0;
    control->Run();
    f44c = 0;
}

/* Is the graph running (State_Running == 2) and not flagged finished? */
// FUNCTION: EZ2AC_UNPACKED 0x0040b920
int Obj40B2E0::isRunning()
{
    int state;
    if (control != 0 && f438 != 0)
    {
        control->GetState(0, &state);
        if (state == 2 && f444 == 0)
            return 1;
    }
    return 0;
}

/* The rest of the DirectShow surface load() touches. */
struct IVideoWindow
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetTypeInfoCount(void *n);
    virtual int __stdcall GetTypeInfo(int i, int lcid, void **out);
    virtual int __stdcall GetIDsOfNames(void *iid, void *names, int n, int lcid, void *ids);
    virtual int __stdcall Invoke(int id, void *iid, int lcid, int flags, void *p, void *r, void *e, void *a);
    virtual int __stdcall put_Caption(void *s);              /* 7 */
    virtual int __stdcall get_Caption(void *s);
    virtual int __stdcall put_WindowStyle(int v);
    virtual int __stdcall get_WindowStyle(int *v);
    virtual int __stdcall put_WindowStyleEx(int v);
    virtual int __stdcall get_WindowStyleEx(int *v);
    virtual int __stdcall put_AutoShow(int v);
    virtual int __stdcall get_AutoShow(int *v);
    virtual int __stdcall put_WindowState(int v);
    virtual int __stdcall get_WindowState(int *v);
    virtual int __stdcall put_BackgroundPalette(int v);
    virtual int __stdcall get_BackgroundPalette(int *v);
    virtual int __stdcall put_Visible(int v);
    virtual int __stdcall get_Visible(int *v);
    virtual int __stdcall put_Left(int v);
    virtual int __stdcall get_Left(int *v);
    virtual int __stdcall put_Width(int v);
    virtual int __stdcall get_Width(int *v);
    virtual int __stdcall put_Top(int v);
    virtual int __stdcall get_Top(int *v);
    virtual int __stdcall put_Height(int v);
    virtual int __stdcall get_Height(int *v);
    virtual int __stdcall put_Owner(void *hwnd);             /* 29 -> +0x74 */
    virtual int __stdcall get_Owner(void **hwnd);
    virtual int __stdcall put_MessageDrain(void *hwnd);      /* 31 -> +0x7c */
    virtual int __stdcall get_MessageDrain(void **hwnd);
    virtual int __stdcall get_BorderColor(int *v);
    virtual int __stdcall put_BorderColor(int v);
    virtual int __stdcall get_FullScreenMode(int *v);
    virtual int __stdcall put_FullScreenMode(int v);
    virtual int __stdcall SetWindowForeground(int focus);
    virtual int __stdcall NotifyOwnerMessage(void *hwnd, int msg, int wp, int lp);
    virtual int __stdcall SetWindowPosition(int l, int t, int w, int h); /* 39 -> +0x9c */
};
struct IMediaSeeking
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetCapabilities(int *c);
    virtual int __stdcall CheckCapabilities(int *c);
    virtual int __stdcall IsFormatSupported(const GUID *f);
    virtual int __stdcall QueryPreferredFormat(GUID *f);
    virtual int __stdcall GetTimeFormat(GUID *f);
    virtual int __stdcall IsUsingTimeFormat(const GUID *f);
    virtual int __stdcall SetTimeFormat(const GUID *f);
    virtual int __stdcall GetDuration(__int64 *d);           /* 10 -> +0x28 */
    virtual int __stdcall GetStopPosition(__int64 *p);
    virtual int __stdcall GetCurrentPosition(__int64 *p);
    virtual int __stdcall ConvertTimeFormat(__int64 *t, const GUID *tf, __int64 s, const GUID *sf);
    virtual int __stdcall SetPositions(__int64 *cur, int curFlags, __int64 *stop, int stopFlags); /* 14 -> +0x38 */
};

/* The media type the grabber is asked for: MEDIATYPE_Video / MEDIASUBTYPE_RGB24
   / FORMAT_VideoInfo, as the 16-byte blocks the original copies them as. */
// GLOBAL: EZ2AC_UNPACKED 0x004a0290
Obj40B2E0::Guid16 g_MEDIATYPE_Video;
// GLOBAL: EZ2AC_UNPACKED 0x004a0280
Obj40B2E0::Guid16 g_MEDIASUBTYPE_RGB24;
// GLOBAL: EZ2AC_UNPACKED 0x004a0260
Obj40B2E0::Guid16 g_FORMAT_VideoInfo;


/* Build the graph for a video file: the file must exist; its .ini (if any)
   gives the per-frame delay and the start offset; a VIDEOTEX texture is found
   or created, the grabber is set to 24-bit RGB, the file is rendered into the
   graph, the connected format is read back for the frame size, the blank and
   gray textures are loaded, the video window is parked off-screen, and the
   graph is seeked to the start offset. */
// FUNCTION: EZ2AC_UNPACKED 0x0040b370
int Obj40B2E0::load(KGraphics *g, char *name)
{
    wchar_t wname[0x104];
    __int64 start;

    if (f438 != 0)
        return 0;
    gfx = g;
    strcpy(path, name);
    strcpy(iniPath, name);
    // STRING: EZ2AC_UNPACKED 0x00488220
    replaceExtension(iniPath, ".ini");
    if (_access(path, 0) == -1)
        return 0;

    KParser parser;     /* declared here, so its ctor runs after the checks */
    if (parser.readFile(iniPath))
    {
        readGdsInt(&f8, &parser);
        readGdsInt(&fc, &parser);
    }
    if (f8 == 0)
    {
        f8 = 5;
        // STRING: EZ2AC_UNPACKED 0x00488208
        Log("Set Default Delay Time");
    }
    // STRING: EZ2AC_UNPACKED 0x004881fc
    gfx->findOrCreateTexture("VIDEOTEX", &tex, 0, 0);

    memset(&mt, 0, sizeof(mt));
    mt.majortype = g_MEDIATYPE_Video;
    mt.subtype = g_MEDIASUBTYPE_RGB24;
    mt.bFixedSizeSamples = 1;
    mt.formattype = g_FORMAT_VideoInfo;
    grabber->SetMediaType(&mt);

    MultiByteToWideChar(0, 0, path, -1, wname, 0x104);
    control->RenderFile(wname);
    // STRING: EZ2AC_UNPACKED 0x004881ec
    Log("Video Path : %s", path);
    grabber->GetConnectedMediaType(&mt);
    format = (VideoInfoHeader *)mt.pbFormat;
    if (format == 0)
        return 0;
    f10 = format->bmi.biWidth;
    f14 = format->bmi.biHeight;

    // STRING: EZ2AC_UNPACKED 0x004881d8
    changeDir("\\System\\ModeSelect", 1);
    // STRING: EZ2AC_UNPACKED 0x004881cc
    gfx->loadTexture("gray.bmp", &gray, 0);
    popDirectory();
    // STRING: EZ2AC_UNPACKED 0x004881bc
    changeDir("\\System\\Common", 1);
    if (f10 == 0x100)
    {
        // STRING: EZ2AC_UNPACKED 0x004881a8
        gfx->loadTexture("video_blank_256.bmp", &tex, 0);
    }
    else if (f10 == 0x200)
    {
        // STRING: EZ2AC_UNPACKED 0x00488194
        gfx->loadTexture("video_blank_512.bmp", &tex, 0);
    }
    popDirectory();

    bufSize = mt.lSampleSize;
    window->put_MessageDrain(*(void **)((char *)gfx + 4));
    window->SetWindowPosition(-100, -100, 0, 0);
    window->put_Owner(*(void **)((char *)gfx + 4));
    if (control != 0)
        f438 = 1;
    start = fc * 10000;
    seeking->SetPositions(&start, 1, 0, 0);
    seeking->GetDuration(&duration);
    return 1;
}

/* The thunk form, not windows.h's dllimport one (videoplayer includes
   windows.h without the multimedia section, so this is the first
   declaration this TU sees). */
extern "C" DWORD __stdcall timeGetTime();          /* 0x4145f0 */

/* Only the two slots the event pump uses, at their COM vtable indices
   (IMediaEvent is IDispatch-based, so the IUnknown and IDispatch slots
   come first). */
struct IMediaEventEx
{
    virtual int __stdcall QueryInterface(const GUID *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetTypeInfoCount(void *n);
    virtual int __stdcall GetTypeInfo(int i, int lcid, void **out);
    virtual int __stdcall GetIDsOfNames(void *iid, void *names, int n, int lcid, void *ids);
    virtual int __stdcall Invoke(int id, void *iid, int lcid, int flags, void *p, void *r, void *e, void *a);
    virtual int __stdcall GetEventHandle(void **h);
    virtual int __stdcall GetEvent(int *code, int *p1, int *p2, int timeout);  /* 8 -> +0x20 */
    virtual int __stdcall WaitForCompletion(int timeout, int *code);
    virtual int __stdcall CancelDefaultHandling(int code);
    virtual int __stdcall RestoreDefaultHandling(int code);
    virtual int __stdcall FreeEventParams(int code, int p1, int p2);           /* 12 -> +0x30 */
};

/* The per-frame event pump: bump the frame counter (f4 says the counter is
   still within the clip), then drain the graph's event queue. On
   EC_COMPLETE (code 1) a looping player (f43c) rewinds to 0, restamps the
   start time and Runs the graph again; either way f444 records that the
   clip has ended once. The argument is unused. */
// FUNCTION: EZ2AC_UNPACKED 0x0040b9f0
void Obj40B2E0::m40b9f0(int)
{
    int code;
    int p1;
    int p2;
    __int64 pos;

    if (f438 == 0)
        return;

    f44c += 1;
    f4 = f44c <= f8;
    while (events->GetEvent(&code, &p1, &p2, 0) >= 0)
    {
        events->FreeEventParams(code, p1, p2);
        if (code == 1)
        {
            if (f43c != 0)
            {
                pos = 0;
                seeking->SetPositions(&pos, 1, 0, 0);
                f44c = 0;
                f448 = timeGetTime();
                f0 = 0;
                f440 = 0;
                control->Run();
                f44c = 0;
            }
            f444 = 1;
        }
    }
}
