#ifndef KPARSER_H
#define KPARSER_H

/* KParser: the config/token parser (1-entry vtable @0x488b28, pinned in
   src/kparser.cpp) with an embedded 0x4000-byte scratch buffer. Field layout
   proven by the buffer ctor @0x4138c0 and setBuffer @0x413910. */
struct KParser
{
    char         *cursor;         /* 0x04 read position */
    char         *base;           /* 0x08 parse pointer / text */
    char          quote;          /* 0x0c quote char (0x27) */
    char          _pad0[3];
    char         *delims;         /* 0x10 -> g_parserDelims */
    int          *table;          /* 0x14 -> g_parserTable */
    char          buf[0x4000];    /* 0x18 embedded scratch buffer */
    int           size;           /* 0x4018 buffer size */
    int           f401c;          /* 0x401c line counter (parseToken) */
    int           ttype;          /* 0x4020 token type: 3=INT 4=FLOAT 5=STR */
    int           lval;           /* 0x4024 int value of the token */
    float         fval;           /* 0x4028 float value of the token */
    char          tok[0x1000];    /* 0x402c token text; size proven by
                                     KProfile::Load's frame (parser spans
                                     0x502c bytes up to the /GS cookie) */

    KParser();
    virtual ~KParser();
    void setBuffer(char *p, int n);
    int  getField(char *key);
    int  seek(char *key);
    int  slurpFile(char *name);   /* 0x4135f0 decrypt .ini -> .int temp */
    int  m4133d0(char *name);     /* 0x4133d0 the encrypt direction */
    int  readFile(char *name);    /* 0x413bc0 load a config file into buf */
    unsigned parseToken(int expected); /* 0x413940 tokenizer */
    int  readGdsString(char *out);/* 0x456170 - copy the next string token out */
};

/* The two .gds readers the mode config loader drives (src/moderunner.cpp): each
   takes the destination first and the parser second, and pulls its value out
   with KParser::parseToken. */
int  readGdsSection(char *out, KParser *p);  /* 0x4561b0 */
int  readGdsInt(int *out, KParser *p);       /* 0x456270 */
/* The rest of the family, all the same skeleton - eat the `=` (token 2) and the
   name (token 0xc), then one value per comma (token 0xa). A float value is
   token 4 and is taken unconditionally; an int is token 3 and is only taken
   when the token really was one. */
int  readGdsFloat(float *out, KParser *p);              /* 0x456230 */
int  readGdsFloat2(float *a, float *b, KParser *p);     /* 0x456590 */
int  readGdsInt2(int *a, int *b, KParser *p);           /* 0x456600 */
int  readGdsColor(unsigned int *out, KParser *p);       /* 0x4562c0 */
int  readGdsTexture(struct KTexture **out, struct KGraphics *g, KParser *p);
                                                        /* 0x4563a0 */
/* N comma-separated ints into an array. */
int  readGdsIntN(int *out, int count, KParser *p);      /* 0x456670 */
/* A .str resource by name; an empty name clears the slot. */
int  readGdsStr(struct Obj402860Head **out, struct KGraphics *g, KParser *p);
                                                        /* 0x456500 */
/* A numbered run of textures - <name>0.bmp .. <name>(count-1).bmp - with the
   base name read from the parser (0x456430) or handed in (0x456010). */
int  readGdsTexSeries(struct KTexture **dst, int count, struct KGraphics *g,
                      KParser *p);                      /* 0x456430 */
int  loadTexSeries(struct KTexture **dst, int count, struct KGraphics *g,
                   char *base);                         /* 0x456010 */
/* Copy the next token's text into `out`, leaving it empty when there is none. */
int  readGdsToken(char *out, KParser *p);               /* 0x4561e0 */

#endif /* KPARSER_H */
