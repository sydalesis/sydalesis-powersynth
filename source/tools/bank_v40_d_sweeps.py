# Driftfield 4.0 bank, part D: SWEEPS — presets whose point is a long, evolving sweep.
# Filter sweeps (LFO, tempo-synced, MSEG one-shots), resonant and Flanges-type sweeps, wavetable scans,
# vowel / formant sweeps, additive spectral sweeps, sync and FM index sweeps, granular and physical
# position sweeps, risers and downlifters. Macro 1 is always the sweep depth or position, macro 2 the
# sweep speed where there is one. MPE: pressure deepens the sweep target, slide offsets it.
SW = 'Sweeps'

def swX(p, tgt, press_amt=0.3, slide_amt=0.35, vib=0.0):
    return X(p, vib=vib, pc=0.2, sc=0.15, press=[(tgt, press_amt)], slide=[(tgt, slide_amt)])

def sweep_lfo(p, dst, amt, rate=None, div=None, wave='tri', second=None):
    """LFO 1 drives the sweep (synced when div is given); optional slower LFO 2 on a second target"""
    if div: p.lfo(1, 0.1, wave, sync=True, div=div)
    else: p.lfo(1, rate, wave)
    p.mod('lfo1', dst, amt)
    if second: p.lfo(2, second[2], 'drift').mod('lfo2', second[0], second[1])
    return p

def sweep_mseg(p, dst, amt, rise=8.0, fall=None, curve=0.5, loop=False):
    pts = [(0, 0.0, 0), (rise, 1.0, curve)]
    if fall: pts.append((fall, 0.0, -curve))
    p.mseg(1, pts, loop=(0, len(pts) - 1) if loop else (1, 1), mode='loop' if loop else 'env').mod('mseg1', dst, amt)
    return p

def macros_sweep(p, dst, depth_amt=0.5, rate_dst='lfo1rate', tone='f1cut', extra=('Resonance', 'f1res', 0.35)):
    p.macro(1, 'Sweep').mod('m1', dst, depth_amt).macro(2, 'Speed').mod('m2', rate_dst, 0.5) \
     .macro(3, extra[0]).mod('m3', extra[1], extra[2]).macro(4, 'Tone').mod('m4', tone, 0.4)
    return p

# ================================================================================================ A. FILTER SWEEP PADS
def fpad(name, filt, cut, res, depth, div=None, rate=None, wa='saw', wb='saw', voices=3, det=10, fine=7, chorus=('ensemble', 0.25, 0.08, 0.45), tags=(), env=None, phaser=False, wave='tri'):
    p = P(name, SW).an('a', wa, voices=voices, detune=det, slop=0.4, drift=0.5, level=0.45) \
        .an('b', wb, fine=fine, voices=2, detune=det, slop=0.4, drift=0.5, level=0.35).off('c') \
        .f1(filt, cut, res=res, key=0.25).aenv(*(env or (2, 3, 1, 5))).chorus(*chorus)
    sweep_lfo(p, 'f1cut', depth, rate=rate, div=div, wave=wave, second=('f1res', 0.1, 0.03))
    if phaser: p.phaser(0.35, 0.03, 0.8, 0.6)
    macros_sweep(p, 'f1cut')
    p.tags('sweep', 'filter-sweep', 'pad', *tags)
    swX(p, 'f1cut')
    return p

