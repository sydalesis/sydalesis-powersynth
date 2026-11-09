# Driftfield 3.1 bank, part A: shared helpers + SOUNDSCAPES
# (exec'd by gen_presets3.py after bank_v3.py; Patch / P and the v3 helpers are in scope)

def X(p, vib=0.012, pa=0.45, pc=0.25, sc=0.15, st=0.5, vel=0.25, bend=48, press=(), slide=(), bendto=(), rate=5.0):
    """MPE layer used by every 3.1 preset: per-note pressure / slide / bend routes on top of the
    global MPE amounts, plus pressure-controlled vibrato (LFO 6, per voice, faded in)."""
    p.mpe(pressCut=pc, pressAmp=pa, slideCut=sc, slideTimbre=st, velAmp=vel, bend=bend)
    if vib:
        p.lfo(6, rate, 'sine', 'voice', fade=0.7).mod('lfo6', 'pitch', vib, via='press')
    for d, a in press: p.mod('press', d, a)
    for d, a in slide: p.mod('slide', d, a)
    for d, a in bendto: p.mod('bend', d, a)
    return p.tags('mpe')

def field(p, o, sample, pos=0.3, scan=0.03, size=0.6, density=14, spray=0.45, level=0.6, prand=0, shape=0.35, spread=1.0, keyed=False, rev=0.0, **kw):
    """granular field-recording layer (fixed pitch unless keyed)"""
    return p.gr(o, 'keyed' if keyed else 'fixed', sample=sample, pos=pos, scan=scan, size=size, density=density,
                spray=spray, prand=prand, shape=shape, spread=spread, level=level, rev=rev, **kw)

LONG = (6, 4, 1, 18)
VLONG = (12, 5, 1, 26)

# =============================================================================================== SOUNDSCAPES
S = 'Soundscapes'

p = field(P('Monsoon Temple', S), 'a', 'Nature/Rain on Leaves', pos=0.2, scan=0.02, size=0.9, density=10, level=0.55)
p.add('b', 'Choir Oo', 'Organ', morph=0.3, partials=64, shim=0.25, srate=0.07, pdet=6, oct=-1, level=0.55) \
 .ph('c', 'Glass Bowl', 'Bow', decay=14, damp=0.15, hard=0.3, couple=0.5, level=0.25, semi=7) \
 .f1('SVF LP12', 5200, res=0.08, key=0.1).aenv(*LONG).curve(a=-0.3) \
 .lfo(1, 0.018, shape=TIDE).lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.35).mod('lfo2', 'bt1', 0.3).mod('lfo1', 'alevel', 0.2) \
 .macro(1, 'Rain').mod('m1', 'alevel', 0.45).macro(2, 'Temple').mod('m2', 'blevel', 0.45).macro(3, 'Bowl').mod('m3', 'clevel', 0.5).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('rain', 'temple', 'roach', 'night')
X(p, press=[('bt2', 0.3), ('clevel', 0.3)], slide=[('at2', 0.4)])

p = field(P('Canyon at Dusk', S), 'a', 'Nature/Canyon Wind', pos=0.3, size=1.0, density=9, level=0.6)
p.wt('b', 'Ambient/Desert Air', pos=0.2, blend='spectral', voices=3, detune=9, level=0.45, oct=-1) \
 .an('c', 'tri', oct=-2, level=0.3, slop=0.5, drift=0.6) \
 .f1('Xpander BP2', 800, res=0.3, key=0.0).f2('SEM LP12', 2600).route('split', 0.5).aenv(*LONG) \
 .lfo(1, 0.011, 'drift').lfo(2, 0.027, 'sine').mod('lfo1', 'f1cut', 0.45).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'balance', 0.2) \
 .macro(1, 'Wind').mod('m1', 'alevel', 0.5).macro(2, 'Dusk').mod('m2', 'bt1', 0.5).macro(3, 'Earth').mod('m3', 'clevel', 0.5).macro(4, 'Gust').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.18, 0.05, 0.45).tags('desert', 'wind', 'roach', 'dusk')
X(p, press=[('f1res', 0.25), ('blevel', 0.25)], slide=[('bt1', 0.4)])

p = field(P('Night Insects Drone', S), 'a', 'Nature/Night Crickets', pos=0.4, size=0.7, density=12, level=0.45)
field(p, 'b', 'Nature/Cicadas', pos=0.2, size=0.9, density=8, level=0.3)
p.add('c', 'Octaves', 'Fifths', morph=0.4, partials=48, tilt=-0.2, shim=0.15, srate=0.05, oct=-1, level=0.55) \
 .f1('SVF LP12', 9000, res=0.05, key=0).aenv(*VLONG) \
 .lfo(1, 0.008, 'drift').lfo(2, 0.03, 'sine').mod('lfo1', 'ct1', 0.5).mod('lfo2', 'blevel', 0.25).mod('lfo1', 'alevel', -0.2) \
 .macro(1, 'Crickets').mod('m1', 'alevel', 0.5).macro(2, 'Cicadas').mod('m2', 'blevel', 0.5).macro(3, 'Drone').mod('m3', 'clevel', 0.4).macro(4, 'Harmonics').mod('m4', 'ct1', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).width(1.4).tags('night', 'insects', 'corbacho', 'field')
X(p, vib=0, press=[('ct2', 0.4)], slide=[('ct3', -0.4)])

p = field(P('Shoreline Meditation', S), 'a', 'Nature/Ocean Swell', pos=0.1, scan=0.01, size=1.2, density=7, spray=0.3, level=0.6)
p.wt('b', 'Ambient/Ocean of Stars', pos=0.3, blend='spectral', voices=4, detune=10, level=0.4) \
 .off('c').sub(0.15).f1('SVF LP12', 4200, res=0.05, key=0.15).aenv(8, 4, 1, 20).curve(a=-0.5) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'bt1', 0.5).mod('lfo1', 'f1cut', 0.2).mod('lfo1', 'alevel', 0.3) \
 .macro(1, 'Tide').mod('m1', 'alevel', 0.5).macro(2, 'Stars').mod('m2', 'bt1', 0.5).macro(3, 'Depth').mod('m3', 'sub', 0.4).macro(4, 'Shine').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('ocean', 'calm', 'corbacho', 'meditative')
X(p, press=[('blevel', 0.35)], slide=[('bt1', 0.4)])

