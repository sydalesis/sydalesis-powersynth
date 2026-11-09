# Driftfield 5.0 bank: SPECTRAL — the spectral oscillator resynthesising the library's recordings
# (keyed resynthesis, time-stretch, freeze, smear, partial shift / stretch, formant keeping, cross-synthesis)
# and the Spectral FX freezing / smearing the other engines. No reverb, no delay.
SPC = 'Spectral'

def spX(p, o='a', press=('at2', 0.4), slide=('at1', 0.25), vib=0.0):
    """MPE: pressure blurs (or what `press` names), slide moves the position"""
    return X(p, vib=vib, pa=0.25, pc=0.15, sc=0.1, st=0.0, press=[press] if press else [], slide=[slide] if slide else [])

def spM(p, o='a', a=('Position', 'at1', 0.5), b=('Blur', 'at2', 0.6), c=('Shift', 'at3', 0.25), d=('Tone', 'f1cut', 0.45)):
    for i, (n, dst, amt) in enumerate((a, b, c, d), 1):
        if n: p.macro(i, n).mod('m%d' % i, dst, amt)
    return p

def sp(name, src, cross=None, mode='off', amount=0.5, aenv=(2.5, 3, 1, 7), cut=8000, chorus=('ensemble', 0.15, 0.05, 0.4), tags=(), macros=None, press=('at2', 0.4), level=0.75, **kw):
    p = P(name, SPC).spec('a', src, cross=cross, mode=mode, amount=amount, level=level, **kw).off('b', 'c')
    p.f1('SVF LP12', cut, res=0.05, key=0.1).set('f1Drive', 0.0).aenv(*aenv)
    if chorus: p.chorus(*chorus)
    else: p.nochorus()
    if macros: macros(p)
    else: spM(p)
    p.tags('spectral', *tags)
    spX(p, press=press)
    return p

# ---------------------------------------------------------------- keyed resynthesis, time-stretched
sp('Spectral Choir Ah', 'Voices/Choir Ah', speed=0.5, blur=0.3, formant=1.0, tags=('choir', 'formant', 'stretched'))
sp('Monk Hum Stretched', 'Voices/Monk Hum Low', speed=0.2, formant=0.5, level=1.0, blur=0.5, tags=('monk', 'ritual', 'stretched'))
sp('Throat Chant Keys', 'Voices/Throat Chant', speed=0.6, formant=1.0, sparse=0.3, tags=('throat', 'overtone'))
sp('Overtone Drone Glass', 'Voices/Overtone Drone', speed=0.3, sparse=0.7, tilt=0.3, blur=1.0, tags=('overtone', 'glass', 'sparse'))
sp('Whisper Spectra', 'Voices/Whisper Air', speed=0.4, blur=2.0, tilt=-0.2, tags=('whisper', 'air', 'breath'))
sp('Shruti Spectral Bed', 'Drones/Shruti Box', speed=0.25, blur=1.5, tags=('shruti', 'drone', 'raga'))
sp('Harmonium Glide', 'Drones/Harmonium Reed', speed=0.5, formant=0.6, tags=('harmonium', 'reed'))
sp('Choir Vowel Slow Motion', 'Voices/Choir Vowel Drift', speed=0.15, formant=0.3, level=1.0, blur=2.0, tags=('choir', 'vowel', 'slow'))

# ---------------------------------------------------------------- freeze
sp('Frozen Cello', 'Strings/Bowed Cello Drone', pos=0.4, freeze=True, blur=0.5, tags=('cello', 'freeze', 'strings'))
sp('Frozen Bowl Halo', 'Metal/Singing Bowl Pair', pos=0.2, freeze=True, sparse=0.5, blur=0.8, tags=('bowl', 'freeze', 'halo'))
sp('Frozen Rain Chord', 'Nature/Rain on Leaves', pos=0.5, freeze=True, blur=1.5, tilt=-0.3, tags=('rain', 'freeze', 'noise-chord'))
sp('Frozen Choir Cloud', 'Voices/Choir Cloud', pos=0.35, freeze=True, blur=3.0, formant=0.7, tags=('choir', 'freeze', 'cloud'))
sp('Ice Creak Freeze', 'Nature/Ice Creaks', pos=0.3, freeze=True, sparse=0.6, blur=1.0, tags=('ice', 'freeze', 'sparse'))
sp('Frozen Breath Choir', 'Voices/Breath Choir', pos=0.45, freeze=True, blur=2.0, tilt=-0.2, tags=('breath', 'freeze'))

