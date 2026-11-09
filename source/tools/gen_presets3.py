#!/usr/bin/env python3
"""Driftfield 3 factory preset generator -> FactoryPresets.inc

Bank = the Driftfield 2 presets (ported, v2 modulation meanings translated to v3, given macros)
     + the new v3 presets in bank_v3.py.
Every preset starts from the parameter defaults and applies only what it sets.
Effects policy: no reverb and no delay in any factory preset. Chorus / ensemble / Solina / flanger / phaser
for movement; drive, EQ and width for tone; a few character presets use tape wow or the frequency shifter.
"""
import math, random, sys, os, json, re

HERE = os.path.dirname(os.path.abspath(__file__))

OSC = {'a': 'oa', 'b': 'ob', 'c': 'oc'}
TYPES = {'wt': 0, 'analog': 1, 'add': 2, 'phys': 3, 'gran': 4, 'spec': 5}
AN_WAVE = {'saw': 0, 'pulse': 1, 'sawpulse': 2, 'tri': 3, 'sine': 4, 'supersaw': 5}
WARP = ['off', 'bend+', 'bend-', 'bend+-', 'pulse', 'sync', 'mirror', 'asym', 'quantize', 'flip']
BLEND = {'smooth': 0, 'spectral': 1, 'stepped': 2}
SPECTRA = ['Saw', 'Square', 'Triangle', 'Organ', 'Choir Ah', 'Choir Oo', 'Bell', 'Glass', 'Hollow', 'Strings', 'Reed',
           'Throat', 'Octaves', 'Fifths', 'Primes', 'Custom']
MODELS = ['String', 'Bar', 'Bell', 'Membrane', 'Plate', 'Glass Bowl', 'Tube', 'Waveguide', 'Tanpura', 'Sympathetic',
          'Reed', 'Flute', 'Gong', 'Analysed']                           # 4.2: last four
EXCITERS = ['Strike', 'Bow', 'Breath', 'Osc Input', 'Scrape', 'Rain', 'Pluck', 'Mallet', 'Sample']   # 4.2: last three
GRSRC = {'wt': 0, 'keyed': 1, 'fixed': 2}
FMSRC = {None: 0, 'off': 0, 'a': 1, 'b': 2, 'c': 3, 'sub': 4, 'noise': 5}
FMMODE = {'pm': 0, 'fm': 1, 'am': 2, 'ring': 3}
SYNCSRC = {None: 0, 'off': 0, 'a': 1, 'b': 2, 'c': 3}
FILTERS = ["Off", "Ladder LP24", "Ladder LP18", "Ladder LP12", "Ladder LP6", "Ladder HP24", "Ladder HP18",
           "Ladder HP12", "Ladder HP6", "Ladder BP24", "Ladder BP12", "Ladder Notch", "Diode LP24",
           "SVF LP12", "SVF HP12", "SVF BP12", "SVF Notch", "SVF Peak", "SVF LP24", "SVF HP24", "SVF BP24",
           "MS-20 LP12", "MS-20 HP12", "SEM LP12", "SEM BP12", "SEM HP12", "SEM Notch", "SEM Morph",
           "OB-Xa LP12", "OB-Xa LP24", "OB-Xa BP", "OB-Xa HP",
           "Xpander LP4", "Xpander LP3", "Xpander LP2", "Xpander LP1", "Xpander HP3", "Xpander HP2", "Xpander HP1",
           "Xpander BP4", "Xpander BP2", "Xpander N2", "Xpander PH3", "Xpander HP2+LP1", "Xpander HP3+LP1",
           "Xpander N2+LP1", "Xpander PH3+LP1",
           "Dual Bandpass", "Dual Notch", "Comb +", "Comb -", "Phaser 4", "Phaser 8", "Formant", "Vowel",
           'Cmb +', 'Cmb -', 'Cmb L6+', 'Cmb L6-', 'Cmb H6+', 'Cmb H6-', 'Cmb HL6+', 'Cmb HL6-', 'Flg +', 'Flg -', 'Flg L6+', 'Flg L6-', 'Flg H6+', 'Flg H6-', 'Flg HL6+', 'Flg HL6-', 'Phs 12+', 'Phs 12-', 'Phs 24+', 'Phs 24-', 'Phs 36+', 'Phs 36-', 'Phs 48+', 'Phs 48-', 'Phs 48L6+', 'Phs 48L6-', 'Phs 48H6+', 'Phs 48H6-', 'Phs 48HL6+', 'Phs 48HL6-', 'FPhs 12HL6+', 'FPhs 12HL6-']
assert len(FILTERS) == 87
DRIVETYPE = {'soft': 0, 'tube': 1, 'diode': 2, 'fold': 3}
FILTFM = {None: 0, 'off': 0, 'a': 1, 'b': 2, 'c': 3, 'noise': 4}
ROUTING = {'serial': 0, 'parallel': 1, 'split': 2}
LFO_WAVE = {'sine': 0, 'tri': 1, 'sawup': 2, 'sawdown': 3, 'square': 4, 'sh': 5, 'drift': 6, 'custom': 7}
LFO_MODE = {'global': 0, 'voice': 1, 'retrig': 2, 'oneshot': 3}
DIVS = ['8 bars', '4 bars', '2 bars', '1 bar', '1/2', '1/2T', '1/4.', '1/4', '1/4T', '1/8.', '1/8', '1/8T', '1/16.', '1/16', '1/16T', '1/32']
MSEG_MODE = {'env': 0, 'loop': 1, 'free': 2}
CURVES = {'linear': 0, 'exp': 1, 'log': 2, 's': 3, 'uni': 4, 'invert': 5, 'steps': 6}
SRC = ['none', 'lfo1', 'lfo2', 'lfo3', 'lfo4', 'lfo5', 'lfo6', 'fenv', 'aenv', 'menv', 'mseg1', 'mseg2',
       'vel', 'press', 'slide', 'bend', 'mw', 'key', 'rand', 'drift', 'vdrift', 'alt',
       'm1', 'm2', 'm3', 'm4', 'm5', 'm6', 'm7', 'm8', 'seq', 'expr', 'breath', 'mseg3', 'mseg4']
DST = ['none', 'f1cut', 'f1res', 'f1drive', 'f1shape', 'f1morph', 'f1fm', 'f2cut', 'f2res', 'f2drive', 'f2shape', 'f2morph', 'f2fm',
       'cut', 'balance', 'pitch', 'timbre', 'detune', 'pan', 'ring', 'sub', 'noise', 'amp']
for _o in 'abc':
    DST += [_o + s for s in ['pitch', 'level', 'pan', 't1', 't2', 't3', 't4', 'fm', 'detune']]
