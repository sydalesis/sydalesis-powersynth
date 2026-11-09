# Driftfield 3.1 bank, part B: TEXTURES (evolving, granular, spectral, cross-modulated)
T = 'Textures'

p = P('Granular Glacier', T).gr('a', 'wt', lib='Ambient/Ice Cathedral', pos=0.2, scan=0.04, size=0.5, density=28, spray=0.35, prand=7, shape=0.4, spread=0.9, level=0.7)
p.gr('b', 'keyed', sample='Textures/Glass Harmonics', pos=0.4, scan=-0.03, size=0.7, density=14, spray=0.5, prand=12, level=0.4, oct=1) \
 .off('c').f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .mseg(1, [(0, 0.0, 0), (16, 1.0, 0.2), (16, 0.0, -0.2)], loop=(0, 2), mode='free').mod('mseg1', 'at1', 0.6).mod('mseg1', 'bt2', -0.3) \
 .lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'at4', 0.3) \
 .macro(1, 'Scan').mod('m1', 'at1', 0.5).macro(2, 'Grain Size').mod('m2', 'at2', 0.5).macro(3, 'Density').mod('m3', 'at3', 0.5).macro(4, 'Sparkle').mod('m4', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('granular', 'glacial', 'corbacho', 'evolving')
X(p, press=[('at3', 0.4), ('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Spectral Erosion', T).wt('a', 'Drift Spectra/Drift Ash', pos=0.0, blend='spectral', voices=4, detune=10, level=0.6)
p.wt('b', 'Spectral/Spectral Smear', pos=0.3, blend='spectral', oct=-1, level=0.35, warp='asym', warpamt=0.3) \
 .off('c').f1('Comb -', 400, res=0.4, key=0.5).f2('SVF LP12', 6000).aenv(*LONG) \
 .mseg(1, [(0, 0.0, 0), (24, 1.0, 0.0), (24, 0.0, 0.0)], loop=(0, 2), mode='free').mod('mseg1', 'at1', 1.0).mod('mseg1', 'f1cut', 0.3) \
 .lfo(1, 0.07, 'drift', 'voice').mod('lfo1', 'bt2', 0.4) \
 .macro(1, 'Erode').mod('m1', 'at1', 0.5).macro(2, 'Comb').mod('m2', 'f1res', 0.5).macro(3, 'Warp').mod('m3', 'bt2', 0.6) \
 .chorus('chorus', 0.18, 0.05, 0.4).tags('spectral', 'evolving', 'dark')
X(p, press=[('f1res', 0.3)], slide=[('bt2', 0.5)])

p = P('Choir Particle Storm', T).gr('a', 'keyed', sample='Voices/Choir Ah', pos=0.3, scan=0.02, size=0.08, density=90, spray=0.6, prand=24, shape=0.6, spread=1.0, level=0.6)
p.gr('b', 'keyed', sample='Voices/Choir Oo', pos=0.5, scan=-0.02, size=0.25, density=30, spray=0.4, prand=5, level=0.4, oct=-1) \
 .off('c').f1('SVF LP12', 6500, res=0.05, key=0.1).aenv(3, 3, 1, 12) \
 .lfo(1, 0.11, 'drift').mod('lfo1', 'at4', 0.5).mod('lfo1', 'at2', 0.3) \
 .macro(1, 'Storm').mod('m1', 'at3', 0.5).macro(2, 'Scatter').mod('m2', 'at4', 0.5).macro(3, 'Body').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.3, 0.07, 0.5).tags('granular', 'choir', 'particles', 'corbacho')
X(p, press=[('at3', 0.5)], slide=[('at2', 0.5)])

p = P('Cross Mod Lattice', T).wt('a', 'Metallic/Bronze Bars', pos=0.3, blend='spectral', level=0.55)
p.an('b', 'sine', semi=7, level=0.0).add('c', 'Primes', 'Bell', morph=0.4, partials=64, shim=0.3, level=0.35) \
 .fm('a', 'b', 0.25, 'pm').fm('c', 'a', 0.15, 'fm').f1('SVF BP12', 1500, res=0.25, key=0.3).f2('SVF LP12', 7000).route('parallel', 0.5).aenv(*LONG) \
 .lfo(1, 0.05, 'sine').lfo(2, 0.13, 'drift', 'voice').mod('lfo1', 'afm', 0.3).mod('lfo2', 'cfm', 0.3).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Lattice').mod('m1', 'afm', 0.5).macro(2, 'Prime FM').mod('m2', 'cfm', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('fm', 'metallic', 'complex', 'cross-mod')
X(p, press=[('afm', 0.4)], slide=[('cfm', 0.5)])

p = P('Frozen Breath Loop', T).gr('a', 'keyed', sample='Voices/Breath Cycle', pos=0.3, scan=0.0, size=0.4, density=22, spray=0.2, level=0.6)
p.add('b', 'Choir Oo', 'Hollow', morph=0.3, partials=48, blur=4, freeze=False, shim=0.2, level=0.4) \
 .off('c').f1('Vowel', 700, res=0.45, key=0.0, shape=0.4).aenv(*LONG) \
 .lfo(1, 0.03, shape=BREATH).mod('lfo1', 'f1shape', 0.45).mod('lfo1', 'at1', 0.2) \
 .macro(1, 'Vowel').mod('m1', 'f1shape', 0.6).macro(2, 'Blur').mod('m2', 'bt1', 0.5).macro(3, 'Breath').mod('m3', 'alevel', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('breath', 'vocal', 'frozen', 'evolving')
X(p, press=[('f1shape', 0.4)], slide=[('at1', 0.4)])

p = P('Dust Devil Steps', T).gr('a', 'fixed', sample='Textures/Granular Dust', pos=0.3, scan=0.05, size=0.12, density=60, spray=0.8, prand=0, level=0.45)
p.wt('b', 'Stepped/Wave Steps Dense', pos=0.3, blend='stepped', voices=3, detune=9, level=0.4) \
 .off('c').f1('SVF BP12', 1200, res=0.3, key=0.2).f2('SVF LP12', 6000).aenv(*LONG) \
 .lfo(1, 0.3, 'sh').lfo(2, 0.05, 'drift').mod('lfo1', 'bt1', 0.3).mod('lfo2', 'f1cut', 0.5) \
 .macro(1, 'Dust').mod('m1', 'alevel', 0.5).macro(2, 'Steps').mod('m2', 'bt1', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.2, 0.08, 0.4).tags('granular', 'desert', 'stepped', 'roach')
X(p, press=[('f1res', 0.3)], slide=[('bt1', 0.5)])

p = P('Resonant Ghost Strings', T).wt('a', 'Ambient/Ghost Strings', pos=0.3, blend='spectral', voices=5, detune=14, level=0.6)
p.ph('b', 'String', 'Osc Input', decay=8, damp=0.2, hard=0.4, level=0.3).off('c') \
 .f1('Formant', 900, res=0.3, key=0.0).f2('SVF LP12', 5000).route('parallel', 0.5).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Ghost').mod('m1', 'at1', 0.5).macro(2, 'Resonator').mod('m2', 'blevel', 0.5).macro(3, 'Formant').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('strings', 'haunted', 'resonant')
X(p, vib=0.015, press=[('blevel', 0.35)], slide=[('at1', 0.5)])

p = P('Spectral Rain Glass', T)
p.add('a', 'Glass', 'Primes', morph=0.5, partials=96, stretch=0.15, shim=0.6, srate=0.4, pdet=8, level=0.55) \
 .gr('b', 'fixed', sample='Nature/Rain Light', pos=0.3, size=0.3, density=20, spray=0.6, level=0.3) \
 .off('c').f1('SVF LP12', 9000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Morph').mod('m1', 'at1', 0.5).macro(2, 'Shimmer').mod('m2', 'at2', 0.4).macro(3, 'Rain').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('glass', 'rain', 'shimmer', 'corbacho')
X(p, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Metal Drift Sculpture', T).gr('a', 'keyed', sample='Metal/Bowed Cymbal', pos=0.3, scan=0.03, size=0.6, density=18, spray=0.4, prand=6, level=0.55)
p.ph('b', 'Plate', 'Scrape', decay=10, damp=0.2, hard=0.45, couple=0.5, oct=-1, level=0.35) \
 .off('c').f1('SVF HP12', 200).f2('SVF LP12', 8000).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').lfo(2, 0.07, 'drift', 'voice').mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.4) \
 .macro(1, 'Scan').mod('m1', 'at1', 0.5).macro(2, 'Plate').mod('m2', 'blevel', 0.5).macro(3, 'Scrape').mod('m3', 'bt2', 0.5) \
 .chorus('flanger', 0.12, 0.03, 0.4, 0.2).tags('metallic', 'sculpture', 'evolving')
X(p, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Overtone Weaver', T).add('a', 'Throat', 'Fifths', morph=0.3, partials=80, peak=8, peakamt=0.8, width=0.05, shim=0.15, level=0.6)
p.add('b', 'Throat', 'Octaves', morph=0.5, partials=64, peak=13, peakamt=0.7, width=0.05, semi=7, level=0.35, pan=0.3) \
 .off('c').f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .mseg(1, [(0, 0.1, 0), (4, 0.4, 0), (4, 0.25, 0), (4, 0.6, 0), (4, 0.35, 0), (4, 0.8, 0), (4, 0.1, 0)], loop=(0, 6), mode='free').mod('mseg1', 'at2', 0.7) \
 .mseg(2, [(0, 0.6, 0), (6, 0.2, 0), (6, 0.7, 0), (6, 0.3, 0), (6, 0.6, 0)], loop=(0, 4), mode='free').mod('mseg2', 'bt2', 0.6) \
 .macro(1, 'Weave A').mod('m1', 'at2', 0.5).macro(2, 'Weave B').mod('m2', 'bt2', 0.5).macro(3, 'Fifth').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('overtone', 'throat', 'roach', 'hypnotic', 'mseg')
X(p, vib=0, press=[('at2', 0.4)], slide=[('bt2', 0.5)])

p = P('Wavefold Nebula', T).wt('a', 'Digital/Fold Sine Deep', pos=0.2, blend='spectral', voices=4, detune=12, level=0.5)
p.wt('b', 'Ambient/Celestial Fold', pos=0.3, blend='spectral', oct=-1, level=0.35) \
 .off('c').f1('OB-Xa LP24', 2500, res=0.15, key=0.2).aenv(*LONG) \
 .lfo(1, 0.017, 'sine').lfo(2, 0.043, 'tri').mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Fold').mod('m1', 'at1', 0.5).macro(2, 'Nebula').mod('m2', 'bt1', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('wavefold', 'space', 'evolving')
X(p, press=[('at1', 0.35)], slide=[('bt1', 0.5)])

p = P('Bowed Glass Machine', T).ph('a', 'Glass Bowl', 'Bow', decay=14, damp=0.12, hard=0.35, couple=0.7, level=0.55)
p.ph('b', 'Bar', 'Bow', decay=10, damp=0.2, hard=0.4, couple=0.7, semi=7, level=0.35) \
 .ph('c', 'Bell', 'Osc Input', decay=8, damp=0.2, hard=0.5, oct=1, level=0.2) \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').lfo(2, 0.08, 'sine', 'voice').mod('lfo1', 'at2', 0.4).mod('lfo2', 'bt1', 0.3) \
 .macro(1, 'Bow Pressure').mod('m1', 'at2', 0.5).mod('m1', 'bt2', 0.5).macro(2, 'Coupling').mod('m2', 'clevel', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('physical', 'glass', 'coupled', 'corbacho')
X(p, vib=0, press=[('at2', 0.4), ('bt2', 0.3)], slide=[('at1', 0.5)])

p = P('Radio Ghost Archive', T).gr('a', 'fixed', sample='Drones/Shortwave Radio', pos=0.3, scan=0.04, size=0.25, density=24, spray=0.5, level=0.45)
p.wt('b', 'Textures/Shortwave', pos=0.3, blend='spectral', voices=3, detune=8, level=0.4) \
 .off('c').f1('SVF BP12', 1400, res=0.3, key=0.1).f2('SVF LP12', 5000).aenv(*LONG).shifter(-0.3, 0.3, 0.3, 0.8) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'f1cut', 0.5).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Tune').mod('m1', 'f1cut', 0.5).macro(2, 'Ghosts').mod('m2', 'bt1', 0.5).macro(3, 'Shift').mod('m3', 'fxshiftmix', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('radio', 'ghost', 'lo-fi', 'shifter')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

p = P('Vowel Kaleidoscope', T).wt('a', 'Vowel Journeys/Tenor Full Circle', pos=0.0, blend='spectral', voices=4, detune=10, level=0.55)
p.wt('b', 'Vowel Journeys/Soprano Rise', pos=0.5, blend='spectral', oct=1, level=0.25) \
 .off('c').f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.033, 'tri').lfo(2, 0.021, 'sine').mod('lfo1', 'at1', 0.8).mod('lfo2', 'bt1', 0.8) \
 .macro(1, 'Vowel A').mod('m1', 'at1', 0.5).macro(2, 'Vowel B').mod('m2', 'bt1', 0.5).macro(3, 'Soprano').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('vocal', 'vowel', 'evolving', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.6)])

p = P('Rusted Comb Field', T).an('a', 'saw', voices=3, detune=10, slop=0.5, level=0.5)
p.noise(0.25, 0.4).gr('b', 'fixed', sample='Metal/Scraped Plate', pos=0.3, size=0.4, density=14, level=0.3).off('c') \
 .f1('Comb +', 220, res=0.6, key=0.8).f2('SVF LP12', 5000).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'noise', 0.15) \
 .macro(1, 'Comb').mod('m1', 'f1res', 0.4).macro(2, 'Rust').mod('m2', 'blevel', 0.5).macro(3, 'Noise').mod('m3', 'noise', 0.4) \
 .chorus('flanger', 0.15, 0.03, 0.4, 0.3).tags('comb', 'industrial', 'resonant')
X(p, press=[('f1res', 0.35)], slide=[('f1cut', 0.4)])

p = P('Chord Cloud Engine', T).wt('a', 'Chords/Chord Minor 9', pos=0.2, blend='spectral', voices=5, detune=14, level=0.5)
p.gr('b', 'wt', lib='Chords/Chord Major 7', pos=0.3, scan=0.03, size=0.4, density=24, spray=0.4, prand=0, level=0.35) \
 .off('c').f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'at1', 0.6).mod('lfo1', 'bt1', -0.4) \
 .macro(1, 'Brightness').mod('m1', 'at1', 0.5).macro(2, 'Cloud').mod('m2', 'blevel', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('chord', 'one-finger', 'lush', 'cloud')
X(p, press=[('at1', 0.35)], slide=[('bt1', 0.5)])

p = P('Bubble Physics', T).ph('a', 'Tube', 'Rain', decay=1.5, damp=0.4, hard=0.4, spread=0.9, level=0.55)
p.gr('b', 'fixed', sample='Nature/Bubbling Spring', pos=0.3, size=0.3, density=16, spray=0.6, level=0.35) \
 .off('c').f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(2, 3, 1, 8) \
 .lfo(1, 0.07, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Bubbles').mod('m1', 'blevel', 0.5).macro(2, 'Tube').mod('m2', 'at1', 0.5).macro(3, 'Damp').mod('m3', 'at4', 0.5) \
 .chorus('chorus', 0.12, 0.06, 0.3).tags('water', 'physical', 'playful')
X(p, vib=0, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Celestial Machinery', T).wt('a', 'Stepped/Wave Steps Glass', pos=0.2, blend='stepped', voices=3, detune=8, level=0.45)
p.add('b', 'Bell', 'Primes', morph=0.5, partials=64, stretch=0.2, shim=0.3, level=0.35) \
 .off('c').f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.5, sync=True, div='1/4', wave='sh').lfo(2, 0.03, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo2', 'bt1', 0.5) \
 .macro(1, 'Gears').mod('m1', 'at1', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.5).macro(3, 'Shine').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('mechanical', 'stepped', 'sync', 'space')
X(p, press=[('bt1', 0.35)], slide=[('at1', 0.4)])

p = P('Haunted Organ Smear', T).wt('a', 'Organs/Organ Mixture', pos=0.3, level=0.5, voices=3, detune=6)
p.add('b', 'Organ', 'Hollow', morph=0.3, partials=48, blur=6, shim=0.2, level=0.35).off('c') \
 .f1('SVF LP12', 4000, res=0.05, key=0.2).aenv(*LONG).tape(0.2, 0.45, 0.1, 0.55, 1.0) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Stops').mod('m1', 'at1', 0.5).macro(2, 'Smear').mod('m2', 'blevel', 0.5).macro(3, 'Warble').mod('m3', 'fxwow', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.5).tags('organ', 'haunted', 'tape')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Insect Swarm Synth', T).gr('a', 'fixed', sample='Nature/Cicadas', pos=0.3, size=0.15, density=70, spray=0.8, prand=0, level=0.45)
p.an('b', 'pulse', pw=0.2, voices=5, detune=30, slop=0.6, level=0.3).off('c') \
 .f1('SVF BP12', 2500, res=0.4, key=0.3).f2('SVF LP12', 7000).aenv(*LONG) \
 .lfo(1, 6.0, 'square', 'voice').lfo(2, 0.05, 'drift').mod('lfo1', 'blevel', 0.2).mod('lfo2', 'f1cut', 0.5) \
 .macro(1, 'Swarm').mod('m1', 'alevel', 0.5).macro(2, 'Buzz').mod('m2', 'blevel', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.1, 0.4).tags('insects', 'swarm', 'eerie')
X(p, vib=0, press=[('lfo1rate', 0.4)], slide=[('f1cut', 0.4)])

p = P('Monolith Pulse Field', T).wt('a', 'Drone/Monolith', pos=0.3, blend='spectral', voices=3, detune=8, level=0.55, oct=-1)
p.an('b', 'saw', oct=-2, level=0.3, slop=0.4).off('c').f1('Ladder LP24', 600, res=0.3, key=0.2).aenv(*LONG) \
 .lfo(1, 0.5, 'sine', sync=True, div='1/2').lfo(2, 0.02, 'drift').mod('lfo1', 'f1cut', 0.25).mod('lfo2', 'at1', 0.5) \
 .macro(1, 'Pulse').mod('m1', 'f1cut', 0.4).macro(2, 'Monolith').mod('m2', 'at1', 0.5).macro(3, 'Weight').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('dark', 'pulse', 'roach', 'tempo')
X(p, press=[('f1res', 0.3)], slide=[('at1', 0.5)])

p = P('Particle Accelerator', T).wt('a', 'Spectral/Partial Rise', pos=0.0, blend='spectral', voices=4, detune=12, level=0.5)
p.gr('b', 'fixed', sample='Textures/Data Stream', pos=0.3, size=0.05, density=120, spray=0.9, level=0.25).off('c') \
 .f1('SVF LP12', 6000, res=0.1, key=0.1).aenv(*LONG) \
 .mseg(1, [(0, 0.0, 0), (8, 1.0, 0.6), (0.5, 0.0, 0)], loop=(0, 2), mode='free').mod('mseg1', 'at1', 1.0).mod('mseg1', 'blevel', 0.3) \
 .macro(1, 'Speed').mod('m1', 'mseg1rate', 0.6).macro(2, 'Particles').mod('m2', 'blevel', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('sci-fi', 'riser', 'evolving', 'mseg')
X(p, press=[('mseg1rate', 0.4)], slide=[('at1', 0.4)])

p = P('Sand Glass Hourglass', T).gr('a', 'fixed', sample='Organic/Sand Flow', pos=0.3, size=0.2, density=40, spray=0.5, level=0.45)
p.add('b', 'Glass', 'Hollow', morph=0.4, partials=64, shim=0.4, srate=0.2, level=0.4).off('c') \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.025, 'drift').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Sand').mod('m1', 'alevel', 0.5).macro(2, 'Glass').mod('m2', 'bt1', 0.5).macro(3, 'Shimmer').mod('m3', 'bt2', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('glass', 'sand', 'time', 'corbacho')
X(p, press=[('bt2', 0.4)], slide=[('bt1', 0.5)])

p = P('Reverse Choir Bloom', T).gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.4, scan=-0.04, size=0.6, density=18, spray=0.3, rev=0.8, level=0.6)
p.add('b', 'Choir Ah', 'Choir Oo', morph=0.5, partials=56, shim=0.25, level=0.35).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG).curve(a=-0.7) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Reverse').mod('m1', 'at4', 0.4).macro(2, 'Bloom').mod('m2', 'blevel', 0.5).macro(3, 'Scan').mod('m3', 'at1', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('reverse', 'choir', 'granular', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Fractured Bells Texture', T).gr('a', 'keyed', sample='Metal/Bell Tree', pos=0.3, scan=0.05, size=0.1, density=50, spray=0.7, prand=12, level=0.5)
p.ph('b', 'Bell', 'Rain', decay=4, damp=0.2, hard=0.6, spread=0.9, level=0.3).off('c') \
 .f1('SVF LP12', 9000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.05, 'drift').mod('lfo1', 'at3', 0.5) \
 .macro(1, 'Fracture').mod('m1', 'at3', 0.5).macro(2, 'Rain Bells').mod('m2', 'blevel', 0.5).macro(3, 'Pitch Scatter').mod('m3', 'at4', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('bells', 'granular', 'sparkle')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Low Earth Orbit', T).wt('a', 'Drift Spectra/Drift Nimbus', pos=0.3, blend='spectral', voices=5, detune=12, level=0.5)
p.wt('b', 'Ambient/Night Sky Shimmer', pos=0.3, blend='spectral', oct=1, level=0.25).sub(0.25).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.007, 'sine').lfo(2, 0.019, 'drift').mod('lfo1', 'at1', 0.7).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'pan', 0.2) \
 .macro(1, 'Orbit').mod('m1', 'at1', 0.5).macro(2, 'Stars').mod('m2', 'blevel', 0.5).macro(3, 'Mass').mod('m3', 'sub', 0.4) \
 .chorus('ensemble', 0.25, 0.04, 0.5).width(1.5).tags('space', 'vast', 'corbacho', 'slow')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Wooden Resonance Box', T).ph('a', 'Bar', 'Osc Input', decay=4, damp=0.3, hard=0.4, couple=0.5, level=0.5)
