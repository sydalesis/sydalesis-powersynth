# Driftfield 5.1 bank, part A: the procedural sound-design engine.
# Exec'd by gen_presets3.py after every hand-made bank, so Patch / P / X / field / the v3 shapes are in scope.
#
# A preset is composed from a section RECIPE (bank_v51_b_recipes.py): 2-3 engine layers picked from the
# section's menus, a filter pair, envelopes, a modulation network (slow LFOs, long free-running MSEGs, a
# bloom envelope, per-voice drift), 8 named macros, scenes or motion lanes, MPE routes, tuning and FX.
# Everything is seeded per (section, index), so the bank regenerates identically.
# Effects policy as everywhere: no reverb, no delay. Space comes from unison, ensemble, the Resonator,
# Spectral FX smear, granular spray and width.
import random as _rnd, zlib as _zl, math as _mm, os as _os

def _R(*k): return _rnd.Random(_zl.crc32('|'.join(str(x) for x in k).encode()))
def _u(r, a, b): return a + (b - a) * r.random()
def _lg(r, a, b): return a * (b / a) ** r.random()
def _ch(r, xs): return xs[int(r.random() * len(xs)) % len(xs)]
def _wch(r, pairs):
    tot = sum(w for _, w in pairs); x = r.random() * tot
    for it, w in pairs:
        x -= w
        if x <= 0: return it
    return pairs[-1][0]
def _p(r, prob): return r.random() < prob
def _t(x, d=3): return round(x, d)

# ------------------------------------------------------------------------------------------- library
_LIB = _os.path.join(HERE, '..', '..', 'library')
def _folder(kind, f): return sorted(n[:-4] for n in _os.listdir(_os.path.join(_LIB, kind, f)) if n.endswith('.wav'))
def _wt(*items):
    out = []
    for it in items:
        if it.endswith('/*'): out += [it[:-2] + '/' + n for n in _folder('Wavetables', it[:-2])]
        else: out.append(it)
    for t in out: assert _os.path.exists(_os.path.join(_LIB, 'Wavetables', t + '.wav')), t
    return out

WTP = {
    'warm': _wt('Ambient/Structures from Silence', 'Ambient/Stillness', 'Ambient/Blue Hour', 'Ambient/Canyon Glow', 'Ambient/Ancient Light',
                'Ambient/Altar of Light', 'Ambient/Desert Air', 'Ambient/Mirage Heat', 'Ambient/Sacred Ground', 'Ambient/Distant Choir',
                'Ambient/Ghost Strings', 'Drift Spectra/*', 'Organs/Organ Celeste', 'Organs/Organ Flutes', 'Strings & Reeds/Ensemble Soft',
                'Strings & Reeds/Flautando', 'Strings & Reeds/Bowed Body', 'Additive/Harmonic Walk 1-32', 'Additive/Fifths Stack',
                'Additive/Octave Stack', 'Chords/Chord Fifths', 'Chords/Chord Sus2', 'Chords/Chord Quartal', 'Chords/Chord Minor 9'),
    'dark': _wt('Drone/*', 'Ambient/Magnificent Void', 'Ambient/Monsoon Haze', 'Textures/Breathing Haze', 'Textures/Dust Spectrum',
                'Bass/Sub Weight', 'Organs/Organ Pedal', 'Vocal/Throat Drone', 'Vocal/Khoomei Low', 'Vocal/Monk Chant', 'Metallic/Gong Body',
                'Drift Spectra/Drift Ash', 'Drift Spectra/Drift Cinder'),
    'cosmic': _wt('Ambient/Night Sky Shimmer', 'Ambient/Ocean of Stars', 'Ambient/Celestial Fold', 'Ambient/Ice Cathedral',
                  'Glass & Bell/Halo', 'Glass & Bell/Starfield', 'Glass & Bell/Ice Shimmer', 'Glass & Bell/Prism', 'Glass & Bell/Frost',
                  'Glass & Bell/Glass Harmonica', 'Spectral/Comb Shimmer', 'Spectral/Brightness Bloom', 'Spectral/Partial Rise',
                  'Spectral/Harmonic Window', 'Spectral/Harmonic Sieve', 'Spectral/Twin Peaks', 'Textures/Aurora', 'Textures/Spectral Freeze',
                  'Drift Spectra/Drift Lumen', 'Drift Spectra/Drift Halo', 'Drift Spectra/Drift Nimbus', 'Drift Spectra/Drift Vapour',
                  'Additive/Prime Partials', 'Additive/Fibonacci Glow'),
    'earth': _wt('Organic/*', 'Ambient/Dreamtime Reed', 'Ambient/Earth Pulse', 'Ambient/Ritual Breath', 'Ambient/Sacred Ground',
                 'Ambient/Desert Air', 'Ambient/Mirage Heat', 'Vocal/Throat Drone', 'Vocal/Khoomei Low', 'Vocal/Overtone Singing',
                 'Vocal/Monk Chant', 'Metallic/Gong Body', 'Metallic/Bowl Ring', 'Metallic/Temple Bell', 'Metallic/Bronze Bars',
                 'Strings & Reeds/Reed Breath'),
    'vocal': _wt('Vocal/*', 'Vowel Journeys/*', 'Ambient/Distant Choir'),
    'analog': _wt('Analog/Double Saw Phase', 'Analog/PWM 50 to 2', 'Analog/Saw Lowpass Sweep', 'Analog/Triangle to Saw', 'Analog/Sine to Saw',
                  'Analog/Saw to Square', 'Analog/Reso Sweep Soft', 'Analog/Reso Sweep Hollow', 'Phase Distortion/PD Bend',
                  'Phase Distortion/PD Double Sine', 'Bass/Round Square', 'Bass/Pulse Body', 'Textures/Tape Saw'),
    'metal': _wt('Metallic/*', 'Glass & Bell/*'),
    'motion': _wt('Spectral/*', 'Textures/Random Walk Spectrum 1', 'Textures/Random Walk Spectrum 2', 'Textures/Random Walk Spectrum 3',
                  'Textures/Random Walk Spectrum 4', 'Textures/Granular Phase Drift', 'Stepped/*', 'Vowel Journeys/*'),
    'organ': _wt('Organs/*', 'Additive/Cathedral Organ', 'Additive/Drawbars Gospel', 'Additive/Pipe Flute', 'Additive/Octave Stack'),
    'strings': _wt('Strings & Reeds/*', 'Ambient/Ghost Strings'),
}