p = field(P('Hearth and Ghosts', S), 'a', 'Nature/Fire Crackle', pos=0.3, size=0.4, density=20, level=0.5)
p.an('b', 'saw', oct=-1, voices=3, detune=14, slop=0.6, drift=0.6, level=0.35) \
 .add('c', 'Choir Oo', 'Choir Ah', morph=0.2, partials=40, vowel=0.6, vowelamt=0.6, shim=0.2, level=0.35) \
 .f1('OB-Xa LP12', 1100, res=0.15, key=0.3, dtype='tube', drive=0.25).aenv(*LONG).tape(0.2, 0.25, 0.1, 0.6, 1.0) \
 .lfo(1, 0.03, 'drift').lfo(2, 0.07, 'drift', 'voice').mod('lfo1', 'f1cut', 0.3).mod('lfo2', 'ct4', 0.4) \
 .macro(1, 'Fire').mod('m1', 'alevel', 0.5).macro(2, 'Warmth').mod('m2', 'f1cut', 0.5).macro(3, 'Ghosts').mod('m3', 'clevel', 0.45).macro(4, 'Tape').mod('m4', 'fxwow', 0.5) \
 .chorus('solina', 0.25, 0.1, 0.5).tags('fire', 'warm', 'haunted', 'tape')
X(p, press=[('clevel', 0.3)], slide=[('ct4', 0.5)])

p = field(P('Mountain Stream Hymn', S), 'a', 'Nature/Mountain Stream', pos=0.2, size=0.5, density=16, level=0.5)
p.add('b', 'Organ', 'Octaves', morph=0.5, partials=56, tilt=-0.1, shim=0.18, srate=0.06, level=0.5) \
 .wt('c', 'Organic/Wood Flute', pos=0.4, oct=1, level=0.2, voices=2, detune=6) \
 .f1('SVF LP12', 6500, res=0.05, key=0.1).aenv(5, 3, 1, 15) \
 .lfo(1, 0.02, 'sine').lfo(2, 0.045, 'drift', 'voice').mod('lfo1', 'bt1', 0.4).mod('lfo2', 'ct1', 0.3) \
 .macro(1, 'Water').mod('m1', 'alevel', 0.5).macro(2, 'Hymn').mod('m2', 'bt1', 0.4).macro(3, 'Flute').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.07, 0.5).tags('water', 'hymn', 'corbacho', 'pastoral')
X(p, press=[('clevel', 0.35)], slide=[('bt2', 0.4)])

