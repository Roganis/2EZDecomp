/* The body of SongSelectDirector::m439f00 @0x439f00 and of its seven
   ICF-folded per-mode twins (src/songselectpage.cpp). Included INSIDE each
   function's braces; it is not a standalone header. */
    int   i;
    float d;
    float s;

    for (i = 0; i < f7c; i++)
    {
        d = (float)(i * 100) - f_c4;
        if (d > (float)(f7c * 50))
            d -= (float)(f7c * 100);
        if ((float)(-f7c * 50) > d)
            d = (float)(f7c * 100) + d;
        d = d * 2.0f;
        if (800.0f > d && d > -400.0f)
        {
            int idx;
            int bright;

            if (100.0f > d && d > -100.0f)
                bright = 255 - abs((int)d);
            else
                bright = 0x9b;
            if (d >= 200.0f || -200.0f >= d)
            {
                s = 112.0f;
            }
            else if (d >= 0.0f)
            {
                s = 176.0f / (1.0f + d * 0.01f);
            }
            else
            {
                s = 176.0f / (1.0f - d * 0.01f);
            }
            idx = (int)d + 0x34c;
            if (idx < 0)
                idx = -0x34c - (int)d;
            idx = idx & 0x3ff;
            m431ae0(i, angleToY(d) - 274.0f,
                    444.0f - g_cosTable[idx] * 500.0f - 500.0f + 480.0f - 56.0f + 88.0f,
                    s, bright);
        }
    }

    m438d30();
    m_bccc8.m403940();
    m_be8d8.setCell(1, 0,
        f_bff64[*(int *)((char *)this + 0x49c + f84 * 0x374) + 4]);
    if (m_be8d8.isDone())
    {
        m_be8d8.seek(0x78);
        m_be8d8.m403940();
        return;
    }
    m_be8d8.m403940();