p.gr('b', 'fixed', sample='Organic/Wood Creak', pos=0.3, size=0.5, density=10, level=0.4).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Creak').mod('m1', 'blevel', 0.5).macro(2, 'Box').mod('m2', 'at1', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('wood', 'resonant', 'organic', 'physical')
X(p, vib=0, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Digital Snowfall', T).wt('a', 'Digital/Bitcrush Sine', pos=0.3, voices=3, detune=8, level=0.4)
p.gr('b', 'fixed', sample='Textures/Granular Dust', pos=0.3, size=0.05, density=100, spray=0.9, level=0.3) \
 .add('c', 'Glass', 'Octaves', morph=0.4, partials=48, shim=0.4, oct=1, level=0.3) \
 .f1('SVF LP12', 9000, res=0.05, key=0.1).aenv(*LONG).crush(12, 2, 0.2) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Snow').mod('m1', 'blevel', 0.5).macro(2, 'Crystal').mod('m2', 'clevel', 0.5).macro(3, 'Bits').mod('m3', 'fxcrush', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('winter', 'digital', 'sparkle')
X(p, press=[('fxcrush', 0.3)], slide=[('at1', 0.5)])

p = P('Gamelan Cloud Chamber', T).gr('a', 'keyed', sample='Metal/Gamelan Pulse', pos=0.3, scan=0.02, size=0.3, density=30, spray=0.5, prand=0, level=0.5)
p.wt('b', 'Metallic/Gamelan Glow', pos=0.3, blend='spectral', level=0.35, voices=2, detune=6).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Cloud').mod('m1', 'at3', 0.5).macro(2, 'Glow').mod('m2', 'bt1', 0.5).macro(3, 'Size').mod('m3', 'at2', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('gamelan', 'granular', 'metallic')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Filter FM Breather', T).an('a', 'saw', voices=3, detune=10, slop=0.4, level=0.5)
p.an('b', 'sine', oct=-1, level=0.0).off('c').f1('SEM BP12', 900, res=0.35, key=0.3, fmsrc='b', fmamt=0.3).f2('SVF LP12', 5000).aenv(*LONG) \
 .lfo(1, 0.03, shape=BREATH).mod('lfo1', 'f1fm', 0.4).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Filter FM').mod('m1', 'f1fm', 0.5).macro(2, 'Band').mod('m2', 'f1cut', 0.5).macro(3, 'Res').mod('m3', 'f1res', 0.4) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('filter-fm', 'breathing', 'analog')
