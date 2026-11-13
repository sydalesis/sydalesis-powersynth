#!/usr/bin/env python3
"""Sydalesis 6.0 factory banks -> FactoryPresets.inc

  * 6.0 bank (bank 0): ~2,500 new presets composed here.
  * Legacy 5.x bank (bank 1): the 6,050 presets of 5.1.7, copied value for value out of the current
    FactoryPresets.inc (they are not regenerated, so they sound exactly as they did; names and categories stay,
    so favourites and sessions that point at them still find them).

usage: gen_presets6.py [out.inc]          (reads ../FactoryPresets.inc.legacy if present, else ../FactoryPresets.inc)

The 6.0 composer reuses the 5.1 procedural engine (bank_v51_a_engine.py / bank_v51_b_recipes.py: layers, filters,
modulation network, macros, scenes, MPE) loaded into a throw-away namespace, and adds the 6.0 sound design on top:
  - movement in the sources instead of the filter: Timbre 5-10 targets (stretch, spread, coupling, formant, tilt,
    shimmer...), at most three cutoff routes per preset, resonance kept clean
  - FM / PM where it suits (bells, glass, keys, space), soft, now also on Additive / Physical / Spectral
  - envelope modulation (velocity -> attack, key -> release, note random -> decay), decay curves that hold for pads
  - start-phase randomisation (analog warmth), LFO random start phase, Custom 2 spectra morphing on Additive
  - FX parameter motion (chorus rate / depth, phaser rate, resonator decay, spectral FX tilt)
  - Level Compensation and 6.0 FM on in every preset
Rules as in 5.1.2: pure and clean (no drive, no crush, no tape drive, low resonance, no grit names), and the
factory effects policy (no Space reverb, no Delay).
"""
import os, sys, re, json, math, copy, struct, random, zlib, importlib.util
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, '..')

# ------------------------------------------------------------------------------------------------ the 5.x toolbox
_spec = importlib.util.spec_from_file_location('g3', os.path.join(HERE, 'gen_presets3.py'))
g3 = importlib.util.module_from_spec(_spec); _spec.loader.exec_module(g3)
g3.load_v2_bank(); g3.load_v3_bank(); g3.load_v31_bank(); g3.load_v40_bank()   # definitions (recipes, word pools, helpers)
NS = vars(g3)
Patch, LayeredPatch = g3.Patch, g3.LayeredPatch
_R, _u, _lg, _ch, _wch, _p, _t = NS['_R'], NS['_u'], NS['_lg'], NS['_ch'], NS['_wch'], NS['_p'], NS['_t']

# 6.0 parameter vocabulary (must match Params.hpp)
g3.DST += ['aoct', 'asemi', 'afine', 'boct', 'bsemi', 'bfine', 'coct', 'csemi', 'cfine']
for _o in 'abc': g3.DST += [_o + 't%d' % k for k in range(5, 11)]
g3.DST += ['fenva', 'fenvd', 'fenvs', 'fenvr', 'aenva', 'aenvd', 'aenvs', 'aenvr', 'menva', 'menvd', 'menvs', 'menvr',
           'aphase', 'bphase', 'cphase',
           'fxphrate', 'fxphfb', 'fxchrate', 'fxchdepth', 'fxdltime', 'fxdlfb', 'fxdldamp', 'fxfsfb', 'fxtpflutter', 'fxtptone',
           'fxrsdecay', 'fxrsbright', 'fxspsize', 'fxspdecay', 'fxspdamp', 'fxsxshift', 'fxsxtilt']
assert len(g3.DST) == 135, len(g3.DST)
g3.FMMODE.update({'pmhard': 4, 'fmhard': 5})
if 'Custom 2' not in g3.SPECTRA: g3.SPECTRA.append('Custom 2')
DST = g3.DST

# Timbre 5..10 per engine kind (Params.hpp kTimbreXNames) and a sensible movement range for each
TX = {
    'an': [('t5', 0.1, 0.35)],
    'add': [('t5', 0.03, 0.12), ('t6', 0.1, 0.35), ('t7', 0.1, 0.3), ('t8', 0.1, 0.4), ('t9', 0.1, 0.3), ('t10', 0.05, 0.2)],
    'ph': [('t5', 0.05, 0.25), ('t6', 0.1, 0.3), ('t7', 0.05, 0.2), ('t9', 0.03, 0.12)],
    'gr': [('t5', 0.05, 0.2), ('t6', 0.02, 0.08), ('t7', 0.1, 0.3)],
    'sp': [('t6', 0.05, 0.25), ('t7', 0.1, 0.3), ('t8', 0.1, 0.3), ('t9', 0.05, 0.2)],
    'wt': [],
}

# 6.0.5: no artist names as tags (removed from both banks; filtered here so a regenerated bank stays clean)
ARTIST_TAGS = ('roach', 'corbacho', 'tangerine', 'vangelis', 'schulze', 'jarre', 'lynch', 'phaedra', 'oxygene')   # tuning names (carlos, harrison, partch, riley, young) stay

