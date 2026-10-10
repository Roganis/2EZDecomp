# score.py - the permuter's scorer: one function of a COFF .obj vs the
# original's bytes at its address. Register choice, scheduling and layout are
# what we are scoring, so those stay in; addresses are masked (a relocated
# field on our side matches any image-range number on the original's side).
#
# Nothing here is ever written to disk: the original's bytes are read live
# from original/ and only scores leave this module.
import os, re, struct, sys, collections

# scratchpad/dis.py would shadow the stdlib `dis` capstone imports
_here = os.path.dirname(os.path.abspath(__file__))
sys.path = [p for p in sys.path
            if os.path.abspath(p or '.') != os.path.dirname(_here)]
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_REG, X86_OP_IMM, X86_OP_MEM

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ORIG = ROOT + '/original/EZ2AC_Unpacked.exe'
IMAGE_LO, IMAGE_HI = 0x400000, 0x2000000   # anything here may be an address

_md = Cs(CS_ARCH_X86, CS_MODE_32)
_md.detail = True

BRANCH = {'jmp', 'je', 'jne', 'jg', 'jge', 'jl', 'jle', 'ja', 'jae', 'jb', 'jbe',
          'js', 'jns', 'jo', 'jno', 'jp', 'jnp', 'jecxz', 'loop', 'loope', 'loopne'}

# ---------------------------------------------------------------- COFF ----

class Obj:
    """Minimal i386 COFF reader: sections, symbols, relocations."""
    def __init__(self, path):
        b = self.b = open(path, 'rb').read()
        _, nsec, _, psym, nsym, optsz, _ = struct.unpack_from('<HHIIIHH', b, 0)
        strtab = psym + nsym * 18
        self.secs = []
        for i in range(nsec):
            o = 20 + optsz + i * 40
            name = b[o:o + 8].rstrip(b'\0').decode('latin-1')
            _, _, rsz, praw, prel, _, nrel, _, ch = struct.unpack_from('<IIIIIIHHI', b, o + 8)
            self.secs.append(dict(name=name, size=rsz, raw=praw, prel=prel, nrel=nrel))
        self.syms = []
        i = 0
        while i < nsym:
            o = psym + i * 18
            if b[o:o + 4] == b'\0\0\0\0':
                so = strtab + struct.unpack_from('<I', b, o + 4)[0]
                name = b[so:b.index(b'\0', so)].decode('latin-1')
            else:
                name = b[o:o + 8].rstrip(b'\0').decode('latin-1')
            val, sec, typ, cls, naux = struct.unpack_from('<IhHBB', b, o + 8)
            self.syms.append(dict(name=name, value=val, sec=sec, type=typ, cls=cls))
            for _ in range(naux):
                self.syms.append(None)
            i += 1 + naux

    def relocs(self, secno):
        s = self.secs[secno - 1]
        out = {}
        for k in range(s['nrel']):
            va, si, typ = struct.unpack_from('<IIH', self.b, s['prel'] + k * 10)
            out[va] = (self.syms[si]['name'], typ)
        return out

    def functions(self):
        return [s for s in self.syms if s and s['type'] == 0x20 and s['sec'] > 0]

    def function(self, sym):
        """-> (code bytes, {offset: (symname, type)}) for the named function."""
        f = [s for s in self.functions() if s['name'] == sym]
        if not f:
            raise KeyError(sym)
        f = f[0]
        sec = self.secs[f['sec'] - 1]
        later = sorted(s['value'] for s in self.functions()
                       if s['sec'] == f['sec'] and s['value'] > f['value'])
        end = later[0] if later else sec['size']
        # a switch's jump table (and byte index table) sits in the function's
        # own section after its code, reached through a DIR32 reloc to a label
        # FORWARD of the referencing instruction: cut the code there, or the
        # table decodes as junk instructions (the original's side is cut by
        # function_extent the same way)
        s_rel = self.relocs(f['sec'])
        k = 0
        for o, (name, typ) in s_rel.items():
            if not (f['value'] <= o < end) or typ != 6:
                continue
            sy = self.symbol(name)
            if sy and sy['sec'] == f['sec'] and o < sy['value'] < end:
                end = min(end, sy['value'])
        code = self.b[sec['raw'] + f['value']:sec['raw'] + end]
        rel = {o - f['value']: r for o, r in s_rel.items() if f['value'] <= o < end}
        return code, rel

    def symbol(self, name):
        if not hasattr(self, '_byname'):
            self._byname = {}
            for sy in self.syms:
                if sy:
                    self._byname.setdefault(sy['name'], sy)
        return self._byname.get(name)


