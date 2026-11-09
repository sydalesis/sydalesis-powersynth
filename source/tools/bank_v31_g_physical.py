# Driftfield 3.1 bank, part G: PHYSICAL MODELS — every model and exciter, coupled systems, oscillators
# driving resonators, sympathetic / tanpura strings, waveguides, scraped and rained bodies.
# MPE convention for this category: pressure -> bow/blow force (Hardness), slide -> contact position,
# per-note bend -> pitch; damping lives on a macro so a performance can mute or open the bodies.
PM = 'Physical Models'

def pmX(p, vib=0.0, extra_press=(), extra_slide=()):
    return X(p, vib=vib, pa=0.35, pc=0.15, st=0.0, sc=0.1,
             press=[('at2', 0.45)] + list(extra_press), slide=[('at1', 0.6)] + list(extra_slide))

def pmMacros(p, extra=None):
    p.macro(1, 'Force').mod('m1', 'at2', 0.5).macro(2, 'Position').mod('m2', 'at1', 0.5) \
     .macro(3, 'Damping').mod('m3', 'at4', 0.6).macro(4, 'Decay').mod('m4', 'at3', 0.5)
    return p

# ---------------------------------------------------------------- strings
p = P('Bowed String Solo', PM).ph('a', 'String', 'Bow', decay=6, damp=0.25, hard=0.5, pos=0.12, level=0.65).off('b', 'c')
pmMacros(p.f1('SVF LP12', 7000, key=0.2).aenv(0.15, 2, 0.9, 1.2).voice('legato', 0.04)).chorus('chorus', 0.1, 0.05, 0.3).tags('string', 'bowed', 'solo', 'expressive')
pmX(p, vib=0.022)

p = P('String Quartet Body', PM).ph('a', 'String', 'Bow', decay=6, damp=0.25, hard=0.5, pos=0.15, level=0.5)
p.ph('b', 'String', 'Bow', decay=6, damp=0.28, hard=0.45, pos=0.2, fine=-6, level=0.4, pan=-0.4) \
 .ph('c', 'Bar', 'Osc Input', decay=1.5, damp=0.5, hard=0.3, oct=-1, level=0.2).f1('SVF LP12', 6000, key=0.2).aenv(0.4, 2, 1, 2)
pmMacros(p).chorus('ensemble', 0.2, 0.05, 0.4).tags('string', 'bowed', 'ensemble', 'body-resonance')
pmX(p, vib=0.018)

p = P('Plucked Gut String', PM).ph('a', 'String', 'Strike', decay=3, damp=0.4, hard=0.35, pos=0.18, level=0.65).off('b', 'c')
pmMacros(p.aenv(0.002, 3, 0.0, 2.5).curve(a=1)).chorus('chorus', 0.1, 0.05, 0.3).tags('string', 'plucked', 'harp', 'nylon')
pmX(p)

p = P('Hammered Dulcimer', PM).ph('a', 'String', 'Strike', decay=5, damp=0.15, hard=0.85, pos=0.1, level=0.55)
p.ph('b', 'String', 'Strike', decay=5, damp=0.15, hard=0.85, pos=0.1, fine=4, level=0.4).off('c').aenv(0.002, 5, 0.0, 5).curve(a=1)
pmMacros(p).chorus('chorus', 0.1, 0.05, 0.3).tags('string', 'hammered', 'dulcimer', 'bright')
pmX(p)

p = P('Waveguide Cello', PM).ph('a', 'Waveguide', 'Bow', decay=10, damp=0.2, hard=0.55, pos=0.12, level=0.6).off('b', 'c')
pmMacros(p.f1('SVF LP12', 5000, key=0.3).aenv(0.2, 2, 0.95, 1.5).voice('legato', 0.05)).chorus('chorus', 0.1, 0.05, 0.3).tags('waveguide', 'cello', 'bowed', 'expressive')
pmX(p, vib=0.025)

p = P('Waveguide Flute', PM).ph('a', 'Waveguide', 'Breath', decay=4, damp=0.35, hard=0.4, pos=0.3, level=0.6).noise(0.05, 0.5).off('b', 'c')
pmMacros(p.f1('SVF LP12', 6000, key=0.3).aenv(0.08, 1, 0.9, 0.5).voice('legato', 0.05)).chorus('chorus', 0.1, 0.05, 0.3).tags('waveguide', 'flute', 'blown', 'expressive')
pmX(p, vib=0.02, extra_press=[('noise', 0.2)])