def _smp(f): return [f + '/' + n for n in _folder('Samples', f)]
SMP = {f: _smp(f) for f in ('Nature', 'Voices', 'Drones', 'Strings', 'Metal', 'Organic', 'Textures')}
# recordings with a clear pitch (keyed playback follows the notes); everything else plays unkeyed
PITCHED = ['Voices/Choir Ah', 'Voices/Choir Oo', 'Voices/Monk Hum Low', 'Voices/Throat Chant', 'Voices/Overtone Drone', 'Voices/Choir Cloud',
           'Voices/Choir Vowel Drift', 'Voices/Breath Choir', 'Drones/Shruti Box', 'Drones/Harmonium Reed', 'Drones/Organ Cluster',
           'Drones/Didgeridoo', 'Drones/Analog Drone', 'Drones/Tape Loop Pad', 'Drones/Spectral Shimmer', 'Strings/Bowed Cello Drone',
           'Strings/Tanpura', 'Strings/Sympathetic Drone', 'Strings/Zither Swarm', 'Metal/Singing Bowl Pair', 'Metal/Bowed Bowl',
           'Metal/Gong Swell', 'Metal/Bowed Cymbal']
for _s in PITCHED: assert _s in sum(SMP.values(), []), _s
SRC_SETS = {
    'voices': [s for s in PITCHED if s.startswith('Voices/')],
    'drones': [s for s in PITCHED if s.startswith('Drones/')],
    'strings': [s for s in PITCHED if s.startswith('Strings/')],
    'bowls': [s for s in PITCHED if s.startswith('Metal/')],
    'pitched': PITCHED,
    'earthy': ['Drones/Didgeridoo', 'Voices/Throat Chant', 'Voices/Monk Hum Low', 'Voices/Overtone Drone', 'Strings/Tanpura', 'Drones/Shruti Box',
               'Drones/Harmonium Reed', 'Metal/Gong Swell'],
    'nature': SMP['Nature'], 'organic': SMP['Organic'], 'textures': SMP['Textures'],
    'metalfx': ['Metal/Scraped Plate', 'Metal/Metal Groan', 'Metal/Rain on Tin', 'Metal/Wind Chimes', 'Metal/Bell Tree', 'Metal/Gamelan Pulse'],
    'hum': ['Drones/Sub Rumble Field', 'Drones/Electric Hum', 'Drones/Shortwave Radio', 'Drones/Tape Loop Pad'],
}
UNPITCHED = [s for s in sum(SMP.values(), []) if s not in PITCHED]

# custom LFO shapes
def _shape(fn): return [max(-1.0, min(1.0, fn(i / 64.0))) for i in range(64)]
def _rand_shape(r):
    h = [(k, _u(r, 0.2, 1.0) / k, _u(r, 0, 6.283)) for k in (1, 2, 3, 5)]
    vals = [sum(a * _mm.sin(2 * _mm.pi * k * x + ph) for k, a, ph in h) for x in [i / 64.0 for i in range(64)]]
    m = max(abs(v) for v in vals) or 1.0
    return [v / m for v in vals]
SHAPES = {'breath': BREATH, 'tide': TIDE, 'ripple': RIPPLE, 'swell': SWELL, 'tribal': TRIBAL, 'heart': HEART}

# ------------------------------------------------------------------------------------------- context
class _Ctx:
    def __init__(self, p, r, rc):
        self.p, self.r, self.rc = p, r, rc
        self.kind = {}          # osc -> engine kind
        self.targets = []       # (dest, lo, hi) movement targets that suit this patch
        self.flags = set()
        self.lfo_routes = []    # mod slots fed by LFOs / MSEGs (scaled by the Motion macro)
        self.macro_rest = [0.0] * 8
        self.engines = []
    def room(self, need=1): return self.p.slots + need <= self.rc.get('slot_cap', 27)

def _mod(c, src, dst, amt, via=None, curve=None, motion=False):
    if not c.room(): return False
    c.p.mod(src, dst, _t(amt), via=via, curve=curve)
    if motion: c.lfo_routes.append(c.p.slots)
    return True

# ------------------------------------------------------------------------------------------- engine layers
def lay_wt(c, o, pal, oct=0, level=0.5, semi=0):
    r = c.r
    t = _ch(r, WTP[pal])
    voices = _ch(r, c.rc.get('wt_voices', [1, 2, 3, 3, 4, 5]))
    warp, wamt = 'off', 0.0
    if _p(r, c.rc.get('warp', 0.15)): warp, wamt = _ch(r, ['bend+-', 'mirror', 'asym', 'bend+']), _u(r, 0.08, 0.35)
    c.p.wt(o, t, pos=_t(_u(r, 0.05, 0.9)), blend=_wch(r, [('spectral', 3), ('smooth', 2)]), warp=warp, warpamt=_t(wamt), voices=voices,
           detune=_t(_u(r, 5, 16), 1), level=_t(level), oct=oct, semi=semi, fine=_t(_u(r, -6, 6), 1), pan=_t(_u(r, -0.25, 0.25)), width=_t(_u(r, 0.5, 1.0)))
    c.kind[o] = 'wt'; c.engines.append('wt:' + t.split('/')[-1])
    c.targets += [(o + 't1', 0.15, 0.5), (o + 't4', 0.05, 0.2), (o + 'level', 0.1, 0.3)]
    if warp != 'off': c.targets.append((o + 't2', 0.1, 0.3))
    if voices > 1: c.targets.append((o + 't3', 0.2, 0.5))

AN_STYLE = {'warm': [('saw', 4), ('sawpulse', 1), ('pulse', 1)], 'hollow': [('pulse', 2), ('tri', 2)], 'sine': [('sine', 2), ('tri', 2)],
            'super': [('supersaw', 1)], 'bright': [('saw', 2), ('supersaw', 1), ('pulse', 1)]}
def lay_an(c, o, style, oct=0, level=0.5, semi=0):
    r = c.r
    wave = _wch(r, AN_STYLE[style])
    voices = 1 if wave == 'supersaw' else _ch(r, c.rc.get('an_voices', [1, 2, 2, 3, 4]))
    c.p.an(o, wave, pw=_t(_u(r, 0.2, 0.5)), slop=_t(_u(r, 0.2, 0.6)), drift=_t(_u(r, 0.15, 0.6)), voices=voices,
           detune=_t(_u(r, 5, 14), 1), level=_t(level), oct=oct, semi=semi, fine=_t(_u(r, -7, 7), 1), pan=_t(_u(r, -0.3, 0.3)), width=_t(_u(r, 0.5, 1.0)))
    c.kind[o] = 'an'; c.engines.append('an:' + wave)
    c.targets += [(o + 't3', 0.1, 0.3), (o + 'level', 0.1, 0.3)]
    if wave in ('pulse', 'sawpulse'): c.targets.append((o + 't1', 0.08, 0.25))
    if voices > 1: c.targets.append((o + 't4', 0.2, 0.45))

