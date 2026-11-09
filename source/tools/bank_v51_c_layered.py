# Driftfield 5.1 bank, part C: LAYERED — two finished patches as layer 1 and layer 2.
# Pairings follow how these records are built: a foundation (drone, dark bed, deep-space floor, pad) under a
# moving surface (texture, atmosphere, field recording, spectral smear, ritual pulse), or a split with the
# low drone under the left hand. CPU: when preset_cpu51.json exists (written by the level check), pairs whose
# summed CPU would be heavy are skipped.
_cpu_path = _os.path.join(HERE, 'preset_cpu51.json')
_CPU = json.load(open(_cpu_path)) if _os.path.exists(_cpu_path) else {}
_LAYER_BUDGET = 0.16
_mcp = _os.path.join(HERE, 'cpu51.json')
_MCPU = json.load(open(_mcp)) if _os.path.exists(_mcp) else {}

_by_cat = {}
for _p_ in PRESETS:
    if isinstance(_p_, LayeredPatch): continue
    _by_cat.setdefault(_p_.cat, []).append(_p_.name)
for _k in _by_cat: _by_cat[_k].sort()

LAYER_PAIRS = [  # (foundation cats, surface cats, weight, modes)
    (['Drones', 'Dark Ambient', 'Deep Space'], ['Textures', 'Atmospheres', 'Soundscapes', 'Spectral'], 5, [('layer', 5), ('pressure', 1), ('morph', 1)]),
    (['Pads', 'Atmospheres'], ['Spectral', 'Textures', 'Deep Space', 'Choirs & Voices'], 3, [('layer', 4), ('morph', 2), ('velocity', 1)]),
    (['Drones', 'Dark Ambient', 'Basses'], ['Pads', 'Atmospheres', 'Deep Space', 'Bells & Resonators'], 2, [('split', 1)]),
    (['Ritual & Tribal'], ['Drones', 'Soundscapes', 'Textures', 'Winds & Reeds'], 2, [('layer', 3), ('pressure', 1)]),
    (['Deep Space', 'Atmospheres'], ['Bells & Resonators', 'Spectral', 'Textures'], 2, [('layer', 3), ('morph', 1)]),
    (['Berlin Sequences'], ['Pads', 'Drones', 'Atmospheres'], 1, [('layer', 1)]),
]
LW_A = ['Twin', 'Double', 'Folded', 'Layered', 'Merged', 'Entwined', 'Mirrored', 'Braided', 'Woven', 'Paired', 'Coupled', 'Stacked', 'Bound', 'Joined', 'Nested']
LW_N = ['Worlds', 'Horizons', 'Currents', 'Strata', 'Tides', 'Fields', 'Veils', 'Realms', 'Skies', 'Depths', 'Rivers', 'Shadows', 'Lights', 'Signals', 'Seasons',
        'Landscapes', 'Elements', 'Spirits', 'Dreams', 'Echoes', 'Planes', 'Spheres', 'Grounds', 'Weathers', 'Kingdoms', 'Thresholds', 'Rooms', 'Paths', 'Altars', 'Fires']
LW_O = ['Earth and Sky', 'Fire and Ash', 'Salt and Stone', 'Dust and Light', 'Iron and Rain', 'Ice and Ember', 'Root and Branch', 'Wind and Bone', 'Smoke and Glass',
        'Night and Dawn', 'Shadow and Gold', 'Sand and Star', 'River and Rock', 'Moss and Metal', 'Breath and Stone']
W['Layered'] = (LW_A, LW_N, LW_O, ['Upper', 'Lower', 'Inner', 'Outer', 'Far', 'Near', 'High', 'Deep', 'Second', 'Hidden', 'Parallel', 'Opposite', 'Shared', 'Split', 'Dual'])

def _layer_ok(a, b):
    return _CPU.get(a, 0.05) + _CPU.get(b, 0.05) <= _LAYER_BUDGET

_have_l = sum(1 for _p_ in PRESETS if _p_.cat == 'Layered')
_nl = max(0, 500 - _have_l)
_rl = _R('layered51')
_seen_pairs = set()
for _nm in make_names('Layered', _nl, _USED):
    for _try in range(60):
        fa, fb, _w, modes = _wch(_rl, [((x[0], x[1], x[2], x[3]), x[2]) for x in LAYER_PAIRS])
        ca, cb = _ch(_rl, fa), _ch(_rl, fb)
        if not _by_cat.get(ca) or not _by_cat.get(cb): continue
        a, b = _ch(_rl, _by_cat[ca]), _ch(_rl, _by_cat[cb])
        if a == b or (a, b) in _seen_pairs or not _layer_ok(a, b): continue
        break
    _seen_pairs.add((a, b))
    if _REROLL.get(_nm):
        # rejected by the audit: re-pick this one pair with its own seed (the shared stream stays as it was),
        # within a CPU budget measured by the level check
        _rr = _R('layered51-reroll', _nm, _REROLL[_nm])
        for _try in range(200):
            fa2, fb2, _w2, _m2 = _wch(_rr, [((x[0], x[1], x[2], x[3]), x[2]) for x in LAYER_PAIRS])
            ca2, cb2 = _ch(_rr, fa2), _ch(_rr, fb2)
            if not _by_cat.get(ca2) or not _by_cat.get(cb2): continue
            a2, b2 = _ch(_rr, _by_cat[ca2]), _ch(_rr, _by_cat[cb2])
            if a2 == b2 or (a2, b2) in _seen_pairs: continue
            if _MCPU.get(a2, 0.1) + _MCPU.get(b2, 0.1) > 0.16: continue
            a, b = a2, b2; break      # not added to _seen_pairs: the shared stream must not change
    mode = _wch(_rl, modes)
    kw = dict(balance=_t(_u(_rl, -4.0, 1.0), 1))
    if mode == 'split': kw.update(split=_ch(_rl, [48, 50, 52, 53, 55, 57]), width=_t(_u(_rl, 2, 8), 1))
    if mode == 'morph': kw.update(morph=_t(_u(_rl, 0.3, 0.7)))
    tags = tuple(sorted(set(_rl.sample(['roach', 'corbacho', 'sydalesis'], 1) + ['layered51'])))
    L(_nm, a, b, mode, tags=tags, **kw)
