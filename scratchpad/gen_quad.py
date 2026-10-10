def gen(cropOrder, elseOrder, commonOrder, armOrder, rhwOrder, bkind='volatile float b;'):
    crops={
      'A':"""        su = tu * u;
        sv = tv * v;
        su = su + tex->u0;
        sv = sv + tex->v0;
        du = tu * uw;
        dv = tv * vh;""",
      'B':"""        su = tu * u;
        du = tu * uw;
        su = su + tex->u0;
        sv = tv * v;
        dv = tv * vh;
        sv = sv + tex->v0;""",
      'C':"""        su = tu * u;
        sv = tv * v;
        du = tu * uw;
        dv = tv * vh;
        su = su + tex->u0;
        sv = sv + tex->v0;""",
    }
    elses={
      'A':"        su = u;\n        du = uw;\n        sv = v;\n        dv = vh;",
      'B':"        dv = vh;\n        du = uw;\n        sv = v;\n        su = u;",
      'C':"        dv = vh;\n        sv = v;\n        du = uw;\n        su = u;",
    }
    fields={
      'A':"""    q[0].x = x;
    q[1].x = x;
    q[2].x = x + w;
    q[3].x = x + w;
    q[0].u = su;
    q[1].u = su;
    q[2].u = su + du;
    q[3].u = su + du;
    q[0].diffuse = cBottom;
    q[0].specular = cBottom;
    q[2].diffuse = cBottom;
    q[2].specular = cBottom;
    q[1].diffuse = cTop;
    q[1].specular = cTop;
    q[3].diffuse = cTop;
    q[3].specular = cTop;""",
      'B':"""    q[0].diffuse = cBottom;
    q[0].specular = cBottom;
    q[2].diffuse = cBottom;
    q[2].specular = cBottom;
    q[1].diffuse = cTop;
    q[1].specular = cTop;
    q[3].diffuse = cTop;
    q[3].specular = cTop;
    q[0].x = x;
    q[1].x = x;
    q[2].x = x + w;
    q[3].x = x + w;
    q[0].u = su;
    q[1].u = su;
    q[2].u = su + du;
    q[3].u = su + du;""",
      'C':"""    q[0].diffuse = cBottom;
    q[0].specular = cBottom;
    q[2].diffuse = cBottom;
    q[2].specular = cBottom;
    q[0].x = x;
    q[1].x = x;
    q[0].u = su;
    q[1].u = su;
    q[2].u = su + du;
    q[3].u = su + du;
    q[2].x = x + w;
    q[3].x = x + w;
    q[1].diffuse = cTop;
    q[1].specular = cTop;
    q[3].diffuse = cTop;
    q[3].specular = cTop;""",
    }
    armsPlain={
      'A':"""        q[0].y = y + h;
        q[2].y = y + h;
        q[1].y = y;
        q[3].y = y;
        q[0].v = sv + dv;
        q[2].v = sv + dv;
        q[1].v = sv;
        q[3].v = sv;""",
      'B':"""        q[2].y = y + h;
        q[0].y = y + h;
        q[3].y = y;
        q[1].y = y;
        q[2].v = sv + dv;
        q[0].v = sv + dv;
        q[3].v = sv;
        q[1].v = sv;""",
    }
    armsFlip={
      'A':"""        b = y + h;
        q[0].y = 479.0f - b;
        q[2].y = 479.0f - b;
        q[1].y = 479.0f - y;
        q[3].y = 479.0f - y;
        q[0].v = sv;
        q[2].v = sv;
        q[1].v = sv + dv;
        q[3].v = sv + dv;""",
      'B':"""        b = y + h;
        q[2].y = 479.0f - b;
        q[0].y = 479.0f - b;
        q[3].y = 479.0f - y;
        q[1].y = 479.0f - y;
        q[2].v = sv;
        q[0].v = sv;
        q[3].v = sv + dv;
        q[1].v = sv + dv;""",
    }
    rhws={
      'A':"    q[0].rhw = 1.0f;\n    q[1].rhw = 1.0f;\n    q[2].rhw = 1.0f;\n    q[3].rhw = 1.0f;",
      'B':"    q[3].rhw = 1.0f;\n    q[2].rhw = 1.0f;\n    q[1].rhw = 1.0f;\n    q[0].rhw = 1.0f;",
    }
    return """#include "ez2acapp.h"
#include "icom.h"

struct QuadVert
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int diffuse;
    unsigned int specular;
    float u;
    float v;
};

// GLOBAL: EZ2AC_UNPACKED 0x004a5198
QuadVert g_quadVerts[4];

extern int g_d36cb0;

// FUNCTION: EZ2AC_UNPACKED 0x00404950
void KGraphics::m404950(KTexture *tex, float x, float y, float w, float h,
                        float u, float v, float uw, float vh,
                        unsigned int cTop, unsigned int cBottom)
{
    QuadVert q[4];
    float su;
    float sv;
    float du;
    float dv;
    %s

    if (w == 0.0f)
        w = tex->w;
    if (h == 0.0f)
        h = tex->h;
    if (tex->f88 != 0)
    {
        float tu = tex->u1 - tex->u0;
        float tv = tex->v1 - tex->v0;

%s
    }
    else
    {
%s
    }
%s
    if (g_d36cb0 == 0)
    {
%s
    }
    else
    {
%s
    }
%s
    m4080c0(0, tex);
    g_quadVerts[0].x = g_quadVerts[0].x - 0.5f;
    g_quadVerts[0].y = g_quadVerts[0].y - 0.5f;
    g_quadVerts[1].x = g_quadVerts[1].x - 0.5f;
    g_quadVerts[1].y = g_quadVerts[1].y - 0.5f;
    g_quadVerts[2].x = g_quadVerts[2].x - 0.5f;
    g_quadVerts[2].y = g_quadVerts[2].y - 0.5f;
    g_quadVerts[3].x = g_quadVerts[3].x - 0.5f;
    g_quadVerts[3].y = g_quadVerts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, q, 4, 0);
}
""" % (bkind, crops[cropOrder], elses[elseOrder], fields[commonOrder],
       armsPlain[armOrder], armsFlip[armOrder], rhws[rhwOrder])
