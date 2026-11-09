#!/usr/bin/env python3
"""Driftfield 2 factory preset generator -> FactoryPresets.inc

Every preset starts from the parameter defaults and applies only what it sets.
Effects policy for the factory bank: chorus / ensemble / flanger / phaser / drive only — no delay, no reverb.
"""
import math, random, sys

OSC = {'a': 'oa', 'b': 'ob', 'c': 'oc'}
TYPES = {'wt': 0, 'analog': 1, 'add': 2, 'phys': 3}
AN_WAVE = {'saw': 0, 'pulse': 1, 'sawpulse': 2, 'tri': 3, 'sine': 4}
SPECTRA = ['Saw', 'Square', 'Triangle', 'Organ', 'Choir Ah', 'Choir Oo', 'Bell', 'Glass', 'Hollow', 'Strings', 'Reed', 'Custom']
MODELS = ['String', 'Bar', 'Bell', 'Membrane', 'Plate', 'Glass Bowl', 'Tube', 'Waveguide']
EXCITERS = ['Strike', 'Bow', 'Breath', 'Osc Input']
FILTERS = ["Off", "Ladder LP24", "Ladder LP18", "Ladder LP12", "Ladder LP6", "Ladder HP24", "Ladder HP18",
           "Ladder HP12", "Ladder HP6", "Ladder BP24", "Ladder BP12", "Ladder Notch", "Diode LP24",
           "SVF LP12", "SVF HP12", "SVF BP12", "SVF Notch", "SVF Peak", "SVF LP24", "SVF HP24", "SVF BP24",
           "MS-20 LP12", "MS-20 HP12", "SEM LP12", "SEM BP12", "SEM HP12", "SEM Notch", "SEM Morph",
           "Dual Bandpass", "Dual Notch", "Comb +", "Comb -", "Phaser 4", "Phaser 8", "Formant"]
ROUTING = {'serial': 0, 'parallel': 1, 'split': 2}
LFO_WAVE = {'sine': 0, 'tri': 1, 'sawup': 2, 'sawdown': 3, 'square': 4, 'sh': 5, 'drift': 6}
LFO_MODE = {'global': 0, 'voice': 1, 'retrig': 2}
SRC = ['—', 'LFO 1', 'LFO 2', 'LFO 3', 'LFO 4', 'Filter Env', 'Amp Env', 'Mod Env', 'Velocity', 'Pressure',
       'Slide', 'Bend', 'Mod Wheel', 'Key', 'Random']
SRC_KEY = {'lfo1': 1, 'lfo2': 2, 'lfo3': 3, 'lfo4': 4, 'fenv': 5, 'aenv': 6, 'menv': 7, 'vel': 8, 'press': 9,
           'slide': 10, 'bend': 11, 'mw': 12, 'key': 13, 'rand': 14}
DST = ['none', 'f1cut', 'f1res', 'f2cut', 'f2res', 'cut', 'drive', 'balance', 'pitch', 'timbre',
       'apitch', 'alevel', 'apan', 'at1', 'at2', 'at3', 'bpitch', 'blevel', 'bpan', 'bt1', 'bt2', 'bt3',
       'cpitch', 'clevel', 'cpan', 'ct1', 'ct2', 'ct3', 'ring', 'sub', 'noise', 'amp', 'pan', 'detune', 'f1shape', 'f2shape']
SUBW = {'sine': 0, 'tri': 1, 'saw': 2, 'square': 3}
RING = {'ab': 0, 'ac': 1, 'bc': 2, 'abc': 3}
POLY = {'poly': 0, 'mono': 1, 'legato': 2}
CHORUS = {'chorus': 0, 'ensemble': 1, 'flanger': 2}


class Patch:
    def __init__(self, name, cat):
        self.name, self.cat = name, cat
        self.v = {}
        self.tables = [None, None, None]
        self.sp = [None, None, None]
        self.mods = 0
        # factory bank policy
        self.v['dlMix'] = 0.0

    def set(self, k, val): self.v[k] = float(val); return self

    def osc(self, o, typ, level=0.7, oct=0, semi=0, fine=0, pan=0, voices=1, detune=12):
        p = OSC[o]
        self.set(p + 'On', 1).set(p + 'Type', TYPES[typ]).set(p + 'Level', level).set(p + 'Oct', oct)
        self.set(p + 'Semi', semi).set(p + 'Fine', fine).set(p + 'Pan', pan).set(p + 'Voices', voices).set(p + 'Detune', detune)
        return self

    def off(self, *oscs):
        for o in oscs: self.set(OSC[o] + 'On', 0)
        return self

    def an(self, o, wave='saw', pw=0.5, sync=0, slop=0.3, **kw):
        self.osc(o, 'analog', **kw)
        p = OSC[o]
        return self.set(p + 'AnWave', AN_WAVE[wave]).set(p + 'AnPW', pw).set(p + 'AnSync', sync).set(p + 'AnSlop', slop)

    def wt(self, o, lib=None, table=0, pos=0.5, **kw):
        self.osc(o, 'wt', **kw)
        p = OSC[o]
        self.set(p + 'Table', table).set(p + 'Pos', pos)
        self.tables['abc'.index(o)] = ('lib:' + lib + '.wav') if lib else None
        return self

    def add(self, o, s1='Saw', s2='Organ', morph=0.0, partials=40, tilt=0.0, oe=0.0, stretch=0.0, peak=8, peakamt=0.0,
            width=0.3, comb=0.0, cshift=0.0, shim=0.2, srate=0.15, pdet=3, custom=None, **kw):
        self.osc(o, 'add', **kw)
        p = OSC[o]
        self.set(p + 'AdSpec1', SPECTRA.index(s1)).set(p + 'AdSpec2', SPECTRA.index(s2)).set(p + 'AdMorph', morph)
        self.set(p + 'AdPartials', partials).set(p + 'AdTilt', tilt).set(p + 'AdOddEven', oe).set(p + 'AdStretch', stretch)
        self.set(p + 'AdPeakPos', peak).set(p + 'AdPeakAmt', peakamt).set(p + 'AdPeakWidth', width)
        self.set(p + 'AdComb', comb).set(p + 'AdCombShift', cshift).set(p + 'AdShimmer', shim).set(p + 'AdShimRate', srate)
        self.set(p + 'AdDetune', pdet)
        if custom is not None:
            amps = custom
            mx = max(amps) or 1.0
            self.sp['abc'.index(o)] = ','.join(str(int(round(max(0.0, a / mx) * 1000))) for a in amps)
        return self

    def ph(self, o, model='String', exc='Bow', decay=5, damp=0.35, hard=0.5, pos=0.3, stretch=0.0, spread=0.5, **kw):
        self.osc(o, 'phys', **kw)
        p = OSC[o]
        self.set(p + 'PmModel', MODELS.index(model)).set(p + 'PmExciter', EXCITERS.index(exc)).set(p + 'PmDecay', decay)
        self.set(p + 'PmDamp', damp).set(p + 'PmHard', hard).set(p + 'PmPos', pos).set(p + 'PmStretch', stretch).set(p + 'PmSpread', spread)
        return self

    def f1(self, typ, cutoff, res=0.15, drive=0.1, key=0.3, env=0.0, shape=0.5):
        return self.set('f1Type', FILTERS.index(typ)).set('f1Cutoff', cutoff).set('f1Res', res).set('f1Drive', drive) \
                   .set('f1Key', key).set('f1Env', env).set('f1Shape', shape)

    def f2(self, typ, cutoff=3000, res=0.15, drive=0.0, key=0.3, env=0.0, shape=0.5):
        return self.set('f2Type', FILTERS.index(typ)).set('f2Cutoff', cutoff).set('f2Res', res).set('f2Drive', drive) \
                   .set('f2Key', key).set('f2Env', env).set('f2Shape', shape)

    def route(self, r, balance=0.5): return self.set('fRouting', ROUTING[r]).set('fBalance', balance)
    def fenv(self, a, d, s, r): return self.set('feA', a).set('feD', d).set('feS', s).set('feR', r)
    def aenv(self, a, d, s, r): return self.set('aeA', a).set('aeD', d).set('aeS', s).set('aeR', r)
    def menv(self, a, d, s, r): return self.set('meA', a).set('meD', d).set('meS', s).set('meR', r)

    def lfo(self, i, rate, wave='sine', mode='global', fade=0.0):
        k = 'lfo%d' % i
        return self.set(k + 'Rate', rate).set(k + 'Wave', LFO_WAVE[wave]).set(k + 'Mode', LFO_MODE[mode]).set(k + 'Fade', fade)

    def mod(self, src, dst, amt):
        assert self.mods < 12, self.name
        self.mods += 1
        k = 'mod%d' % self.mods
        return self.set(k + 'Src', SRC_KEY[src]).set(k + 'Dst', DST.index(dst)).set(k + 'Amt', amt)

    def sub(self, level, wave='sine', oct=-1): return self.set('subLevel', level).set('subWave', SUBW[wave]).set('subOct', oct)
    def noise(self, level, color=0.3): return self.set('noiseLevel', level).set('noiseColor', color)
    def ring(self, level, src='ab'): return self.set('ringLevel', level).set('ringSource', RING[src])
    def chorus(self, mode='chorus', mix=0.3, rate=0.3, depth=0.5, fb=0.0):
        return self.set('chMode', CHORUS[mode]).set('chMix', mix).set('chRate', rate).set('chDepth', depth).set('chFb', fb)
    def nochorus(self): return self.set('chMix', 0)
    def phaser(self, mix=0.4, rate=0.1, depth=0.7, fb=0.5):
        return self.set('phMix', mix).set('phRate', rate).set('phDepth', depth).set('phFb', fb)
    def drive(self, x): return self.set('fxDrive', x)
    def voice(self, mode='poly', glide=0.001): return self.set('polyMode', POLY[mode]).set('glide', glide)
    def mpe(self, pressCut=None, pressAmp=None, slideCut=None, slideTimbre=None, velAmp=None, mwCut=None):
        for k, val in [('pressCut', pressCut), ('pressAmp', pressAmp), ('slideCut', slideCut),
                       ('slideTimbre', slideTimbre), ('velAmp', velAmp), ('mwCut', mwCut)]:
            if val is not None: self.set(k, val)
        return self
    def misc(self, drift=None, spread=None):
        if drift is not None: self.set('drift', drift)
        if spread is not None: self.set('spread', spread)
        return self


PRESETS = []
def P(name, cat):
    p = Patch(name, cat)
    PRESETS.append(p)
    return p


# ------------------------------------------------------------------------------------ custom spectra
def spec_formant(peaks, base_tilt=0.7, n=64):
    out = []
    for i in range(n):
        h = i + 1
        a = h ** -base_tilt * 0.08
        for c, w, g in peaks: a += g * math.exp(-0.5 * ((h - c) / w) ** 2)
        out.append(a)
    return out

def spec_series(hs, decay=0.0, n=64):
    out = [0.0] * n
    for k, h in enumerate(hs):
        if 1 <= h <= n: out[h - 1] = max(out[h - 1], (1.0 / (1 + k * decay)))
    return out