# ------------------------------------------------------------------------------------------------ legacy bank
CSTR = r'"((?:[^"\\]|\\.)*)"'
def parse_legacy(path):
    """the 5.x bank: from a 5.1.7 FactoryPresets.inc, or from the legacy half (bank 1) of a 6.0 one"""
    src = open(path, encoding='utf-8').read()
    a = src.index('static const char* const kPresetSyms[] = {'); b = src.index('};', a)
    syms = [m.group(1) for m in re.finditer(CSTR, src[a:b].split('{', 1)[1])]
    a = src.index('static const uint32_t kPresetData[] = {'); b = src.index('};', a)
    data = [int(x[:-1]) for x in re.findall(r'\d+u', src[a:b])]
    states = {}
    for m in re.finditer(r'static const PresetState kS([TL])(\d+)\[\] = \{\n(.*?)\{ nullptr, nullptr \} \};', src, re.S):
        states[(m.group(1), int(m.group(2)))] = [(k, v) for k, v in re.findall(r'\{ "([^"]+)", ' + CSTR + r' \},', m.group(3))]
    a = src.index('static const FactoryPreset kFactoryPresets[] = {'); b = src.index('};', a)
    ents, allents = [], 0
    for m in re.finditer(r'\{ ' + CSTR + ', ' + CSTR + ', ' + CSTR + r', (\d+)u, (\d+)u, kS([TLV])(\d+)(?:, (\d+))? \},', src[a:b]):
        allents += 1
        kind, bank = m.group(6), m.group(8)
        if kind == 'V' or (bank is not None and bank != '1'): continue
        ents.append([m.group(1), m.group(2), ','.join(t for t in m.group(3).split(',') if t not in ARTIST_TAGS), int(m.group(4)), int(m.group(5)), (kind, int(m.group(7))), allents - 1])
    a = src.index('static const uint8_t kPresetCpu[] = {'); b = src.index('};', a)
    cpuall = [int(x) for x in re.findall(r'\d+', src[a:b].split('{', 1)[1])]
    assert len(cpuall) == allents, (len(cpuall), allents)
    cpu = [cpuall[e[6]] for e in ents]
    # keep only the legacy settings (a 6.0 file has the 6.0 bank's settings after them)
    end = max(e[3] + e[4] for e in ents)
    data = data[:2 * end]
    st2 = {}
    out = []
    for i, e in enumerate(ents):
        st2[i] = states[e[5]]
        out.append((e[0], e[1], e[2], e[3], e[4], i))
    return syms, data, st2, out, cpu

def unesc(s): return bytes(s, 'utf-8').decode('unicode_escape').encode('latin-1').decode('utf-8') if '\\' in s else s

# ------------------------------------------------------------------------------------------------ 6.0 composer
RC5 = NS['RC']
FX6 = dict(chorus=0.7, phaser=0.18, drive=0.0, tape=0.12, shift=0.04, reso=0.3, sfx=0.4, shimmer=0.12, width=0.5, eq=0.55, crush=0.0)

def rc6(base, **kw):
    rc = copy.copy(base)
    rc['fx'] = dict(FX6, **{k: v for k, v in base.get('fx', {}).items() if k in ('reso', 'sfx', 'shimmer', 'width', 'phaser', 'eq_high', 'chorus')})
    rc['fx'].update(drive=0.0, crush=0.0)
    rc['res'] = (0.0, 0.05)
    lo, hi = rc.get('cut_mod', (0.15, 0.45)); rc['cut_mod'] = (lo * 0.4, hi * 0.45)     # the filter moves less; the sources move more
    rc.update(kw)
    return rc

RC6 = {}
for _k in ('Drones', 'Pads', 'Atmospheres', 'Textures', 'Soundscapes', 'Spectral', 'Sweeps', 'Dark Ambient', 'Ritual & Tribal',
           'Berlin Sequences', 'Deep Space', 'Choirs & Voices', 'Bells & Resonators', 'Physical Models', 'Winds & Reeds', 'Just Intonation',
           'FX & Details', 'Keys & Organs', 'Basses', 'Leads', 'Sequence & Plucks', 'Performance'):
    RC6[_k] = rc6(RC5[_k])
# section accents for 6.0
RC6['Pads'].update(dcurve=(-0.75, -0.25), fm=0.15, custom2=0.35)
RC6['Drones'].update(dcurve=(-0.8, -0.3), fm=0.12, custom2=0.25)
RC6['Atmospheres'].update(dcurve=(-0.8, -0.3), fm=0.12, custom2=0.25)
RC6['Deep Space'].update(dcurve=(-0.8, -0.3), fm=0.3, fm_style='space', custom2=0.3)
RC6['Dark Ambient'].update(dcurve=(-0.8, -0.3), fm=0.18, fm_style='dark')
RC6['Choirs & Voices'].update(dcurve=(-0.6, -0.2), custom2=0.45)
RC6['Bells & Resonators'].update(dcurve=(0.0, 0.5), fm=0.6, fm_style='bell')
RC6['Keys & Organs'].update(dcurve=(-0.2, 0.4), fm=0.45, fm_style='keys')
RC6['Sequence & Plucks'].update(dcurve=(0.0, 0.5), fm=0.3, fm_style='keys')
RC6['Physical Models'].update(fm=0.2, fm_style='bell')
RC6['Textures'].update(fm=0.2, fm_style='texture')
RC6['FX & Details'].update(fm=0.25, fm_style='texture')
RC6['Spectral'].update(fm=0.12, fm_style='space')
RC6['Just Intonation'].update(dcurve=(-0.8, -0.3), custom2=0.3)
RC6['Sweeps'].update(res=(0.0, 0.05))
RC6['Berlin Sequences'].update(res=(0.0, 0.05), dcurve=(0.0, 0.4))
RC6['Basses'].update(dcurve=(0.0, 0.3))
RC6['Leads'].update(fm=0.15, fm_style='keys')
# 6.0: the Analog section gets a recipe (5.x had only hand-made analog presets)
RC6['Analog'] = rc6(dict(RC5['Pads'], cat='Analog', variant='an6',
    menuA=[('an', ['warm', 'super', 'hollow', 'bright'], 6), ('wt', ['analog'], 1)],
    menuB=[('an', ['warm', 'hollow', 'sine'], 5), ('wt', ['analog', 'warm'], 1)],
    menuC=[('an', ['sine', 'hollow'], 3), ('wt', ['analog'], 1)],
    env=((0.02, 3), (1, 6), (0.6, 1.0), (1.5, 8)), fenv=((0.02, 2), (1, 5), (0.3, 0.7), (1.5, 8)), fenv_amt=(0.05, 0.3),
    cut=(500, 4000), ftone=[('warm', 5), ('open', 1)], tune=0.0, sub=0.4, noise=0.15, vib=(0.0, 0.006),
    styles=['analog'], moods=['analog', 'warm', 'vintage', 'polyphonic', 'brass', 'strings', 'classic', 'nostalgic'], tags=('analog',)),
    dcurve=(-0.4, 0.2), fm=0.15, fm_style='keys')