ADD_FLAV = {
    'choir': [('Choir Ah', 'Choir Oo'), ('Choir Oo', 'Hollow'), ('Choir Ah', 'Throat'), ('Choir Oo', 'Organ')],
    'organ': [('Organ', 'Octaves'), ('Fifths', 'Organ'), ('Octaves', 'Fifths'), ('Organ', 'Hollow')],
    'glass': [('Glass', 'Bell'), ('Bell', 'Primes'), ('Glass', 'Octaves'), ('Glass', 'Fifths')],
    'throat': [('Throat', 'Reed'), ('Throat', 'Choir Oo'), ('Throat', 'Organ')],
    'strings': [('Strings', 'Saw'), ('Strings', 'Hollow'), ('Strings', 'Choir Ah')],
    'hollow': [('Hollow', 'Square'), ('Triangle', 'Hollow'), ('Hollow', 'Octaves')],
    'primes': [('Primes', 'Fifths'), ('Primes', 'Bell'), ('Fifths', 'Primes')],
    'reed': [('Reed', 'Organ'), ('Reed', 'Throat'), ('Reed', 'Square')],
}
def lay_add(c, o, flav, oct=0, level=0.5, semi=0):
    r = c.r
    s1, s2 = _ch(r, ADD_FLAV[flav])
    if _p(r, 0.5): s1, s2 = s2, s1
    inharm = flav in ('glass', 'primes')
    peakamt = _u(r, 0.15, 0.5) if _p(r, 0.45) else 0.0
    vowel = flav in ('choir', 'throat') and _p(r, 0.7)
    kw = {}
    if vowel: kw.update(vowel=_t(_u(r, 0, 1)), vowelamt=_t(_u(r, 0.3, 0.8)))
    if _p(r, c.rc.get('add_blur', 0.3)): kw['blur'] = _t(_u(r, 0.3, 3.0))
    c.p.add(o, s1, s2, morph=_t(_u(r, 0, 1)), partials=_ch(r, [32, 40, 48, 64, 64, 80, 96]), tilt=_t(_u(r, -0.45, 0.1)), oe=_t(_u(r, -0.3, 0.3)),
            stretch=_t(_u(r, 0.0, 0.22) if inharm else _u(r, 0.0, 0.04)), peak=_ch(r, [4, 6, 8, 12, 16, 24]), peakamt=_t(peakamt), width=_t(_u(r, 0.2, 0.5)),
            comb=_t(_u(r, 0.05, 0.3)) if _p(r, 0.15) else 0.0, shim=_t(_u(r, 0.1, 0.45)), srate=_t(_lg(r, 0.03, 0.25)), pdet=_t(_u(r, 2, 9), 1),
            level=_t(level), oct=oct, semi=semi, fine=_t(_u(r, -5, 5), 1), pan=_t(_u(r, -0.25, 0.25)), **kw)
    c.kind[o] = 'add'; c.engines.append('add:' + s1)
    c.targets += [(o + 't1', 0.2, 0.6), (o + 't3', 0.1, 0.3), (o + 'level', 0.1, 0.3)]
    if peakamt: c.targets.append((o + 't2', 0.1, 0.4))
    if vowel: c.targets.append((o + 't4', 0.2, 0.6))

PH_FLAV = {
    'bowed': ([('String', 3), ('Sympathetic', 2), ('Glass Bowl', 2), ('Plate', 1), ('Gong', 1), ('Bar', 1)], 'Bow'),
    'breath': ([('Flute', 2), ('Reed', 2), ('Tube', 2)], 'Breath'),
    'rain': ([('Plate', 1), ('Glass Bowl', 1), ('String', 1), ('Sympathetic', 1), ('Bell', 1)], 'Rain'),
    'scrape': ([('Plate', 1), ('Gong', 1), ('Bar', 1)], 'Scrape'),
    'strike': ([('Bell', 2), ('Bar', 2), ('Glass Bowl', 2), ('Gong', 1), ('Membrane', 1), ('Plate', 1)], 'Mallet'),
    'pluck': ([('String', 3), ('Tanpura', 2), ('Sympathetic', 1)], 'Pluck'),
    'membrane': ([('Membrane', 3), ('Bar', 1), ('Tube', 1)], 'Strike'),
}
def lay_ph(c, o, flav, oct=0, level=0.5, semi=0):
    r = c.r
    models, exc = PH_FLAV[flav]
    model = _wch(r, models)
    sustained = exc in ('Bow', 'Breath', 'Rain', 'Scrape')
    decay = _t(_u(r, 4, 20) if sustained else _u(r, *c.rc.get('strike_decay', (1.5, 9))), 2)
    c.p.ph(o, model, exc, decay=decay, damp=_t(_u(r, 0.05, 0.35)), hard=_t(_u(r, 0.2, 0.55) if sustained else _u(r, 0.35, 0.75)), pos=_t(_u(r, 0.1, 0.4)),
           stretch=_t(_u(r, -0.05, 0.12) if model in ('Bell', 'Bar', 'Plate', 'Gong', 'Glass Bowl') else _u(r, -0.02, 0.02)), spread=_t(_u(r, 0.3, 0.8)),
           level=_t(level), oct=oct, semi=semi, fine=_t(_u(r, -4, 4), 1), pan=_t(_u(r, -0.3, 0.3)))
    c.p.ph2(o, press=_t(_u(r, 0.3, 0.65)) if exc in ('Bow', 'Breath') else None, noise=_t(_u(r, 0.0, 0.2)),
            nonlin=_t(_u(r, 0.05, 0.3)) if model in ('Gong', 'Plate') and _p(r, 0.5) else None)
    c.kind[o] = 'ph'; c.engines.append('ph:' + model)
    c.targets += [(o + 't1', 0.05, 0.25), (o + 't4', 0.05, 0.15), (o + 'level', 0.1, 0.25)]
    if exc in ('Bow', 'Breath'): c.targets.append((o + 'press', 0.1, 0.3))
    if not sustained: c.flags.add('struck')