p = P('Tanpura Pair', PM).ph('a', 'Tanpura', 'Strike', decay=12, damp=0.05, hard=0.8, buzz=0.75, level=0.55)
p.ph('b', 'Tanpura', 'Strike', decay=12, damp=0.06, hard=0.75, buzz=0.65, semi=7, level=0.45, pan=0.35).off('c') \
 .aenv(0.002, 10, 0.6, 10).arp('order', 1, '1/2', 0.95, hold=True)
pmMacros(p).macro(5, 'Jawari').mod('m5', 'at4', -0.3).chorus('chorus', 0.1, 0.05, 0.3).tags('tanpura', 'indian', 'roach', 'arp', 'hold-a-chord')
pmX(p)

p = P('Bowed Tanpura', PM).ph('a', 'Tanpura', 'Bow', decay=12, damp=0.05, hard=0.5, buzz=0.85, level=0.6).off('b', 'c')
pmMacros(p.aenv(0.8, 3, 1, 6)).chorus('chorus', 0.1, 0.05, 0.3).tags('tanpura', 'bowed', 'jawari', 'roach')
pmX(p, vib=0.01)

p = P('Sympathetic Sitar', PM).ph('a', 'Sympathetic', 'Strike', decay=6, damp=0.12, hard=0.8, buzz=0.9, level=0.6).off('b', 'c')
pmMacros(p.aenv(0.002, 5, 0.4, 5).voice('legato', 0.08)).chorus('chorus', 0.1, 0.05, 0.3).tags('sympathetic', 'sitar', 'indian', 'expressive')
pmX(p, vib=0.015)

p = P('Sympathetic Viola', PM).ph('a', 'Sympathetic', 'Bow', decay=8, damp=0.2, hard=0.5, buzz=0.85, level=0.6).off('b', 'c')
pmMacros(p.aenv(0.3, 2, 0.95, 2.5).voice('legato', 0.05)).chorus('chorus', 0.1, 0.05, 0.3).tags('sympathetic', 'viola', 'bowed', 'baroque')
pmX(p, vib=0.022)

p = P('Aeolian Strings', PM).ph('a', 'Sympathetic', 'Breath', decay=12, damp=0.15, hard=0.3, buzz=0.95, level=0.55)
p.ph('b', 'String', 'Breath', decay=10, damp=0.2, hard=0.3, oct=1, level=0.25).off('c').aenv(3, 3, 1, 8) \
 .lfo(1, 0.05, 'drift').mod('lfo1', 'at2', 0.4).mod('lfo1', 'bt2', 0.4)
pmMacros(p).chorus('chorus', 0.12, 0.04, 0.4).tags('aeolian', 'wind', 'harp', 'corbacho')
pmX(p)

# ---------------------------------------------------------------- bars, bells, plates
p = P('Marimba Rosewood', PM).ph('a', 'Bar', 'Strike', decay=1.6, damp=0.35, hard=0.5, pos=0.2, level=0.65).off('b', 'c')
p.ph('b', 'Tube', 'Osc Input', decay=1.2, damp=0.4, hard=0.3, level=0.3).aenv(0.002, 1.6, 0.0, 1.6).curve(a=1)
pmMacros(p).chorus('chorus', 0.08, 0.05, 0.3).tags('marimba', 'mallet', 'wood', 'resonator-tube')
pmX(p)

p = P('Vibraphone Motor', PM).ph('a', 'Bar', 'Strike', decay=6, damp=0.12, hard=0.55, stretch=0.2, level=0.6).off('b', 'c')
p.aenv(0.002, 6, 0.0, 6).curve(a=1).lfo(1, 5.5, 'sine').mod('lfo1', 'alevel', 0.25, via='mw')
pmMacros(p).macro(5, 'Motor Speed').mod('m5', 'lfo1rate', 0.3).chorus('chorus', 0.1, 0.05, 0.3).tags('vibraphone', 'mallet', 'jazz', 'metal')
pmX(p)

p = P('Glockenspiel Steel', PM).ph('a', 'Bar', 'Strike', decay=4, damp=0.08, hard=0.9, stretch=0.4, level=0.55).off('b', 'c')
pmMacros(p.f1('SVF HP12', 300).aenv(0.002, 4, 0.0, 4).curve(a=1)).chorus('chorus', 0.08, 0.05, 0.3).tags('glockenspiel', 'bright', 'metal', 'mallet')
pmX(p)