X(p, press=[('f1fm', 0.4)], slide=[('f1cut', 0.4)])

p = P('Shimmer Spectral Freeze', T).add('a', 'Strings', 'Glass', morph=0.4, partials=96, shim=0.5, srate=0.3, freeze=False, blur=3, level=0.55)
p.wt('b', 'Textures/Spectral Freeze', pos=0.3, blend='spectral', voices=3, detune=8, level=0.35).off('c') \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG).shifter(0.15, 0.2, 0.5, 0.9) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Morph').mod('m1', 'at1', 0.5).macro(2, 'Blur').mod('m2', 'bt1', 0.5).macro(3, 'Shimmer').mod('m3', 'fxshiftmix', 0.4) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('shimmer', 'frozen', 'corbacho', 'shifter')
X(p, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Sympathetic Swarm', T).ph('a', 'Sympathetic', 'Rain', decay=6, damp=0.25, hard=0.5, buzz=0.9, spread=0.9, level=0.55)
p.ph('b', 'Tanpura', 'Bow', decay=10, damp=0.1, hard=0.4, buzz=0.6, oct=-1, level=0.3).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Swarm').mod('m1', 'at1', 0.5).macro(2, 'Drone').mod('m2', 'blevel', 0.5).macro(3, 'Buzz').mod('m3', 'at4', -0.4) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('sympathetic', 'strings', 'roach', 'shimmering')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Electro Static Field', T).noise(0.4, 0.8).an('a', 'pulse', pw=0.1, oct=1, voices=3, detune=40, level=0.2, slop=0.8)
p.gr('b', 'fixed', sample='Textures/Vinyl Crackle Bed', pos=0.3, size=0.3, density=20, level=0.35).off('c') \
 .f1('SVF BP12', 3000, res=0.4, key=0.0).f2('SVF HP12', 300).aenv(*LONG) \
 .lfo(1, 0.4, 'sh').lfo(2, 0.03, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo2', 'noise', 0.2) \
 .macro(1, 'Static').mod('m1', 'noise', 0.5).macro(2, 'Crackle').mod('m2', 'blevel', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.1, 0.3).tags('noise', 'static', 'electric', 'texture')
X(p, vib=0, press=[('f1res', 0.35)], slide=[('f1cut', 0.4)])

p = P('Slow Mutation Garden', T).wt('a', 'Drift Spectra/Drift Lichen', pos=0.0, blend='spectral', voices=3, detune=8, level=0.5)
p.wt('b', 'Drift Spectra/Drift Ochre', pos=1.0, blend='spectral', voices=3, detune=8, level=0.4, semi=7).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .mseg(1, [(0, 0.0, 0), (30, 1.0, 0), (30, 0.0, 0)], loop=(0, 2), mode='free').mod('mseg1', 'at1', 1.0).mod('mseg1', 'bt1', -1.0) \
 .macro(1, 'Mutation').mod('m1', 'mseg1rate', 0.6).macro(2, 'Fifth').mod('m2', 'blevel', 0.5).macro(3, 'Light').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.2, 0.04, 0.5).tags('evolving', 'slow', 'organic', 'corbacho', 'mseg')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.4)])