fpad('Eight Bar Ladder Sweep', 'Ladder LP24', 700, 0.35, 0.6, div='8 bars', tags=('synced', 'slow'))
fpad('Four Bar OB Sweep', 'OB-Xa LP24', 900, 0.3, 0.55, div='4 bars', tags=('synced', 'oberheim'))
fpad('Two Bar SEM Breath', 'SEM LP12', 1100, 0.25, 0.5, div='2 bars', tags=('synced', 'sem'))
fpad('Slow Tide Lowpass', 'SVF LP24', 800, 0.2, 0.65, rate=0.025, tags=('tidal', 'slow'), wave='sine')
fpad('Glacial Ladder Drift', 'Ladder LP24', 1000, 0.3, 0.6, rate=0.012, tags=('glacial', 'roach'), env=(6, 4, 1, 14))
fpad('Pulse Sweep Strings', 'OB-Xa LP12', 1500, 0.2, 0.45, div='2 bars', wa='pulse', wb='saw', tags=('pwm', 'strings'))
fpad('Highpass Rising Air', 'SVF HP12', 200, 0.3, 0.6, div='4 bars', tags=('highpass', 'airy'))
fpad('Bandpass Searchlight', 'SVF BP12', 900, 0.5, 0.7, rate=0.05, tags=('bandpass', 'searching'))
fpad('Xpander Notch Wander', 'Xpander N2', 1200, 0.3, 0.6, rate=0.04, tags=('notch', 'xpander'))
fpad('Phaser Tri Sweep', 'Ladder LP12', 2200, 0.15, 0.35, rate=0.06, tags=('phaser', 'seventies'), phaser=True)
fpad('Sawtooth Ramp Sweep', 'Ladder LP24', 600, 0.4, 0.6, div='1 bar', wave='sawup', tags=('ramp', 'synced', 'rhythmic'))
fpad('Square Gate Sweep', 'SEM LP12', 700, 0.35, 0.55, div='1/2', wave='square', tags=('stepped', 'synced', 'rhythmic'))
fpad('Drift Random Sweep', 'OB-Xa LP24', 900, 0.3, 0.55, rate=0.08, wave='drift', tags=('random', 'organic'))
fpad('Diode Dusk Sweep', 'Diode LP24', 500, 0.45, 0.6, div='4 bars', tags=('diode', 'dark'))
fpad('MS-20 Twin Peak Sweep', 'MS-20 LP12', 800, 0.6, 0.55, div='2 bars', tags=('ms20', 'resonant'))
fpad('Morning Bright Sweep', 'SVF LP12', 2500, 0.1, 0.4, rate=0.03, tags=('bright', 'gentle'), chorus=('solina', 0.3, 0.1, 0.5))

# ================================================================================================ B. MSEG ONE-SHOT SWELLS
def swell(name, filt, cut, res, rise, depth=0.9, fall=None, engine='wt', lib='Ambient/Ancient Light', tags=(), extra_dst=None):
    p = P(name, SW)
    if engine == 'wt': p.wt('a', lib, pos=0.3, blend='spectral', voices=4, detune=8, level=0.5)
    else: p.an('a', 'saw', voices=3, detune=12, slop=0.4, level=0.45)
    p.an('b', 'saw', oct=-1, voices=2, detune=8, slop=0.4, level=0.3).off('c') \
     .f1(filt, cut, res=res, key=0.2).aenv(rise * 0.4, 3, 1, 6).chorus('ensemble', 0.22, 0.06, 0.45)
    sweep_mseg(p, 'f1cut', depth, rise, fall)
    if extra_dst: p.mod('mseg1', extra_dst[0], extra_dst[1])
    macros_sweep(p, 'f1cut', rate_dst='mseg1rate')
    p.tags('sweep', 'swell', 'one-shot-sweep', *tags)
    swX(p, 'f1cut')
    return p

swell('Ten Second Dawn', 'Ladder LP24', 250, 0.25, 10, tags=('dawn', 'cinematic'))
swell('Twenty Second Ascent', 'OB-Xa LP24', 200, 0.3, 20, tags=('very-slow', 'ascent'))
swell('Open and Close', 'SEM LP12', 300, 0.3, 6, fall=8, tags=('open-close',))
swell('Spectral Bloom Swell', 'SVF LP24', 300, 0.15, 8, lib='Spectral/Brightness Bloom', tags=('spectral', 'bloom'), extra_dst=('at1', 0.6))
swell('Haze Lift', 'SVF LP12', 400, 0.1, 12, lib='Drift Spectra/Drift Haze', tags=('haze', 'roach'), extra_dst=('at1', 0.5))
swell('Saw Wall Ascent', 'Ladder LP24', 180, 0.35, 9, engine='an', tags=('saw', 'wall', 'epic'))
swell('Ember Rise', 'Diode LP24', 220, 0.4, 7, lib='Drift Spectra/Drift Ember', tags=('ember', 'warm'), extra_dst=('at1', 0.5))
swell('Lumen Unfold', 'SVF LP24', 350, 0.1, 15, lib='Drift Spectra/Drift Lumen', tags=('light', 'corbacho'), extra_dst=('at1', 0.6))
swell('Night Close', 'OB-Xa LP24', 6000, 0.15, 10, depth=-0.8, lib='Ambient/Night Sky Shimmer', tags=('closing', 'night'))
swell('Breathing Sweep Loop', 'SEM LP12', 400, 0.3, 4, fall=4, tags=('breathing', 'looped')).mseg(1, [(0, 0.0, 0), (4, 1.0, 0.5), (4, 0.0, -0.5)], loop=(0, 2), mode='loop')

