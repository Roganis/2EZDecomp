/* TotalResultDirector::TotalResultDirector @0x454990 - READING COMPLETE
   (loop work, 2026-08-14). 0x113b bytes, ~160 calls, EH prologue, frame
   sub esp,0x154, esi=this, cookie [esp+0x15c] post-push. Ends 0x455acb.
   Regenerate the decode: objdump 0x454990..0x455acb > scratchpad/tres_ctor.asm,
   then decode.py with scratchpad/names.txt.

   The class is fully modeled in include/directors.h (7 members, matched
   teardown @0x4545d0, loadDigits @0x454480 matched). This is the NEXT
   byte-match after ResultDirector (parked at 70.44%, commit 575db93) - far
   more mechanical: 55 setCells, 17 drawLines, 16 loadTextures.

   NEW CALLEE: 0x4544f0 = char *m4544f0(int stage) - the per-stage song-name
   trim (the m450c00 family, but indexed). STUB on TotalResultDirector.

   ---- structure, in order --------------------------------------------------
   - members 0..6 (m_10..m_cb8), vptr 0x496ecc.
   - lamps: m4189e0(1); if (findListA(0)) m418a50(1); if (findListA(1))
     m418af0(1);  [NOTE: unconditional first, unlike ResultDirector's 4/5/8/9
     grouping - no mode test here]
   - f1434 = 0; if (countListA() == 2 || countListC() == 2)
     ((SoundBank *)g_1b2e844)->v0(0);
   - PER-STAGE LOOP i=0..g_elemCount (guarded jl, walker over g_textSlots
     +0x80 [esp+0x14], disc dst f11c4? via ecx-garble - RAW CHECK, songname
     dst f1204[i] via ebx, lv dst ??):
       radio? sprintf(buf28, "system\\disc\\%s.bmp", m4544f0(i))
            : sprintf(buf28, ..., textSlotWalker)
       loadTexture -> disc tex [i]
       sprintf(buf24, "system\\SongName\\%s.bmp", m4544f0(i)); -> f1204[i]
       sprintf(buf24, "system\\SongSelect\\LvFont\\LV%02d.bmp", g_1b2ea50[i]);
       loadTexture -> lv tex [i]     (g_1b2ea50 = per-stage level array, NEW)
   - discsmall\x.bmp x3 -> f140c[3]; Icon_%s (g_modeName) -> f141c;
     AllCombo_not -> f11a0; AllCombo_not_m -> f11a4;
   - lv%02d loop x0x30 (48!) -> walker from &f1348?? RAW CHECK (probably
     f1248[48] with the walker stored at [esp+0x14]); fadeblack -> f11a8;
     result_grad -> f1408.
   - channel tag: modes 6 -> 5radio fmt(0x4970e4)!, 7 -> radio, 8 -> 10radio,
     9 -> 14radio (SEPARATE tests, same lesson as ResultDirector's chain,
     but 6 gets 5radio here, NOT shared with 7!). -> f1344.
     Log("Channel Name : %s", buf60).
   - changeDir("System\\TotalResult", 1);
     radio ? sprintf(buf20,"bg_r.str") + triplet fmt "bg\\TRCourse_bg.spv"
           : sprintf(buf20,"bg.str")   + triplet fmt "bg\\TotalResult.spv"
     m_cb8.m40b370(g, fmt); setField43c(1); m40b8b0(timeGetTime());
     loadStr(buf20,&res); m_10.attach(res,1);
   - TotalResult(_R).str -> m_22c.attach(res,0)  [radio picks _R]
   - radio-only: sprintf(buf20,"TotalR_csName_R.str") (NON-radio arm reuses
     the PREVIOUS buf20 = TotalResult(.str)!? RAW-CHECK: the jne skips the
     sprintf entirely, so non-radio loads buf20's old content -> m_a9c gets
     TotalResult.str content again? Verify which buffer 454e7f loads.)
     -> m_a9c.attach(res,0)
   - TotalResult_sc / totalresult_scR -> m_448.attach(res,0)
   - TotalR_Songname / _R -> m_664.attach(res,0)   (ebx=&m_664 named ptr?)
   - Total_Allcombo / TotalR_Allcombo_R -> m_880.attach(res,0) (edi=&m_880)
   - radio-only float pairs (the SetPair inlined-helper pattern, cookbook):
     0x496f70 (=probably 0.75f/0.5f? dump it) into f214/f218 of m_448, m_664,
     m_880, m_a9c: {m_448.f214=k, .f218=0} x4 - order: m_448, m_664, m_880,
     m_a9c via (esi+0x65c/0x660), (ebx+0x214/218), (edi+0x214/218),
     (esi+0xcb0/0xcb4).
   - THE ALLCOMBO CELL BLOCK on m_880 (edi):
     modes 6/7/8/9 -> arms with (&g_1b2f070)[i] gates i=0..3:
       gate0: setCell(1,0,f11a4); setCell(2,0,*ebp??); setCell(3,0,??)
       gate1: 4,5,6  gate2: 7,8,9  gate3: a,b,c
     modes a/b: NO gates, cells 1..6 then shared 7,8 + 9?? RAW-CHECK the
     third-arg garbles (%ecx) and what ebp points at (0x0(%ebp) loads).
     else (plain): gates 0..2 only, cells 1..3/4..6/7,8 + shared c.
   - grade: m42ef30 on first player (cur from ONE nextListA(&cur) call,
     [esp+0x18], winner idx [esp+0x1c]=0); f1424 = total;
     rec = cur + 0x230 + (g_elemCount<<6);
     pct = radio ? (g_1b2ec00+total+g_1b2ebfc+g_1b2ebf8)*k0x496f48
                 : (g_1b2ec00+total+g_1b2ebf8)*k0x496f4c;   [k = 100/x?]
     r10 = (int)(pct*10) -> [esp+0x14]
     comiss ladder vs pool floats: >=100(0x48dbfc)->10, 0x4911d4->9,
     0x4911e0->8, 0x4911d0->7, 0x48fcbc->6, 0x4911cc->5, 0x4911dc->4,
     0x4911d8->3, 0x4904ac->2, 0x48fd08->1, else 0 -> rec->f30 (grade),
     stored via [eax+0x30] with eax=rec.
     Log("m_nRank Value = %d", rec->f30)?? RAW-CHECK (base_dtor 0x496f34 with
     garbled arg).
     rank art: loadTexture(g_4ae2ec[rec->f30], &f1428??, 0) - dst garble.
   - the per-stage RESULT-ART loop, three variants:
     plain: i=0..2: tex = g_1b2efc0[i]==1 ? g_4ae344[songId?RAW] : direct
            hmm - plain arm: `cmpl $1,g_1b2efc0(%edi4)`; taken ->
            g_4ae344[ecx??]; else -> g_1b2f038[i] used AS THE NAME PTR?!
            RAW-CHECK - dst f11b4[i], i<0xc/4=3.
     radio: i=0..3: g_1b2efc0[i]==1 ? g_4ae344[g_1b2f038[i]]
            : g_4ae318[g_1b2f038[i]] -> f11b4[i]; Log("m_nRank Value = %d",
            g_1b2f038[i]); i<0x10/4=4.
     modes a/b: i=0..2: g_4ae318[g_1b2f038[i]] -> f11b4[i].
   - THE 17 drawLine READOUTS on m_448 (edi=&m_448), fonts f122c (BigFnt,
     ebx) / f1254 (SmlFnt): cells with values from POOL-LOADED coordinates
     (mov eax,[0x496e88] etc - .rdata int/float constants) and globals:
       (edi, 9,  g_1b2ebf8, ?, fontB)   (edi, 0xd, k496e88, ?, fontB)
       (edi, 0x11, g_1b2ec00, k496e90, fontB)
       radio: (edi, 0x3b, g_1b2ec04, ?, fontB)
       mode a: (0x15, k496e60), (0x1c, g_1b2f05c, k496e68), then 0x23 with
               g_1b2f060 ... mode b: g_1b2f098/9c; radio: g_1b2e594/98 with
               EXTRA (0x23, g_1b2e598) inline + k496e78;
       shared: (0x3f, k496e78?), (0x2a, r10[esp+0x14], k496ea0, fontA=f122c)
       session total: mode a: f058[0]+[1]+[2]... a: g_1b2f060+g_1b2f05c+
       g_1b2f058; b: g_1b2f098+9c+94; else f1424 -> (0x2e, that, fontA)
   - the final cells: non-radio: m_448 cells 0x38(f11ac!RAW),0x35,0x36(f11b8),
     0x37(f11bc) then m_664 cells 1..9 from f1204/f1208/f120c + f1410/f1414 +
     f121c/f1220 (disc/name/lv triples); radio: m_448 0x38,0x35(f11b4),
     0x36(f11b8),0x37,0x46(f11c0) then m_664 1..8 + the 9999-clamped
     g_1b2efec -> drawLine(edi,0x47,clamped,k496eb0,?).
   - samples: TotalResult_bgm(1) -> f1190 + immediate vcall slot4(0)!;
     Rank.wav(0) -> f1198; popDirectory(); modeselect\decide(0) -> f119c;
     fadeblack -> f11a8 AGAIN (second load!).

   Buffers: [esp+0x10] res, 0x14 walker/r10, 0x18 cur, 0x1c winner0,
   0x20 bufA(bg/csName), 0x24 bufB(songname/lv/TotalResult...), 0x28 bufC
   (disc), 0x60/0x64 bufD (icon/chantag + Log) - sizes/overlaps from the
   frame 0x154: verify per zoo.

   NEW globals: g_1b2ea50[] (per-stage level), g_1b2efc0[4] (new-record
   flags?), g_1b2ec04, g_1b2e594/g_1b2e598, g_1b2f09c already?, the pool
   coordinate constants 0x496e60/68/78/88/90/a0/b0 (values - dump before
   writing), grade thresholds (pool floats, auto-resolved), g_4ae2ec /
   g_4ae318 / g_4ae344 name-pointer tables.

   NEXT TICK: write src/totalresultctor.cpp from this + raw windows for the
   flagged RAW-CHECK spots; zoo + zoodiff (SYM '??0TotalResultDirector',
   LO 0x454990, HI 0x455acb); STUB m4544f0; keep the ResultDirector lessons:
   named clip pointers per block, separate mode tests, (double) for x87
   casts, buffer sizes from the frame, audit STRING literals against bytes. */
