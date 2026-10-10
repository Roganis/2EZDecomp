import sys; sys.path.insert(0,'scratchpad')
from sweep import score
from gen_quad import gen
best=(0,None)
for c in 'ABC':
  for e in 'ABC':
    for f in 'ABC':
      for a in 'AB':
        for r in 'AB':
          p,b=score(gen(c,e,f,a,r), "%s%s%s%s%s"%(c,e,f,a,r))
          if p>best[0]: best=(p,(c,e,f,a,r))
print("BEST", best, flush=True)