# ================================================================================================ C. RESONANT SWEEPS
def reso(name, filt, cut, res, depth, rate=None, div=None, wave='tri', drive=0.25, tags=(), wa='saw'):
    p = P(name, SW).an('a', wa, voices=2, detune=6, slop=0.3, level=0.5).an('b', 'saw', oct=-1, level=0.3, slop=0.3).off('c') \
        .f1(filt, cut, res=res, key=0.3, drive=drive).aenv(0.8, 2, 1, 3).chorus('chorus', 0.15, 0.1, 0.35)
    sweep_lfo(p, 'f1cut', depth, rate=rate, div=div, wave=wave)
    macros_sweep(p, 'f1cut', extra=('Drive', 'f1drive', 0.4))
    p.tags('sweep', 'resonant', *tags)
    swX(p, 'f1res', 0.2, 0.3)
    return p

reso('Acid Wash Sweep', 'Ladder LP24', 400, 0.75, 0.6, div='2 bars', tags=('acid',))
reso('Whistling Ladder', 'Ladder LP24', 600, 0.85, 0.55, rate=0.05, drive=0.1, tags=('whistle', 'self-oscillation'))
reso('SEM Vocal Resonance', 'SEM BP12', 700, 0.7, 0.6, rate=0.07, tags=('sem', 'vocal'))
reso('MS-20 Shriek Sweep', 'MS-20 LP12', 500, 0.8, 0.6, div='4 bars', drive=0.4, tags=('ms20', 'aggressive'))
reso('Diode Bubble Sweep', 'Diode LP24', 350, 0.7, 0.55, div='1 bar', tags=('diode', 'bubbling'))
reso('OB Singing Filter', 'OB-Xa BP', 800, 0.65, 0.55, rate=0.04, tags=('oberheim', 'singing'))
reso('Xpander Phaser Res', 'Xpander PH3', 900, 0.7, 0.6, rate=0.06, tags=('xpander', 'phasing'))
reso('Highpass Resonance Lift', 'Ladder HP24', 300, 0.7, 0.6, div='4 bars', tags=('highpass', 'lift'))
reso('Square Res Wobble', 'Ladder LP24', 500, 0.7, 0.4, div='1/4', wave='sine', wa='pulse', tags=('wobble', 'synced'))
reso('Peak Hunter', 'SVF Peak', 1000, 0.8, 0.7, rate=0.11, wave='drift', tags=('peak', 'random'))

# ================================================================================================ D. FLANGES SWEEPS (comb / flanger / phaser filter types)
def flange(name, ftype, cut, res, depth, shape=0.5, rate=None, div=None, src='an', lib=None, key=1.0, tags=(), noise=0.0):
    p = P(name, SW)
    if src == 'an': p.an('a', 'saw', voices=3, detune=10, slop=0.4, level=0.45).an('b', 'saw', oct=-1, voices=2, detune=8, level=0.3, slop=0.3)
    elif src == 'noise': p.off('a', 'b')
    else: p.wt('a', lib, pos=0.3, blend='spectral', voices=3, detune=8, level=0.45).an('b', 'saw', oct=-1, level=0.25, slop=0.3)
    p.off('c')
    if noise or src == 'noise': p.noise(noise or 0.8, 0.6)
    p.f1(ftype, cut, res=res, key=key, shape=shape).f2('SVF LP12', 9000).aenv(1, 2, 1, 4).chorus('chorus', 0.1, 0.08, 0.3)
    sweep_lfo(p, 'f1cut', depth, rate=rate, div=div, wave='tri', second=('f1shape', 0.3, 0.02))
    macros_sweep(p, 'f1cut', extra=('Feedback', 'f1res', 0.3), tone='f2cut')
    p.tags('sweep', 'flanges', 'flanger-filter' if ftype.startswith(('Flg', 'FPhs')) else ('phaser-filter' if ftype.startswith('Phs') else 'comb'), *tags)
    swX(p, 'f1cut')
    return p

