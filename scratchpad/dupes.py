#!/usr/bin/env python3
"""dupes.py - every address annotated more than once in src/.

Two entities on one address is the quietest failure mode in this tree: the
original side resolves to whichever annotation wins, every caller still
symbolizes the other, and the CALLERS drop a fraction of a percent with
every byte right. It has cost this session two functions already."""
import re, os, collections
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
hits=collections.defaultdict(list)
for root,_,files in os.walk(ROOT+'/src'):
    for f in sorted(files):
        if not f.endswith(('.cpp','.h')): continue
        p=os.path.join(root,f)
        for n,line in enumerate(open(p, errors='ignore'), 1):
            m=re.search(r'//\s*(FUNCTION|STUB|GLOBAL|VTABLE|STRING|LIBRARY):\s*EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)(\s+folded)?', line)
            if m:
                hits[int(m.group(2),16)].append((m.group(1), os.path.basename(p), n, bool(m.group(3))))
bad=0
for a in sorted(hits):
    v=hits[a]
    if len(v)==1: continue
    kinds={k for k,_,_,_ in v}
    folded=all(f for _,_,_,f in v if _ ) if False else any(f for _,_,_,f in v)
    # a `folded` set is legitimate: several source functions share one address
    if folded: continue
    # STRING/GLOBAL duplicates of the same kind are the reccmp "Dropped duplicate" warning
    bad+=1
    print("0x%08x  %s" % (a, "  ".join("%s@%s:%d" % (k,f,n) for k,f,n,_ in v)))
print("%d addresses with a non-folded duplicate annotation" % bad)