p = P('Church Bell Tower', PM).ph('a', 'Bell', 'Strike', decay=18, damp=0.06, hard=0.8, level=0.6, oct=-1).off('b', 'c')
pmMacros(p.aenv(0.002, 16, 0.0, 16).curve(a=1)).chorus('chorus', 0.1, 0.04, 0.4).tags('bell', 'church', 'tower', 'big')
pmX(p)

p = P('Handbell Choir', PM).ph('a', 'Bell', 'Strike', decay=6, damp=0.12, hard=0.6, level=0.55)
p.ph('b', 'Bell', 'Strike', decay=6, damp=0.12, hard=0.6, fine=5, oct=1, level=0.2).off('c').aenv(0.002, 6, 0.0, 6).curve(a=1)
pmMacros(p).chorus('ensemble', 0.15, 0.05, 0.4).tags('handbell', 'choir', 'christmas', 'bright')
pmX(p)

p = P('Thunder Sheet', PM).ph('a', 'Plate', 'Strike', decay=8, damp=0.1, hard=0.95, couple=0.6, spread=0.9, oct=-2, level=0.6).off('b', 'c')
pmMacros(p.aenv(0.002, 8, 0.0, 8).curve(a=1)).nochorus().tags('plate', 'thunder', 'cinematic', 'impact')
pmX(p)

p = P('Bowed Plate Drone', PM).ph('a', 'Plate', 'Bow', decay=20, damp=0.08, hard=0.6, couple=0.5, oct=-1, level=0.6).off('b', 'c')
pmMacros(p.aenv(2, 3, 1, 10)).lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.3).chorus('chorus', 0.1, 0.04, 0.4).tags('plate', 'bowed', 'drone', 'roach')
pmX(p)

p = P('Scraped Gong Ritual', PM).ph('a', 'Plate', 'Scrape', decay=14, damp=0.1, hard=0.5, couple=0.7, oct=-1, level=0.6).off('b', 'c')
pmMacros(p.aenv(1, 3, 1, 8)).lfo(1, 0.07, 'drift').mod('lfo1', 'at1', 0.4).chorus('chorus', 0.1, 0.04, 0.4).tags('gong', 'scraped', 'ritual', 'roach')
pmX(p)

p = P('Glass Bowl Pair Bowed', PM).ph('a', 'Glass Bowl', 'Bow', decay=18, damp=0.1, hard=0.3, couple=0.6, level=0.55)
p.ph('b', 'Glass Bowl', 'Bow', decay=18, damp=0.1, hard=0.3, couple=0.6, fine=9, level=0.45, pan=0.4).off('c').aenv(2, 3, 1, 10)
pmMacros(p).chorus('chorus', 0.1, 0.04, 0.4).tags('glass', 'bowl', 'bowed', 'beating', 'corbacho')
pmX(p)

p = P('Wine Glass Rubbed', PM).ph('a', 'Glass Bowl', 'Scrape', decay=10, damp=0.12, hard=0.25, level=0.55).off('b', 'c')
pmMacros(p.aenv(1.2, 2, 1, 4)).chorus('chorus', 0.1, 0.05, 0.3).tags('glass', 'rubbed', 'singing', 'eerie')
pmX(p, vib=0.006)

p = P('Rain on Bells', PM).ph('a', 'Bell', 'Rain', decay=5, damp=0.15, hard=0.6, spread=1.0, level=0.55)
p.ph('b', 'Bar', 'Rain', decay=2, damp=0.3, hard=0.7, spread=1.0, oct=1, level=0.3).off('c').aenv(1, 2, 1, 5)
pmMacros(p).macro(5, 'Rain Density').mod('m5', 'at3', 0.3).chorus('chorus', 0.1, 0.05, 0.4).width(1.5).tags('rain', 'bells', 'sparkle', 'texture')
pmX(p)

p = P('Rain on the Membrane', PM).ph('a', 'Membrane', 'Rain', decay=1.5, damp=0.4, hard=0.4, spread=1.0, level=0.6).off('b', 'c')
pmMacros(p.aenv(1, 2, 1, 4)).chorus('chorus', 0.08, 0.05, 0.3).tags('rain', 'drum', 'membrane', 'texture')
pmX(p)