# ---------------------------------------------------------------- smear / stretch
sp('Ocean Swell Smear', 'Nature/Ocean Swell', speed=0.1, blur=6.0, keyed=False, aenv=(4, 4, 1, 10), tags=('ocean', 'smear', 'slow'))
sp('Thunder Bloom', 'Nature/Distant Thunder', speed=0.15, blur=4.0, tilt=-0.4, keyed=False, oct=-1, aenv=(3, 4, 1, 10), tags=('thunder', 'smear', 'dark'))
sp('Fire Spectrum', 'Nature/Fire Crackle', speed=0.3, sparse=0.5, level=1.0, blur=1.5, keyed=False, tags=('fire', 'sparse', 'texture'))
sp('Cicada Stretch', 'Nature/Cicadas', speed=0.05, blur=3.0, keyed=False, tags=('cicadas', 'smear', 'summer'))
sp('Stream Glass', 'Nature/Mountain Stream', speed=0.5, sparse=0.8, tilt=0.3, blur=0.5, tags=('stream', 'glass', 'sparse'))
sp('Bell Stretch Pad', 'Metal/Bell Tree', speed=0.2, stretch=-0.5, blur=1.0, tags=('bells', 'stretched'))
sp('Gamelan Smear', 'Metal/Gamelan Pulse', speed=0.3, blur=2.0, tags=('gamelan', 'smear'))

# ---------------------------------------------------------------- shift / stretch (inharmonic)
sp('Inharmonic Choir', 'Voices/Choir Oo', speed=0.4, stretch=0.6, formant=0.3, level=1.0, tags=('choir', 'inharmonic', 'stretch'))
sp('Shifted Sympathy', 'Strings/Sympathetic Drone', speed=0.3, shift=60.0, blur=0.5, tags=('strings', 'inharmonic', 'shifted'))
sp('Metal Groan Shifted', 'Metal/Metal Groan', speed=0.25, shift=-35.0, level=1.0, blur=1.0, keyed=False, tags=('metal', 'shifted', 'dark'))

# ---------------------------------------------------------------- cross-synthesis
def xM(p):
    return spM(p, a=('Morph', 'at4', 0.5), b=('Blur', 'at2', 0.6), c=('Position', 'at1', 0.4), d=('Tone', 'f1cut', 0.45))
sp('Rain Choir', 'Nature/Rain Light', cross='Voices/Choir Ah', mode='vocode', amount=1.0, speed=0.5, keyed=False, macros=xM, tags=('rain', 'choir', 'vocode'))
sp('Wind Speaks', 'Nature/Wind Bands', cross='Voices/Choir Vowel Drift', mode='vocode', amount=0.8, speed=0.4, keyed=False, macros=xM, tags=('wind', 'vowel', 'vocode'))
sp('Choir into Bowl', 'Voices/Choir Oo', cross='Metal/Bowed Bowl', mode='morph', amount=0.3, speed=0.3, macros=xM, tags=('choir', 'bowl', 'morph')) \
  .lfo(1, 0.02, 'sine').mod('lfo1', 'at4', 0.3)
sp('Cello into Throat', 'Strings/Bowed Cello Drone', cross='Voices/Throat Chant', mode='morph', amount=0.5, speed=0.3, macros=xM, tags=('cello', 'throat', 'morph'))
sp('Ocean Through the Organ', 'Nature/Ocean Swell', cross='Drones/Organ Cluster', mode='vocode', amount=1.0, speed=0.2, keyed=False, blur=1.0, macros=xM, tags=('ocean', 'organ', 'vocode'))
sp('Fire Choir', 'Nature/Fire Crackle', cross='Voices/Choir Cloud', mode='vocode', amount=0.9, speed=0.4, keyed=False, macros=xM, tags=('fire', 'choir', 'vocode'))
sp('Tanpura Ghost', 'Strings/Tanpura', cross='Voices/Overtone Drone', mode='morph', amount=0.4, speed=0.3, level=1.0, macros=xM, tags=('tanpura', 'overtone', 'morph')) \
  .lfo(1, 0.013, 'tri').mod('lfo1', 'at4', 0.35)
