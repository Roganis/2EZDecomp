#!/usr/bin/env bash
# scratchpad/fmt/run.sh - run the decomp's own FILE-FORMAT code on the game's
# real data and diff it against an independent decoder.
#
# The C is compiled with the project's VC7.1 toolchain and linked against the
# objects ./match.sh left in build/obj, so what runs is exactly the code (and
# the stack/global layout) the matching build has. The reference side is
# ../EZ2REWRITE's Lua decoders (cipher, .ez) or a small Python parser here
# (quoted-token .ini, .gds header). Nothing from the binary or the game is
# stored: key and delimiter tables are read out of original/ at run time.
#
#   ./match.sh                      # once, so build/obj is current
#   scratchpad/fmt/run.sh build     # compile + link the four harnesses
#   scratchpad/fmt/run.sh cipher ez|ezi|ini|rt [N]   # decrypt (rt = slurpFile + m4133d0 round trip)
#   scratchpad/fmt/run.sh load [N]  # decryptFile + loadEZ, per-note dump
#   scratchpad/fmt/run.sh prof      # KProfile::Load on every quoted-token .ini
#   scratchpad/fmt/run.sh gds       # loadModeGds's header read on every .gds
#   scratchpad/fmt/run.sh gdsmode   # the EZ2DJ director's whole-file .gds read (m421190)
#   scratchpad/fmt/run.sh scr [N]   # BackGround::m41b680's .scr script parse (loads recorded)
#   scratchpad/fmt/run.sh ini [N]   # EZ2DJMainGameDirector::loadGds on every chart .ini
#   scratchpad/fmt/run.sh str [N]   # KGraphics::m401b80's .str clip parse (loadTexture recorded)
#
# N limits a run to the first N files (default: all). Env:
#   EZ2GAME     the root of YOUR game install (required)
#   EZ2REWRITE  the sibling project (default ../EZ2REWRITE)
#   OUT         work dir (default ${TMPDIR:-/tmp}/ez2fmt)
#
# Known reference artifacts in `load` (not decomp bugs): a header float of
# exactly x.xxx5 prints differently under MSVC and glibc %.6g, and 8200re's
# charts merge unsorted because lane 0 holds a type-0 record out of tick order
# in the data itself. 5radiomix1p-10harpe-rd3.ez is damaged (lane 26's size
# field is 2 GB) and is skipped on both sides. The 128-byte name buffer in
# the ciphers is the original's: harnesses run from OUT with short relative
# names, as the game does.
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
HERE="$ROOT/scratchpad/fmt"
GAME="${EZ2GAME:?set EZ2GAME to the root of your own game install}"
export EZ2REWRITE="${EZ2REWRITE:-$(cd "$ROOT/../EZ2REWRITE" 2>/dev/null && pwd)}"
OUT="${OUT:-${TMPDIR:-/tmp}/ez2fmt}"
mkdir -p "$OUT"

export WINEPREFIX="$ROOT/tools/wineprefix" WINEDEBUG=-all DISPLAY= WAYLAND_DISPLAY=
towin() { printf 'Z:%s' "$(printf '%s' "$1" | tr '/' '\\')"; }
EXE="$(towin "$ROOT/original/EZ2AC_Unpacked.exe")"

cmd="${1:-}"; shift || true