# ------------------------------------------------------------------------------------------------ 6.0 FM oscillator layers
def _fm_carriers():
    """carriers of the 32 algorithms, read from FmAlgos.hpp (the engine's own table)"""
    src = open(os.path.join(SRC, 'FmAlgos.hpp')).read()
    out = []
    for m in re.finditer(r'\{ \{ ((?:\{\d,\d\},?)+) \}, (\d), (\d), (\d) \},\s*// (\d+)', src):
        edges = [tuple(map(int, e)) for e in re.findall(r'\{(\d),(\d)\}', m.group(1))][:int(m.group(2))]
        mods = {a for a, b in edges}
        out.append({k for k in range(6) if (k + 1) not in mods})
    assert len(out) == 32, len(out)
    return out
FM_CARRIERS = _fm_carriers()
g3.TYPES['fm'] = 6
g3.ENGINE_TAG[6] = 'fm'
NS['LEVEL_BOOST']['fm'] = 1.0
TX['fm'] = []

def _fm_env(c, carrier):
    """carrier envelopes follow the category's amp envelope character (the amp env still shapes the note)"""
    r, rc = c.r, c.rc
    a, d, s, rr = rc['env']
    plucky = s[1] <= 0.5
    if plucky: return (_u(r, 0.001, 0.006), _u(r, d[0], d[1]) * _u(r, 0.8, 1.5), _u(r, 0.0, 0.12), _u(r, 0.3, 2.0))
    return (_u(r, 0.003, 0.03), _u(r, 3, 12), _u(r, 0.6, 0.95), _u(r, 3, 10))

def lay_fm(c, o, style, oct=0, level=0.5, semi=0):
    r, p, rc = c.r, c.p, c.rc
    pre = g3.OSC[o]
    voices = _ch(r, [1, 1, 2, 2, 3]) if style in ('pad', 'evolving', 'glass', 'dark') else _ch(r, [1, 1, 2])
    p.osc(o, 'fm', level=_t(level), oct=oct, semi=semi, fine=_t(_u(r, -4, 4), 1), pan=_t(_u(r, -0.25, 0.25)), voices=voices,
          detune=_t(_u(r, 3, 9), 1), width=_t(_u(r, 0.5, 1.0)))
    plucky = rc['env'][2][1] <= 0.5
    ops = [None] * 6        # (coarse, fine, level, (A, D, S, R), detune, vel, key, wave)
    def car(k, ratio, lvl, det=0.0):
        cf, ff = (int(ratio), ratio - int(ratio)) if ratio >= 1 else (0, 0.0)
        ops[k] = (cf, ff, lvl, _fm_env(c, True), det, _u(r, 0.0, 0.3), 0.0, 0)
    def mod(k, ratio, lvl, env, det=0.0, vel=None, key=None, wave=0):
        cf, ff = (int(ratio), ratio - int(ratio)) if ratio >= 1 else (0, 0.0)
        ops[k] = (cf, ff, lvl, env, det, _u(r, 0.2, 0.6) if vel is None else vel, _u(r, 0.0, 0.35) if key is None else key, wave)
    slow = lambda: (_u(r, 1.0, 6.0), _u(r, 5, 15), _u(r, 0.3, 0.75), _u(r, 4, 10))
    bellmod = lambda: (_u(r, 0.001, 0.01), _u(r, 0.5, 3.0), _u(r, 0.0, 0.12), _u(r, 1.0, 4.0))
    held = lambda: (_u(r, 0.01, 0.3), _u(r, 2, 8), _u(r, 0.4, 0.8), _u(r, 3, 8))
    fb, idx, bright, spread, vidx = 0.0, _u(r, 0.7, 1.1), 0.0, _u(r, 0.0, 0.25), _u(r, 0.1, 0.4)
    if style == 'glass':
        alg = 4
        d = _u(r, 2, 6)
        car(0, 1, 1.0, -d); car(2, _ch(r, [1, 2, 2, 3]), _u(r, 0.4, 0.7), d); car(4, _ch(r, [1, 4, 0.5]), _u(r, 0.2, 0.45), 0.0)
        for k in (1, 3, 5): mod(k, _ch(r, [1, 2, 3, 4]), _u(r, 0.15, 0.38), held())
    elif style == 'bell':
        alg = _ch(r, [4, 4, 6])
        car(0, 1, 1.0); car(2, _ch(r, [3.5, 2.76, 4.2]), _u(r, 0.3, 0.55)); car(4, _ch(r, [0.5, 1, 2]), _u(r, 0.2, 0.4))
        for k, rt in ((1, _ch(r, [3.5, 1.41, 2.76])), (3, _ch(r, [1.41, 5.19, 7])), (5, _ch(r, [1, 3.5, 2]))):
            mod(k, rt, _u(r, 0.28, 0.5), bellmod(), vel=_u(r, 0.4, 0.8))
        vidx = _u(r, 0.3, 0.6)
    elif style == 'pad':
        alg = _ch(r, [0, 13, 15, 17])
        for k in range(6): mod(k, _ch(r, [1, 1, 2, 3]), _u(r, 0.12, 0.32), slow())
        car(0, 1, 1.0)
        spread = _u(r, 0.15, 0.45)
    elif style == 'evolving':
        alg = _ch(r, [1, 2, 10])
        for k in range(6): mod(k, 1 + _ch(r, [0.004, 0.007, 0.011, 0.017, 1.0, 2.0]), _u(r, 0.15, 0.35), slow())
        car(0, 1, 1.0)
        idx = _u(r, 0.6, 1.0); spread = _u(r, 0.1, 0.35)
    elif style == 'dark':
        alg = _ch(r, [0, 4, 13])
        for k in range(6): mod(k, _ch(r, [0.5, 1, 1, 1.5]), _u(r, 0.12, 0.3), slow())
        car(0, _ch(r, [0.5, 1]), 1.0)
        bright = _u(r, -0.7, -0.25); idx = _u(r, 0.5, 0.9)
    elif style == 'organ':
        alg = _ch(r, [31, 31, 24])
        bars = r.sample([0.5, 1, 2, 3, 4, 6, 8], 6)
        for k in range(6): car(k, bars[k], _u(r, 0.25, 0.8) if bars[k] <= 2 else _u(r, 0.1, 0.4))
        ops[0] = (1, 0.0, 1.0) + ops[0][3:]
        fb = _u(r, 0.0, 0.12)
    else:   # 'epiano'
        alg = 4
        car(0, 1, 1.0); car(2, 1, _u(r, 0.4, 0.6))
        mod(1, 1, _u(r, 0.3, 0.45), (_u(r, 0.001, 0.005), _u(r, 1.0, 2.5), _u(r, 0.05, 0.2), 1.5), vel=_u(r, 0.5, 0.8))
        mod(3, 14, _u(r, 0.12, 0.22), (0.001, _u(r, 0.15, 0.4), 0.0, 0.3), vel=_u(r, 0.6, 0.9), key=_u(r, 0.3, 0.6))
        ops[4] = ops[5] = None
        vidx = _u(r, 0.4, 0.7)
    # the algorithm decides who is a carrier: every op that is a carrier there gets a carrier envelope
    carriers = FM_CARRIERS[alg]
    for k in range(6):
        if ops[k] is None: ops[k] = (1, 0.0, 0.0, (0.01, 2, 0.8, 1), 0.0, 0.0, 0.0, 0); continue
        cf, ff, lv, env, det, vel, key, wave = ops[k]
        if k in carriers and style not in ('organ',):
            if k != 0 and style in ('pad', 'evolving', 'dark'): lv = _u(r, 0.0, 0.35) if _p(r, 0.5) else 0.0   # extra carriers: quiet or off
            env = _fm_env(c, True)
            key = 0.0
        elif k in carriers:
            env = _fm_env(c, True)
            env = (env[0], max(env[1], 4.0), max(env[2], 0.85), env[3]) if not plucky else env    # drawbars hold
        else:
            if plucky and style in ('pad', 'evolving', 'dark', 'glass'): env = bellmod()
        A, D, S, R = env
        n = str(k + 1)
        p.set(pre + 'Fm' + n + 'Coarse', cf).set(pre + 'Fm' + n + 'Fine', _t(ff)).set(pre + 'Fm' + n + 'Level', _t(lv))
        p.set(pre + 'Fm' + n + 'Atk', _t(max(0.001, A), 3)).set(pre + 'Fm' + n + 'Dec', _t(max(0.005, D), 3)).set(pre + 'Fm' + n + 'Sus', _t(S))
        p.set(pre + 'Fm' + n + 'Rel', _t(max(0.005, R), 3)).set(pre + 'Fm' + n + 'Detune', _t(det, 1)).set(pre + 'Fm' + n + 'Vel', _t(vel))
        p.set(pre + 'Fm' + n + 'Key', _t(key)).set(pre + 'Fm' + n + 'Wave', wave)
    p.set(pre + 'FmAlgo', alg).set(pre + 'FmFeedback', _t(fb)).set(pre + 'FmIndex', _t(idx)).set(pre + 'FmBright', _t(bright))
    p.set(pre + 'FmSpread', _t(spread)).set(pre + 'FmVelIdx', _t(vidx)).set(pre + 'FmKeyIdx', _t(_u(r, 0.0, 0.3)))
    c.kind[o] = 'fm'; c.engines.append('fm:' + style)
    c.flags.add('fm')
    c.targets += [(o + 't1', 0.08, 0.3), (o + 'level', 0.1, 0.25)]
    if style != 'organ':
        mods = [k for k in range(6) if k not in carriers and ops[k][2] > 0][:2]
        for k in mods: c.targets.append((o + 't%d' % (5 + k), 0.05, 0.18))
    if style in ('dark', 'pad', 'evolving'): c.targets.append((o + 't3', 0.1, 0.3))
