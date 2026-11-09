# Driftfield 4.2 bank (v4 phase C): WINDS & REEDS (reed / flute models), PHYSICAL 2.0 (pluck, mallet,
# sample exciters, gong bloom, tension), ANALYSED BODIES (modal banks measured from the library's
# recordings), RESONATOR patches (sympathetic strings tuned to the scale or to the held notes), and
# PERFORMANCE presets built on scenes, motion lanes and the four MSEGs.
# No reverb (Space stays off) and no delay, as everywhere in the factory bank.
import math as _m

W = 'Winds & Reeds'
PF = 'Performance'

def windX(p, vib=0.014, o='a', press=0.35):
    """breath on pressure: MPE pressure (or Studio One's Expression lane) blows harder"""
    return X(p, vib=vib, pa=0.15, pc=0.2, sc=0.15, st=0.0, vel=0.3, press=[(o + 'press', press)], slide=[(o + 't1', 0.3)])

def exsmp(p, o, name):
    """the oscillator's grain sample, used by the Sample exciter"""
    p.st['sm' + o.upper()] = 'smp:' + name + '.wav'
    return p

def wm(p, breath='apress', reed='at1', bell='at4'):
    p.macro(1, 'Breath').mod('m1', breath, 0.3).macro(2, 'Embouchure').mod('m2', reed, 0.4) \
     .macro(3, 'Bell').mod('m3', bell, 0.5).macro(4, 'Air').mod('m4', 'f1cut', 0.4)
    return p

# =============================================================================================== WINDS & REEDS
p = P('Low Clarinet Drone', W).ph('a', 'Reed', 'Breath', damp=0.45, hard=0.5, pos=0.3, level=0.7).ph2('a', press=0.55, noise=0.05).off('b', 'c')
wm(p.f1('SVF LP12', 3500, key=0.3).aenv(0.25, 2, 1, 1.2).voice('legato', 0.05)).chorus('chorus', 0.12, 0.05, 0.3).tags('clarinet', 'reed', 'drone', 'breath')
windX(p)

p = P('Breathy Bass Clarinet', W).ph('a', 'Reed', 'Breath', damp=0.55, hard=0.45, pos=0.35, level=0.7, oct=-1).ph2('a', press=0.5, noise=0.22).off('b', 'c')
wm(p.f1('SVF LP12', 2400, key=0.25).aenv(0.18, 2, 1, 1.0).voice('legato', 0.06)).tags('clarinet', 'reed', 'bass', 'breathy')
windX(p, vib=0.01)

p = P('Reed Organ Chant', W).ph('a', 'Reed', 'Breath', damp=0.4, hard=0.55, pos=0.28, level=0.55).ph2('a', press=0.58, noise=0.04)
p.ph('b', 'Reed', 'Breath', damp=0.45, hard=0.5, pos=0.32, level=0.45, semi=7, fine=3).ph2('b', press=0.55, noise=0.04).off('c')
wm(p.f1('SVF LP12', 3000, key=0.3).aenv(0.6, 2, 1, 2.2), breath='timbre').chorus('ensemble', 0.25, 0.06, 0.45).tags('reed', 'organ', 'fifths', 'chant')
p.lfo(1, 0.05, 'drift').mod('lfo1', 'apress', 0.08).mod('lfo1', 'bpress', -0.08)
windX(p, press=0.3)

p = P('Choked Reed Growl', W).ph('a', 'Reed', 'Breath', damp=0.3, hard=0.65, pos=0.22, level=0.65, oct=-1).ph2('a', press=0.78, noise=0.1).off('b', 'c')
wm(p.f1('Ladder LP24', 1800, res=0.25, key=0.3).aenv(0.05, 1, 1, 0.6).voice('legato', 0.04)).drive(0.3, 0.6).tags('reed', 'growl', 'overblown', 'aggressive')
windX(p, press=0.25, vib=0.0)

p = P('Reed Swarm', W).ph('a', 'Reed', 'Breath', damp=0.45, hard=0.5, pos=0.3, level=0.45).ph2('a', press=0.55, noise=0.06)
p.ph('b', 'Reed', 'Breath', damp=0.5, hard=0.5, pos=0.34, level=0.4, fine=9, pan=-0.5).ph2('b', press=0.53, noise=0.06)
p.ph('c', 'Reed', 'Breath', damp=0.5, hard=0.45, pos=0.26, level=0.4, fine=-8, oct=-1, pan=0.5).ph2('c', press=0.56, noise=0.06)
wm(p.f1('SVF LP12', 2800, key=0.25).aenv(1.2, 3, 1, 3.5), breath='timbre').chorus('ensemble', 0.3, 0.05, 0.5).tags('reed', 'swarm', 'drone', 'wide')
p.lfo(1, 0.03, 'drift', 'voice').lfo(2, 0.045, 'drift', 'voice').mod('lfo1', 'apress', 0.1).mod('lfo2', 'cpress', 0.1)
windX(p, press=0.25)

