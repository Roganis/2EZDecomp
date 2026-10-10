/* KGraphics' spline tube - "the worm" - the 0x409330..0x40a46c cluster, ONE
   translation unit by construction: two public methods (createWorm @0x409d90,
   the per-frame m40a300 @0x40a300), the draw @0x409480 and freeBuffers
   @0x409420, over six `static` helpers that take their arguments in whatever
   registers cl chose for them (see wip/mesh-tu-409000.md), all sharing the
   one static WormMesh @0xd49cb8.

   What it builds: fifty control points on a helix (mode 1, "TWIST") or a
   straight line, a Catmull-Rom curve through them, and a tube of `rings`
   vertices per ring, twenty-one rings, two triangles per quad. Each frame the
   eye advances along the curve, the tube is scrolled a ring at a time behind
   it, and a look-at view matrix plus a point light at the eye are rebuilt. */

#include <math.h>
#include <string.h>
#include "kcrt.h"
#include "ez2acapp.h"
#include "icom.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */

/* src/accessors.cpp / src/com_wrappers.cpp model the same object under other
   names for the two camera helpers the draw calls first. */
struct Obj40BBB0 { char _pad[0x140]; float m140[16]; void loadIdentity(); };
struct Obj40BB00 { void doSub(); };

struct WormVertex               /* FVF 0x112 = XYZ | NORMAL | TEX1 */
{
    GVec3 pos;
    GVec3 nrm;
    float u, v;
};