build() {
    local INC LIBP
    INC="$(towin "$ROOT/tools/msvc71/include");$(towin "$ROOT/tools/platformsdk/Include");$(towin "$ROOT/tools/dx8sdk/include")"
    LIBP="$(towin "$ROOT/tools/msvc71/lib");$(towin "$ROOT/tools/platformsdk/Lib");$(towin "$ROOT/tools/dx8sdk/lib")"
    export INCLUDE="$INC" LIB="$LIBP"
    cd "$ROOT"
    for h in ftcipher ftload ftprof ftgds ftstr ftscr ftini ftgdsmode; do
        # a harness may stand in for whole TUs: ftstr replaces loadtexture.obj
        # (loadTexture needs a DirectDraw device), ftscr also loadstr.obj
        local skip=" "; [ $h = ftstr ] && skip=" loadtexture "; [ $h = ftscr ] && skip=" loadtexture loadstr "
        local objs=() c
        for c in "$ROOT"/src/*.cpp; do
            case "$skip" in *" $(basename "$c" .cpp) "*) continue ;; esac
            objs+=("$(towin "$ROOT/build/obj/$(basename "$c" .cpp).obj")")
        done
        wine "$ROOT/tools/msvc71/bin/cl.exe" /nologo /c /O2 /MT /GS /Gy /GF /Gd /G7 /arch:SSE2 /Zi \
            /Iinclude /Iinclude/dx /EHsc "/Fo$(towin "$OUT/$h.obj")" "/Fd$(towin "$OUT/fmt.pdb")" \
            "$(towin "$HERE/$h.cpp")" 2>&1 | grep -E 'error|warning C'
        wine "$ROOT/tools/msvc71/bin/link.exe" /nologo "/out:$(towin "$OUT/$h.exe")" /subsystem:console \
            /opt:noref /opt:noicf /incremental:no libcpmt.lib libcmt.lib winmm.lib user32.lib gdi32.lib \
            advapi32.lib ole32.lib dinput.lib ddraw.lib dsound.lib "$(towin "$OUT/$h.obj")" "${objs[@]}" \
            2>&1 | grep -v 'warning LNK'
        [ -f "$OUT/$h.exe" ] && echo "built $h"
    done
}

# list FIND-ARGS... : sorted game files into $D/list.txt, honouring $N
list() {
    find "$GAME" "$@" | sort > "$D/list.txt"
    if [ "${N:-0}" != 0 ]; then head -"$N" "$D/list.txt" > "$D/l" && mv "$D/l" "$D/list.txt"; fi
    sed 's|/|\\|g; s|^|Z:|' "$D/list.txt" > "$D/listw.txt"
}

case "$cmd" in
build) build ;;
cipher)
    mode="$1"; N="${2:-0}"; ext=$mode; tbl=$mode
    [ "$mode" = rt ] && { ext=ini; tbl=ini; }
    D="$OUT/cipher_$mode"; rm -rf "$D"; mkdir -p "$D"
    list -iname "*.$ext"
    (cd "$D" && wine "$OUT/ftcipher.exe" "$EXE" "$mode" listw.txt . 2>&1 | grep -v 'starting debugger')
    [ "$mode" != rt ] && luajit "$HERE/ref.lua" "$EZ2REWRITE/bible/data/cipher-tables/$tbl.bin" "$D/list.txt" "$D"
    python3 - "$D" "$mode" <<'PY'
import sys,os
D,mode=sys.argv[1],sys.argv[2]
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
same=diff=miss=0; ex=[]
for i,p in enumerate(L):
    o=D+'/%d.out'%i; ref=p if mode=='rt' else D+'/%d.ref'%i
    if not os.path.exists(o) or not os.path.exists(ref): miss+=1; ex.append(('missing',p)); continue
    if open(o,'rb').read()==open(ref,'rb').read(): same+=1
    else: diff+=1; ex.append(('diff',p))
print('%s: %d identical, %d differ, %d missing'%(mode,same,diff,miss))
for e in ex[:8]: print('  ',*e)
PY
    ;;
load)
    N="${1:-0}"; D="$OUT/load"; rm -rf "$D"; mkdir -p "$D"
    list -iname '*.ez'
    luajit "$HERE/refload.lua" "$D/list.txt" "$D"
    python3 - "$D" <<'PY'
import sys
D=sys.argv[1]; skip=set()
try:
    for l in open(D+'/skip.txt',encoding='utf-8'): skip.add(int(l.split('\t')[0]))
except FileNotFoundError: pass
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
with open(D+'/listw.txt','w',encoding='utf-8') as f:
    for i,p in enumerate(L): f.write(('#' if i in skip else '')+'Z:'+p.replace('/','\\')+'\n')
print(len(skip),'charts skipped as damaged')
PY
    (cd "$D" && wine "$OUT/ftload.exe" "$EXE" listw.txt . 2>&1 | grep -v 'starting debugger')
    python3 - "$D" <<'PY'
import sys,collections
D=sys.argv[1]
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
same=0; diffs=[]
for i,p in enumerate(L):
    a=open(D+'/%d.dump'%i,encoding='latin-1').read(); b=open(D+'/%d.ref'%i,encoding='latin-1').read()
    if a==b: same+=1; continue
    al,bl=a.split('\n'),b.split('\n')
    k=next(j for j in range(max(len(al),len(bl))) if (al[j] if j<len(al) else None)!=(bl[j] if j<len(bl) else None))
    diffs.append((p,k,al[k] if k<len(al) else '<end>',bl[k] if k<len(bl) else '<end>'))
print('%d identical, %d differ'%(same,len(diffs)))
print(collections.Counter((d[2].split(' ')[0],d[3].split(' ')[0]) for d in diffs).most_common(10))
for d in diffs[:10]: print(' ',d)
PY
    ;;
prof)
    D="$OUT/prof"; rm -rf "$D"; mkdir -p "$D"
    (echo "$GAME/Ez2ac.ini"; find "$GAME/system" -iname '*.ini') | sort > "$D/all.txt"
    : > "$D/list.txt"
    while read -r f; do
        c=$(luajit "$HERE/decini.lua" "$f" | tr -d ' \t\r\n' | head -c1)
        [ "$c" = '"' ] && echo "$f" >> "$D/list.txt"
    done < "$D/all.txt"
    sed 's|/|\\|g; s|^|Z:|' "$D/list.txt" > "$D/listw.txt"
    (cd "$D" && wine "$OUT/ftprof.exe" "$EXE" listw.txt 2>&1 | grep -v 'starting debugger')
    python3 "$HERE/profref.py" "$D"
    ;;
gds)
    D="$OUT/gds"; rm -rf "$D"; mkdir -p "$D"
    list -iname '*.gds'
    (cd "$D" && wine "$OUT/ftgds.exe" "$EXE" listw.txt 2>&1 | grep -v 'starting debugger') > "$D/ours.txt"
    python3 - "$D" <<'PY'
import sys,re
D=sys.argv[1]
L=open(D+'/list.txt').read().split('\n')[:-1]; O=open(D+'/ours.txt').read().split('\n')
ok=0
for p,o in zip(L,O):
    t=open(p,encoding='latin-1').read()
    m=re.search(r'\[([^\]]*)\]',t); body=t[m.end():]
    body=body[:body.find('[')] if '[' in body else body
    v=[int(x) for x in re.findall(r'=\s*(-?\d+)',body)][:3]
    ref='%s %d %d %d'%(m.group(1),*v) if len(v)==3 else '?'
    ok+=ref==o
    if ref!=o: print('DIFF',p,'| ours:',o,'| ref:',ref)
print('%d/%d identical'%(ok,len(L)))
PY
    ;;
str)
    N="${1:-0}"; D="$OUT/str"; rm -rf "$D"; mkdir -p "$D"
    list -iname '*.str'
    (cd "$D" && wine "$OUT/ftstr.exe" listw.txt 2>&1 | grep -v 'starting debugger')
    luajit "$HERE/refstr.lua" "$D/list.txt" "$D"
    python3 - "$D" <<'PY'
import sys,collections
D=sys.argv[1]
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
same=0; diffs=[]; padded=0; bmp=0
for i,p in enumerate(L):
    a=open(D+'/%d.dump'%i,encoding='latin-1').read().split('\n')
    b=open(D+'/%d.ref'%i,encoding='latin-1').read().split('\n')
    ok=len(a)==len(b)
    if ok:
        for x,y in zip(a,b):
            if x==y: continue
            if x.startswith('T ') and y.startswith('T '):
                n,r=x[2:],y[2:]
                if n.rstrip(' ')==r or n.split(' ')[0]==r: padded+=1; continue
                if n==r+'.bmp': bmp+=1; continue
            ok=False; diffs.append((p,x[:100],y[:100])); break
    else: diffs.append((p,'lines %d'%len(a),'lines %d'%len(b)))
    same+=ok
print('%d identical, %d differ; texture names: %d space-padded in the file (kept by the original), %d given .bmp'%(same,len(diffs),padded,bmp))
print(collections.Counter((d[1].split(' ')[0],d[2].split(' ')[0]) for d in diffs).most_common(8))
for d in diffs[:8]: print(' ',d)
PY
    ;;
scr)
    N="${1:-0}"; D="$OUT/scr"; rm -rf "$D"; mkdir -p "$D"
    list -iname '*.scr'
    (cd "$D" && wine "$OUT/ftscr.exe" listw.txt 2>&1 | grep -v 'starting debugger')
    luajit "$HERE/refscr.lua" "$D/list.txt" "$D"
    python3 - "$D" <<'PY'
import sys,re,collections
D=sys.argv[1]
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
noext=lambda n: re.sub(r'\.[^.\\/]*$','',n)
same=0; diffs=[]; kinds=collections.Counter()
for i,p in enumerate(L):
    a=open(D+'/%d.dump'%i,encoding='latin-1').read().split('\n')
    b=open(D+'/%d.ref'%i,encoding='latin-1').read().split('\n')
    ok=len(a)==len(b)
    if ok:
        for x,y in zip(a,b):
            if x==y: continue
            xs,ys=x.split(' ',3),y.split(' ',3)
            if xs[0]=='E' and ys[0]=='E' and xs[1:3]==ys[1:3] and noext(xs[3] if len(xs)>3 else '')==noext(ys[3] if len(ys)>3 else ''): continue
            ok=False; kinds[(xs[0],ys[0],xs[2] if len(xs)>2 else '')]+=1; diffs.append((p,x[:90],y[:90])); break
    else: kinds['length']+=1; diffs.append((p,'lines %d'%len(a),'lines %d'%len(b)))
    same+=ok
print('%d identical, %d differ'%(same,len(diffs)))
print(kinds.most_common(8))
for d in diffs[:10]: print(' ',d)
PY
    ;;
ini)
    N="${1:-0}"; D="$OUT/ini"; rm -rf "$D"; mkdir -p "$D"
    list -ipath '*/Sound/*' -iname '*.ini'
    (cd "$D" && wine "$OUT/ftini.exe" "$EXE" listw.txt 2>&1 | grep -v 'starting debugger')
    python3 "$HERE/iniref.py" "$D"
    ;;
gdsmode)
    D="$OUT/gdsmode"; rm -rf "$D"; mkdir -p "$D"
    list -iname '*.gds'
    (cd "$D" && wine "$OUT/ftgdsmode.exe" "$EXE" listw.txt 2>&1 | grep -v 'starting debugger')
    python3 "$HERE/gdsref.py" "$D"
    ;;
*) sed -n '2,32p' "$0"; exit 2 ;;
esac