def spec_random(seed, tilt=0.8, density=0.5, n=64):
    rnd = random.Random(seed)
    return [((i + 1) ** -tilt) * (rnd.random() if rnd.random() < density or i < 2 else 0.0) for i in range(n)]

OVERTONE = spec_formant([(1, 0.6, 0.7), (8, 0.7, 1.0), (16, 1.2, 0.25)])
THROAT = spec_formant([(1, 0.6, 1.0), (3, 0.8, 0.6), (6, 1.0, 0.7), (12, 1.5, 0.3)], 0.9)
PRIMES = spec_series([1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61], 0.25)
OCTAVES = spec_series([1, 2, 4, 8, 16, 32, 64], 0.6)
FIFTHS = spec_series([1, 3, 9, 27], 0.4)
GONG = spec_random(11, 0.55, 0.45)
EMBER = spec_formant([(1, 0.6, 1.0), (5, 2.0, 0.5), (11, 3.0, 0.25)], 1.1)
CATHEDRAL = spec_series([1, 2, 3, 4, 6, 8, 12, 16, 24, 32], 0.15)
DESERT = spec_formant([(2, 0.7, 0.6), (7, 1.5, 0.8), (13, 2.5, 0.4), (21, 3.0, 0.2)], 0.6)
AURORA = spec_formant([(1, 0.6, 0.5), (4, 0.6, 0.7), (9, 0.8, 0.6), (17, 1.0, 0.45), (33, 2.0, 0.3)], 0.5)
HOLLOW_WIND = [((i + 1) ** -0.5) * (1.0 if (i + 1) % 2 else 0.12) * (1 + 2 * math.exp(-0.5 * ((i + 1 - 19) / 4.0) ** 2)) for i in range(64)]
BOWL = spec_series([1, 3, 6, 10, 15, 21, 28, 36, 45, 55], 0.5)    # triangular numbers
VOID = spec_formant([(1, 0.6, 1.0), (2, 0.5, 0.5)], 2.0)

# =============================================================================================== DRONES
P('Dreamtime Ground', 'Drones') \
    .an('a', 'saw', voices=3, detune=6, level=0.7, slop=0.4).an('b', 'saw', oct=-1, level=0.55, slop=0.4, fine=-4) \
    .wt('c', 'Vocal/Throat Drone', pos=0.3, level=0.35).f1('Formant', 260, res=0.55, key=0.0, shape=0.45) \
    .f2('SEM LP12', 1800, res=0.1).aenv(4, 6, 0.95, 14).lfo(1, 0.045, 'tri').lfo(2, 0.11, 'drift', 'voice') \
    .mod('lfo1', 'f1cut', 0.22).mod('lfo2', 'f1shape', 0.25).mod('lfo1', 'ct1', 0.35).sub(0.35).chorus('chorus', 0.25, 0.12, 0.4)

P('Silent Structures', 'Drones') \
    .an('a', 'sawpulse', pw=0.35, voices=5, detune=10, slop=0.35).an('b', 'saw', oct=-1, voices=3, detune=8, level=0.5) \
    .off('c').f1('Ladder LP24', 380, res=0.35, drive=0.25, env=0.25, key=0.4).fenv(9, 14, 0.5, 18).aenv(7, 8, 0.9, 18) \
    .lfo(1, 0.021, 'sine').lfo(2, 0.17, 'drift', 'voice').mod('lfo1', 'f1cut', 0.28).mod('lfo2', 'at1', 0.2) \
    .mod('press', 'f1cut', 0.4).chorus('ensemble', 0.35, 0.18, 0.5)

P('Kiva Breath', 'Drones') \
    .add('a', 'Hollow', 'Choir Oo', morph=0.3, partials=32, tilt=0.2, shim=0.45, srate=0.08, pdet=4, level=0.75) \
    .an('b', 'tri', oct=-1, level=0.4, slop=0.5).off('c').noise(0.22, 0.15).f1('SVF BP12', 900, res=0.3, key=0.2) \
    .f2('SEM LP12', 2600).aenv(6, 4, 1.0, 16).lfo(1, 0.03, 'drift', 'global').lfo(2, 0.09, 'sine', 'voice') \
    .mod('lfo1', 'f1cut', 0.35).mod('lfo2', 'at1', 0.4).mod('lfo1', 'noise', 0.3).chorus('chorus', 0.2, 0.08, 0.35)

P('Cave Mantra', 'Drones') \
    .add('a', 'Custom', 'Choir Ah', custom=OVERTONE, morph=0.0, partials=48, tilt=-0.1, peak=8, peakamt=0.3, width=0.2, shim=0.25, srate=0.1, level=0.8) \
    .an('b', 'saw', oct=-1, level=0.35, slop=0.3).off('c').f1('SEM LP12', 3200, res=0.1, key=0.0).aenv(5, 4, 1, 15) \
    .lfo(1, 0.025, 'tri').lfo(2, 0.06, 'drift', 'voice').mod('lfo1', 'at2', 0.35).mod('lfo2', 'at1', 0.2).sub(0.3).nochorus()

P('Ochre Horizon', 'Drones') \
    .wt('a', 'Drone/Abyssal', pos=0.4, voices=3, detune=7).wt('b', 'Drone/Ember Glow', pos=0.2, oct=1, level=0.35) \
    .add('c', 'Custom', 'Saw', custom=DESERT, partials=40, shim=0.4, srate=0.06, level=0.3, oct=1) \
    .f1('Ladder LP24', 900, res=0.2, drive=0.3, key=0.2).aenv(8, 10, 0.95, 20).lfo(1, 0.018, 'sine').lfo(3, 0.07, 'drift', 'voice') \
    .mod('lfo1', 'f1cut', 0.3).mod('lfo3', 'at1', 0.4).mod('lfo1', 'bt1', 0.5).mod('lfo3', 'ct1', 0.4).sub(0.25).chorus('chorus', 0.22, 0.1, 0.45)

P('Vespers of Stone', 'Drones') \
    .add('a', 'Custom', 'Organ', custom=CATHEDRAL, morph=0.2, partials=40, tilt=0.15, shim=0.2, srate=0.05, level=0.7) \
    .ph('b', 'Tube', 'Breath', decay=6, damp=0.4, hard=0.3, level=0.45, oct=-1).off('c') \
    .f1('SEM LP12', 2400, res=0.15).aenv(5, 6, 1, 16).lfo(1, 0.02, 'sine').mod('lfo1', 'at1', 0.4).mod('lfo1', 'f1cut', 0.15).chorus('ensemble', 0.25, 0.1, 0.35)

P('Magnetic North', 'Drones') \
    .an('a', 'pulse', pw=0.18, voices=4, detune=8, slop=0.3).an('b', 'saw', oct=-2, level=0.5).wt('c', 'Spectral/Comb Sweep', pos=0.4, level=0.3) \
    .f1('Comb +', 110, res=0.6, key=1.0).f2('Ladder LP24', 1400, res=0.25).route('serial').aenv(6, 4, 1, 14) \
    .lfo(1, 0.012, 'sine').lfo(2, 0.08, 'drift', 'voice').mod('lfo1', 'at1', 0.3).mod('lfo2', 'f2cut', 0.3).mod('lfo1', 'ct1', 0.5).sub(0.3, 'tri')

P('Desert Solstice', 'Drones') \
    .an('a', 'saw', voices=7, detune=14, slop=0.45).wt('b', 'Strings & Reeds/Reed Breath', pos=0.5, oct=1, level=0.3).off('c') \
    .f1('SEM Morph', 700, res=0.3, shape=0.15, key=0.3).f2('Phaser 4', 1200, res=0.4).aenv(9, 6, 1, 18) \
    .lfo(1, 0.016, 'tri').lfo(2, 0.05, 'drift').mod('lfo1', 'f1shape', 0.35).mod('lfo2', 'f2cut', 0.6).mod('lfo1', 'f1cut', 0.2) \
    .noise(0.08, 0.2).chorus('chorus', 0.2, 0.1, 0.4)

P('Earth Hum', 'Drones') \
    .an('a', 'sine', oct=-1, level=0.7, slop=0.2).an('b', 'tri', level=0.5, fine=3).add('c', 'Custom', 'Triangle', custom=FIFTHS, partials=30, level=0.35, shim=0.3, srate=0.05) \
    .f1('Ladder LP12', 1600, res=0.1, drive=0.4).aenv(4, 4, 1, 12).lfo(1, 0.03, 'drift').mod('lfo1', 'alevel', 0.2).mod('lfo1', 'clevel', -0.2).sub(0.4).nochorus()

P('Shadow Plateau', 'Drones') \
    .wt('a', 'Drone/Deep Murmur', pos=0.5, voices=3, detune=9).wt('b', 'Drone/Night Wind', pos=0.3, level=0.5, oct=-1).off('c') \
    .f1('Diode LP24', 450, res=0.3, drive=0.35, key=0.3).aenv(10, 8, 1, 22).lfo(1, 0.01, 'sine').lfo(2, 0.04, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.6).mod('lfo1', 'f1cut', 0.25).sub(0.3).chorus('chorus', 0.18, 0.06, 0.4)

P('Altar Smoke', 'Drones') \
    .add('a', 'Choir Oo', 'Choir Ah', morph=0.4, partials=48, shim=0.5, srate=0.1, pdet=6, level=0.7) \
    .wt('b', 'Textures/Spectral Freeze', pos=0.5, level=0.3).off('c').noise(0.1, 0.1).f1('SVF LP24', 2200, res=0.2) \
    .aenv(8, 6, 1, 20).lfo(1, 0.025, 'sine').lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.5) \
    .mod('lfo1', 'f1cut', 0.2).chorus('ensemble', 0.3, 0.12, 0.4)

P('Basalt Organ', 'Drones') \
    .add('a', 'Organ', 'Custom', custom=OCTAVES, morph=0.3, partials=40, tilt=0.1, shim=0.15, level=0.7) \
    .an('b', 'pulse', pw=0.3, oct=-1, level=0.3).off('c').f1('SEM LP12', 1800, res=0.15, drive=0.2).aenv(3, 4, 1, 10) \
    .lfo(1, 0.04, 'tri').mod('lfo1', 'at1', 0.5).mod('mw', 'at3', -0.5).sub(0.2).phaser(0.25, 0.04, 0.6, 0.4)

P('Moonless Mesa', 'Drones') \
    .an('a', 'saw', voices=5, detune=8, slop=0.4).an('b', 'pulse', pw=0.4, voices=3, detune=6, oct=-1, level=0.5).off('c') \
    .f1('Ladder LP24', 260, res=0.55, drive=0.4, key=0.5, env=0.2).fenv(12, 10, 0.4, 20).aenv(9, 8, 1, 20) \
    .lfo(1, 0.008, 'sine').lfo(2, 0.11, 'drift', 'voice').mod('lfo1', 'f1cut', 0.35).mod('lfo2', 'bt1', 0.25).mod('press', 'f1cut', 0.4) \
    .sub(0.3).chorus('chorus', 0.3, 0.1, 0.5)