p = field(P('Thunder Over Mesa', S), 'a', 'Nature/Distant Thunder', pos=0.0, scan=0.02, size=1.4, density=5, spray=0.6, level=0.65)
p.wt('b', 'Drone/Slow Storm', pos=0.3, blend='spectral', voices=3, detune=8, oct=-1, level=0.45) \
 .an('c', 'saw', oct=-2, level=0.2, slop=0.5).sub(0.25) \
 .f1('Ladder LP24', 700, res=0.2, key=0.2).aenv(*VLONG).curve(a=-0.4) \
 .mseg(1, [(0, 0.0, 0), (14, 1.0, 0.3), (10, 0.4, -0.2), (16, 0.8, 0.2), (12, 0.0, 0)], loop=(0, 4), mode='free') \
 .mod('mseg1', 'f1cut', 0.5).mod('mseg1', 'bt1', 0.4).lfo(1, 0.04, 'drift').mod('lfo1', 'alevel', 0.25) \
 .macro(1, 'Storm').mod('m1', 'alevel', 0.5).macro(2, 'Mesa').mod('m2', 'f1cut', 0.5).macro(3, 'Weight').mod('m3', 'sub', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('storm', 'desert', 'roach', 'dark', 'cinematic')
X(p, press=[('f1cut', 0.3)], slide=[('bt1', 0.4)])

p = field(P('Wind Chime Garden', S), 'a', 'Metal/Wind Chimes', pos=0.4, size=0.8, density=8, spray=0.6, level=0.5)
field(p, 'b', 'Organic/Leaves Rustle', pos=0.3, size=0.6, density=10, level=0.3)
p.add('c', 'Glass', 'Bell', morph=0.3, partials=48, stretch=0.1, shim=0.3, srate=0.12, level=0.4) \
 .f1('SVF LP12', 9000, res=0.05, key=0).aenv(4, 3, 1, 14) \
 .lfo(1, 0.025, 'drift').mod('lfo1', 'alevel', 0.3).mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Chimes').mod('m1', 'alevel', 0.5).macro(2, 'Leaves').mod('m2', 'blevel', 0.5).macro(3, 'Glass').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.08, 0.4).width(1.5).tags('chimes', 'garden', 'bright', 'corbacho')
X(p, press=[('ct2', 0.35)], slide=[('ct1', 0.5)])

p = field(P('Ice Field Groans', S), 'a', 'Nature/Ice Creaks', pos=0.3, size=0.7, density=6, spray=0.6, level=0.5)
p.wt('b', 'Glass & Bell/Frost', pos=0.3, blend='spectral', voices=3, detune=7, level=0.35) \
 .ph('c', 'Bar', 'Scrape', decay=6, damp=0.3, hard=0.35, couple=0.4, oct=-1, level=0.3) \
 .f1('SVF HP12', 120, res=0.0, key=0).f2('SVF LP12', 7000).aenv(*LONG) \
 .lfo(1, 0.015, 'drift').lfo(2, 0.04, 'sine').mod('lfo1', 'bt1', 0.5).mod('lfo2', 'ct1', 0.4) \
 .macro(1, 'Ice').mod('m1', 'alevel', 0.5).macro(2, 'Frost').mod('m2', 'blevel', 0.4).macro(3, 'Groan').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('ice', 'cold', 'arctic', 'texture')
X(p, press=[('ct2', 0.35)], slide=[('bt1', 0.4)])

p = field(P('Old Wooden House', S), 'a', 'Organic/Wood Creak', pos=0.3, size=0.9, density=5, spray=0.6, level=0.55)
field(p, 'b', 'Nature/Rain Light', pos=0.2, size=0.6, density=12, level=0.3)
p.an('c', 'saw', oct=-1, voices=2, detune=10, slop=0.7, drift=0.7, level=0.35) \
 .f1('Ladder LP12', 900, res=0.1, key=0.3).aenv(*LONG).tape(0.25, 0.3, 0.12, 0.55, 1.0) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Creaks').mod('m1', 'alevel', 0.5).macro(2, 'Rain').mod('m2', 'blevel', 0.5).macro(3, 'Memory').mod('m3', 'clevel', 0.4).macro(4, 'Tape').mod('m4', 'fxwow', 0.5) \
 .chorus('solina', 0.2, 0.08, 0.4).tags('wood', 'rain', 'nostalgic', 'tape')
X(p, press=[('f1cut', 0.3)], slide=[('cdetune', 0.4)])

p = field(P('Shortwave Pilgrim', S), 'a', 'Drones/Shortwave Radio', pos=0.3, size=0.5, density=14, level=0.4)
p.add('b', 'Choir Ah', 'Throat', morph=0.4, partials=56, peak=7, peakamt=0.5, width=0.1, shim=0.2, oct=-1, level=0.5) \
 .an('c', 'pulse', pw=0.3, oct=-1, level=0.2, slop=0.4) \
 .f1('SEM LP12', 2400, res=0.15, key=0.2).aenv(*LONG).shifter(0.25, 0.25, 0.4, 0.8) \
 .lfo(1, 0.02, 'sine').lfo(2, 0.06, 'drift', 'voice').mod('lfo1', 'bt2', 0.4).mod('lfo2', 'ct1', 0.3) \
 .macro(1, 'Radio').mod('m1', 'alevel', 0.5).macro(2, 'Overtone').mod('m2', 'bt2', 0.5).macro(3, 'Shift').mod('m3', 'fxshiftmix', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('radio', 'shifter', 'roach', 'lost-signal')
X(p, press=[('bt2', 0.35)], slide=[('bt1', 0.4)])

p = field(P('Sacred Spring', S), 'a', 'Nature/Bubbling Spring', pos=0.3, size=0.4, density=18, level=0.45)
p.add('b', 'Choir Ah', 'Choir Oo', morph=0.5, partials=64, shim=0.3, srate=0.06, pdet=7, level=0.5) \
 .wt('c', 'Ambient/Altar of Light', pos=0.2, blend='spectral', oct=1, level=0.2, voices=3, detune=8) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(7, 4, 1, 18) \
 .lfo(1, 0.016, shape=BREATH).lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'bt1', 0.4).mod('lfo2', 'ct1', 0.4) \
 .macro(1, 'Spring').mod('m1', 'alevel', 0.5).macro(2, 'Choir').mod('m2', 'blevel', 0.4).macro(3, 'Light').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('water', 'choir', 'sacred', 'corbacho')
X(p, press=[('clevel', 0.35)], slide=[('bt4', 0.5)])

p = field(P('Gamelan Dreams', S), 'a', 'Metal/Gamelan Pulse', pos=0.3, size=0.7, density=10, spray=0.4, level=0.5, keyed=True)
p.add('b', 'Bell', 'Fifths', morph=0.4, partials=48, stretch=0.15, shim=0.25, oct=-1, level=0.4) \
 .off('c').f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(3, 3, 1, 14).curve(a=0.2) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Gongs').mod('m1', 'alevel', 0.4).macro(2, 'Scan').mod('m2', 'at1', 0.5).macro(3, 'Bronze').mod('m3', 'bt1', 0.5) \
 .chorus('chorus', 0.15, 0.07, 0.4).tags('gamelan', 'metallic', 'dream', 'world')
X(p, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = field(P('Didgeridoo Desert', S), 'a', 'Drones/Didgeridoo', pos=0.2, size=0.8, density=10, level=0.55, keyed=True, oct=-1)
field(p, 'b', 'Organic/Sand Flow', pos=0.3, size=0.6, density=10, level=0.3)
p.add('c', 'Throat', 'Throat', partials=48, peak=7, peakamt=0.8, width=0.08, shim=0.05, oct=-1, level=0.3) \
 .f1('Vowel', 600, res=0.4, key=0.0, shape=0.3).aenv(*LONG) \
 .lfo(1, 0.05, 'drift').lfo(2, 0.13, 'sine').mod('lfo1', 'f1shape', 0.4).mod('lfo2', 'ct2', 0.3) \
 .macro(1, 'Drone').mod('m1', 'alevel', 0.4).macro(2, 'Sand').mod('m2', 'blevel', 0.5).macro(3, 'Mouth').mod('m3', 'f1shape', 0.6).macro(4, 'Overtone').mod('m4', 'ct2', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('didgeridoo', 'desert', 'roach', 'dreamtime', 'ritual')
X(p, vib=0, press=[('f1shape', 0.4)], slide=[('ct2', 0.5)])

p = field(P('Rain on the Tin Roof', S), 'a', 'Metal/Rain on Tin', pos=0.3, size=0.5, density=16, level=0.5)
field(p, 'b', 'Nature/Rain Light', pos=0.5, size=0.8, density=10, level=0.35)
p.an('c', 'tri', level=0.4, voices=3, detune=8, slop=0.4).f1('SEM LP12', 1800, res=0.1, key=0.3).aenv(4, 3, 1, 12) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Tin').mod('m1', 'alevel', 0.5).macro(2, 'Rain').mod('m2', 'blevel', 0.5).macro(3, 'Tone').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.2, 0.08, 0.4).tags('rain', 'metal', 'cozy')
X(p, press=[('clevel', 0.3)], slide=[('cdetune', 0.4)])

p = field(P('Monks in the Valley', S), 'a', 'Voices/Monk Hum Low', pos=0.3, size=0.9, density=10, level=0.55, keyed=True)
field(p, 'b', 'Nature/Canyon Wind', pos=0.6, size=1.0, density=8, level=0.3)
p.add('c', 'Throat', 'Choir Oo', morph=0.3, partials=48, peak=8, peakamt=0.5, shim=0.1, oct=-1, level=0.3) \
 .f1('SVF LP12', 3500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.012, shape=BREATH).mod('lfo1', 'alevel', 0.3).mod('lfo1', 'ct2', 0.3) \
 .macro(1, 'Monks').mod('m1', 'alevel', 0.4).macro(2, 'Valley').mod('m2', 'blevel', 0.5).macro(3, 'Overtone').mod('m3', 'ct2', 0.5) \
 .chorus('ensemble', 0.22, 0.05, 0.5).tags('monks', 'chant', 'roach', 'sacred')
X(p, press=[('ct2', 0.4)], slide=[('at1', 0.4)])

p = field(P('Electric Dusk Field', S), 'a', 'Drones/Electric Hum', pos=0.2, size=0.6, density=12, level=0.35)
field(p, 'b', 'Nature/Night Crickets', pos=0.3, size=0.6, density=12, level=0.35)
p.wt('c', 'Drift Spectra/Drift Vapour', pos=0.3, blend='spectral', voices=4, detune=10, level=0.45) \
 .f1('OB-Xa LP24', 2200, res=0.1, key=0.2).aenv(*LONG) \
 .lfo(1, 0.02, 'sine').lfo(2, 0.05, 'drift', 'voice').mod('lfo1', 'ct1', 0.5).mod('lfo2', 'f1cut', 0.2) \
 .macro(1, 'Hum').mod('m1', 'alevel', 0.5).macro(2, 'Field').mod('m2', 'blevel', 0.5).macro(3, 'Vapour').mod('m3', 'ct1', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('suburban', 'night', 'electric', 'lo-fi')
X(p, press=[('f1cut', 0.3)], slide=[('ct1', 0.5)])

p = field(P('Bamboo Forest Rite', S), 'a', 'Organic/Bamboo Knocks', pos=0.3, size=0.3, density=6, spray=0.7, level=0.5)
field(p, 'b', 'Organic/Leaves Rustle', pos=0.4, size=0.7, density=10, level=0.35)
p.wt('c', 'Organic/Shakuhachi', pos=0.3, level=0.45, voices=2, detune=5) \
 .f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(3, 3, 1, 12) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Knocks').mod('m1', 'alevel', 0.5).macro(2, 'Forest').mod('m2', 'blevel', 0.5).macro(3, 'Breath').mod('m3', 'ct1', 0.5) \
 .chorus('chorus', 0.12, 0.06, 0.3).tags('forest', 'bamboo', 'flute', 'zen')
X(p, vib=0.018, press=[('ct1', 0.4)], slide=[('f1cut', 0.3)])

p = field(P('Abandoned Station', S), 'a', 'Metal/Metal Groan', pos=0.3, size=0.9, density=7, spray=0.5, level=0.5)
field(p, 'b', 'Textures/Vinyl Crackle Bed', pos=0.3, size=0.4, density=16, level=0.3)
p.wt('c', 'Drone/Iron Hum', pos=0.3, blend='spectral', oct=-1, level=0.4, voices=2, detune=6) \
 .f1('Comb +', 300, res=0.5, key=0.5).f2('SVF LP12', 5000).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Steel').mod('m1', 'alevel', 0.5).macro(2, 'Dust').mod('m2', 'blevel', 0.5).macro(3, 'Comb').mod('m3', 'f1res', 0.4) \
 .chorus('flanger', 0.15, 0.03, 0.4, 0.3).tags('industrial', 'dark', 'abandoned', 'cinematic')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

p = field(P('Breathing Earth', S), 'a', 'Voices/Breath Cycle', pos=0.2, size=1.0, density=8, level=0.5)
p.an('b', 'tri', oct=-1, level=0.4, slop=0.5, drift=0.5).sub(0.3) \
 .add('c', 'Hollow', 'Octaves', morph=0.3, partials=32, shim=0.1, level=0.3) \
 .f1('SVF LP12', 2500, res=0.05, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.25, shape=BREATH).mod('lfo1', 'blevel', 0.2).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Breath').mod('m1', 'alevel', 0.5).macro(2, 'Earth').mod('m2', 'sub', 0.4).macro(3, 'Hollow').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('breath', 'organic', 'roach', 'deep')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('ct1', 0.4)])

p = field(P('Tidal Cathedral', S), 'a', 'Nature/Ocean Swell', pos=0.4, size=1.2, density=7, level=0.45)
p.wt('b', 'Organs/Organ Cathedral', pos=0.3, level=0.45, voices=3, detune=6) \
 .add('c', 'Choir Ah', 'Organ', morph=0.5, partials=64, shim=0.2, oct=1, level=0.2) \
 .f1('SVF LP12', 4000, res=0.05, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'bt1', 0.5).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Sea').mod('m1', 'alevel', 0.5).macro(2, 'Stops').mod('m2', 'bt1', 0.5).macro(3, 'Choir').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('organ', 'ocean', 'sacred', 'roach')
X(p, vib=0, press=[('clevel', 0.3)], slide=[('bt1', 0.5)])

p = field(P('Seed Pod Rain Dance', S), 'a', 'Organic/Seed Rattle', pos=0.3, size=0.3, density=14, spray=0.6, level=0.45)
p.ph('b', 'Membrane', 'Strike', decay=1.5, damp=0.4, hard=0.4, oct=-1, level=0.45) \
 .add('c', 'Reed', 'Throat', morph=0.3, partials=40, shim=0.15, oct=-1, level=0.35) \
 .f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(0.01, 2, 0.6, 4).curve(a=0.6) \
 .arp('order', 2, '1/8', 0.5, swing=0.12, hold=True) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Rattle').mod('m1', 'alevel', 0.5).macro(2, 'Drum').mod('m2', 'blevel', 0.5).macro(3, 'Reed').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.1, 0.06, 0.3).tags('tribal', 'ritual', 'roach', 'percussive', 'arp')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('ct1', 0.4)])

p = field(P('Glass Harmonica Forest', S), 'a', 'Textures/Glass Harmonics', pos=0.3, size=0.8, density=10, level=0.45, keyed=True)
field(p, 'b', 'Organic/Leaves Rustle', pos=0.2, size=0.7, density=8, level=0.25)
p.wt('c', 'Glass & Bell/Glass Harmonica', pos=0.4, blend='spectral', level=0.35, voices=3, detune=6) \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'ct1', 0.3) \
 .macro(1, 'Glass').mod('m1', 'alevel', 0.5).macro(2, 'Forest').mod('m2', 'blevel', 0.5).macro(3, 'Harmonica').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.2, 0.06, 0.4).tags('glass', 'forest', 'corbacho', 'luminous')
X(p, press=[('clevel', 0.35)], slide=[('at1', 0.5)])

p = field(P('Lost Expedition', S), 'a', 'Nature/Wind Whistle', pos=0.3, size=0.9, density=8, level=0.45)
field(p, 'b', 'Organic/Stone Scrape', pos=0.3, size=0.6, density=8, level=0.3)
p.wt('c', 'Drone/Cave Resonance', pos=0.3, blend='spectral', oct=-1, voices=3, detune=8, level=0.4) \
 .f1('Xpander BP2', 700, res=0.3, key=0.1).f2('SVF LP12', 4000).route('split', 0.5).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').lfo(2, 0.03, 'sine').mod('lfo1', 'f1cut', 0.4).mod('lfo2', 'ct1', 0.4) \
 .macro(1, 'Wind').mod('m1', 'alevel', 0.5).macro(2, 'Stones').mod('m2', 'blevel', 0.5).macro(3, 'Cave').mod('m3', 'ct1', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('cave', 'wind', 'roach', 'dark', 'exploration')
X(p, press=[('f1res', 0.3)], slide=[('ct1', 0.5)])

p = field(P('Tanpura at Dawn', S), 'a', 'Strings/Tanpura', pos=0.2, scan=0.04, size=0.9, density=10, level=0.55, keyed=True)
field(p, 'b', 'Nature/Night Crickets', pos=0.5, size=0.6, density=10, level=0.25)
p.add('c', 'Strings', 'Throat', morph=0.3, partials=56, peak=10, peakamt=0.4, shim=0.15, level=0.3) \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(4, 3, 1, 16) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Tanpura').mod('m1', 'alevel', 0.4).macro(2, 'Dawn').mod('m2', 'blevel', 0.5).macro(3, 'Strings').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('indian', 'raga', 'dawn', 'roach')
X(p, press=[('ct2', 0.4)], slide=[('at1', 0.4)])

p = field(P('Data Center Hymn', S), 'a', 'Textures/Data Stream', pos=0.3, size=0.3, density=20, spray=0.6, level=0.3)
field(p, 'b', 'Drones/Electric Hum', pos=0.3, size=0.8, density=8, level=0.3)
p.wt('c', 'Chords/Chord Sus4', pos=0.3, voices=4, detune=10, level=0.45) \
 .f1('SVF LP12', 3500, res=0.1, key=0.2).aenv(*LONG) \
 .lfo(1, 0.025, 'sine').mod('lfo1', 'ct1', 0.5) \
 .macro(1, 'Data').mod('m1', 'alevel', 0.5).macro(2, 'Hum').mod('m2', 'blevel', 0.5).macro(3, 'Chord').mod('m3', 'ct1', 0.5) \
 .chorus('chorus', 0.18, 0.06, 0.4).crush(10, 2, 0.15).tags('digital', 'future', 'cold')
X(p, press=[('fxcrush', 0.4)], slide=[('ct1', 0.5)])

p = field(P('Singing Bowl Circle', S), 'a', 'Metal/Singing Bowl Pair', pos=0.2, size=1.0, density=8, level=0.5, keyed=True)
p.ph('b', 'Glass Bowl', 'Bow', decay=18, damp=0.12, hard=0.3, couple=0.6, level=0.35, semi=7) \
 .add('c', 'Bell', 'Glass', morph=0.5, partials=48, stretch=0.2, shim=0.2, oct=-1, level=0.2) \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Bowls').mod('m1', 'alevel', 0.4).macro(2, 'Bowing').mod('m2', 'bt2', 0.5).macro(3, 'Shimmer').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('bowls', 'meditation', 'healing', 'corbacho')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('bt1', 0.5)])

p = field(P('Tape Memories', S), 'a', 'Drones/Tape Loop Pad', pos=0.3, size=0.9, density=10, level=0.55, keyed=True)
field(p, 'b', 'Textures/Vinyl Crackle Bed', pos=0.3, size=0.5, density=14, level=0.3)
p.off('c').f1('Ladder LP12', 2500, res=0.1, key=0.2).aenv(*LONG).tape(0.3, 0.35, 0.15, 0.5, 1.0) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Loop').mod('m1', 'at1', 0.5).macro(2, 'Crackle').mod('m2', 'blevel', 0.5).macro(3, 'Wow').mod('m3', 'fxwow', 0.6).macro(4, 'Age').mod('m4', 'f1cut', -0.5) \
 .chorus('solina', 0.2, 0.08, 0.4).tags('tape', 'nostalgic', 'lo-fi', 'memory')
X(p, press=[('f1cut', 0.3)], slide=[('at1', 0.5)])

p = field(P('Orbital Station', S), 'a', 'Textures/Frozen Spectrum', pos=0.3, size=1.0, density=8, level=0.4, keyed=True)
field(p, 'b', 'Drones/Sub Rumble Field', pos=0.3, size=1.0, density=6, level=0.35)
p.wt('c', 'Ambient/Night Sky Shimmer', pos=0.3, blend='spectral', voices=4, detune=10, oct=1, level=0.25) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.01, 'sine').lfo(2, 0.03, 'drift').mod('lfo1', 'ct1', 0.5).mod('lfo2', 'at1', 0.3) \
 .macro(1, 'Spectrum').mod('m1', 'alevel', 0.5).macro(2, 'Rumble').mod('m2', 'blevel', 0.5).macro(3, 'Stars').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.2, 0.04, 0.5).width(1.5).tags('space', 'sci-fi', 'corbacho', 'vast')
