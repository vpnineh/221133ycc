#!/usr/bin/env python3
"""MQL5 -> C++ for the test harness only. MQL5 resolves globals/functions declared later; C++ does not,
so this hoists plain global variables and emits prototypes (defaults moved to the prototype) at a marker."""
import re, sys
src_path, out_path = sys.argv[1], sys.argv[2]
marker = sys.argv[3] if len(sys.argv) > 3 else r'^void ApplyPreset\('
src = open(src_path, encoding='utf-8-sig', errors='replace').read().replace('\r\n', '\n')
lines = []
for line in src.split('\n'):
    s = line.strip()
    if s.startswith('#property') or s.startswith('#include') or s.startswith('input group'): lines.append(''); continue
    lines.append(re.sub(r'^(\s*)input\s+', r'\1', line))
# brace depth at the start of each line (strings/comments ignored)
depth = []; d = 0
for line in lines:
    depth.append(d); i = 0; ins = False
    while i < len(line):
        c = line[i]
        if ins:
            if c == '\\': i += 2; continue
            if c == '"': ins = False
        elif c == '"': ins = True
        elif line.startswith('//', i): break
        elif c == '{': d += 1
        elif c == '}': d -= 1
        i += 1
mi = next(i for i, l in enumerate(lines) if re.match(marker, l))
SIMPLE = r'(?:double|int|bool|long|string|datetime|ulong|uint|color|CSub|CH1Filter|CD1Regime|CTrade)'
hoist = []; protos = []
i = mi
while i < len(lines):
    l = lines[i]
    if depth[i] == 0 and re.match(r'^%s\s+\w+(\s*\[[^\]]*\])*\s*(=[^;(]*)?(,\s*\w+(\s*\[[^\]]*\])*\s*(=[^;(]*)?)*;\s*(//.*)?$' % SIMPLE, l):
        hoist.append(l); lines[i] = ''; i += 1; continue
    m = re.match(r'^([A-Za-z_][\w\s\*&]*?)\s+(\w+)\s*\(', l) if depth[i] == 0 else None
    if m and m.group(1).split()[-1] not in ('return', 'else', 'new') and not l.rstrip().endswith(';'):
        j = i; sig = l
        while sig.count('(') > sig.count(')'): j += 1; sig += '\n' + lines[j]
        o0 = sig.index('('); dp = 0
        for ci in range(o0, len(sig)):
            dp += sig[ci] == '('; dp -= sig[ci] == ')'
            if dp == 0: break
        close = ci
        rest = re.sub(r'//.*', '', sig[close+1:])          # a trailing comment after the parameter list is allowed
        if rest.lstrip().startswith('{') or (not rest.strip() and lines[j + 1].strip().startswith('{')):
            head = sig[:close + 1]
            o = head.index('('); params = head[o+1:-1]
            # split params at top-level commas, strip defaults for the definition
            parts, cur, dp, q = [], '', 0, False
            for ch in params:
                if ch == '"': q = not q
                if not q and ch in '([': dp += 1
                if not q and ch in ')]': dp -= 1
                if ch == ',' and dp == 0 and not q: parts.append(cur); cur = ''
                else: cur += ch
            if cur.strip(): parts.append(cur)
            nodef = [re.sub(r'\s*=.*$', '', p, flags=re.S) for p in parts]
            protos.append(head[:o+1] + ','.join(parts) + ');')
            newhead = head[:o+1] + ','.join(nodef) + ')'
            seg = '\n'.join(lines[i:j+1]); seg = seg.replace(head, newhead, 1)
            seg_lines = seg.split('\n')
            seg_lines += [''] * ((j - i + 1) - len(seg_lines))
            lines[i:j+1] = seg_lines[:j - i + 1] if len(seg_lines) >= j - i + 1 else seg_lines
            i = j + 1; continue
    i += 1
t = '\n'.join(lines[:mi] + ['// ---- hoisted globals'] + hoist + ['// ---- prototypes'] + protos + lines[mi:])
t = re.sub(r"C'(\d+),(\d+),(\d+)'", r'MQL_RGB(\1,\2,\3)', t)
TYPES = r'(?:double|long|int|string|MqlRates|bool|datetime|ulong|uint|char|color)'
def dyn(m):
    return 'MArr<%s> %s;' % (m.group(1), ', '.join(re.findall(r'(\w+)\[\]', m.group(2))))
t = re.sub(r'\b(%s)\s+((?:\w+\[\]\s*,\s*)*\w+\[\])\s*;' % TYPES, dyn, t)
t = re.sub(r'\bCSub\s*\*\s*s\s*=\s*GetPointer\(([^;]*)\);', r'CSub &s = \1;', t)
t = re.sub(r'"([^"\n]*)"\+\(', r'string("\1")+(', t)
t = re.sub(r'\b_Symbol\b', 'Symbol()', t)
t = t.replace('_Point', 'SymbolInfoDouble(CSYM,SYMBOL_POINT)').replace('_Digits', '(int)SymbolInfoInteger(CSYM,SYMBOL_DIGITS)')
open(out_path, 'w').write('#include "sim.h"\n' + t)
print('hoisted', len(hoist), 'prototypes', len(protos))