P('Throat of the World', 'Drones') \
    .add('a', 'Custom', 'Choir Ah', custom=THROAT, morph=0.25, partials=48, peak=10, peakamt=0.5, width=0.15, shim=0.2, level=0.75) \
    .an('b', 'saw', oct=-1, level=0.3, slop=0.3).off('c').f1('Formant', 220, res=0.6, shape=0.4, key=0.0).f2('SEM LP12', 3000) \
    .aenv(4, 4, 1, 14).lfo(1, 0.06, 'tri').lfo(2, 0.02, 'drift').mod('lfo1', 'at2', 0.25).mod('lfo2', 'f1shape', 0.3).sub(0.25).nochorus()

P('Iron Tide', 'Drones') \
    .an('a', 'saw', voices=3, detune=5, slop=0.3).an('b', 'saw', oct=-1, sync=7, level=0.45).off('c').ring(0.15, 'ab') \
    .f1('Ladder LP18', 600, res=0.4, drive=0.5).aenv(5, 6, 1, 16).lfo(1, 0.015, 'tri').lfo(2, 0.035, 'drift') \
    .mod('lfo1', 'bt2', 0.4).mod('lfo2', 'f1cut', 0.3).sub(0.35).chorus('chorus', 0.2, 0.08, 0.4)

P('Gravity Well', 'Drones') \
    .add('a', 'Custom', 'Saw', custom=VOID, morph=0.2, partials=24, tilt=0.4, level=0.8, oct=-1, shim=0.1) \
    .an('b', 'sine', oct=-1, level=0.5, slop=0.15).wt('c', 'Drone/Undertow', pos=0.3, level=0.3) \
    .f1('Ladder LP24', 320, res=0.2, drive=0.3).aenv(6, 6, 1, 20).lfo(1, 0.01, 'sine').mod('lfo1', 'ct1', 0.6).mod('lfo1', 'at1', 0.4) \
    .sub(0.4).nochorus()

P('Sandstone Choir', 'Drones') \
    .wt('a', 'Vocal/Monk Chant', pos=0.4, voices=3, detune=6).wt('b', 'Vocal/Bass Vowels U-O-A-E-I', pos=0.2, oct=-1, level=0.4) \
    .off('c').f1('SEM LP12', 1700, res=0.15).aenv(5, 5, 1, 16).lfo(1, 0.03, 'tri').lfo(2, 0.045, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.5).chorus('ensemble', 0.3, 0.1, 0.45)

P('Black Glass River', 'Drones') \
    .wt('a', 'Drone/Black Glass', pos=0.5, voices=5, detune=10).ph('b', 'Glass Bowl', 'Bow', decay=10, damp=0.25, hard=0.2, level=0.35, oct=1) \
    .off('c').f1('SVF LP24', 1500, res=0.2).aenv(7, 6, 1, 18).lfo(1, 0.02, 'sine').lfo(2, 0.06, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.3).chorus('chorus', 0.25, 0.08, 0.4)

P('Pilgrim Drone', 'Drones') \
    .an('a', 'sawpulse', pw=0.3, voices=3, detune=5, slop=0.4).ph('b', 'String', 'Osc Input', decay=4, damp=0.5, level=0.6) \
    .off('c').f1('Ladder LP24', 900, res=0.2).aenv(5, 5, 1, 14).lfo(1, 0.03, 'drift', 'voice').mod('lfo1', 'bt1', 0.4) \
    .mod('lfo1', 'f1cut', 0.2).sub(0.25).chorus('chorus', 0.2, 0.1, 0.4)

P('Lunar Tones', 'Drones') \
    .add('a', 'Glass', 'Bell', morph=0.3, partials=40, stretch=0.15, shim=0.5, srate=0.07, level=0.6, oct=1) \
    .an('b', 'tri', oct=-1, level=0.4).add('c', 'Triangle', 'Hollow', morph=0.5, partials=16, level=0.4, shim=0.2) \
    .f1('SEM LP12', 3500, res=0.1).aenv(6, 5, 1, 18).lfo(1, 0.02, 'tri').lfo(2, 0.04, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.6).mod('lfo2', 'ct1', 0.5).nochorus()

P('Subterranean Hymn', 'Drones') \
    .an('a', 'saw', oct=-1, voices=5, detune=7, slop=0.35).wt('b', 'Additive/Cathedral Organ', pos=0.4, level=0.4).off('c') \
    .f1('SEM Morph', 500, res=0.35, shape=0.1).aenv(7, 6, 1, 18).lfo(1, 0.015, 'sine').mod('lfo1', 'f1shape', 0.25) \
    .mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'bt1', 0.4).sub(0.4, 'tri').chorus('ensemble', 0.25, 0.08, 0.35)

P('Volcanic Breath', 'Drones') \
    .an('a', 'saw', voices=3, detune=9).noise(0.3, 0.3).off('b', 'c').f1('Dual Bandpass', 500, res=0.6, shape=0.6) \
    .f2('SEM LP12', 2500).aenv(4, 4, 1, 12).lfo(1, 0.04, 'drift').lfo(2, 0.13, 'sh', 'voice') \
    .mod('lfo1', 'f1cut', 0.5).mod('lfo2', 'f1shape', 0.2).sub(0.3).chorus('chorus', 0.2, 0.15, 0.5)

P('Ancestral Low', 'Drones') \
    .ph('a', 'Tube', 'Breath', decay=4, damp=0.5, hard=0.4, level=0.8, oct=-1).an('b', 'saw', oct=-1, level=0.35).off('c') \
    .f1('Formant', 300, res=0.5, shape=0.5).aenv(3, 4, 1, 12).lfo(1, 0.08, 'tri').lfo(2, 0.03, 'drift') \
    .mod('lfo1', 'f1shape', 0.3).mod('lfo2', 'at1', 0.4).mod('press', 'alevel', 0.3).sub(0.3).nochorus()

P('Nocturnal Emanation', 'Drones') \
    .add('a', 'Custom', 'Strings', custom=AURORA, morph=0.3, partials=48, shim=0.6, srate=0.05, pdet=5, level=0.7) \
    .wt('b', 'Drone/Low Fog', pos=0.5, oct=-1, level=0.4).off('c').f1('Ladder LP12', 1300, res=0.15) \
    .aenv(9, 6, 1, 22).lfo(1, 0.012, 'sine').lfo(2, 0.03, 'drift', 'voice').mod('lfo1', 'at1', 0.5).mod('lfo2', 'at2', 0.3) \
    .mod('lfo1', 'f1cut', 0.25).chorus('chorus', 0.25, 0.06, 0.45)

# =============================================================================================== ATMOSPHERES
P('Magnificent Void', 'Atmospheres') \
    .an('a', 'saw', voices=7, detune=18, slop=0.4).wt('b', 'Textures/Aurora', pos=0.5, voices=3, detune=10, level=0.4).off('c') \
    .f1('Ladder LP24', 700, res=0.25, env=0.2).fenv(10, 12, 0.5, 20).aenv(10, 8, 1, 25).lfo(1, 0.009, 'sine') \
    .lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'f1cut', 0.3).mod('lfo2', 'bt1', 0.6).chorus('ensemble', 0.4, 0.12, 0.5)

P('Star Gate', 'Atmospheres') \
    .add('a', 'Glass', 'Custom', custom=PRIMES, morph=0.4, partials=56, stretch=0.1, shim=0.7, srate=0.15, pdet=8, level=0.6) \
    .wt('b', 'Glass & Bell/Ice Shimmer', pos=0.4, oct=1, level=0.3).off('c').f1('SVF HP12', 300, res=0.1).f2('SEM LP12', 7000) \
    .aenv(6, 6, 1, 20).lfo(1, 0.03, 'tri').lfo(2, 0.08, 'drift', 'voice').mod('lfo1', 'at1', 0.7).mod('lfo2', 'bt1', 0.5) \
    .chorus('chorus', 0.3, 0.1, 0.5)

P('Light Fields', 'Atmospheres') \
    .add('a', 'Bell', 'Glass', morph=0.5, partials=48, tilt=0.1, shim=0.6, srate=0.2, pdet=6, level=0.6, oct=1) \
    .an('b', 'tri', voices=3, detune=10, level=0.5).off('c').f1('SEM LP12', 4500, res=0.1).aenv(5, 4, 1, 16) \
    .lfo(1, 0.05, 'sine').mod('lfo1', 'at1', 0.5).mod('lfo1', 'pan', 0.2).phaser(0.3, 0.06, 0.6, 0.4)

P('Atmospheric Pressure', 'Atmospheres') \
    .noise(0.5, 0.2).wt('a', 'Textures/Breathing Haze', pos=0.4, level=0.4).off('b', 'c').f1('Ladder BP24', 900, res=0.5, key=0.0) \
    .f2('SVF LP12', 3000).aenv(5, 4, 1, 14).lfo(1, 0.02, 'drift').lfo(2, 0.07, 'sine', 'voice') \
    .mod('lfo1', 'f1cut', 0.7).mod('lfo2', 'at1', 0.4).mod('press', 'f1cut', 0.5).chorus('chorus', 0.3, 0.1, 0.5)

P('Thermal Rise', 'Atmospheres') \
    .an('a', 'pulse', pw=0.12, voices=5, detune=12).an('b', 'saw', voices=3, detune=7, oct=1, level=0.35).off('c') \
    .f1('Phaser 8', 600, res=0.6, shape=0.4).f2('Ladder LP24', 2400, res=0.15).aenv(7, 6, 1, 18) \
    .lfo(1, 0.025, 'tri').lfo(2, 0.06, 'drift', 'voice').mod('lfo1', 'f1cut', 0.6).mod('lfo2', 'at1', 0.3).chorus('chorus', 0.25, 0.1, 0.45)

P('Glacier Calving', 'Atmospheres') \
    .ph('a', 'Plate', 'Bow', decay=12, damp=0.2, hard=0.3, spread=0.8, level=0.6).wt('b', 'Glass & Bell/Frost', pos=0.5, level=0.4).off('c') \
    .noise(0.08, 0.6).f1('SVF LP24', 5000, res=0.1).aenv(4, 6, 1, 18).lfo(1, 0.02, 'drift', 'voice').lfo(2, 0.04, 'sine') \
    .mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.5).chorus('chorus', 0.2, 0.08, 0.4)

P('Spectral Dunes', 'Atmospheres') \
    .add('a', 'Saw', 'Custom', custom=DESERT, morph=0.6, partials=56, comb=0.5, cshift=0.0, shim=0.3, srate=0.08, level=0.7) \
    .off('b', 'c').f1('SEM LP12', 3000, res=0.15).aenv(6, 4, 1, 16).lfo(1, 0.015, 'sawup').lfo(2, 0.05, 'drift', 'voice') \
    .lfo(3, 0.03, 'tri').mod('lfo3', 'at1', 0.6).mod('lfo2', 'at2', 0.4).mod('lfo1', 'f1cut', 0.2).chorus('chorus', 0.25, 0.1, 0.4)