def lay_gr(c, o, srcset, oct=0, level=0.5, keyed=None, semi=0):
    r = c.r
    pool = SRC_SETS[srcset]
    smp = _ch(r, pool)
    if keyed is None: keyed = smp in PITCHED and _p(r, 0.6)
    if keyed and smp not in PITCHED: keyed = False
    c.p.gr(o, 'keyed' if keyed else 'fixed', sample=smp, pos=_t(_u(r, 0.05, 0.8)), scan=_t(_u(r, -0.03, 0.03)), size=_t(_lg(r, 0.18, 1.2)),
           density=_ch(r, [6, 8, 10, 12, 16, 20, 24, 30]), spray=_t(_u(r, 0.1, 0.6)), prand=int(_u(r, 0, 30) if keyed else _ch(r, [0, 0, 50, 120, 300])),
           shape=_t(_u(r, 0.3, 0.7)), spread=_t(_u(r, 0.5, 1.0)), rev=_t(_u(r, 0.3, 0.5)) if _p(r, 0.2) else 0.0,
           level=_t(level), oct=oct if keyed else 0, semi=semi if keyed else 0)
    c.kind[o] = 'gr'; c.engines.append('gr:' + smp.split('/')[-1])
    c.targets += [(o + 't1', 0.05, 0.3), (o + 't2', 0.1, 0.3), (o + 't3', 0.1, 0.3), (o + 't4', 0.1, 0.4), (o + 'level', 0.1, 0.3)]
    c.flags.add('field' if not keyed else 'grain')

def lay_sp(c, o, srcset, oct=0, level=0.85, semi=0, cross=None):
    r = c.r
    src = _ch(r, SRC_SETS[srcset])
    keyed = src in PITCHED
    freeze = _p(r, c.rc.get('sp_freeze', 0.15))
    kw = dict(pos=_t(_u(r, 0.1, 0.75)), speed=_t(_ch(r, [0.05, 0.08, 0.12, 0.18, 0.25, 0.35, 0.5]) * (-1 if _p(r, 0.12) else 1)),
              blur=_t(_lg(r, 0.2, 6.0), 2), shift=0.0, stretch=_t(_u(r, -0.35, 0.35)) if _p(r, c.rc.get('sp_stretch', 0.15)) else 0.0,
              tilt=_t(_u(r, -0.3, 0.25)), formant=_t(_u(r, 0.3, 0.8)) if src.startswith('Voices/') and _p(r, 0.6) else 0.0,
              sparse=_t(_u(r, 0.1, 0.5)) if _p(r, 0.25) else 0.0, keyed=keyed, freeze=freeze)
    if _p(r, c.rc.get('sp_shift', 0.0)): kw['shift'] = _t(_ch(r, [-1, 1]) * _u(r, 8, 60), 1)
    xmode = None
    if cross is None: cross = _p(r, c.rc.get('sp_cross', 0.2))
    if cross:
        partner = _ch(r, SRC_SETS[_ch(r, c.rc.get('cross_sets', ['voices', 'nature', 'bowls', 'drones']))])
        if partner != src:
            xmode = 'vocode' if (not keyed or partner.startswith('Voices/')) and _p(r, 0.5) else 'morph'
            kw.update(cross=partner, mode=xmode, amount=_t(_u(r, 0.3, 0.85)))
    c.p.spec(o, src, level=_t(level), oct=oct if keyed else 0, semi=semi if keyed else 0, fine=_t(_u(r, -4, 4), 1), pan=_t(_u(r, -0.25, 0.25)), **kw)
    c.kind[o] = 'sp'; c.engines.append('sp:' + src.split('/')[-1])
    c.targets += [(o + 't2', 0.1, 0.4), (o + 'level', 0.1, 0.25)]
    if not freeze: c.targets.append((o + 't1', 0.05, 0.25))
    if xmode: c.targets.append((o + 't4', 0.2, 0.5))
    c.flags.add('spectral')
    if freeze: c.flags.add('frozen')

LAYERS = {'wt': lay_wt, 'an': lay_an, 'add': lay_add, 'ph': lay_ph, 'gr': lay_gr, 'sp': lay_sp}
LEVEL_BOOST = {'sp': 1.25, 'gr': 1.1, 'ph': 1.0, 'wt': 1.0, 'an': 0.9, 'add': 1.0}

# ------------------------------------------------------------------------------------------- composer
def _layers(c):
    r, rc = c.r, c.rc
    n = 3 if _p(r, rc.get('p3', 0.6)) else 2
    menus = [rc['menuA'], rc.get('menuB', rc['menuA']), rc.get('menuC', rc.get('menuB', rc['menuA']))]
    levels = [_u(r, 0.5, 0.65), _u(r, 0.3, 0.5), _u(r, 0.18, 0.38)]
    octs = rc.get('octs', [[(0, 3), (-1, 1)], [(0, 2), (-1, 1), (1, 1)], [(-1, 2), (1, 1), (0, 1)]])
    for i, o in enumerate('abc'):
        if i >= n: c.p.off(o); continue
        kind, opts = _wch(r, [((k, v), w) for k, v, w in menus[i]])
        oc = _wch(r, octs[i])
        semi = 0
        if i > 0 and _p(r, rc.get('fifths', 0.25)): semi = _ch(r, [7, 7, 7, -5, 12, 5])
        if semi == 12: semi, oc = 0, oc + 1
        lvl = min(1.0, levels[i] * LEVEL_BOOST[kind])
        LAYERS[kind](c, o, _ch(r, opts), oct=max(-3, min(3, oc)), level=lvl, semi=semi)
    c.n = n

