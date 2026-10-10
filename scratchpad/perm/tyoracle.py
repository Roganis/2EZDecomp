# tyoracle.py - just enough C++ typing to NAME a subexpression in a local.
#
# Built from the TU and include/*.h with tree-sitter: struct fields, globals
# and function return types BY NAME (a name declared with two different
# types anywhere is unknown, never guessed), plus the function's own params
# and locals. ty(expr) gives (base, depth) - depth counts pointer/array
# levels - or None, and None means "do not name this". Wrong is worse than
# unknown here: a mis-typed local truncates or converts and still compiles.
import os, re
import mutate as M

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_CTX = {'fields': {}, 'globals': {}, 'funcs': {}, 'classes': {}, 'bases': {}, 'self': None}
SCALAR = {'int', 'unsigned int', 'float', 'double', 'char', 'unsigned char', 'short',
          'unsigned short', 'long', 'unsigned long', 'unsigned', 'signed char', 'bool',
          'DWORD', 'BYTE', 'WORD', 'BOOL', 'UINT'}
_RANK = {'double': 5, 'float': 4, 'unsigned long': 3, 'unsigned int': 3, 'unsigned': 3,
         'DWORD': 3, 'UINT': 3, 'long': 2, 'int': 2, 'BOOL': 2}
_CANON = {'unsigned': 'unsigned int', 'DWORD': 'unsigned long', 'UINT': 'unsigned int',
          'BOOL': 'int', 'signed': 'int'}


def _base(typenode_text):
    t = re.sub(r'\b(const|volatile|static|extern|register|struct|class|enum|inline|'
               r'__declspec\([^)]*\))\b', ' ', typenode_text)
    t = ' '.join(t.split())
    return _CANON.get(t, t)


def _decl_names(d):
    """declarator node -> (name, depth, is_function)."""
    depth, fn = 0, False
    while d is not None:
        if d.type in ('identifier', 'field_identifier'):
            return M.T(d), depth, fn
        if d.type == 'pointer_declarator':
            depth += 1
        elif d.type == 'array_declarator':
            depth += 1
        elif d.type == 'function_declarator':
            fn = True
        elif d.type == 'reference_declarator':
            return None
        elif d.type in ('qualified_identifier',):
            n = d.child_by_field_name('name')
            return (M.T(n), depth, fn) if n is not None else None
        nxt = d.child_by_field_name('declarator')
        if nxt is None and d.named_children:
            nxt = d.named_children[0] if d.type == 'init_declarator' else None
        d = nxt
    return None


def _add(tab, name, ty):
    if name in tab and tab[name] != ty:
        tab[name] = None                      # ambiguous: never guess
    else:
        tab.setdefault(name, ty)


def _owner(n):
    p = n.parent
    while p is not None:
        if p.type in ('class_specifier', 'struct_specifier', 'union_specifier'):
            nm = p.child_by_field_name('name')
            return M.T(nm) if nm is not None else None
        if p.type in ('function_definition', 'translation_unit'):
            return None
        p = p.parent
    return None


def _scan(root):
    for n in M.walk(root):
        if n.type in ('class_specifier', 'struct_specifier'):
            nm = n.child_by_field_name('name')
            bc = [c for c in n.children if c.type == 'base_class_clause']
            if nm is not None and bc:
                _CTX['bases'][M.T(nm)] = [M.T(c) for c in M.walk(bc[0])
                                          if c.type == 'type_identifier']
        if n.type not in ('field_declaration', 'declaration'):
            continue
        tnode = n.child_by_field_name('type')
        if tnode is None:
            continue
        base = _base(M.T(tnode))
        top = n.type == 'declaration' and n.parent is not None and \
            n.parent.type in ('translation_unit', 'declaration_list', 'linkage_specification')
        for d in n.children_by_field_name('declarator'):
            r = _decl_names(d)
            if not r:
                continue
            name, depth, isfn = r
            if isfn:
                _add(_CTX['funcs'], name, (base, depth))
            elif n.type == 'field_declaration':
                _add(_CTX['fields'], name, (base, depth))
                cls = _owner(n)
                if cls:
                    _add(_CTX['classes'].setdefault(cls, {}), name, (base, depth))
            elif top:
                _add(_CTX['globals'], name, (base, depth))


def set_context(tu_bytes):
    _CTX.update(fields={}, globals={}, funcs={}, classes={}, bases={}, self=None)
    inc = os.path.join(ROOT, 'include')
    for f in sorted(os.listdir(inc)):
        if f.endswith('.h'):
            _scan(M.parse(open(os.path.join(inc, f), 'rb').read()))
    _scan(M.parse(tu_bytes))


def member(cls, name, seen=None):
    """A field of cls or of one of its bases, by name; None if unknown."""
    seen = seen or set()
    if cls is None or cls in seen:
        return None
    seen.add(cls)
    tab = _CTX['classes'].get(cls, {})
    if name in tab:
        return tab[name]
    for b in _CTX['bases'].get(cls, []):
        r = member(b, name, seen)
        if r:
            return r
    return None