def resolve_symbol(obj, name):
    """Class::method / free name -> the MSVC-mangled COFF function symbol."""
    parts = name.split('::')
    meth, scope = parts[-1], parts[:-1]
    tail = ''.join(p + '@' for p in reversed(scope)) + '@'
    pats = []
    if scope and meth == scope[-1]:
        pats.append('??0' + tail)
    elif scope and meth == '~' + scope[-1]:
        pats.append('??1' + tail)
    else:
        pats.append('?' + meth + '@' + tail)
    if not scope:
        pats.append('_' + meth)
    hits = [s['name'] for s in obj.functions()
            if any(s['name'].startswith(p) if p[0] == '?' else s['name'] == p for p in pats)]
    hits = sorted(set(hits))
    if len(hits) != 1:
        raise SystemExit('cannot pick a symbol for %s from %r - pass --sym' % (name, hits))
    return hits[0]

# ----------------------------------------------------------- original ----

import functools


@functools.lru_cache(maxsize=1)
def _pe_sections():
    b = open(ORIG, 'rb').read()
    pe = struct.unpack_from('<I', b, 0x3c)[0]
    nsec = struct.unpack_from('<H', b, pe + 6)[0]
    optsz = struct.unpack_from('<H', b, pe + 20)[0]
    base = struct.unpack_from('<I', b, pe + 52)[0]
    secs = []
    for i in range(nsec):
        o = pe + 24 + optsz + i * 40
        vsz, va, rsz, praw = struct.unpack_from('<IIII', b, o + 8)
        secs.append((base + va, rsz, praw, b[o:o + 8].rstrip(b'\0')))
    return b, secs


def _raw(addr, n):
    """Bytes of a READ-ONLY constant (.rdata), or None."""
    b, secs = _pe_sections()
    for va, rsz, praw, name in secs:
        if name == b'.rdata' and va <= addr and addr + n <= va + rsz:
            return b[praw + addr - va:praw + addr - va + n]
    return None


def orig_bytes(addr, maxlen=0x8000):
    b, secs = _pe_sections()
    for va, rsz, praw, _ in secs:
        if va <= addr < va + rsz:
            n = min(maxlen, va + rsz - addr)
            return b[praw + addr - va:praw + addr - va + n]
    raise SystemExit('0x%x is not in a section' % addr)


def function_extent(code, base):
    """Linear sweep to the REAL end: the first ret/jmp that no forward branch
    jumps past (CLAUDE.md: never use the next annotation as the end). A jump
    table in the function's own body stops the decode where the table starts."""
    reach, tables, last = base, [], None
    for i in _md.disasm(code, base):
        if tables and i.address >= min(tables):
            return min(tables) - base
        m = i.mnemonic
        if m in BRANCH and i.operands and i.operands[0].type == X86_OP_IMM:
            t = i.operands[0].imm
            if t > reach and t < base + len(code):
                reach = t
        if m == 'jmp' and i.operands and i.operands[0].type == X86_OP_MEM:
            d = i.operands[0].mem.disp
            if base < d < base + 0x8000:
                tables.append(d)
        end = i.address + i.size
        if m in ('ret', 'retn') or (m == 'jmp' and not tables):
            if end > reach:
                return end - base
        last = end
    return last - base

# --------------------------------------------------------- normalize ----

def _reg(r):
    return _md.reg_name(r) if r else ''


_FNOT = {'fild', 'fistp', 'fist', 'fnstcw', 'fldcw', 'fstp', 'fst', 'fisttp', 'fnstsw'}


def _is_float_op(m):
    return m.endswith(('ss', 'sd')) or (m.startswith('f') and m not in _FNOT)


