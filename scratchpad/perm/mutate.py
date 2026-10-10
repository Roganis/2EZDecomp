# mutate.py - source rewrites for the permuter, on ONE function's text.
#
# Every mutation is meant to be behaviour-preserving; the ones that cannot
# prove it from syntax alone (statement swaps) use a conservative
# independence test, and the driver's content guard (score.content) rejects
# anything that changes callees, globals or constants. Each kind is a lever
# docs/cookbook.md has shown to move cl 13.10's output.
import random, re
import tree_sitter_cpp, tree_sitter

_P = tree_sitter.Parser(tree_sitter.Language(tree_sitter_cpp.language()))


def parse(src):
    return _P.parse(src).root_node


def walk(n):
    yield n
    for c in n.children:
        yield from walk(c)


def T(n):
    return n.text.decode('latin-1')


def edit(src, edits):
    """edits: list of (start, end, newbytes), non-overlapping."""
    # right to left; at one offset the REPLACEMENT goes before the insertion,
    # or the replacement's range would land on the inserted text
    for s, e, new in sorted(edits, key=lambda x: (-x[0], -(x[1] - x[0]))):
        src = src[:s] + new + src[e:]
    return src


def has(n, types):
    return any(c.type in types for c in walk(n))

SIDE = ('call_expression', 'assignment_expression', 'update_expression')
STMT = ('expression_statement', 'declaration', 'if_statement', 'while_statement',
        'do_statement', 'for_statement', 'return_statement', 'compound_statement')

# ------------------------------------------------------ dependence ----

def _locals(fn):
    names = set()
    for n in walk(fn):
        if n.type in ('declaration', 'parameter_declaration'):
            for c in walk(n):
                if c.type == 'identifier' and c.parent.type in (
                        'init_declarator', 'pointer_declarator', 'declaration',
                        'parameter_declaration', 'array_declarator', 'reference_declarator'):
                    if c.parent.type == 'init_declarator' and c.parent.child_by_field_name('value') == c:
                        continue
                    names.add(T(c))
    return names


def _loc(n, locs):
    """A memory location as ('f', root-name, const-index-tuple|None) or 'COMPLEX'.
    Two locations alias when the root names match and one index tuple is a
    prefix of the other; different fields never alias (no unions here)."""
    idx = []
    while True:
        if n.type == 'parenthesized_expression' and n.named_children:
            n = n.named_children[0]
        elif n.type == 'subscript_expression':
            i = n.child_by_field_name('indices') or n.child_by_field_name('index')
            if i is not None and i.type == 'subscript_argument_list' and i.named_children:
                i = i.named_children[0]
            if i is None or i.type != 'number_literal':
                return 'COMPLEX'
            idx.insert(0, T(i))
            n = n.child_by_field_name('argument')
        else:
            break
    if n.type == 'identifier':
        nm = T(n)
        if nm in locs:
            return ('f', 'local:' + nm, tuple(idx)) if idx else None
        return ('f', nm, tuple(idx))
    if n.type == 'field_expression':
        f = n.child_by_field_name('field')
        return ('f', T(f), tuple(idx)) if f is not None else 'COMPLEX'
    return 'COMPLEX'


MEMX = ('field_expression', 'subscript_expression', 'pointer_expression')