DST += ['lfo%drate' % i for i in range(1, 7)] + ['mseg1rate', 'mseg2rate',
        'fxphaser', 'fxchorus', 'fxdelay', 'fxshift', 'fxshiftmix', 'fxwow', 'fxcrush', 'fxwidth', 'fxdrive',
        'mseg3rate', 'mseg4rate', 'apress', 'bpress', 'cpress', 'fxreso', 'fxspace',   # 4.2
        'fxspecmix', 'fxspecblur']                                                      # 5.0
assert len(DST) == 76, len(DST)
SUBW = {'sine': 0, 'tri': 1, 'saw': 2, 'square': 3}
RING = {'ab': 0, 'ac': 1, 'bc': 2, 'abc': 3}
POLY = {'poly': 0, 'mono': 1, 'legato': 2}
CHORUS = {'chorus': 0, 'ensemble': 1, 'solina': 2, 'flanger': 3}
# 4.0 tuning library: names in the order of kBuiltinScaleSrc in Tuning.hpp
SCALES = re.findall(r'^\s*\{ "([^"]+)", "', open(os.path.join(HERE, '..', 'Tuning.hpp')).read(), re.M)
assert len(SCALES) == 27 and SCALES[0] == 'JI 5-limit Major', SCALES
ROOTS = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
def scale_tags(name):
    n = name.lower()
    if n.startswith('raga') or 'shruti' in n: return ('raga', 'indian', 'just-intonation')
    if n.startswith('maqam'): return ('maqam', 'quarter-tone')
    if n in ('slendro', 'pelog'): return ('gamelan',)
    if n.startswith('ji') or 'harmonic' in n or 'partch' in n or 'well-tuned' in n or 'bohlen' in n: return ('just-intonation',)
    if 'edo' in n or 'alpha' in n: return ('xenharmonic',)
    return ('temperament', 'historical')
def scala_escape(t): return t.replace('\\', '\\\\').replace('\n', '\\n').replace('\r', '')
ARP = {'up': 0, 'down': 1, 'updown': 2, 'downup': 3, 'random': 4, 'order': 5, 'chord': 6}
SEQ = {'off': 0, 'cv': 1, 'notes': 2}
QUALITY = {'eco': 0, 'normal': 1, 'hq': 2}
FXU = ['drive', 'phaser', 'chorus', 'delay', 'shift', 'tape', 'crush', 'width', 'eq', 'reso', 'space', 'spectral']
FX_DEFAULT = [0, 1, 2, 11, 9, 3, 4, 5, 6, 7, 8, 10]
SPX = {'off': 0, 'morph': 1, 'vocode': 2}
RS_MODE = {'fifths': 0, 'octaves': 1, 'major': 2, 'minor': 3, 'tanpura': 4, 'held': 5}
MO_MODE = {'off': 0, 'loop': 1, 'note': 2, 'host': 3}
MODAL = json.load(open(os.path.join(HERE, 'modal_v42.json'))) if os.path.exists(os.path.join(HERE, 'modal_v42.json')) else {}
ENGINE_TAG = {0: 'wavetable', 1: 'analog', 2: 'additive', 3: 'physical', 4: 'granular', 5: 'spectral'}


def spec_str(amps):
    mx = max(amps) or 1.0
    vals = [int(round(max(0.0, a / mx) * 1000)) for a in amps]
    while len(vals) > 1 and vals[-1] == 0: vals.pop()
    return ','.join(str(v) for v in vals)


