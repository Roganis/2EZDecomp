/* The body of SongSelectDirector::drawWheel @0x433180 and of its two
   ICF-folded per-mode twins (src/songselect.cpp). Included INSIDE each
   function's braces; it is not a standalone header. */
    int i;
    float pos;
    Obj402860 *t;

    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    for (i = 0; i < f7c; i++)
    {
        pos = (float)(i * 100) - f_c4;
        if (pos > (float)(f7c * 50))
            pos -= (float)(f7c * 100);
        if ((float)(-f7c * 50) > pos)
            pos = (float)(f7c * 100) + pos;
        if (pos > -150.0f && 150.0f > pos)
            ((Obj432AC0 *)this)->drawEntry(i, pos);
    }
    m_bcaac.m403940();
    m_be6bc.m403940();
    t = &m_20d7b8;
    t->m403940();
    if (t->isDone())
        t->seek(15);
    if (f_20c0e0 == 1)
        m_20d380.m403940();
    else if (f_20c0e0 == 0)
        m_20d164.m403940();
    drawRankTable(120.0f, 89.0f);