def info(st, locs):
    """-> (calls, local writes, local reads, mem writes, mem reads)."""
    calls = has(st, ('call_expression', 'new_expression', 'delete_expression'))
    wl, rl, wm, rm = set(), set(), [], []

    def reads(n):
        # a location read: record it, then only its non-constant indices and
        # pointer bases are further reads
        if n.type == 'identifier':
            if n.parent is not None and n.parent.type == 'call_expression' and \
                    n.parent.child_by_field_name('function') == n:
                return
            k = _loc(n, locs)
            if k is None:
                rl.add(T(n))
            else:
                rm.append(k)
            return
        if n.type in MEMX:
            if n.type == 'pointer_expression' and T(n).startswith('&'):
                addr(n.child_by_field_name('argument'))
                return
            k = _loc(n, locs)
            rm.append(k)
            sub(n)
            return
        if n.type == 'field_identifier':
            return
        for c in n.named_children:
            reads(c)

    def sub(n):
        # the parts of a location expression that are themselves reads
        while n is not None and n.type in MEMX + ('parenthesized_expression',):
            if n.type == 'subscript_expression':
                i = n.child_by_field_name('indices') or n.child_by_field_name('index')
                if i is not None:
                    reads(i)
                n = n.child_by_field_name('argument')
            elif n.type == 'field_expression':
                op = n.child_by_field_name('operator')
                arg = n.child_by_field_name('argument')
                if op is not None and T(op) == '->':
                    reads(arg)
                    return
                n = arg
            elif n.type == 'pointer_expression':
                reads(n.child_by_field_name('argument'))
                return
            else:
                n = n.named_children[0] if n.named_children else None
        if n is not None and n.type == 'identifier' and T(n) in locs:
            pass   # a local array/struct base: its storage is the location

    def addr(n):
        # &x: no read of x's value, but a local whose address escapes is memory
        k = _loc(n, locs) if n is not None else None
        if k is None and n is not None and n.type == 'identifier':
            rl.add(T(n))                     # &local USES the local (it must exist)
            rm.append('COMPLEX')
        elif n is not None:
            sub(n)

    def walk_st(n):
        if n.type == 'assignment_expression':
            lhs, rhs = n.child_by_field_name('left'), n.child_by_field_name('right')
            compound = T(n.child_by_field_name('operator')) != '='
            k = _loc(lhs, locs)
            if k is None:
                wl.add(T(lhs))
                if compound:
                    rl.add(T(lhs))
            else:
                wm.append(k)
                if compound:
                    rm.append(k)
                sub(lhs)
            walk_st(rhs)
            return
        if n.type == 'update_expression':
            lhs = n.child_by_field_name('argument')
            k = _loc(lhs, locs)
            if k is None:
                wl.add(T(lhs)); rl.add(T(lhs))
            else:
                wm.append(k); rm.append(k); sub(lhs)
            return
        if n.type == 'init_declarator':
            d = n.child_by_field_name('declarator')
            while d is not None and d.type != 'identifier':
                d = d.child_by_field_name('declarator') or (d.named_children[-1] if d.named_children else None)
            if d is not None:
                wl.add(T(d))
            v = n.child_by_field_name('value')
            if v is not None:
                walk_st(v)
            return
        if n.type in ('identifier',) + MEMX:
            reads(n)
            return
        if n.type in ('pointer_declarator', 'array_declarator', 'reference_declarator'):
            # a declarator with no initializer still DEFINES its name here
            d = n
            while d is not None and d.type != 'identifier':
                d = d.child_by_field_name('declarator')
            if d is not None:
                wl.add(T(d))
            return
        if n.type == 'identifier' and n.parent is not None and n.parent.type == 'declaration':
            wl.add(T(n))
            return
        if n.type in ('primitive_type', 'type_identifier', 'field_identifier', 'sized_type_specifier'):
            return
        for c in n.named_children:
            walk_st(c)

    walk_st(st)
    return calls, wl, rl, wm, rm


def _alias(a, b):
    if a == 'COMPLEX' or b == 'COMPLEX':
        return True
    if a[1] != b[1]:
        return False
    n = min(len(a[2]), len(b[2]))
    return a[2][:n] == b[2][:n]


_JUMPS = ('return_statement', 'break_statement', 'continue_statement', 'goto_statement')


def independent(s1, s2, locs):
    c1, wl1, rl1, wm1, rm1 = info(s1, locs)
    c2, wl2, rl2, wm2, rm2 = info(s2, locs)
    # A statement that can leave (an early return, a break) is a barrier:
    # moving a store or a call across it changes whether it happens at all.
    # (A permuter candidate once moved `g_1b2eb74 = 1;` below two early
    # returns - same globals, same constants, so the guard passed it.)
    if has(s1, _JUMPS) and (c2 or wl2 or wm2):
        return False
    if has(s2, _JUMPS) and (c1 or wl1 or wm1):
        return False
    if wl1 & (rl2 | wl2) or wl2 & rl1:
        return False
    if c1 and (c2 or wm2 or rm2):
        return False
    if c2 and (wm1 or rm1):
        return False
    for w in wm1:
        if any(_alias(w, x) for x in wm2 + rm2):
            return False
    for w in wm2:
        if any(_alias(w, x) for x in rm1):
            return False
    return True