class Patch:
    v2 = False

    def __init__(self, name, cat):
        self.name, self.cat = name, cat
        self.v = {}
        self.st = {}
        self.types = [1, 1, 0]       # defaults: A analog, B analog, C wavetable
        self.on = [True, True, False]
        self.slots = 0
        self.macros = []             # names
        self.tagset = set()
        self.gave_macros = False

    # ---------------------------------------------------------------- basics
    def set(self, k, val): self.v[k] = float(val); return self
    def tags(self, *t): self.tagset.update(t); return self

    def osc(self, o, typ, level=0.7, oct=0, semi=0, fine=0, pan=0, voices=1, detune=12, width=None, phase=None, retrig=None):
        p = OSC[o]; i = 'abc'.index(o)
        self.types[i] = TYPES[typ]; self.on[i] = True
        self.set(p + 'On', 1).set(p + 'Type', TYPES[typ]).set(p + 'Level', level).set(p + 'Oct', oct)
        self.set(p + 'Semi', semi).set(p + 'Fine', fine).set(p + 'Pan', pan).set(p + 'Voices', voices).set(p + 'Detune', detune)
        if width is not None: self.set(p + 'UniWidth', width)
        if phase is not None: self.set(p + 'Phase', phase)
        if retrig is not None: self.set(p + 'Retrig', 1 if retrig else 0)
        return self

    def off(self, *oscs):
        for o in oscs: self.set(OSC[o] + 'On', 0); self.on['abc'.index(o)] = False
        return self

    def an(self, o, wave='saw', pw=0.5, sync=0, slop=0.3, drift=None, **kw):
        self.osc(o, 'analog', **kw)
        p = OSC[o]
        self.set(p + 'AnWave', AN_WAVE[wave]).set(p + 'AnPW', pw).set(p + 'AnSync', sync).set(p + 'AnSlop', slop)
        if drift is not None: self.set(p + 'AnDrift', drift)
        return self

    def wt(self, o, lib=None, table=0, pos=0.5, warp='off', warpamt=0.0, blend='smooth', **kw):
        self.osc(o, 'wt', **kw)
        p = OSC[o]
        self.set(p + 'Table', table).set(p + 'Pos', pos).set(p + 'Warp', WARP.index(warp)).set(p + 'WarpAmt', warpamt).set(p + 'WtBlend', BLEND[blend])
        if lib: self.st['wt' + o.upper()] = 'lib:' + lib + '.wav'
        return self

    def add(self, o, s1='Saw', s2='Organ', morph=0.0, partials=40, tilt=0.0, oe=0.0, stretch=0.0, peak=8, peakamt=0.0,
            width=0.3, comb=0.0, cshift=0.0, shim=0.2, srate=0.15, pdet=3, custom=None, vowel=None, vowelamt=None,
            blur=None, freeze=None, **kw):
        self.osc(o, 'add', **kw)
        p = OSC[o]
        self.set(p + 'AdSpec1', SPECTRA.index(s1)).set(p + 'AdSpec2', SPECTRA.index(s2)).set(p + 'AdMorph', morph)
        self.set(p + 'AdPartials', partials).set(p + 'AdTilt', tilt).set(p + 'AdOddEven', oe).set(p + 'AdStretch', stretch)
        self.set(p + 'AdPeakPos', peak).set(p + 'AdPeakAmt', peakamt).set(p + 'AdPeakWidth', width)
        self.set(p + 'AdComb', comb).set(p + 'AdCombShift', cshift).set(p + 'AdShimmer', shim).set(p + 'AdShimRate', srate)
        self.set(p + 'AdDetune', pdet)
        if vowel is not None: self.set(p + 'AdVowel', vowel)
        if vowelamt is not None: self.set(p + 'AdVowelAmt', vowelamt)
        if blur is not None: self.set(p + 'AdBlur', blur)
        if freeze is not None: self.set(p + 'AdFreeze', 1 if freeze else 0)
        if custom is not None: self.st['sp' + o.upper()] = spec_str(custom)
        return self

    def ph(self, o, model='String', exc='Bow', decay=5, damp=0.35, hard=0.5, pos=0.3, stretch=0.0, spread=0.5, couple=None, buzz=None, **kw):
        self.osc(o, 'phys', **kw)
        p = OSC[o]
        self.set(p + 'PmModel', MODELS.index(model)).set(p + 'PmExciter', EXCITERS.index(exc)).set(p + 'PmDecay', decay)
        self.set(p + 'PmDamp', damp).set(p + 'PmHard', hard).set(p + 'PmPos', pos).set(p + 'PmStretch', stretch).set(p + 'PmSpread', spread)
        if couple is not None: self.set(p + 'PmCouple', couple)
        if buzz is not None: self.set(p + 'PmBuzz', buzz)
        return self

    # 4.2 physical 2.0: pressure (breath / bow force), exciter noise, nonlinearity
    def ph2(self, o, press=None, noise=None, nonlin=None):
        p = OSC[o]
        if press is not None: self.set(p + 'PmPress', press)
        if noise is not None: self.set(p + 'PmNoise', noise)
        if nonlin is not None: self.set(p + 'PmNonlin', nonlin)
        return self
    def modes(self, o, name):
        assert MODAL.get(name), 'no analysed modes for ' + name
        self.st['modes' + o.upper()] = MODAL[name]
        return self
    # 4.2 Resonator (sympathetic strings tuned to the scale); Space is never used by factory presets
    def reso(self, mode='fifths', root='C', oct=2, mix=0.35, decay=4.0, bright=0.6, width=0.7):
        return self.set('rsOn', 1).set('rsMode', RS_MODE[mode]).set('rsRoot', ROOTS.index(root)).set('rsOct', oct) \
                   .set('rsMix', mix).set('rsDecay', decay).set('rsBright', bright).set('rsWidth', width)
    # 4.2 scenes: up to 8 snapshots of the 8 macros (None = empty)
    def scenes(self, snaps, on=True, pos=0.0, glide=1.0):
        parts = []
        for sn in list(snaps) + [None] * (8 - len(snaps)):
            if sn is None: parts.append('-'); continue
            vals = list(sn) + [0.0] * (8 - len(sn))
            parts.append(','.join(str(int(round(max(0, min(1, x)) * 1000))) for x in vals))
        self.st['scenes'] = ';'.join(parts)
        return self.set('scOn', 1 if on else 0).set('scPos', pos).set('scGlide', glide)
    # 4.2 motion: lanes = {macro_index_1_based: function(t in 0..1) -> 0..1}
    def motion(self, seconds, lanes, mode='loop', speed=1.0):
        frames = max(2, int(round(seconds * 50)))
        s = '%.3f' % seconds
        for m, f in sorted(lanes.items()):
            vals = [max(0.0, min(1.0, f(i / frames))) for i in range(frames)]
            s += ';%d:' % (m - 1) + ','.join(str(int(round(x * 1000))) for x in vals)
        self.st['motion'] = s
        return self.set('moMode', MO_MODE[mode]).set('moSpeed', speed)

    # 5.0 spectral oscillator: resynthesis of a library recording ('Category/Name'), optional cross partner
    def spec(self, o, src, cross=None, mode='off', amount=0.5, pos=0.0, speed=1.0, blur=0.0, shift=0.0, stretch=0.0,
             tilt=0.0, formant=0.0, sparse=0.0, keyed=True, freeze=False, **kw):
        self.osc(o, 'spec', **kw)
        p = OSC[o]
        self.st['src' + o.upper()] = 'smp:' + src + '.wav'
        if cross: self.st['xsrc' + o.upper()] = 'smp:' + cross + '.wav'
        for k, val in (('SpPos', pos), ('SpSpeed', speed), ('SpBlur', blur), ('SpShift', shift), ('SpStretch', stretch), ('SpTilt', tilt),
                       ('SpFormant', formant), ('SpSparse', sparse), ('SpCross', amount), ('SpCrossMode', SPX[mode]),
                       ('SpKeyed', 1 if keyed else 0), ('SpFreeze', 1 if freeze else 0)):
            self.set(p + k, val)
        return self
    # 5.0 Spectral FX on the output (freeze / smear / shift)
    def sfx(self, mix=0.5, blur=2.0, shift=0.0, tilt=0.0, spread=0.3, gate=0.0, freeze=False):
        return self.set('sxOn', 1).set('sxMix', mix).set('sxBlur', blur).set('sxShift', shift).set('sxTilt', tilt) \
                   .set('sxSpread', spread).set('sxGate', gate).set('sxFreeze', 1 if freeze else 0)

    def gr(self, o, source='wt', sample=None, lib=None, table=0, pos=0.3, scan=0.0, size=0.18, density=24, spray=0.1,
           prand=0, shape=0.5, spread=0.6, rev=0.0, **kw):
        self.osc(o, 'gran', **kw)
        p = OSC[o]
        self.set(p + 'GrSource', GRSRC[source]).set(p + 'GrPos', pos).set(p + 'GrScan', scan).set(p + 'GrSize', size)
        self.set(p + 'GrDensity', density).set(p + 'GrSpray', spray).set(p + 'GrPitchRand', prand).set(p + 'GrShape', shape)
        self.set(p + 'GrSpread', spread).set(p + 'GrReverse', rev).set(p + 'Table', table)
        if sample: self.st['sm' + o.upper()] = 'smp:' + sample + '.wav'
        if lib: self.st['wt' + o.upper()] = 'lib:' + lib + '.wav'
        return self

    # ---------------------------------------------------------------- 4.0 tuning / note expression
    def tune(self, scale, root='C', amount=1.0, ref=None, snap=False, glide=None):
        self.set('tnSource', 1).set('tnScale', SCALES.index(scale)).set('tnRoot', ROOTS.index(root)).set('tnAmount', amount)
        if ref is not None: self.set('tnRef', ref)
        if snap: self.set('tnSnap', 1)
        if glide is not None: self.set('tnGlide', glide)
        self.tagset.update(('microtonal',) + scale_tags(scale))
        self.scale_name = scale
        return self
    def scala(self, scl, kbm=None, root='C', amount=1.0, glide=None):
        self.set('tnSource', 2).set('tnRoot', ROOTS.index(root)).set('tnAmount', amount)
        self.st['scl'] = scala_escape(scl)
        if kbm: self.st['kbm'] = scala_escape(kbm)
        if glide is not None: self.set('tnGlide', glide)
        self.tagset.update(('microtonal', 'scala'))
        return self
    def nevib(self, depth=0.5, rate=5.5): return self.set('neVibDepth', depth).set('neVibRate', rate)

    def fm(self, o, src, amt, mode='pm'):
        p = OSC[o]
        return self.set(p + 'FmSrc', FMSRC[src]).set(p + 'FmMode', FMMODE[mode]).set(p + 'FmAmt', amt)
    def hardsync(self, o, src): return self.set(OSC[o] + 'SyncSrc', SYNCSRC[src])

    def _filt(self, n, typ, cutoff, res, drive, key, env, shape, dtype, morphto, morph, fmsrc, fmamt):
        k = 'f%d' % n
        self.set(k + 'Type', FILTERS.index(typ)).set(k + 'Cutoff', cutoff).set(k + 'Res', res).set(k + 'Drive', drive)
        self.set(k + 'Key', key).set(k + 'Env', env).set(k + 'Shape', shape)
        if dtype is not None: self.set(k + 'DriveType', DRIVETYPE[dtype])
        if morphto is not None: self.set(k + 'Type2', FILTERS.index(morphto)).set(k + 'Morph', morph)
        if fmsrc is not None: self.set(k + 'FmSrc', FILTFM[fmsrc]).set(k + 'FmAmt', fmamt)
        return self
    def f1(self, typ, cutoff, res=0.15, drive=0.1, key=0.3, env=0.0, shape=0.5, dtype=None, morphto=None, morph=0.0, fmsrc=None, fmamt=0.0):
        return self._filt(1, typ, cutoff, res, drive, key, env, shape, dtype, morphto, morph, fmsrc, fmamt)
    def f2(self, typ, cutoff=3000, res=0.15, drive=0.0, key=0.3, env=0.0, shape=0.5, dtype=None, morphto=None, morph=0.0, fmsrc=None, fmamt=0.0):
        return self._filt(2, typ, cutoff, res, drive, key, env, shape, dtype, morphto, morph, fmsrc, fmamt)

    def route(self, r, balance=0.5): return self.set('fRouting', ROUTING[r]).set('fBalance', balance)
    def fenv(self, a, d, s, r): return self.set('feA', a).set('feD', d).set('feS', s).set('feR', r)
    def aenv(self, a, d, s, r): return self.set('aeA', a).set('aeD', d).set('aeS', s).set('aeR', r)
    def menv(self, a, d, s, r): return self.set('meA', a).set('meD', d).set('meS', s).set('meR', r)
    def curve(self, a=None, f=None, m=None):
        """attack curves: -1 slow swell, 0 soft S (default), +1 punchy"""
        if a is not None: self.set('aEnvCurve', a)
        if f is not None: self.set('fEnvCurve', f)
        if m is not None: self.set('mEnvCurve', m)
        return self

    def lfo(self, i, rate, wave='sine', mode='global', fade=0.0, sync=False, div=None, phase=None, smooth=None, shape=None):
        k = 'lfo%d' % i
        self.set(k + 'Rate', rate).set(k + 'Wave', LFO_WAVE[wave]).set(k + 'Mode', LFO_MODE[mode]).set(k + 'Fade', fade)
        if sync: self.set(k + 'Sync', 1)
        if div is not None: self.set(k + 'Div', DIVS.index(div))
        if phase is not None: self.set(k + 'Phase', phase)
        if smooth is not None: self.set(k + 'Smooth', smooth)
        if shape is not None:
            assert len(shape) == 64
            self.st['lfo%d' % i] = ','.join(str(int(round(max(-1, min(1, x)) * 1000))) for x in shape)
            self.set(k + 'Wave', 7)
        return self

    def mseg(self, i, points, loop=(1, 2), mode='env', scale=1.0, sync=False):
        """points: [(time_from_previous, level, curve), ...]; the first point's time is ignored."""
        s = '%d,%d' % loop
        for k, (t, l, c) in enumerate(points): s += ';%.4g,%.4g,%.3g' % (0 if k == 0 else t, l, c)
        self.st['mseg%d' % i] = s
        return self.set('mseg%dMode' % i, MSEG_MODE[mode]).set('mseg%dScale' % i, scale).set('mseg%dSync' % i, 1 if sync else 0)

    def mod(self, src, dst, amt, via=None, curve=None):
        assert self.slots < 32, self.name
        self.slots += 1
        k = 'mod%d' % self.slots
        self.set(k + 'Src', SRC.index(src)).set(k + 'Dst', DST.index(dst)).set(k + 'Amt', max(-1.0, min(1.0, amt)))
        if via: self.set(k + 'Via', SRC.index(via))
        if curve: self.set(k + 'Curve', CURVES[curve])
        return self

    def macro(self, i, name, value=0.0):
        while len(self.macros) < i: self.macros.append('')
        self.macros[i - 1] = name
        if value: self.set('macro%d' % i, value)
        self.gave_macros = True
        return self

    def sub(self, level, wave='sine', oct=-1): return self.set('subLevel', level).set('subWave', SUBW[wave]).set('subOct', oct)
    def noise(self, level, color=0.3): return self.set('noiseLevel', level).set('noiseColor', color)
    def ring(self, level, src='ab'): return self.set('ringLevel', level).set('ringSource', RING[src])

    # FX
    def chorus(self, mode='chorus', mix=0.3, rate=0.3, depth=0.5, fb=0.0):
        return self.set('chOn', 1).set('chMode', CHORUS[mode]).set('chMix', mix).set('chRate', rate).set('chDepth', depth).set('chFb', fb)
    def nochorus(self): return self.set('chOn', 0)
    def phaser(self, mix=0.4, rate=0.1, depth=0.7, fb=0.5):
        return self.set('phOn', 1).set('phMix', mix).set('phRate', rate).set('phDepth', depth).set('phFb', fb)
    def drive(self, x, tone=1.0): return self.set('drvOn', 1).set('drvAmt', x).set('drvTone', tone)
    def shifter(self, shift, mix=0.5, fb=0.3, stereo=0.5): return self.set('fsOn', 1).set('fsShift', shift).set('fsMix', mix).set('fsFb', fb).set('fsStereo', stereo)
    def tape(self, drive=0.2, wow=0.3, flutter=0.15, tone=0.7, mix=1.0):
        return self.set('tpOn', 1).set('tpDrive', drive).set('tpWow', wow).set('tpFlutter', flutter).set('tpTone', tone).set('tpMix', mix)
    def crush(self, bits=8, down=4, mix=0.4): return self.set('crOn', 1).set('crBits', bits).set('crDown', down).set('crMix', mix)
    def width(self, w=1.4, bassmono=120): return self.set('wdOn', 1).set('wdWidth', w).set('wdBassMono', bassmono)
    def eq(self, lowg=0, lowf=120, midg=0, midf=1000, q=0.8, highg=0, highf=6000):
        return self.set('eqOn', 1).set('eqLowG', lowg).set('eqLowF', lowf).set('eqMidG', midg).set('eqMidF', midf).set('eqMidQ', q).set('eqHighG', highg).set('eqHighF', highf)
    def fxorder(self, *units):
        order = [FXU.index(u) for u in units]
        order += [i for i in FX_DEFAULT if i not in order]
        self.st['fxOrder'] = ','.join(str(i) for i in order)
        return self

    # performance
    def arp(self, mode='up', oct=1, rate='1/16', gate=0.6, swing=0.0, hold=False):
        return self.set('arpOn', 1).set('arpMode', ARP[mode]).set('arpOct', oct).set('arpRate', DIVS.index(rate)).set('arpGate', gate).set('arpSwing', swing).set('arpHold', 1 if hold else 0)
    def seq(self, mode='notes', rate='1/16', steps=16, gate=0.5, swing=0.0, glide=0.0, values=None, gates=None):
        self.set('seqMode', SEQ[mode]).set('seqRate', DIVS.index(rate)).set('seqSteps', steps).set('seqGate', gate).set('seqSwing', swing).set('seqGlide', glide)
        for k, val in enumerate(values or []):
            if mode == 'notes' and isinstance(val, int): val = val / 24.0
            self.set('seqV%d' % (k + 1), val)
        for k, g in enumerate(gates or []): self.set('seqG%d' % (k + 1), 1 if g else 0)
        return self
    def voice(self, mode='poly', glide=0.001, poly=None):
        self.set('polyMode', POLY[mode]).set('glide', glide)
        if poly is not None: self.set('polyphony', poly)
        return self
    def mpe(self, pressCut=None, pressAmp=None, slideCut=None, slideTimbre=None, velAmp=None, mwCut=None, bend=None):
        for k, val in [('pressCut', pressCut), ('pressAmp', pressAmp), ('slideCut', slideCut),
                       ('slideTimbre', slideTimbre), ('velAmp', velAmp), ('mwCut', mwCut), ('bendRange', bend)]:
            if val is not None: self.set(k, val)
        return self
    def misc(self, drift=None, spread=None, quality=None, rndg=None, rndv=None):
        if drift is not None: self.set('drift', drift)
        if spread is not None: self.set('spread', spread)
        if quality is not None: self.set('quality', QUALITY[quality])
        if rndg is not None: self.set('rndGRate', rndg)
        if rndv is not None: self.set('rndVRate', rndv)
        return self

    # ---------------------------------------------------------------- finishing
    def finalize(self):
        if not self.gave_macros: self.default_macros()
        if self.macros: self.st['macroNames'] = '|'.join(self.macros)
        t = set(self.tagset)
        t.add(self.cat.split(' ')[0].lower())
        for i in range(3):
            if self.on[i] and self.v.get(OSC['abc'[i]] + 'Level', 0.7) > 0: t.add(ENGINE_TAG[self.types[i]])
        if any(k.endswith('FmAmt') and val > 0 for k, val in self.v.items()): t.add('fm')
        if any(k.endswith('SyncSrc') and val > 0 for k, val in self.v.items()) or any(k.endswith('AnSync') and val > 0 for k, val in self.v.items()): t.add('sync')
        if any(('mseg%d' % i) in self.st for i in range(1, 5)): t.add('mseg')
        if self.v.get('rsOn', 0) > 0: t.add('resonator')
        if 'scenes' in self.st: t.add('scenes')
        if 'motion' in self.st: t.add('motion')
        if self.v.get('sxOn', 0) > 0: t.add('spectral-fx')
        if any(('xsrc' + o) in self.st for o in 'ABC'): t.add('cross-synthesis')
        if self.v.get('arpOn', 0) > 0: t.add('arp')
        if self.v.get('seqMode', 0) > 0: t.add('sequencer')
        if self.v.get('aeA', 1.5) >= 3: t.add('slow')
        if self.v.get('polyMode', 0) > 0: t.add('mono')
        self.tag_str = ','.join(sorted(t))

    def default_macros(self):
        """Give every ported preset four useful macros without changing its sound at rest."""
        self.macro(1, 'Tone').mod('m1', 'cut', 0.4)
        # Motion: scales the existing LFO routes (via macro 2, resting at 0.5 with doubled depth)
        lfo_slots = [k for k in range(1, self.slots + 1) if SRC[int(self.v.get('mod%dSrc' % k, 0))].startswith('lfo')]
        if lfo_slots and all(abs(self.v['mod%dAmt' % k]) <= 0.5 for k in lfo_slots):
            for k in lfo_slots:
                self.set('mod%dVia' % k, SRC.index('m2')).set('mod%dAmt' % k, self.v['mod%dAmt' % k] * 2)
            self.macro(2, 'Motion', 0.5)
        else:
            self.macro(2, 'Motion').mod('m2', 'timbre', 0.35)
        if self.v.get('chOn', 1) > 0: self.macro(3, 'Chorus').mod('m3', 'fxchorus', 0.5)
        elif self.v.get('phOn', 0) > 0: self.macro(3, 'Phaser').mod('m3', 'fxphaser', 0.5)
        else: self.macro(3, 'Width').width(1.0).mod('m3', 'fxwidth', 0.8)
        self.macro(4, 'Drift').mod('m4', 'detune', 0.3)
        self.gave_macros = True