X(p, press=[('clevel', 0.3)], slide=[('ct1', 0.5)])

p = field(P('Harmonium Village', S), 'a', 'Drones/Harmonium Reed', pos=0.3, size=0.8, density=10, level=0.5, keyed=True)
field(p, 'b', 'Nature/Cicadas', pos=0.3, size=0.8, density=8, level=0.25)
p.off('c').f1('SEM LP12', 3000, res=0.1, key=0.2).aenv(1.5, 3, 1, 8) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Reed').mod('m1', 'alevel', 0.4).macro(2, 'Summer').mod('m2', 'blevel', 0.5).macro(3, 'Tone').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.3).tags('harmonium', 'village', 'warm', 'world')
X(p, press=[('f1cut', 0.3)], slide=[('at1', 0.4)])

p = field(P('Cello in the Mist', S), 'a', 'Strings/Bowed Cello Drone', pos=0.3, size=1.0, density=9, level=0.55, keyed=True)
field(p, 'b', 'Nature/Canyon Wind', pos=0.2, size=1.0, density=6, level=0.25)
p.wt('c', 'Strings & Reeds/Cello Body', pos=0.3, voices=3, detune=7, level=0.3) \
 .f1('SVF LP12', 4000, res=0.05, key=0.2).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo1', 'ct1', 0.3) \
 .macro(1, 'Cello').mod('m1', 'alevel', 0.4).macro(2, 'Mist').mod('m2', 'blevel', 0.5).macro(3, 'Section').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.2, 0.06, 0.4).tags('strings', 'mist', 'cinematic', 'melancholic')