p = P('Ritual Bone Rattle', T).gr('a', 'fixed', sample='Organic/Seed Rattle', pos=0.3, size=0.2, density=30, spray=0.8, level=0.45)
p.ph('b', 'Tube', 'Breath', decay=3, damp=0.4, hard=0.3, oct=-1, level=0.4) \
 .add('c', 'Throat', 'Reed', morph=0.3, partials=40, peak=7, peakamt=0.6, oct=-1, level=0.3) \
 .f1('Vowel', 600, res=0.4, key=0.0, shape=0.3).aenv(*LONG) \
 .lfo(1, 2.0, shape=TRIBAL, sync=True, div='1 bar').mod('lfo1', 'alevel', 0.3).lfo(2, 0.03, 'drift').mod('lfo2', 'f1shape', 0.4) \
 .macro(1, 'Rattle').mod('m1', 'alevel', 0.5).macro(2, 'Breath').mod('m2', 'blevel', 0.5).macro(3, 'Chant').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('tribal', 'ritual', 'roach', 'shamanic')
X(p, vib=0, press=[('f1shape', 0.4)], slide=[('ct2', 0.5)])

p = P('Aurora Grain Curtain', T).gr('a', 'wt', lib='Textures/Aurora', pos=0.2, scan=0.02, size=0.8, density=16, spray=0.5, prand=5, level=0.6)
p.gr('b', 'wt', lib='Ambient/Ice Cathedral', pos=0.4, scan=-0.02, size=0.6, density=12, spray=0.5, prand=1200, level=0.2).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.012, 'sine').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Curtain').mod('m1', 'at1', 0.5).macro(2, 'Octave Grains').mod('m2', 'blevel', 0.5).macro(3, 'Size').mod('m3', 'at2', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).width(1.5).tags('aurora', 'granular', 'corbacho', 'shimmer')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Sync Sweep Organism', T).an('a', 'saw', sync=12, voices=3, detune=10, slop=0.4, level=0.45)
p.an('b', 'pulse', pw=0.3, sync=7, semi=-5, level=0.3).off('c') \
 .f1('Xpander LP4', 2200, res=0.2, key=0.3).aenv(*LONG) \
 .lfo(1, 0.04, 'drift', 'voice').lfo(2, 0.07, 'sine').mod('lfo1', 'at2', 0.4).mod('lfo2', 'bt2', 0.4).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Sync').mod('m1', 'at2', 0.5).mod('m1', 'bt2', 0.4).macro(2, 'Filter').mod('m2', 'f1cut', 0.5).macro(3, 'Pulse').mod('m3', 'bt1', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('sync', 'analog', 'organic', 'virus')