p = P('Shakuhachi Breath', W).ph('a', 'Flute', 'Breath', damp=0.45, pos=0.25, level=0.7).ph2('a', press=0.52, noise=0.35).off('b', 'c')
wm(p.f1('SVF LP12', 5200, key=0.3).aenv(0.12, 2, 1, 1.4).voice('legato', 0.08)).chorus('chorus', 0.1, 0.05, 0.3).tags('flute', 'shakuhachi', 'breathy', 'japan')
windX(p, vib=0.02)

p = P('Bamboo Flute', W).ph('a', 'Flute', 'Breath', damp=0.35, pos=0.3, level=0.7).ph2('a', press=0.56, noise=0.1).off('b', 'c')
wm(p.f1('SVF LP12', 6500, key=0.3).aenv(0.06, 1.5, 1, 0.8).voice('legato', 0.05)).tags('flute', 'bamboo', 'lead')
windX(p, vib=0.024)

p = P('Overblown Flute Harmonics', W).ph('a', 'Flute', 'Breath', damp=0.3, pos=0.14, level=0.65).ph2('a', press=0.62, noise=0.12).off('b', 'c')
p.lfo(1, 0.08, 'tri').mod('lfo1', 'at1', 0.12)
wm(p.f1('SVF LP12', 7000, key=0.3).aenv(0.2, 2, 1, 1.5)).chorus('chorus', 0.15, 0.05, 0.35).tags('flute', 'overblown', 'harmonics', 'evolving')
windX(p, vib=0.0)

p = P('Ney Drone', W).ph('a', 'Flute', 'Breath', damp=0.5, pos=0.32, level=0.7, oct=-1).ph2('a', press=0.5, noise=0.28).off('b', 'c')
p.lfo(1, 0.04, 'drift').mod('lfo1', 'apress', 0.1)
wm(p.f1('SVF LP12', 4200, key=0.2).aenv(0.8, 3, 1, 3)).tags('flute', 'ney', 'drone', 'sufi')
windX(p, vib=0.018)

p = P('Cave Ocarina', W).ph('a', 'Flute', 'Breath', damp=0.6, pos=0.45, level=0.7).ph2('a', press=0.54, noise=0.05).off('b', 'c')
wm(p.f1('SVF LP12', 4000, key=0.3).aenv(0.08, 1.5, 1, 1.2)).reso('octaves', 'C', 2, mix=0.25, decay=5, bright=0.5).tags('ocarina', 'cave', 'resonator')
windX(p, vib=0.016)

p = P('Breath Organ', W).ph('a', 'Flute', 'Breath', damp=0.4, pos=0.3, level=0.5).ph2('a', press=0.55, noise=0.08)
p.ph('b', 'Flute', 'Breath', damp=0.45, pos=0.28, level=0.35, oct=1, fine=4).ph2('b', press=0.52, noise=0.08)
p.ph('c', 'Reed', 'Breath', damp=0.55, hard=0.45, pos=0.35, level=0.3, oct=-1).ph2('c', press=0.52, noise=0.03)
wm(p.f1('SVF LP12', 4500, key=0.25).aenv(0.5, 2, 1, 2), breath='timbre').chorus('ensemble', 0.2, 0.05, 0.4).tags('organ', 'breath', 'flute', 'reed')
windX(p, press=0.25, vib=0.0)

p = P('Reed and Strings Sympathy', W).ph('a', 'Reed', 'Breath', damp=0.45, hard=0.5, pos=0.3, level=0.65).ph2('a', press=0.55, noise=0.06).off('b', 'c')
wm(p.f1('SVF LP12', 3600, key=0.3).aenv(0.3, 2, 1, 1.5)).reso('held', 'C', 2, mix=0.45, decay=7, bright=0.65).tags('reed', 'resonator', 'sympathetic')
windX(p)

p = P('Bass Reed Pedal', W).ph('a', 'Reed', 'Breath', damp=0.55, hard=0.45, pos=0.3, level=0.7, oct=-2).ph2('a', press=0.5, noise=0.05).off('b', 'c')
wm(p.f1('SVF LP12', 1600, key=0.2).aenv(0.2, 2, 1, 1.2).voice('legato', 0.08)).tags('reed', 'bass', 'pedal')
windX(p, vib=0.0)

p = P('Flute Choir Fifths', W).ph('a', 'Flute', 'Breath', damp=0.4, pos=0.3, level=0.45).ph2('a', press=0.55, noise=0.12)
p.ph('b', 'Flute', 'Breath', damp=0.4, pos=0.29, level=0.4, semi=7, fine=-3, pan=-0.4).ph2('b', press=0.55, noise=0.12)
p.ph('c', 'Flute', 'Breath', damp=0.45, pos=0.31, level=0.35, oct=1, fine=3, pan=0.4).ph2('c', press=0.53, noise=0.12)
wm(p.f1('SVF LP12', 5000, key=0.25).aenv(1.0, 2, 1, 2.5), breath='timbre').chorus('ensemble', 0.3, 0.05, 0.45).tags('flute', 'choir', 'fifths', 'pad')
windX(p, press=0.25)