FILT_TONE = {
    'warm': [('OB-Xa LP12', 3), ('SEM LP12', 3), ('Ladder LP24', 2), ('Ladder LP12', 2), ('Xpander LP2', 2), ('SVF LP12', 2), ('Diode LP24', 1), ('Xpander LP3', 1)],
    'open': [('SVF LP12', 3), ('Xpander LP1', 2), ('Ladder LP6', 2), ('OB-Xa LP12', 2), ('SEM LP12', 1)],
    'dark': [('Ladder LP24', 3), ('Xpander LP4', 2), ('Diode LP24', 2), ('OB-Xa LP24', 2), ('SVF LP24', 1)],
    'vocal': [('Formant', 2), ('Vowel', 2), ('Xpander BP2', 1), ('SEM BP12', 1), ('Dual Bandpass', 1)],
    'motion': [('Phs 24+', 2), ('Phs 48L6+', 1), ('Cmb L6+', 2), ('Flg L6+', 1), ('Phaser 8', 1), ('Comb +', 1), ('SEM Morph', 1), ('Xpander PH3+LP1', 1)],
    'thin': [('Xpander HP2+LP1', 2), ('SVF BP12', 1), ('OB-Xa BP', 1), ('Xpander BP4', 1)],
}
def _filters(c):
    r, rc = c.r, c.rc
    tone = _wch(r, rc.get('ftone', [('warm', 3), ('open', 1)]))
    typ = _wch(r, FILT_TONE[tone])
    lo, hi = rc.get('cut', (500, 5000))
    cut = _lg(r, lo, hi)
    res = _u(r, *rc.get('res', (0.05, 0.3)))
    if tone in ('vocal', 'motion', 'thin'): res = max(res, _u(r, 0.25, 0.55))
    kw = {}
    if _p(r, rc.get('fmorph', 0.15)):
        kw.update(morphto=_wch(r, FILT_TONE[_ch(r, ['vocal', 'motion', 'warm'])]), morph=_t(_u(r, 0.0, 0.4)))
        c.targets.append(('f1morph', 0.15, 0.4))
    # filter env is 7 octaves full scale: cap it so the sweep peaks at 8 kHz (no fizzing resonant spikes)
    famt = _u(r, *rc.get('fenv_amt', (0.0, 0.0)))
    famt = min(famt, max(0.0, _mm.log2(8000.0 / cut) / 7.0))
    c.p.f1(typ, int(cut), res=_t(res), drive=_t(_u(r, 0.0, 0.2)), key=_t(_u(r, *rc.get('key', (0.1, 0.4)))), env=_t(famt),
           shape=_t(_u(r, 0.2, 0.8)), **kw)
    c.targets += [('f1cut', *rc.get('cut_mod', (0.15, 0.45)))]
    if tone in ('vocal', 'motion'): c.targets.append(('f1shape', 0.2, 0.6))
    if _p(r, rc.get('f2', 0.3)):
        t2 = _wch(r, FILT_TONE[_ch(r, ['vocal', 'motion', 'thin', 'open'])])
        route = _wch(r, [('parallel', 2), ('serial', 2), ('split', 1)])
        c.p.f2(t2, int(_lg(r, 400, 6000)), res=_t(_u(r, 0.15, 0.5)), key=_t(_u(r, 0.0, 0.4)), shape=_t(_u(r, 0.2, 0.8))).route(route, _t(_u(r, 0.3, 0.7)))
        c.targets += [('f2cut', 0.15, 0.4), ('balance', 0.1, 0.3)] if route != 'serial' else [('f2cut', 0.15, 0.4)]
    a, d, s, rr = rc['env']
    c.p.aenv(_t(_u(r, *a), 3), _t(_u(r, *d), 2), _t(_u(r, *s)), _t(_u(r, *rr), 2)).curve(a=_t(_u(r, *rc.get('acurve', (-0.6, 0.1)))))
    if rc.get('fenv'):
        fa, fd, fs, fr = rc['fenv']
        c.p.fenv(_t(_u(r, *fa), 3), _t(_u(r, *fd), 2), _t(_u(r, *fs)), _t(_u(r, *fr), 2))
    if _p(r, rc.get('sub', 0.35)): c.p.sub(_t(_u(r, 0.1, 0.3)), _ch(r, ['sine', 'sine', 'tri']), _ch(r, [-1, -1, -2])); c.targets.append(('sub', 0.1, 0.3))
    if _p(r, rc.get('noise', 0.2)): c.p.noise(_t(_u(r, 0.02, 0.1)), _t(_u(r, 0.2, 0.8))); c.targets.append(('noise', 0.03, 0.12))
    if _p(r, rc.get('ring', 0.0)): c.p.ring(_t(_u(r, 0.03, 0.12)), 'ab'); c.targets.append(('ring', 0.03, 0.1))

def _lfo_net(c):
    """slow modulation network: each source drives 1-3 targets; the Motion macro scales them all"""
    r, rc = c.r, c.rc
    srcs = []
    lo, hi = rc.get('lfo_rates', (0.005, 0.06))
    # LFO 1: very slow drift / custom tide
    w1 = _wch(r, [('drift', 3), ('custom', 3), ('sine', 2), ('tri', 1)])
    if w1 == 'custom': c.p.lfo(1, _t(_lg(r, lo, hi), 4), shape=_ch(r, [SHAPES['tide'], SHAPES['breath'], SHAPES['ripple'], _rand_shape(r), _rand_shape(r)]))
    else: c.p.lfo(1, _t(_lg(r, lo, hi), 4), w1)
    srcs.append('lfo1')
    c.p.lfo(2, _t(_lg(r, lo * 1.5, hi * 2), 4), _ch(r, ['sine', 'tri', 'drift']), phase=_t(_u(r, 0, 1)))
    srcs.append('lfo2')
    # LFO 3: per-voice drift so every note breathes on its own
    c.p.lfo(3, _t(_lg(r, 0.03, 0.25), 3), 'drift', 'voice', phase=_t(_u(r, 0, 1)))
    # LFO 4: texture source (S&H sparkle / medium custom)
    if rc.get('sh', 0.3) and _p(r, rc.get('sh', 0.3)): c.p.lfo(4, _t(_lg(r, 0.2, 3.0), 3), 'sh', 'voice', smooth=_t(_u(r, 0.3, 0.9)))
    else: c.p.lfo(4, _t(_lg(r, 0.04, 0.4), 3), shape=_rand_shape(r))
    srcs.append('lfo4')
    c.p.lfo(5, _t(_lg(r, 0.01, 0.12), 4), _ch(r, ['sine', 'tri', 'drift']))
    srcs.append('lfo5')
    # MSEGs: a long free-running walk, plus a bloom envelope per note
    nm = 1 + (1 if _p(r, rc.get('mseg2', 0.6)) else 0)
    for m in range(1, nm + 1):
        k = _ch(r, [4, 5, 6, 7, 8])
        tl = _lg(r, *rc.get('mseg_len', (20, 120)))
        pts = [(0, _t(_u(r, 0, 1)), 0)]
        for i in range(k):
            pts.append((_t(tl / k * _u(r, 0.5, 1.5), 2), _t(_u(r, 0, 1)), _t(_u(r, -0.5, 0.5))))
        pts.append((_t(tl / k, 2), pts[0][1], 0))
        c.p.mseg(m, pts, loop=(0, len(pts) - 1), mode='free')
        srcs.append('mseg%d' % m)
    if _p(r, rc.get('bloom', 0.55)):
        att = _lg(r, *rc.get('bloom_len', (6, 40)))
        c.p.mseg(3, [(0, 0.0, 0), (_t(att, 2), 1.0, _t(_u(r, 0.1, 0.5))), (_t(att * _u(r, 0.5, 1.5), 2), _t(_u(r, 0.4, 0.9)), _t(_u(r, -0.4, 0.2)))], loop=(2, 2), mode='env')
        c.bloom = 'mseg3'
    else:
        c.bloom = None
    # routes: every source to 1-3 targets, unique (source, target)
    nroutes = int(_u(r, *rc.get('routes', (9, 15))))
    used = set()
    tg = list(c.targets) + [(d, lo2, hi2) for d, lo2, hi2 in rc.get('extra_targets', [])]
    tries = 0
    while nroutes > 0 and tries < 200 and c.room(13):
        tries += 1
        src = _ch(r, srcs + ['lfo3'])
        d, a, b = _ch(r, tg)
        if (src, d) in used: continue
        if src == 'lfo3' and not (d.endswith('t1') or d.endswith('pan') or d == 'f1cut' or d.endswith('level')): continue
        used.add((src, d))
        amt = _u(r, a, b) * (1 if _p(r, 0.75) else -1)
        if src.startswith('mseg'): amt = abs(amt)
        # routed via the Motion macro (rests at 0.5): written at double depth so the resting depth is amt
        if _mod(c, src, d, max(-1.0, min(1.0, 2 * amt)), via='m2', motion=True): nroutes -= 1
    # meta modulation: one LFO's speed breathes with another
    if _p(r, 0.5): _mod(c, _ch(r, ['lfo5', 'mseg1']), 'lfo%drate' % _ch(r, [1, 2, 4]), _u(r, 0.1, 0.35))
    if c.bloom:
        for d, a, b in r.sample(tg, min(len(tg), _ch(r, [1, 2, 2, 3]))):
            _mod(c, 'mseg3', d, _u(r, a, b))
    # per-voice pan drift
    if _p(r, 0.5): _mod(c, 'lfo3', _ch(r, ['apan', 'bpan']), _u(r, 0.1, 0.3))
    if _p(r, 0.4): _mod(c, 'vdrift', 'detune', _u(r, 0.1, 0.25))