# ---------------------------------------------------------- mutations ----
# Each returns a list of candidate edit-lists for this tree.

def m_swap_stmts(src, fn, locs):
    out = []
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        kids = [c for c in blk.named_children if c.type != 'comment']
        for a, b in zip(kids, kids[1:]):
            if a.type not in ('expression_statement', 'declaration') or \
                    b.type not in ('expression_statement', 'declaration'):
                continue
            if independent(a, b, locs):
                out.append([(a.start_byte, a.end_byte, b.text), (b.start_byte, b.end_byte, a.text)])
    return out


def m_move_decl(src, fn, locs):
    """Move an initializer-less declaration one statement up or down."""
    out = []
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        kids = [c for c in blk.named_children if c.type != 'comment']
        for i, d in enumerate(kids):
            if d.type != 'declaration' or has(d, ('init_declarator',)):
                continue
            names = info(d, locs)[1] or {T(c) for c in walk(d) if c.type == 'identifier'}
            for j in (i - 1, i + 1):
                if 0 <= j < len(kids):
                    o = kids[j]
                    if j > i and any(T(c) in names for c in walk(o) if c.type == 'identifier'):
                        continue
                    out.append([(d.start_byte, d.end_byte, o.text), (o.start_byte, o.end_byte, d.text)])
    return out


def m_volatile(src, fn, locs):
    out = []
    body = fn.child_by_field_name('body')
    for d in walk(body):
        if d.type != 'declaration' or 'static' in T(d).split():
            continue
        t = T(d)
        if t.startswith('volatile '):
            out.append([(d.start_byte, d.start_byte + 9, b'')])
        elif not re.search(r'\bconst\b', t):
            out.append([(d.start_byte, d.start_byte, b'volatile ')])
    return out


def m_incdec(src, fn, locs):
    out = []
    for n in walk(fn):
        if n.type == 'update_expression' and n.parent.type in ('expression_statement', 'for_statement',
                                                               'comma_expression'):
            arg = n.child_by_field_name('argument')
            op = n.child_by_field_name('operator')
            if arg is None or op is None:
                continue
            new = (T(op) + T(arg)) if op.start_byte > arg.start_byte else (T(arg) + T(op))
            out.append([(n.start_byte, n.end_byte, new.encode())])
    return out


def _neg(c):
    t = T(c).strip()
    if c.type == 'unary_expression' and t.startswith('!'):
        inner = c.child_by_field_name('argument')
        return T(inner)
    if c.type == 'binary_expression':
        op = T(c.child_by_field_name('operator'))
        flip = {'==': '!=', '!=': '==', '<': '>=', '>=': '<', '>': '<=', '<=': '>'}
        if op in flip and not has(c, SIDE):
            l, r = c.child_by_field_name('left'), c.child_by_field_name('right')
            return '%s %s %s' % (T(l), flip[op], T(r))
    return '!(%s)' % t


def _cond(ifn):
    cc = ifn.child_by_field_name('condition')
    return cc.child_by_field_name('value') if cc is not None else None


def m_if_negate(src, fn, locs):
    out = []
    for n in walk(fn):
        if n.type != 'if_statement':
            continue
        c, a, e = _cond(n), n.child_by_field_name('consequence'), n.child_by_field_name('alternative')
        if c is None or e is None or a is None:
            continue
        eb = e.named_children[-1] if e.named_children else None
        if eb is None or eb.type == 'if_statement':
            continue
        new = 'if (%s)\n%s\nelse\n%s' % (_neg(c), T(eb), T(a))
        out.append([(n.start_byte, n.end_byte, new.encode())])
    return out


def _single_assign(s):
    if s.type == 'compound_statement':
        k = [c for c in s.named_children if c.type != 'comment']
        if len(k) != 1:
            return None
        s = k[0]
    if s.type == 'expression_statement' and s.named_children and \
            s.named_children[0].type == 'assignment_expression':
        a = s.named_children[0]
        if T(a.child_by_field_name('operator')) == '=':
            return a
    return None