# =============================================================================================== PHYSICAL 2.0
p = P('Nylon Pluck', PM).ph('a', 'String', 'Pluck', decay=4, damp=0.4, hard=0.55, pos=0.18, level=0.65).off('b', 'c')
pmMacros(p.aenv(0.002, 4, 0.0, 3).curve(a=1)).chorus('chorus', 0.1, 0.05, 0.3).tags('string', 'plucked', 'nylon', 'guitar')
pmX(p)

p = P('Felt Hammer Strings', PM).ph('a', 'String', 'Mallet', decay=6, damp=0.45, hard=0.25, pos=0.12, couple=0.4, level=0.65).off('b', 'c')
pmMacros(p.aenv(0.002, 6, 0.0, 4).curve(a=1)).chorus('chorus', 0.12, 0.05, 0.3).tags('felt', 'piano', 'hammer', 'soft')
pmX(p)

p = P('Marimba Hall', PM).ph('a', 'Bar', 'Mallet', decay=1.8, damp=0.4, hard=0.45, pos=0.25, level=0.7).off('b', 'c')
pmMacros(p.aenv(0.001, 2, 0.0, 2).curve(a=1)).tags('marimba', 'mallet', 'wood')
pmX(p)

p = P('Tension Koto', PM).ph('a', 'Waveguide', 'Pluck', decay=4, damp=0.35, hard=0.6, pos=0.2, spread=0.5, level=0.6).ph2('a', nonlin=0.6).off('b', 'c')
pmMacros(p.aenv(0.002, 4, 0.0, 3).curve(a=1)).tags('koto', 'plucked', 'tension', 'japan')
pmX(p)

p = P('Tanpura Pluck Cycle', PM).ph('a', 'Tanpura', 'Pluck', decay=9, damp=0.3, hard=0.45, pos=0.3, buzz=0.6, level=0.6).off('b', 'c')
pmMacros(p.aenv(0.002, 9, 0.0, 6).curve(a=1)).tags('tanpura', 'raga', 'jawari', 'plucked')
pmX(p)

p = exsmp(P('Stream Through a Bowl', PM).ph('a', 'Glass Bowl', 'Sample', decay=10, damp=0.2, hard=0.4, couple=0.4, level=0.6).ph2('a', press=0.55).off('b', 'c'), 'a', 'Nature/Mountain Stream')
pmMacros(p.aenv(1.5, 3, 1, 6)).tags('bowl', 'field-recording', 'sample-exciter', 'water')
pmX(p)

p = exsmp(P('Rain Through Strings', PM).ph('a', 'Sympathetic', 'Sample', decay=8, damp=0.3, hard=0.5, pos=0.3, buzz=0.7, level=0.55).ph2('a', press=0.6).off('b', 'c'), 'a', 'Nature/Rain Light')
pmMacros(p.aenv(1.5, 3, 1, 6)).chorus('ensemble', 0.15, 0.05, 0.4).tags('rain', 'sympathetic', 'sample-exciter', 'strings')
pmX(p)

p = exsmp(P('Wind Through the Gong', PM).ph('a', 'Gong', 'Sample', decay=14, damp=0.25, hard=0.4, level=0.55, oct=-1).ph2('a', press=0.6, nonlin=0.4).off('b', 'c'), 'a', 'Nature/Wind Bands')
pmMacros(p.aenv(2, 3, 1, 8).set('quality', 2)).tags('gong', 'wind', 'sample-exciter', 'ritual')
pmX(p)

p = P('Bowed Gong Bloom', PM).ph('a', 'Gong', 'Bow', decay=12, damp=0.2, hard=0.4, level=0.55, oct=-1).ph2('a', press=0.5, nonlin=0.7).off('b', 'c')
pmMacros(p.aenv(1.5, 3, 1, 8).set('quality', 2)).tags('gong', 'bowed', 'bloom', 'ritual')
pmX(p)

p = P('Tam-Tam Swell', PM).ph('a', 'Gong', 'Mallet', decay=15, damp=0.15, hard=0.2, level=0.6, oct=-1).ph2('a', nonlin=0.9).off('b', 'c')
pmMacros(p.aenv(0.002, 15, 0.0, 10).curve(a=1).set('quality', 2)).tags('gong', 'tam-tam', 'bloom', 'cinematic')
pmX(p)

p = P('Plate Bloom Bell', PM).ph('a', 'Plate', 'Mallet', decay=8, damp=0.25, hard=0.5, level=0.6).ph2('a', nonlin=0.5).off('b', 'c')
pmMacros(p.aenv(0.002, 8, 0.0, 6).curve(a=1)).tags('plate', 'bell', 'bloom')
pmX(p)

