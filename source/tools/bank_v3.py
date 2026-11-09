# Driftfield 3 — new factory presets (exec'd by gen_presets3.py; P / Patch helpers are in scope).
# Tags 'roach' / 'corbacho' mark presets aimed at those territories so the browser search finds them.
import math as _m

# ------------------------------------------------------------------------------------------- helpers
def shape(fn): return [max(-1.0, min(1.0, fn(i / 64.0))) for i in range(64)]
BREATH = shape(lambda x: -1 + 2 * (_m.sin(x / 0.62 * _m.pi / 2) ** 2 if x < 0.62 else _m.cos((x - 0.62) / 0.38 * _m.pi / 2) ** 2))
HEART = shape(lambda x: (1.0 if x < 0.06 else (-0.3 if x < 0.12 else (0.6 if 0.18 < x < 0.24 else -0.7))))
STAIRS4 = shape(lambda x: [-0.9, -0.2, 0.4, 1.0][int(x * 4)])
TRIBAL = shape(lambda x: [1, -0.6, 0.2, -0.6, 0.8, -0.6, 0.3, -0.8][int(x * 8)])
SWELL = shape(lambda x: -1 + 2 * (x ** 3))
TIDE = shape(lambda x: _m.sin(2 * _m.pi * x) * 0.7 + 0.3 * _m.sin(6 * _m.pi * x + 1))
RIPPLE = shape(lambda x: _m.sin(2 * _m.pi * x) * (1 - x) + 0.25 * _m.sin(2 * _m.pi * 7 * x) * x)

def slow_swell(level_peak=1.0, attack=12.0, sustain=0.75, fall=20.0):
    return [(0, 0.0, 0), (attack, level_peak, 0.4), (fall * 0.4, sustain, -0.3), (fall, 0.0, 0.3)]

OVERTONE_SER = spec_series([1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 16, 20, 24, 32], 0.18, 128)
ORGAN_FIFTHS = spec_series([1, 3, 2, 6, 4, 12, 8, 24, 16, 48], 0.22, 128)
STONE = spec_random(71, 0.65, 0.4, 128)
EMBER128 = spec_formant([(1, 0.6, 1.0), (5, 2.0, 0.5), (11, 3.0, 0.25), (23, 4.0, 0.12)], 1.0, 128)
LUMINOUS = spec_formant([(1, 0.5, 0.8), (6, 1.0, 0.6), (12, 1.5, 0.5), (24, 2.5, 0.35), (48, 4.0, 0.2)], 0.45, 128)
DIDJ = spec_formant([(1, 0.5, 1.0), (3, 0.7, 0.5), (7, 1.0, 0.6), (11, 1.4, 0.3)], 1.2, 128)

# =============================================================================================== INIT
for nm, typ in [('Init', None), ('Init Wavetable', 'wt'), ('Init Analog', 'analog'), ('Init Additive', 'add'), ('Init Physical', 'phys'), ('Init Granular', 'gran')]:
    p = P(nm, 'Init')
    if typ == 'wt': p.wt('a', table=0, pos=0.0).off('b')
    elif typ == 'analog': p.an('a', 'saw').off('b')
    elif typ == 'add': p.add('a', 'Saw', 'Organ').off('b')
    elif typ == 'phys': p.ph('a', 'String', 'Bow').off('b')
    elif typ == 'gran': p.gr('a', 'keyed', sample='Voices/Choir Cloud').off('b')
    p.macro(1, 'Macro 1').tags('init')

# =============================================================================================== DRONES
P('Silica Breath', 'Drones') \
    .gr('a', 'keyed', sample='Drones/Spectral Shimmer', pos=0.35, scan=0.05, size=0.42, density=26, spray=0.25, prand=12, shape=0.4, spread=0.8, level=0.8) \
    .wt('b', 'Glass & Bell/Halo', pos=0.3, blend='spectral', oct=-1, level=0.35, voices=3, detune=8) \
    .off('c').f1('SVF LP12', 5200, res=0.1, key=0.1).aenv(6, 4, 1, 16) \
    .lfo(1, 0.02, shape=BREATH).lfo(2, 0.05, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.2).mod('lfo2', 'bt1', 0.4).mod('lfo1', 'f1cut', 0.2) \
    .macro(1, 'Scan', 0.0).mod('m1', 'at1', 0.6).macro(2, 'Grain Size').mod('m2', 'at2', 0.6).macro(3, 'Glass').mod('m3', 'blevel', 0.5).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
    .chorus('ensemble', 0.3, 0.1, 0.5).tags('luminous', 'corbacho', 'glacial')

P('Tanpura Field', 'Drones') \
    .ph('a', 'Tanpura', 'Strike', decay=14, damp=0.05, hard=0.8, pos=0.25, buzz=0.75, level=0.8) \
    .ph('b', 'Tanpura', 'Strike', decay=14, damp=0.07, hard=0.75, pos=0.3, buzz=0.6, semi=7, level=0.55, pan=0.3) \
    .ph('c', 'Sympathetic', 'Bow', decay=10, damp=0.25, hard=0.4, buzz=0.9, oct=-1, level=0.3) \
    .f1('Off', 20000).aenv(0.002, 4, 1, 12).arp('order', 1, '1/2', 0.95, hold=True) \
    .lfo(1, 0.03, 'drift').mod('lfo1', 'cpitch', 0.01).mpe(velAmp=0.2) \
    .macro(1, 'Ring Time').mod('m1', 'at3', 0.4).mod('m1', 'bt3', 0.4).macro(2, 'Drone Bow').mod('m2', 'clevel', 0.5).macro(3, 'Brightness').mod('m3', 'at2', 0.25).mod('m3', 'bt2', 0.25) \
    .chorus('chorus', 0.15, 0.08, 0.3).tags('ritual', 'plucked', 'indian', 'roach', 'hold-a-chord')

P('Coupled Bowls', 'Drones') \
    .ph('a', 'Glass Bowl', 'Bow', decay=16, damp=0.12, hard=0.35, pos=0.4, couple=0.65, level=0.8) \
    .ph('b', 'Glass Bowl', 'Bow', decay=14, damp=0.15, hard=0.3, pos=0.25, couple=0.4, semi=7, oct=-1, level=0.45, pan=-0.3) \
    .add('c', 'Glass', 'Primes', morph=0.3, partials=48, shim=0.5, srate=0.07, level=0.18, oct=1) \
    .f1('SVF LP12', 7000, res=0.05, key=0).aenv(5, 4, 1, 18) \
    .lfo(1, 0.03, 'tri').lfo(2, 0.011, 'sine').mod('lfo1', 'at1', 0.3).mod('lfo2', 'bt1', -0.35).mod('lfo2', 'clevel', 0.15) \
    .macro(1, 'Brightness').mod('m1', 'at4', -0.4).mod('m1', 'bt4', -0.4).macro(2, 'Bow Pressure').mod('m2', 'at2', 0.4).mod('m2', 'bt2', 0.4).macro(3, 'Halo').mod('m3', 'clevel', 0.5) \
    .chorus('chorus', 0.2, 0.06, 0.4).tags('metallic', 'bowed', 'corbacho', 'meditative')

P('Void Engine', 'Drones') \
    .an('a', 'supersaw', pw=0.35, detune=30, oct=-1, level=0.7).an('b', 'tri', oct=-2, level=0.4, slop=0.5).off('c') \
    .f1('OB-Xa LP24', 260, res=0.45, drive=0.3, key=0.1, env=0.0).sub(0.25) \
    .mseg(1, [(0, 0.05, 0), (18, 1.0, 0.5), (8, 0.55, -0.2), (22, 0.1, 0.4), (14, 0.05, 0)], loop=(0, 4), mode='free', scale=1.0) \
    .aenv(8, 6, 1, 18).lfo(1, 0.045, 'drift', 'voice').mod('mseg1', 'f1cut', 0.55).mod('lfo1', 'f1res', 0.15).mod('lfo1', 'at1', 0.15) \
    .tape(0.25, 0.15, 0.05, 0.6, 1.0) \
    .macro(1, 'Opening').mod('m1', 'f1cut', 0.5).macro(2, 'Weight').mod('m2', 'sub', 0.5).macro(3, 'Saw Mix').mod('m3', 'at1', 0.4).macro(4, 'Grit').mod('m4', 'f1drive', 0.6) \
    .chorus('ensemble', 0.25, 0.08, 0.5).tags('dark', 'roach', 'magnificent', 'slow')

P('Basalt Choir', 'Drones') \
    .gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.2, scan=0.04, size=0.55, density=20, spray=0.35, prand=8, shape=0.35, spread=0.9, oct=-1, level=0.85) \
    .an('b', 'saw', oct=-1, level=0.25, voices=3, detune=8, slop=0.4).off('c') \
    .f1('Vowel', 700, res=0.55, key=0.0, shape=0.3, morphto='SEM LP12', morph=0.4).f2('SEM LP12', 3000) \
    .lfo(1, 0.017, shape=BREATH).lfo(2, 0.07, 'drift', 'voice').mod('lfo1', 'f1shape', 0.35).mod('lfo2', 'at1', 0.2).mod('lfo1', 'f1morph', 0.3) \
    .aenv(7, 4, 1, 16) \
    .macro(1, 'Vowel').mod('m1', 'f1shape', 0.7).macro(2, 'Cloud').mod('m2', 'at4', 0.5).macro(3, 'Body').mod('m3', 'blevel', 0.5) \
    .chorus('solina', 0.3, 0.12, 0.5).tags('choir', 'dark', 'roach', 'granular-cloud')

P('Overtone Ladder', 'Drones') \
    .add('a', 'Throat', 'Throat', partials=64, peak=6, peakamt=1.0, width=0.06, shim=0.05, pdet=2, level=0.85) \
    .an('b', 'saw', oct=-1, level=0.2, slop=0.3).off('c').f1('SEM LP12', 4000, res=0.05, key=0.2) \
    .mseg(1, [(0, 0.0, 0), (6, 0.35, 0), (6, 0.55, 0), (6, 0.7, 0), (6, 0.45, 0), (6, 0.85, 0), (6, 0.25, 0), (6, 0.0, 0)], loop=(0, 7), mode='free') \
    .mod('mseg1', 'at2', 0.75).aenv(3, 4, 1, 12).lfo(1, 0.05, 'drift').mod('lfo1', 'at3', 0.05) \
    .macro(1, 'Overtone').mod('m1', 'at2', 0.5).macro(2, 'Nasal').mod('m2', 'at3', -0.3).macro(3, 'Drone Saw').mod('m3', 'blevel', 0.5) \
    .chorus('chorus', 0.12, 0.05, 0.3).tags('throat', 'overtone', 'vocal', 'roach', 'ritual')

P('Desert Monolith', 'Drones') \
    .wt('a', 'Drone/Monolith', pos=0.2, blend='spectral', voices=3, detune=7, level=0.75) \
    .wt('b', 'Spectral/Resonant BP Sweep', pos=0.5, blend='spectral', oct=-1, level=0.35, warp='bend+', warpamt=0.2) \
    .off('c').f1('Xpander BP4', 900, res=0.35, key=0.2).f2('Xpander LP4', 3000, res=0.1).route('parallel', 0.55) \
    .lfo(1, 0.013, 'sine').lfo(2, 0.031, 'tri').lfo(3, 0.07, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.5).mod('lfo3', 'f1cut', 0.3).mod('lfo1', 'balance', 0.2) \
    .aenv(6, 4, 1, 18).sub(0.2) \
    .macro(1, 'Sand').mod('m1', 'f1cut', 0.5).macro(2, 'Monolith').mod('m2', 'at1', 0.5).macro(3, 'Bend').mod('m3', 'bt2', 0.6) \
    .chorus('ensemble', 0.25, 0.07, 0.5).tags('desert', 'roach', 'dark', 'evolving')

P('Iron Harmonium', 'Drones') \
    .wt('a', table=0, pos=0.0, level=0.7, voices=2, detune=5).an('b', 'sine', semi=7, level=0.0, slop=0.1) \
    .fm('a', 'b', 0.35, 'pm').an('c', 'saw', oct=-1, level=0.3, slop=0.3).f1('Ladder LP24', 1400, res=0.2, key=0.3) \
    .lfo(1, 0.04, 'sine').lfo(2, 0.09, 'drift', 'voice').mod('lfo1', 'afm', 0.25).mod('lfo2', 'f1cut', 0.2) \
    .aenv(2, 3, 1, 10).macro(1, 'FM Index').mod('m1', 'afm', 0.5).macro(2, 'Reed').mod('m2', 'clevel', 0.5) \
    .chorus('ensemble', 0.3, 0.1, 0.5).tags('fm', 'organ', 'metallic')

P('Deep Hollow Earth', 'Drones') \
    .an('a', 'tri', oct=-1, level=0.7, slop=0.5, drift=0.5).an('b', 'saw', oct=-2, level=0.35, slop=0.5).off('c') \
    .noise(0.15, 0.1).f1('Vowel', 500, res=0.5, key=0.0, shape=0.9).f2('Ladder LP12', 900, res=0.1) \
    .lfo(1, 0.009, 'sine').lfo(2, 0.023, 'drift').mod('lfo1', 'f1shape', 0.45).mod('lfo2', 'f1cut', 0.35).mod('lfo2', 'noise', 0.15) \
    .aenv(8, 4, 1, 20).sub(0.45, 'sine', -1) \
    .macro(1, 'Cavern').mod('m1', 'f1shape', -0.6).macro(2, 'Breath').mod('m2', 'noise', 0.5).macro(3, 'Depth').mod('m3', 'sub', 0.4) \
    .chorus('chorus', 0.15, 0.05, 0.4).tags('dark', 'sub', 'roach', 'cave')