NS['LAYERS']['fm'] = lay_fm

FM_MENU = {   # styles, weight in the first layer's menu, weight in the other layers' menus
    'Bells & Resonators': (['bell', 'bell', 'glass'], 2, 1.5), 'Keys & Organs': (['organ', 'epiano', 'epiano', 'bell', 'glass'], 2, 1),
    'Pads': (['pad', 'glass', 'evolving'], 1, 1), 'Deep Space': (['evolving', 'glass', 'pad'], 1, 1), 'Drones': (['evolving', 'dark', 'organ'], 1, 1),
    'Atmospheres': (['pad', 'evolving'], 1, 0.7), 'Dark Ambient': (['dark', 'evolving'], 1, 0.7), 'Sequence & Plucks': (['bell', 'glass', 'epiano'], 1.5, 0.7),
    'Berlin Sequences': (['bell', 'epiano'], 0.7, 0.4), 'Basses': (['dark', 'epiano'], 0.5, 0.3), 'Leads': (['glass', 'organ'], 0.5, 0.4),
    'Just Intonation': (['organ', 'glass', 'evolving'], 1, 0.7), 'Ritual & Tribal': (['bell', 'dark'], 0.5, 0.4), 'Sweeps': (['evolving'], 0.4, 0.3),
    'Performance': (['evolving', 'pad'], 0.5, 0.4),
    # engine-defined sections: FM only as a supporting layer
    'Textures': (['evolving', 'bell'], 0, 0.6), 'Spectral': (['evolving'], 0, 0.5), 'Choirs & Voices': (['pad'], 0, 0.5),
    'Physical Models': (['bell'], 0, 0.6), 'FX & Details': (['bell', 'evolving'], 0, 0.6), 'Soundscapes': (['evolving'], 0, 0.4),
    'Winds & Reeds': (['organ'], 0, 0.4),
}
for _k, (_st, _wa, _wb) in FM_MENU.items():
    _rc = RC6[_k]
    _a = list(_rc['menuA']); _b = list(_rc.get('menuB', _a)); _c = list(_rc.get('menuC', _rc.get('menuB', _a)))
    _rc['menuA'] = _a + ([('fm', _st, _wa)] if _wa else []); _rc['menuB'] = _b + [('fm', _st, _wb)]; _rc['menuC'] = _c + [('fm', _st, _wb)]

PHASE_KINDS = ('an', 'wt')

def _slot_list(p):
    return [i for i in range(1, p.slots + 1) if p.v.get('mod%dSrc' % i, 0) > 0]