P('Northern Lights', 'Atmospheres') \
    .add('a', 'Custom', 'Glass', custom=AURORA, morph=0.2, partials=48, shim=0.7, srate=0.12, pdet=8, level=0.65, oct=1) \
    .an('b', 'saw', voices=7, detune=15, level=0.35).off('c').f1('Ladder LP12', 2800, res=0.2).aenv(8, 6, 1, 20) \
    .lfo(1, 0.018, 'sine').lfo(2, 0.07, 'drift', 'voice').mod('lfo1', 'at1', 0.6).mod('lfo2', 'at2', 0.4).mod('lfo1', 'f1cut', 0.2) \
    .chorus('ensemble', 0.35, 0.15, 0.5)

P('Deep Field', 'Atmospheres') \
    .wt('a', 'Textures/Random Walk Spectrum 2', pos=0.5, voices=5, detune=12).wt('b', 'Drone/Slow Storm', pos=0.3, level=0.4).off('c') \
    .f1('Ladder LP24', 1500, res=0.2).aenv(9, 8, 1, 22).lfo(1, 0.01, 'sine').lfo(2, 0.03, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.8).mod('lfo2', 'bt1', 0.8).mod('lfo1', 'f1cut', 0.2).chorus('chorus', 0.3, 0.08, 0.5)

P('Hollow Wind', 'Atmospheres') \
    .add('a', 'Custom', 'Hollow', custom=HOLLOW_WIND, morph=0.3, partials=48, shim=0.5, srate=0.25, pdet=10, level=0.6) \
    .noise(0.25, 0.4).off('b', 'c').f1('SVF BP12', 1200, res=0.3, key=0.2).f2('SEM LP12', 4000).aenv(5, 4, 1, 15) \
    .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.6).mod('lfo1', 'at1', 0.4).chorus('chorus', 0.25, 0.1, 0.5)

P('Canyon Echoes', 'Atmospheres') \
    .ph('a', 'Membrane', 'Breath', decay=6, damp=0.4, hard=0.3, spread=0.8, level=0.6).an('b', 'tri', voices=3, detune=8, level=0.4) \
    .off('c').f1('SVF LP12', 2500, res=0.2).aenv(4, 6, 1, 15).lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.4) \
    .mod('lfo1', 'f1cut', 0.3).chorus('chorus', 0.25, 0.1, 0.5)

P('Ether Tapestry', 'Atmospheres') \
    .wt('a', 'Vocal/Ethereal Ah', pos=0.4, voices=5, detune=10).add('b', 'Choir Ah', 'Glass', morph=0.4, partials=40, shim=0.5, level=0.4, oct=1) \
    .wt('c', 'Textures/Aurora', pos=0.5, level=0.3).f1('SEM LP12', 3200, res=0.1).aenv(7, 6, 1, 20) \
    .lfo(1, 0.02, 'tri').lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'ct1', 0.5) \
    .chorus('ensemble', 0.3, 0.12, 0.45)

P('Monsoon Distance', 'Atmospheres') \
    .noise(0.35, 0.5).an('a', 'saw', voices=5, detune=12, level=0.35).off('b', 'c').f1('Comb -', 220, res=0.7, key=1.0) \
    .f2('SVF LP12', 2200).route('serial').aenv(5, 6, 1, 15).lfo(1, 0.02, 'drift').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'noise', 0.3) \
    .chorus('chorus', 0.3, 0.1, 0.5)

P('Inner Orbit', 'Atmospheres') \
    .an('a', 'sine', voices=3, detune=8, level=0.5).an('b', 'sine', oct=1, semi=7, level=0.4).add('c', 'Glass', 'Bell', partials=32, shim=0.6, level=0.35, oct=2) \
    .ring(0.25, 'ab').f1('SEM LP12', 5000, res=0.1).aenv(4, 5, 1, 16).lfo(1, 0.03, 'sine').lfo(2, 0.11, 'drift', 'voice') \
    .mod('lfo1', 'ring', 0.2).mod('lfo2', 'ct1', 0.5).phaser(0.3, 0.05, 0.6, 0.4)

P('Liminal Hours', 'Atmospheres') \
    .wt('a', 'Spectral/Harmonic Window', pos=0.3, voices=5, detune=10).wt('b', 'Spectral/Spectral Mirror', pos=0.5, level=0.4).off('c') \
    .f1('Ladder LP24', 6500, res=0.15).aenv(8, 8, 1, 20).lfo(1, 0.015, 'tri').lfo(2, 0.025, 'sine') \
    .mod('lfo1', 'at1', 0.8).mod('lfo2', 'bt1', -0.8).mod('lfo1', 'f1cut', 0.2).chorus('chorus', 0.25, 0.08, 0.45)

P('Dust Devils', 'Atmospheres') \
    .noise(0.4, 0.6).wt('a', 'Textures/Dust Spectrum', pos=0.5, level=0.3).off('b', 'c').f1('SVF BP24', 1500, res=0.55) \
    .f2('Phaser 8', 1800, res=0.5).aenv(3, 4, 1, 12).lfo(1, 0.15, 'sh', 'voice').lfo(2, 0.05, 'drift') \
    .mod('lfo1', 'f1cut', 0.4).mod('lfo2', 'f2cut', 0.6).mod('lfo1', 'pan', 0.5).nochorus()

P('Celestial Weather', 'Atmospheres') \
    .add('a', 'Strings', 'Choir Oo', morph=0.5, partials=48, shim=0.5, srate=0.1, pdet=6, level=0.6) \
    .wt('b', 'Glass & Bell/Halo', pos=0.5, oct=1, level=0.35).off('c').f1('SEM LP12', 2800, res=0.15).aenv(8, 6, 1, 22) \
    .lfo(1, 0.012, 'sine').lfo(2, 0.04, 'drift', 'voice').mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.6).chorus('ensemble', 0.35, 0.1, 0.5)

P('Aeolian Ridge', 'Atmospheres') \
    .ph('a', 'Waveguide', 'Breath', decay=6, damp=0.3, hard=0.5, pos=0.4, level=0.6).noise(0.15, 0.4) \
    .an('b', 'tri', voices=3, detune=6, level=0.3, oct=1).off('c').f1('SVF LP12', 3500, res=0.15).aenv(4, 4, 1, 14) \
    .lfo(1, 0.04, 'drift', 'voice').mod('lfo1', 'at1', 0.5).mod('press', 'noise', 0.3).chorus('chorus', 0.25, 0.1, 0.5)

# =============================================================================================== PADS
P('OB Silk Strings', 'Pads') \
    .an('a', 'saw', voices=3, detune=9, slop=0.35).an('b', 'saw', voices=3, detune=11, fine=6, slop=0.35).off('c') \
    .f1('SEM LP12', 2200, res=0.2, env=0.25).fenv(1.5, 3, 0.6, 3).aenv(1.2, 2, 0.85, 3) \
    .lfo(1, 4.5, 'tri', 'global', fade=2.5).mod('lfo1', 'pitch', 0.004).chorus('ensemble', 0.45, 0.4, 0.55)

P('Analog Brass Pad', 'Pads') \
    .an('a', 'saw', voices=2, detune=6, slop=0.3).an('b', 'saw', voices=2, detune=8, fine=-5, slop=0.3).off('c') \
    .f1('SEM LP12', 700, res=0.25, env=0.55, key=0.4).fenv(0.35, 1.5, 0.45, 1.2).aenv(0.25, 1.5, 0.8, 1.5) \
    .mod('vel', 'f1cut', 0.2).chorus('chorus', 0.3, 0.35, 0.4)

P('Obsidian Pad', 'Pads') \
    .an('a', 'sawpulse', pw=0.4, voices=5, detune=12, slop=0.35).an('b', 'pulse', pw=0.25, voices=3, detune=10, oct=-1, level=0.5).off('c') \
    .f1('Ladder LP24', 1000, res=0.3, drive=0.3, env=0.35).fenv(3, 5, 0.5, 6).aenv(2.5, 4, 0.85, 7) \
    .lfo(1, 0.08, 'tri').lfo(2, 0.3, 'sine', 'voice').mod('lfo1', 'f1cut', 0.15).mod('lfo2', 'bt1', 0.25).chorus('ensemble', 0.35, 0.2, 0.5)

P('Hypersaw Horizon', 'Pads') \
    .an('a', 'saw', voices=7, detune=22, slop=0.25).an('b', 'saw', voices=7, detune=16, oct=1, level=0.35).off('c') \
    .f1('Ladder LP12', 2600, res=0.15, env=0.2).fenv(2, 4, 0.6, 5).aenv(1.5, 3, 0.9, 6).lfo(1, 0.06, 'sine') \
    .mod('lfo1', 'f1cut', 0.1).chorus('chorus', 0.3, 0.3, 0.5)

P('Velvet PWM', 'Pads') \
    .an('a', 'pulse', pw=0.5, voices=3, detune=7).an('b', 'pulse', pw=0.5, voices=3, detune=9, fine=4).off('c') \
    .f1('SEM LP12', 1800, res=0.15, env=0.2).aenv(1.5, 3, 0.85, 4).lfo(1, 0.35, 'tri').lfo(2, 0.27, 'tri') \
    .mod('lfo1', 'at1', 0.38).mod('lfo2', 'bt1', -0.38).chorus('ensemble', 0.4, 0.25, 0.5)

P('Saturn Rings', 'Pads') \
    .wt('a', 'Analog/Sync Saw 1-8', pos=0.2, voices=5, detune=12).wt('b', 'Glass & Bell/Crystal Squares', pos=0.4, oct=1, level=0.3).off('c') \
    .f1('Ladder LP24', 1800, res=0.2, env=0.2).aenv(3, 4, 0.9, 8).lfo(1, 0.04, 'tri').lfo(2, 0.07, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.4).chorus('chorus', 0.3, 0.2, 0.5)

P('Warm Blanket', 'Pads') \
    .an('a', 'tri', voices=3, detune=8).an('b', 'saw', voices=3, detune=10, level=0.4).off('c').f1('SEM LP12', 1300, res=0.1) \
    .aenv(2, 3, 0.9, 6).lfo(1, 0.1, 'sine').mod('lfo1', 'f1cut', 0.08).sub(0.2).chorus('ensemble', 0.35, 0.2, 0.45)

P('Glass Cathedral', 'Pads') \
    .add('a', 'Organ', 'Glass', morph=0.6, partials=48, shim=0.4, srate=0.1, pdet=4, level=0.6) \
    .an('b', 'saw', voices=5, detune=12, level=0.3).off('c').f1('SEM LP12', 3500, res=0.1).aenv(3, 4, 0.9, 10) \
    .lfo(1, 0.03, 'tri').mod('lfo1', 'at1', 0.4).chorus('ensemble', 0.35, 0.15, 0.45)