P('Shifted Monastery', 'Drones') \
    .add('a', 'Choir Ah', 'Choir Oo', morph=0.3, partials=48, shim=0.3, srate=0.08, pdet=5, vowel=0.2, vowelamt=0.5, level=0.8) \
    .an('b', 'saw', oct=-1, voices=3, detune=10, level=0.25).off('c').f1('SEM LP12', 2600, res=0.1, key=0.2) \
    .shifter(0.6, 0.35, 0.55, 0.8).lfo(1, 0.021, 'sine').lfo(2, 0.06, 'drift', 'voice').mod('lfo1', 'at1', 0.35).mod('lfo2', 'at4', 0.3) \
    .aenv(5, 3, 1, 16).macro(1, 'Shift Hz').mod('m1', 'fxshift', 0.04).macro(2, 'Barber Pole').mod('m2', 'fxshiftmix', 0.5).macro(3, 'Vowel').mod('m3', 'at4', 0.6) \
    .chorus('chorus', 0.15, 0.08, 0.3).tags('choir', 'shimmer', 'corbacho', 'shifter')

P('Sympathetic Resonance', 'Drones') \
    .ph('a', 'Sympathetic', 'Bow', decay=12, damp=0.2, hard=0.45, pos=0.3, buzz=0.85, level=0.8) \
    .ph('b', 'Sympathetic', 'Breath', decay=10, damp=0.3, hard=0.3, buzz=0.7, oct=-1, level=0.4, pan=0.4) \
    .off('c').f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(4, 3, 1, 14) \
    .lfo(1, 0.025, 'drift').lfo(2, 0.07, 'sine', 'voice').mod('lfo1', 'at2', 0.25).mod('lfo2', 'at1', 0.25) \
    .macro(1, 'Damping').mod('m1', 'at4', 0.4).macro(2, 'Bow').mod('m2', 'at2', 0.5).macro(3, 'Breath Layer').mod('m3', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.07, 0.4).tags('bowed', 'strings', 'corbacho', 'resonant')

P('Nocturne Bands', 'Drones') \
    .gr('a', 'fixed', sample='Nature/Wind Bands', pos=0.4, scan=0.03, size=0.8, density=12, spray=0.5, shape=0.3, spread=1.0, level=0.6) \
    .add('b', 'Organ', 'Octaves', morph=0.4, partials=40, tilt=0.25, shim=0.15, oct=-1, level=0.55).off('c') \
    .f1('Dual Bandpass', 900, res=0.4, shape=0.6, key=0.0).f2('SEM LP12', 2400).route('split', 0.5) \
    .lfo(1, 0.012, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'bt1', 0.3).aenv(8, 4, 1, 18) \
    .macro(1, 'Wind').mod('m1', 'alevel', 0.4).macro(2, 'Organ').mod('m2', 'blevel', 0.4).macro(3, 'Bands').mod('m3', 'f1shape', 0.5) \
    .chorus('ensemble', 0.2, 0.05, 0.5).tags('night', 'wind', 'corbacho', 'granular-cloud')

P('Gong Bath', 'Drones') \
    .gr('a', 'keyed', sample='Metal/Gong Swell', pos=0.5, scan=0.02, size=0.9, density=10, spray=0.3, shape=0.3, spread=0.9, oct=-1, level=0.75) \
    .ph('b', 'Plate', 'Bow', decay=22, damp=0.1, hard=0.65, couple=0.35, oct=-1, level=0.4).off('c') \
    .f1('SVF LP12', 8000, res=0.05, key=0).aenv(6, 4, 1, 22).lfo(1, 0.015, shape=TIDE).mod('lfo1', 'at1', 0.25).mod('lfo1', 'bt1', 0.3) \
    .macro(1, 'Bowing').mod('m1', 'blevel', 0.5).macro(2, 'Strike Position').mod('m2', 'at1', 0.5).macro(3, 'Darken').mod('m3', 'f1cut', -0.6) \
    .chorus('chorus', 0.15, 0.04, 0.4).tags('metallic', 'gong', 'meditative', 'roach')

P('Magnetic Field Lines', 'Drones') \
    .wt('a', 'Drone/Iron Hum', pos=0.3, voices=4, detune=9, level=0.75, blend='spectral').fm('a', 'noise', 0.12, 'pm') \
    .an('b', 'sawpulse', pw=0.3, oct=-1, level=0.3).off('c') \
    .f1('Ladder LP18', 700, res=0.55, key=0.2, fmsrc='b', fmamt=0.18).f2('Comb +', 220, res=0.5, shape=0.5).route('serial') \
    .lfo(1, 0.018, 'sine').lfo(2, 0.11, 'sh', 'voice', smooth=0.6).mod('lfo1', 'at1', 0.5).mod('lfo2', 'f1fm', 0.3).mod('lfo1', 'f2cut', 0.3) \
    .aenv(6, 4, 1, 16).macro(1, 'Field').mod('m1', 'afm', 0.4).macro(2, 'Comb').mod('m2', 'f2res', 0.4).macro(3, 'Audio FM').mod('m3', 'f1fm', 0.5) \
    .chorus('flanger', 0.2, 0.03, 0.6, 0.4).tags('metallic', 'dark', 'fm', 'industrial')

P('Smoke Spirals', 'Drones') \
    .add('a', 'Strings', 'Reed', partials=48, morph=0.5, shim=0.4, srate=0.09, level=0.8) \
    .add('b', 'Hollow', 'Choir Oo', oct=-1, partials=32, morph=0.3, shim=0.2, level=0.4).off('c') \
    .mseg(1, [(0, 0.0, 0), (9, 1.0, 0.3), (9, 0.2, -0.3), (12, 0.8, 0.2), (10, 0.0, -0.2)], loop=(0, 4), mode='free') \
    .mseg(2, [(0, 1.0, 0), (14, 0.0, 0.3), (14, 1.0, -0.3)], loop=(0, 2), mode='free') \
    .mod('mseg1', 'at1', 0.5).mod('mseg2', 'bt1', 0.6).mod('mseg1', 'f1cut', 0.2).f1('SEM LP12', 3000, res=0.1).phaser(0.35, 0.03, 0.8, 0.4) \
    .aenv(5, 4, 1, 15).macro(1, 'Spiral Speed', 0.0).mod('m1', 'mseg1rate', 0.6).mod('m1', 'mseg2rate', 0.6).macro(2, 'Smoke').mod('m2', 'at3', 0.4) \
    .nochorus().tags('evolving', 'additive', 'corbacho')

P('Glass Horizon', 'Drones') \
    .add('a', 'Glass', 'Custom', custom=LUMINOUS, morph=0.4, partials=64, shim=0.6, srate=0.1, stretch=0.05, pdet=6, level=0.75) \
    .wt('b', 'Glass & Bell/Ice Shimmer', pos=0.4, blend='spectral', level=0.3, oct=1, voices=3, detune=10).off('c') \
    .f1('SVF LP12', 9000, res=0.05, key=0).f2('SVF HP12', 120, res=0.1, key=0).lfo(1, 0.011, 'tri').lfo(2, 0.04, 'drift', 'voice') \
    .mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'at3', -0.15).aenv(9, 4, 1, 20) \
    .macro(1, 'Glow').mod('m1', 'at3', 0.4).macro(2, 'Ice').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'at1', 0.3) \
    .chorus('ensemble', 0.25, 0.06, 0.5).width(1.5).tags('luminous', 'glass', 'corbacho', 'bright')

P('Lithic Pulse', 'Drones') \
    .an('a', 'saw', voices=5, detune=10, level=0.7, slop=0.4).an('b', 'pulse', pw=0.3, oct=-1, level=0.4).off('c') \
    .f1('Ladder LP24', 500, res=0.35, drive=0.2, key=0.2).lfo(1, 0.1, 'sine', sync=True, div='4 bars').lfo(2, 0.1, shape=HEART, sync=True, div='1 bar') \
    .mod('lfo1', 'f1cut', 0.45).mod('lfo2', 'amp', 0.12).mod('lfo1', 'bt1', 0.3).aenv(4, 3, 1, 12).sub(0.25) \
    .macro(1, 'Pulse').mod('m1', 'f1cut', 0.3, via='lfo2').macro(2, 'Open').mod('m2', 'f1cut', 0.5) \
    .chorus('ensemble', 0.25, 0.12, 0.5).tags('tempo-synced', 'roach', 'breathing', 'heartbeat')

P('Cavern Voice', 'Drones') \
    .an('a', 'supersaw', pw=0.5, detune=22, level=0.6).an('b', 'saw', oct=-1, level=0.35, voices=2, detune=6).off('c') \
    .f1('Vowel', 800, res=0.7, key=0.1, shape=0.2).f2('SEM LP12', 2500, res=0.1) \
    .mseg(1, [(0, 0.0, 0), (7, 0.25, 0), (7, 0.5, 0), (7, 0.75, 0), (7, 1.0, 0), (10, 0.0, 0.3)], loop=(0, 5), mode='free') \
    .mod('mseg1', 'f1shape', 0.8).lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'f1cut', 0.2).aenv(4, 3, 1, 14) \
    .macro(1, 'Vowel Offset').mod('m1', 'f1shape', 0.5).macro(2, 'Throat Size').mod('m2', 'f1cut', -0.5) \
    .chorus('ensemble', 0.25, 0.08, 0.5).tags('vocal', 'vowel', 'roach', 'cave')

P('Ancestor Hum', 'Drones') \
    .add('a', 'Custom', 'Throat', custom=DIDJ, morph=0.2, partials=48, peak=8, peakamt=0.4, width=0.1, shim=0.05, level=0.8, oct=-1) \
    .noise(0.12, 0.2).off('b', 'c').f1('Formant', 300, res=0.6, key=0.0, shape=0.5).f2('SEM LP12', 1800) \
    .lfo(1, 0.28, shape=TRIBAL, mode='voice').lfo(2, 0.03, 'drift').mod('lfo1', 'f1cut', 0.2).mod('lfo1', 'at2', 0.18).mod('lfo2', 'f1shape', 0.3) \
    .mod('press', 'at2', 0.4).mod('slide', 'f1shape', 0.5).aenv(1.5, 2, 1, 8).sub(0.3) \
    .macro(1, 'Circular Breath').mod('m1', 'lfo1rate', 0.4).macro(2, 'Mouth').mod('m2', 'f1shape', 0.6).macro(3, 'Breath').mod('m3', 'noise', 0.5) \
    .nochorus().tags('didgeridoo', 'tribal', 'roach', 'dreamtime', 'mpe')

P('Silent Bloom', 'Drones') \
    .add('a', 'Organ', 'Custom', custom=ORGAN_FIFTHS, morph=0.6, partials=64, shim=0.25, srate=0.06, level=0.75) \
    .an('b', 'tri', oct=1, level=0.15, slop=0.4).off('c').f1('SEM LP12', 1200, res=0.1, key=0.2) \
    .mseg(1, slow_swell(1.0, 14, 0.7, 24), loop=(2, 2), mode='env').mod('mseg1', 'f1cut', 0.6).mod('mseg1', 'at1', -0.4).mod('mseg1', 'blevel', 0.6) \
    .aenv(10, 4, 1, 22).macro(1, 'Bloom Time', 0.0).mod('m1', 'mseg1rate', -0.5).macro(2, 'Fifths').mod('m2', 'at1', 0.5) \
    .chorus('ensemble', 0.3, 0.05, 0.5).tags('slow', 'luminous', 'corbacho', 'swell')

# =============================================================================================== ATMOSPHERES
P('Rain Temple', 'Atmospheres') \
    .ph('a', 'Plate', 'Rain', decay=3, damp=0.3, hard=0.4, spread=0.95, level=0.6) \
    .gr('b', 'fixed', sample='Nature/Water Drops', pos=0.5, scan=0.05, size=0.12, density=30, spray=0.8, spread=1.0, level=0.4) \
    .add('c', 'Choir Oo', 'Hollow', partials=24, morph=0.3, oct=-1, shim=0.3, level=0.4) \
    .f1('SVF LP12', 7000, res=0.05, key=0).aenv(3, 3, 1, 12).lfo(1, 0.02, 'drift').mod('lfo1', 'at2', 0.3).mod('lfo1', 'clevel', 0.2) \
    .macro(1, 'Rain Density').mod('m1', 'at2', 0.5).macro(2, 'Drops').mod('m2', 'blevel', 0.5).macro(3, 'Temple Choir').mod('m3', 'clevel', 0.5) \
    .chorus('chorus', 0.2, 0.05, 0.4).tags('rain', 'water', 'corbacho', 'field')