X(p, press=[('at2', 0.4)], slide=[('f1cut', 0.4)])

p = P('Ghost Frequencies', T).wt('a', 'Spectral/Vocoder Bands', pos=0.3, blend='spectral', voices=3, detune=8, level=0.45)
p.add('b', 'Hollow', 'Primes', morph=0.5, partials=64, comb=0.5, cshift=0.3, shim=0.3, level=0.35).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG).shifter(0.08, 0.3, 0.6, 0.9) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Bands').mod('m1', 'at1', 0.5).macro(2, 'Comb').mod('m2', 'bt2', 0.5).macro(3, 'Ghost Shift').mod('m3', 'fxshiftmix', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('ghost', 'spectral', 'eerie', 'shifter')
X(p, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Tidal Grains', T).gr('a', 'fixed', sample='Nature/Ocean Swell', pos=0.3, scan=0.01, size=0.3, density=40, spray=0.4, level=0.5)
p.gr('b', 'keyed', sample='Voices/Choir Oo', pos=0.3, size=0.5, density=16, spray=0.3, level=0.35).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'at3', 0.5).mod('lfo1', 'blevel', 0.3) \
 .macro(1, 'Tide').mod('m1', 'at3', 0.5).macro(2, 'Choir').mod('m2', 'blevel', 0.5).macro(3, 'Grain Size').mod('m3', 'at2', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('ocean', 'granular', 'choir', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Phaser Cathedral Dust', T).wt('a', 'Organs/Organ Flutes', pos=0.3, voices=3, detune=6, level=0.45)
p.gr('b', 'fixed', sample='Textures/Granular Dust', pos=0.3, size=0.1, density=50, spray=0.7, level=0.25).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG).phaser(0.5, 0.03, 0.85, 0.7) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Stops').mod('m1', 'at1', 0.5).macro(2, 'Dust').mod('m2', 'blevel', 0.5).macro(3, 'Phaser').mod('m3', 'fxphaser', 0.5) \
 .chorus('chorus', 0.1, 0.04, 0.3).tags('phaser', 'organ', 'dust', 'psychedelic')
X(p, press=[('fxphaser', 0.4)], slide=[('at1', 0.5)])

p = P('Microtonal Swarm', T).an('a', 'saw', voices=7, detune=60, slop=0.8, drift=0.8, level=0.45)
p.an('b', 'tri', voices=5, detune=45, slop=0.8, oct=1, level=0.25).off('c') \
 .f1('SEM LP12', 2500, res=0.1, key=0.3).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'adetune', 0.4).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Swarm').mod('m1', 'adetune', 0.5).mod('m1', 'bdetune', 0.5).macro(2, 'Filter').mod('m2', 'f1cut', 0.5).macro(3, 'High').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.08, 0.5).tags('microtonal', 'swarm', 'detuned', 'analog')
