import sys
f=open('original/EZ2AC_Unpacked.exe','rb').read()
for a in sys.argv[1:]:
    v=int(a,16); o=v-0x400000
    e=f.index(b'\0',o)
    print('%x %r'%(v,f[o:e].decode('latin-1')))