P('Scraped Sky', 'Atmospheres') \
    .ph('a', 'Plate', 'Scrape', decay=6, damp=0.2, hard=0.3, couple=0.5, level=0.65) \
    .gr('b', 'keyed', sample='Metal/Scraped Plate', pos=0.4, scan=-0.04, size=0.3, density=22, spray=0.6, prand=50, oct=1, level=0.35).off('c') \
    .f1('SVF BP12', 1800, res=0.3, key=0.2).f2('SVF LP12', 6000).shifter(-1.3, 0.3, 0.4, 0.7) \
    .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.5).mod('lfo1', 'at2', 0.3).aenv(4, 4, 1, 14) \
    .macro(1, 'Scrape').mod('m1', 'at2', 0.6).macro(2, 'Shift').mod('m2', 'fxshiftmix', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
    .nochorus().tags('metallic', 'industrial', 'dark', 'shifter')

P('Salt Flats Wind', 'Atmospheres') \
    .gr('a', 'fixed', sample='Nature/Wind Bands', pos=0.3, scan=0.06, size=1.0, density=14, spray=0.6, shape=0.3, spread=1.0, level=1.0) \
    .noise(0.5, 0.6).off('b', 'c').f1('Vowel', 1000, res=0.6, shape=0.5, key=0).f2('SVF HP12', 200) \
    .lfo(1, 0.013, shape=TIDE).lfo(2, 0.031, 'drift').mod('lfo1', 'f1shape', 0.5).mod('lfo2', 'f1cut', 0.4).mod('lfo1', 'noise', 0.15) \
    .aenv(5, 4, 1, 14).macro(1, 'Gust').mod('m1', 'noise', 0.6).macro(2, 'Whistle').mod('m2', 'f1res', 0.3) \
    .chorus('chorus', 0.2, 0.05, 0.5).width(1.6).tags('wind', 'desert', 'roach', 'field')

P('Aurora Granules', 'Atmospheres') \
    .gr('a', 'keyed', sample='Drones/Spectral Shimmer', pos=0.6, scan=0.1, size=0.25, density=60, spray=0.7, prand=700, shape=0.5, spread=1.0, level=0.6) \
    .wt('b', 'Textures/Aurora', pos=0.4, blend='spectral', voices=4, detune=12, level=0.35).off('c') \
    .f1('SVF LP12', 8000, res=0.05, key=0).lfo(1, 0.02, 'sine').lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'bt1', 0.5).mod('lfo2', 'at4', 0.3) \
    .aenv(6, 4, 1, 16).macro(1, 'Sparkle').mod('m1', 'at3', 0.5).macro(2, 'Pitch Scatter').mod('m2', 'at4', 0.5) \
    .chorus('ensemble', 0.3, 0.05, 0.5).width(1.5).tags('cosmic', 'bright', 'corbacho', 'granular-cloud')

P('Ember Field', 'Atmospheres') \
    .gr('a', 'keyed', sample='Drones/Analog Drone', pos=0.3, scan=0.03, size=0.6, density=18, spray=0.4, rev=0.5, spread=0.8, level=0.7) \
    .add('b', 'Custom', 'Saw', custom=EMBER128, partials=40, tilt=0.3, shim=0.3, srate=0.05, oct=-1, level=0.35).off('c') \
    .f1('Ladder LP24', 1500, res=0.2, key=0.2).tape(0.3, 0.35, 0.1, 0.55, 1.0).lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.3) \
    .aenv(5, 4, 1, 16).macro(1, 'Reverse Grains').mod('m1', 'at4', 0.4).macro(2, 'Embers').mod('m2', 'blevel', 0.5).macro(3, 'Wow').mod('m3', 'fxwow', 0.6) \
    .chorus('chorus', 0.15, 0.05, 0.3).tags('warm', 'tape', 'roach', 'lofi')

P('Night Insects', 'Atmospheres') \
    .gr('a', 'fixed', sample='Nature/Water Drops', pos=0.5, scan=0.2, size=0.02, density=140, spray=1.0, prand=1200, shape=0.2, spread=1.0, level=0.45, oct=2) \
    .add('b', 'Hollow', 'Glass', partials=24, oct=-1, shim=0.5, srate=0.04, level=0.45).off('c') \
    .f1('SVF HP12', 1800, res=0.2, key=0).f2('SVF LP12', 3000).route('split', 0.5).lfo(1, 0.7, 'sh', 'global', smooth=0.4).mod('lfo1', 'alevel', 0.3) \
    .aenv(4, 3, 1, 12).macro(1, 'Chirps').mod('m1', 'at3', 0.5).macro(2, 'Night Drone').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.15, 0.1, 0.4).width(1.8).tags('night', 'field', 'corbacho', 'organic')

P('Floating Stones', 'Atmospheres') \
    .ph('a', 'Bell', 'Strike', decay=9, damp=0.15, hard=0.55, couple=0.5, level=0.6) \
    .add('b', 'Custom', 'Bell', custom=STONE, partials=48, shim=0.5, srate=0.08, level=0.4) \
    .off('c').f1('SVF LP12', 7000, res=0.05, key=0).arp('random', 2, '1/4', 0.4, hold=True).aenv(0.01, 6, 0.2, 9) \
    .mseg(1, [(0, 0.0, 0), (16, 1.0, 0.3), (16, 0.0, -0.3)], loop=(0, 2), mode='free').mod('mseg1', 'at1', 0.4).mod('mseg1', 'bt1', 0.5) \
    .macro(1, 'Arp Rate').mod('m1', 'at3', 0.4).macro(2, 'Stone Layer').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.05, 0.4).tags('bells', 'arp', 'hold-a-chord', 'meditative')

P('Distant Engine', 'Atmospheres') \
    .an('a', 'saw', voices=4, detune=14, oct=-1, level=0.6, slop=0.5).an('b', 'pulse', pw=0.15, oct=-2, level=0.3).off('c') \
    .f1('Diode LP24', 400, res=0.5, drive=0.4, key=0.1).shifter(0.25, 0.3, 0.6, 1.0).lfo(1, 0.05, 'drift').lfo(2, 0.019, 'tri') \
    .mod('lfo1', 'f1cut', 0.35).mod('lfo2', 'fxshift', 0.01).aenv(6, 4, 1, 14).sub(0.2) \
    .macro(1, 'Doppler').mod('m1', 'fxshift', 0.03).macro(2, 'Motor').mod('m2', 'f1res', 0.3) \
    .nochorus().tags('dark', 'industrial', 'shifter')

P('Polar Night', 'Atmospheres') \
    .wt('a', 'Glass & Bell/Frost', pos=0.2, blend='spectral', voices=5, detune=8, level=0.6) \
    .add('b', 'Glass', 'Octaves', partials=48, shim=0.6, srate=0.05, oct=1, level=0.25).off('c') \
    .f1('Xpander HP2+LP1', 1800, res=0.3, key=0.1).lfo(1, 0.008, 'sine').lfo(2, 0.04, 'drift', 'voice').mod('lfo1', 'at1', 0.7).mod('lfo2', 'f1cut', 0.3) \
    .aenv(8, 4, 1, 20).macro(1, 'Frost').mod('m1', 'at1', 0.5).macro(2, 'Aurora').mod('m2', 'blevel', 0.5) \
    .chorus('ensemble', 0.3, 0.04, 0.6).width(1.5).tags('glacial', 'cold', 'corbacho')

P('Light Sculpture', 'Atmospheres') \
    .add('a', 'Custom', 'Glass', custom=LUMINOUS, morph=0.5, partials=64, shim=0.7, srate=0.2, stretch=0.1, pdet=8, level=0.6) \
    .add('b', 'Octaves', 'Fifths', morph=0.5, partials=48, shim=0.4, srate=0.07, oct=-1, level=0.4).off('c') \
    .f1('SVF LP12', 9000, res=0.05).lfo(1, 0.016, shape=RIPPLE).mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt1', -0.5).aenv(7, 3, 1, 16) \
    .macro(1, 'Stretch').mod('m1', 'at3', 0.3).macro(2, 'Prism').mod('m2', 'at2', 0.5) \
    .chorus('chorus', 0.2, 0.06, 0.4).tags('luminous', 'corbacho', 'additive')

P('Tidal Glass', 'Atmospheres') \
    .wt('a', 'Glass & Bell/Glass Harmonica', pos=0.3, blend='spectral', voices=3, detune=6, level=0.6) \
    .gr('b', 'keyed', sample='Metal/Bowed Bowl', pos=0.5, scan=0.05, size=0.5, density=16, spray=0.4, level=0.45).off('c') \
    .f1('SVF LP12', 6000, res=0.1).lfo(1, 0.03, shape=TIDE).mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt1', 0.4).mod('lfo1', 'alevel', -0.15) \
    .aenv(5, 3, 1, 14).macro(1, 'Tide').mod('m1', 'lfo1rate', 0.5).macro(2, 'Bowl').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.05, 0.4).tags('glass', 'water', 'corbacho')

P('Dust Light', 'Atmospheres') \
    .wt('a', 'Textures/Dust Spectrum', pos=0.5, warp='quantize', warpamt=0.3, voices=3, detune=10, level=0.55) \
    .gr('b', 'wt', lib='Textures/Breathing Haze', pos=0.5, scan=0.03, size=0.3, density=40, spray=0.6, prand=24, level=0.45).off('c') \
    .f1('SVF LP12', 5000, res=0.1).lfo(1, 0.4, 'sh', smooth=0.8).lfo(2, 0.02, 'sine').mod('lfo1', 'at2', 0.3).mod('lfo2', 'at1', 0.4) \
    .aenv(5, 3, 1, 14).macro(1, 'Grit').mod('m1', 'at2', 0.6).macro(2, 'Haze').mod('m2', 'blevel', 0.5) \
    .chorus('ensemble', 0.25, 0.08, 0.5).tags('lofi', 'textural', 'warp')

P('Ghost Valley', 'Atmospheres') \
    .gr('a', 'keyed', sample='Voices/Breath Choir', pos=0.5, scan=0.04, size=0.7, density=12, spray=0.5, spread=1.0, level=0.6) \
    .an('b', 'sine', oct=-1, level=0.3, slop=0.6).an('c', 'tri', semi=7, level=0.15, slop=0.6) \
    .f1('Formant', 900, res=0.5, shape=0.4, key=0).lfo(1, 0.011, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'f1shape', 0.3) \
    .aenv(8, 4, 1, 20).macro(1, 'Ghosts').mod('m1', 'alevel', 0.5).macro(2, 'Valley Floor').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.04, 0.5).width(1.5).tags('vocal', 'haunted', 'roach')

P('Moon Orbit', 'Atmospheres') \
    .add('a', 'Octaves', 'Glass', morph=0.3, partials=48, shim=0.4, srate=0.05, level=0.6) \
    .an('b', 'sine', oct=-1, level=0.3).an('c', 'sine', semi=19, level=0.12) \
    .f1('SVF LP12', 7000).lfo(1, 0.1, 'sine', sync=True, div='8 bars').lfo(2, 0.1, 'tri', sync=True, div='4 bars') \
    .mod('lfo1', 'apan', 0.6).mod('lfo2', 'cpan', -0.8).mod('lfo1', 'at1', 0.4).aenv(6, 3, 1, 16) \
    .macro(1, 'Orbit').mod('m1', 'pan', 0.5, via='lfo1').macro(2, 'Moon').mod('m2', 'clevel', 0.4) \
    .chorus('chorus', 0.15, 0.04, 0.3).tags('cosmic', 'tempo-synced', 'corbacho')

P('Mirage Heat', 'Atmospheres') \
    .wt('a', 'Spectral/Formant Glide', pos=0.4, blend='spectral', voices=3, detune=8, level=0.65) \
    .an('b', 'saw', oct=-1, level=0.3, slop=0.6).off('c').f1('SEM Morph', 1600, res=0.3, shape=0.2) \
    .tape(0.15, 0.7, 0.3, 0.6, 1.0).lfo(1, 0.03, 'drift').lfo(2, 0.07, 'sine', 'voice').mod('lfo1', 'f1shape', 0.4).mod('lfo2', 'at1', 0.3) \
    .aenv(5, 3, 1, 14).macro(1, 'Heat Shimmer').mod('m1', 'fxwow', 0.4).macro(2, 'SEM Morph').mod('m2', 'f1shape', 0.6) \
    .nochorus().tags('desert', 'tape', 'roach', 'warm')

P('Spectral Weather', 'Atmospheres') \
    .wt('a', 'Textures/Random Walk Spectrum 3', pos=0.5, blend='spectral', voices=4, detune=10, level=0.6) \
    .wt('b', 'Textures/Random Walk Spectrum 1', pos=0.2, blend='stepped', oct=-1, level=0.35).off('c') \
    .f1('Phaser 8', 900, res=0.5, shape=0.5).f2('SVF LP12', 6000).lfo(1, 0.6, 'sh').lfo(2, 0.02, 'sine') \
    .mod('lfo1', 'bt1', 0.4).mod('lfo2', 'at1', 0.6).mod('lfo2', 'f1cut', 0.4).aenv(5, 3, 1, 14) \
    .macro(1, 'Storm').mod('m1', 'bt1', 0.6).macro(2, 'Front').mod('m2', 'f1cut', 0.5) \
    .chorus('ensemble', 0.25, 0.06, 0.5).tags('evolving', 'textural', 'corbacho')