def _decode(code, base, is_reloc, fconst=None):
    """-> list of (mnemonic, operand-tuple, branch-target-or-None). An operand
    whose field is relocated (ours) or image-range (original's) becomes
    ('A', value) - comparable to the other side only by being an address."""
    out = []
    for i in _md.disasm(code, base):
        ops = []
        tgt = None
        if i.mnemonic in BRANCH and i.operands and i.operands[0].type == X86_OP_IMM:
            tgt = i.operands[0].imm
            ops.append(('T',))
        elif i.mnemonic == 'call' and i.operands and i.operands[0].type == X86_OP_IMM:
            ops.append(('CALL',))
        else:
            for op in i.operands:
                if op.type == X86_OP_REG:
                    ops.append(('r', _reg(op.reg)))
                elif op.type == X86_OP_IMM:
                    v = op.imm & 0xffffffff
                    if is_reloc(i, i.imm_offset, v):
                        ops.append(('A',))
                    else:
                        ops.append(('i', v))
                elif op.type == X86_OP_MEM:
                    m = op.mem
                    d = m.disp & 0xffffffff
                    da = is_reloc(i, i.disp_offset, d) if i.disp_offset else False
                    tok = 'A' if da else d
                    # a float pool constant compares by VALUE ('F:<bits>'):
                    # masked as an address, 40.0 and 0.0 looked identical
                    if fconst and not m.base and not m.index and _is_float_op(i.mnemonic):
                        fv = fconst(i, op, d)
                        if fv:
                            tok = fv
                    ops.append(('m', op.size, _reg(m.segment), _reg(m.base), _reg(m.index),
                                m.scale, tok))
        out.append([i.mnemonic, tuple(ops), tgt, i.address - base])
    # branch targets -> instruction index (or out-of-function)
    at = {e[3] + base: k for k, e in enumerate(out)}
    for e in out:
        if e[2] is not None:
            e[2] = at.get(e[2], 'X')
    return [(m, o, t) for m, o, t, _ in out]


def decode_orig(addr, length=None):
    code = orig_bytes(addr)
    n = length or function_extent(code, addr)
    code = code[:n]
    # never mask here: a member offset (0x420228) is image-range too. Our
    # relocated fields are what carry the address marker (see _opcost).
    def fc(i, op, d):
        if op.size in (4, 8) and IMAGE_LO <= d < IMAGE_HI:
            b = _raw(d, op.size)          # None in .bss: a variable, not a constant
            if b and len(b) == op.size:
                return 'F:%0*x' % (op.size * 2, int.from_bytes(b, 'little'))
        return None
    return _decode(code, addr, lambda i, off, v: False, fc), n


def decode_obj(code, rel):
    def isr(i, off, v):
        return off and (i.address + off) in rel

    def fc(i, op, d):
        r = rel.get(i.address + i.disp_offset) if i.disp_offset else None
        if r and r[0].startswith('__real@'):
            return 'F:' + r[0][7:].lower()
        return None
    return _decode(code, 0, isr, fc)

# ------------------------------------------------------------- cost ----

INDEL = 3

def _img(v):
    return isinstance(v, int) and IMAGE_LO <= v < IMAGE_HI


def _dispeq(x, y):
    if isinstance(x, str) and x.startswith('F:') or isinstance(y, str) and y.startswith('F:'):
        # two known float constants must be the same value; a named float
        # global on one side ('A') cannot be checked and passes
        return x == y or x == 'A' or y == 'A'
    return x == y or (x == 'A' and _img(y)) or (y == 'A' and _img(x))


def _opcost(a, b):
    if a == b:
        return 0
    if (a[0] == 'A' and b[0] == 'i' and _img(b[1])) or (b[0] == 'A' and a[0] == 'i' and _img(a[1])):
        return 0
    if a[0] == b[0] == 'r':
        return 1                                   # register only
    if a[0] == b[0] == 'm':
        if a[1] == b[1] and _dispeq(a[6], b[6]):
            return 0 if a[2:6] == b[2:6] else 1    # same slot, other regs
        return 2
    return 2


def inscost(a, b):
    # branch TARGETS are not compared here: they are instruction indices, and
    # one inserted instruction would shift every later target and turn a
    # single difference into dozens. distance() checks them through the
    # alignment instead.
    if a[0] != b[0] or len(a[1]) != len(b[1]):
        return 4
    c = sum(_opcost(x, y) for x, y in zip(a[1], b[1]))
    return min(c, 4)


def _notgt(L):
    return [(m, o, None) for m, o, t in L]


def distance(A, B):
    """Weighted edit distance over the two instruction lists, plus 2 for
    every aligned branch whose target is not the aligned instruction."""
    return _dist(_notgt(A), _notgt(B)) + 2 * _bad_targets(A, B)