# macro roles: (role, names, routing builder)  — routing returns a list of (dest, amount)
def _role_routes(c, role):
    r = c.r
    oscs = [o for o in 'abc' if o in c.kind]
    pick = lambda pref: [t for t in c.targets if t[0].endswith(pref)]
    if role == 'tone': return [('f1cut', _u(r, 0.35, 0.55))] + ([('f2cut', _u(r, 0.2, 0.4))] if any(t[0] == 'f2cut' for t in c.targets) else [])
    if role == 'bloom':
        ts = pick('t1')
        return ([(ts[0][0], _u(r, 0.3, 0.55))] if ts else []) + [('f1cut', _u(r, 0.15, 0.3))]
    if role == 'depth': return [('sub', _u(r, 0.25, 0.45)), ('f1cut', -_u(r, 0.05, 0.15))]
    if role == 'air': return [('noise', _u(r, 0.08, 0.2))] + ([('f2cut', _u(r, 0.2, 0.4))] if any(t[0] == 'f2cut' for t in c.targets) else [('f1res', _u(r, 0.05, 0.15))])
    if role == 'shimmer': return [('fxchorus', _u(r, 0.3, 0.5))] + ([(o + 't3', _u(r, 0.2, 0.4)) for o in oscs if c.kind[o] == 'add'][:1])
    if role == 'smear':
        if 'sfx' in c.flags: return [('fxspecblur', _u(r, 0.3, 0.6)), ('fxspecmix', _u(r, 0.15, 0.3))]
        sp = [o for o in oscs if c.kind[o] == 'sp']
        if sp: return [(sp[0] + 't2', _u(r, 0.3, 0.6))]
        return [('fxchorus', _u(r, 0.3, 0.5)), ('detune', _u(r, 0.15, 0.3))]
    if role == 'space': return [('fxwidth', _u(r, 0.4, 0.8))] + ([('fxspecmix', _u(r, 0.2, 0.4))] if 'sfx' in c.flags else [])
    if role == 'drift': return [('detune', _u(r, 0.25, 0.45))]
    if role == 'heat': return [('f1drive', _u(r, 0.2, 0.45))] + ([('fxdrive', _u(r, 0.2, 0.4))] if 'drive' in c.flags else [])
    if role == 'strings': return [('fxreso', _u(r, 0.3, 0.5))] if 'reso' in c.flags else []
    if role == 'resonance': return [('f1res', _u(r, 0.2, 0.4))]
    if role == 'layer':
        if len(oscs) < 2: return []
        return [(oscs[0] + 'level', -_u(r, 0.2, 0.35)), (oscs[1] + 'level', _u(r, 0.25, 0.4))]
    if role == 'third':
        if len(oscs) < 3: return []
        return [('clevel', _u(r, 0.25, 0.45))]
    if role == 'position':
        ts = pick('t1')
        return [(ts[-1][0], _u(r, 0.3, 0.6))] if ts else []
    if role == 'texture':
        ts = pick('t4') + pick('t3') + pick('t2')
        return [(t[0], _u(r, t[1] + 0.1, t[2] + 0.2)) for t in r.sample(ts, min(2, len(ts)))] if ts else []
    if role == 'pulse': return [('amp', _u(r, 0.15, 0.3))] if 'pulse' in c.flags else []
    if role == 'motion': return 'MOTION'
    if role == 'phaser': return [('fxphaser', _u(r, 0.3, 0.5))] if 'phaser' in c.flags else []
    if role == 'shift': return [('fxshiftmix', _u(r, 0.2, 0.4))] if 'shift' in c.flags else []
    if role == 'wow': return [('fxwow', _u(r, 0.2, 0.4))] if 'tape' in c.flags else []
    if role == 'cross':
        ts = [t for t in c.targets if t[0].endswith('t4') and c.kind.get(t[0][0]) == 'sp']
        return [(ts[0][0], _u(r, 0.3, 0.5))] if ts else []
    if role == 'speed': return [('lfo1rate', _u(r, 0.25, 0.45)), ('lfo2rate', _u(r, 0.2, 0.4))]
    return []