X(p, vib=0, press=[('adetune', 0.4)], slide=[('f1cut', 0.4)])

p = P('Harmonic Glissando Mist', T).gr('a', 'keyed', sample='Strings/Harmonic Glissando', pos=0.3, scan=0.04, size=0.5, density=16, spray=0.4, level=0.5)
p.add('b', 'Strings', 'Octaves', morph=0.4, partials=64, shim=0.25, level=0.35).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Glissando').mod('m1', 'at1', 0.5).macro(2, 'Mist').mod('m2', 'blevel', 0.5).macro(3, 'Density').mod('m3', 'at3', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('strings', 'harmonics', 'granular', 'mist')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Undertow Machine', T).wt('a', 'Drone/Undertow', pos=0.3, blend='spectral', voices=3, detune=8, oct=-1, level=0.55)
p.ph('b', 'Tube', 'Osc Input', decay=5, damp=0.3, hard=0.4, level=0.3).off('c') \
 .f1('Ladder LP24', 700, res=0.25, key=0.2).aenv(*LONG) \
 .lfo(1, 0.25, 'tri', sync=True, div='1 bar').lfo(2, 0.02, 'drift').mod('lfo1', 'f1cut', 0.2).mod('lfo2', 'at1', 0.5) \
 .macro(1, 'Pull').mod('m1', 'at1', 0.5).macro(2, 'Tube').mod('m2', 'blevel', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('dark', 'machine', 'roach', 'tempo')
X(p, press=[('f1res', 0.3)], slide=[('at1', 0.5)])

p = P('Cicada Spectral Bloom', T).gr('a', 'fixed', sample='Nature/Cicadas', pos=0.3, size=0.5, density=14, level=0.35)
p.add('b', 'Glass', 'Octaves', morph=0.4, partials=80, shim=0.4, srate=0.2, level=0.45).off('c') \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*LONG).curve(a=-0.6) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Insects').mod('m1', 'alevel', 0.5).macro(2, 'Bloom').mod('m2', 'bt1', 0.5).macro(3, 'Shimmer').mod('m3', 'bt2', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('summer', 'shimmer', 'corbacho')
X(p, press=[('bt2', 0.4)], slide=[('bt1', 0.5)])