def m_ternary(src, fn, locs):
    out = []
    for n in walk(fn):
        if n.type == 'if_statement':
            c, a, e = _cond(n), n.child_by_field_name('consequence'), n.child_by_field_name('alternative')
            if c is None or e is None:
                continue
            eb = e.named_children[-1] if e.named_children else None
            x, y = _single_assign(a), eb is not None and _single_assign(eb)
            if x and y and T(x.child_by_field_name('left')) == T(y.child_by_field_name('left')):
                new = '%s = (%s) ? %s : %s;' % (T(x.child_by_field_name('left')), T(c),
                                               T(x.child_by_field_name('right')),
                                               T(y.child_by_field_name('right')))
                out.append([(n.start_byte, n.end_byte, new.encode())])
        elif n.type == 'expression_statement' and n.named_children and \
                n.named_children[0].type == 'assignment_expression':
            a = n.named_children[0]
            r = a.child_by_field_name('right')
            if T(a.child_by_field_name('operator')) == '=' and r is not None and \
                    r.type == 'conditional_expression':
                l = T(a.child_by_field_name('left'))
                new = 'if (%s)\n%s = %s;\nelse\n%s = %s;' % (
                    T(r.child_by_field_name('condition')), l, T(r.child_by_field_name('consequence')),
                    l, T(r.child_by_field_name('alternative')))
                out.append([(n.start_byte, n.end_byte, new.encode())])
    return out


def _has_continue(b):
    for n in walk(b):
        if n.type in ('while_statement', 'do_statement', 'for_statement') and n is not b:
            continue
        if n.type == 'continue_statement':
            return True
    return False


def m_loop(src, fn, locs):
    out = []
    for n in walk(fn):
        if n.type == 'while_statement':
            c, b = _cond(n), n.child_by_field_name('body')
            if c is None or b is None:
                continue
            new = 'if (%s)\ndo\n%s\nwhile (%s);' % (T(c), T(b), T(c))
            out.append([(n.start_byte, n.end_byte, new.encode())])
        elif n.type == 'if_statement' and n.child_by_field_name('alternative') is None:
            c, a = _cond(n), n.child_by_field_name('consequence')
            if a is not None and a.type == 'compound_statement':
                k = [x for x in a.named_children if x.type != 'comment']
                if len(k) == 1:
                    a = k[0]
            if c is not None and a is not None and a.type == 'do_statement':
                dc = a.child_by_field_name('condition')
                dcv = dc.named_children[0] if dc is not None and dc.type == 'parenthesized_expression' else dc
                if dcv is not None and T(dcv) == T(c):
                    new = 'while (%s)\n%s' % (T(c), T(a.child_by_field_name('body')))
                    out.append([(n.start_byte, n.end_byte, new.encode())])
        elif n.type == 'for_statement':
            init = n.child_by_field_name('initializer')
            c = n.child_by_field_name('condition')
            upd = n.child_by_field_name('update')
            b = n.child_by_field_name('body')
            if c is None or b is None or _has_continue(b) or \
                    (init is not None and init.type == 'declaration'):
                continue
            body = T(b)
            if b.type == 'compound_statement':
                body = body.rstrip()[:-1] + ('\n%s;\n}' % T(upd) if upd is not None else '\n}')
            else:
                body = '{\n%s\n%s\n}' % (body, (T(upd) + ';') if upd is not None else '')
            new = '%s\nwhile (%s)\n%s' % ((T(init) + ';') if init is not None else '', T(c), body)
            out.append([(n.start_byte, n.end_byte, new.encode())])
    return out


PRIMARY = ('identifier', 'number_literal', 'field_expression', 'call_expression',
           'subscript_expression', 'parenthesized_expression', 'char_literal',
           'string_literal', 'this')