flange('Jet Flange Saws', 'Flg +', 400, 0.6, 0.6, rate=0.05, tags=('jet',))
flange('Negative Flange Wash', 'Flg -', 500, 0.65, 0.6, rate=0.04, tags=('hollow',))
flange('Comb Rain Sweep', 'Cmb L6+', 300, 0.7, 0.6, src='noise', key=0.0, rate=0.06, tags=('noise', 'rain'))
flange('Comb Choir Glide', 'Cmb +', 220, 0.75, 0.5, rate=0.03, tags=('choir-like',))
flange('Phaser Stack 48', 'Phs 48+', 600, 0.6, 0.65, rate=0.04, tags=('deep-phaser',))
flange('Phaser 24 Drift', 'Phs 24-', 800, 0.5, 0.6, rate=0.07, tags=('gentle',))
flange('Feedback Phaser Storm', 'Phs 48HL6+', 500, 0.8, 0.6, rate=0.05, tags=('feedback', 'storm'))
flange('Flange-Phaser Spiral', 'FPhs 12HL6+', 400, 0.7, 0.6, div='4 bars', tags=('spiral', 'synced'))
flange('Wavetable Comb Halo', 'Cmb H6+', 350, 0.7, 0.55, src='wt', lib='Ambient/Ice Cathedral', rate=0.03, tags=('halo',))
flange('Lowpass Flanger Dusk', 'Flg L6+', 300, 0.7, 0.55, src='wt', lib='Ambient/Blue Hour', rate=0.025, tags=('dusk', 'soft'))
flange('Noise Jet Engine', 'Flg HL6+', 250, 0.75, 0.7, src='noise', key=0.0, rate=0.08, tags=('noise', 'jet', 'fx'))
flange('Comb Tuned Resonator Sweep', 'Cmb HL6-', 200, 0.8, 0.5, div='2 bars', tags=('tuned-comb', 'metallic'))

# ================================================================================================ E. WAVETABLE SCANS
def scan(name, lib, depth=0.9, rate=None, div=None, mseg=None, blend='spectral', cut=7000, tags=(), wave='tri', oct_b=-1):
    p = P(name, SW).wt('a', lib, pos=0.0, blend=blend, voices=3, detune=7, level=0.5) \
        .wt('b', lib, pos=0.5, blend=blend, oct=oct_b, voices=2, detune=5, level=0.3).off('c') \
        .f1('SVF LP12', cut, res=0.05, key=0.15).aenv(1.5, 3, 1, 5).chorus('ensemble', 0.2, 0.06, 0.4)
    if mseg: sweep_mseg(p, 'at1', depth, mseg).mod('mseg1', 'bt1', -depth * 0.6)
    else: sweep_lfo(p, 'at1', depth, rate=rate, div=div, wave=wave).mod('lfo1', 'bt1', -depth * 0.5)
    macros_sweep(p, 'at1', rate_dst='mseg1rate' if mseg else 'lfo1rate', extra=('Layer', 'blevel', 0.4))
    p.tags('sweep', 'wavetable-scan', 'wavetable', *tags)
    swX(p, 'at1', 0.3, 0.4)
    return p