P('Vapor Trail', 'Atmospheres') \
    .gr('a', 'wt', lib='Vocal/Ethereal Ah', pos=0.2, scan=0.08, size=0.35, density=30, spray=0.5, prand=15, level=0.6) \
    .an('b', 'supersaw', pw=0.2, detune=20, level=0.25).off('c').f1('OB-Xa LP12', 3000, res=0.2, key=0.2) \
    .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'at4', 0.3).aenv(5, 3, 1, 15) \
    .macro(1, 'Vapor').mod('m1', 'at3', 0.5).macro(2, 'Trail').mod('m2', 'blevel', 0.5) \
    .chorus('solina', 0.3, 0.08, 0.5).tags('vocal', 'airy', 'corbacho')

P('Inner Sky', 'Atmospheres') \
    .add('a', 'Choir Oo', 'Glass', morph=0.4, partials=48, shim=0.4, srate=0.06, vowel=0.6, vowelamt=0.4, level=0.65) \
    .wt('b', 'Glass & Bell/Starfield', pos=0.5, blend='spectral', oct=1, voices=3, detune=10, level=0.25).off('c') \
    .f1('SVF LP12', 8000).mseg(1, [(0, 0.2, 0), (20, 0.9, 0.3), (20, 0.4, -0.2), (20, 0.2, 0.2)], loop=(0, 3), mode='free') \
    .mod('mseg1', 'at1', 0.5).mod('mseg1', 'at4', 0.4).mod('mseg1', 'blevel', 0.3).aenv(7, 3, 1, 18) \
    .macro(1, 'Stars').mod('m1', 'blevel', 0.4).macro(2, 'Vowel').mod('m2', 'at4', 0.5) \
    .chorus('ensemble', 0.3, 0.05, 0.5).tags('cosmic', 'choir', 'corbacho', 'luminous')

P('Cold Signal', 'Atmospheres') \
    .wt('a', 'Digital/Sine Ring Ratio', pos=0.3, warp='mirror', warpamt=0.4, level=0.5, voices=2, detune=5) \
    .an('b', 'sine', semi=12, level=0.0).fm('a', 'b', 0.25, 'fm').ring(0.2, 'ab').off('c') \
    .f1('Xpander BP2', 1500, res=0.4).lfo(1, 0.09, 'sh', 'global', smooth=0.3).lfo(2, 0.02, 'sine').mod('lfo1', 'f1cut', 0.4).mod('lfo2', 'afm', 0.3) \
    .aenv(4, 3, 1, 12).macro(1, 'Signal').mod('m1', 'afm', 0.5).macro(2, 'Mirror').mod('m2', 'at2', 0.5) \
    .crush(12, 2.5, 0.2).chorus('chorus', 0.2, 0.1, 0.4).tags('cold', 'digital', 'fm')

# =============================================================================================== PADS
P('Super Horizon', 'Pads') \
    .an('a', 'supersaw', pw=0.45, detune=28, level=0.7).an('b', 'supersaw', pw=0.3, detune=18, oct=-1, level=0.35).off('c') \
    .f1('OB-Xa LP24', 2400, res=0.15, key=0.4, env=0.2).fenv(2, 5, 0.6, 6).aenv(1.2, 3, 0.9, 6) \
    .lfo(1, 0.08, 'tri').mod('lfo1', 'at1', 0.15).mod('vel', 'f1cut', 0.2).mpe(pressCut=0.4) \
    .macro(1, 'Brightness').mod('m1', 'f1cut', 0.5).macro(2, 'Saw Mix').mod('m2', 'at1', 0.5).mod('m2', 'bt1', 0.5).macro(3, 'Octave').mod('m3', 'blevel', 0.5) \
    .chorus('ensemble', 0.3, 0.2, 0.5).tags('supersaw', 'trance', 'bright', 'wide')

P('Eight Voice Strings', 'Pads') \
    .an('a', 'saw', voices=2, detune=8, level=0.7, slop=0.45, drift=0.4).an('b', 'saw', fine=-6, voices=2, detune=6, level=0.6, slop=0.45).off('c') \
    .f1('SEM LP12', 2200, res=0.1, key=0.5, env=0.25).fenv(1.5, 3, 0.7, 5).aenv(1.2, 2, 0.9, 5) \
    .lfo(1, 4.8, 'sine', 'voice', fade=1.5).mod('lfo1', 'pitch', 0.004).mod('vel', 'f1cut', 0.15) \
    .macro(1, 'Bow Bite').mod('m1', 'f1cut', 0.4).macro(2, 'Vibrato').mod('m2', 'pitch', 0.01, via='lfo1').macro(3, 'Ensemble').mod('m3', 'fxchorus', 0.5) \
    .chorus('solina', 0.45, 0.2, 0.6).tags('strings', 'vintage', 'warm', 'solina')

P('Xpander Silk', 'Pads') \
    .an('a', 'sawpulse', pw=0.4, voices=3, detune=9, level=0.65, slop=0.35).wt('b', 'Strings & Reeds/Ensemble Soft', pos=0.4, voices=2, detune=6, level=0.4).off('c') \
    .f1('Xpander LP3', 1800, res=0.3, key=0.4, env=0.3).f2('Xpander N2', 2600, res=0.4).fenv(2, 4, 0.5, 6).aenv(1.5, 3, 0.9, 7) \
    .lfo(1, 0.12, 'tri').lfo(2, 0.35, 'sine', 'voice').mod('lfo1', 'f2cut', 0.4).mod('lfo2', 'at1', 0.2).mod('press', 'f1cut', 0.3) \
    .macro(1, 'Silk').mod('m1', 'f1cut', 0.4).macro(2, 'Notch Sweep').mod('m2', 'f2cut', 0.6).macro(3, 'Body').mod('m3', 'blevel', 0.5) \
    .chorus('ensemble', 0.35, 0.18, 0.5).tags('xpander', 'warm', 'vintage')

P('Matrix Velvet', 'Pads') \
    .an('a', 'pulse', pw=0.35, voices=2, detune=7, level=0.65, slop=0.4).an('b', 'saw', oct=-1, level=0.35, slop=0.4).off('c') \
    .f1('Xpander HP2+LP1', 1200, res=0.25, key=0.3).f2('Xpander LP4', 3000, res=0.2).fenv(3, 4, 0.6, 8).aenv(2, 3, 0.9, 8) \
    .lfo(1, 0.6, 'tri', 'voice').mod('lfo1', 'at1', 0.25).lfo(2, 0.05, 'drift').mod('lfo2', 'f1cut', 0.25) \
    .macro(1, 'Velvet').mod('m1', 'f2cut', -0.5).macro(2, 'PWM Depth').mod('m2', 'at1', 0.5, via='lfo1') \
    .chorus('ensemble', 0.3, 0.15, 0.5).tags('xpander', 'pwm', 'vintage')

P('Solina Dusk', 'Pads') \
    .an('a', 'saw', level=0.6, slop=0.2).an('b', 'saw', oct=1, level=0.35, slop=0.2).off('c') \
    .f1('SVF LP12', 3200, res=0.05, key=0.3).f2('SVF HP12', 180).aenv(0.9, 2, 1, 4) \
    .macro(1, 'Violin/Viola').mod('m1', 'blevel', -0.35).macro(2, 'Tone').mod('m2', 'f1cut', 0.4) \
    .chorus('solina', 0.6, 0.25, 0.7).tags('strings', 'solina', 'vintage', 'roach')

P('Morph Pad A>B', 'Pads') \
    .an('a', 'saw', voices=3, detune=9, level=0.65).wt('b', 'Vocal/Choir Aah to Ooh', pos=0.3, voices=2, detune=6, level=0.45).off('c') \
    .f1('Ladder LP24', 1600, res=0.35, key=0.4, morphto='Vowel', morph=0.0).fenv(2, 4, 0.6, 6).aenv(1.5, 3, 0.9, 7) \
    .mseg(1, [(0, 0.0, 0), (6, 1.0, 0.3), (6, 0.0, -0.3)], loop=(0, 2), mode='free').mod('mseg1', 'f1morph', 0.85).mod('mseg1', 'bt1', 0.3) \
    .macro(1, 'Morph Ladder>Vowel').mod('m1', 'f1morph', 0.8).macro(2, 'Morph Speed', 0.0).mod('m2', 'mseg1rate', 0.6) \
    .chorus('ensemble', 0.3, 0.1, 0.5).tags('morph', 'vocal', 'evolving')

P('FM Glass Pad', 'Pads') \
    .wt('a', table=0, pos=0.0, level=0.65, voices=3, detune=7).an('b', 'sine', oct=1, semi=7, level=0.0).fm('a', 'b', 0.3, 'pm') \
    .add('c', 'Glass', 'Bell', partials=32, shim=0.5, level=0.2, oct=1).f1('SVF LP12', 6000, res=0.05) \
    .menv(3, 5, 0.3, 6).mod('menv', 'afm', 0.4).lfo(1, 0.07, 'sine').mod('lfo1', 'afm', 0.15).aenv(1.5, 3, 0.85, 7) \
    .macro(1, 'Glass Index').mod('m1', 'afm', 0.5).macro(2, 'Chime').mod('m2', 'clevel', 0.5) \
    .chorus('chorus', 0.25, 0.15, 0.5).tags('fm', 'glass', 'bright')

P('Sync Choir Pad', 'Pads') \
    .an('a', 'saw', level=0.55, voices=3, detune=8).hardsync('a', 'b').set('oaAnSync', 9).an('b', 'sine', oct=-1, level=0.3) \
    .off('c').f1('Formant', 700, res=0.5, shape=0.5, key=0.3).f2('SEM LP12', 3000).aenv(1.5, 3, 0.9, 6) \
    .lfo(1, 0.09, 'tri').mod('lfo1', 'at2', 0.25).mod('lfo1', 'f1shape', 0.3).mod('slide', 'at2', 0.4) \
    .macro(1, 'Sync Sweep').mod('m1', 'at2', 0.5).macro(2, 'Vowel').mod('m2', 'f1shape', 0.6) \
    .chorus('ensemble', 0.3, 0.12, 0.5).tags('sync', 'vocal', 'mpe')

P('Warm Analog Bank', 'Pads') \
    .an('a', 'saw', voices=2, detune=6, level=0.6, slop=0.5, drift=0.5).an('b', 'pulse', pw=0.42, fine=5, level=0.5, slop=0.5, drift=0.5).off('c') \
    .f1('OB-Xa LP12', 1500, res=0.2, key=0.5, env=0.35).fenv(1.2, 3, 0.55, 4).aenv(0.8, 2, 0.9, 4) \
    .lfo(1, 0.25, 'tri', 'voice').mod('lfo1', 'bt1', 0.2).mod('vel', 'f1cut', 0.25) \
    .macro(1, 'Filter').mod('m1', 'f1cut', 0.5).macro(2, 'Env Amount').mod('m2', 'f1cut', 0.4, via='fenv').macro(3, 'Detune').mod('m3', 'detune', 0.3) \
    .chorus('chorus', 0.35, 0.3, 0.5).tags('analog', 'warm', 'vintage', 'polysynth')

P('Brass Section Dusk', 'Pads') \
    .an('a', 'saw', voices=2, detune=5, level=0.65, slop=0.3).an('b', 'saw', fine=-4, level=0.55, slop=0.3).off('c') \
    .f1('OB-Xa LP24', 700, res=0.15, key=0.5, env=0.6).fenv(0.25, 1.2, 0.4, 2).aenv(0.15, 1, 0.9, 2) \
    .lfo(1, 5.2, 'sine', 'voice', fade=1).mod('lfo1', 'pitch', 0.003).mod('vel', 'f1cut', 0.3).mpe(pressCut=0.5) \
    .macro(1, 'Blare').mod('m1', 'f1cut', 0.4).macro(2, 'Swell').mod('m2', 'f1cut', 0.4, via='press') \
    .chorus('chorus', 0.25, 0.3, 0.5).tags('brass', 'vintage', 'polysynth')

P('Spectral Strings', 'Pads') \
    .wt('a', 'Strings & Reeds/Viola Body', pos=0.2, blend='spectral', voices=3, detune=8, level=0.6) \
    .add('b', 'Strings', 'Saw', morph=0.3, partials=48, shim=0.25, level=0.35).off('c') \
    .f1('SEM LP12', 3000, res=0.1, key=0.4).lfo(1, 0.06, 'tri').lfo(2, 5, 'sine', 'voice', fade=2).mod('lfo1', 'at1', 0.4).mod('lfo2', 'pitch', 0.003) \
    .aenv(1.5, 3, 0.9, 6).macro(1, 'Bow Position').mod('m1', 'at1', 0.6).macro(2, 'Air').mod('m2', 'bt1', 0.5) \
    .chorus('solina', 0.35, 0.15, 0.5).tags('strings', 'spectral', 'warm')

P('Glacier Pad', 'Pads') \
    .wt('a', 'Glass & Bell/Frost', pos=0.5, blend='spectral', voices=4, detune=10, level=0.6) \
    .an('b', 'tri', oct=-1, level=0.3).off('c').f1('Xpander LP2', 4000, res=0.2, key=0.3).aenv(3, 3, 0.9, 10) \
    .lfo(1, 0.03, 'sine').mod('lfo1', 'at1', 0.4).macro(1, 'Thaw').mod('m1', 'at1', 0.5).macro(2, 'Depth').mod('m2', 'blevel', 0.4) \
    .chorus('ensemble', 0.3, 0.05, 0.5).width(1.4).tags('glacial', 'cold', 'corbacho')