p = P('Mallet on Glass Bowl', PM).ph('a', 'Glass Bowl', 'Mallet', decay=12, damp=0.15, hard=0.3, couple=0.5, level=0.6).off('b', 'c')
pmMacros(p.aenv(0.002, 12, 0.0, 8).curve(a=1)).tags('bowl', 'mallet', 'singing-bowl')
pmX(p)

# =============================================================================================== ANALYSED BODIES
BR = 'Bells & Resonators'
def analysed(name, modes, exc='Strike', decay=5, damp=0.15, hard=0.5, oct=0, nonlin=None, aenv=(0.002, 10, 0.0, 8), sample=None, press=None, tags=(), cat=BR):
    p = P(name, cat).ph('a', 'Analysed', exc, decay=decay, damp=damp, hard=hard, level=0.65, oct=oct).modes('a', modes).off('b', 'c')
    if nonlin is not None: p.ph2('a', nonlin=nonlin)
    if press is not None: p.ph2('a', press=press)
    if sample: exsmp(p, 'a', sample)
    pmMacros(p.aenv(*aenv).curve(a=1 if aenv[0] < 0.01 else 0)).tags('analysed', 'modal', *tags)
    pmX(p)
    return p

analysed('Singing Bowl (Analysed)', 'Singing Bowl Pair', tags=('bowl', 'singing-bowl'))
analysed('Bowl Pair Bowed (Analysed)', 'Singing Bowl Pair', exc='Bow', decay=6, aenv=(1.2, 3, 1, 8), cat=PM, tags=('bowl', 'bowed'))
analysed('Gong Swell (Analysed)', 'Gong Swell', exc='Mallet', decay=5, hard=0.25, oct=-1, nonlin=0.4, aenv=(0.002, 14, 0.0, 10), tags=('gong', 'ritual'))
analysed('Bell Tree Shimmer (Analysed)', 'Bell Tree', decay=4, hard=0.7, aenv=(0.002, 6, 0.0, 5), tags=('bell-tree', 'shimmer'))
analysed('Cymbal Wash (Analysed)', 'Bowed Cymbal', exc='Scrape', decay=3, hard=0.4, aenv=(1.0, 3, 1, 5), cat=PM, tags=('cymbal', 'scraped', 'wash'))
analysed('Glass Harmonics (Analysed)', 'Glass Harmonics', exc='Bow', decay=3, aenv=(0.8, 3, 1, 5), cat=PM, tags=('glass', 'bowed'))
analysed('Scraped Plate Body (Analysed)', 'Scraped Plate', exc='Rain', decay=2, hard=0.3, oct=-1, aenv=(1.5, 3, 1, 6), cat=PM, tags=('plate', 'rain', 'texture'))
analysed('Wind Chime Body (Analysed)', 'Wind Chimes', exc='Mallet', decay=5, hard=0.6, aenv=(0.002, 8, 0.0, 6), tags=('chimes',))
analysed('Bowl Under the Stream (Analysed)', 'Bowed Bowl', exc='Sample', decay=4, sample='Nature/Bubbling Spring', press=0.6, aenv=(1.5, 3, 1, 6), cat=PM, tags=('bowl', 'water', 'sample-exciter'))

# =============================================================================================== RESONATOR PATCHES
p = field(P('Aeolian Harp', 'Atmospheres'), 'a', 'Nature/Wind Whistle', pos=0.3, scan=0.02, size=0.8, density=12, level=0.6)
p.off('b', 'c').f1('SVF LP12', 6000, key=0.0).aenv(*LONG).reso('fifths', 'D', 2, mix=0.75, decay=9, bright=0.7) \
 .macro(1, 'Wind').mod('m1', 'alevel', 0.4).macro(2, 'Strings').mod('m2', 'fxreso', 0.3).macro(3, 'Gust').mod('m3', 'at4', 0.4).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
 .tags('aeolian', 'wind', 'resonator', 'harp')
X(p, vib=0.0)

p = field(P('Rain Harp', 'Textures'), 'a', 'Nature/Rain Light', pos=0.2, scan=0.02, size=0.5, density=16, level=0.6)
p.off('b', 'c').f1('SVF HP12', 300, key=0.0).aenv(*LONG).reso('minor', 'A', 2, mix=0.7, decay=6, bright=0.75) \
 .macro(1, 'Rain').mod('m1', 'alevel', 0.4).macro(2, 'Strings').mod('m2', 'fxreso', 0.3).macro(3, 'Density').mod('m3', 'at3', 0.5).macro(4, 'Air').mod('m4', 'f1cut', -0.3) \
 .tags('rain', 'resonator', 'harp', 'minor')
X(p, vib=0.0)