struct D3DMatrix
{
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

struct D3DColorValue { float r, g, b, a; };

struct D3DMaterial7             /* 0x44 bytes */
{
    D3DColorValue diffuse;
    D3DColorValue ambient;
    D3DColorValue specular;
    D3DColorValue emissive;
    float power;
};

struct D3DLight7                /* 0x68 bytes */
{
    int   type;
    D3DColorValue diffuse;
    D3DColorValue specular;
    D3DColorValue ambient;
    GVec3 position;
    GVec3 direction;
    float range;
    float falloff;
    float attenuation0;
    float attenuation1;
    float attenuation2;
    float theta;
    float phi;
};

struct WormMesh
{
    int   alive;                /* 0x000 */
    WormVertex *verts;          /* 0x004 */
    unsigned short *indices;    /* 0x008 */
    GVec3 *ctrl;                /* 0x00c */
    GVec3 eye;                  /* 0x010 */
    GVec3 eyeDir;               /* 0x01c */
    GVec3 eyeUp;                /* 0x028 */
    float t;                    /* 0x034 - where the eye is on the curve */
    GVec3 center;               /* 0x038 - the newest ring's centre */
    GVec3 tangent;              /* 0x044 */
    GVec3 normal;               /* 0x050 */
    float tHead;                /* 0x05c - where the newest ring is */
    unsigned short segCursor;   /* 0x060 */
    unsigned short segCursor2;  /* 0x062 */
    D3DMaterial7 material;      /* 0x064 */
    D3DLight7 light;            /* 0x0a8 */
    int   rings;                /* 0x110 - vertices per ring */
    int   segments;             /* 0x114 - 20 */
    int   subdiv;               /* 0x118 - 5 */
    int   nverts;               /* 0x11c */
    int   ntris;                /* 0x120 */
    float radius;               /* 0x124 */
    int   nctrl;                /* 0x128 - 50 */
    float twist;                /* 0x12c - 5.0 */
    float step;                 /* 0x130 - 0.05 per ring */
    float speed;                /* 0x134 */
    float lead;                 /* 0x138 - 0.8 */
    float wrap;                 /* 0x13c - 25.0 */
};

// GLOBAL: EZ2AC_UNPACKED 0x00d49cb4
static unsigned short g_wormSeg;

// GLOBAL: EZ2AC_UNPACKED 0x00d49cb8
static WormMesh g_worm;

/* The worm's view matrix, right after the mesh (0xd49cb8 + 0x140). Its own
   object rather than a member: the draw pushes its address as an immediate,
   which only resolves at an entity's base. */
// GLOBAL: EZ2AC_UNPACKED 0x00d49df8
static D3DMatrix g_wormView;

static float dot(const GVec3 *a, const GVec3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

static GVec3 vec3(float x, float y, float z)
{
    GVec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static void cross(GVec3 *out, const GVec3 *a, const GVec3 *b)
{
    GVec3 v;
    v.x = a->y * b->z - b->y * a->z;
    v.y = b->x * a->z - a->x * b->z;
    v.z = b->y * a->x - b->x * a->y;
    *out = v;
}

/* Two triangles per ring between ring `lo` and ring `hi`, six indices each,
   into `out`. Sixteen-bit arithmetic throughout. */
// FUNCTION: EZ2AC_UNPACKED 0x00409330
static void buildIndices(WormMesh *m, unsigned short hi, unsigned short lo,
                         unsigned short *out)
{
    unsigned short i, j;

    for (j = 0, i = 0; j < m->rings; j++)
    {
        unsigned short *p = &out[i * 3];
        i++;
        p[0] = m->rings * hi + j;
        p[1] = m->rings * lo + j;
        p[2] = (j + 1) % m->rings + m->rings * lo;
        p = &out[i * 3];
        i++;
        p[1] = (j + 1) % m->rings + m->rings * lo;
        p[2] = (j + 1) % m->rings + m->rings * hi;
        p[0] = m->rings * hi + j;
    }
}

/* Catmull-Rom: the four basis weights of t, then the weighted sum of the four
   control points. */
// FUNCTION: EZ2AC_UNPACKED 0x004094f0
static GVec3 *evalSpline(float t, GVec3 *out, GVec3 **cp)
{
    GVec3 r;
    float w[4];
    float t2, t3;
    unsigned short k;

    r.x = 0.0f;
    r.y = 0.0f;
    r.z = 0.0f;
    t2 = t * t;
    t3 = t * t * t;
    {
        float a, b, c;
        a = -1.0f * t3; b = 2.0f * t2; c = -1.0f * t;
        w[0] = (a + b + c + 0.0f) * 0.5f;
        a = 3.0f * t3; b = -5.0f * t2; c = 0.0f * t;
        w[1] = (a + b + c + 2.0f) * 0.5f;
        a = -3.0f * t3; b = 4.0f * t2; c = 1.0f * t;
        w[2] = (a + b + c + 0.0f) * 0.5f;
        a = 1.0f * t3; b = -1.0f * t2; c = 0.0f * t;
        w[3] = (a + b + c + 0.0f) * 0.5f;
    }
    for (k = 0; k < 4; k++)
    {
        r.x += cp[k]->x * w[k];
        r.y += cp[k]->y * w[k];
        r.z += cp[k]->z * w[k];
    }
    *out = r;
    return out;
}

/* One ring of `rings` vertices around `center`, in the plane of the normal
   and the binormal, radius `m->radius`, normals pointing inward, u running
   once round, v as given. */
// FUNCTION: EZ2AC_UNPACKED 0x00409660
static void addRing(WormMesh *m, GVec3 *center, GVec3 *tan, GVec3 *nrm,
                    float v, WormVertex *out)
{
    GVec3 b;
    unsigned short i;

    cross(&b, nrm, tan);

    for (i = 0; i < m->rings; i++)
    {
        WormVertex *o = &out[i];
        float a = (float)i * 6.2831855f / m->rings;
        float s = (float)sin(a);
        GVec3 sv;
        sv.x = nrm->x * s;
        sv.y = nrm->y * s;
        sv.z = nrm->z * s;
        float c = (float)cos(a);
        GVec3 d;
        d.x = b.x;
        d.y = b.y;
        d.z = b.z;
        d.x *= c;
        d.y *= c;
        d.z *= c;
        d.x += sv.x;
        d.y += sv.y;
        d.z += sv.z;
        o->pos.x = center->x + m->radius * d.x;
        o->pos.y = center->y + m->radius * d.y;
        o->pos.z = center->z + m->radius * d.z;
        o->nrm.x = 0.0f - d.x;
        o->nrm.y = 0.0f - d.y;
        o->nrm.z = 0.0f - d.z;
        o->u = 1.0f - a * 0.15915494f;
        o->v = v;
    }
}

/* A look-at basis: `dir` normalized is the z axis, `up` made orthogonal to it
   is the y axis, their cross product the x axis; the translation row is left
   for the caller. */
// FUNCTION: EZ2AC_UNPACKED 0x00409830
static void lookAt(D3DMatrix *out, GVec3 *dir, GVec3 *up)
{
    float len;
    GVec3 z, u, x, t;
    float d;

    len = (float)sqrt(dot(dir, dir));
    z.x = 1.0f / len * dir->x;
    z.y = 1.0f / len * dir->y;
    z.z = 1.0f / len * dir->z;
    u = *up;
    d = z.z * u.z;
    d += z.y * u.y;
    d += z.x * u.x;
    t.x = d * z.x;
    t.y = d * z.y;
    t.z = d * z.z;
    t.x = u.x - t.x;
    t.y = u.y - t.y;
    t.z = u.z - t.z;
    len = (float)sqrt(dot(&t, &t));
    u = vec3(1.0f / len * t.x, 1.0f / len * t.y, 1.0f / len * t.z);
    cross(&x, &z, &u);
    out->_11 = x.x;
    out->_22 = u.y;
    out->_21 = x.y;
    out->_14 = 0.0f;
    out->_24 = 0.0f;
    out->_34 = 0.0f;
    out->_31 = x.z;
    out->_12 = u.x;
    out->_32 = u.z;
    out->_13 = z.x;
    out->_23 = z.y;
    out->_33 = z.z;
    out->_44 = 1.0f;
}

/* Where the curve is at t (with t wrapped past the last control point), the
   unit tangent there, and `nrm` re-made perpendicular to it. */
// FUNCTION: EZ2AC_UNPACKED 0x00409a30
static void evalCurve(WormMesh *m, float t, GVec3 *pos, GVec3 *tan, GVec3 *nrm)
{
    float tt = t;
    unsigned short i = 0;
    unsigned short k;
    GVec3 *cp[4];
    GVec3 tmp, d, c, n;
    GVec3 *q;
    float len;

    while (tt > 1.0f)
    {
        i++;
        if (i == m->nctrl) i = 0;
        tt -= 1.0f;
    }
    for (k = 0; k < 4; k++)
    {
        cp[k] = &m->ctrl[i];
        i++;
        if (i == m->nctrl) i = 0;
    }
    *pos = *evalSpline(tt, &tmp, cp);
    q = evalSpline(tt - 0.01f, &tmp, cp);
    d.x = pos->x - q->x;
    d.y = pos->y - q->y;
    d.z = pos->z - q->z;
    len = (float)sqrt(dot(&d, &d));
    *tan = vec3(1.0f / len * d.x, 1.0f / len * d.y, 1.0f / len * d.z);
    cross(&c, nrm, tan);
    cross(&n, tan, &c);
    len = (float)sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    *nrm = vec3(1.0f / len * n.x, 1.0f / len * n.y, 1.0f / len * n.z);
}

/* Scroll the tube back one ring, grow a new ring at the head, and step the
   head along the curve. */
// FUNCTION: EZ2AC_UNPACKED 0x00409ce0
static void advanceRing(WormMesh *m)
{
    memmove(m->verts + m->rings, m->verts, (m->nverts - m->rings) * sizeof(WormVertex));
    addRing(m, &m->center, &m->tangent, &m->normal,
            (float)g_wormSeg / (float)m->subdiv, m->verts);
    evalCurve(m, m->tHead, &m->center, &m->tangent, &m->normal);
    m->segCursor = (m->segCursor + 1) % (m->segments + 1);
    m->segCursor2 = (m->segCursor2 + 1) % m->segments;
    g_wormSeg = (g_wormSeg + 1) % m->subdiv;
}

// FUNCTION: EZ2AC_UNPACKED 0x00409420
void KGraphics::freeBuffers(void *mesh)
{
    WormMesh *m = (WormMesh *)mesh;

    if (m != 0)
    {
        m->alive = 0;
        if (m->verts != 0)
        {
            delete m->verts;
            m->verts = 0;
        }
        if (m->indices != 0)
        {
            delete m->indices;
            m->indices = 0;
        }
        if (m->ctrl != 0)
        {
            delete m->ctrl;
            m->ctrl = 0;
        }
    }
}

/* Identity world, the worm's own view, the standard projection, and one
   indexed triangle list. */
// FUNCTION: EZ2AC_UNPACKED 0x00409480
void KGraphics::m409480(void *mesh)
{
    WormMesh *m = (WormMesh *)mesh;

    ((Obj40BBB0 *)this)->loadIdentity();
    ((Obj40BB00 *)this)->doSub();
    device->SetTransform(2, &g_wormView);
    setProjection(1.0f, 10000.0f, 0.78539819f);
    device->DrawIndexedPrimitive(4, 0x112, m->verts, m->nverts, m->indices,
                                 m->ntris * 3, 0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00409d90
int KGraphics::createWorm(void **out, int rings, float speed, float radius, int mode)
{
    float tacc;
    unsigned short i, k;

    if (g_worm.alive != 0)
        freeBuffers(&g_worm);
    g_worm.twist = 5.0f;
    g_worm.rings = rings;
    g_worm.nverts = rings * 21;
    g_worm.radius = radius;
    g_worm.step = 0.05f;
    g_worm.speed = speed;
    g_worm.lead = 0.8f;
    g_worm.alive = 1;
    g_worm.segments = 20;
    g_worm.subdiv = 5;
    g_worm.ntris = rings * 40;
    g_worm.nctrl = 50;
    g_worm.wrap = 25.0f;
    g_worm.verts = new WormVertex[g_worm.nverts];
    g_worm.indices = new unsigned short[g_worm.ntris * 3];
    g_worm.ctrl = new GVec3[g_worm.nctrl];
    if (g_worm.verts == 0 || g_worm.indices == 0 || g_worm.ctrl == 0)
        return 0;

    tacc = 0.0f;
    if (mode == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x00487f94
        Log("TWIST");
        for (i = 0; i < g_worm.nctrl; i++)
        {
            float a = (float)i * g_worm.twist * 3.1415927f / (float)g_worm.nctrl;
            float b = a * 32.0f;
            g_worm.ctrl[i].x = (float)cos(b) * 20.0f;
            g_worm.ctrl[i].y = (float)sin(b) * 20.0f;
            g_worm.ctrl[i].z = (float)cos(a + a) * 20.0f;
            // STRING: EZ2AC_UNPACKED 0x00487f80
            Log("%d: %f, %f", i, a, g_worm.ctrl[i].z);
        }
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00487f78
        Log("Straite");
        for (i = 0; i < g_worm.nctrl; i++)
        {
            float a = (float)i * g_worm.twist * 3.1415927f / (float)g_worm.nctrl;
            g_worm.ctrl[i].x = 0.0f;
            g_worm.ctrl[i].y = 0.0f;
            g_worm.ctrl[i].z = (float)cos(a + a) * 20.0f;
            Log("%d: %f, %f", i, a, g_worm.ctrl[i].z);
        }
    }

    g_worm.normal = vec3(0.0f, 1.0f, 0.0f);
    for (k = 0; k < g_worm.segments + 1; k++)
    {
        evalCurve(&g_worm, tacc, &g_worm.center, &g_worm.tangent, &g_worm.normal);
        tacc += g_worm.step;
        addRing(&g_worm, &g_worm.center, &g_worm.tangent, &g_worm.normal,
                (float)(k % g_worm.subdiv) / (float)g_worm.subdiv,
                g_worm.verts + (g_worm.segments - k) * g_worm.rings);
    }
    for (k = 0; k < g_worm.segments; k++)
        buildIndices(&g_worm, k + 1, k, g_worm.indices + k * g_worm.rings * 6);

    g_worm.eyeUp = vec3(0.0f, 1.0f, 0.0f);
    evalCurve(&g_worm, 0.0f, &g_worm.eye, &g_worm.eyeDir, &g_worm.eyeUp);
    g_worm.tHead = tacc;
    g_worm.segCursor = 0;
    g_worm.segCursor2 = 0;
    g_worm.t = 0.0f;

    memset(&g_worm.material, 0, sizeof(g_worm.material));
    g_worm.material.ambient.r = g_worm.material.diffuse.r = 1.0f;
    g_worm.material.ambient.g = g_worm.material.diffuse.g = 1.0f;
    g_worm.material.ambient.b = g_worm.material.diffuse.b = 1.0f;
    g_worm.material.ambient.a = g_worm.material.diffuse.a = 1.0f;

    memset(&g_worm.light, 0, sizeof(g_worm.light));
    g_worm.light.type = 1;
    g_worm.light.position = g_worm.eye;
    g_worm.light.diffuse.r = 1.0f;
    g_worm.light.diffuse.g = 1.0f;
    g_worm.light.diffuse.b = 1.0f;
    g_worm.light.ambient.r = 1.0f;
    g_worm.light.ambient.g = 1.0f;
    g_worm.light.ambient.b = 1.0f;
    g_worm.light.specular.r = 0.0f;
    g_worm.light.specular.g = 0.0f;
    g_worm.light.specular.b = 0.0f;
    g_worm.light.attenuation2 = 0.05f;

    *out = &g_worm;
    return 1;
}

/* Per frame: the eye moves `speed` along the curve (wrapping at `wrap`),
   the tube grows a ring whenever the head is less than `lead` ahead, and the
   view matrix and the light follow the eye. */
// FUNCTION: EZ2AC_UNPACKED 0x0040a300
void KGraphics::m40a300(void *mesh)
{
    WormMesh *m = (WormMesh *)mesh;

    m->t += m->speed;
    if (m->t > m->wrap)
    {
        m->t -= m->wrap;
        m->tHead -= m->wrap;
    }
    GVec3 *e = &m->eye;
    float f;

    evalCurve(m, m->t, e, &m->eyeDir, &m->eyeUp);
    if (m->lead > m->tHead - m->t)
    {
        m->tHead += m->step;
        advanceRing(m);
    }
    lookAt(&g_wormView, &m->eyeDir, &m->eyeUp);
    f = e->x * g_wormView._11;
    f += e->y * g_wormView._21;
    f += e->z * g_wormView._31;
    g_wormView._41 = 0.0f - f;
    f = e->x * g_wormView._12;
    f += e->y * g_wormView._22;
    f += e->z * g_wormView._32;
    g_wormView._42 = 0.0f - f;
    f = e->x * g_wormView._13;
    f += e->y * g_wormView._23;
    f += e->z * g_wormView._33;
    g_wormView._43 = 0.0f - f;
    m->light.position = *e;
}