P('Pressure Garden', 'Pads') \
    .add('a', 'Strings', 'Choir Ah', morph=0.3, partials=40, shim=0.2, level=0.6).an('b', 'saw', voices=2, detune=7, level=0.3).off('c') \
    .f1('SEM Morph', 1200, res=0.3, shape=0.0, key=0.4).aenv(1, 2, 0.9, 5) \
    .mod('press', 'f1shape', 0.8).mod('press', 'at1', 0.5).mod('slide', 'f1cut', 0.6).mod('bend', 'at2', 0.2).mpe(pressCut=0.2, slideCut=0.2, slideTimbre=0.3, bend=48) \
    .macro(1, 'LP > Notch > HP').mod('m1', 'f1shape', 0.9).macro(2, 'Strings/Choir').mod('m2', 'at1', 0.6) \
    .chorus('ensemble', 0.3, 0.12, 0.5).tags('mpe', 'expressive', 'sem')

P('Oberhaus Sunrise', 'Pads') \
    .an('a', 'saw', voices=3, detune=9, level=0.6, slop=0.4).an('b', 'saw', oct=-1, voices=2, detune=6, level=0.45, slop=0.4).off('c') \
    .f1('SEM LP12', 500, res=0.2, key=0.4, env=0.5).fenv(5, 6, 0.6, 8).aenv(3, 3, 0.9, 8) \
    .mseg(1, slow_swell(1.0, 10, 0.6, 16), loop=(2, 2), mode='env').mod('mseg1', 'f1cut', 0.4).mod('mseg1', 'detune', 0.2) \
    .macro(1, 'Sunrise').mod('m1', 'f1cut', 0.5).macro(2, 'Glow').mod('m2', 'detune', 0.3) \
    .chorus('ensemble', 0.35, 0.1, 0.5).tags('analog', 'warm', 'roach', 'swell')

P('Ash Cathedral', 'Pads') \
    .add('a', 'Organ', 'Choir Ah', morph=0.4, partials=56, shim=0.2, srate=0.05, level=0.6) \
    .wt('b', 'Additive/Cathedral Organ', pos=0.3, blend='spectral', voices=2, detune=5, level=0.35).off('c') \
    .f1('Xpander LP4', 2200, res=0.15, key=0.3).aenv(2.5, 3, 0.9, 9).lfo(1, 0.04, 'tri').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', 0.4) \
    .macro(1, 'Stops').mod('m1', 'bt1', 0.6).macro(2, 'Choir').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.25, 0.08, 0.4).tags('organ', 'choir', 'roach', 'sacred')

P('Night Bus Pad', 'Pads') \
    .an('a', 'sawpulse', pw=0.3, voices=3, detune=10, level=0.6).an('b', 'sine', oct=-1, level=0.35).off('c') \
    .f1('Diode LP24', 900, res=0.4, drive=0.3, key=0.4, env=0.2).fenv(1, 3, 0.5, 5).aenv(1.2, 3, 0.85, 6) \
    .lfo(1, 0.1, 'sine', sync=True, div='2 bars').mod('lfo1', 'f1cut', 0.3).tape(0.2, 0.2, 0.15, 0.55, 1.0) \
    .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Tape').mod('m2', 'fxwow', 0.5) \
    .chorus('chorus', 0.3, 0.2, 0.5).tags('lofi', 'tape', 'tempo-synced')

P('Prism Ensemble', 'Pads') \
    .wt('a', 'Glass & Bell/Prism', pos=0.4, blend='spectral', voices=5, detune=12, level=0.55) \
    .an('b', 'supersaw', pw=0.25, detune=16, level=0.3).off('c').f1('SVF LP24', 5000, res=0.15, key=0.3) \
    .lfo(1, 0.05, 'tri').mod('lfo1', 'at1', 0.5).aenv(2, 3, 0.9, 7).macro(1, 'Prism').mod('m1', 'at1', 0.5).macro(2, 'Saw Layer').mod('m2', 'blevel', 0.5) \
    .chorus('ensemble', 0.3, 0.1, 0.5).width(1.5).tags('bright', 'wide', 'glass')

P('Velour Granular Pad', 'Pads') \
    .gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.5, scan=0.02, size=0.25, density=40, spray=0.2, prand=6, level=0.6) \
    .an('b', 'saw', voices=3, detune=8, level=0.3).off('c').f1('SEM LP12', 2500, res=0.1, key=0.4) \
    .aenv(1.5, 3, 0.9, 6).lfo(1, 0.07, 'drift', 'voice').mod('lfo1', 'at1', 0.2) \
    .macro(1, 'Cloud Position').mod('m1', 'at1', 0.6).macro(2, 'Analog').mod('m2', 'blevel', 0.5) \
    .chorus('ensemble', 0.3, 0.1, 0.5).tags('granular-cloud', 'choir', 'warm')

P('Deep Field Strings', 'Pads') \
    .an('a', 'saw', voices=4, detune=11, level=0.6, slop=0.4).wt('b', 'Strings & Reeds/Cello Body', pos=0.3, oct=-1, level=0.4).off('c') \
    .f1('Ladder LP12', 1500, res=0.15, key=0.4).f2('Xpander HP1', 150).aenv(2.5, 3, 0.9, 10).lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'f1cut', 0.2) \
    .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Cello').mod('m2', 'blevel', 0.5) \
    .chorus('solina', 0.4, 0.1, 0.5).tags('strings', 'dark', 'roach')

# =============================================================================================== TEXTURES
P('FM Noise Lattice', 'Textures') \
    .wt('a', 'FM/FM 2-5 Index Sweep', pos=0.5, level=0.55).fm('a', 'noise', 0.25, 'fm').an('b', 'pulse', pw=0.1, oct=-1, level=0.3).off('c') \
    .f1('Comb -', 300, res=0.7, shape=0.4).lfo(1, 0.2, 'sh', smooth=0.5).mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'afm', 0.3) \
    .aenv(2, 3, 0.9, 8).macro(1, 'Noise FM').mod('m1', 'afm', 0.5).macro(2, 'Comb').mod('m2', 'f1res', 0.3) \
    .chorus('flanger', 0.3, 0.05, 0.6, 0.5).tags('noise', 'fm', 'industrial')

P('Bit Monastery', 'Textures') \
    .add('a', 'Choir Ah', 'Organ', morph=0.3, partials=40, shim=0.3, level=0.6).off('b', 'c').f1('SEM LP12', 3000) \
    .crush(6, 6, 0.45).lfo(1, 0.05, 'tri').mod('lfo1', 'fxcrush', 0.3).mod('lfo1', 'at1', 0.3).aenv(2, 3, 0.9, 10) \
    .macro(1, 'Bits').mod('m1', 'fxcrush', 0.5).macro(2, 'Choir').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.2, 0.1, 0.4).tags('lofi', 'choir', 'digital')

P('Frozen Breath', 'Textures') \
    .add('a', 'Custom', 'Choir Oo', custom=EMBER128, morph=0.5, partials=56, shim=0.6, srate=0.3, freeze=False, blur=2.5, level=0.6) \
    .noise(0.15, 0.4).off('b', 'c').f1('SVF BP12', 1400, res=0.3, key=0.2).lfo(1, 0.15, 'sh', smooth=0.6).mod('lfo1', 'at1', 0.5).mod('lfo1', 'f1cut', 0.3) \
    .aenv(3, 3, 0.9, 10).macro(1, 'Freeze Blur').mod('m1', 'at1', 0.3).macro(2, 'Breath').mod('m2', 'noise', 0.5) \
    .chorus('chorus', 0.2, 0.05, 0.4).tags('blur', 'spectral', 'corbacho')

P('Filter FM Hive', 'Textures') \
    .an('a', 'saw', voices=3, detune=12, level=0.55).an('b', 'sine', semi=-5, level=0.0).off('c') \
    .f1('Ladder BP24', 900, res=0.6, key=0.3, fmsrc='b', fmamt=0.45).lfo(1, 0.07, 'tri').lfo(2, 0.3, 'sh', smooth=0.5) \
    .mod('lfo1', 'f1fm', 0.4).mod('lfo2', 'f1cut', 0.3).aenv(2, 3, 0.9, 8) \
    .macro(1, 'Swarm').mod('m1', 'f1fm', 0.5).macro(2, 'Hive Size').mod('m2', 'f1cut', 0.5) \
    .chorus('ensemble', 0.3, 0.12, 0.5).tags('filter-fm', 'buzzing', 'organic')

P('Quantized Memory', 'Textures') \
    .wt('a', 'Analog/Saw Lowpass Sweep', pos=0.4, warp='quantize', warpamt=0.6, voices=2, detune=6, level=0.55) \
    .gr('b', 'wt', lib='Textures/Spectral Freeze', pos=0.4, size=0.08, density=60, spray=0.8, prand=1200, level=0.3).off('c') \
    .f1('SVF LP12', 4000).lfo(1, 0.1, 'sine', sync=True, div='1 bar').mod('lfo1', 'at2', 0.4).mod('lfo1', 'at1', 0.3) \
    .aenv(1, 3, 0.9, 6).macro(1, 'Quantize').mod('m1', 'at2', 0.5).macro(2, 'Dust').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.25, 0.1, 0.4).tags('digital', 'warp', 'lofi')

P('Reverse Rain', 'Textures') \
    .gr('a', 'fixed', sample='Metal/Rain on Tin', pos=0.5, scan=0.03, size=0.4, density=25, spray=0.9, rev=1.0, spread=1.0, level=0.6) \
    .gr('b', 'fixed', sample='Metal/Rain on Tin', pos=0.2, scan=-0.02, size=0.15, density=40, spray=0.9, rev=0.0, oct=1, level=0.3).off('c') \
    .f1('SVF LP12', 6000).aenv(3, 3, 1, 10).lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.3) \
    .macro(1, 'Reverse/Forward').mod('m1', 'alevel', -0.5).mod('m1', 'blevel', 0.5).macro(2, 'Downpour').mod('m2', 'at3', 0.5) \
    .chorus('chorus', 0.2, 0.05, 0.4).tags('rain', 'reverse', 'granular-cloud', 'field')

P('Tape Ghosts', 'Textures') \
    .gr('a', 'keyed', sample='Voices/Throat Chant', pos=0.4, scan=0.05, size=0.5, density=14, spray=0.6, rev=0.3, level=0.6) \
    .an('b', 'tri', oct=-1, level=0.25, slop=0.6).off('c').f1('SEM BP12', 1200, res=0.2, key=0.2) \
    .tape(0.35, 0.6, 0.4, 0.45, 1.0).lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.3).aenv(4, 3, 1, 12) \
    .macro(1, 'Tape Age').mod('m1', 'fxwow', 0.5).macro(2, 'Ghost Voice').mod('m2', 'alevel', 0.4) \
    .nochorus().tags('tape', 'lofi', 'haunted', 'roach')

P('Wavefold Embers', 'Textures') \
    .wt('a', 'Digital/Wavefolder Sine', pos=0.4, blend='spectral', level=0.55).an('b', 'tri', oct=-1, level=0.3).off('c') \
    .f1('Ladder LP24', 1400, res=0.3, drive=0.6, dtype='fold', key=0.3).lfo(1, 0.04, 'sine').lfo(2, 0.13, 'drift') \
    .mod('lfo1', 'at1', 0.5).mod('lfo2', 'f1drive', 0.3).aenv(2, 3, 0.9, 8) \
    .macro(1, 'Fold').mod('m1', 'f1drive', 0.5).macro(2, 'Embers').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.2, 0.1, 0.4).tags('wavefold', 'warm', 'gritty')

P('Mirror Engine', 'Textures') \
    .wt('a', 'Phase Distortion/PD Saw Resonance', pos=0.3, warp='mirror', warpamt=0.5, voices=3, detune=8, level=0.55) \
    .wt('b', 'Phase Distortion/PD Pulse', pos=0.6, warp='asym', warpamt=0.4, oct=-1, level=0.35).off('c') \
    .f1('Xpander PH3', 1000, res=0.5).f2('Xpander LP4', 4000).lfo(1, 0.06, 'tri').lfo(2, 0.09, 'sine', 'voice') \
    .mod('lfo1', 'at2', 0.4).mod('lfo2', 'bt2', 0.4).mod('lfo1', 'f1cut', 0.4).aenv(2, 3, 0.9, 8) \
    .macro(1, 'Mirror').mod('m1', 'at2', 0.5).macro(2, 'Asym').mod('m2', 'bt2', 0.5).macro(3, 'Phase Notch').mod('m3', 'f1cut', 0.5) \
    .chorus('ensemble', 0.25, 0.1, 0.5).tags('warp', 'phase-distortion', 'digital')

P('Cicada Grid', 'Textures') \
    .an('a', 'pulse', pw=0.1, level=0.4).an('b', 'sine', semi=31, level=0.0).fm('a', 'b', 0.4, 'am').off('c') \
    .f1('SVF BP12', 4500, res=0.5, key=0).lfo(1, 0.1, shape=STAIRS4, sync=True, div='1/4').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'afm', 0.3) \
    .aenv(1, 2, 1, 4).macro(1, 'Buzz').mod('m1', 'afm', 0.5).macro(2, 'Pitch').mod('m2', 'f1cut', 0.5) \
    .chorus('chorus', 0.2, 0.2, 0.5).width(1.8).tags('insect', 'tempo-synced', 'organic')

