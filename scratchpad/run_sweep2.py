import sys; sys.path.insert(0,'scratchpad')
from sweep import score
from gen_quad import gen
best=(0,None)
for c in 'ABC':
  for e in 'ABC':
    for f in 'ABC':
      p,b=score(gen(c,e,f,'B','B'), "%s%s%sBB"%(c,e,f))
      if p>best[0]: best=(p,(c,e,f))
print("BEST", best, flush=True)