def m_parens(src, fn, locs):
    out = []
    for n in walk(fn):
        if n.type in ('binary_expression', 'cast_expression', 'unary_expression') and \
                n.parent.type in ('binary_expression', 'assignment_expression', 'init_declarator',
                                  'argument_list', 'return_statement', 'cast_expression'):
            out.append([(n.start_byte, n.start_byte, b'('), (n.end_byte, n.end_byte, b')')])
        elif n.type == 'parenthesized_expression' and n.parent.type not in (
                'condition_clause', 'sizeof_expression', 'do_statement', 'while_statement'):
            inner = n.named_children[0] if n.named_children else None
            if inner is None or inner.type in ('comma_expression',):
                continue
            safe = inner.type in PRIMARY or (
                n.parent.type in ('expression_statement', 'argument_list', 'init_declarator',
                                  'return_statement')
                or (n.parent.type == 'assignment_expression' and n.parent.child_by_field_name('right') == n
                    and inner.type != 'assignment_expression'))
            if safe:
                out.append([(n.start_byte, n.end_byte, inner.text)])
    return out


def _decl_parts(d):
    """`T x = e;` -> (type text up to the name, name, init node) or None."""
    ids = [c for c in d.named_children if c.type == 'init_declarator']
    if len(ids) != 1 or len([c for c in d.named_children if 'declarator' in c.type]) != 1:
        return None
    idn = ids[0]
    v = idn.child_by_field_name('value')
    dd = idn.child_by_field_name('declarator')
    if v is None or dd is None or v.type in ('initializer_list', 'argument_list') or \
            dd.type not in ('identifier', 'pointer_declarator'):
        return None
    stars = T(dd).count('*') if dd.type == 'pointer_declarator' else 0
    name = T(dd).replace('*', '').strip()
    if not re.match(r'^\w+$', name):
        return None
    typ = src_slice(d, d.start_byte, idn.start_byte).strip() + ' ' + '*' * stars
    return typ.strip(), name, v


def src_slice(n, s, e):
    root = n
    while root.parent is not None:
        root = root.parent
    return root.text[s - root.start_byte:e - root.start_byte].decode('latin-1')


def m_split_init(src, fn, locs):
    out = []
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        kids = [c for c in blk.named_children if c.type != 'comment']
        for i, d in enumerate(kids):
            if d.type != 'declaration' or re.search(r'\b(const|static)\b', T(d)):
                continue
            p = _decl_parts(d)
            if p:
                typ, name, v = p
                new = '%s %s;\n%s = %s;' % (typ, name, name, T(v))
                out.append([(d.start_byte, d.end_byte, new.encode())])
            elif not has(d, ('init_declarator',)) and i + 1 < len(kids):
                nx = kids[i + 1]
                a = _single_assign(nx)
                nm = [T(c) for c in d.named_children if c.type in ('identifier', 'pointer_declarator')]
                if a is not None and len(nm) == 1 and nx.type == 'expression_statement' and \
                        T(a.child_by_field_name('left')) == nm[0].replace('*', '').strip():
                    new = T(d).rstrip().rstrip(';') + ' = ' + T(a.child_by_field_name('right')) + ';'
                    out.append([(d.start_byte, nx.end_byte, new.encode())])
    return out


def m_inline_temp(src, fn, locs):
    out = []
    body = fn.child_by_field_name('body')
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        kids = [c for c in blk.named_children if c.type != 'comment']
        for d, nx in zip(kids, kids[1:]):
            if d.type != 'declaration' or nx.type not in ('expression_statement', 'declaration',
                                                         'return_statement'):
                continue
            p = _decl_parts(d)
            if not p:
                continue
            typ, name, v = p
            uses = [c for c in walk(body) if c.type == 'identifier' and T(c) == name]
            inx = [c for c in uses if nx.start_byte <= c.start_byte < nx.end_byte]
            if len(uses) != 2 or len(inx) != 1:
                continue
            if has(v, SIDE) and re.search(r'&&|\|\||\?', T(nx)):
                continue
            u = inx[0]
            if u.parent.type in ('assignment_expression', 'update_expression') and \
                    u.parent.child_by_field_name('left') == u:
                continue
            new = '((%s)(%s))' % (typ, T(v))
            body_t = nx.text[:u.start_byte - nx.start_byte] + new.encode() + nx.text[u.end_byte - nx.start_byte:]
            out.append([(d.start_byte, nx.end_byte, body_t)])
    return out


