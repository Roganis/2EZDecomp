#!/usr/bin/env python3
"""frame.py LO HI [saves] - disassemble a function and rewrite every [esp+X] as a
frame-relative slot. F = esp at entry (pointing at the return address), so
the arguments are F+4, F+8 ... and the locals are negative. Assumes every
`call` is stdcall/thiscall (the callee pops what was pushed since the last
call), which is true of the game's own code; a cdecl call would show up as a
following `add esp,N` and is handled by the lookahead below.

CAVEAT: the walk is LINEAR, so a block reached by a jump from ABOVE an
early exit's pops is printed with the esp that fell out of those pops -
every slot in it reads high by the saves. Two exits reading the same
out-pointer will look like two different arguments. Check the raw
displacements before believing a second exit's frame offsets."""
import re, subprocess, sys
LO=int(sys.argv[1],16); HI=int(sys.argv[2],16)
# addresses of `push <reg>` that are REGISTER SAVES, not call arguments - the
# tool cannot tell them apart, and one mis-classified save shifts every later
# slot. Pass them comma-separated as argv[3].
SAVES=set(int(x,16) for x in sys.argv[3].split(',')) if len(sys.argv)>3 else set()
# argv[4], when given, is a PE to read instead of the raw original - pass the
# recomp build to normalize OUR frame the same way and compare slot for slot.
if len(sys.argv)>4:
    cmd=['objdump','-d','-Mintel','--start-address=0x%x'%LO,'--stop-address=0x%x'%HI,
         sys.argv[4]]
else:
    cmd=['objdump','-D','-b','binary','-m','i386','-Mintel','--adjust-vma=0x400000',
         '--start-address=0x%x'%LO,'--stop-address=0x%x'%HI,'original/EZ2AC_Unpacked.exe']
p=subprocess.run(cmd, capture_output=True,text=True)
esp=0; pending=0
rows=[]
for l in p.stdout.splitlines():
    m=re.match(r'^\s*([0-9a-f]+):\s*(?:[0-9a-f]{2} )+\s*(.+)$', l)
    if m:
        ins=m.group(2).strip()
        if ins.startswith('int3'): break
        rows.append((int(m.group(1),16), ins))
for n,(a,ins) in enumerate(rows):
    # a __cdecl call cleans its own arguments with the `add esp,N` that
    # follows; letting the callee-pops rule fire too would count them twice
    cdecl_next = ins.startswith('call') and n+1 < len(rows) and re.match(r'^add\s+esp,0x[0-9a-f]+$', rows[n+1][1])
    def slot(mm):
        off=int(mm.group(1),16) if mm.group(1) else 0
        v=esp+off
        return '[F%+#x]'%v if v else '[F]'
    shown=re.sub(r'\[esp(?:\+(0x[0-9a-f]+))?\]', slot, ins)
    print('  %06x: %-46s esp=F%+#x'%(a, shown, esp))
    mo=re.match(r'^sub\s+esp,(0x[0-9a-f]+)$', ins)
    if mo: esp-=int(mo.group(1),16); continue
    mo=re.match(r'^add\s+esp,(0x[0-9a-f]+)$', ins)
    if mo: esp+=int(mo.group(1),16); pending=0; continue
    if ins.startswith('push'):
        esp-=4
        if a not in SAVES: pending+=4
        continue
    if ins.startswith('pop'):  esp+=4; continue
    if ins.startswith('call'):
        if not cdecl_next: esp+=pending
        pending=0
        continue
    if ins.startswith('ret'):  esp=0; pending=0; continue