P('Rust Belt', 'Pads') \
    .an('a', 'saw', voices=3, detune=7, slop=0.5).an('b', 'pulse', pw=0.3, sync=5, level=0.5).off('c').f1('MS-20 LP12', 900, res=0.45, drive=0.4) \
    .f2('SEM HP12', 120, res=0.1).aenv(1.5, 3, 0.8, 5).lfo(1, 0.07, 'tri').mod('lfo1', 'bt2', 0.3).mod('lfo1', 'f1cut', 0.15).drive(0.2) \
    .chorus('chorus', 0.25, 0.3, 0.4)

P('Polar Ensemble', 'Pads') \
    .an('a', 'saw', voices=5, detune=10).wt('b', 'Strings & Reeds/Ensemble Soft', pos=0.6, voices=3, detune=8, level=0.5).off('c') \
    .f1('SEM LP12', 2400, res=0.1).aenv(2.5, 3, 0.9, 7).lfo(1, 0.05, 'sine').mod('lfo1', 'bt1', 0.3).chorus('ensemble', 0.5, 0.3, 0.6)

P('Sunrise Over Ash', 'Pads') \
    .an('a', 'saw', voices=5, detune=12).an('b', 'saw', voices=3, detune=8, oct=-1, level=0.45).off('c') \
    .f1('Ladder LP24', 300, res=0.3, env=0.7, drive=0.2).fenv(10, 12, 0.6, 12).aenv(6, 6, 0.95, 12) \
    .chorus('ensemble', 0.35, 0.15, 0.45)

P('Velour Choir Pad', 'Pads') \
    .wt('a', 'Vocal/Choir Aah to Ooh', pos=0.3, voices=5, detune=10).an('b', 'saw', voices=3, detune=10, level=0.3).off('c') \
    .f1('SEM LP12', 2600, res=0.1).aenv(2.5, 3, 0.9, 8).lfo(1, 0.05, 'tri').mod('lfo1', 'at1', 0.3).mod('mw', 'at1', 0.5) \
    .chorus('ensemble', 0.4, 0.2, 0.5)

P('Oberheim Sunset', 'Pads') \
    .an('a', 'saw', voices=2, detune=7, slop=0.4).an('b', 'pulse', pw=0.45, voices=2, detune=7, slop=0.4).off('c') \
    .f1('SEM Morph', 1500, res=0.3, shape=0.2, env=0.3).fenv(2, 4, 0.4, 4).aenv(1.5, 3, 0.85, 5).lfo(1, 0.12, 'tri') \
    .mod('lfo1', 'f1shape', 0.2).mod('lfo1', 'bt1', 0.25).chorus('chorus', 0.35, 0.3, 0.5)

P('Frozen Lake', 'Pads') \
    .wt('a', 'Glass & Bell/Glass Harmonica', pos=0.3, voices=5, detune=8).add('b', 'Glass', 'Triangle', morph=0.4, partials=32, shim=0.5, level=0.4) \
    .off('c').f1('SVF LP12', 4500, res=0.1).aenv(3, 4, 0.9, 10).lfo(1, 0.04, 'sine').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt1', 0.4) \
    .chorus('chorus', 0.3, 0.15, 0.45)

P('Midnight Bloom', 'Pads') \
    .an('a', 'saw', voices=5, detune=10).off('b', 'c').f1('Ladder LP24', 250, res=0.35, env=0.1).aenv(1.5, 3, 0.9, 7) \
    .mod('press', 'f1cut', 1.0).mod('press', 'amp', 0.4).mod('slide', 'at1', 0.3).mpe(pressCut=0.0, pressAmp=0.0) \
    .chorus('ensemble', 0.35, 0.2, 0.5)

P('Analog Heaven', 'Pads') \
    .an('a', 'saw', voices=3, detune=9, slop=0.35).an('b', 'saw', voices=3, detune=9, semi=7, level=0.45).off('c') \
    .f1('SEM LP12', 1600, res=0.2, env=0.3).fenv(3, 4, 0.5, 6).aenv(2, 4, 0.85, 7).lfo(1, 0.07, 'drift') \
    .mod('lfo1', 'f1cut', 0.15).chorus('ensemble', 0.4, 0.25, 0.5)

P('Driftwood Pad', 'Pads') \
    .wt('a', 'Strings & Reeds/Bowed Body', pos=0.5, voices=3, detune=8).wt('b', 'Strings & Reeds/Viola Body', pos=0.4, oct=1, level=0.35).off('c') \
    .f1('SEM LP12', 2200, res=0.15).aenv(2.5, 3, 0.9, 8).lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
    .chorus('ensemble', 0.35, 0.18, 0.5)

P('Additive Strings', 'Pads') \
    .add('a', 'Strings', 'Saw', morph=0.3, partials=56, pdet=8, shim=0.3, srate=0.3, level=0.7).off('b', 'c') \
    .f1('SEM LP12', 3000, res=0.1, env=0.2).aenv(1.5, 3, 0.9, 5).lfo(1, 5, 'tri', 'global', fade=2).mod('lfo1', 'pitch', 0.003) \
    .chorus('ensemble', 0.45, 0.3, 0.55)

P('Ghost Machine', 'Pads') \
    .an('a', 'pulse', pw=0.2, voices=3, detune=8).wt('b', 'Digital/Bitcrush Sine', pos=0.4, level=0.3).off('c') \
    .f1('Ladder LP12', 1600, res=0.3).aenv(2, 4, 0.85, 7).lfo(1, 0.09, 'sh', 'voice').mod('lfo1', 'bt1', 0.2) \
    .phaser(0.35, 0.07, 0.7, 0.5).chorus('chorus', 0.2, 0.2, 0.4)

P('Expressive Plains', 'Pads') \
    .an('a', 'saw', voices=3, detune=8).wt('b', 'Spectral/Brightness Bloom', pos=0.2, level=0.5).off('c') \
    .f1('Ladder LP24', 500, res=0.25).aenv(1, 3, 0.9, 6).mod('slide', 'bt1', 0.8).mod('press', 'f1cut', 0.6).mod('press', 'amp', 0.3) \
    .mpe(pressCut=0.0, slideTimbre=0.0).chorus('chorus', 0.3, 0.2, 0.45)

# =============================================================================================== TEXTURES
P('Shimmering Grid', 'Textures') \
    .add('a', 'Custom', 'Glass', custom=PRIMES, partials=64, shim=0.9, srate=1.2, pdet=10, level=0.6).off('b', 'c') \
    .f1('SVF HP12', 400).f2('SEM LP12', 8000).aenv(3, 4, 1, 12).lfo(1, 0.3, 'sh', 'voice').mod('lfo1', 'at2', 0.3).nochorus()