class PatchV2(Patch):
    """Driftfield 2 preset code runs on this; v2 meanings are translated to v3."""
    v2 = True

    def __init__(self, name, cat):
        super().__init__(name, cat)
        self.set('chOn', 1)
        if cat in ('Drones', 'Atmospheres', 'Choirs & Voices', 'Textures'): self.tagset.update(('roach', 'corbacho'))

    def mod(self, src, dst, amt, via=None, curve=None):
        src = {'rand': 'rand'}.get(src, src)
        if dst == 'drive': dst = 'f1drive'
        m = re.match(r'^([abc])t([123])$', dst)
        if m:
            o, k = m.group(1), int(m.group(2))
            t = self.types['abc'.index(o)]
            if t == 0:      # v2 wavetable: T1 pos*0.5, T2 detune*50, T3 pos fine
                if k == 1: amt *= 0.5
                elif k == 2: dst = o + 'detune'
                else: dst = o + 't4'
            elif t == 1:    # v2 analog: T1 pw, T2 sync, T3 detune
                if k == 3: dst = o + 'detune'
        return super().mod(src, dst, amt, via, curve)

    def chorus(self, mode='chorus', mix=0.3, rate=0.3, depth=0.5, fb=0.0):
        return super().chorus(mode, mix, rate, depth, fb)
    def drive(self, x, tone=1.0): return super().drive(x, 1.0)


