#ifndef KPROFILE_H
#define KPROFILE_H

/* KProfile: the config/profile parser ("KProfile::Load - STR/FLOAT/INT %s:%s").
   vtable @0x488b68 { [0]=~KProfile deleting dtor 0x413e90, [1]=getName 0x413e80 }.
   Derives from a base (vtable 0x488c24, ctor 0x414390, non-deleting dtor
   0x4143b0) — only the base's non-trivial virtual dtor is load-bearing (it
   forces the explicit base-dtor call + EH funclet in ~KProfile). */
// VTABLE: EZ2AC_UNPACKED 0x00488c24 (pinned in src/krescontainer.cpp)
struct KResourceContainer
{
    int   f4;       /* 0x04 */
    int   f8;       /* 0x08 */
    void *arena;    /* 0x0c VirtualAlloc'd, freed by the dtor */

    KResourceContainer();                   /* 0x414390 */
    virtual ~KResourceContainer();          /* 0x4143b0 (deleting ??_G 0x414580) */
    virtual const char *getName();          /* 0x414380 -> "KResourceContainer" */
};

/* Parsed config entry: a string-keyed binary-tree node. Value payload at +0x50
   (read as int by getInt, as a string by getStr/getValue). */
struct KProfileNode
{
    KProfileNode *left;   /* 0x00 */
    KProfileNode *right;  /* 0x04 */
    char          key[0x40];  /* 0x08 */
    int           type;   /* 0x48 */
    int           f4c;    /* 0x4c */
    int           value;  /* 0x50 */
};

struct KProfile : KResourceContainer
{
    /* base KResourceContainer occupies 0x00..0x10 (vptr,f4,f8,arena);
       the ctor @0x413dc0 stores its 2nd arg into the base's f8 @0x08. */
    void         *pad10;   /* 0x10 */
    KProfileNode *root;    /* 0x14 tree root */
    /* 0x18: the KGraphics that owns this profile - freeNodes408b40 @0x408b40
       keeps its texture byte/count totals up to date through it. (The method
       @0x414200 calls slot 6 on it, so the owner is polymorphic.) Makes
       sizeof 0x1c, which is the `new KProfile` size in the app init. */
    struct KGraphics *m_owner;  /* 0x18 */

    virtual ~KProfile();                       /* 0x413e20 (slot 0 -> ??_G 0x413e90) */
    virtual const char *getName();             /* 0x413e80 -> "KProfile" */

    KProfile(int size, void *p);               /* 0x413dc0 */
    int Load(char *name);                      /* 0x413fe0 */
    void init();                               /* 0x4143e0 */
    KProfileNode *allocNode(int size);         /* 0x4144a0 */
    KProfileNode *create(char *key, int type, int size); /* 0x4144f0 */
    int setStr(char *key, char *value);        /* 0x413ef0 */
    int  setInt(char *key, int value);         /* 0x413eb0 */
    int writeFile(char *name);                 /* 0x4142b0 */
    /* Walk the tree writing each node out through the KFile parked at +0x18
       (it calls slot 6, KFile::printf). 0x414200 (STUB). */
    int  writeTree(KProfileNode *n);           /* 0x414200 (src/writetree.cpp) */
    KProfileNode *find(char *name);            /* 0x414460 */
    void freeNodes(KProfileNode *n);           /* 0x403f20 (STUB) */
    /* 0x408b40: the same walk over the texture cache, whose nodes carry a
       whole KTexture; returns nonzero once it has emptied a node. */
    int freeNodes408b40(struct KTextureNode *n);
    /* 0x408bf0: the drawing half of the same walk - depth first over the
       same tree, drawing every cached texture at the origin. */
    void drawTree408bf0(struct KTextureNode *n);
    int getInt(char *name, int *out);          /* 0x413f40 */
    int getStr(char *name, char *out);         /* 0x413f70 */
    int getValue(char *name, char **out);      /* 0x413fb0 */
};

#endif /* KPROFILE_H */