# ---------------------------------------------------------------- membranes & tubes
p = P('Frame Drum', PM).ph('a', 'Membrane', 'Strike', decay=1.2, damp=0.45, hard=0.4, pos=0.3, oct=-1, level=0.7).off('b', 'c')
pmMacros(p.aenv(0.001, 1.2, 0.0, 1.0).curve(a=1)).nochorus().tags('drum', 'frame-drum', 'ritual', 'roach')
pmX(p)

p = P('Talking Drum', PM).ph('a', 'Membrane', 'Strike', decay=0.9, damp=0.4, hard=0.5, pos=0.25, level=0.7).off('b', 'c')
p.aenv(0.001, 0.9, 0.0, 0.8).curve(a=1).mpe(bend=12)
pmMacros(p).nochorus().tags('drum', 'talking-drum', 'pitch-bend', 'african')
X(p, vib=0, pa=0.3, st=0.0, press=[('pitch', 0.12), ('at2', 0.3)], slide=[('at1', 0.6)])

p = P('Udu Clay Drum', PM).ph('a', 'Tube', 'Strike', decay=0.8, damp=0.5, hard=0.3, oct=-1, level=0.7)
p.ph('b', 'Membrane', 'Strike', decay=0.5, damp=0.5, hard=0.5, level=0.3).off('c').aenv(0.001, 0.8, 0.0, 0.7).curve(a=1) \
 .mseg(1, [(0, 1.0, 0), (0.25, 0.0, -0.4)], loop=(1, 1), mode='env').mod('mseg1', 'apitch', 0.05)
pmMacros(p).nochorus().tags('udu', 'clay', 'drum', 'world')
pmX(p)

p = P('Bamboo Flute Tube', PM).ph('a', 'Tube', 'Breath', decay=3, damp=0.3, hard=0.4, level=0.6).noise(0.05, 0.5).off('b', 'c')
pmMacros(p.f1('SVF LP12', 6000, key=0.3).aenv(0.08, 1, 0.9, 0.4).voice('legato', 0.04)).chorus('chorus', 0.1, 0.05, 0.3).tags('tube', 'flute', 'bamboo', 'expressive')
pmX(p, vib=0.02, extra_press=[('noise', 0.2)])

p = P('Pan Pipes Breath', PM).ph('a', 'Tube', 'Breath', decay=2, damp=0.35, hard=0.35, level=0.55)
p.ph('b', 'Tube', 'Breath', decay=2, damp=0.35, hard=0.35, oct=1, level=0.2).noise(0.08, 0.6).off('c').aenv(0.05, 1, 0.85, 0.5)
pmMacros(p).chorus('chorus', 0.12, 0.05, 0.3).tags('pan-pipes', 'breath', 'andes', 'blown')
pmX(p, vib=0.015, extra_press=[('noise', 0.25)])

p = P('Didgeridoo Tube', PM).ph('a', 'Tube', 'Breath', decay=4, damp=0.25, hard=0.6, oct=-2, level=0.65).off('b', 'c')
p.f1('Vowel', 600, res=0.45, key=0.0, shape=0.4).aenv(0.2, 2, 1, 1.5).lfo(1, 0.4, 'drift').mod('lfo1', 'f1shape', 0.4)
pmMacros(p).macro(5, 'Mouth').mod('m5', 'f1shape', 0.6).nochorus().tags('didgeridoo', 'tube', 'roach', 'drone')
pmX(p, extra_press=[('f1shape', 0.4)])

p = P('Steam Pipe Organ', PM).ph('a', 'Tube', 'Breath', decay=5, damp=0.2, hard=0.5, level=0.5)
p.ph('b', 'Tube', 'Breath', decay=5, damp=0.2, hard=0.5, semi=7, level=0.3).ph('c', 'Tube', 'Breath', decay=5, damp=0.2, hard=0.5, oct=1, level=0.25) \
 .aenv(0.15, 1, 1, 1)
pmMacros(p).chorus('ensemble', 0.15, 0.05, 0.4).tags('organ', 'pipe', 'steam', 'blown')
pmX(p)

# ---------------------------------------------------------------- oscillators exciting resonators (Osc Input)
p = P('Saw Driven Strings', PM).an('b', 'saw', voices=3, detune=10, level=0.0).off('c')
p.ph('a', 'String', 'Osc Input', decay=6, damp=0.25, hard=0.5, pos=0.15, level=0.6).f1('SVF LP12', 5000, key=0.2).aenv(0.6, 2, 1, 3) \
 .macro(5, 'Drive Osc').mod('m5', 'blevel', 0.4)