LY_MODES = {'off': 0, 'layer': 1, 'split': 2, 'velocity': 3, 'pressure': 4, 'morph': 5}

class LayeredPatch(Patch):
    """4.1: two finished presets as layer 1 and layer 2 (layer 2 = 'b_' params / states).
    Level: preset_levels3.json holds the suggested layer-1 master; layer 2 keeps its balance to it."""
    def __init__(self, name, cat, a, b, mode='layer', split=60, width=6.0, morph=0.5, balance=0.0, tags=()):
        super().__init__(name, cat)
        self.src = (a, b); self.mode = mode; self.split = split; self.width = width; self.morph = morph
        self.balance = balance; self.extra_tags = tags
    def finalize(self): pass
    def resolve(self, byname, levels):
        a, b = byname[self.src[0]], byname[self.src[1]]
        self.v = dict(a.v)
        for k, val in b.v.items(): self.v['b_' + k] = val
        for k in ('master',):
            self.v.setdefault(k, -6.0); self.v.setdefault('b_' + k, -6.0)
        self.v['b_master'] = b.v.get('master', -6.0) + self.balance
        self.st = dict(a.st)
        for k, val in b.st.items(): self.st['b_' + k] = val
        self.v['lyMode'] = LY_MODES[self.mode]; self.v['lySplit'] = self.split; self.v['lyWidth'] = self.width; self.v['lyMorph'] = self.morph
        # two normalised sounds summed: start 3 dB down, then the level check's correction
        if self.name in levels:
            shift = float(levels[self.name]) - self.v.get('master', -6.0)
            self.v['master'] = float(levels[self.name]); self.v['b_master'] += shift
        else:
            self.v['master'] = self.v.get('master', -6.0) - 3.0; self.v['b_master'] -= 3.0
        t = set(a.tag_str.split(',')) | set(b.tag_str.split(','))
        t -= {a.cat.split(' ')[0].lower(), b.cat.split(' ')[0].lower()}
        t |= {'layered', self.mode} | set(self.extra_tags)
        self.tag_str = ','.join(sorted(x for x in t if x))
        self.layer_names = (a.name, b.name)
        self.st['layerNames'] = a.name + '|' + b.name     # read by the editor (not a layer state)