def _compact(p, keep):
    """keep only the given slot numbers (in order) and renumber them 1..n"""
    fields = ('Src', 'Dst', 'Amt', 'Via', 'Curve')
    saved = [{f: p.v.get('mod%d%s' % (i, f)) for f in fields} for i in keep]
    for i in range(1, 33):
        for f in fields: p.v.pop('mod%d%s' % (i, f), None)
    for n, s in enumerate(saved, 1):
        for f in fields:
            if s[f] is not None: p.v['mod%d%s' % (n, f)] = s[f]
    p.slots = len(saved)

def _v6_targets(c):
    """6.0 movement targets: Timbre 5-10 of every layer (sources breathe; the filter rests)"""
    for o, k in list(c.kind.items()):
        for t, lo, hi in TX.get(k, []):
            c.targets.append((o + t, lo, hi))
    # the 5.1 lists weight the cutoff heavily: thin it out
    c.targets = [t for t in c.targets if not (t[0] in ('f1cut', 'f2cut') and _p(c.r, 0.5))] or c.targets

def _additive_custom2(c):
    r, p = c.r, c.p
    adds = [o for o, k in c.kind.items() if k == 'add']
    if not adds or not _p(r, c.rc.get('custom2', 0.0)): return
    o = adds[0]
    mk = [lambda: NS['spec_formant']([(_u(r, 2, 6), _u(r, 0.8, 2), _u(r, 0.5, 1.0)), (_u(r, 8, 20), _u(r, 1, 4), _u(r, 0.2, 0.6))], _u(r, 0.5, 1.0)),
          lambda: NS['spec_series']([1, 2, 3, 5, 7, 9, 11, 13, 17, 19, 23][:_ch(r, [5, 7, 9, 11])], _u(r, 0.1, 0.5)),
          lambda: NS['spec_random'](r.randint(0, 99999), _u(r, 0.6, 1.1), _u(r, 0.3, 0.7)),
          lambda: NS['spec_formant']([(_u(r, 3, 12), _u(r, 1, 3), 1.0)], _u(r, 0.7, 1.2))]
    p.st['sp' + o.upper()] = g3.spec_str(_ch(r, mk)())
    p.st['sp2' + o.upper()] = g3.spec_str(_ch(r, mk)())
    pre = g3.OSC[o]
    p.set(pre + 'AdSpec1', g3.SPECTRA.index('Custom')).set(pre + 'AdSpec2', g3.SPECTRA.index('Custom 2'))
    p.set(pre + 'AdPartials', _ch(r, [48, 64, 80, 96]))
    c.engines.append('add:custom2')
    c.flags.add('custom2')
    c.targets.append((o + 't1', 0.25, 0.5))      # MORPH between the two drawings