P('Shortwave Dreams', 'Textures') \
    .wt('a', 'Textures/Shortwave', pos=0.4, blend='stepped', voices=2, detune=10, level=0.5).noise(0.12, 0.8).off('b', 'c') \
    .f1('SVF BP12', 1800, res=0.5, key=0.0).shifter(3.5, 0.4, 0.3, 0.5).lfo(1, 0.9, 'sh').lfo(2, 0.03, 'drift') \
    .mod('lfo1', 'at1', 0.3).mod('lfo2', 'f1cut', 0.5).aenv(2, 3, 0.9, 8) \
    .macro(1, 'Tuning').mod('m1', 'fxshift', 0.1).macro(2, 'Static').mod('m2', 'noise', 0.5) \
    .nochorus().tags('radio', 'lofi', 'shifter')

P('Bowed Plate Stutter', 'Textures') \
    .ph('a', 'Plate', 'Bow', decay=6, damp=0.2, hard=0.6, couple=0.4, level=0.6).off('b', 'c').f1('SVF LP12', 7000) \
    .lfo(1, 0.1, shape=TRIBAL, sync=True, div='1/2').mod('lfo1', 'amp', 0.35).mod('lfo1', 'at2', 0.3).aenv(1, 3, 1, 8) \
    .macro(1, 'Stutter').mod('m1', 'amp', 0.4, via='lfo1').macro(2, 'Pressure').mod('m2', 'at2', 0.5) \
    .chorus('chorus', 0.2, 0.06, 0.4).tags('metallic', 'rhythmic', 'tempo-synced')

P('Particle Choir', 'Textures') \
    .gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.4, scan=0.1, size=0.03, density=120, spray=0.9, prand=60, shape=0.2, level=0.55) \
    .off('b', 'c').f1('SVF LP12', 7000).lfo(1, 0.04, 'drift').mod('lfo1', 'at2', 0.4).mod('lfo1', 'at3', 0.3).aenv(2, 3, 1, 10) \
    .macro(1, 'Grain Size').mod('m1', 'at2', 0.6).macro(2, 'Density').mod('m2', 'at3', 0.5) \
    .chorus('ensemble', 0.3, 0.08, 0.5).width(1.6).tags('granular-cloud', 'choir', 'particles')

P('Stone Loom', 'Textures') \
    .add('a', 'Custom', 'Primes', custom=STONE, morph=0.3, partials=64, comb=0.5, cshift=0.2, shim=0.4, level=0.55).off('b', 'c') \
    .f1('Comb +', 180, res=0.6, shape=0.6).lfo(1, 0.02, 'sine').lfo(2, 0.05, 'tri').mod('lfo1', 'at1', 0.4).mod('lfo2', 'f1cut', 0.3) \
    .aenv(3, 3, 0.9, 10).macro(1, 'Comb Shift').mod('m1', 'f1cut', 0.5).macro(2, 'Weave').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.2, 0.06, 0.4).tags('comb', 'metallic', 'corbacho')

# =============================================================================================== CHOIRS & VOICES
P('Vowel Choir AEIOU', 'Choirs & Voices') \
    .add('a', 'Choir Ah', 'Choir Oo', morph=0.2, partials=64, shim=0.25, pdet=6, vowel=0.0, vowelamt=0.8, level=0.7) \
    .an('b', 'saw', voices=3, detune=10, level=0.15).off('c').f1('SEM LP12', 4000) \
    .mseg(1, [(0, 0.0, 0), (4, 0.25, 0), (4, 0.5, 0), (4, 0.75, 0), (4, 1.0, 0), (6, 0.0, 0.3)], loop=(0, 5), mode='free').mod('mseg1', 'at4', 0.9) \
    .aenv(1.5, 3, 0.9, 6).mod('slide', 'at4', 0.6).macro(1, 'Vowel').mod('m1', 'at4', 0.8).macro(2, 'Vowel Cycle', 0.0).mod('m2', 'mseg1rate', 0.5) \
    .chorus('ensemble', 0.3, 0.1, 0.5).tags('choir', 'vowel', 'mpe', 'roach')

P('Monks Below', 'Choirs & Voices') \
    .gr('a', 'keyed', sample='Voices/Throat Chant', pos=0.3, scan=0.03, size=0.6, density=14, spray=0.3, oct=-1, level=0.7) \
    .add('b', 'Throat', 'Choir Ah', morph=0.3, partials=48, peak=10, peakamt=0.4, width=0.1, oct=-1, level=0.4).off('c') \
    .f1('Vowel', 600, res=0.5, shape=0.75, key=0).lfo(1, 0.02, 'drift').mod('lfo1', 'f1shape', 0.2).mod('lfo1', 'bt2', 0.3).aenv(3, 3, 1, 12) \
    .macro(1, 'Overtones').mod('m1', 'bt2', 0.5).macro(2, 'Chant Grains').mod('m2', 'alevel', 0.4) \
    .chorus('chorus', 0.15, 0.05, 0.3).tags('choir', 'throat', 'roach', 'sacred')

P('Angelic Grains', 'Choirs & Voices') \
    .gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.7, scan=0.04, size=0.35, density=30, spray=0.3, prand=8, level=0.7) \
    .add('b', 'Choir Oo', 'Glass', morph=0.2, partials=32, shim=0.4, oct=1, level=0.2).off('c').f1('SVF LP12', 9000) \
    .aenv(2, 3, 0.9, 8).lfo(1, 0.05, 'sine').mod('lfo1', 'at1', 0.2) \
    .macro(1, 'Heaven').mod('m1', 'blevel', 0.5).macro(2, 'Choir Position').mod('m2', 'at1', 0.6) \
    .chorus('ensemble', 0.3, 0.08, 0.5).tags('choir', 'bright', 'corbacho', 'granular-cloud')

P('Breath Choir Pad', 'Choirs & Voices') \
    .gr('a', 'keyed', sample='Voices/Breath Choir', pos=0.4, size=0.4, density=24, spray=0.4, level=0.6) \
    .add('b', 'Choir Ah', 'Choir Oo', morph=0.5, partials=40, vowel=0.4, vowelamt=0.5, level=0.45).off('c').f1('SEM LP12', 3500) \
    .aenv(2, 3, 0.9, 8).lfo(1, 0.04, 'tri').mod('lfo1', 'bt4', 0.4) \
    .macro(1, 'Breath').mod('m1', 'alevel', 0.5).macro(2, 'Vowel').mod('m2', 'bt4', 0.6) \
    .chorus('solina', 0.3, 0.1, 0.5).tags('choir', 'breathy')

P('Formant Lead Voice', 'Choirs & Voices') \
    .an('a', 'saw', level=0.65, slop=0.2).off('b', 'c').f1('Vowel', 900, res=0.7, shape=0.0, key=0.3).f2('SEM LP12', 4000) \
    .lfo(1, 5.5, 'sine', 'voice', fade=0.8).mod('lfo1', 'pitch', 0.004).mod('slide', 'f1shape', 0.9).mod('press', 'f1cut', 0.3) \
    .voice('legato', 0.08).aenv(0.1, 1, 0.9, 1.5).mpe(bend=48) \
    .macro(1, 'Vowel').mod('m1', 'f1shape', 0.9).macro(2, 'Vibrato').mod('m2', 'pitch', 0.008, via='lfo1') \
    .chorus('chorus', 0.2, 0.2, 0.4).tags('vocal', 'lead', 'mpe', 'mono')

P('Tibetan Bass Choir', 'Choirs & Voices') \
    .add('a', 'Throat', 'Choir Oo', morph=0.2, partials=48, peak=12, peakamt=0.7, width=0.07, oct=-1, level=0.75) \
    .an('b', 'saw', oct=-2, level=0.2).off('c').f1('Formant', 260, res=0.55, key=0, shape=0.6) \
    .lfo(1, 0.03, 'drift').mod('lfo1', 'at2', 0.2).aenv(2, 3, 1, 10).sub(0.25) \
    .macro(1, 'Overtone Peak').mod('m1', 'at2', 0.5).macro(2, 'Growl').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.1, 0.05, 0.3).tags('throat', 'choir', 'dark', 'roach')

P('Whisper Field', 'Choirs & Voices') \
    .wt('a', 'Vocal/Whisper Air', pos=0.3, blend='spectral', voices=3, detune=8, level=0.55) \
    .gr('b', 'keyed', sample='Voices/Breath Choir', pos=0.5, size=0.2, density=40, spray=0.8, prand=30, level=0.4).off('c') \
    .f1('SVF HP12', 300).f2('SVF LP12', 8000).lfo(1, 0.04, 'sine').mod('lfo1', 'at1', 0.5).aenv(3, 3, 1, 10) \
    .macro(1, 'Whisper').mod('m1', 'at1', 0.5).macro(2, 'Breath Grains').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.06, 0.4).width(1.6).tags('vocal', 'airy', 'corbacho')

P('Ritual Chant', 'Choirs & Voices') \
    .add('a', 'Choir Ah', 'Throat', morph=0.4, partials=48, vowel=0.3, vowelamt=0.6, peak=8, peakamt=0.3, level=0.7) \
    .off('b', 'c').f1('SEM LP12', 3000).lfo(1, 0.1, shape=TRIBAL, sync=True, div='2 bars').mod('lfo1', 'at4', 0.4).mod('lfo1', 'at2', 0.2) \
    .aenv(1, 3, 0.9, 6).macro(1, 'Chant Rhythm').mod('m1', 'at4', 0.5, via='lfo1').macro(2, 'Throat').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.15, 0.06, 0.3).tags('ritual', 'tribal', 'roach', 'tempo-synced')

# =============================================================================================== BELLS & RESONATORS
P('Coupled Bell Garden', 'Bells & Resonators') \
    .ph('a', 'Bell', 'Strike', decay=7, damp=0.15, hard=0.6, couple=0.7, level=0.7).off('b', 'c').f1('SVF LP12', 9000) \
    .aenv(0.002, 6, 0, 8).mpe(velAmp=0.6).mod('vel', 'at2', 0.4).mod('rand', 'at1', 0.2) \
    .macro(1, 'Coupling Beat').mod('m1', 'at3', 0.3).macro(2, 'Hardness').mod('m2', 'at2', 0.5) \
    .chorus('chorus', 0.15, 0.1, 0.3).tags('bells', 'beating', 'meditative')

P('FM Temple Bell', 'Bells & Resonators') \
    .wt('a', table=0, pos=0.0, level=0.6).an('b', 'sine', semi=-5, oct=2, level=0.0).fm('a', 'b', 0.6, 'pm').off('c') \
    .menv(0.002, 4, 0, 6).mod('menv', 'afm', 0.4).aenv(0.002, 7, 0, 8).f1('SVF LP12', 10000).mpe(velAmp=0.6).mod('vel', 'afm', 0.3) \
    .macro(1, 'Brightness').mod('m1', 'afm', 0.4).macro(2, 'Ratio Detune').mod('m2', 'bpitch', 0.02) \
    .chorus('chorus', 0.15, 0.1, 0.3).tags('fm', 'bells', 'dx')

P('Shifted Bells', 'Bells & Resonators') \
    .ph('a', 'Bell', 'Strike', decay=6, damp=0.2, hard=0.65, level=0.65).add('b', 'Bell', 'Glass', partials=32, oct=1, level=0.2).off('c') \
    .shifter(7.0, 0.35, 0.5, 0.5).aenv(0.002, 5, 0, 7).mpe(velAmp=0.6) \
    .macro(1, 'Shift').mod('m1', 'fxshift', 0.05).macro(2, 'Inharmonic').mod('m2', 'fxshiftmix', 0.4) \
    .nochorus().tags('bells', 'shifter', 'inharmonic')

P('Rain Bells', 'Bells & Resonators') \
    .ph('a', 'Bell', 'Rain', decay=4, damp=0.25, hard=0.5, spread=1.0, level=0.6).ph('b', 'Bar', 'Rain', decay=3, damp=0.3, hard=0.4, oct=1, level=0.35).off('c') \
    .aenv(2, 3, 1, 8).f1('SVF LP12', 9000).lfo(1, 0.02, 'drift').mod('lfo1', 'at2', 0.3) \
    .macro(1, 'Density').mod('m1', 'at2', 0.5).mod('m1', 'bt2', 0.5).macro(2, 'Bars').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.15, 0.05, 0.3).tags('bells', 'rain', 'corbacho', 'generative')

P('Tanpura Pluck', 'Bells & Resonators') \
    .ph('a', 'Tanpura', 'Strike', decay=9, damp=0.06, hard=0.8, buzz=0.7, level=0.75).off('b', 'c').aenv(0.002, 5, 0.5, 6) \
    .mpe(velAmp=0.5).mod('vel', 'at2', 0.3).macro(1, 'Jawari').mod('m1', 'at4', -0.3).macro(2, 'Pluck').mod('m2', 'at2', 0.4) \
    .chorus('chorus', 0.1, 0.05, 0.2).tags('plucked', 'indian', 'strings')