def L(name, a, b, mode='layer', **kw):
    p = LayeredPatch(name, 'Layered', a, b, mode, **kw)
    PRESETS.append(p)
    return p

PRESETS = []
_FACTORY = [Patch]
def P(name, cat, **_):
    p = _FACTORY[0](name, cat)
    PRESETS.append(p)
    return p


# ------------------------------------------------------------------------------------ custom spectra (shared)
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


def load_v2_bank():
    src = open(os.path.join(HERE, 'gen_presets.py')).read()
    a = src.index('OVERTONE = spec_formant')
    b = src.index("P('Init', 'Init')")
    _FACTORY[0] = PatchV2
    exec(src[a:b], globals())
    _FACTORY[0] = Patch


def load_v3_bank():
    _FACTORY[0] = Patch
    exec(open(os.path.join(HERE, 'bank_v3.py')).read(), globals())


def load_v40_bank():
    import glob as _g
    _FACTORY[0] = Patch
    for f in sorted(_g.glob(os.path.join(HERE, 'bank_v40_*.py'))) + sorted(_g.glob(os.path.join(HERE, 'bank_v41_*.py'))) + sorted(_g.glob(os.path.join(HERE, 'bank_v42_*.py'))) + sorted(_g.glob(os.path.join(HERE, 'bank_v50_*.py'))) + sorted(_g.glob(os.path.join(HERE, 'bank_v51_*.py'))):
        exec(open(f).read(), globals())


def load_v31_bank():
    import glob as _g
    _FACTORY[0] = Patch
    for f in sorted(_g.glob(os.path.join(HERE, 'bank_v31_*.py'))):
        exec(open(f).read(), globals())


LIBRARY = os.path.join(HERE, '..', '..', 'library')
def check_files():
    bad = []
    for p in PRESETS:
        for k, val in p.st.items():
            for pre, d in (('lib:', 'Wavetables'), ('smp:', 'Samples')):
                if val.startswith(pre) and not os.path.exists(os.path.join(LIBRARY, d, val[len(pre):])): bad.append((p.name, val))
    for b in bad: print('MISSING FILE', b)
    assert not bad


# ---------------------------------------------------------------------------------------------- emit
CATS = ['Init', 'Spectral', 'Performance', 'Winds & Reeds', 'Layered', 'Just Intonation', 'Analog', 'Sweeps', 'Drones', 'Dark Ambient', 'Deep Space', 'Atmospheres', 'Ritual & Tribal', 'Pads', 'Textures', 'Soundscapes', 'FX & Details', 'Physical Models', 'Choirs & Voices', 'Bells & Resonators', 'Keys & Organs',
        'Basses', 'Leads', 'Sequence & Plucks', 'Berlin Sequences']

def cstr(s): return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

# 5.1.1: the engine's modal decay was cut short by fastExp2's error (fixed in the engine). Presets keep the ring
# they had in 5.1 (at 48 kHz, Normal): Decay becomes the time the old engine actually rang, Damping scales with it.
MODAL_MODELS = {0, 1, 2, 3, 4, 5, 6, 12, 13}
TYPES_DEFAULT = [1, 1, 0]     # A analog, B analog, C wavetable (Patch defaults)
PMDAMP_DEFAULT = 0.35
def _fast_exp2_f32(x):
    import numpy as np
    f32 = np.float32
    x = f32(max(-30.0, min(30.0, float(x))))
    fl = f32(math.floor(float(x))); f = f32(x - fl)
    p = f32(1) + f * (f32(0.6931472) + f * (f32(0.2402265) + f * (f32(0.0555041) + f * (f32(0.0096181) + f * f32(0.0013334)))))
    return float(f32(p * f32(2.0 ** float(fl))))
def _old_ring(T, fs=48000.0):
    import numpy as np
    r = _fast_exp2_f32(np.float32(-9.9658) / (np.float32(T) * np.float32(fs)))
    return -9.9658 / (fs * math.log2(r)) if r < 1.0 else T