scan('Formant Glide Scan', 'Spectral/Formant Glide', rate=0.04, tags=('formant',))
scan('Resonant Saw Table Sweep', 'Analog/Reso Sweep Saw', rate=0.06, tags=('resonant', 'analog-table'))
scan('Tilt Dark to Bright Scan', 'Spectral/Tilt Dark to Bright', mseg=12, tags=('brightening', 'one-shot-sweep'))
scan('Comb Sweep Table', 'Spectral/Comb Sweep Saw', div='4 bars', tags=('comb', 'synced'))
scan('Harmonic Walk Scan', 'Additive/Harmonic Walk 1-32', rate=0.02, tags=('harmonic', 'slow'))
scan('FM Index Sweep Pad', 'FM/FM 1-2 Index Sweep', rate=0.05, tags=('fm', 'digital'))
scan('Sync Table Sweep', 'Analog/Sync Saw 1-8', div='2 bars', tags=('sync', 'synced'))
scan('Vowel Table Journey', 'Vocal/Tenor Vowels A-E-I-O-U', rate=0.03, tags=('vowel', 'vocal'))
scan('Stepped Glass Sweep', 'Stepped/Wave Steps Glass', div='1 bar', blend='stepped', wave='sawup', tags=('stepped', 'ppg', 'rhythmic'))
scan('Random Walk Spectrum Scan', 'Textures/Random Walk Spectrum 2', rate=0.015, wave='drift', tags=('random', 'evolving'))
scan('PD Resonance Sweep', 'Phase Distortion/PD Saw Resonance', rate=0.07, tags=('phase-distortion', 'cz'))
scan('Partial Rise Scan', 'Spectral/Partial Rise', mseg=10, tags=('rising', 'one-shot-sweep'))

# ================================================================================================ F. VOWEL / FORMANT SWEEPS
def vowel(name, rate, src='add', tags=(), cut=1200):
    p = P(name, SW)
    if src == 'add': p.add('a', 'Choir Ah', 'Choir Oo', morph=0.3, partials=48, vowel=0.0, vowelamt=0.8, shim=0.15, srate=0.06, level=0.5)
    else: p.an('a', 'saw', voices=3, detune=10, slop=0.3, level=0.45)
    p.an('b', 'saw', oct=-1, level=0.25, slop=0.3).off('c').f1('Vowel', cut, res=0.3, key=0.2, shape=0.0).aenv(1.5, 3, 1, 4).chorus('ensemble', 0.2, 0.06, 0.4)
    sweep_lfo(p, 'f1shape', 0.9, rate=rate, wave='tri')
    if src == 'add': p.mod('lfo1', 'at4', 0.8)
    macros_sweep(p, 'f1shape', extra=('Formant', 'f1cut', 0.3))
    p.tags('sweep', 'vowel', 'formant', *tags)
    swX(p, 'f1shape', 0.3, 0.5)
    return p

vowel('A E I O U Choir Sweep', 0.04, tags=('choir',))
vowel('Talking Saw Wall', 0.12, src='an', tags=('talking', 'saw'))
vowel('Slow Vowel Monks', 0.015, tags=('monks', 'slow'), cut=800)
vowel('Formant Wah Pad', 0.3, src='an', tags=('wah', 'rhythmic'))
vowel('Whispered Vowels', 0.05, tags=('whisper', 'airy'), cut=1600).noise(0.1, 0.8)
vowel('Throat Formant Drift', 0.025, tags=('throat', 'deep'), cut=700)

# ================================================================================================ G. ADDITIVE SPECTRAL SWEEPS
def spectral(name, dst, amt, rate, s1='Saw', s2='Organ', tags=(), peak=8, peakamt=0.7, comb=0.0, extra=None):
    p = P(name, SW).add('a', s1, s2, morph=0.3, partials=96, peak=peak, peakamt=peakamt, width=0.2, comb=comb, shim=0.15, srate=0.05, level=0.5) \
        .an('b', 'sine', oct=-1, level=0.3, slop=0.1).off('c').f1('SVF LP12', 9000, res=0.03, key=0.1).aenv(1.5, 3, 1, 5).chorus('ensemble', 0.2, 0.05, 0.4)
    sweep_lfo(p, dst, amt, rate=rate, wave='tri')
    if extra: p.mod('lfo1', extra[0], extra[1])
    macros_sweep(p, dst, extra=('Morph', 'at1', 0.4))
    p.tags('sweep', 'additive', 'spectral', *tags)
    swX(p, dst, 0.3, 0.4)
    return p

spectral('Harmonic Searchlight', 'at2', 0.9, 0.03, tags=('peak-sweep', 'overtones'))
spectral('Overtone Scanner', 'at2', 0.9, 0.06, s1='Octaves', s2='Primes', peakamt=0.9, tags=('overtone', 'singing'))
spectral('Spectral Tilt Tide', 'at3', 0.8, 0.02, tags=('tilt', 'tidal'))
spectral('Comb Partials Sweep', 'at2', 0.7, 0.05, comb=0.6, tags=('comb', 'partials'), extra=('at1', 0.4))
spectral('Glass Spectrum Lift', 'at2', 0.8, 0.025, s1='Glass', s2='Bell', tags=('glass', 'bright'))
spectral('Organ Drawbar Sweep', 'at1', 0.9, 0.04, s1='Organ', s2='Octaves', peakamt=0.3, tags=('organ', 'drawbar'))