X(p, vib=0.016, press=[('clevel', 0.3)], slide=[('at1', 0.4)])

p = field(P('Thunder Drums Far Away', S), 'a', 'Nature/Distant Thunder', pos=0.3, size=1.2, density=5, level=0.5)
p.ph('b', 'Membrane', 'Strike', decay=3, damp=0.35, hard=0.3, oct=-2, level=0.5) \
 .an('c', 'sine', oct=-2, level=0.3).f1('SVF LP12', 1200, res=0.05, key=0.1).aenv(0.01, 3, 0.4, 5).curve(a=0.8) \
 .arp('order', 1, '1/4', 0.4, swing=0.1, hold=True) \
 .lfo(1, 0.05, 'drift').mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Thunder').mod('m1', 'alevel', 0.5).macro(2, 'Drum').mod('m2', 'blevel', 0.5).macro(3, 'Boom').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('tribal', 'drums', 'roach', 'ritual', 'arp')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('bt4', -0.3)])

p = field(P('Underwater Chapel', S), 'a', 'Nature/Bubbling Spring', pos=0.3, size=0.5, density=14, level=0.35)
p.wt('b', 'Organs/Organ Celeste', pos=0.3, voices=3, detune=8, level=0.45) \
 .add('c', 'Hollow', 'Choir Oo', morph=0.4, partials=40, shim=0.2, level=0.3) \
 .f1('Ladder LP24', 900, res=0.25, key=0.2).aenv(*LONG).phaser(0.3, 0.05, 0.7, 0.5) \
 .lfo(1, 0.04, 'sine').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Bubbles').mod('m1', 'alevel', 0.5).macro(2, 'Depth').mod('m2', 'f1cut', -0.5).macro(3, 'Choir').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('underwater', 'organ', 'dreamy')