p = P('Tape Saturated Haze', T).wt('a', 'Textures/Tape Saw', pos=0.3, voices=4, detune=12, level=0.45)
p.wt('b', 'Textures/Breathing Haze', pos=0.3, blend='spectral', level=0.35).off('c') \
 .f1('Ladder LP12', 2000, res=0.1, key=0.2).aenv(*LONG).tape(0.45, 0.3, 0.15, 0.5, 1.0) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Saturation').mod('m1', 'fxdrive', 0.5).macro(2, 'Haze').mod('m2', 'blevel', 0.5).macro(3, 'Wow').mod('m3', 'fxwow', 0.5) \
 .drive(0.15).chorus('solina', 0.25, 0.08, 0.5).tags('tape', 'lo-fi', 'warm', 'haze')
X(p, press=[('f1cut', 0.3)], slide=[('at1', 0.5)])

p = P('Cavern Water Drops', T).gr('a', 'fixed', sample='Nature/Water Drops', pos=0.3, size=0.3, density=8, spray=0.8, level=0.45)
p.wt('b', 'Drone/Cave Resonance', pos=0.3, blend='spectral', oct=-1, voices=3, detune=6, level=0.35) \
 .ph('c', 'Tube', 'Rain', decay=2, damp=0.3, hard=0.4, spread=1.0, oct=1, level=0.2) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Drops').mod('m1', 'alevel', 0.5).macro(2, 'Cavern').mod('m2', 'blevel', 0.5).macro(3, 'Echo Drops').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('cave', 'water', 'roach', 'dark')
X(p, vib=0, press=[('clevel', 0.3)], slide=[('bt1', 0.5)])

p = P('Quantum Foam', T).wt('a', 'Spectral/Phase Scatter', pos=0.3, blend='spectral', voices=4, detune=14, level=0.45)
p.an('b', 'sine', semi=5, level=0.0).fm('a', 'b', 0.2, 'fm').gr('c', 'fixed', sample='Textures/Granular Dust', pos=0.3, size=0.04, density=150, spray=1.0, level=0.2) \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.13, 'sh').lfo(2, 0.03, 'drift').mod('lfo1', 'afm', 0.3).mod('lfo2', 'at1', 0.5) \
 .macro(1, 'Foam').mod('m1', 'afm', 0.5).macro(2, 'Scatter').mod('m2', 'at1', 0.5).macro(3, 'Dust').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.08, 0.4).tags('sci-fi', 'fm', 'granular', 'abstract')