p = P('Held-Note Halo', 'Pads').an('a', 'saw', voices=3, detune=10, slop=0.4, level=0.45).an('b', 'pulse', pw=0.4, fine=6, voices=2, level=0.35).off('c') \
 .f1('OB-Xa LP12', 1700, res=0.15, key=0.35, env=0.2).aenv(1.5, 3, 0.9, 4).reso('held', 'C', 2, mix=0.5, decay=8, bright=0.6) \
 .chorus('chorus', 0.2, 0.2, 0.4).macro(1, 'Tone').mod('m1', 'f1cut', 0.4).macro(2, 'Halo').mod('m2', 'fxreso', 0.4).macro(3, 'Detune').mod('m3', 'detune', 0.3).macro(4, 'Motion').mod('m4', 'at1', 0.3) \
 .tags('halo', 'resonator', 'held-notes', 'analog')
X(p)

p = P('Sitar Sympathy', PM).ph('a', 'Waveguide', 'Pluck', decay=5, damp=0.25, hard=0.65, pos=0.15, level=0.55).ph2('a', nonlin=0.35).off('b', 'c') \
 .reso('tanpura', 'C', 2, mix=0.5, decay=9, bright=0.85)
pmMacros(p.aenv(0.002, 5, 0.0, 4).curve(a=1)).tags('sitar', 'raga', 'resonator', 'plucked')
pmX(p)

p = P('Fifths Resonance Drone', 'Drones').an('a', 'saw', voices=2, detune=8, slop=0.5, level=0.5).an('b', 'saw', oct=-1, fine=4, level=0.4).off('c') \
 .f1('Ladder LP24', 900, res=0.2, key=0.3).aenv(4, 4, 1, 12).reso('fifths', 'D', 2, mix=0.55, decay=12, bright=0.55) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.45).macro(2, 'Resonance').mod('m2', 'fxreso', 0.4).macro(3, 'Drift').mod('m3', 'detune', 0.4).macro(4, 'Pulse').mod('m4', 'lfo1rate', 0.5) \
 .tags('drone', 'fifths', 'resonator', 'roach')
X(p, vib=0.0)

p = P('JI Sympathetic Field', 'Just Intonation').add('a', 'Strings', 'Choir Ah', morph=0.3, partials=64, shim=0.2, level=0.55).off('b', 'c') \
 .f1('SVF LP12', 4000, key=0.2).aenv(3, 3, 1, 9).tune('JI 5-limit Major', 'D').reso('major', 'D', 2, mix=0.55, decay=10, bright=0.6) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.4).macro(2, 'Strings').mod('m2', 'fxreso', 0.4).macro(3, 'Morph').mod('m3', 'at1', 0.5).macro(4, 'Shimmer').mod('m4', 'at4', 0.3) \
 .tags('just-intonation', 'resonator', 'sympathetic')
X(p, vib=0.0)

p = P('Octave Bloom Drone', 'Drones').add('a', 'Organ', 'Octaves', morph=0.5, partials=48, shim=0.3, level=0.55, oct=-1).off('b', 'c') \
 .f1('SVF LP12', 3000, key=0.2).aenv(5, 4, 1, 14).reso('octaves', 'C', 1, mix=0.6, decay=14, bright=0.5) \
 .lfo(1, 0.015, 'sine').mod('lfo1', 'fxreso', 0.25) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.45).macro(2, 'Bloom').mod('m2', 'fxreso', 0.4).macro(3, 'Morph').mod('m3', 'at1', 0.5).macro(4, 'Air').mod('m4', 'noise', 0.2) \
 .tags('drone', 'octaves', 'resonator', 'bloom')
X(p, vib=0.0)

p = field(P('Stream Through Strings', 'Soundscapes'), 'a', 'Nature/Mountain Stream', pos=0.3, scan=0.02, size=0.7, density=12, level=0.55)
p.add('b', 'Choir Oo', 'Organ', morph=0.2, partials=48, level=0.35, oct=-1).off('c').f1('SVF LP12', 5500, key=0.1).aenv(*LONG) \
 .reso('held', 'C', 2, mix=0.6, decay=8, bright=0.7) \
 .macro(1, 'Water').mod('m1', 'alevel', 0.4).macro(2, 'Strings').mod('m2', 'fxreso', 0.35).macro(3, 'Choir').mod('m3', 'blevel', 0.4).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
 .tags('stream', 'water', 'resonator', 'held-notes')
X(p, vib=0.0)

p = P('Tanpura Box Drone', 'Drones').ph('a', 'Tanpura', 'Bow', decay=10, damp=0.25, hard=0.4, pos=0.3, buzz=0.55, level=0.55).ph2('a', press=0.45).off('b', 'c') \
 .aenv(2, 3, 1, 8).reso('tanpura', 'C', 2, mix=0.45, decay=11, bright=0.7)
pmMacros(p).tags('tanpura', 'drone', 'resonator', 'raga')
pmX(p)