X(p, press=[('f1cut', 0.35)], slide=[('bt1', 0.4)])

p = field(P('Prayer Flags', S), 'a', 'Nature/Wind Whistle', pos=0.4, size=0.8, density=8, level=0.4)
field(p, 'b', 'Metal/Bell Tree', pos=0.3, size=0.6, density=6, spray=0.6, level=0.3)
p.add('c', 'Throat', 'Choir Ah', morph=0.4, partials=56, peak=9, peakamt=0.6, width=0.08, oct=-1, level=0.4) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*VLONG) \
 .mseg(1, [(0, 0.2, 0), (8, 0.6, 0), (8, 0.35, 0), (8, 0.8, 0), (8, 0.2, 0)], loop=(0, 4), mode='free').mod('mseg1', 'ct2', 0.6) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'alevel', 0.3) \
 .macro(1, 'Wind').mod('m1', 'alevel', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.5).macro(3, 'Chant').mod('m3', 'clevel', 0.4) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('tibet', 'mountain', 'chant', 'roach')
X(p, press=[('ct2', 0.4)], slide=[('ct1', 0.5)])

p = field(P('Rainforest Canopy', S), 'a', 'Nature/Rain on Leaves', pos=0.4, size=0.6, density=14, level=0.45)
field(p, 'b', 'Nature/Cicadas', pos=0.5, size=0.6, density=10, level=0.3)
p.wt('c', 'Organic/Bamboo Breath', pos=0.3, voices=3, detune=8, level=0.4) \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'ct1', 0.4).mod('lfo1', 'blevel', 0.2) \
 .macro(1, 'Rain').mod('m1', 'alevel', 0.5).macro(2, 'Life').mod('m2', 'blevel', 0.5).macro(3, 'Breath').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).width(1.5).tags('jungle', 'rain', 'lush', 'field')
X(p, press=[('clevel', 0.3)], slide=[('ct1', 0.5)])

p = field(P('Iron Bridge Wind', S), 'a', 'Metal/Bowed Cymbal', pos=0.3, size=0.8, density=9, level=0.45, keyed=True)
field(p, 'b', 'Nature/Canyon Wind', pos=0.4, size=1.0, density=7, level=0.35)
p.ph('c', 'Plate', 'Bow', decay=12, damp=0.15, hard=0.55, couple=0.4, oct=-1, level=0.3) \
 .f1('SVF HP12', 150).f2('SVF LP12', 8000).aenv(*LONG) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'ct1', 0.3) \
 .macro(1, 'Cymbal').mod('m1', 'alevel', 0.5).macro(2, 'Wind').mod('m2', 'blevel', 0.5).macro(3, 'Steel').mod('m3', 'clevel', 0.5) \
 .chorus('flanger', 0.12, 0.03, 0.4, 0.2).tags('metallic', 'wind', 'industrial', 'cinematic')
X(p, press=[('ct2', 0.4)], slide=[('at1', 0.5)])

p = field(P('Zither by the Lake', S), 'a', 'Strings/Zither Swarm', pos=0.3, size=0.6, density=10, level=0.5, keyed=True)
field(p, 'b', 'Nature/Mountain Stream', pos=0.4, size=0.6, density=10, level=0.3)
p.add('c', 'Strings', 'Fifths', morph=0.4, partials=48, shim=0.2, oct=-1, level=0.3) \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(2, 3, 1, 12) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Zither').mod('m1', 'alevel', 0.4).macro(2, 'Lake').mod('m2', 'blevel', 0.5).macro(3, 'Strings').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('zither', 'lake', 'pastoral', 'asian')
X(p, press=[('clevel', 0.3)], slide=[('at1', 0.5)])

p = field(P('Smoke Signals', S), 'a', 'Nature/Fire Crackle', pos=0.5, size=0.4, density=16, level=0.4)
field(p, 'b', 'Voices/Whisper Air', pos=0.3, size=0.7, density=9, level=0.35)
p.wt('c', 'Ambient/Ritual Breath', pos=0.3, blend='spectral', voices=3, detune=8, oct=-1, level=0.4) \
 .f1('Vowel', 800, res=0.4, key=0.0, shape=0.4).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'f1shape', 0.4).mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Fire').mod('m1', 'alevel', 0.5).macro(2, 'Whispers').mod('m2', 'blevel', 0.5).macro(3, 'Vowel').mod('m3', 'f1shape', 0.6) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('ritual', 'fire', 'roach', 'shamanic')
X(p, press=[('f1shape', 0.35)], slide=[('ct1', 0.5)])

p = field(P('Arctic Aurora', S), 'a', 'Textures/Glass Harmonics', pos=0.3, size=0.9, density=10, level=0.4, keyed=True)
field(p, 'b', 'Nature/Ice Creaks', pos=0.3, size=0.6, density=6, level=0.3)
p.wt('c', 'Textures/Aurora', pos=0.3, blend='spectral', voices=5, detune=12, level=0.4) \
 .f1('SVF LP12', 7500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.009, 'sine').lfo(2, 0.031, 'drift').mod('lfo1', 'ct1', 0.6).mod('lfo2', 'at1', 0.3) \
 .macro(1, 'Glass').mod('m1', 'alevel', 0.5).macro(2, 'Ice').mod('m2', 'blevel', 0.5).macro(3, 'Aurora').mod('m3', 'ct1', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).width(1.5).tags('arctic', 'aurora', 'corbacho', 'glacial')
X(p, press=[('clevel', 0.35)], slide=[('ct1', 0.5)])