P('Particle Sea', 'Textures') \
    .wt('a', 'Textures/Dust Spectrum', pos=0.5, voices=7, detune=30).off('b', 'c').f1('SVF LP12', 4000, res=0.2) \
    .aenv(4, 4, 1, 12).lfo(1, 0.6, 'sh', 'voice').lfo(2, 0.05, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo2', 'f1cut', 0.4) \
    .mod('lfo1', 'apan', 0.6).chorus('chorus', 0.3, 0.2, 0.5)

P('Clockwork Fog', 'Textures') \
    .add('a', 'Saw', 'Square', morph=0.5, partials=48, comb=0.8, cshift=0.0, shim=0.3, level=0.6).off('b', 'c') \
    .f1('Comb +', 300, res=0.5, key=0.0).f2('SEM LP12', 3000).aenv(3, 3, 1, 10).lfo(1, 0.07, 'sawup').lfo(2, 0.03, 'drift') \
    .mod('lfo1', 'at1', 0.3).mod('lfo2', 'f1cut', 0.5).chorus('chorus', 0.2, 0.15, 0.4)

P('Ring Nebula', 'Textures') \
    .an('a', 'sine', level=0.2).an('b', 'sine', semi=5, fine=17, level=0.2).wt('c', 'FM/FM 1-3 Index Sweep', pos=0.4, level=0.25) \
    .ring(0.6, 'abc').f1('SVF LP24', 5000, res=0.15).aenv(3, 4, 1, 12).lfo(1, 0.04, 'tri').lfo(2, 0.11, 'drift', 'voice') \
    .mod('lfo1', 'ct1', 0.6).mod('lfo2', 'bpitch', 0.02).phaser(0.3, 0.05, 0.6, 0.5)

P('Metallic Rain', 'Textures') \
    .ph('a', 'Plate', 'Breath', decay=3, damp=0.5, hard=0.7, spread=0.9, level=0.6).noise(0.1, 0.8).off('b', 'c') \
    .f1('SVF HP12', 800, res=0.1).aenv(2, 4, 1, 10).lfo(1, 0.4, 'sh', 'voice').mod('lfo1', 'at1', 0.5).mod('lfo1', 'at2', 0.3).nochorus()

P('Broken Radio', 'Textures') \
    .wt('a', 'Textures/Shortwave', pos=0.5, voices=3, detune=20).wt('b', 'Digital/Bitwise Shuffle', pos=0.3, level=0.3).off('c') \
    .f1('Dual Bandpass', 1200, res=0.6, shape=0.3).aenv(1, 3, 1, 8).lfo(1, 0.25, 'sh', 'voice').lfo(2, 0.03, 'drift') \
    .mod('lfo1', 'f1cut', 0.4).mod('lfo2', 'at1', 0.6).mod('lfo1', 'bt1', 0.5).drive(0.3).nochorus()

P('Tidal Membrane', 'Textures') \
    .ph('a', 'Membrane', 'Bow', decay=8, damp=0.3, hard=0.4, spread=0.7, level=0.6).off('b', 'c').f1('SEM LP12', 3000) \
    .aenv(3, 4, 1, 14).lfo(1, 0.03, 'tri').lfo(2, 0.08, 'drift', 'voice').mod('lfo1', 'at1', 0.5).mod('lfo2', 'at2', 0.4).chorus('chorus', 0.2, 0.1, 0.4)

P('Spectral Loom', 'Textures') \
    .add('a', 'Organ', 'Bell', morph=0.5, partials=64, peak=16, peakamt=0.7, width=0.2, shim=0.4, level=0.6).off('b', 'c') \
    .f1('SEM LP12', 6000).aenv(3, 4, 1, 12).lfo(1, 0.02, 'tri').lfo(2, 0.05, 'sine').mod('lfo1', 'at2', 0.9).mod('lfo2', 'at1', 0.5) \
    .chorus('chorus', 0.25, 0.1, 0.4)

P('Insect Choir', 'Textures') \
    .an('a', 'pulse', pw=0.08, voices=7, detune=40).off('b', 'c').f1('SVF BP12', 3000, res=0.5).aenv(2, 3, 1, 8) \
    .lfo(1, 7, 'sine', 'voice').lfo(2, 0.1, 'drift', 'voice').mod('lfo1', 'amp', 0.4).mod('lfo2', 'f1cut', 0.5).mod('lfo2', 'at1', 0.3).nochorus()

P('Iron Lung', 'Textures') \
    .noise(0.5, 0.1).an('a', 'saw', oct=-1, level=0.3).off('b', 'c').f1('Formant', 400, res=0.5, shape=0.5) \
    .aenv(2, 4, 1, 8).lfo(1, 0.18, 'tri').mod('lfo1', 'f1cut', 0.6).mod('lfo1', 'noise', 0.3).nochorus()

P('Prism Fracture', 'Textures') \
    .wt('a', 'Glass & Bell/Prism', pos=0.5, voices=5, detune=15).ph('b', 'Bar', 'Osc Input', decay=3, damp=0.4, level=0.5).off('c') \
    .f1('SVF LP12', 5000).aenv(2, 4, 1, 10).lfo(1, 0.06, 'drift', 'voice').mod('lfo1', 'at1', 0.6).mod('lfo1', 'bt1', 0.4).nochorus()

P('Spirit Catcher', 'Textures') \
    .wt('a', 'Vocal/Whisper Air', pos=0.5, voices=3, detune=10).noise(0.15, 0.3).off('b', 'c').f1('Formant', 800, res=0.5) \
    .aenv(3, 4, 1, 12).lfo(1, 0.05, 'drift').lfo(2, 0.13, 'sh', 'voice').mod('lfo1', 'f1cut', 0.5).mod('lfo2', 'f1shape', 0.3) \
    .chorus('chorus', 0.3, 0.1, 0.5)

# =============================================================================================== CHOIRS & VOICES
P('Monastery Voices', 'Choirs & Voices') \
    .add('a', 'Choir Ah', 'Choir Oo', morph=0.3, partials=48, pdet=7, shim=0.3, level=0.7).an('b', 'saw', voices=3, detune=10, level=0.25).off('c') \
    .f1('Formant', 450, res=0.45, shape=0.45).f2('SEM LP12', 3500).route('parallel', 0.5).aenv(2.5, 3, 0.9, 8) \
    .lfo(1, 0.05, 'tri').mod('lfo1', 'at1', 0.4).mod('mw', 'at1', 0.6).chorus('ensemble', 0.35, 0.2, 0.5)

P('Overtone Singer', 'Choirs & Voices') \
    .add('a', 'Custom', 'Choir Oo', custom=OVERTONE, partials=48, peak=8, peakamt=0.6, width=0.1, shim=0.1, level=0.8).off('b', 'c') \
    .f1('SEM LP12', 4000).aenv(1.5, 3, 1, 6).lfo(1, 0.08, 'tri').mod('lfo1', 'at2', 0.25).mod('slide', 'at2', 0.5).sub(0.2).nochorus()

P('Cathedral Choir', 'Choirs & Voices') \
    .wt('a', 'Vocal/Soprano Vowels U-A-I', pos=0.3, voices=5, detune=12).wt('b', 'Vocal/Alto Vowels O-A-E', pos=0.4, voices=3, detune=10, level=0.5) \
    .off('c').f1('SEM LP12', 3200, res=0.1).aenv(2.5, 3, 0.9, 8).lfo(1, 0.04, 'tri').mod('lfo1', 'at1', 0.3).mod('mw', 'at1', 0.5) \
    .chorus('ensemble', 0.4, 0.2, 0.5)

P('Low Chant', 'Choirs & Voices') \
    .wt('a', 'Vocal/Bass Vowels U-O-A-E-I', pos=0.1, voices=3, detune=8).an('b', 'saw', oct=-1, level=0.3).off('c') \
    .f1('Formant', 260, res=0.5, shape=0.3).aenv(1.5, 3, 1, 6).lfo(1, 0.06, 'tri').mod('lfo1', 'f1shape', 0.3).mod('lfo1', 'at1', 0.3).sub(0.25).nochorus()

P('Breathing Choir', 'Choirs & Voices') \
    .add('a', 'Choir Oo', 'Choir Ah', partials=40, shim=0.4, srate=0.2, pdet=6, level=0.7).noise(0.12, 0.3).off('b', 'c') \
    .f1('SEM LP12', 2800).aenv(3, 3, 1, 10).lfo(1, 0.15, 'sine', 'voice').mod('lfo1', 'at1', 0.5).mod('press', 'at1', 0.5) \
    .chorus('ensemble', 0.4, 0.2, 0.5)

P('Vocoder Ghosts', 'Choirs & Voices') \
    .an('a', 'saw', voices=3, detune=8).noise(0.15, 0.6).off('b', 'c').f1('Formant', 700, res=0.6, shape=0.5) \
    .aenv(1, 2, 1, 5).lfo(1, 0.3, 'sh', 'voice').mod('lfo1', 'f1shape', 0.6).nochorus().phaser(0.2, 0.1, 0.5, 0.3)

P('Angel Hair', 'Choirs & Voices') \
    .wt('a', 'Vocal/Ethereal Ah', pos=0.6, voices=7, detune=18).add('b', 'Glass', 'Choir Ah', partials=40, shim=0.6, level=0.3, oct=1).off('c') \
    .f1('SVF LP12', 5000).aenv(4, 4, 1, 14).lfo(1, 0.03, 'tri').mod('lfo1', 'at1', 0.4).chorus('ensemble', 0.4, 0.15, 0.5)

P('Throat Singers', 'Choirs & Voices') \
    .add('a', 'Custom', 'Custom', custom=THROAT, partials=48, peak=12, peakamt=0.8, width=0.1, level=0.8).off('b', 'c') \
    .f1('SEM LP12', 3000).aenv(1, 2, 1, 6).lfo(1, 0.1, 'tri').lfo(2, 0.04, 'drift').mod('lfo1', 'at2', 0.25).mod('lfo2', 'at2', 0.15) \
    .mod('slide', 'at2', 0.6).sub(0.3).nochorus()

# =============================================================================================== BELLS & RESONATORS
P('Temple Bowl', 'Bells & Resonators') \
    .ph('a', 'Glass Bowl', 'Strike', decay=14, damp=0.2, hard=0.4, spread=0.6, level=0.9).off('b', 'c').f1('Off', 20000).route('serial') \
    .aenv(0.002, 30, 1, 14).mod('vel', 'at2', 0.4).nochorus()

P('Singing Bowl Choir', 'Bells & Resonators') \
    .ph('a', 'Glass Bowl', 'Bow', decay=14, damp=0.2, hard=0.3, spread=0.8, level=0.8).add('b', 'Custom', 'Glass', custom=BOWL, partials=55, shim=0.5, level=0.25) \
    .off('c').f1('SVF LP12', 9000).aenv(2, 4, 1, 12).lfo(1, 0.03, 'sine').mod('lfo1', 'at1', 0.3).mod('press', 'at2', 0.5).nochorus()

P('Bronze Gong', 'Bells & Resonators') \
    .ph('a', 'Plate', 'Strike', decay=16, damp=0.25, hard=0.6, spread=0.8, level=0.8).add('b', 'Custom', 'Bell', custom=GONG, partials=48, stretch=0.2, level=0.25, shim=0.4) \
    .off('c').f1('SVF LP24', 9000).aenv(0.003, 30, 1, 16).fenv(0.001, 8, 0, 8).nochorus()

P('Celesta Dream', 'Bells & Resonators') \
    .ph('a', 'Bar', 'Strike', decay=3, damp=0.4, hard=0.6, pos=0.25, level=0.8).add('b', 'Bell', 'Glass', partials=24, level=0.2, oct=1) \
    .off('c').f1('SVF LP12', 9000).aenv(0.002, 6, 0, 4).mod('vel', 'at2', 0.4).chorus('chorus', 0.2, 0.3, 0.4)

P('Iron Bells', 'Bells & Resonators') \
    .ph('a', 'Bell', 'Strike', decay=8, damp=0.3, hard=0.7, pos=0.22, spread=0.7, level=0.9).off('b', 'c') \
    .f1('SVF LP24', 8000).aenv(0.003, 30, 1, 8).mod('vel', 'at2', 0.3).nochorus()

P('Marimba Night', 'Bells & Resonators') \
    .ph('a', 'Bar', 'Strike', decay=1.2, damp=0.6, hard=0.45, pos=0.3, level=0.9).an('b', 'sine', level=0.25).off('c') \
    .f1('SEM LP12', 4500).aenv(0.002, 1.8, 0, 1.2).mod('vel', 'at2', 0.4).nochorus()

P('Plucked Sitar Air', 'Bells & Resonators') \
    .ph('a', 'Waveguide', 'Strike', decay=4, damp=0.25, hard=0.75, pos=0.15, spread=0.6, level=0.9).off('b', 'c') \
    .f1('SVF Peak', 2800, res=0.5).aenv(0.002, 8, 0.2, 4).lfo(1, 0.2, 'drift', 'voice').mod('lfo1', 'f1cut', 0.2).nochorus()

P('Resonant Tubes', 'Bells & Resonators') \
    .ph('a', 'Tube', 'Strike', decay=5, damp=0.3, hard=0.5, level=0.85).ph('b', 'Tube', 'Strike', decay=5, damp=0.3, hard=0.5, semi=7, level=0.4) \
    .off('c').f1('SVF LP12', 7000).aenv(0.003, 12, 0.3, 6).nochorus()

P('Hammered Strings', 'Bells & Resonators') \
    .ph('a', 'String', 'Strike', decay=6, damp=0.4, hard=0.7, pos=0.12, level=0.8).ph('b', 'String', 'Strike', decay=6, damp=0.4, hard=0.6, fine=6, level=0.5) \
    .off('c').f1('SEM LP12', 5000).aenv(0.002, 10, 0.2, 5).mod('vel', 'at2', 0.4).chorus('chorus', 0.2, 0.3, 0.4)

P('Resonated Pad', 'Bells & Resonators') \
    .an('a', 'saw', voices=3, detune=10, level=0.25).ph('b', 'Membrane', 'Osc Input', decay=5, damp=0.3, spread=0.8, level=0.7).off('c') \
    .f1('Ladder LP24', 2500, res=0.15).aenv(2, 4, 0.9, 10).lfo(1, 0.04, 'sine').mod('lfo1', 'bt1', 0.4).chorus('chorus', 0.25, 0.15, 0.45)

# =============================================================================================== KEYS & ORGANS
P('Additive Organ', 'Keys & Organs') \
    .add('a', 'Organ', 'Custom', custom=CATHEDRAL, morph=0.0, partials=32, shim=0.0, pdet=0, level=0.8).off('b', 'c') \
    .f1('Off', 20000).aenv(0.005, 1, 1, 0.15).lfo(1, 6, 'sine').mod('mw', 'at1', 1.0).mod('lfo1', 'pitch', 0.002) \
    .chorus('ensemble', 0.3, 0.8, 0.4)

P('Cave Organ', 'Keys & Organs') \
    .add('a', 'Custom', 'Organ', custom=OCTAVES, morph=0.5, partials=48, tilt=0.2, pdet=3, shim=0.2, level=0.7) \
    .an('b', 'pulse', pw=0.3, oct=-1, level=0.2).off('c').f1('SEM LP12', 3000).aenv(0.3, 2, 1, 4).mod('mw', 'at1', -0.5) \
    .phaser(0.3, 0.08, 0.5, 0.4)

P('Electric Piano Dusk', 'Keys & Organs') \
    .ph('a', 'Bar', 'Strike', decay=3, damp=0.5, hard=0.35, pos=0.18, level=0.8).an('b', 'sine', level=0.3).off('c') \
    .f1('SEM LP12', 3000, res=0.1).aenv(0.003, 4, 0.2, 1.5).lfo(1, 4.5, 'sine').mod('lfo1', 'pan', 0.25).chorus('chorus', 0.3, 0.4, 0.4)

P('Harmonium Haze', 'Keys & Organs') \
    .add('a', 'Reed', 'Organ', morph=0.3, partials=40, pdet=5, shim=0.2, level=0.7).noise(0.05, 0.3).off('b', 'c') \
    .f1('SEM LP12', 2500).aenv(0.2, 1, 1, 1.5).chorus('ensemble', 0.3, 0.3, 0.4)

P('Glass Keys', 'Keys & Organs') \
    .add('a', 'Glass', 'Bell', morph=0.3, partials=32, level=0.6).ph('b', 'Bar', 'Strike', decay=2, damp=0.5, hard=0.6, level=0.5, oct=1) \
    .off('c').f1('SVF LP12', 7000).aenv(0.003, 3, 0.3, 2).chorus('chorus', 0.25, 0.3, 0.4)

# =============================================================================================== BASSES
P('SEM Bass', 'Basses') \
    .an('a', 'saw', slop=0.2).an('b', 'pulse', pw=0.4, oct=-1, level=0.5).off('c').f1('SEM LP12', 400, res=0.35, env=0.5, key=0.5) \
    .fenv(0.002, 0.5, 0.2, 0.3).aenv(0.002, 1, 0.8, 0.3).voice('mono', 0.03).nochorus()

P('Low Earth Bass', 'Basses') \
    .an('a', 'sawpulse', pw=0.3).an('b', 'sine', oct=-1, level=0.5).off('c').f1('Ladder LP24', 250, res=0.3, drive=0.4, env=0.45) \
    .fenv(0.002, 0.8, 0.3, 0.5).aenv(0.003, 1.5, 0.8, 0.4).voice('legato', 0.08).nochorus()

P('Drone Bass', 'Basses') \
    .an('a', 'saw', voices=3, detune=6).an('b', 'saw', oct=-1, level=0.6).off('c').f1('Ladder LP24', 300, res=0.4, drive=0.5) \
    .aenv(0.5, 2, 1, 3).lfo(1, 0.05, 'tri').mod('lfo1', 'f1cut', 0.3).sub(0.4).nochorus()

P('Sync Growl', 'Basses') \
    .an('a', 'saw', sync=12).an('b', 'saw', oct=-1, level=0.4).off('c').f1('Ladder LP24', 600, res=0.3, drive=0.5, env=0.4) \
    .fenv(0.002, 0.6, 0.3, 0.4).menv(0.002, 0.8, 0.0, 0.5).mod('menv', 'at2', 0.6).aenv(0.002, 1, 0.8, 0.3).voice('mono', 0.02).drive(0.2).nochorus()

P('Submerged', 'Basses') \
    .an('a', 'tri').an('b', 'sine', oct=-1, level=0.6).off('c').f1('SEM LP12', 500, res=0.2).aenv(0.01, 1, 1, 0.6) \
    .sub(0.3).voice('legato', 0.1).nochorus()

P('Acid Dune', 'Basses') \
    .an('a', 'saw').off('b', 'c').f1('Diode LP24', 350, res=0.7, drive=0.5, env=0.6, key=0.4).fenv(0.002, 0.35, 0.1, 0.2) \
    .aenv(0.002, 0.6, 0.7, 0.2).voice('legato', 0.06).drive(0.3).nochorus()

# =============================================================================================== LEADS
P('OB Lead', 'Leads') \
    .an('a', 'saw', slop=0.3).an('b', 'saw', fine=8, slop=0.3).off('c').f1('SEM LP12', 1800, res=0.25, env=0.35, key=0.5) \
    .fenv(0.01, 1, 0.4, 0.6).aenv(0.01, 1, 0.9, 0.5).lfo(1, 5.2, 'tri', 'global', fade=1).mod('lfo1', 'pitch', 0.006) \
    .voice('legato', 0.08).chorus('chorus', 0.2, 0.4, 0.4)

P('Sync Scream', 'Leads') \
    .an('a', 'saw', sync=9).off('b', 'c').f1('Ladder LP24', 3000, res=0.2, env=0.2).menv(0.01, 1.5, 0.2, 0.6) \
    .mod('menv', 'at2', 0.8).mod('mw', 'at2', 0.5).aenv(0.01, 1, 0.9, 0.4).voice('legato', 0.05).drive(0.3).nochorus()

P('Desert Flute', 'Leads') \
    .ph('a', 'Tube', 'Breath', decay=2, damp=0.4, hard=0.6, level=0.8).noise(0.06, 0.6).off('b', 'c').f1('SVF LP12', 4000) \
    .aenv(0.08, 1, 1, 0.8).lfo(1, 5, 'sine', 'global', fade=1.2).mod('lfo1', 'pitch', 0.005).mod('press', 'alevel', 0.3) \
    .voice('legato', 0.06).nochorus()

P('Whistling Wire', 'Leads') \
    .add('a', 'Triangle', 'Hollow', morph=0.3, partials=16, shim=0.0, pdet=0, level=0.8).off('b', 'c').f1('SEM LP12', 5000) \
    .aenv(0.05, 1, 1, 1).lfo(1, 4.8, 'sine', 'global', fade=1.5).mod('lfo1', 'pitch', 0.006).mod('slide', 'at1', 0.8).voice('legato', 0.12).nochorus()

P('Analog Cry', 'Leads') \
    .an('a', 'pulse', pw=0.3).an('b', 'saw', semi=12, level=0.3).off('c').f1('SEM Morph', 1500, res=0.4, shape=0.3, env=0.3) \
    .fenv(0.05, 1.5, 0.5, 1).aenv(0.02, 1, 0.9, 0.8).lfo(1, 5.5, 'tri', 'global', fade=1).mod('lfo1', 'pitch', 0.005).mod('lfo1', 'at1', 0.1) \
    .voice('legato', 0.1).chorus('chorus', 0.2, 0.4, 0.4)

P('Ribbon Voice', 'Leads') \
    .wt('a', 'Vocal/Tenor Vowels A-E-I-O-U', pos=0.2).off('b', 'c').f1('SEM LP12', 3000).aenv(0.05, 1, 1, 1) \
    .mod('slide', 'at1', 1.0).mod('press', 'f1cut', 0.4).lfo(1, 5, 'sine', 'global', fade=1).mod('lfo1', 'pitch', 0.004) \
    .voice('legato', 0.08).mpe(slideTimbre=0.0).nochorus()

# =============================================================================================== SEQUENCE & PLUCKS (Berlin school)
P('Berlin Pulse', 'Sequence & Plucks') \
    .an('a', 'saw').an('b', 'pulse', pw=0.4, oct=-1, level=0.4).off('c').f1('Ladder LP24', 500, res=0.4, env=0.55, key=0.5) \
    .fenv(0.002, 0.25, 0.05, 0.25).aenv(0.002, 0.4, 0.0, 0.3).lfo(1, 0.03, 'tri').mod('lfo1', 'f1cut', 0.3).nochorus()

P('Empetus Seq', 'Sequence & Plucks') \
    .an('a', 'sawpulse', pw=0.3).off('b', 'c').f1('SEM LP12', 700, res=0.45, env=0.5, key=0.4).fenv(0.002, 0.3, 0.1, 0.3) \
    .aenv(0.002, 0.5, 0.0, 0.35).lfo(1, 0.02, 'sine').mod('lfo1', 'f1cut', 0.35).mod('lfo1', 'at1', 0.2).phaser(0.3, 0.05, 0.6, 0.4)

P('Glass Arp', 'Sequence & Plucks') \
    .ph('a', 'Glass Bowl', 'Strike', decay=1.5, damp=0.4, hard=0.7, level=0.8).add('b', 'Glass', 'Bell', partials=24, level=0.2, oct=1) \
    .off('c').f1('SVF LP12', 8000).aenv(0.002, 2, 0, 1.5).nochorus()

P('Tape Pluck', 'Sequence & Plucks') \
    .wt('a', 'Textures/Tape Saw', pos=0.5).off('b', 'c').f1('Ladder LP12', 900, res=0.3, env=0.5).fenv(0.002, 0.3, 0.1, 0.3) \
    .aenv(0.002, 0.6, 0.0, 0.4).lfo(1, 0.3, 'drift', 'voice').mod('lfo1', 'pitch', 0.002).chorus('chorus', 0.3, 0.5, 0.4)

P('Pulse Motion', 'Sequence & Plucks') \
    .an('a', 'pulse', pw=0.2).an('b', 'pulse', pw=0.35, semi=7, level=0.3).off('c').f1('Ladder LP24', 800, res=0.35, env=0.4) \
    .fenv(0.002, 0.2, 0.0, 0.2).aenv(0.002, 0.3, 0.0, 0.25).lfo(1, 0.1, 'tri').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', -0.3).nochorus()

# =============================================================================================== more drones / atmospheres
P('Spirit of the Plateau', 'Drones') \
    .an('a', 'saw', voices=5, detune=9, slop=0.4).add('b', 'Custom', 'Choir Ah', custom=EMBER, morph=0.3, partials=40, shim=0.4, level=0.4) \
    .off('c').f1('SEM LP12', 1100, res=0.25).aenv(8, 6, 1, 20).lfo(1, 0.014, 'tri').lfo(2, 0.05, 'drift', 'voice') \
    .mod('lfo1', 'f1cut', 0.3).mod('lfo2', 'bt1', 0.5).sub(0.25).chorus('ensemble', 0.3, 0.1, 0.45)

P('Dusk Ritual', 'Drones') \
    .ph('a', 'Membrane', 'Bow', decay=6, damp=0.4, hard=0.3, level=0.6, oct=-1).an('b', 'saw', voices=3, detune=6, level=0.35) \
    .off('c').f1('Formant', 350, res=0.5).f2('SEM LP12', 2500).aenv(5, 5, 1, 15).lfo(1, 0.07, 'tri').lfo(2, 0.025, 'drift') \
    .mod('lfo1', 'f1shape', 0.3).mod('lfo2', 'f1cut', 0.3).sub(0.3).nochorus()

P('Infinite Corridor', 'Atmospheres') \
    .wt('a', 'Spectral/Formant Glide', pos=0.4, voices=5, detune=12).an('b', 'saw', voices=5, detune=14, level=0.3).off('c') \
    .f1('Phaser 8', 900, res=0.5, shape=0.6).f2('Ladder LP24', 3000).aenv(8, 6, 1, 20).lfo(1, 0.01, 'tri').lfo(2, 0.02, 'sine') \
    .mod('lfo1', 'at1', 0.8).mod('lfo2', 'f1cut', 0.7).chorus('chorus', 0.3, 0.08, 0.5)

P('Mineral Light', 'Atmospheres') \
    .add('a', 'Bell', 'Custom', custom=GONG, morph=0.5, partials=56, stretch=0.1, shim=0.6, srate=0.08, pdet=4, level=0.6) \
    .an('b', 'tri', voices=3, detune=7, level=0.35, oct=-1).off('c').f1('SEM LP12', 5000).aenv(6, 5, 1, 20) \
    .lfo(1, 0.015, 'tri').mod('lfo1', 'at1', 0.8).nochorus()

P('Origin Point', 'Drones') \
    .an('a', 'sine', oct=-2, level=0.6).an('b', 'saw', oct=-1, voices=3, detune=5, level=0.4).add('c', 'Custom', 'Organ', custom=FIFTHS, level=0.3, shim=0.3) \
    .f1('Ladder LP24', 700, res=0.3, drive=0.3).aenv(6, 6, 1, 20).lfo(1, 0.012, 'sine').lfo(2, 0.03, 'drift') \
    .mod('lfo1', 'f1cut', 0.3).mod('lfo2', 'clevel', 0.3).sub(0.3, 'tri').nochorus()

P('Weightless', 'Pads') \
    .add('a', 'Triangle', 'Glass', morph=0.4, partials=24, shim=0.5, pdet=6, level=0.6).an('b', 'tri', voices=5, detune=12, level=0.4) \
    .off('c').f1('SEM LP12', 4000).aenv(4, 4, 1, 14).lfo(1, 0.03, 'sine').mod('lfo1', 'at1', 0.5).chorus('ensemble', 0.35, 0.12, 0.5)

P('Cloud Chamber', 'Textures') \
    .wt('a', 'Textures/Granular Phase Drift', pos=0.5, voices=7, detune=25).off('b', 'c').f1('SVF LP12', 3000, res=0.2) \
    .aenv(5, 4, 1, 16).lfo(1, 0.02, 'tri').lfo(2, 0.2, 'sh', 'voice').mod('lfo1', 'at1', 0.9).mod('lfo2', 'apan', 0.4).chorus('chorus', 0.3, 0.1, 0.5)

P('Stone Garden', 'Bells & Resonators') \
    .ph('a', 'Bar', 'Strike', decay=5, damp=0.3, hard=0.5, pos=0.4, spread=0.8, level=0.8).ph('b', 'Membrane', 'Strike', decay=3, damp=0.4, hard=0.3, oct=-1, level=0.5) \
    .off('c').f1('SVF LP12', 6000).aenv(0.003, 10, 0.4, 8).mod('rand', 'at1', 0.3).nochorus()

P('Long Shadows', 'Pads') \
    .an('a', 'saw', voices=5, detune=10).an('b', 'saw', voices=3, detune=8, oct=1, level=0.25).off('c') \
    .f1('Ladder LP24', 600, res=0.3, env=0.3).f2('SEM HP12', 90).fenv(6, 8, 0.4, 12).aenv(5, 6, 0.9, 14).lfo(1, 0.02, 'tri') \
    .mod('lfo1', 'f1cut', 0.2).chorus('ensemble', 0.35, 0.15, 0.5)

P('Twilight Motor', 'Textures') \
    .an('a', 'saw', voices=3, detune=10).an('b', 'saw', semi=7, voices=3, detune=10, level=0.4).off('c') \
    .f1('Ladder LP24', 1000, res=0.35).aenv(3, 4, 1, 10).lfo(3, 0.4, 'square').mod('lfo3', 'f1cut', 0.25).lfo(1, 0.03, 'tri') \
    .mod('lfo1', 'f1res', 0.2).phaser(0.35, 0.06, 0.7, 0.6)

P('Empty Quarter', 'Drones') \
    .wt('a', 'Drone/Monolith', pos=0.4, voices=3, detune=7).noise(0.12, 0.2).an('b', 'saw', oct=-1, level=0.35).off('c') \
    .f1('SEM Morph', 600, res=0.3, shape=0.3).aenv(8, 6, 1, 20).lfo(1, 0.012, 'tri').lfo(2, 0.03, 'drift', 'voice') \
    .mod('lfo1', 'f1shape', 0.3).mod('lfo2', 'at1', 0.5).sub(0.3).chorus('chorus', 0.2, 0.08, 0.4)

P('Sacred Geometry', 'Atmospheres') \
    .add('a', 'Custom', 'Bell', custom=FIFTHS, morph=0.3, partials=48, shim=0.4, level=0.5).add('b', 'Custom', 'Glass', custom=OCTAVES, partials=48, shim=0.5, level=0.4, oct=1) \
    .off('c').f1('SEM LP12', 5000).aenv(6, 4, 1, 18).lfo(1, 0.02, 'tri').lfo(2, 0.03, 'sine') \
    .mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.6).chorus('chorus', 0.25, 0.1, 0.45)