def keep_old_modal_ring(p):
    for pre in ('', 'b_'):
        for i, o in enumerate('abc'):
            k = pre + OSC[o]
            if p.v.get(k + 'Type', TYPES_DEFAULT[i]) != TYPES['phys']: continue
            if p.v.get(k + 'On', 1.0 if o != 'c' else 0.0) <= 0: continue
            model = int(p.v.get(k + 'PmModel', 0))
            if model not in MODAL_MODELS: continue
            d = p.v.get(k + 'PmDecay', 5.0)
            ref = 5.0
            if model == 13:   # analysed: the loudest mode's own decay scales with the knob
                ms = p.st.get(pre + 'modes' + o.upper(), '')
                best = None
                for part in ms.split(';'):
                    try: rt, am, dc = (float(x) for x in part.split(','))
                    except ValueError: continue
                    if best is None or am > best[0]: best = (am, dc)
                if best: ref = best[1]
            T = ref * d / 5.0
            d2 = max(0.05, min(30.0, _old_ring(T) * 5.0 / ref))
            p.v[k + 'PmDecay'] = round(d2, 4)
            p.v[k + 'PmDamp'] = round(max(0.0, min(1.0, p.v.get(k + 'PmDamp', PMDAMP_DEFAULT) * d2 / d)), 4)

# ---------------------------------------------------------------- 5.1.2: pure, clean sound everywhere
GRIT_RE = re.compile(r'\b(acid|distort\w*|overdriv\w*|fuzz\w*|saturat\w*|crush\w*|lo-?fi|broken|grit\w*|burn|burns|burnt|bitcrush\w*|dirt\w*|grunge\w*)\b')
RENAME_WORDS = [('Burnished', 'Polished'), ('Burning', 'Glowing')]
DARKEN = {}
STRUCT_FILTERS = ('Formant', 'Vowel', 'Comb', 'Cmb', 'Flg', 'Phs', 'Phaser')
def is_grit(p):
    return bool(GRIT_RE.search((p.name + ' ' + ' '.join(sorted(getattr(p, 'tagset', set())))).lower()))
def clean_pass(p):
    v = p.v
    for pre in ('', 'b_') if any(k.startswith('b_') for k in v) else ('',):
        for f in ('f1', 'f2'):
            k = pre + f
            if k + 'Drive' in v: v[k + 'Drive'] = 0.0
            if int(v.get(k + 'DriveType', 0)) in (2, 3): v[k + 'DriveType'] = 0.0          # diode / fold -> soft
            ftype = FILTERS[int(v.get(k + 'Type', 1 if f == 'f1' else 0))]
            res = v.get(k + 'Res', 0.15)                                                    # 5.1 default was 0.15
            if any(t in ftype for t in STRUCT_FILTERS): v[k + 'Res'] = round(min(res, 0.6), 4)   # the resonance IS the vowel / comb / notch
            else: v[k + 'Res'] = round(min(res, 0.05), 4)
            if v.get(k + 'FmAmt', 0) > 0.05: v[k + 'FmAmt'] = 0.05
        if v.get(pre + 'drvOn', 0) > 0: v[pre + 'drvOn'] = 0.0
        if v.get(pre + 'crOn', 0) > 0: v[pre + 'crOn'] = 0.0
        if pre + 'tpOn' in v or v.get(pre + 'tpOn', 0) > 0: v[pre + 'tpDrive'] = 0.0
        if v.get(pre + 'ringLevel', 0) > 0.1: v[pre + 'ringLevel'] = 0.1
        oscs_carry = any(v.get(pre + o + 'On', 1.0 if o != 'oc' else 0.0) > 0 and v.get(pre + o + 'Level', 0.7) > 0.2 for o in ('oa', 'ob', 'oc'))
        if oscs_carry and v.get(pre + 'noiseLevel', 0) > 0.1: v[pre + 'noiseLevel'] = 0.1      # noise-made sounds keep their noise
        for o in ('oa', 'ob', 'oc'):
            k = pre + o
            if v.get(k + 'FmAmt', 0) > 0.15: v[k + 'FmAmt'] = 0.15                         # no harsh sidebands
            if int(v.get(k + 'Warp', 0)) in (WARP.index('sync'), WARP.index('quantize'), WARP.index('flip')):
                v[k + 'Warp'] = 0.0; v[k + 'WarpAmt'] = 0.0
            if v.get(k + 'AnSync', 0) > 0.15: v[k + 'AnSync'] = 0.15
        # fast sample-and-hold on a cutoff steps the filter audibly: a quarter of the depth
        for i in range(1, 33):
            s_ = int(v.get(pre + 'mod%dSrc' % i, 0)); d_ = int(v.get(pre + 'mod%dDst' % i, 0))
            if not s_ or DST[d_] not in ('f1cut', 'f2cut', 'cut'): continue
            src = SRC[s_]
            if src.startswith('lfo'):
                L = pre + 'lfo' + src[3:]
                if int(v.get(L + 'Wave', 0)) == LFO_WAVE['sh'] and v.get(L + 'Rate', 1.0) > 1.5 and v.get(L + 'Sync', 0) < 0.5:
                    v[pre + 'mod%dAmt' % i] = round(v.get(pre + 'mod%dAmt' % i, 0) * 0.25, 4)

def compress_motion(s):
    """5.1: store each motion lane as keyframes every 10 / 5 / 2 frames ('~step:' form, linear in between)
    when that stays within 0.004 of every frame; otherwise the lane is kept frame by frame."""
    parts = s.split(';')
    out = [parts[0]]
    for lane in parts[1:]:
        if ':' not in lane or lane.startswith('rec'): out.append(lane); continue
        m, vals = lane.split(':', 1)
        if vals.startswith('~'): out.append(lane); continue
        v = [int(x) for x in vals.split(',') if x != '']
        n = len(v); best = None
        for step in (10, 5, 2):
            keys = list(range(0, n, step))
            if keys[-1] != n - 1: keys.append(n - 1)
            ok = True
            for a, b in zip(keys, keys[1:]):
                for i in range(a + 1, b):
                    y = v[a] + (v[b] - v[a]) * (i - a) / (b - a)
                    if abs(y - v[i]) > 4: ok = False; break
                if not ok: break
            if ok: best = step; break
        if best is None: out.append(lane); continue
        keys = [v[i] for i in range(0, n, best)]
        if (n - 1) % best: keys.append(v[n - 1])
        out.append('%s:~%d:%s' % (m, best, ','.join(str(x) for x in keys)))
    return ';'.join(out)