def m_commute(src, fn, locs):
    out = []
    flip = {'+': '+', '*': '*', '&': '&', '|': '|', '^': '^', '==': '==', '!=': '!=',
            '<': '>', '>': '<', '<=': '>=', '>=': '<='}
    for n in walk(fn):
        if n.type != 'binary_expression':
            continue
        op = n.child_by_field_name('operator')
        l, r = n.child_by_field_name('left'), n.child_by_field_name('right')
        if op is None or l is None or r is None or T(op) not in flip or has(n, SIDE):
            continue
        new = '%s %s %s' % (T(r), flip[T(op)], T(l))
        out.append([(n.start_byte, n.end_byte, new.encode())])
    return out


def _ends_return(s):
    if s.type == 'return_statement':
        return True
    if s.type == 'compound_statement':
        k = [c for c in s.named_children if c.type != 'comment']
        return bool(k) and k[-1].type == 'return_statement'
    return False


def m_early_return(src, fn, locs):
    """`if (c) {..return;} rest` <-> `if (c) {..return;} else {rest}`."""
    out = []
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        kids = [c for c in blk.named_children if c.type != 'comment']
        for i, n in enumerate(kids):
            if n.type != 'if_statement':
                continue
            a, e = n.child_by_field_name('consequence'), n.child_by_field_name('alternative')
            if a is None or not _ends_return(a):
                continue
            if e is None and i + 1 < len(kids) and not any(k.type == 'declaration' for k in kids[i + 1:]):
                rest = src[kids[i + 1].start_byte:kids[-1].end_byte]
                new = n.text + b'\nelse\n{\n' + rest + b'\n}'
                out.append([(n.start_byte, kids[-1].end_byte, new)])
            elif e is not None:
                eb = e.named_children[-1] if e.named_children else None
                if eb is None or eb.type == 'if_statement':
                    continue
                inner = eb.text
                if eb.type == 'compound_statement':
                    inner = inner.strip()[1:-1]
                new = src[n.start_byte:a.end_byte] + b'\n' + inner
                out.append([(n.start_byte, n.end_byte, new)])
    return out


def m_zero_cmp(src, fn, locs):
    out = []
    for n in walk(fn):
        p = n.parent
        if p is None:
            continue
        boolctx = p.type in ('condition_clause',) or \
            (p.type == 'binary_expression' and T(p.child_by_field_name('operator')) in ('&&', '||')) or \
            (p.type == 'unary_expression' and T(p).startswith('!')) or \
            (p.type == 'conditional_expression' and p.child_by_field_name('condition') == n) or \
            (p.type == 'parenthesized_expression' and p.parent is not None and
             p.parent.type in ('do_statement', 'while_statement'))
        if not boolctx:
            continue
        if n.type == 'binary_expression':
            op = T(n.child_by_field_name('operator'))
            r = n.child_by_field_name('right')
            l = n.child_by_field_name('left')
            if r is not None and T(r) == '0' and op in ('==', '!='):
                new = ('!(%s)' if op == '==' else '(%s)') % T(l)
                out.append([(n.start_byte, n.end_byte, new.encode())])
        elif n.type == 'unary_expression' and T(n).startswith('!'):
            a = n.child_by_field_name('argument')
            out.append([(n.start_byte, n.end_byte, ('%s == 0' % T(a)).encode())])
        elif n.type in ('identifier', 'field_expression', 'pointer_expression'):
            out.append([(n.start_byte, n.end_byte, ('%s != 0' % T(n)).encode())])
    return out


def _indent(src, n):
    i = src.rfind(b'\n', 0, n.start_byte) + 1
    return src[i:n.start_byte] if src[i:n.start_byte].strip() == b'' else b''


def m_move_far(src, fn, locs):
    """Move one statement 2..8 places up or down its block, past statements
    it is independent of EVERY one of (swap_stmts only ever crosses one)."""
    out = []
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        kids = [c for c in blk.named_children if c.type != 'comment']
        for i, s in enumerate(kids):
            if s.type not in ('expression_statement', 'declaration'):
                continue
            ind = _indent(src, s)
            for step in (-1, 1):
                j = i
                for _ in range(8):
                    j += step
                    if not (0 <= j < len(kids)) or not independent(s, kids[j], locs):
                        break
                    if abs(j - i) < 2:
                        continue
                    if step < 0:
                        ins = (kids[j].start_byte, kids[j].start_byte, s.text + b'\n' + ind)
                    else:
                        ins = (kids[j].end_byte, kids[j].end_byte, b'\n' + ind + s.text)
                    out.append([(s.start_byte, s.end_byte, b''), ins])
    return out