# ================================================================================================ H. SYNC / FM SWEEPS
def syncfm(name, kind, amt, rate=None, div=None, tags=(), cut=4000):
    p = P(name, SW).an('a', 'saw', level=0.4, slop=0.3).an('b', 'saw', level=0.4, slop=0.3, voices=2, detune=6).off('c') \
        .f1('OB-Xa LP12', cut, res=0.1, key=0.3).aenv(0.6, 2, 1, 3).chorus('ensemble', 0.2, 0.1, 0.4)
    if kind == 'sync':
        p.hardsync('b', 'a').set('obAnSync', 4)
        sweep_lfo(p, 'bt2', amt, rate=rate, div=div)
        dst = 'bt2'
    else:
        p.fm('b', 'a', 0.1, kind)
        sweep_lfo(p, 'bfm', amt, rate=rate, div=div)
        dst = 'bfm'
    macros_sweep(p, dst, extra=('Detune', 'detune', 0.3))
    p.tags('sweep', kind if kind == 'sync' else 'fm', *tags)
    swX(p, dst, 0.35, 0.4)
    return p

syncfm('Sync Sweep Classic', 'sync', 0.7, rate=0.08, tags=('classic',))
syncfm('Sync Tear Synced', 'sync', 0.8, div='1 bar', tags=('synced', 'aggressive'))
syncfm('Slow Sync Dawn', 'sync', 0.6, rate=0.02, tags=('slow', 'evolving'), cut=2500)
syncfm('FM Index Breathing', 'fm', 0.5, rate=0.05, tags=('breathing',))
syncfm('PM Bell Sweep', 'pm', 0.6, rate=0.04, tags=('bell', 'pm'))
syncfm('Ring Sweep Shimmer', 'ring', 0.7, rate=0.06, tags=('ring', 'shimmer'))

# ================================================================================================ I. GRANULAR & PHYSICAL SWEEPS
p = P('Granular Position Sweep', SW).gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.0, size=0.6, density=14, spray=0.2, level=0.5) \
    .an('b', 'sine', oct=-1, level=0.25, slop=0.1).off('c').f1('SVF LP12', 6000, res=0.05, key=0.15).aenv(1.5, 3, 1, 5).chorus('ensemble', 0.2, 0.05, 0.4)
sweep_lfo(p, 'at1', 0.9, rate=0.02)
macros_sweep(p, 'at1', extra=('Density', 'at3', 0.4)).tags('sweep', 'granular', 'position-scan')
swX(p, 'at1', 0.3, 0.4)

p = P('Grain Size Bloom', SW).gr('a', 'keyed', sample='Strings/Bowed Cello Drone', pos=0.3, size=0.1, density=18, spray=0.3, level=0.5) \
    .off('b', 'c').f1('SVF LP12', 7000, res=0.05, key=0.15).aenv(1, 3, 1, 5).chorus('ensemble', 0.2, 0.05, 0.4)
sweep_mseg(p, 'at2', 0.9, 10)
macros_sweep(p, 'at2', rate_dst='mseg1rate', extra=('Spray', 'at4', 0.4)).tags('sweep', 'granular', 'bloom', 'one-shot-sweep')
swX(p, 'at2', 0.3, 0.4)

p = P('Rain Stream Sweep', SW).gr('a', 'fixed', sample='Nature/Rain Light', pos=0.2, size=0.5, density=18, spray=0.5, level=0.9) \
    .off('b', 'c').f1('Phs 48+', 600, res=0.6, key=0.0, shape=0.5).aenv(1, 2, 1, 4)
sweep_lfo(p, 'f1cut', 0.6, rate=0.04)
macros_sweep(p, 'f1cut', extra=('Feedback', 'f1res', 0.3)).tags('sweep', 'granular', 'rain', 'phaser-filter', 'soundscape')
swX(p, 'f1cut')