p = field(P('Shruti Ganges', S), 'a', 'Drones/Shruti Box', pos=0.3, size=0.9, density=9, level=0.5, keyed=True)
field(p, 'b', 'Nature/Mountain Stream', pos=0.5, size=0.8, density=10, level=0.25)
p.add('c', 'Throat', 'Reed', morph=0.3, partials=48, peak=8, peakamt=0.5, oct=-1, level=0.3) \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, shape=BREATH).mod('lfo1', 'alevel', 0.2).mod('lfo1', 'ct2', 0.3) \
 .macro(1, 'Shruti').mod('m1', 'alevel', 0.4).macro(2, 'River').mod('m2', 'blevel', 0.5).macro(3, 'Overtone').mod('m3', 'ct2', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('india', 'river', 'drone', 'sacred')
X(p, press=[('ct2', 0.4)], slide=[('at1', 0.4)])

p = field(P('Dust Storm Approaching', S), 'a', 'Organic/Sand Flow', pos=0.3, size=0.6, density=16, level=0.5)
field(p, 'b', 'Nature/Canyon Wind', pos=0.5, size=1.0, density=8, level=0.4)
p.wt('c', 'Drift Spectra/Drift Dune', pos=0.3, blend='spectral', voices=3, detune=8, oct=-1, level=0.35) \
 .f1('SVF BP12', 900, res=0.2, key=0).f2('SVF LP12', 6000).aenv(*VLONG) \
 .mseg(1, [(0, 0.0, 0), (20, 1.0, 0.4), (8, 0.7, 0), (12, 0.0, -0.3)], loop=(0, 3), mode='free').mod('mseg1', 'f1cut', 0.6).mod('mseg1', 'alevel', 0.3) \
 .macro(1, 'Sand').mod('m1', 'alevel', 0.5).macro(2, 'Wind').mod('m2', 'blevel', 0.5).macro(3, 'Dune').mod('m3', 'ct1', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('desert', 'storm', 'roach', 'cinematic')
X(p, press=[('f1cut', 0.35)], slide=[('ct1', 0.5)])

p = field(P('Choir of the Caves', S), 'a', 'Voices/Choir Vowel Drift', pos=0.3, size=1.0, density=10, level=0.5, keyed=True, oct=-1)
field(p, 'b', 'Nature/Water Drops', pos=0.3, size=0.4, density=8, spray=0.7, level=0.3)
p.wt('c', 'Drone/Cave Resonance', pos=0.3, blend='spectral', voices=3, detune=7, oct=-1, level=0.3) \
 .f1('SVF LP12', 3500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.012, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'ct1', 0.3) \
 .macro(1, 'Choir').mod('m1', 'alevel', 0.4).macro(2, 'Drops').mod('m2', 'blevel', 0.5).macro(3, 'Cave').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.22, 0.05, 0.5).tags('choir', 'cave', 'roach', 'dark', 'sacred')
X(p, press=[('clevel', 0.3)], slide=[('at1', 0.5)])

p = field(P('Gongs in the Fog', S), 'a', 'Metal/Gong Swell', pos=0.4, size=1.1, density=8, level=0.5, keyed=True, oct=-1)
p.wt('b', 'Drone/Low Fog', pos=0.3, blend='spectral', voices=3, detune=8, level=0.4) \
 .noise(0.1, 0.2).off('c').f1('SVF LP12', 3000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'bt1', 0.5).mod('lfo1', 'noise', 0.1) \
 .macro(1, 'Gongs').mod('m1', 'alevel', 0.5).macro(2, 'Fog').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'noise', 0.3) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('gong', 'fog', 'roach', 'dark', 'ritual')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = field(P('Summer Meadow', S), 'a', 'Nature/Cicadas', pos=0.3, size=0.8, density=8, level=0.3)
field(p, 'b', 'Organic/Leaves Rustle', pos=0.3, size=0.6, density=10, level=0.3)
p.an('c', 'pulse', pw=0.4, voices=3, detune=10, slop=0.4, level=0.4) \
 .f1('SEM LP12', 2500, res=0.1, key=0.3).aenv(2, 3, 1, 10) \
 .lfo(1, 0.15, 'tri').mod('lfo1', 'ct1', 0.2) \
 .macro(1, 'Insects').mod('m1', 'alevel', 0.5).macro(2, 'Breeze').mod('m2', 'blevel', 0.5).macro(3, 'Warmth').mod('m3', 'f1cut', 0.5) \
 .chorus('solina', 0.3, 0.1, 0.5).tags('summer', 'warm', 'pastoral', 'happy')
X(p, press=[('f1cut', 0.3)], slide=[('ct1', 0.4)])

p = field(P('Temple Bell Rain', S), 'a', 'Nature/Rain Light', pos=0.3, size=0.7, density=12, level=0.4)
p.ph('b', 'Bell', 'Strike', decay=9, damp=0.15, hard=0.6, level=0.45) \
 .add('c', 'Choir Oo', 'Octaves', morph=0.4, partials=40, shim=0.2, oct=-1, level=0.35) \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(0.01, 6, 0.5, 10).curve(a=0.5) \
 .arp('random', 2, '1/2', 0.8, hold=True) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Rain').mod('m1', 'alevel', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.5).macro(3, 'Choir').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('bells', 'rain', 'temple', 'generative', 'arp')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('bt1', 0.5)])

p = field(P('Railway Night', S), 'a', 'Metal/Metal Groan', pos=0.2, size=0.8, density=8, level=0.35)
field(p, 'b', 'Nature/Rain Light', pos=0.3, size=0.8, density=10, level=0.35)
p.an('c', 'saw', oct=-1, voices=4, detune=12, slop=0.5, level=0.35) \
 .f1('OB-Xa LP24', 1300, res=0.15, key=0.2).aenv(*LONG).tape(0.2, 0.25, 0.1, 0.55, 1.0) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Rails').mod('m1', 'alevel', 0.5).macro(2, 'Rain').mod('m2', 'blevel', 0.5).macro(3, 'Pad').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.07, 0.5).tags('urban', 'night', 'melancholic', 'noir')
X(p, press=[('f1cut', 0.3)], slide=[('cdetune', 0.4)])

p = field(P('Breath of the Mountain', S), 'a', 'Voices/Breath Cycle', pos=0.3, size=1.0, density=8, level=0.4)
field(p, 'b', 'Voices/Overtone Drone', pos=0.3, size=0.9, density=9, level=0.4, keyed=True, oct=-1)
p.off('c').f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.012, 'drift').mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Breath').mod('m1', 'alevel', 0.5).macro(2, 'Overtones').mod('m2', 'bt1', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('mountain', 'overtone', 'roach', 'breath')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('bt1', 0.5)])

p = field(P('Sunken Bells', S), 'a', 'Metal/Bell Tree', pos=0.4, size=0.7, density=7, level=0.4, keyed=True, oct=-1)
field(p, 'b', 'Nature/Ocean Swell', pos=0.4, size=1.0, density=7, level=0.35)
p.wt('c', 'Metallic/Temple Bell', pos=0.3, blend='spectral', voices=2, detune=6, level=0.3) \
 .f1('Ladder LP24', 1400, res=0.2, key=0.2).aenv(*LONG).phaser(0.25, 0.04, 0.6, 0.4) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Bells').mod('m1', 'alevel', 0.5).macro(2, 'Sea').mod('m2', 'blevel', 0.5).macro(3, 'Depth').mod('m3', 'f1cut', -0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('underwater', 'bells', 'mysterious')
X(p, vib=0, press=[('f1cut', 0.35)], slide=[('ct1', 0.5)])

p = field(P('Cicada Temple Noon', S), 'a', 'Nature/Cicadas', pos=0.2, size=0.9, density=8, level=0.35)
p.wt('b', 'Ambient/Structures from Silence', pos=0.3, blend='spectral', voices=3, detune=7, level=0.45) \
 .ph('c', 'Bell', 'Strike', decay=12, damp=0.1, hard=0.5, level=0.2, oct=1) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Heat').mod('m1', 'alevel', 0.5).macro(2, 'Silence').mod('m2', 'bt1', 0.5).macro(3, 'Bell').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('japan', 'temple', 'summer', 'roach')