P('Bowed Metal', 'Bells & Resonators') \
    .ph('a', 'Plate', 'Bow', decay=10, damp=0.25, hard=0.5, spread=0.8, level=0.7).off('b', 'c').f1('SVF LP12', 7000) \
    .aenv(1.5, 4, 1, 10).lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.3).mod('press', 'at2', 0.6).nochorus()

P('Sun Drenched', 'Pads') \
    .an('a', 'saw', voices=7, detune=14).wt('b', 'Spectral/Tilt Dark to Bright', pos=0.6, voices=3, detune=8, level=0.4).off('c') \
    .f1('SEM LP12', 3000, res=0.15).aenv(3, 4, 0.9, 10).lfo(1, 0.05, 'tri').mod('lfo1', 'bt1', 0.3).phaser(0.25, 0.08, 0.6, 0.4) \
    .chorus('ensemble', 0.35, 0.2, 0.5)

P('Arcane Signal', 'Textures') \
    .wt('a', 'FM/FM Index Wobble', pos=0.5, voices=3, detune=8).ring(0.3, 'ab').an('b', 'sine', semi=7, level=0.2).off('c') \
    .f1('SVF BP24', 1800, res=0.4).aenv(2, 4, 1, 10).lfo(1, 0.05, 'tri').lfo(2, 0.2, 'sh', 'voice') \
    .mod('lfo1', 'at1', 0.6).mod('lfo2', 'f1cut', 0.3).nochorus()