p = P('Bow Position Sweep', SW).ph('a', 'String', 'Bow', decay=8, damp=0.2, hard=0.5, pos=0.1, level=0.55) \
    .ph('b', 'Sympathetic', 'Osc Input', decay=12, damp=0.05, hard=0.3, level=0.25).off('c').f1('SVF LP12', 6000, res=0.04, key=0.3).aenv(0.6, 2, 1, 3)
sweep_lfo(p, 'at1', 0.7, rate=0.05)
macros_sweep(p, 'at1', extra=('Bow Force', 'at2', 0.4)).tags('sweep', 'physical', 'bowed', 'position-scan')
swX(p, 'at2', 0.4, 0.3, vib=0.02)

p = P('Plate Strike Position Sweep', SW).ph('a', 'Plate', 'Strike', decay=10, damp=0.06, hard=0.6, pos=0.1, level=0.5).off('b', 'c') \
    .f1('SVF LP12', 9000, res=0.03, key=0.1).aenv(0.002, 10, 0.0, 10).curve(a=1).arp('up', 2, '1/8', 0.5)
sweep_lfo(p, 'at1', 0.8, div='4 bars')
macros_sweep(p, 'at1', extra=('Hardness', 'at2', 0.4)).tags('sweep', 'physical', 'plate', 'arp')
swX(p, 'at2', 0.3, 0.3)

p = P('Tube Breath Resonance Sweep', SW).ph('a', 'Tube', 'Breath', decay=4, damp=0.3, hard=0.45, pos=0.3, level=0.55).noise(0.06, 0.6).off('b', 'c') \
    .f1('SVF LP12', 2600, res=0.35, key=0.5).aenv(0.4, 2, 1, 2).voice('legato', 0.06)
sweep_lfo(p, 'f1cut', 0.5, rate=0.06)
macros_sweep(p, 'f1cut', extra=('Breath', 'at2', 0.4)).tags('sweep', 'physical', 'breath', 'flute')
swX(p, 'at2', 0.45, 0.3, vib=0.02)

# ================================================================================================ J. RISERS & DOWNLIFTERS
def riser(name, length, down=False, noise=0.6, pitch=0.5, cut=200, tags=(), ftype='SVF LP24', src='saw'):
    p = P(name, SW)
    if src == 'saw': p.an('a', 'saw', voices=5, detune=20, slop=0.3, level=0.4)
    else: p.off('a')
    p.off('b', 'c').noise(noise, 0.5).f1(ftype, cut if not down else 12000, res=0.35, key=0.0).aenv(0.01, 1, 1, 1.5) \
     .mseg(1, [(0, 0.0, 0), (length, 1.0, 0.6)], loop=(1, 1), mode='env') \
     .mod('mseg1', 'f1cut', -1.0 if down else 1.0).mod('mseg1', 'pitch', -pitch if down else pitch).mod('mseg1', 'f1res', 0.2) \
     .phaser(0.3, 0.2, 0.7, 0.5).width(1.5)
    macros_sweep(p, 'f1cut', rate_dst='mseg1rate', extra=('Noise', 'noise', 0.3))
    p.tags('sweep', 'fx', 'downlifter' if down else 'riser', 'one-shot', *tags)
    swX(p, 'f1cut', 0.2, 0.2)
    return p

riser('Four Bar Riser', 8, tags=('build-up',))
riser('Eight Second Noise Riser', 8, src='noise', noise=0.9, pitch=0.0, tags=('noise',))
riser('Long Saw Uplifter', 16, tags=('long', 'cinematic'))
riser('Short Whoosh Up', 2, noise=0.8, pitch=0.3, tags=('whoosh', 'short'))
riser('Downlifter Sink', 6, down=True, tags=('falling',))
riser('Noise Downsweep', 4, down=True, src='noise', noise=0.9, pitch=0.0, tags=('noise', 'falling'))
riser('Resonant Rise Scream', 10, ftype='Ladder LP24', cut=150, tags=('resonant', 'tension'))
riser('Phaser Tunnel Rise', 12, src='noise', noise=0.8, pitch=0.0, ftype='Phs 48HL6+', cut=200, tags=('tunnel', 'phaser-filter'))