FM_STYLE = {
    # (carrier kinds allowed, modulator ratio choices in semitones, mode weights, amount range)
    'bell': (('add', 'ph', 'wt', 'an'), [19, 24, 28, 31, 36, 7.02, 14.98], [('pm', 6), ('fm', 1)], (0.12, 0.35)),
    'keys': (('wt', 'an', 'add'), [12, 19, 24, 0], [('pm', 6), ('fm', 1)], (0.08, 0.25)),
    'space': (('wt', 'add', 'sp', 'an'), [7, 12, 19, 24, 0.07], [('pm', 4), ('fm', 2)], (0.06, 0.22)),
    'dark': (('wt', 'an', 'add', 'ph'), [-12, -5, 0.13, 6], [('pm', 3), ('fm', 2), ('ring', 1)], (0.05, 0.18)),
    'texture': (('wt', 'add', 'ph', 'sp', 'gr'), [3, 6, 11, 13, 17], [('pm', 2), ('fm', 2), ('pmhard', 1)], (0.04, 0.14)),
    'default': (('wt', 'an', 'add'), [12, 7, 19], [('pm', 5), ('fm', 1)], (0.05, 0.18)),
}
def _fm6(c):
    r, p, rc = c.r, c.p, c.rc
    if not _p(r, rc.get('fm', 0.08)): return
    kinds, ratios, modes, (lo, hi) = FM_STYLE[rc.get('fm_style', 'default')]
    carriers = [o for o, k in c.kind.items() if k in kinds]
    if not carriers: return
    o = _ch(r, carriers)
    others = [m for m in 'abc' if m != o and m in c.kind]
    mode = _wch(r, modes)
    amt = _u(r, lo, hi) * (0.4 if mode == 'pmhard' else 1.0)
    pre = g3.OSC[o]
    if others and _p(r, 0.55):
        src = _ch(r, others)
    else:
        # a dedicated quiet sine modulator: use the free oscillator if there is one
        free = [m for m in 'abc' if m not in c.kind]
        if not free:
            src = _ch(r, others) if others else None
        else:
            src = free[0]
            st = _ch(r, ratios)
            octs, semi = int(st // 12), st - 12 * int(st // 12)
            p.an(src, 'sine', level=0.0, oct=max(-3, min(3, octs)), semi=int(round(semi)), fine=_t((semi - round(semi)) * 100, 1), slop=0.1)
            c.kind[src] = 'an'; c.engines.append('fmsine')
    if src is None: return
    p.fm(o, src, _t(amt), mode)
    c.flags.add('fm')
    # movement in the index: a slow source and pressure
    if c.room(2):
        NS['_mod'](c, _ch(r, ['lfo1', 'lfo2', 'mseg1', 'lfo5']), o + 'fm', _t(_u(r, 0.04, 0.12)), motion=True)
        if _p(r, 0.6): NS['_mod'](c, 'press', o + 'fm', _t(_u(r, 0.05, 0.15)))

def _v6_voice(c):
    """per-note life: start phase, envelope variation, decay curves, LFO start phase"""
    r, p, rc = c.r, c.p, c.rc
    for o, k in c.kind.items():
        pre = g3.OSC[o]
        if k in PHASE_KINDS and _p(r, 0.4):
            p.set(pre + 'Retrig', 1).set(pre + 'Phase', _t(_u(r, 0, 1))).set(pre + 'PhaseRnd', _t(_u(r, 0.15, 1.0)))
            if _p(r, 0.35) and c.room(): NS['_mod'](c, 'rand', o + 'phase', _t(_u(r, 0.1, 0.3)))
    dc = rc.get('dcurve')
    if dc: p.set('aDecCurve', _t(_u(r, *dc))).set('fDecCurve', _t(_u(r, *dc)))
    if _p(r, 0.45) and c.room(): NS['_mod'](c, 'vel', 'aenva', -_t(_u(r, 0.15, 0.35)))           # harder: faster bloom
    if _p(r, 0.35) and c.room(): NS['_mod'](c, 'key', 'aenvr', -_t(_u(r, 0.1, 0.25)))           # higher: shorter tail
    if _p(r, 0.3) and c.room(): NS['_mod'](c, 'rand', 'fenvd', _t(_u(r, 0.1, 0.25)))
    if _p(r, 0.25) and c.room(): NS['_mod'](c, 'press', 'aenvr', _t(_u(r, 0.1, 0.25)))
    for i in range(1, 7):
        if int(p.v.get('lfo%dMode' % i, 0)) >= 2: p.set('lfo%dRndPh' % i, _t(_u(r, 0.3, 1.0)))
    # pressure into the sources (MPE)
    tx = [t for t in c.targets if re.match(r'^[abc]t(5|6|7|8|9|10)$', t[0])]
    for d, lo, hi in r.sample(tx, min(len(tx), _ch(r, [1, 1, 2]))):
        if c.room(): NS['_mod'](c, 'press', d, _t(_u(r, lo, hi)))

def _v6_fx(c):
    r, p = c.r, c.p
    v = p.v
    if v.get('chOn', 0) > 0 and _p(r, 0.4) and c.room(): NS['_mod'](c, _ch(r, ['lfo5', 'mseg1', 'lfo2']), _ch(r, ['fxchrate', 'fxchdepth']), _t(_u(r, 0.1, 0.3)), motion=True)
    if v.get('phOn', 0) > 0 and _p(r, 0.5) and c.room(): NS['_mod'](c, _ch(r, ['lfo1', 'mseg1']), 'fxphrate', _t(_u(r, 0.15, 0.35)), motion=True)
    if v.get('rsOn', 0) > 0 and _p(r, 0.5) and c.room(): NS['_mod'](c, _ch(r, ['mseg1', 'lfo5', 'm1']), _ch(r, ['fxrsdecay', 'fxrsbright']), _t(_u(r, 0.1, 0.3)))
    if v.get('sxOn', 0) > 0 and _p(r, 0.5) and c.room(): NS['_mod'](c, _ch(r, ['mseg2', 'lfo4', 'mseg1']), 'fxsxtilt', _t(_u(r, 0.1, 0.25)), motion=True)

CUT_D = None
def _cutoff_cap(c, cap=3):
    """at most `cap` cutoff routes: the rest are dropped (the 5.x bank leaned on the filter for movement)"""
    p = c.p
    global CUT_D
    if CUT_D is None: CUT_D = {DST.index(x) for x in ('f1cut', 'f2cut', 'cut')}
    slots = _slot_list(p)
    cut = [i for i in slots if int(p.v.get('mod%dDst' % i, 0)) in CUT_D]
    if len(cut) <= cap: return
    drop = set(sorted(cut, key=lambda i: abs(p.v.get('mod%dAmt' % i, 0)))[:len(cut) - cap])
    _compact(p, [i for i in slots if i not in drop])

STRUCT = ('Formant', 'Vowel', 'Comb', 'Cmb', 'Flg', 'Phs', 'Phaser', 'Dual', 'Morph')
def clean6(p):
    v = p.v
    for f in ('f1', 'f2'):
        v[f + 'Drive'] = 0.0
        if int(v.get(f + 'DriveType', 0)) in (2, 3): v[f + 'DriveType'] = 0.0
        ft = g3.FILTERS[int(v.get(f + 'Type', 1 if f == 'f1' else 0))]
        res = v.get(f + 'Res', 0.05)
        v[f + 'Res'] = round(min(res, 0.55 if any(s in ft for s in STRUCT) else 0.05), 4)
        if v.get(f + 'FmAmt', 0) > 0.05: v[f + 'FmAmt'] = 0.05
    for k in ('drvOn', 'crOn'): v[k] = 0.0
    if v.get('tpOn', 0) > 0: v['tpDrive'] = 0.0
    if v.get('ringLevel', 0) > 0.1: v['ringLevel'] = 0.1
    v['spOn'] = 0.0; v['dlOn'] = 0.0
    for o in ('oa', 'ob', 'oc'):
        md = int(v.get(o + 'FmMode', 0))
        cap = 0.14 if md in (4, 5) else (0.38 if md == 0 else 0.26)
        if v.get(o + 'FmAmt', 0) > cap: v[o + 'FmAmt'] = cap
        if int(v.get(o + 'Warp', 0)) in (g3.WARP.index('sync'), g3.WARP.index('quantize'), g3.WARP.index('flip')): v[o + 'Warp'] = 0.0; v[o + 'WarpAmt'] = 0.0
        if v.get(o + 'AnSync', 0) > 0.15: v[o + 'AnSync'] = 0.15
    # drop any route that would add drive or crush
    bad = {DST.index(x) for x in ('f1drive', 'f2drive', 'fxdrive', 'fxcrush')}
    slots = [i for i in range(1, 33) if v.get('mod%dSrc' % i, 0) > 0]
    if any(int(v.get('mod%dDst' % i, 0)) in bad for i in slots):
        _compact(p, [i for i in slots if int(v.get('mod%dDst' % i, 0)) not in bad])

def compose6(rc, name, idx):
    r = _R('sy60', rc['cat'], rc.get('variant', ''), idx)
    p = g3.Patch(name, rc['cat'])
    c = NS['_Ctx'](p, r, rc)
    NS['_tuning'](c)
    if rc.get('builder'): rc['builder'](c)
    else: NS['_layers'](c)
    _additive_custom2(c)
    _v6_targets(c)
    NS['_filters'](c)
    NS['_fx'](c)
    if rc.get('post'): rc['post'](c)
    _fm6(c)
    NS['_lfo_net'](c)
    _v6_fx(c)
    NS['_macros'](c)
    NS['_scenes_motion'](c)
    NS['_mpe'](c)
    _v6_voice(c)
    NS['_cut_budget'](c)
    _cutoff_cap(c)
    p.misc(drift=_t(_u(r, 0.12, 0.45)), spread=_t(_u(r, 0.5, 0.9)))
    p.set('levelComp', 1).set('fmV6', 7)
    tags = set(rc.get('tags', ()))
    # 6.0.5: no artist tags (roach / corbacho removed from the bank); recipes without styles add none
    st = [x for x in rc.get('styles', []) if x not in ARTIST_TAGS]
    if st: tags.update(r.sample(st, min(len(st), _ch(r, [1, 1, 2]))))
    tags.update(r.sample(rc.get('moods', ['evolving']), min(2, len(rc.get('moods', ['evolving'])))))
    for f, t in (('frozen', 'freeze'), ('field', 'field-recording'), ('tuned', 'just-intonation'), ('fm', 'fm'), ('custom2', 'drawn-spectrum')):
        if f in c.flags: tags.add(t)
    tags.add('v6')
    p.tags(*sorted(tags))
    return p

# ------------------------------------------------------------------------------------------------ names
W6_EXTRA_A = ['Opaline', 'Kindled', 'Lantern-lit', 'Unhurried', 'Starless', 'Feathered', 'Hushed', 'Wintering', 'Unspoken', 'Weightless',
              'Becalmed', 'Tidebound', 'Moonward', 'Sunward', 'Inward', 'Outer', 'Twilit', 'Clouded', 'Untold', 'Lingering', 'Murmuring',
              'Glimmering', 'Turning', 'Rising', 'Settling', 'Mirrored', 'Unfolding', 'Gathering', 'Waking', 'Resting']
W6_EXTRA_N = ['Threshold', 'Lantern', 'Passage', 'Clearing', 'Reliquary', 'Estuary', 'Hourglass', 'Firmament', 'Undercurrent', 'Sanctuary',
              'Waystone', 'Lodestar', 'Archive', 'Stillwater', 'Seabed', 'Watchtower', 'Causeway', 'Sundial', 'Chapel', 'Terrace',
              'Aqueduct', 'Atrium', 'Belfry', 'Labyrinth', 'Observatory', 'Panorama', 'Refuge', 'Crossroads', 'Wellspring', 'Basilica']
def names6(cat, n, used):
    W = NS['W']
    key = cat if cat in W else ('Pads' if cat in ('Analog', 'Performance') else 'Atmospheres')
    A, N, O, PL = W[key]
    A = list(A) + NS['W_COMMON_A'] + W6_EXTRA_A
    N = list(N) + W6_EXTRA_N
    r = _R('names6', cat)
    out, tries = [], 0
    grit = g3.GRIT_RE
    while len(out) < n and tries < n * 800:
        tries += 1
        f = _wch(r, [(0, 45), (1, 22), (2, 20), (3, 13)])
        if f == 0: nm = '%s %s' % (_ch(r, A), _ch(r, N))
        elif f == 1: nm = '%s of %s' % (_ch(r, N), _ch(r, O))
        elif f == 2: nm = '%s %s' % (_ch(r, PL), _ch(r, N))
        else: nm = '%s %s %s' % (_ch(r, A), _ch(r, PL), _ch(r, N))
        words = nm.split()
        if len(nm) > 25 or len(set(w.lower() for w in words)) < len(words): continue
        if any(b in nm.lower() for b in NS['_BLOCK']) or grit.search(nm.lower()): continue
        for a_, b_ in g3.RENAME_WORDS: nm = nm.replace(a_, b_)
        if nm.lower() in used: continue
        used.add(nm.lower()); out.append(nm)
    assert len(out) == n, (cat, len(out), n)
    return out

# ------------------------------------------------------------------------------------------------ the 6.0 bank
COUNTS = {'Drones': 165, 'Dark Ambient': 165, 'Pads': 160, 'Deep Space': 155, 'Atmospheres': 155, 'Textures': 130, 'Soundscapes': 125,
          'Spectral': 105, 'Ritual & Tribal': 100, 'Choirs & Voices': 95, 'Bells & Resonators': 95, 'Physical Models': 95,
          'Just Intonation': 90, 'Berlin Sequences': 90, 'Sweeps': 80, 'Analog': 75, 'FX & Details': 75, 'Sequence & Plucks': 75,
          'Winds & Reeds': 65, 'Keys & Organs': 65, 'Basses': 60, 'Leads': 60, 'Performance': 60}
N_LAYERED = 2500 - 6 - sum(COUNTS.values())

def init_presets():
    out = []
    for nm, typ in (('Init', 'wt'), ('Init Analog', 'analog'), ('Init Additive', 'add'), ('Init Physical', 'phys'), ('Init Granular', 'gran'), ('Init Spectral', 'spec')):
        p = g3.Patch(nm, 'Init')
        p.v['oaType'] = float(g3.TYPES[typ]); p.v['obOn'] = 0.0
        if typ == 'spec': p.st['srcA'] = 'smp:Drones/Shruti Box.wav'
        p.set('levelComp', 1).set('fmV6', 7)
        p.macros = []; p.gave_macros = True
        p.tags('init', 'v6')
        out.append(p)
    return out

def build_bank(legacy_names):
    rr = os.path.join(HERE, 'reroll6.json')
    reroll = json.load(open(rr)) if os.path.exists(rr) else {}
    used = set(n.lower() for n in legacy_names) | {'init', 'init analog', 'init additive', 'init physical', 'init granular', 'init spectral'}
    bank = init_presets()
    for cat, n in COUNTS.items():
        for i, nm in enumerate(names6(cat, n, used)):
            bank.append(compose6(RC6[cat], nm, i + 100000 * reroll.get(nm, 0)))
    # layered: pairs from the new bank (a sustained bed + a moving or bright layer)
    r = _R('sy60', 'layered')
    beds = [p for p in bank if p.cat in ('Pads', 'Drones', 'Deep Space', 'Choirs & Voices', 'Analog')]
    tops = [p for p in bank if p.cat in ('Textures', 'Atmospheres', 'Bells & Resonators', 'Spectral', 'Soundscapes', 'Physical Models')]
    lnames = names6('Layered', N_LAYERED, used)
    layered = []
    for k, nm in enumerate(lnames):
        a, b = _ch(r, beds), _ch(r, tops)
        mode = _wch(r, [('layer', 6), ('velocity', 1), ('pressure', 2), ('morph', 2)])
        lp = LayeredPatch(nm, 'Layered', a.name, b.name, mode, morph=_t(_u(r, 0.3, 0.7)), balance=_t(_u(r, -6, -1), 1))
        layered.append(lp)
    return bank, layered

# ------------------------------------------------------------------------------------------------ emit
def cstr(s): return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'
CATS = ['Init', 'Drones', 'Dark Ambient', 'Deep Space', 'Atmospheres', 'Pads', 'Textures', 'Soundscapes', 'Spectral', 'Choirs & Voices',
        'Ritual & Tribal', 'Just Intonation', 'Bells & Resonators', 'Physical Models', 'Winds & Reeds', 'Analog', 'Sweeps', 'Berlin Sequences',
        'Sequence & Plucks', 'Keys & Organs', 'Basses', 'Leads', 'FX & Details', 'Performance', 'Layered']

def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(SRC, 'FactoryPresets.inc')
    leg_path = os.path.join(SRC, 'FactoryPresets.inc.legacy')
    if not os.path.exists(leg_path): leg_path = os.path.join(SRC, 'FactoryPresets.inc')
    syms, data, states, ents, cpu = parse_legacy(leg_path)
    print('legacy: %d presets, %d symbols, %d settings' % (len(ents), len(syms), len(data) // 2), file=sys.stderr)
    legacy_names = [unesc(e[0]) for e in ents]
    bank, layered = build_bank(legacy_names)
    lv = os.path.join(HERE, 'preset_levels6.json')
    levels = json.load(open(lv)) if os.path.exists(lv) else {}
    for p in bank:
        p.finalize()
        clean6(p)
        if p.cat != 'Init': p.v['master'] = float(levels.get(p.name, -6.0))
    byname = {p.name: p for p in bank}
    for lp in layered:
        lp.resolve(byname, levels)
        lp.v['levelComp'] = 1.0; lp.v['b_levelComp'] = 1.0; lp.v['fmV6'] = 7.0; lp.v['b_fmV6'] = 7.0
        lp.tag_str = ','.join(sorted(set(lp.tag_str.split(',')) | {'v6'}))
    allp = bank + layered
    for p in allp:
        assert p.cat in CATS, p.cat
        assert p.v.get('spOn', 0) == 0 and p.v.get('dlOn', 0) == 0, p.name
    # symbols / data: legacy first (unchanged), new symbols appended
    symix = {s: i for i, s in enumerate(syms)}
    span6 = []
    for p in allp:
        d0 = len(data) // 2
        for k, val in p.v.items():
            if k not in symix: symix[k] = len(syms); syms.append(k)
            data.append(symix[k]); data.append(struct.unpack('<I', struct.pack('<f', float(val)))[0])
        span6.append((d0, len(data) // 2 - d0))
    out = ['// Generated by tools/gen_presets6.py — do not edit by hand.', '// bank 0 = 6.0 (%d presets), bank 1 = Legacy 5.x (%d presets, values copied unchanged from 5.1.7)' % (len(allp), len(ents)), '']
    out.append('static const char* const kPresetSyms[] = {')
    for i in range(0, len(syms), 8): out.append('    ' + ' '.join(cstr(x) + ',' for x in syms[i:i + 8]))
    out.append('};')
    out.append('static constexpr uint32_t kPresetSymCount = %d;' % len(syms))
    out.append('static const uint32_t kPresetData[] = {')
    for i in range(0, len(data), 16): out.append('    ' + ','.join('%du' % x for x in data[i:i + 16]) + ',')
    out.append('};')
    for i, p in enumerate(allp):
        out.append('static const PresetState kSV%d[] = {' % i)
        for k, val in p.st.items():
            if k in ('motion', 'b_motion'): val = g3.compress_motion(val)
            out.append('    { "%s", %s },' % (k, cstr(val)))
        out.append('    { nullptr, nullptr } };')
    for i in sorted(states):
        out.append('static const PresetState kSL%d[] = {' % i)
        for k, val in states[i]: out.append('    { "%s", "%s" },' % (k, val))     # already escaped
        out.append('    { nullptr, nullptr } };')
    out.append('')
    out.append('static const FactoryPreset kFactoryPresets[] = {')
    order = sorted(range(len(allp)), key=lambda i: (CATS.index(allp[i].cat), allp[i].cat == 'Init' and allp[i].name != 'Init', allp[i].name))
    for i in order:
        p = allp[i]
        out.append('    { %s, %s, %s, %du, %du, kSV%d, 0 },' % (cstr(p.name), cstr(p.cat), cstr(p.tag_str), span6[i][0], span6[i][1], i))
    for (nm, cat, tags, d0, dn, st) in ents:
        out.append('    { "%s", "%s", "%s", %du, %du, kSL%d, 1 },' % (nm, cat, tags, d0, dn, st))
    out.append('};')
    out.append('static constexpr uint32_t kFactoryPresetCount = sizeof(kFactoryPresets) / sizeof(kFactoryPresets[0]);')
    out.append('static constexpr uint32_t kFactoryBank6Count = %du;   // entries 0 .. this-1 are the 6.0 bank' % len(allp))
    cf = os.path.join(HERE, 'preset_cpu6.json')
    cp = json.load(open(cf)) if os.path.exists(cf) else {}
    vals = [max(0, min(255, int(round(float(cp.get(allp[i].name, 0.0)) * 100)))) for i in order] + cpu
    out.append('static const uint8_t kPresetCpu[] = {')
    for i in range(0, len(vals), 32): out.append('    ' + ','.join(str(x) for x in vals[i:i + 32]) + ',')
    out.append('};')
    open(out_path, 'w').write('\n'.join(out) + '\n')
    c = Counter(p.cat for p in allp)
    print('6.0 bank: %d presets' % len(allp))
    for k in CATS: print('  %-20s %d' % (k, c.get(k, 0)))

if __name__ == '__main__':
    main()