_NAMEABLE = ('field_expression', 'subscript_expression', 'binary_expression',
             'cast_expression', 'call_expression', 'pointer_expression')


def _conditional(n, stop):
    """Is n evaluated only conditionally within stop (&&/|| right side, ?: arm)?"""
    while n is not None and n.id != stop.id:
        p = n.parent
        if p is None:
            return False
        if p.type == 'binary_expression' and T(p.child_by_field_name('operator')) in ('&&', '||') \
                and p.child_by_field_name('right') == n:
            return True
        if p.type == 'conditional_expression' and p.child_by_field_name('condition') != n:
            return True
        n = p
    return False


def m_name_subexpr(src, fn, locs):
    """Pull a subexpression into a typed local declared just before its
    statement - 'a named local and a compiler temporary are packed and
    preferred differently' (cookbook). Only where the type is KNOWN
    (tyoracle) and the move cannot change what is read: never out of a
    loop condition or a conditional arm, never past a call or a store."""
    import tyoracle as TY
    scope = TY.fn_scope(fn)
    used = set(re.findall(rb'_p(\d+)', src))
    k = 0
    while str(k).encode() in used:
        k += 1
    name = '_p%d' % k
    out = []
    for blk in walk(fn):
        if blk.type != 'compound_statement':
            continue
        for s in [c for c in blk.named_children if c.type != 'comment']:
            if s.type == 'if_statement':
                region = _cond(s)
            elif s.type in ('expression_statement', 'declaration', 'return_statement'):
                region = s
            else:
                continue
            if region is None:
                continue
            # only the top-level store may write: no ++, no nested assignment
            stores = [n for n in walk(region) if n.type in ('assignment_expression', 'update_expression')]
            top = region.named_children[0] if region.type == 'expression_statement' and \
                region.named_children else None
            if any(n.type == 'update_expression' for n in stores) or \
                    any(n is not top and n.id != (top.id if top else -1) for n in stores):
                continue
            calls = [n for n in walk(region) if n.type == 'call_expression']
            lhs = top.child_by_field_name('left') if top is not None and \
                top.type == 'assignment_expression' else None
            ind = _indent(src, s)
            for n in walk(region):
                if n.id == region.id or (top is not None and n.id == top.id):
                    continue
                if n.type not in _NAMEABLE and not (n.type == 'identifier' and T(n) not in scope
                                                    and n.parent is not None and n.parent.type not in
                                                    ('call_expression', 'field_expression',
                                                     'init_declarator', 'qualified_identifier')):
                    continue
                if n.type == 'pointer_expression' and T(n).startswith('&'):
                    continue
                if lhs is not None and lhs.start_byte <= n.start_byte < lhs.end_byte:
                    continue
                p = n.parent
                if p is not None and (p.type == 'pointer_expression' and T(p).startswith('&') or
                                      p.type == 'call_expression' and p.child_by_field_name('function') == n or
                                      p.type == 'field_expression' and p.child_by_field_name('argument') == n
                                      and T(p.child_by_field_name('operator')) == '.'):
                    continue
                if _conditional(n, region):
                    continue
                inner = [c for c in calls if n.start_byte <= c.start_byte < n.end_byte]
                if len(inner) != len(calls):
                    if inner:
                        continue                     # would reorder calls
                    if calls and info(n, locs)[4]:
                        continue                     # a read hoisted above a call
                t = TY.ty(n, scope)
                sp = t and TY.spell(t)
                if not sp:
                    continue
                # a float may also be held in a double: the value is exact
                for sp2 in ([sp, 'double'] if sp == 'float' else [sp]):
                    decl = ('%s %s = %s;\n' % (sp2, name, T(n))).encode() + ind
                    out.append([(s.start_byte, s.start_byte, decl),
                                (n.start_byte, n.end_byte, name.encode())])
    return out