def fn_scope(fn):
    """params + locals of one function: name -> (base, depth) or None.
    Also records the method's class, for implicit-this members."""
    d = fn.child_by_field_name('declarator')
    _CTX['self'] = None
    while d is not None:
        if d.type == 'qualified_identifier':
            sc = d.child_by_field_name('scope')
            _CTX['self'] = M.T(sc) if sc is not None else None
            break
        d = d.child_by_field_name('declarator')
    tab = {}
    for n in M.walk(fn):
        if n.type in ('parameter_declaration', 'declaration'):
            tnode = n.child_by_field_name('type')
            if tnode is None:
                continue
            base = _base(M.T(tnode))
            for d in n.children_by_field_name('declarator'):
                r = _decl_names(d)
                if r and not r[2]:
                    _add(tab, r[0], (base, r[1]))
    return tab


def _lit(t):
    t = t.lower()
    if t.startswith('0x'):
        return ('unsigned int' if 'u' in t else 'int', 0)
    if '.' in t or 'e' in t:
        return ('float', 0) if t.endswith('f') else ('double', 0)
    return ('unsigned int' if 'u' in t else 'int', 0)


def ty(n, scope):
    t = n.type
    if t == 'number_literal':
        return _lit(M.T(n))
    if t == 'char_literal':
        return ('int', 0)
    if t == 'parenthesized_expression':
        return ty(n.named_children[0], scope) if n.named_children else None
    if t == 'identifier':
        nm = M.T(n)
        if nm in scope:
            return scope[nm]
        m = member(_CTX['self'], nm)
        if m:
            return m
        for tab in (_CTX['globals'], _CTX['fields']):
            if nm in tab:
                return tab[nm]
        return None
    if t == 'field_expression':
        f = n.child_by_field_name('field')
        if f is None:
            return None
        a = n.child_by_field_name('argument')
        op = M.T(n.child_by_field_name('operator'))
        if a is not None and a.type == 'this':
            at = (_CTX['self'], 1)
        else:
            at = ty(a, scope) if a is not None else None
        if at and ((op == '->' and at[1] == 1) or (op == '.' and at[1] == 0)):
            m = member(at[0], M.T(f))
            if m:
                return m
        return _CTX['fields'].get(M.T(f))
    if t == 'subscript_expression':
        b = ty(n.child_by_field_name('argument'), scope)
        return (b[0], b[1] - 1) if b and b[1] > 0 else None
    if t == 'pointer_expression':
        op = M.T(n)[:1]
        b = ty(n.child_by_field_name('argument'), scope)
        if not b:
            return None
        if op == '*':
            return (b[0], b[1] - 1) if b[1] > 0 else None
        return (b[0], b[1] + 1) if op == '&' else None
    if t == 'cast_expression':
        tn = n.child_by_field_name('type')
        if tn is None:
            return None
        txt = M.T(tn)
        return (_base(txt.replace('*', ' ')), txt.count('*'))
    if t == 'unary_expression':
        op = M.T(n.child_by_field_name('operator'))
        a = ty(n.child_by_field_name('argument'), scope)
        if op == '!':
            return ('int', 0)
        if a and a[1] == 0 and a[0] in _RANK:
            return a if _RANK[a[0]] >= 2 else ('int', 0)
        return None
    if t == 'binary_expression':
        op = M.T(n.child_by_field_name('operator'))
        if op in ('==', '!=', '<', '>', '<=', '>=', '&&', '||'):
            return ('int', 0)
        a = ty(n.child_by_field_name('left'), scope)
        b = ty(n.child_by_field_name('right'), scope)
        if not a or not b:
            return None
        if a[1] or b[1]:
            if op == '-' and a[1] and b[1]:
                return ('int', 0)
            if op in ('+', '-') and a[1] and not b[1]:
                return a
            if op == '+' and b[1] and not a[1]:
                return b
            return None
        if a[0] not in _RANK and a[0] not in SCALAR or b[0] not in _RANK and b[0] not in SCALAR:
            return None
        ra, rb = _RANK.get(a[0], 2), _RANK.get(b[0], 2)     # char/short promote to int
        if op in ('%', '&', '|', '^', '<<', '>>') and max(ra, rb) >= 4:
            return None
        r = max(ra, rb)
        if op in ('<<', '>>'):
            r = max(ra, 2)
        return {5: ('double', 0), 4: ('float', 0), 3: ('unsigned int', 0)}.get(r, ('int', 0))
    if t == 'call_expression':
        f = n.child_by_field_name('function')
        if f is None:
            return None
        if f.type == 'field_expression':
            f = f.child_by_field_name('field')
        if f is None or f.type not in ('identifier', 'field_identifier'):
            return None
        r = _CTX['funcs'].get(M.T(f))
        return r if r and r[0] != 'void' else None
    return None


def spell(t):
    base, depth = t
    if depth == 0 and base not in SCALAR and base not in _RANK:
        return None                           # a struct by value: do not copy it
    if base == 'void' and depth == 0:
        return None
    return base + (' ' + '*' * depth if depth else '')
