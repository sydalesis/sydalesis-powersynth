#!/usr/bin/env python3
"""Sydalesis 6.0 bank audit: reads presetcheck6 TSV rows, decides which generated presets to re-roll.

usage: audit6.py rows.tsv [rows.tsv ...]
Updates reroll6.json (name -> attempt) and writes reroll6_names.txt (the presets to render again).
Criteria = the 5.1 audit (silent, harsh, bright, phasey, cpu, quiet, static) plus 'loud' (the level check had to pull
the master below -24 dB: something in the patch is far hotter than its siblings). Init presets are never judged.
"""
import sys, json, os
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
EVOLVING = {'Drones', 'Pads', 'Atmospheres', 'Textures', 'Soundscapes', 'Spectral', 'Sweeps', 'Dark Ambient', 'Deep Space',
            'Layered', 'Performance', 'Just Intonation', 'Choirs & Voices'}
MAX_TRIES = 6        # after this many re-rolls a preset is kept as it is (reported)

def verdict(r):
    name, cat = r[0], r[1]
    rms, pk, mast, cpu, cen, lo, hi, flux, evo, corr = map(float, r[2:12])
    why = []
    if rms < -60 and pk < -40: why.append('silent')
    if hi > (0.25 if cat == 'FX & Details' else 0.15): why.append('harsh %.2f' % hi)
    if cen > (5000 if cat == 'FX & Details' else 3500): why.append('bright %.0f' % cen)
    if corr < -0.1: why.append('phasey %.2f' % corr)
    if cpu > 0.25: why.append('cpu %.0f%%' % (cpu * 100))
    if mast > 16: why.append('quiet %+.1f' % mast)
    if mast < -24: why.append('loud %+.1f' % mast)
    if cat in EVOLVING and evo < 0.03 and flux < 0.03: why.append('static')
    return why

def main():
    rows = {}
    for f in sys.argv[1:]:
        for l in open(f):
            r = l.rstrip('\n').split('\t')
            if len(r) >= 12: rows[r[0]] = r
    rp = os.path.join(HERE, 'reroll6.json')
    reroll = json.load(open(rp)) if os.path.exists(rp) else {}
    bad, reasons, kept = [], {}, []
    for name, r in rows.items():
        if r[1] in ('Init', 'Layered'): continue      # layered presets inherit their parts
        w = verdict(r)
        if not w: continue
        if reroll.get(name, 0) >= MAX_TRIES: kept.append((name, w)); continue
        bad.append(name); reasons[name] = (r[1], w)
    for n in bad: reroll[n] = reroll.get(n, 0) + 1
    json.dump(reroll, open(rp, 'w'), indent=1, sort_keys=True)
    open(os.path.join(HERE, 'reroll6_names.txt'), 'w').write(''.join(n + '\n' for n in bad))
    print('judged', len(rows), 'rejected', len(bad), 'kept after %d tries' % MAX_TRIES, len(kept))
    print(Counter(w.split()[0] for _, ws in reasons.values() for w in ws).most_common())
    print(Counter(c for c, _ in reasons.values()).most_common())
    for n, w in kept: print('  kept:', n, w)

if __name__ == '__main__':
    main()