def m_nest_cond(src, fn, locs):
    """`if (c1 && c2) A else B` -> `if (c1) { if (c2) A else B } else B`, and
    `if (c1 || c2) B else A` -> `if (c1) B else { if (c2) B else A }`: one
    arm written twice. Short-circuit order is unchanged. This is what lets
    cl head-merge a common call set-up past BOTH compares (cookbook: call
    set-up hoisted between two compares; drawCounter @0x434a30)."""
    out = []
    for n in walk(fn):
        if n.type != 'if_statement':
            continue
        c, a, e = _cond(n), n.child_by_field_name('consequence'), n.child_by_field_name('alternative')
        if c is None or a is None or e is None:
            continue
        while c.type == 'parenthesized_expression' and c.named_children:
            c = c.named_children[0]
        if c.type != 'binary_expression':
            continue
        op = T(c.child_by_field_name('operator'))
        if op not in ('&&', '||'):
            continue
        eb = e.named_children[-1] if e.named_children else None
        if eb is None or eb.type == 'if_statement' or has(eb, ('labeled_statement',)) or \
                has(a, ('labeled_statement',)):
            continue
        c1, c2 = T(c.child_by_field_name('left')), T(c.child_by_field_name('right'))
        A, B = T(a), T(eb)
        if op == '&&':
            new = 'if (%s)\n{\nif (%s)\n%s\nelse\n%s\n}\nelse\n%s' % (c1, c2, A, B, B)
        else:
            new = 'if (%s)\n%s\nelse\n{\nif (%s)\n%s\nelse\n%s\n}' % (c1, A, c2, A, B)
        out.append([(n.start_byte, n.end_byte, new.encode())])
    return out


MUTATIONS = {
    'swap_stmts': m_swap_stmts, 'move_decl': m_move_decl, 'volatile': m_volatile,
    'incdec': m_incdec, 'if_negate': m_if_negate, 'ternary': m_ternary, 'loop': m_loop,
    'parens': m_parens, 'split_init': m_split_init, 'inline_temp': m_inline_temp,
    'commute': m_commute, 'early_return': m_early_return, 'zero_cmp': m_zero_cmp,
    'move_far': m_move_far, 'name_subexpr': m_name_subexpr, 'nest_cond': m_nest_cond,
}

# Weights: the levers that most often moved a wall in the cookbook go first.
WEIGHTS = {'swap_stmts': 6, 'move_decl': 4, 'volatile': 2, 'incdec': 1, 'if_negate': 4,
           'ternary': 2, 'loop': 2, 'parens': 2, 'split_init': 3, 'inline_temp': 2,
           'commute': 3, 'early_return': 4, 'zero_cmp': 3, 'move_far': 5, 'name_subexpr': 5, 'nest_cond': 3}


def function_node(root):
    for n in walk(root):
        if n.type == 'function_definition':
            return n
    return None


# (lo, hi) byte range of the function text that mutations may touch, or None.
# Set by perm.py --region; edits that would reach outside it are dropped.
REGION = None


def mutate_once(fntext, rng, only=None):
    """-> (new text, kind) or None. fntext is the function definition alone."""
    root = parse(fntext)
    fn = function_node(root)
    if fn is None:
        return None
    locs = _locals(fn)
    kinds = [k for k in MUTATIONS if not only or k in only]
    rng.shuffle(kinds)
    kinds.sort(key=lambda k: -rng.random() * WEIGHTS[k])
    for k in kinds:
        try:
            opts = MUTATIONS[k](fntext, fn, locs)
        except Exception:
            continue
        if REGION and opts:
            lo, hi = REGION
            opts = [o for o in opts if all(lo <= a and b <= hi for a, b, _ in o)]
        if opts:
            return edit(fntext, rng.choice(opts)), k
    return None


def mutate(fntext, rng, n=None, only=None):
    n = n or rng.choice((1, 1, 1, 2, 2, 3))
    kinds = []
    for _ in range(n):
        r = mutate_once(fntext, rng, only)
        if r is None:
            break
        fntext, k = r
        kinds.append(k)
    return fntext, kinds