sp('Stone Voice', 'Organic/Stone Scrape', cross='Voices/Monk Hum Low', mode='vocode', amount=1.0, speed=0.3, keyed=False, macros=xM, tags=('stone', 'monk', 'vocode'))

# ---------------------------------------------------------------- Spectral FX on the other engines
p = P('Smeared Analog Pad', SPC).an('a', 'saw', voices=3, detune=10, slop=0.4, level=0.5).an('b', 'pulse', pw=0.4, fine=6, voices=2, level=0.4).off('c') \
 .f1('OB-Xa LP12', 1800, res=0.15, key=0.35, env=0.2).aenv(1.5, 3, 0.9, 4).chorus('chorus', 0.2, 0.2, 0.4).sfx(mix=0.55, blur=6.0) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.45).macro(2, 'Smear').mod('m2', 'fxspecblur', 0.5).macro(3, 'Wet').mod('m3', 'fxspecmix', 0.4).macro(4, 'Detune').mod('m4', 'detune', 0.3) \
 .tags('analog', 'smear', 'spectral-fx')
X(p)
p = P('Spectral Shimmer Glass', SPC).add('a', 'Glass', 'Bell', morph=0.3, partials=64, shim=0.3, level=0.5).off('b', 'c') \
 .f1('SVF LP12', 6000, key=0.2).aenv(2, 3, 1, 6).sfx(mix=0.35, blur=3.0, shift=12.0, spread=0.6) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.4).macro(2, 'Shimmer').mod('m2', 'fxspecmix', 0.4).macro(3, 'Smear').mod('m3', 'fxspecblur', 0.4).macro(4, 'Morph').mod('m4', 'at1', 0.5) \
 .tags('glass', 'shimmer', 'spectral-fx')
X(p)
p = string_machine('Smeared Strings', cut=4200, mode='solina', mix=0.4)
p.cat = SPC
p.sfx(mix=0.6, blur=10.0, spread=0.5).tags('strings', 'smear', 'spectral-fx')
p = field(P('Granular Smear Field', SPC), 'a', 'Nature/Rain on Leaves', pos=0.2, scan=0.02, size=0.6, density=14, level=1.0)
p.off('b', 'c').f1('SVF LP12', 7000, key=0.0).aenv(*LONG).sfx(mix=0.7, blur=8.0, gate=0.15, spread=0.7) \
 .macro(1, 'Rain').mod('m1', 'alevel', 0.4).macro(2, 'Smear').mod('m2', 'fxspecblur', 0.5).macro(3, 'Wet').mod('m3', 'fxspecmix', 0.3).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
 .tags('rain', 'smear', 'spectral-fx')
X(p, vib=0.0)

# ---------------------------------------------------------------- performance
p = sp('Spectral Tide', 'Nature/Ocean Swell', speed=0.05, blur=2.0, keyed=False, aenv=(4, 4, 1, 10), tags=('motion', 'ocean'))
p.motion(20.0, {1: lambda t: 0.5 + 0.4 * _m.sin(2 * _m.pi * t), 2: lambda t: 0.3 + 0.3 * _m.sin(2 * _m.pi * t * 2 + 1)})
p = sp('Scrub with Pressure', 'Voices/Breath Choir', speed=0.0, pos=0.3, blur=0.4, formant=0.8, press=('at1', 0.6), tags=('mpe', 'scrub', 'breath'))
p = sp('Scene Morph Spectra', 'Voices/Choir Oo', cross='Metal/Singing Bowl Pair', mode='morph', amount=0.0, speed=0.3, macros=xM, tags=('scenes', 'morph'))
p.scenes([[0.0, 0.0, 0.3, 0.6], [0.5, 0.4, 0.5, 0.5], [1.0, 0.8, 0.7, 0.4]], glide=4.0)