pmMacros(p).chorus('ensemble', 0.2, 0.05, 0.4).tags('osc-input', 'hybrid', 'strings', 'resonator')
pmX(p)

p = P('Noise Fed Bowl', PM).noise(0.25, 0.4).ph('a', 'Glass Bowl', 'Breath', decay=12, damp=0.1, hard=0.4, couple=0.4, level=0.6).off('b', 'c')
pmMacros(p.aenv(1.5, 3, 1, 8)).lfo(1, 0.05, 'drift').mod('lfo1', 'noise', 0.2).chorus('chorus', 0.1, 0.04, 0.4).tags('breath', 'bowl', 'noise', 'resonator')
pmX(p, extra_press=[('noise', 0.3)])

p = P('Granular Excited Plate', PM).gr('b', 'fixed', sample='Nature/Rain Light', pos=0.3, size=0.3, density=20, level=0.0)
p.ph('a', 'Plate', 'Osc Input', decay=8, damp=0.15, hard=0.5, couple=0.4, level=0.6).off('c').aenv(1, 2, 1, 6)
pmMacros(p).chorus('chorus', 0.1, 0.04, 0.4).tags('osc-input', 'granular', 'plate', 'rain', 'hybrid')
pmX(p)

p = P('FM Struck Bar', PM).an('b', 'sine', semi=7, level=0.0).fm('b', 'c', 0.5, 'pm').an('c', 'sine', oct=1, level=0.0)
p.ph('a', 'Bar', 'Osc Input', decay=3, damp=0.25, hard=0.6, level=0.6).aenv(0.002, 3, 0.0, 3).curve(a=1) \
 .menv(0.001, 0.3, 0.0, 0.2).mod('menv', 'bfm', 0.5)
pmMacros(p).chorus('chorus', 0.1, 0.05, 0.3).tags('osc-input', 'fm', 'bar', 'metallic')
pmX(p)

p = P('Voice Into Strings', PM).gr('b', 'keyed', sample='Voices/Choir Ah', pos=0.3, size=0.6, density=14, level=0.15)
p.ph('a', 'Sympathetic', 'Osc Input', decay=10, damp=0.15, hard=0.4, buzz=0.9, level=0.55).off('c').aenv(1, 3, 1, 6)
pmMacros(p).macro(5, 'Voice').mod('m5', 'blevel', 0.4).chorus('ensemble', 0.15, 0.05, 0.4).tags('osc-input', 'choir', 'sympathetic', 'hybrid', 'corbacho')
pmX(p)

# ---------------------------------------------------------------- coupled systems
p = P('Coupled Bar Chain', PM).ph('a', 'Bar', 'Strike', decay=5, damp=0.15, hard=0.7, couple=0.8, level=0.5)
p.ph('b', 'Bar', 'Osc Input', decay=5, damp=0.15, hard=0.5, couple=0.8, semi=5, level=0.35) \
 .ph('c', 'Bar', 'Osc Input', decay=5, damp=0.15, hard=0.5, couple=0.8, semi=12, level=0.25).aenv(0.002, 5, 0.0, 5).curve(a=1)
pmMacros(p).chorus('chorus', 0.1, 0.05, 0.3).tags('coupled', 'bar', 'chain', 'metallic')
pmX(p)

p = P('Bell Into Plate', PM).ph('a', 'Bell', 'Strike', decay=8, damp=0.12, hard=0.75, couple=0.7, level=0.5)
p.ph('b', 'Plate', 'Osc Input', decay=10, damp=0.1, hard=0.5, couple=0.7, oct=-1, level=0.4).off('c').aenv(0.002, 9, 0.0, 9).curve(a=1)
pmMacros(p).chorus('chorus', 0.1, 0.04, 0.4).tags('coupled', 'bell', 'plate', 'cinematic')
pmX(p)

p = P('String Into Membrane', PM).ph('a', 'String', 'Strike', decay=4, damp=0.25, hard=0.6, couple=0.7, level=0.5)
p.ph('b', 'Membrane', 'Osc Input', decay=2, damp=0.3, hard=0.4, couple=0.7, level=0.4).off('c').aenv(0.002, 4, 0.0, 4).curve(a=1)
pmMacros(p).chorus('chorus', 0.1, 0.05, 0.3).tags('coupled', 'banjo', 'membrane', 'twang')
pmX(p)