X(p, press=[('afm', 0.4)], slide=[('at1', 0.5)])

p = P('Silk Road Caravan', T).wt('a', 'Organic/Didgeridoo Morph', pos=0.3, voices=2, detune=5, oct=-1, level=0.45)
p.gr('b', 'keyed', sample='Drones/Harmonium Reed', pos=0.3, size=0.6, density=12, spray=0.3, level=0.35) \
 .gr('c', 'fixed', sample='Metal/Wind Chimes', pos=0.3, size=0.5, density=6, spray=0.7, level=0.2) \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Drone').mod('m1', 'alevel', 0.5).macro(2, 'Reed').mod('m2', 'blevel', 0.5).macro(3, 'Bells').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('world', 'desert', 'caravan', 'roach')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Liquid Formant Flow', T).an('a', 'saw', voices=3, detune=10, level=0.45, slop=0.4)
p.add('b', 'Choir Ah', 'Choir Oo', morph=0.5, partials=48, vowel=0.5, vowelamt=0.8, shim=0.2, level=0.35).off('c') \
 .f1('Vowel', 900, res=0.5, key=0.0, shape=0.5, morphto='Formant', morph=0.3).aenv(*LONG) \
 .lfo(1, 0.09, 'sine').lfo(2, 0.037, 'tri').mod('lfo1', 'f1shape', 0.4).mod('lfo2', 'bt4', 0.6).mod('lfo2', 'f1morph', 0.4) \
 .macro(1, 'Vowel').mod('m1', 'f1shape', 0.6).macro(2, 'Liquid').mod('m2', 'f1morph', 0.5).macro(3, 'Choir').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('vocal', 'formant', 'liquid', 'talking')
X(p, press=[('f1shape', 0.4)], slide=[('bt4', 0.5)])

p = P('Bronze Age Hum', T).wt('a', 'Metallic/Gong Body', pos=0.3, blend='spectral', voices=3, detune=7, oct=-1, level=0.5)
p.ph('b', 'Plate', 'Bow', decay=16, damp=0.12, hard=0.55, couple=0.4, oct=-1, level=0.3).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Bronze').mod('m1', 'at1', 0.5).macro(2, 'Plate').mod('m2', 'blevel', 0.5).macro(3, 'Bow').mod('m3', 'bt2', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.4).tags('metallic', 'ancient', 'roach', 'dark')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Thousand Flutes', T).gr('a', 'wt', lib='Organic/Shakuhachi', pos=0.3, scan=0.03, size=0.3, density=40, spray=0.5, prand=15, level=0.5)
p.wt('b', 'Organic/Wood Flute', pos=0.3, voices=5, detune=18, level=0.35).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bdetune', 0.3) \
 .macro(1, 'Flutes').mod('m1', 'at3', 0.5).macro(2, 'Section').mod('m2', 'blevel', 0.5).macro(3, 'Breath').mod('m3', 'at4', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('flute', 'ensemble', 'granular', 'pastoral')
X(p, vib=0.02, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Neural Dream', T).wt('a', 'Stepped/Wave Steps Choir', pos=0.3, blend='spectral', voices=3, detune=8, level=0.45)
p.add('b', 'Choir Ah', 'Primes', morph=0.5, partials=64, shim=0.4, level=0.3) \
 .an('c', 'sine', oct=-1, level=0.0).fm('b', 'c', 0.15, 'pm').f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.2, 'sh', smooth=0.6).lfo(2, 0.03, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'bfm', 0.2) \
 .macro(1, 'Thoughts').mod('m1', 'at1', 0.5).macro(2, 'Dream').mod('m2', 'bt1', 0.5).macro(3, 'FM').mod('m3', 'bfm', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('dream', 'abstract', 'evolving')
X(p, press=[('bfm', 0.4)], slide=[('at1', 0.5)])

p = P('Cathedral Bell Grains', T).gr('a', 'keyed', sample='Metal/Singing Bowl Pair', pos=0.3, scan=0.02, size=0.4, density=24, spray=0.4, prand=0, level=0.5)
p.wt('b', 'Glass & Bell/Bell Cluster', pos=0.3, blend='spectral', level=0.3, voices=2, detune=6).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Grains').mod('m1', 'at3', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.5).macro(3, 'Scan').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('bells', 'granular', 'sacred')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Wind Through Wires', T).noise(0.5, 0.5).an('a', 'sine', level=0.25)
p.gr('b', 'fixed', sample='Nature/Wind Whistle', pos=0.3, size=0.8, density=9, level=0.35).off('c') \
 .f1('SVF Peak', 1200, res=0.75, key=1.0).f2('SVF Peak', 2400, res=0.7, key=1.0).route('parallel', 0.5).aenv(*LONG) \
 .lfo(1, 0.05, 'drift').lfo(2, 0.08, 'drift', 'voice').mod('lfo1', 'f1cut', 0.15).mod('lfo2', 'f2cut', 0.15).mod('lfo1', 'noise', 0.2) \
 .macro(1, 'Wind').mod('m1', 'noise', 0.5).macro(2, 'Whistle').mod('m2', 'f1res', 0.3).macro(3, 'Wires').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('wind', 'resonant', 'eerie', 'pitched-noise')
X(p, vib=0.01, press=[('noise', 0.3)], slide=[('balance', 0.4)])