P('Scraped Gong', 'Bells & Resonators') \
    .ph('a', 'Plate', 'Scrape', decay=12, damp=0.1, hard=0.4, couple=0.6, oct=-1, level=0.7).off('b', 'c').f1('SVF LP12', 8000) \
    .aenv(1, 4, 1, 10).lfo(1, 0.05, 'drift').mod('lfo1', 'at2', 0.4) \
    .macro(1, 'Scrape Rate').mod('m1', 'at2', 0.5).macro(2, 'Size').mod('m2', 'apitch', -0.1) \
    .chorus('chorus', 0.15, 0.04, 0.3).tags('gong', 'metallic', 'roach')

P('Sympathetic Harp', 'Bells & Resonators') \
    .ph('a', 'Sympathetic', 'Strike', decay=8, damp=0.15, hard=0.7, buzz=0.8, level=0.7).off('b', 'c').aenv(0.002, 5, 0.4, 8) \
    .mpe(velAmp=0.5).macro(1, 'Sympathy').mod('m1', 'at4', -0.3).macro(2, 'Brightness').mod('m2', 'at2', 0.4) \
    .chorus('chorus', 0.15, 0.08, 0.3).tags('plucked', 'harp', 'resonant')

P('Bowl Arpeggio', 'Bells & Resonators') \
    .ph('a', 'Glass Bowl', 'Strike', decay=10, damp=0.1, hard=0.45, couple=0.4, level=0.65).off('b', 'c') \
    .arp('updown', 2, '1/8', 0.3, hold=True).aenv(0.002, 6, 0.3, 8).f1('SVF LP12', 9000) \
    .macro(1, 'Damping').mod('m1', 'at4', -0.3).macro(2, 'Strike Position').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.15, 0.06, 0.3).tags('arp', 'bowl', 'meditative', 'hold-a-chord')

P('Glass Rods', 'Bells & Resonators') \
    .ph('a', 'Bar', 'Bow', decay=8, damp=0.15, hard=0.5, couple=0.3, level=0.65).add('b', 'Glass', 'Primes', partials=40, oct=1, shim=0.4, level=0.2).off('c') \
    .aenv(1, 3, 0.9, 8).mod('press', 'at2', 0.5).macro(1, 'Bow Pressure').mod('m1', 'at2', 0.5).macro(2, 'Halo').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.05, 0.4).tags('bowed', 'glass', 'mpe')

P('Monastery Chimes', 'Bells & Resonators') \
    .ph('a', 'Tube', 'Strike', decay=7, damp=0.15, hard=0.6, level=0.6).ph('b', 'Bell', 'Strike', decay=6, damp=0.2, hard=0.55, semi=7, level=0.35).off('c') \
    .arp('random', 2, '1/4', 0.5, hold=True).aenv(0.002, 5, 0.2, 7).f1('SVF LP12', 9000) \
    .macro(1, 'Tubes/Bells').mod('m1', 'blevel', 0.5).macro(2, 'Tone').mod('m2', 'at2', 0.4) \
    .chorus('chorus', 0.15, 0.05, 0.3).tags('chimes', 'arp', 'generative', 'hold-a-chord')

# =============================================================================================== KEYS & ORGANS
P('FM Rhodes Dusk', 'Keys & Organs') \
    .wt('a', table=0, pos=0.0, level=0.6).an('b', 'sine', oct=1, level=0.0).fm('a', 'b', 0.35, 'pm').an('c', 'sine', oct=-1, level=0.2).off() \
    .menv(0.002, 1.5, 0.15, 1).mod('menv', 'afm', 0.4).mod('vel', 'afm', 0.4).aenv(0.002, 3, 0.4, 1.5).f1('SVF LP12', 7000) \
    .lfo(1, 4, 'sine').mod('lfo1', 'apan', 0.3).mpe(velAmp=0.6) \
    .macro(1, 'Tine').mod('m1', 'afm', 0.4).macro(2, 'Tremolo').mod('m2', 'amp', 0.2, via='lfo1') \
    .chorus('chorus', 0.2, 0.3, 0.4).tags('fm', 'electric-piano', 'keys')

P('Drawbar Fifths', 'Keys & Organs') \
    .add('a', 'Organ', 'Custom', custom=ORGAN_FIFTHS, morph=0.5, partials=48, shim=0.0, level=0.65).off('b', 'c') \
    .aenv(0.01, 1, 1, 0.3).f1('Off', 20000).lfo(1, 6, 'sine').mod('lfo1', 'apan', 0.3) \
    .macro(1, 'Fifth Bars').mod('m1', 'at1', 0.5).macro(2, 'Leslie').mod('m2', 'fxchorus', 0.5) \
    .chorus('chorus', 0.25, 0.8, 0.4).tags('organ', 'drawbar')

P('Reed Harmonium Air', 'Keys & Organs') \
    .an('a', 'pulse', pw=0.25, level=0.55, slop=0.2).an('b', 'saw', fine=8, level=0.45).noise(0.05, 0.2).off('c') \
    .f1('Formant', 700, res=0.3, shape=0.3, key=0.4).aenv(0.15, 1, 1, 0.6).lfo(1, 0.3, 'tri').mod('lfo1', 'amp', 0.06) \
    .macro(1, 'Bellows').mod('m1', 'amp', 0.15, via='lfo1').macro(2, 'Reed').mod('m2', 'f1shape', 0.5) \
    .chorus('chorus', 0.2, 0.2, 0.3).tags('harmonium', 'reed', 'roach')

P('Ice Celesta', 'Keys & Organs') \
    .wt('a', 'Glass & Bell/Celesta Tines', pos=0.3, level=0.6).ph('b', 'Bar', 'Strike', decay=3, damp=0.3, hard=0.7, oct=1, level=0.25).off('c') \
    .aenv(0.002, 3, 0.2, 3).f1('SVF LP12', 9000).mpe(velAmp=0.6) \
    .macro(1, 'Bars').mod('m1', 'blevel', 0.4).macro(2, 'Frame').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.2, 0.15, 0.3).tags('celesta', 'keys', 'bright')

P('Sync Clav', 'Keys & Organs') \
    .an('a', 'pulse', pw=0.3, sync=12, level=0.55).off('b', 'c').f1('Ladder HP12', 300, res=0.2).f2('SVF LP12', 5000) \
    .menv(0.002, 0.5, 0, 0.3).mod('menv', 'at2', 0.4).aenv(0.002, 0.8, 0.3, 0.3).mpe(velAmp=0.7).mod('vel', 'at2', 0.3) \
    .macro(1, 'Sync').mod('m1', 'at2', 0.5).macro(2, 'Bite').mod('m2', 'f2cut', 0.4) \
    .chorus('chorus', 0.15, 0.3, 0.3).tags('clav', 'sync', 'funky')

# =============================================================================================== BASSES
P('Sync Sub Bass', 'Basses') \
    .an('a', 'saw', oct=-1, level=0.6).hardsync('a', 'b').set('oaAnSync', 7).an('b', 'sine', oct=-2, level=0.45).off('c') \
    .f1('Ladder LP24', 600, res=0.3, key=0.4, env=0.4).fenv(0.002, 0.4, 0.3, 0.3).aenv(0.002, 0.5, 0.9, 0.3) \
    .menv(0.002, 0.3, 0.2, 0.3).mod('menv', 'at2', 0.4).voice('mono', 0.03).mpe(velAmp=0.4) \
    .macro(1, 'Sync').mod('m1', 'at2', 0.5).macro(2, 'Filter').mod('m2', 'f1cut', 0.4) \
    .nochorus().tags('bass', 'sync', 'mono')

P('FM Pluck Bass', 'Basses') \
    .wt('a', table=0, pos=0.0, oct=-1, level=0.6).an('b', 'sine', oct=-1, level=0.0).fm('a', 'b', 0.5, 'pm').off('c') \
    .menv(0.002, 0.25, 0.1, 0.2).mod('menv', 'afm', 0.5).aenv(0.002, 0.6, 0.6, 0.2).f1('SVF LP12', 4000).voice('mono', 0.02).sub(0.3) \
    .macro(1, 'Pluck').mod('m1', 'afm', 0.4).macro(2, 'Sub').mod('m2', 'sub', 0.4) \
    .nochorus().tags('bass', 'fm', 'mono')

P('OB Bass Throb', 'Basses') \
    .an('a', 'saw', oct=-1, level=0.6, slop=0.3).an('b', 'pulse', pw=0.4, oct=-1, fine=-8, level=0.5).off('c') \
    .f1('OB-Xa LP24', 350, res=0.45, drive=0.3, key=0.4, env=0.45).fenv(0.002, 0.6, 0.25, 0.4).aenv(0.002, 0.8, 0.85, 0.3) \
    .lfo(1, 0.1, 'sine', sync=True, div='1/8', mode='retrig').mod('lfo1', 'f1cut', 0.15).voice('mono', 0.04).mpe(velAmp=0.4) \
    .macro(1, 'Throb').mod('m1', 'f1cut', 0.3, via='lfo1').macro(2, 'Cutoff').mod('m2', 'f1cut', 0.4) \
    .nochorus().tags('bass', 'analog', 'mono', 'tempo-synced')

P('Xpander Notch Bass', 'Basses') \
    .an('a', 'sawpulse', pw=0.35, oct=-1, level=0.6).off('b', 'c').sub(0.3) \
    .f1('Xpander N2+LP1', 500, res=0.5, key=0.5, env=0.5).fenv(0.002, 0.5, 0.3, 0.3).aenv(0.002, 0.7, 0.85, 0.3).voice('legato', 0.05) \
    .mod('press', 'f1cut', 0.4).macro(1, 'Notch').mod('m1', 'f1cut', 0.4).macro(2, 'Weight').mod('m2', 'sub', 0.4) \
    .nochorus().tags('bass', 'xpander', 'mono', 'mpe')

P('Granite Drone Bass', 'Basses') \
    .an('a', 'supersaw', pw=0.25, detune=14, oct=-2, level=0.55).an('b', 'sine', oct=-2, level=0.5).off('c') \
    .f1('Diode LP24', 180, res=0.35, drive=0.5, key=0.2).aenv(0.5, 2, 1, 3).lfo(1, 0.07, 'sine').mod('lfo1', 'f1cut', 0.3) \
    .macro(1, 'Grind').mod('m1', 'f1drive', 0.5).macro(2, 'Open').mod('m2', 'f1cut', 0.5) \
    .nochorus().tags('bass', 'drone', 'dark', 'roach')

P('Tube Bass', 'Basses') \
    .ph('a', 'Tube', 'Breath', decay=3, damp=0.4, hard=0.4, oct=-1, level=0.7).an('b', 'sine', oct=-2, level=0.35).off('c') \
    .f1('SVF LP12', 2000, res=0.1).aenv(0.05, 1, 0.9, 0.5).mod('press', 'at2', 0.4).voice('mono', 0.05) \
    .macro(1, 'Breath').mod('m1', 'at2', 0.5).macro(2, 'Sub').mod('m2', 'blevel', 0.4) \
    .nochorus().tags('bass', 'physical', 'woody')

P('Ring Mod Bass', 'Basses') \
    .an('a', 'saw', oct=-1, level=0.5).an('b', 'sine', oct=-1, semi=7, level=0.2).ring(0.35, 'ab').off('c') \
    .f1('MS-20 LP12', 800, res=0.55, key=0.4, env=0.4).fenv(0.002, 0.4, 0.3, 0.3).aenv(0.002, 0.6, 0.85, 0.3).voice('mono', 0.02) \
    .macro(1, 'Ring').mod('m1', 'ring', 0.5).macro(2, 'Scream').mod('m2', 'f1res', 0.3) \
    .nochorus().tags('bass', 'ring-mod', 'mono')

P('Pressure Sub', 'Basses') \
    .an('a', 'tri', oct=-1, level=0.6, slop=0.3).an('b', 'sine', oct=-2, level=0.5).off('c').f1('SEM LP12', 900, res=0.2, key=0.3) \
    .aenv(0.01, 1, 1, 0.5).mod('press', 'f1cut', 0.6).mod('press', 'at1', 0.3).mod('slide', 'f1res', 0.4).mpe(bend=24).voice('mono', 0.03) \
    .macro(1, 'Warmth').mod('m1', 'f1cut', 0.4).macro(2, 'Sub').mod('m2', 'blevel', 0.4) \
    .nochorus().tags('bass', 'mpe', 'sub')

# =============================================================================================== LEADS
P('Supersaw Lead', 'Leads') \
    .an('a', 'supersaw', pw=0.6, detune=20, level=0.6).off('b', 'c').f1('OB-Xa LP24', 3500, res=0.2, key=0.4, env=0.2) \
    .fenv(0.01, 1, 0.6, 0.5).aenv(0.01, 1, 0.9, 0.6).lfo(1, 5.5, 'sine', 'voice', fade=0.5).mod('lfo1', 'pitch', 0.004).voice('legato', 0.06) \
    .macro(1, 'Width').mod('m1', 'adetune', 0.4).macro(2, 'Vibrato').mod('m2', 'pitch', 0.01, via='lfo1') \
    .chorus('ensemble', 0.25, 0.3, 0.5).tags('lead', 'supersaw', 'mono')

