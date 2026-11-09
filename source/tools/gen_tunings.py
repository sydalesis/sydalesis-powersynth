#!/usr/bin/env python3
"""Writes the built-in tuning library (Tuning.hpp) and the bank's Scala examples as .scl / .kbm files
into library/Tunings, so they can be edited and reloaded through LOAD .SCL."""
import os, re
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', '..', 'library', 'Tunings')
src = open(os.path.join(HERE, '..', 'Tuning.hpp')).read()
os.makedirs(OUT, exist_ok=True)
n = 0
rows = re.findall(r'\{ "([^"]+)", "([^"]+)", (?:true|false), (?:0x[0-9A-Fa-f]+|0),\s*\n?\s*(nullptr|"[^"]*")', src)
for name, desc, pitches in rows:
    if pitches == 'nullptr':
        edo = 24 if name.startswith('24') else 19
        steps = ['%.5f' % (1200.0 * k / edo) for k in range(1, edo + 1)]
    else:
        steps = pitches.strip('"').split()
    fn = re.sub(r'[^A-Za-z0-9-]+', '_', name).strip('_') + '.scl'
    with open(os.path.join(OUT, fn), 'w', newline='\n') as f:
        f.write('! %s\n!\n%s: %s\n %d\n!\n' % (fn, name, desc, len(steps)))
        for s in steps: f.write(' %s\n' % s)
    n += 1
# examples from the 4.0 bank
bank = open(os.path.join(HERE, 'bank_v40_b_ji.py')).read()
for var, fn in [('HARRISON_PENTA', 'Harrison_Pentatonic.scl'), ('RILEY_ALBION', 'Riley_New_Albion.scl'),
                ('SEVEN_LIMIT_PENTA', 'Septimal_Pentatonic.scl'), ('DIATONIC_JI', 'Just_Diatonic.scl'), ('WHITE_KEYS_KBM', 'White_Keys_7.kbm')]:
    m = re.search(var + r' = """(.*?)"""', bank, re.S)
    open(os.path.join(OUT, fn), 'w', newline='\n').write(m.group(1)); n += 1
print('wrote %d files to %s' % (n, os.path.normpath(OUT)))
