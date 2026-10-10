#ifndef KFILE_H
#define KFILE_H

/* KFile: a thin Win32 file wrapper. Layout { vtable @0x0; HANDLE handle @0x4 }.
   7 virtual methods; vtable @0x488b0c (pinned in src/kfile.cpp). Method bodies
   live only in src/kfile.cpp so callers in other TUs emit real calls. */
struct KFile
{
    void *handle;    /* 0x4 (after the vtable pointer at 0x0) */

    KFile();
    virtual ~KFile();                          /* slot 0 @0x413220 */
    virtual int open(char *name, int flags);   /* slot 1 @0x413090 */
    virtual void close();                      /* slot 2 @0x413100 */
    virtual int read(void *buf, int len);      /* slot 3 @0x413120 */
    virtual int write(void *buf, int len);     /* slot 4 @0x413150 */
    virtual unsigned size();                   /* slot 5 @0x413180 */
    virtual int printf(const char *fmt, ...);  /* slot 6 @0x413190 */
};

#endif /* KFILE_H */