def emit(path):
    lv = os.path.join(HERE, 'preset_levels3.json')
    levels = json.load(open(lv)) if os.path.exists(lv) else {}
    names = set()
    # 5.1.2: grit presets go (and layered presets built on them); burn words are renamed; everything else is cleaned
    gone = set(p.name for p in PRESETS if not isinstance(p, LayeredPatch) and is_grit(p))
    PRESETS[:] = [p for p in PRESETS if p.name not in gone and not (isinstance(p, LayeredPatch) and (is_grit(p) or set(p.src) & gone))]
    taken = set(p.name for p in PRESETS)
    renamed = {}
    for p in PRESETS:
        nn = p.name
        for a_, b_ in RENAME_WORDS: nn = nn.replace(a_, b_)
        if nn != p.name:
            k = 2
            base = nn
            while nn in taken: nn = '%s %d' % (base, k); k += 1
            taken.discard(p.name); taken.add(nn); renamed[p.name] = nn; p.name = nn
    for p in PRESETS:
        if isinstance(p, LayeredPatch):
            p.src = tuple(renamed.get(x, x) for x in p.src)
    print('5.1.2: removed %d grit presets, renamed %d' % (len(gone), len(renamed)), file=sys.stderr)
    dk_ = os.path.join(HERE, 'darken512.json')
    global DARKEN
    DARKEN = json.load(open(dk_)) if os.path.exists(dk_) else {}
    gf = os.path.join(HERE, 'gainfix511.json')
    gainfix = json.load(open(gf)) if os.path.exists(gf) else {}
    for p in PRESETS:
        if isinstance(p, LayeredPatch): continue
        p.finalize()
        if p.name in levels: p.v['master'] = float(levels[p.name])
        if p.name in gainfix:
            # 5.1.1: presets that needed more than the +12 dB master: raise the oscillators (and sub / noise) instead
            g = 10 ** (float(gainfix[p.name]) / 20.0)
            for o in 'abc':
                k = OSC[o] + 'Level'
                if p.v.get(OSC[o] + 'On', 1.0 if o != 'c' else 0.0) > 0: p.v[k] = round(min(1.0, p.v.get(k, 0.7) * g), 4)
            for k in ('subLevel', 'noiseLevel'):
                if p.v.get(k, 0) > 0: p.v[k] = round(min(1.0, p.v[k] * g), 4)
        keep_old_modal_ring(p)
        clean_pass(p)
        if p.name in DARKEN:   # 5.1.2: hand-made presets over the stricter brightness limits (can't be re-rolled)
            k = float(DARKEN[p.name])
            if int(p.v.get('f1Type', 1)) != 0: p.v['f1Cutoff'] = round(max(200.0, p.v.get('f1Cutoff', 20000.0) * k), 1)
            p.v['eqOn'] = 1.0; p.v['eqHighF'] = 4000.0; p.v['eqHighG'] = round(min(p.v.get('eqHighG', 0.0), 0.0) - 6.0, 1)
    byname = {p.name: p for p in PRESETS if not isinstance(p, LayeredPatch)}
    for p in PRESETS:
        if isinstance(p, LayeredPatch): p.resolve(byname, levels)
    for p in PRESETS:
        assert p.name not in names, 'duplicate ' + p.name
        names.add(p.name)
        assert p.cat in CATS, p.cat
        assert p.v.get('spOn', 0) == 0 and p.v.get('dlOn', 0) == 0, 'no reverb / delay in factory presets: ' + p.name
    out = ['// Generated by tools/gen_presets3.py — do not edit by hand.', '']
    import struct as _st
    syms, symix, data, span = [], {}, [], []
    for p in PRESETS:
        d0 = len(data) // 2
        for k, val in p.v.items():
            if k not in symix: symix[k] = len(syms); syms.append(k)
            data.append(symix[k]); data.append(_st.unpack('<I', _st.pack('<f', float(val)))[0])
        span.append((d0, len(data) // 2 - d0))
    out.append('static const char* const kPresetSyms[] = {')
    for i in range(0, len(syms), 8): out.append('    ' + ' '.join(cstr(x) + ',' for x in syms[i:i + 8]))
    out.append('};')
    out.append('static constexpr uint32_t kPresetSymCount = %d;' % len(syms))
    out.append('static const uint32_t kPresetData[] = {')
    for i in range(0, len(data), 16): out.append('    ' + ','.join('%du' % x for x in data[i:i + 16]) + ',')
    out.append('};')
    for i, p in enumerate(PRESETS):
        out.append('static const PresetState kST%d[] = {' % i)
        for k, val in p.st.items():
            if k == 'motion' or k == 'b_motion': val = compress_motion(val)
            out.append('    { "%s", %s },' % (k, cstr(val)))
        out.append('    { nullptr, nullptr } };')
    out.append('')
    out.append('static const FactoryPreset kFactoryPresets[] = {')
    order = sorted(range(len(PRESETS)), key=lambda i: (CATS.index(PRESETS[i].cat), PRESETS[i].cat == 'Init' and PRESETS[i].name != 'Init', PRESETS[i].name))
    for i in order:
        p = PRESETS[i]
        out.append('    { %s, %s, %s, %du, %du, kST%d },' % (cstr(p.name), cstr(p.cat), cstr(p.tag_str), span[i][0], span[i][1], i))
    out.append('};')
    out.append('static constexpr uint32_t kFactoryPresetCount = sizeof(kFactoryPresets) / sizeof(kFactoryPresets[0]);')
    # 5.1.1: measured CPU per preset (% of one core, 4-note chord; HQ when measured) for the browser badge
    cp = None
    for nm in ('preset_cpu_hq.json', 'cpu51.json'):
        f = os.path.join(HERE, nm)
        if os.path.exists(f): cp = json.load(open(f)); break
    cp = cp or {}
    vals = [max(0, min(255, int(round(float(cp.get(PRESETS[i].name, 0.0)) * 100)))) for i in order]
    out.append('static const uint8_t kPresetCpu[] = {')
    for i in range(0, len(vals), 32): out.append('    ' + ','.join(str(x) for x in vals[i:i + 32]) + ',')
    out.append('};')
    open(path, 'w').write('\n'.join(out) + '\n')
    print('%d presets' % len(PRESETS))
    from collections import Counter
    c = Counter(p.cat for p in PRESETS)
    for k in CATS: print('  %-20s %d' % (k, c.get(k, 0)))


if __name__ == '__main__':
    load_v2_bank()
    n2 = len(PRESETS)
    load_v3_bank()
    n3 = len(PRESETS)
    load_v31_bank()
    n31 = len(PRESETS)
    load_v40_bank()
    print('ported %d, v3 %d, v3.1 %d, v4.0 %d' % (n2, n3 - n2, n31 - n3, len(PRESETS) - n31))
    check_files()
    emit(sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, '..', 'FactoryPresets.inc'))
