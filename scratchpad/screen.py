#!/usr/bin/env python3
"""screen.py LO SIZE [LO SIZE ...] - is this function tick-sized to WRITE?

The two ranking metrics in PORT-WORKQUEUE.md - branch density and calls per
instruction - both predict TRANSCRIPTION cost, and both mis-sized three
targets in a row because what actually costs time is READING:

  * FLOAT MATH. A run of movss/comiss/cvtsi2ss has to be read expression by
    expression. 0x42a630 looked ideal at 0.090 branches/insn and is the
    hold-note renderer's geometry; 0x45bc30 looked ideal at 0.106 calls/insn
    and is a float score count-up with easing.
  * A MODE LADDER. Runs of cmp/je against one register, each arm calling
    something different. 0x4249e0 scored 0.136 calls/insn - the top of the
    queue - and is a lamp-routing ladder over eight call sites.

So screen for both before committing a tick to a target. A function with no
float ops and no ladder really is a table of calls and really does get
written in one sitting; the four that passed this screen were written first
try at 73-91%.
"""
import subprocess, re, sys

FLOAT = re.compile(r'\b(movss|comiss|ucomiss|cvtsi2ss|cvttss2si|mulss|divss'
                   r'|addss|subss|fld|fstp|fmul|fdiv|fadd|fsub)\b')

def look(a, size):
    p = subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel',
        '--no-show-raw-insn','--adjust-vma=0x400000','--start-address=%#x'%a,
        '--stop-address=%#x'%(a+size),'original/EZ2AC_Unpacked.exe'],
        capture_output=True, text=True)
    ins = [l.split('\t',1)[1].strip() for l in p.stdout.splitlines()
           if '\t' in l and re.match(r'^\s*[0-9a-f]+:', l)]
    n = len(ins) or 1
    fl = sum(1 for t in ins if FLOAT.search(t))
    ca = sum(1 for t in ins if t.startswith('call'))
    lad = sum(1 for i in range(len(ins)-5)
              if sum(1 for t in ins[i:i+6] if t.startswith('cmp ')) >= 3
              and sum(1 for t in ins[i:i+6] if t.startswith('je ')) >= 3)
    return n, ca, fl, lad

args = sys.argv[1:]
print("%-10s %6s %6s %5s %6s %5s  %s" %
      ("addr","bytes","insns","calls","float","ladd","verdict"))
for i in range(0, len(args), 2):
    a, size = int(args[i], 16), int(args[i+1], 0)
    n, ca, fl, lad = look(a, size)
    v = ("FLOAT-HEAVY - sitting" if fl/n >= 0.10 else
         "LADDER - sitting"      if lad >= 3 else
         "table - tick-sized")
    print("0x%06x %6d %6d %5d %6d %5d  %s" % (a, size, n, ca, fl, lad, v))