def _bad_targets(A, B):
    import difflib
    ca, cb = _canon(_notgt(A)), _canon(_notgt(B))
    amap = {}
    for blk in difflib.SequenceMatcher(None, ca, cb, autojunk=False).get_matching_blocks():
        for k in range(blk.size):
            amap[blk.a + k] = blk.b + k
    bad = 0
    for i, j in amap.items():
        ta, tb = A[i][2], B[j][2]
        if ta is None and tb is None:
            continue
        if ta == 'X' or tb == 'X':
            bad += ta != tb
        elif amap.get(ta) != tb:
            bad += 1
    return bad


def _dist(A, B):
    n, m = len(A), len(B)
    prev = list(range(0, (m + 1) * INDEL, INDEL))
    for i in range(1, n + 1):
        cur = [i * INDEL] + [0] * m
        a = A[i - 1]
        for j in range(1, m + 1):
            b = B[j - 1]
            c = prev[j - 1] if a == b else prev[j - 1] + inscost(a, b)

            if prev[j] + INDEL < c: c = prev[j] + INDEL
            if cur[j - 1] + INDEL < c: c = cur[j - 1] + INDEL
            cur[j] = c
        prev = cur
    return prev[m]


def _canon(L):
    def op(o):
        if o[0] == 'A' or (o[0] == 'i' and _img(o[1])):
            return ('A',)
        if o[0] == 'm' and (o[6] == 'A' or _img(o[6])):
            return o[:6] + ('A',)
        return o
    return [(m, tuple(op(o) for o in ops), t) for m, ops, t in L]


def exact_pct(A, B):
    """reccmp-like: the share of identical instructions. Branch targets are
    NOT compared as indices (one inserted instruction would make every later
    jump differ - the same cascade distance() had); an aligned branch whose
    target does not correspond counts as different instead."""
    import difflib
    sm = difflib.SequenceMatcher(None, _canon(_notgt(A)), _canon(_notgt(B)), autojunk=False)
    same = sum(x.size for x in sm.get_matching_blocks()) - _bad_targets(A, B)
    return 100.0 * 2 * max(0, same) / max(1, len(A) + len(B))

# ------------------------------------------------------------ guard ----

_STACK = {'esp', 'ebp'}

def content(code, rel):
    """Register-free content of OUR function (semscan's idea): callees and
    referenced globals by symbol name, plus the non-stack constants. A
    candidate that changes these changed behaviour, not codegen."""
    calls = collections.Counter()
    data = set()
    consts = set()
    for i in _md.disasm(code, 0):
        for off, (name, typ) in rel.items():
            if i.address <= off < i.address + i.size:
                if i.mnemonic in ('call', 'jmp') and typ == 0x14:
                    calls[name] += 1
                else:
                    # cl numbers its own labels and string literals in source
                    # order ($L61033, $SG4711): a reordering renames them
                    data.add(re.sub(r'^\$(L|SG)\d+$', r'$\1', name))
        if i.mnemonic in ('push', 'sub', 'add') and i.operands and \
                i.operands[0].type == X86_OP_REG and _reg(i.operands[0].reg) == 'esp':
            continue
        for op in i.operands:
            if op.type == X86_OP_IMM and not (i.imm_offset and i.address + i.imm_offset in rel) \
                    and i.mnemonic not in BRANCH and i.mnemonic != 'call':
                v = op.imm & 0xffffffff
                consts.add(min(v, (-v) & 0xffffffff))
            elif op.type == X86_OP_MEM and _reg(op.mem.base) not in _STACK \
                    and not (i.disp_offset and i.address + i.disp_offset in rel):
                consts.add(op.mem.disp & 0xffffffff)
    return calls, data, consts


def orig_consts(addr, n):
    consts = set()
    for i in _md.disasm(orig_bytes(addr)[:n], addr):
        for op in i.operands:
            if op.type == X86_OP_IMM:
                v = op.imm & 0xffffffff
                consts.add(min(v, (-v) & 0xffffffff))
            elif op.type == X86_OP_MEM:
                consts.add(op.mem.disp & 0xffffffff)
    return consts


def guard_ok(base, cand, allowed_consts):
    bc, bd, _ = base
    cc, cd, ck = cand
    if cc != bc:
        return 'calls changed'
    if cd != bd:
        return 'globals changed'
    extra = ck - allowed_consts
    if extra:
        return 'new constants %s' % ','.join('0x%x' % v for v in sorted(extra)[:4])
    return None