ROLE_NAMES = {
    'tone': ['Tone', 'Glow', 'Light', 'Ember', 'Haze', 'Warmth', 'Clarity', 'Dusk', 'Lumen', 'Brightness'],
    'motion': ['Motion', 'Drift Depth', 'Tides', 'Breathing', 'Wander', 'Currents', 'Evolution', 'Flow'],
    'bloom': ['Bloom', 'Unfold', 'Awaken', 'Open', 'Rise', 'Opening'],
    'depth': ['Depth', 'Undertow', 'Abyss', 'Earth', 'Floor', 'Gravity', 'Root'],
    'air': ['Air', 'Breath', 'Wind', 'Mist', 'Dust', 'Hiss'],
    'shimmer': ['Shimmer', 'Halo', 'Glint', 'Sparkle', 'Aura'],
    'smear': ['Smear', 'Blur', 'Melt', 'Dissolve', 'Haze Trail', 'Afterglow'],
    'space': ['Space', 'Width', 'Expanse', 'Horizon', 'Distance'],
    'drift': ['Drift', 'Detune', 'Instability', 'Waver'],
    'heat': ['Heat', 'Grit', 'Saturate', 'Burn', 'Drive'],
    'strings': ['Strings', 'Sympathy', 'Resonance Halo', 'Overtones'],
    'resonance': ['Resonance', 'Focus', 'Ring', 'Edge'],
    'layer': ['Layer Mix', 'Balance', 'Crossfade', 'Dual'],
    'third': ['Third Voice', 'Upper Layer', 'Under Layer', 'Ghost Layer'],
    'position': ['Position', 'Scan', 'Journey', 'Wander Pos'],
    'texture': ['Texture', 'Grain', 'Detail', 'Character', 'Surface'],
    'pulse': ['Pulse', 'Heartbeat', 'Throb', 'Ritual Beat'],
    'phaser': ['Phase', 'Swirl', 'Vortex'],
    'shift': ['Shift', 'Ghost Shift', 'Spiral'],
    'wow': ['Wow', 'Tape Age', 'Wobble'],
    'cross': ['Cross', 'Morph', 'Possession', 'Imprint'],
    'speed': ['Speed', 'Time', 'Tempo of Change', 'Pace'],
}

def _macros(c):
    r, rc = c.r, c.rc
    order = ['tone', 'motion'] + rc.get('macro_core', ['bloom', 'depth'])
    pool = list(rc.get('macro_pool', ['air', 'shimmer', 'smear', 'space', 'drift', 'heat', 'texture', 'layer', 'third', 'position', 'resonance', 'speed', 'strings', 'cross', 'phaser', 'shift', 'wow', 'pulse']))
    r.shuffle(pool)
    # roles that the patch can actually use get priority
    pool.sort(key=lambda role: 0 if _role_routes_ok(c, role) else 1)
    roles = order + [x for x in pool if x not in order]
    names_used = set()
    i = 1
    for role in roles:
        if i > 8: break
        if not c.room(1) and role != 'motion': break
        routes = _role_routes(c, role)
        if not routes: continue
        name = _ch(r, ROLE_NAMES[role])
        if name in names_used: name = ROLE_NAMES[role][0] if ROLE_NAMES[role][0] not in names_used else name + ' II'
        names_used.add(name)
        if routes == 'MOTION':
            # the Motion macro rests at 0.5 and scales every LFO / MSEG route (via m2), doubling depth at 1
            c.p.macro(i, name, 0.5); c.macro_rest[i - 1] = 0.5
        else:
            rest = _ch(r, [0.0, 0.0, 0.0, 0.25, 0.5])
            c.p.macro(i, name, rest); c.macro_rest[i - 1] = rest
            for d, a in routes[:2]:
                if not _mod(c, 'm%d' % i, d, a): break
        i += 1
    c.nmacros = i - 1

def _role_routes_ok(c, role):
    rr = _role_routes(c, role)
    return bool(rr)

def _scenes_motion(c):
    r, rc = c.r, c.rc
    nm = c.nmacros
    rest = c.macro_rest[:nm]
    if _p(r, rc.get('scenes', 0.45)):
        k = _ch(r, [4, 5, 6, 8])
        snaps = [rest]
        for s in range(k - 1):
            v = [min(1.0, max(0.0, x + _u(r, -0.45, 0.6))) for x in rest]
            if 1 < len(v): v[1] = rest[1]          # keep Motion where it is
            snaps.append([_t(x) for x in v])
        c.p.scenes(snaps, on=True, pos=0.0, glide=_t(_u(r, 2.0, 9.0), 1))
        c.p.st['perf'] = '0,1,2,3'
        c.flags.add('scenes')
    elif _p(r, rc.get('motion', 0.2)):
        secs = _ch(r, [16, 24, 32, 48, 64, 90, 120])
        lanes = {}
        for m in r.sample([i for i in range(1, nm + 1) if i != 2], min(nm - 1, _ch(r, [2, 3, 3, 4]))):
            base = rest[m - 1] if rest[m - 1] > 0 else _u(r, 0.15, 0.35)
            cyc = [_ch(r, [1, 1, 2, 3]), _ch(r, [2, 3, 5])]
            ph = [_u(r, 0, 1), _u(r, 0, 1)]
            a1, a2 = _u(r, 0.15, 0.35), _u(r, 0.05, 0.15)
            lanes[m] = (lambda b, cy, p_, x1, x2: (lambda t: b + x1 * _mm.sin(2 * _mm.pi * (t * cy[0] + p_[0])) + x2 * _mm.sin(2 * _mm.pi * (t * cy[1] + p_[1]))))(base, cyc, ph, a1, a2)
        if lanes:
            c.p.motion(float(secs), lanes, mode='loop')
            c.flags.add('motion')