p = P('Triple Glass Resonance', PM).ph('a', 'Glass Bowl', 'Bow', decay=14, damp=0.1, hard=0.3, couple=0.75, level=0.45)
p.ph('b', 'Glass Bowl', 'Osc Input', decay=14, damp=0.1, hard=0.3, couple=0.75, semi=7, level=0.35) \
 .ph('c', 'Bell', 'Osc Input', decay=12, damp=0.1, hard=0.4, couple=0.75, oct=1, level=0.2).aenv(2, 3, 1, 9)
pmMacros(p).chorus('chorus', 0.1, 0.04, 0.4).tags('coupled', 'glass', 'corbacho', 'resonant')
pmX(p)

p = P('Kalimba Box', PM).ph('a', 'Bar', 'Strike', decay=2, damp=0.3, hard=0.6, stretch=0.3, couple=0.5, level=0.55)
p.ph('b', 'Tube', 'Osc Input', decay=1.5, damp=0.35, hard=0.3, couple=0.5, oct=-1, level=0.3).off('c').aenv(0.002, 2, 0.0, 2).curve(a=1)
pmMacros(p).chorus('chorus', 0.1, 0.05, 0.3).tags('kalimba', 'box', 'coupled', 'africa')
pmX(p)

# ---------------------------------------------------------------- long-form physical textures
p = P('Bowed Bar Cathedral', PM).ph('a', 'Bar', 'Bow', decay=14, damp=0.12, hard=0.45, couple=0.5, level=0.5)
p.ph('b', 'Bell', 'Bow', decay=14, damp=0.12, hard=0.4, couple=0.5, oct=-1, level=0.35).off('c').aenv(3, 3, 1, 12) \
 .lfo(1, 0.03, 'drift', 'voice').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', 0.3)
pmMacros(p).chorus('ensemble', 0.15, 0.04, 0.4).tags('bowed', 'bar', 'bell', 'cathedral', 'roach')
pmX(p)

p = P('Scraped Wire Fence', PM).ph('a', 'Waveguide', 'Scrape', decay=6, damp=0.1, hard=0.5, level=0.55)
p.ph('b', 'Sympathetic', 'Osc Input', decay=8, damp=0.15, buzz=0.8, level=0.3).off('c').aenv(0.5, 2, 1, 4) \
 .lfo(1, 0.2, 'drift').mod('lfo1', 'at1', 0.4)
pmMacros(p).nochorus().tags('scrape', 'wire', 'industrial', 'eerie')
pmX(p)

p = P('Breathing Tubes Choir', PM).ph('a', 'Tube', 'Breath', decay=6, damp=0.2, hard=0.4, level=0.45)
p.ph('b', 'Tube', 'Breath', decay=6, damp=0.2, hard=0.4, semi=7, level=0.35).ph('c', 'Tube', 'Breath', decay=6, damp=0.2, hard=0.4, oct=-1, level=0.3) \
 .f1('Vowel', 900, res=0.35, key=0.0, shape=0.6).aenv(2, 3, 1, 6).lfo(1, 0.04, shape=BREATH).mod('lfo1', 'at2', 0.3).mod('lfo1', 'f1shape', 0.3)
pmMacros(p).chorus('ensemble', 0.2, 0.05, 0.5).tags('tubes', 'choir', 'breath', 'corbacho')
pmX(p)

p = P('Membrane Rain Storm', PM).ph('a', 'Membrane', 'Rain', decay=2, damp=0.35, hard=0.45, spread=1.0, oct=-1, level=0.55)
p.ph('b', 'Plate', 'Rain', decay=4, damp=0.2, hard=0.6, spread=1.0, level=0.3).off('c').aenv(2, 2, 1, 6) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at3', 0.3)
pmMacros(p).chorus('chorus', 0.1, 0.04, 0.4).width(1.5).tags('rain', 'storm', 'drums', 'texture', 'roach')
pmX(p)

p = P('Tanpura Sympathy Field', PM).ph('a', 'Tanpura', 'Bow', decay=14, damp=0.05, hard=0.45, buzz=0.8, level=0.5)
p.ph('b', 'Sympathetic', 'Osc Input', decay=12, damp=0.12, buzz=0.95, level=0.35, pan=0.3).off('c').aenv(2, 3, 1, 10) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at2', 0.3)
pmMacros(p).chorus('chorus', 0.1, 0.04, 0.4).tags('tanpura', 'sympathetic', 'drone', 'roach')
pmX(p)