P('Glide Sync Lead', 'Leads') \
    .an('a', 'saw', level=0.6).hardsync('a', 'b').set('oaAnSync', 5).an('b', 'saw', oct=-1, level=0.3).off('c') \
    .f1('Ladder LP24', 2500, res=0.25, key=0.4).menv(0.5, 2, 0.5, 1).mod('menv', 'at2', 0.5).mod('slide', 'at2', 0.4).voice('legato', 0.12).aenv(0.01, 1, 0.9, 0.8) \
    .macro(1, 'Sync').mod('m1', 'at2', 0.5).macro(2, 'Glide').mod('m2', 'f1cut', 0.4) \
    .chorus('chorus', 0.2, 0.3, 0.4).tags('lead', 'sync', 'mono', 'mpe')

P('Bowed Lead', 'Leads') \
    .ph('a', 'String', 'Bow', decay=4, damp=0.3, hard=0.5, level=0.7).off('b', 'c').f1('SVF LP12', 6000) \
    .mod('press', 'at2', 0.5).mod('slide', 'at1', 0.4).lfo(1, 5.2, 'sine', 'voice', fade=1).mod('lfo1', 'pitch', 0.004).voice('legato', 0.08).aenv(0.1, 1, 1, 1) \
    .mpe(bend=48).macro(1, 'Bow Pressure').mod('m1', 'at2', 0.5).macro(2, 'Vibrato').mod('m2', 'pitch', 0.008, via='lfo1') \
    .chorus('chorus', 0.15, 0.2, 0.3).tags('lead', 'bowed', 'mpe', 'expressive')

P('Shakuhachi Wind', 'Leads') \
    .ph('a', 'Tube', 'Breath', decay=2, damp=0.35, hard=0.45, level=0.7).noise(0.08, 0.4).off('b', 'c').f1('SVF LP12', 5000) \
    .mod('press', 'at2', 0.5).mod('press', 'noise', 0.3).lfo(1, 4.5, 'sine', 'voice', fade=1.2).mod('lfo1', 'pitch', 0.005).voice('legato', 0.06).aenv(0.15, 1, 1, 0.8) \
    .macro(1, 'Breath').mod('m1', 'noise', 0.5).macro(2, 'Overblow').mod('m2', 'at2', 0.5) \
    .chorus('chorus', 0.1, 0.1, 0.2).tags('lead', 'flute', 'roach', 'mpe')

P('FM Whistle', 'Leads') \
    .wt('a', table=0, pos=0.0, level=0.6).an('b', 'sine', oct=2, level=0.0).fm('a', 'b', 0.2, 'pm').off('c') \
    .lfo(1, 5, 'sine', 'voice', fade=0.6).mod('lfo1', 'pitch', 0.005).mod('lfo1', 'afm', 0.1).voice('legato', 0.08).aenv(0.05, 1, 0.9, 0.6) \
    .macro(1, 'Air').mod('m1', 'afm', 0.4).macro(2, 'Vibrato').mod('m2', 'pitch', 0.008, via='lfo1') \
    .chorus('chorus', 0.15, 0.2, 0.3).tags('lead', 'fm', 'whistle')

P('Desert Sitar Lead', 'Leads') \
    .ph('a', 'Tanpura', 'Strike', decay=5, damp=0.08, hard=0.75, buzz=0.6, level=0.7).off('b', 'c') \
    .voice('legato', 0.1).aenv(0.002, 3, 0.6, 1).mpe(bend=24, velAmp=0.4).macro(1, 'Buzz').mod('m1', 'at4', -0.3).macro(2, 'Pluck').mod('m2', 'at2', 0.4) \
    .chorus('chorus', 0.1, 0.1, 0.2).tags('lead', 'sitar', 'plucked', 'roach')

P('Vowel Pressure Lead', 'Leads') \
    .an('a', 'saw', voices=2, detune=5, level=0.6).off('b', 'c').f1('Vowel', 1000, res=0.7, shape=0.2).f2('Ladder LP12', 5000) \
    .mod('press', 'f1shape', 0.7).mod('slide', 'f1cut', 0.4).voice('legato', 0.07).aenv(0.02, 1, 0.9, 0.6).mpe(bend=48) \
    .macro(1, 'Vowel').mod('m1', 'f1shape', 0.8).macro(2, 'Throat').mod('m2', 'f1cut', -0.4) \
    .chorus('chorus', 0.2, 0.25, 0.4).tags('lead', 'vocal', 'mpe')

# =============================================================================================== SEQUENCE & PLUCKS
def berlin(name, notes, gates, rate='1/16', cut=700, res=0.45, tags=()):
    p = P(name, 'Sequence & Plucks') \
        .an('a', 'saw', level=0.6, slop=0.2).an('b', 'pulse', pw=0.3, oct=-1, level=0.35).off('c') \
        .f1('Ladder LP24', cut, res=res, key=0.4, env=0.55).fenv(0.002, 0.25, 0.1, 0.2).aenv(0.002, 0.3, 0.5, 0.25) \
        .seq('notes', rate, len(notes), 0.45, 0.0, 0.0, notes, gates).voice('mono', 0.01) \
        .lfo(1, 0.1, 'sine', sync=True, div='4 bars').mod('lfo1', 'f1cut', 0.35).mod('alt', 'f1cut', 0.08) \
        .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Resonance').mod('m2', 'f1res', 0.4).macro(3, 'Decay').mod('m3', 'f1cut', 0.4, via='fenv')
    p.tags('berlin', 'sequenced', 'hold-a-note', *tags)
    return p

berlin('Berlin Ostinato', [0, 12, 7, 12, 3, 12, 7, 10, 0, 12, 7, 12, 5, 12, 7, 3], [1] * 16, tags=('roach',)).chorus('chorus', 0.15, 0.2, 0.3)
berlin('Empty City Seq', [0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 12, 0, 15, 0, 10, 7], [1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1], cut=500, tags=('roach', 'dark')).nochorus()
berlin('Midnight Sequencer', [0, 7, 12, 15, 12, 7, 3, 7], [1] * 8, rate='1/8', cut=900, res=0.35, tags=('roach',)).chorus('ensemble', 0.2, 0.15, 0.4)
berlin('Pulse Circuit', [0, 12, 0, 10, 0, 12, 0, 7, 0, 12, 0, 5, 0, 3, 0, 7], [1] * 16, cut=650, res=0.55, tags=('tangerine',)).drive(0.2).nochorus()
berlin('Stone Steps Seq', [0, 3, 7, 10, 12, 10, 7, 3, -2, 3, 7, 10, 12, 15, 12, 7], [1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1], cut=800, tags=('roach',)).chorus('chorus', 0.2, 0.2, 0.3)

P('Tribal Pulse Seq', 'Sequence & Plucks') \
    .ph('a', 'Membrane', 'Strike', decay=0.6, damp=0.4, hard=0.6, level=0.7).ph('b', 'Bar', 'Strike', decay=0.8, damp=0.35, hard=0.7, semi=7, level=0.3).off('c') \
    .seq('notes', '1/16', 16, 0.3, 0.12, 0.0, [0, 0, 12, 0, 0, 7, 0, 0, 0, 0, 12, 0, 5, 0, 7, 0], [1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1]) \
    .aenv(0.002, 0.6, 0, 0.5).mod('alt', 'at2', 0.3).mpe(velAmp=0.5) \
    .macro(1, 'Skin').mod('m1', 'at2', 0.5).macro(2, 'Wood').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.1, 0.1, 0.2).tags('tribal', 'percussion', 'roach', 'sequenced', 'hold-a-note')

P('CV Sequenced Drone', 'Sequence & Plucks') \
    .an('a', 'saw', voices=3, detune=8, level=0.6).an('b', 'saw', oct=-1, level=0.4).off('c') \
    .f1('Ladder LP24', 600, res=0.5, key=0.3).seq('cv', '1/8', 16, 0.5, 0.0, 0.35, [0.1, 0.6, 0.2, 0.9, 0.0, 0.5, 0.3, 0.8, 0.1, 0.7, 0.2, 1.0, 0.0, 0.4, 0.3, 0.6]) \
    .mod('seq', 'f1cut', 0.4).mod('seq', 'at1', 0.2).aenv(3, 3, 1, 10).sub(0.25) \
    .macro(1, 'Seq Depth').mod('m1', 'f1cut', 0.4, via='seq').macro(2, 'Cutoff').mod('m2', 'f1cut', 0.4) \
    .chorus('ensemble', 0.25, 0.1, 0.5).tags('sequenced', 'drone', 'roach', 'cv')

P('Arp Crystal Rain', 'Sequence & Plucks') \
    .wt('a', 'Glass & Bell/Crystal Squares', pos=0.4, level=0.55).ph('b', 'Bar', 'Strike', decay=1.5, damp=0.3, hard=0.7, oct=1, level=0.25).off('c') \
    .arp('updown', 3, '1/16', 0.4).aenv(0.002, 1, 0.2, 1.5).f1('SVF LP12', 6000).lfo(1, 0.05, 'tri').mod('lfo1', 'at1', 0.4) \
    .macro(1, 'Glass').mod('m1', 'at1', 0.5).macro(2, 'Bars').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.2, 0.15, 0.3).tags('arp', 'bright', 'glass')

P('Plucked Kalimba', 'Sequence & Plucks') \
    .ph('a', 'Bar', 'Strike', decay=2, damp=0.25, hard=0.6, level=0.7).off('b', 'c').aenv(0.002, 2, 0, 2).mpe(velAmp=0.6) \
    .mod('vel', 'at2', 0.4).macro(1, 'Metal Tines').mod('m1', 'at1', 0.4).macro(2, 'Hardness').mod('m2', 'at2', 0.5) \
    .chorus('chorus', 0.15, 0.1, 0.3).tags('pluck', 'kalimba', 'physical')

P('Granular Pluck', 'Sequence & Plucks') \
    .gr('a', 'keyed', sample='Strings/Tanpura', pos=0.0, size=0.2, density=50, spray=0.02, level=1.0).off('b', 'c') \
    .aenv(0.002, 1.4, 0.2, 1.0).f1('SVF LP12', 7000).mpe(velAmp=0.6).macro(1, 'Scatter').mod('m1', 'at4', 0.5).macro(2, 'Position').mod('m2', 'at1', 0.5) \
    .chorus('chorus', 0.15, 0.1, 0.3).tags('pluck', 'granular')

P('Arp Dreamtime', 'Sequence & Plucks') \
    .ph('a', 'String', 'Strike', decay=3, damp=0.2, hard=0.6, level=0.6).an('b', 'tri', oct=-1, level=0.25).off('c') \
    .arp('order', 2, '1/8', 0.5, swing=0.2, hold=True).aenv(0.002, 2, 0.2, 2).f1('SEM LP12', 4000) \
    .macro(1, 'Pluck').mod('m1', 'at2', 0.4).macro(2, 'Low Tri').mod('m2', 'blevel', 0.5) \
    .chorus('chorus', 0.15, 0.1, 0.3).tags('arp', 'roach', 'dreamtime', 'hold-a-chord')

P('Seq Glass Harmonics', 'Sequence & Plucks') \
    .add('a', 'Glass', 'Octaves', partials=32, shim=0.2, level=0.6).off('b', 'c') \
    .seq('notes', '1/8', 8, 0.6, 0.0, 0.0, [0, 12, 7, 19, 12, 24, 7, 12], [1] * 8).aenv(0.002, 0.8, 0.2, 1).voice('mono', 0.0) \
    .macro(1, 'Morph').mod('m1', 'at1', 0.5).macro(2, 'Shimmer').mod('m2', 'at3', 0.4) \
    .chorus('ensemble', 0.25, 0.1, 0.4).tags('sequenced', 'glass', 'corbacho', 'hold-a-note')

P('Analog Arp Classic', 'Sequence & Plucks') \
    .an('a', 'saw', level=0.6).an('b', 'pulse', pw=0.3, fine=6, level=0.4).off('c').f1('OB-Xa LP24', 800, res=0.35, key=0.4, env=0.5) \
    .fenv(0.002, 0.25, 0.1, 0.25).aenv(0.002, 0.35, 0.4, 0.3).arp('up', 2, '1/16', 0.55) \
    .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Env').mod('m2', 'f1cut', 0.4, via='fenv') \
    .chorus('chorus', 0.2, 0.3, 0.4).tags('arp', 'analog', 'classic')

P('Swing Steps', 'Sequence & Plucks') \
    .wt('a', 'Analog/PWM 50 to 2', pos=0.3, level=0.55).off('b', 'c').f1('Ladder LP24', 1200, res=0.35, key=0.4, env=0.4) \
    .fenv(0.002, 0.2, 0.1, 0.2).aenv(0.002, 0.3, 0.3, 0.25) \
    .seq('notes', '1/16', 12, 0.5, 0.35, 0.0, [0, 7, 12, 7, 3, 10, 0, 7, 12, 15, 10, 7], [1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1]).voice('mono', 0.0) \
    .lfo(1, 0.1, 'tri', sync=True, div='2 bars').mod('lfo1', 'at1', 0.5) \
    .macro(1, 'PWM').mod('m1', 'at1', 0.5).macro(2, 'Cutoff').mod('m2', 'f1cut', 0.5) \
    .chorus('chorus', 0.15, 0.2, 0.3).tags('sequenced', 'swing', 'hold-a-note')
