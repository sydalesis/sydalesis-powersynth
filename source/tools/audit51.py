#!/usr/bin/env python3
"""Driftfield 5.1 bank audit: reads presetcheck3 TSV rows and decides which generated presets to re-roll.

usage: audit51.py old_names.txt rows.tsv [rows.tsv ...]
Writes reroll51.json (name -> attempt) and reroll_names.txt (the presets to render again).
Only presets not in old_names.txt (the 5.0 bank) are judged; hand-made presets are never touched.
"""
import sys, json, os

HERE = os.path.dirname(os.path.abspath(__file__))
old = set(l.rstrip('\n') for l in open(sys.argv[1]) if l.strip())
rows = {}
for f in sys.argv[2:]:
    for l in open(f):
        r = l.rstrip('\n').split('\t')
        if len(r) >= 12: rows[r[0]] = r

EVOLVING = {'Drones', 'Pads', 'Atmospheres', 'Textures', 'Soundscapes', 'Spectral', 'Sweeps', 'Dark Ambient', 'Deep Space',
            'Layered', 'Performance', 'Just Intonation', 'Choirs & Voices'}

def verdict(r):
    name, cat = r[0], r[1]
    rms, pk, mast, cpu, cen, lo, hi, flux, evo, corr = map(float, r[2:12])
    why = []
    if rms < -60 and pk < -40: why.append('silent')
    if hi > (0.25 if cat == 'FX & Details' else 0.15): why.append('harsh %.2f' % hi)      # 5.1.2: stricter (was 0.35 / 0.25)
    if cen > (5000 if cat == 'FX & Details' else 3500): why.append('bright %.0f' % cen)  # 5.1.2: stricter (was 6000 / 4500)
    if corr < -0.1: why.append('phasey %.2f' % corr)
    cpu = min(cpu, CPU_REF.get(name, cpu))      # 5.1.2: the clean pass never adds CPU; a busy machine inflates one reading
    if cpu > 0.25: why.append('cpu %.0f%%' % (cpu * 100))
    if mast > 16: why.append('quiet %+.1f' % mast)
    if cat in EVOLVING and evo < 0.03 and flux < 0.03: why.append('static')
    return why

_cr = os.path.join(HERE, 'cpu51.json')
CPU_REF = json.load(open(_cr)) if os.path.exists(_cr) else {}
rp = os.path.join(HERE, 'reroll51.json')
reroll = json.load(open(rp)) if os.path.exists(rp) else {}
bad, reasons = [], {}
for name, r in rows.items():
    if name in old: continue
    w = verdict(r)
    if w:
        bad.append(name); reasons[name] = (r[1], w)
for n in bad: reroll[n] = reroll.get(n, 0) + 1
json.dump(reroll, open(rp, 'w'), indent=1, sort_keys=True)
open(os.path.join(HERE, 'reroll_names.txt'), 'w').write(''.join(n + '\n' for n in bad))
from collections import Counter
print('judged', sum(1 for n in rows if n not in old), 'rejected', len(bad))
print(Counter(w.split()[0] for _, ws in reasons.values() for w in ws).most_common())
print(Counter(c for c, _ in reasons.values()).most_common())