def _fx(c):
    r, rc = c.r, c.rc
    fx = rc.get('fx', {})
    p = c.p
    if _p(r, fx.get('chorus', 0.7)):
        p.chorus(_wch(r, [('ensemble', 3), ('chorus', 2), ('solina', 1)]), _t(_u(r, 0.12, 0.35)), _t(_lg(r, 0.03, 0.3)), _t(_u(r, 0.3, 0.6)))
    else: p.nochorus()
    if _p(r, fx.get('phaser', 0.15)): p.phaser(_t(_u(r, 0.2, 0.45)), _t(_lg(r, 0.02, 0.12)), _t(_u(r, 0.5, 0.8)), _t(_u(r, 0.2, 0.6))); c.flags.add('phaser')
    if _p(r, fx.get('drive', 0.12)): p.drive(_t(_u(r, 0.08, 0.25)), _t(_u(r, 0.3, 0.8))); c.flags.add('drive')
    if _p(r, fx.get('tape', 0.15)): p.tape(_t(_u(r, 0.1, 0.3)), _t(_u(r, 0.05, 0.3)), _t(_u(r, 0.03, 0.15)), _t(_u(r, 0.5, 0.8))); c.flags.add('tape')
    if _p(r, fx.get('crush', 0.0)): p.crush(_ch(r, [10, 11, 12]), _ch(r, [2, 3, 4]), _t(_u(r, 0.1, 0.25))); c.flags.add('crush')
    if _p(r, fx.get('shift', 0.05)): p.shifter(_t(_u(r, 0.1, 1.5) * _ch(r, [-1, 1]), 2), _t(_u(r, 0.15, 0.35)), _t(_u(r, 0.1, 0.3)), _t(_u(r, 0.4, 0.8))); c.flags.add('shift')
    if _p(r, fx.get('reso', 0.25)):
        mode = _wch(r, [('held', 6), ('octaves', 2), ('fifths', 1), ('tanpura', 1)])
        p.reso(mode, getattr(c, 'root', 'D'), _ch(r, [2, 2, 3]), mix=_t(_u(r, 0.12, 0.32)), decay=_t(_u(r, 3, 15), 1), bright=_t(_u(r, 0.3, 0.7)), width=_t(_u(r, 0.5, 0.9)))
        c.flags.add('reso')
    if _p(r, fx.get('sfx', 0.3)):
        shift = 0.0
        if _p(r, fx.get('shimmer', 0.1)): shift = _ch(r, [12, 12, 7, 19, -12])
        p.sfx(mix=_t(_u(r, 0.15, 0.3) if shift else _u(r, 0.2, 0.5)), blur=_t(_lg(r, 2.0, 16.0), 1), shift=shift, tilt=_t(_u(r, -0.3, 0.15)),
              spread=_t(_u(r, 0.3, 0.8)), gate=_t(_u(r, 0.0, 0.2)))
        c.flags.add('sfx')
    if _p(r, fx.get('width', 0.4)): p.width(_t(_u(r, 1.15, 1.6)), int(_u(r, 100, 180)))
    if _p(r, fx.get('eq', 0.5)):
        p.eq(_t(_u(r, -4, 2), 1), int(_u(r, 60, 200)), _t(_u(r, -3, 2), 1), int(_lg(r, 300, 3000)), _t(_u(r, 0.5, 1.5)), _t(_u(r, *fx.get('eq_high', (-6, 1.5))), 1), int(_lg(r, 4000, 10000)))

def _tuning(c):
    r, rc = c.r, c.rc
    c.root = _ch(r, ['C', 'D', 'D', 'E', 'F', 'G', 'A', 'A'])
    if _p(r, rc.get('tune', 0.0)):
        c.p.tune(_ch(r, rc.get('scales', ['JI 5-limit Major', 'JI 5-limit Minor', 'JI 7-limit Septimal', 'Harmonics 8-16'])), c.root, glide=_t(_u(r, 0.05, 0.4)))
        c.flags.add('tuned')

def _mpe(c):
    r, rc = c.r, c.rc
    press, slide = [], []
    pt = [t for t in c.targets if t[0].endswith('press') or t[0].endswith('t2') or t[0] in ('f1cut', 'fxspecmix', 'noise') or t[0].endswith('level')]
    st = [t for t in c.targets if t[0].endswith('t1') or t[0].endswith('t4') or t[0] in ('f1cut', 'f2cut', 'f1morph', 'balance')]
    for pool, out, n in ((pt, press, 2), (st, slide, 1)):
        for d, a, b in r.sample(pool, min(n, len(pool))):
            out.append((d, _t(_u(r, a, b))))
    X(c.p, vib=_t(_u(r, *rc.get('vib', (0.0, 0.0))), 4), pa=_t(_u(r, 0.2, 0.45)), pc=_t(_u(r, 0.15, 0.35)), sc=_t(_u(r, 0.1, 0.25)), st=_t(_u(r, 0.3, 0.6)),
      vel=_t(_u(r, *rc.get('vel', (0.1, 0.3)))), press=press, slide=slide)

# worst-case cutoff budget: LFO peak + macros at full + envelope peak + two octaves of key tracking must stay
# under ~14 kHz (9 kHz when resonant) so no preset can whistle or fizz at the top of its modulation.
_CUT_DST = {'f1cut': ('f1Cutoff', 'f1Res', 'f1Env', 'f1Key'), 'f2cut': ('f2Cutoff', 'f2Res', 'f2Env', 'f2Key')}
def _cut_budget(c):
    v = c.p.v
    for dname, (kc, kr, ke, kk) in _CUT_DST.items():
        if v.get(kc) is None: continue
        di = DST.index(dname); ci = DST.index('cut')
        slots = [i for i in range(1, 33) if v.get('mod%dSrc' % i, 0) > 0 and int(v.get('mod%dDst' % i, 0)) in (di, ci)]
        if not slots: continue
        def up(i):
            src = SRC[int(v['mod%dSrc' % i])]; a = v.get('mod%dAmt' % i, 0.0)
            bip = src.startswith(('lfo', 'mseg')) or src in ('drift', 'vdrift', 'rand', 'alt', 'seq', 'bend', 'slide')
            return abs(a) if bip else max(0.0, a)
        routes = sum(up(i) for i in slots) * 5.0
        fixed = v.get(ke, 0.0) * 7.0 + v.get(kk, 0.0) * 2.0
        top = 9000.0 if v.get(kr, 0.0) >= 0.3 else 14000.0
        room = _mm.log2(top / max(20.0, v[kc])) - fixed
        if routes > room:
            k = max(0.0, room) / routes
            for i in slots: v['mod%dAmt' % i] = _t(v['mod%dAmt' % i] * k)

def compose(rc, name, idx):
    """one preset from a recipe; returns the Patch"""
    r = _R('df51', rc['cat'], rc.get('variant', ''), idx)
    p = P(name, rc['cat'])
    c = _Ctx(p, r, rc)
    _tuning(c)
    if rc.get('builder'): rc['builder'](c)          # section-specific front end (sequences, pulses, ...)
    else: _layers(c)
    _filters(c)
    _fx(c)
    if rc.get('post'): rc['post'](c)
    _lfo_net(c)
    _macros(c)
    _scenes_motion(c)
    _mpe(c)
    _cut_budget(c)
    p.misc(drift=_t(_u(r, 0.12, 0.45)), spread=_t(_u(r, 0.5, 0.9)))
    tags = set(rc.get('tags', ()))
    tags.update(r.sample(rc.get('styles', ['roach', 'corbacho']), _ch(r, [1, 1, 2])))
    tags.update(r.sample(rc.get('moods', ['evolving']), min(2, len(rc.get('moods', ['evolving'])))))
    for f in ('frozen', 'field', 'tuned'):
        if f in c.flags: tags.add({'frozen': 'freeze', 'field': 'field-recording', 'tuned': 'just-intonation'}[f])
    p.tags(*sorted(tags))
    p._ctx_engines = c.engines
    return p