p = P('Physical Choir of Bowls', PM).ph('a', 'Glass Bowl', 'Breath', decay=16, damp=0.08, hard=0.3, couple=0.5, level=0.5)
p.ph('b', 'Bell', 'Breath', decay=12, damp=0.1, hard=0.3, couple=0.5, semi=7, level=0.3) \
 .gr('c', 'keyed', sample='Voices/Choir Oo', pos=0.3, size=0.8, density=10, level=0.2).aenv(3, 3, 1, 12)
pmMacros(p).chorus('ensemble', 0.2, 0.04, 0.5).tags('bowls', 'choir', 'breath', 'corbacho')
pmX(p)

p = P('Struck String Flanged', PM).ph('a', 'String', 'Strike', decay=5, damp=0.2, hard=0.7, pos=0.15, level=0.6).off('b', 'c')
p.f1('Flg HL6+', 500, res=0.5, key=0.5, shape=0.5).aenv(0.002, 5, 0.3, 5).curve(a=1).lfo(1, 0.1, 'tri').mod('lfo1', 'f1cut', 0.2)
pmMacros(p).macro(5, 'Flange').mod('m5', 'f1res', 0.4).chorus('chorus', 0.1, 0.05, 0.3).tags('string', 'flanger-filter', 'psychedelic')
pmX(p)

p = P('Bar Through Comb', PM).ph('a', 'Bar', 'Strike', decay=3, damp=0.25, hard=0.7, level=0.6).off('b', 'c')
p.f1('Cmb L6+', 300, res=0.7, key=1.0, shape=0.5).aenv(0.002, 3, 0.0, 3).curve(a=1)
pmMacros(p).macro(5, 'Comb').mod('m5', 'f1res', 0.2).chorus('chorus', 0.1, 0.05, 0.3).tags('bar', 'comb', 'metallic', 'flanger-filter')
pmX(p)

p = P('Glass Rain Phaser', PM).ph('a', 'Glass Bowl', 'Rain', decay=4, damp=0.2, hard=0.6, spread=1.0, level=0.55).off('b', 'c')
p.f1('Phs 48H6+', 1500, res=0.55, key=0.0, shape=0.5).aenv(1, 2, 1, 5).lfo(1, 0.04, 'sine').mod('lfo1', 'f1cut', 0.6)
pmMacros(p).macro(5, 'Phase').mod('m5', 'f1res', 0.3).chorus('chorus', 0.1, 0.04, 0.4).tags('glass', 'rain', 'phaser-filter', 'texture')
pmX(p)

p = P('Waveguide Bass Pluck', PM).ph('a', 'Waveguide', 'Strike', decay=3, damp=0.35, hard=0.5, oct=-1, level=0.7).off('b', 'c')
p.sub(0.2).f1('Ladder LP24', 1500, res=0.1, key=0.4).aenv(0.002, 2, 0.4, 0.6).curve(a=1).voice('mono', 0.03)
pmMacros(p).nochorus().tags('bass', 'waveguide', 'plucked', 'upright')
pmX(p)

p = P('Membrane Bass Drum', PM).ph('a', 'Membrane', 'Strike', decay=1.5, damp=0.3, hard=0.35, oct=-2, level=0.75).off('b', 'c')
p.aenv(0.001, 1.5, 0.0, 1.2).curve(a=1).mseg(1, [(0, 1.0, 0), (0.08, 0.0, -0.6)], loop=(1, 1), mode='env').mod('mseg1', 'apitch', 0.1)
pmMacros(p).drive(0.1).nochorus().tags('drum', 'kick', 'membrane', 'tribal')
pmX(p)

p = P('Expressive Bowed Glass Lead', PM).ph('a', 'Glass Bowl', 'Bow', decay=10, damp=0.12, hard=0.35, couple=0.3, level=0.6).off('b', 'c')
p.aenv(0.25, 2, 0.95, 1.5).voice('legato', 0.08)
pmMacros(p).chorus('chorus', 0.1, 0.05, 0.3).tags('lead', 'glass', 'bowed', 'expressive', 'mpe-showcase')
X(p, vib=0.02, pa=0.6, pc=0.1, st=0.0, press=[('at2', 0.6), ('at4', -0.3)], slide=[('at1', 0.7)])