p = P('Minor Strings Shimmer', 'Atmospheres').wt('a', 'Ambient/Ghost Strings', pos=0.4, blend='spectral', voices=3, detune=10, level=0.5).off('b', 'c') \
 .f1('SVF LP12', 5000, key=0.2).aenv(2.5, 3, 1, 7).reso('minor', 'E', 3, mix=0.5, decay=8, bright=0.9) \
 .lfo(1, 0.025, 'tri').mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.4).macro(2, 'Shimmer').mod('m2', 'fxreso', 0.4).macro(3, 'Position').mod('m3', 'at1', 0.4).macro(4, 'Detune').mod('m4', 'detune', 0.3) \
 .tags('strings', 'minor', 'resonator', 'shimmer')
X(p)

# =============================================================================================== PERFORMANCE
def tri(t, ph=0.0): x = (t + ph) % 1.0; return 1 - abs(2 * x - 1)
def sn(t, cyc=1.0, ph=0.0): return 0.5 + 0.5 * _m.sin(2 * _m.pi * (t * cyc + ph))

p = P('Scene Journey: Dawn to Night', PF).wt('a', 'Ambient/Ancient Light', pos=0.3, blend='spectral', voices=3, detune=10, level=0.5) \
 .add('b', 'Choir Oo', 'Glass', morph=0.2, partials=64, shim=0.2, level=0.35).noise(0.05, 0.6).off('c') \
 .f1('SVF LP12', 2400, key=0.25).aenv(3, 3, 1, 8) \
 .macro(1, 'Light').mod('m1', 'f1cut', 0.5).mod('m1', 'at1', 0.3) \
 .macro(2, 'Choir').mod('m2', 'blevel', 0.4).mod('m2', 'bt1', 0.5) \
 .macro(3, 'Air').mod('m3', 'noise', 0.3) \
 .macro(4, 'Depth').sub(0.0).mod('m4', 'sub', 0.5) \
 .macro(5, 'Shimmer').mod('m5', 'fxchorus', 0.4).chorus('ensemble', 0.15, 0.05, 0.45) \
 .macro(6, 'Drift').mod('m6', 'detune', 0.4) \
 .scenes([[0.75, 0.6, 0.5, 0.0, 0.6, 0.2], [0.55, 0.3, 0.2, 0.2, 0.4, 0.3], [0.35, 0.5, 0.3, 0.45, 0.3, 0.5], [0.1, 0.15, 0.6, 0.8, 0.15, 0.7]], glide=4.0) \
 .tags('scenes', 'journey', 'evolving')
p.st['perf'] = '0,1,2,3'
X(p)

p = P('Tide Machine', PF).wt('a', 'Ambient/Ocean of Stars', pos=0.3, blend='spectral', voices=3, detune=12, level=0.5).an('b', 'saw', oct=-1, voices=2, detune=8, level=0.35).off('c') \
 .f1('Ladder LP24', 1200, res=0.2, key=0.3).aenv(2.5, 3, 1, 7).chorus('ensemble', 0.2, 0.05, 0.45) \
 .macro(1, 'Tide').mod('m1', 'f1cut', 0.55).mod('m1', 'at1', 0.3) \
 .macro(2, 'Undertow').mod('m2', 'blevel', 0.4).mod('m2', 'detune', 0.2) \
 .macro(3, 'Foam').mod('m3', 'f1res', 0.3).macro(4, 'Swell').mod('m4', 'amp', 0.2) \
 .motion(16.0, {1: lambda t: sn(t, 1, -0.25) * 0.8 + 0.1, 2: lambda t: sn(t, 2, 0.1) * 0.6 + 0.2}) \
 .mseg(3, [(0, 0, 0), (12, 1, 0.3), (12, 0, -0.3)], loop=(0, 2), mode='free').mod('mseg3', 'timbre', 0.25) \
 .tags('motion', 'tide', 'evolving', 'mseg3')
X(p)

p = field(P('Weather System', PF), 'a', 'Nature/Wind Bands', pos=0.3, scan=0.02, size=0.8, density=12, level=0.5)
p.wt('b', 'Ambient/Monsoon Haze', pos=0.5, blend='spectral', voices=3, detune=10, level=0.4).off('c') \
 .f1('SVF LP12', 3000, key=0.1).aenv(*LONG) \
 .macro(1, 'Wind').mod('m1', 'alevel', 0.45).mod('m1', 'at3', 0.4) \
 .macro(2, 'Cloud').mod('m2', 'bt1', 0.6).macro(3, 'Pressure').mod('m3', 'f1cut', 0.5).macro(4, 'Front').mod('m4', 'detune', 0.4) \
 .motion(60.0, {1: lambda t: 0.5 + 0.3 * _m.sin(2 * _m.pi * t * 3) + 0.15 * _m.sin(2 * _m.pi * t * 7 + 1),
                2: lambda t: 0.5 + 0.35 * _m.sin(2 * _m.pi * t * 2 + 2),
                3: lambda t: 0.45 + 0.3 * _m.sin(2 * _m.pi * t + 0.5) + 0.1 * _m.sin(2 * _m.pi * t * 5)}) \
 .mseg(4, [(0, 0.2, 0), (20, 0.9, 0.2), (25, 0.3, -0.2), (15, 0.2, 0)], loop=(0, 3), mode='free').mod('mseg4', 'balance', 0.3) \
 .tags('motion', 'weather', 'wind', 'evolving')