P('Red Rock Resonance', 'Drones') \
    .an('a', 'saw', voices=3, detune=7).ph('b', 'Tube', 'Osc Input', decay=6, damp=0.35, level=0.6).ph('c', 'String', 'Osc Input', decay=5, damp=0.4, level=0.4, oct=1) \
    .f1('Ladder LP24', 1200, res=0.2).aenv(5, 5, 1, 16).lfo(1, 0.03, 'drift', 'voice').mod('lfo1', 'bt1', 0.3).mod('lfo1', 'ct1', -0.3) \
    .sub(0.25).nochorus()

P('Aurora Engine', 'Atmospheres') \
    .wt('a', 'Textures/Aurora', pos=0.3, voices=5, detune=12).add('b', 'Custom', 'Strings', custom=AURORA, partials=48, shim=0.6, level=0.4) \
    .an('c', 'saw', voices=5, detune=12, oct=-1, level=0.25).f1('Ladder LP12', 2000, res=0.2).aenv(7, 6, 1, 20) \
    .lfo(1, 0.012, 'sine').lfo(2, 0.035, 'drift', 'voice').mod('lfo1', 'at1', 0.8).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'f1cut', 0.2) \
    .chorus('ensemble', 0.3, 0.1, 0.45)

P('Ice Monastery', 'Choirs & Voices') \
    .add('a', 'Choir Oo', 'Glass', morph=0.5, partials=48, shim=0.6, pdet=8, level=0.6).wt('b', 'Vocal/Choir Ooh to Eeh', pos=0.3, voices=3, detune=10, level=0.4) \
    .off('c').f1('SVF LP12', 4500).aenv(4, 4, 1, 14).lfo(1, 0.03, 'tri').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt1', 0.5).chorus('ensemble', 0.35, 0.12, 0.45)

P('Midnight Sun Lead', 'Leads') \
    .add('a', 'Saw', 'Square', morph=0.0, partials=40, pdet=0, shim=0.0, level=0.8).off('b', 'c').f1('SEM LP12', 2500, res=0.2, env=0.3) \
    .fenv(0.02, 1, 0.5, 0.6).aenv(0.02, 1, 0.9, 0.6).mod('slide', 'at1', 1.0).mod('press', 'f1cut', 0.5) \
    .lfo(1, 5, 'sine', 'global', fade=1).mod('lfo1', 'pitch', 0.005).voice('legato', 0.1).chorus('chorus', 0.2, 0.4, 0.4)

P('Init', 'Init')


# ---------------------------------------------------------------------------------------------- emit
def emit(path):
    import json, os
    lv = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'preset_levels.json')
    levels = json.load(open(lv)) if os.path.exists(lv) else {}
    for p in PRESETS:
        if p.name in levels: p.v['master'] = float(levels[p.name])
    names = set()
    out = ['// Generated by tools/gen_presets.py — do not edit by hand.', '']
    for i, p in enumerate(PRESETS):
        assert p.name not in names, p.name
        names.add(p.name)
        out.append('static const PresetSetting kPS%d[] = {' % i)
        for k, val in p.v.items():
            lit = '%.7g' % val
            if '.' not in lit and 'e' not in lit: lit += '.0'
            out.append('    { "%s", %sf },' % (k, lit))
        out.append('    { nullptr, 0.f } };')
    out.append('')
    out.append('static const FactoryPreset kFactoryPresets[] = {')
    order = sorted(range(len(PRESETS)), key=lambda i: (PRESETS[i].cat != 'Init', CATS.index(PRESETS[i].cat), PRESETS[i].name))
    for i in order:
        p = PRESETS[i]
        wt = ', '.join('"%s"' % w if w else 'nullptr' for w in p.tables)
        sp = ', '.join('"%s"' % s if s else 'nullptr' for s in p.sp)
        out.append('    { "%s", "%s", { %s }, { %s }, kPS%d },' % (p.name, p.cat, wt, sp, i))
    out.append('};')
    out.append('static constexpr uint32_t kFactoryPresetCount = sizeof(kFactoryPresets) / sizeof(kFactoryPresets[0]);')
    open(path, 'w').write('\n'.join(out) + '\n')
    print('%d presets' % len(PRESETS))
    from collections import Counter
    for c, n in Counter(p.cat for p in PRESETS).items(): print('  %-20s %d' % (c, n))

CATS = ['Init', 'Drones', 'Atmospheres', 'Pads', 'Textures', 'Choirs & Voices', 'Bells & Resonators', 'Keys & Organs',
        'Basses', 'Leads', 'Sequence & Plucks']

if __name__ == '__main__':
    emit(sys.argv[1] if len(sys.argv) > 1 else 'FactoryPresets.inc')