X(p, press=[('clevel', 0.35)], slide=[('bt1', 0.5)])

p = field(P('Distant Village Fire', S), 'a', 'Nature/Fire Crackle', pos=0.3, size=0.5, density=14, level=0.35)
field(p, 'b', 'Voices/Choir Ah', pos=0.3, size=1.0, density=9, level=0.4, keyed=True)
p.an('c', 'tri', oct=-1, slop=0.5, level=0.3).f1('SVF LP12', 3500, res=0.05, key=0.2).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Fire').mod('m1', 'alevel', 0.5).macro(2, 'Voices').mod('m2', 'blevel', 0.5).macro(3, 'Warmth').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('village', 'fire', 'voices', 'warm')
X(p, press=[('blevel', 0.3)], slide=[('bt1', 0.5)])

p = field(P('Black Lodge Hum', S), 'a', 'Drones/Electric Hum', pos=0.3, size=0.9, density=8, level=0.35)
p.an('b', 'saw', oct=-1, voices=3, detune=7, slop=0.4, level=0.4) \
 .wt('c', 'Drone/Black Glass', pos=0.3, blend='spectral', level=0.35, voices=2, detune=6) \
 .f1('Ladder LP24', 700, res=0.35, key=0.2).aenv(*LONG).phaser(0.3, 0.03, 0.7, 0.6) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'f1cut', 0.35).mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Hum').mod('m1', 'alevel', 0.5).macro(2, 'Curtain').mod('m2', 'f1cut', 0.5).macro(3, 'Glass').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('noir', 'dark', 'lynch', 'eerie')
X(p, press=[('f1res', 0.3)], slide=[('ct1', 0.5)])

p = field(P('Oasis Mirage', S), 'a', 'Organic/Sand Flow', pos=0.3, size=0.6, density=10, level=0.3)
p.wt('b', 'Ambient/Mirage Heat', pos=0.3, blend='spectral', voices=4, detune=10, level=0.45) \
 .wt('c', 'Organic/Ocarina', pos=0.3, oct=1, level=0.2) \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG).tape(0.1, 0.4, 0.05, 0.7, 0.6) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Sand').mod('m1', 'alevel', 0.5).macro(2, 'Mirage').mod('m2', 'bt1', 0.5).macro(3, 'Ocarina').mod('m3', 'clevel', 0.5).macro(4, 'Heat Wobble').mod('m4', 'fxwow', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('desert', 'mirage', 'roach', 'heat')
X(p, vib=0.018, press=[('clevel', 0.35)], slide=[('bt1', 0.5)])

p = field(P('Stone Circle Solstice', S), 'a', 'Organic/Stone Scrape', pos=0.3, size=0.7, density=8, level=0.35)
p.add('b', 'Throat', 'Octaves', morph=0.4, partials=56, peak=6, peakamt=0.6, width=0.1, oct=-1, level=0.45) \
 .ph('c', 'Bar', 'Bow', decay=10, damp=0.2, hard=0.4, couple=0.4, level=0.3) \
 .f1('SVF LP12', 4000, res=0.05, key=0.1).aenv(*VLONG) \
 .mseg(1, [(0, 0.0, 0), (10, 0.5, 0), (10, 1.0, 0), (10, 0.5, 0), (10, 0.0, 0)], loop=(0, 4), mode='free').mod('mseg1', 'bt2', 0.6) \
 .macro(1, 'Stones').mod('m1', 'alevel', 0.5).macro(2, 'Chant').mod('m2', 'bt2', 0.5).macro(3, 'Bar').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('ancient', 'ritual', 'roach', 'solstice')
X(p, press=[('bt2', 0.4)], slide=[('ct1', 0.5)])

p = field(P('Kalimba Rain Garden', S), 'a', 'Nature/Rain Light', pos=0.3, size=0.7, density=10, level=0.35)
p.wt('b', 'Organic/Kalimba Tine', pos=0.2, level=0.5).ph('c', 'Bar', 'Strike', decay=2.5, damp=0.3, hard=0.6, oct=1, level=0.25) \
 .f1('SVF LP12', 7000, res=0.05, key=0.2).aenv(0.005, 1.6, 0.0, 2.0).curve(a=0.8) \
 .arp('random', 2, '1/8', 0.5, swing=0.15, hold=True) \
 .lfo(1, 0.1, 'sh').mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Rain').mod('m1', 'alevel', 0.5).macro(2, 'Kalimba').mod('m2', 'blevel', 0.5).macro(3, 'Tines').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.06, 0.3).tags('kalimba', 'rain', 'generative', 'arp', 'happy')
X(p, vib=0, press=[('bt1', 0.4)], slide=[('ct2', 0.4)])

p = field(P('Lighthouse Keeper', S), 'a', 'Nature/Ocean Swell', pos=0.2, size=1.2, density=6, level=0.45)
field(p, 'b', 'Nature/Wind Whistle', pos=0.4, size=0.8, density=7, level=0.3)
p.wt('c', 'Ambient/Blue Hour', pos=0.3, blend='spectral', voices=3, detune=7, level=0.4) \
 .f1('SVF LP12', 4500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'ct1', 0.5).mod('lfo1', 'alevel', 0.2) \
 .macro(1, 'Sea').mod('m1', 'alevel', 0.5).macro(2, 'Wind').mod('m2', 'blevel', 0.5).macro(3, 'Light').mod('m3', 'ct1', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.5).tags('ocean', 'solitude', 'corbacho', 'night')
X(p, press=[('clevel', 0.35)], slide=[('ct1', 0.5)])

p = field(P('Abyssal Plain', S), 'a', 'Drones/Sub Rumble Field', pos=0.3, size=1.2, density=6, level=0.45)
field(p, 'b', 'Nature/Bubbling Spring', pos=0.3, size=0.5, density=5, spray=0.7, level=0.2)
p.wt('c', 'Drone/Abyssal', pos=0.3, blend='spectral', voices=3, detune=7, oct=-1, level=0.45) \
 .f1('Ladder LP24', 500, res=0.2, key=0.2).aenv(*VLONG).curve(a=-0.5) \
 .lfo(1, 0.008, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'Pressure').mod('m1', 'alevel', 0.5).macro(2, 'Bubbles').mod('m2', 'blevel', 0.5).macro(3, 'Abyss').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.03, 0.4).tags('deep', 'ocean', 'roach', 'dark', 'sub')
X(p, vib=0, press=[('f1cut', 0.35)], slide=[('ct1', 0.5)])