X(p, vib=0.0)

p = P('Breathing Drone', PF).an('a', 'saw', voices=2, detune=8, slop=0.5, level=0.5).an('b', 'pulse', pw=0.35, oct=-1, level=0.4).off('c') \
 .f1('Ladder LP24', 700, res=0.25, key=0.3).aenv(4, 4, 1, 10).sub(0.15) \
 .macro(1, 'Breath').mod('m1', 'f1cut', 0.5).mod('m1', 'amp', 0.25) \
 .macro(2, 'Chest').mod('m2', 'f1res', 0.3).macro(3, 'Drift').mod('m3', 'detune', 0.4).macro(4, 'Depth').mod('m4', 'sub', 0.4) \
 .motion(8.0, {1: lambda t: (0.5 - 0.5 * _m.cos(2 * _m.pi * t)) ** 1.5 * 0.85 + 0.05, 2: lambda t: 0.3 + 0.2 * _m.sin(2 * _m.pi * t - 1)}) \
 .tags('motion', 'breathing', 'drone')
X(p, vib=0.0)

p = P('Four Seasons', PF).wt('a', 'Ambient/Canyon Glow', pos=0.3, blend='spectral', voices=2, detune=8, level=0.45) \
 .add('b', 'Glass', 'Bell', morph=0.3, partials=64, shim=0.3, level=0.3, oct=1) \
 .an('c', 'saw', oct=-1, voices=2, detune=10, level=0.35) \
 .f1('SVF LP12', 2500, key=0.25).aenv(2.5, 3, 1, 7).chorus('ensemble', 0.2, 0.05, 0.45) \
 .macro(1, 'Warmth').mod('m1', 'clevel', 0.4).mod('m1', 'f1cut', -0.2) \
 .macro(2, 'Frost').mod('m2', 'blevel', 0.45).mod('m2', 'bt2', 0.4) \
 .macro(3, 'Bloom').mod('m3', 'at1', 0.5).mod('m3', 'f1cut', 0.3) \
 .macro(4, 'Rain').noise(0.0, 0.7).mod('m4', 'noise', 0.35) \
 .scenes([[0.3, 0.2, 0.8, 0.5], [0.8, 0.1, 0.6, 0.1], [0.6, 0.3, 0.2, 0.6], [0.1, 0.9, 0.1, 0.2]], glide=6.0) \
 .tags('scenes', 'seasons', 'evolving')
X(p)

p = analysed('Bowl Ritual Motion', 'Singing Bowl Pair', exc='Bow', decay=6, aenv=(1.2, 3, 1, 8), cat=PF, tags=('motion', 'ritual'))
p.reso('fifths', 'A', 2, mix=0.3, decay=8, bright=0.6) \
 .macro(5, 'Strings').mod('m5', 'fxreso', 0.4) \
 .motion(24.0, {1: lambda t: 0.35 + 0.3 * sn(t, 1), 2: lambda t: 0.5 + 0.3 * _m.sin(2 * _m.pi * t * 2 + 1), 5: lambda t: tri(t)})

p = P('Pulse in Bars', PF).an('a', 'saw', voices=2, detune=8, level=0.5).an('b', 'pulse', pw=0.3, fine=5, level=0.4).off('c') \
 .f1('Ladder LP24', 600, res=0.45, key=0.3).aenv(1.5, 2, 1, 4).chorus('chorus', 0.2, 0.2, 0.4) \
 .macro(1, 'Steps').mod('m1', 'f1cut', 0.6).macro(2, 'Accent').mod('m2', 'f1res', 0.3).macro(3, 'Width').mod('m3', 'detune', 0.3).macro(4, 'Tone').mod('m4', 'bt1', 0.4) \
 .motion(8.0, {1: lambda t: [0.2, 0.7, 0.35, 0.9, 0.25, 0.6, 0.45, 1.0][int(t * 8) % 8], 2: lambda t: 0.6 if int(t * 16) % 4 == 0 else 0.2}, mode='host') \
 .tags('motion', 'host-sync', 'rhythmic', 'berlin')
X(p, vib=0.0)

p = P('Note Swell Motion', PF).wt('a', 'Ambient/Altar of Light', pos=0.2, blend='spectral', voices=3, detune=10, level=0.5).off('b', 'c') \
 .f1('SVF LP12', 1500, key=0.25).aenv(1, 3, 1, 5).chorus('ensemble', 0.2, 0.05, 0.45) \
 .macro(1, 'Swell').mod('m1', 'f1cut', 0.6).mod('m1', 'at1', 0.5).macro(2, 'Shine').mod('m2', 'fxchorus', 0.4) \
 .macro(3, 'Drift').mod('m3', 'detune', 0.3).macro(4, 'Tone').mod('m4', 'f1res', 0.2) \
 .motion(12.0, {1: lambda t: min(1.0, t * 2.0) ** 1.4 * (1.0 - max(0.0, t - 0.6) * 1.5), 2: lambda t: min(1.0, t * 1.5)}, mode='note') \
 .tags('motion', 'swell', 'phrase')
X(p)

p = P('XY Glass Field', PF).add('a', 'Glass', 'Bell', morph=0.3, partials=64, shim=0.35, level=0.45) \
 .gr('b', 'wt', table=3, pos=0.4, size=0.25, density=30, spray=0.3, level=0.35).off('c') \
 .f1('SVF LP12', 5000, key=0.2).aenv(1.5, 3, 1, 6).reso('fifths', 'C', 3, mix=0.25, decay=6, bright=0.8) \
 .macro(1, 'Glass').mod('m1', 'at1', 0.6).macro(2, 'Brightness').mod('m2', 'f1cut', 0.5) \
 .macro(3, 'Grains').mod('m3', 'bt3', 0.6).mod('m3', 'blevel', 0.3).macro(4, 'Strings').mod('m4', 'fxreso', 0.45) \
 .scenes([[0.2, 0.4, 0.1, 0.2], [0.7, 0.7, 0.5, 0.3], [0.4, 0.2, 0.8, 0.7]], glide=2.0) \
 .tags('xy', 'scenes', 'glass', 'resonator')
p.st['perf'] = '0,1,2,3'
X(p)

p = P('Scene Morph Strings', PF).an('a', 'saw', voices=2, detune=6, slop=0.3, level=0.5).an('b', 'saw', oct=1, voices=2, detune=6, level=0.3).off('c') \
 .f1('SVF LP12', 3200, res=0.05, key=0.3).f2('SVF HP12', 140).aenv(0.6, 1.5, 1, 2.2).chorus('solina', 0.5, 0.12, 0.6) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.5).macro(2, 'Octave').mod('m2', 'blevel', 0.45).macro(3, 'Ensemble').mod('m3', 'fxchorus', 0.4).macro(4, 'Swell').mod('m4', 'amp', 0.2) \
 .scenes([[0.2, 0.1, 0.3, 0.3], [0.6, 0.5, 0.6, 0.5], [0.9, 0.9, 0.9, 0.7]], glide=3.0) \
 .tags('scenes', 'strings', 'solina')
X(p)

p = P('Reed Choir Scenes', PF).ph('a', 'Reed', 'Breath', damp=0.45, hard=0.5, pos=0.3, level=0.5).ph2('a', press=0.55, noise=0.05)
p.ph('b', 'Flute', 'Breath', damp=0.4, pos=0.3, level=0.4, oct=1).ph2('b', press=0.55, noise=0.1)
p.ph('c', 'Reed', 'Breath', damp=0.55, hard=0.45, pos=0.32, level=0.4, oct=-1).ph2('c', press=0.52, noise=0.04)
p.f1('SVF LP12', 3800, key=0.25).aenv(0.8, 2, 1, 2.5).chorus('ensemble', 0.25, 0.05, 0.45) \
 .macro(1, 'Reed').mod('m1', 'alevel', 0.4).macro(2, 'Flute').mod('m2', 'blevel', 0.4).macro(3, 'Bass').mod('m3', 'clevel', 0.4) \
 .macro(4, 'Breath').mod('m4', 'apress', 0.2).mod('m4', 'bpress', 0.2).mod('m4', 'cpress', 0.2) \
 .scenes([[0.5, 0.1, 0.3, 0.3], [0.2, 0.6, 0.2, 0.5], [0.4, 0.4, 0.7, 0.6]], glide=2.5) \
 .tags('scenes', 'reed', 'flute', 'choir')
windX(p, press=0.25, vib=0.0)

p = P('Gong Motion Bloom', PF).ph('a', 'Gong', 'Bow', decay=12, damp=0.2, hard=0.4, level=0.55, oct=-1).ph2('a', press=0.45, nonlin=0.6).off('b', 'c') \
 .aenv(1.5, 3, 1, 8).set('quality', 2)
pmMacros(p).macro(5, 'Pressure').mod('m5', 'apress', 0.4) \
 .motion(20.0, {5: lambda t: 0.5 + 0.4 * _m.sin(2 * _m.pi * t), 3: lambda t: 0.5 + 0.3 * _m.sin(2 * _m.pi * t * 2 + 2)}) \
 .tags('motion', 'gong', 'bloom')
pmX(p)
