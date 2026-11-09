# Driftfield 3.1 bank, part C: PADS
D = 'Pads'
PAD = (2.5, 3, 1, 9)
SLOWPAD = (5, 4, 1, 14)

p = P('OB Velvet Brass Pad', D).an('a', 'saw', voices=3, detune=12, slop=0.4, drift=0.4, level=0.55)
p.an('b', 'pulse', pw=0.4, voices=2, detune=9, slop=0.4, semi=0, fine=7, level=0.4).off('c') \
 .f1('OB-Xa LP24', 1200, res=0.15, key=0.4, env=0.35).fenv(1.2, 3, 0.4, 4).aenv(*PAD).curve(a=0.2, f=0.3) \
 .lfo(1, 0.3, 'tri').mod('lfo1', 'bt1', 0.15).lfo(2, 0.05, 'drift', 'voice').mod('lfo2', 'f1cut', 0.12) \
 .macro(1, 'Brass').mod('m1', 'f1cut', 0.5).macro(2, 'PWM').mod('m2', 'bt1', 0.4).macro(3, 'Env').mod('m3', 'f1res', 0.3).macro(4, 'Width').mod('m4', 'adetune', 0.4) \
 .chorus('ensemble', 0.3, 0.12, 0.5).tags('oberheim', 'brass', 'analog', 'warm', '80s')
X(p, press=[('f1cut', 0.35), ('f1res', 0.2)], slide=[('bt1', 0.4)])

p = P('Virus Hyper Lush', D).an('a', 'supersaw', pw=0.6, detune=30, voices=3, level=0.5)
p.wt('b', 'Analog/Saw Lowpass Sweep', pos=0.5, voices=5, detune=22, level=0.3, oct=1).off('c') \
 .f1('Ladder LP24', 3000, res=0.2, key=0.3, env=0.2).fenv(0.5, 2, 0.6, 3).aenv(*PAD) \
 .lfo(1, 0.08, 'tri').mod('lfo1', 'at1', 0.2).mod('lfo1', 'bt1', 0.25) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Hyper').mod('m2', 'at1', 0.5).macro(3, 'Shine').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.35, 0.1, 0.6).width(1.4).tags('virus', 'supersaw', 'trance', 'lush')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Slow Breath Strings', D).wt('a', 'Strings & Reeds/Ensemble Soft', pos=0.3, voices=5, detune=14, level=0.55)
p.an('b', 'saw', oct=-1, voices=2, detune=8, slop=0.4, level=0.3).off('c') \
 .f1('SEM LP12', 2600, res=0.05, key=0.3).aenv(*SLOWPAD).curve(a=-0.4) \
 .lfo(1, 0.04, shape=BREATH).mod('lfo1', 'f1cut', 0.25).mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Bow').mod('m1', 'at1', 0.5).macro(2, 'Celli').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('solina', 0.35, 0.1, 0.6).tags('strings', 'solina', 'slow', 'cinematic')
X(p, vib=0.016, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Glass Choir Pad', D).add('a', 'Choir Ah', 'Glass', morph=0.4, partials=64, shim=0.25, srate=0.1, pdet=6, level=0.55)
p.wt('b', 'Glass & Bell/Halo', pos=0.3, blend='spectral', voices=3, detune=8, level=0.3, oct=1).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'sine').mod('lfo1', 'at1', 0.4).lfo(2, 0.06, 'drift', 'voice').mod('lfo2', 'bt1', 0.3) \
 .macro(1, 'Glass').mod('m1', 'at1', 0.5).macro(2, 'Halo').mod('m2', 'blevel', 0.5).macro(3, 'Vowel').mod('m3', 'at4', 0.5) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('choir', 'glass', 'corbacho', 'luminous')
X(p, press=[('at1', 0.35)], slide=[('at4', 0.6)])

p = P('Dreamtime Pad', D).wt('a', 'Ambient/Dreamtime Reed', pos=0.3, blend='spectral', voices=3, detune=10, level=0.5)
p.add('b', 'Reed', 'Throat', morph=0.3, partials=48, peak=8, peakamt=0.4, shim=0.15, oct=-1, level=0.35).off('c') \
 .f1('SVF LP12', 4000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Reed').mod('m1', 'at1', 0.5).macro(2, 'Throat').mod('m2', 'bt2', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.2, 0.05, 0.4).tags('dreamtime', 'roach', 'reed', 'earthy')
X(p, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Polysynth Sunset', D).an('a', 'sawpulse', pw=0.45, voices=2, detune=10, slop=0.4, level=0.5)
p.an('b', 'saw', semi=7, voices=2, detune=8, slop=0.4, level=0.25).off('c') \
 .f1('Ladder LP24', 1600, res=0.25, key=0.4, env=0.3).fenv(1.5, 4, 0.5, 5).aenv(*PAD).curve(f=0.3) \
 .lfo(1, 0.15, 'tri').mod('lfo1', 'at1', 0.15).lfo(2, 0.04, 'drift').mod('lfo2', 'f1cut', 0.15) \
 .macro(1, 'Glow').mod('m1', 'f1cut', 0.5).macro(2, 'Fifth').mod('m2', 'blevel', 0.5).macro(3, 'Resonance').mod('m3', 'f1res', 0.4) \
 .chorus('ensemble', 0.3, 0.1, 0.5).tags('analog', 'warm', 'sunset', '80s')
X(p, press=[('f1cut', 0.35)], slide=[('f1res', 0.3)])

p = P('Spectral Morph Pad', D).wt('a', 'Drift Spectra/Drift Haze', pos=0.2, blend='spectral', voices=4, detune=10, level=0.5)
p.wt('b', 'Drift Spectra/Drift Lumen', pos=0.6, blend='spectral', voices=3, detune=8, level=0.35, oct=1).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.025, 'sine').lfo(2, 0.017, 'tri').mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.6) \
 .macro(1, 'Morph A').mod('m1', 'at1', 0.5).macro(2, 'Morph B').mod('m2', 'bt1', 0.5).macro(3, 'Octave').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('spectral', 'evolving', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Chord Memory Pad', D).wt('a', 'Chords/Chord Major', pos=0.3, blend='spectral', voices=4, detune=12, level=0.5)
p.an('b', 'saw', oct=-1, voices=2, detune=8, level=0.25).off('c').f1('OB-Xa LP12', 2200, res=0.1, key=0.3).aenv(*PAD) \
 .lfo(1, 0.04, 'sine').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Brightness').mod('m1', 'at1', 0.5).macro(2, 'Bass').mod('m2', 'blevel', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.08, 0.5).tags('chord', 'one-finger', 'house', 'warm')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Tundra Hymn', D).add('a', 'Organ', 'Choir Oo', morph=0.5, partials=56, tilt=-0.2, shim=0.2, srate=0.05, pdet=5, level=0.55)
p.wt('b', 'Ambient/Ice Cathedral', pos=0.3, blend='spectral', voices=3, detune=7, level=0.3).off('c') \
 .f1('SVF LP12', 4500, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Hymn').mod('m1', 'at1', 0.5).macro(2, 'Ice').mod('m2', 'blevel', 0.5).macro(3, 'Tilt').mod('m3', 'at3', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('cold', 'hymn', 'corbacho', 'organ')
X(p, press=[('at3', 0.35)], slide=[('at1', 0.5)])

p = P('Analog Cathedral', D).an('a', 'saw', voices=4, detune=14, slop=0.5, level=0.45)
p.an('b', 'saw', oct=-1, voices=3, detune=10, slop=0.5, level=0.35).an('c', 'pulse', pw=0.25, oct=1, voices=2, detune=12, level=0.2) \
 .f1('SEM LP12', 1800, res=0.1, key=0.3).f2('SEM HP12', 120).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.3).lfo(2, 0.2, 'tri').mod('lfo2', 'ct1', 0.2) \
 .macro(1, 'Light').mod('m1', 'f1cut', 0.5).macro(2, 'Low').mod('m2', 'blevel', 0.5).macro(3, 'High').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.35, 0.08, 0.6).tags('analog', 'big', 'roach', 'cathedral')
X(p, press=[('f1cut', 0.35)], slide=[('ct1', 0.4)])

p = P('Velvet Vowel Pad', D).wt('a', 'Vowel Journeys/Alto Bloom', pos=0.2, blend='spectral', voices=4, detune=10, level=0.55)
p.an('b', 'tri', oct=-1, level=0.25, slop=0.4).off('c').f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'tri').mod('lfo1', 'at1', 0.6) \
 .macro(1, 'Vowel').mod('m1', 'at1', 0.5).macro(2, 'Body').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('vocal', 'vowel', 'velvet')
X(p, press=[('f1cut', 0.3)], slide=[('at1', 0.6)])

p = P('Fifths Under Ice', D).add('a', 'Fifths', 'Octaves', morph=0.4, partials=64, tilt=-0.3, shim=0.2, srate=0.06, level=0.55)
p.wt('b', 'Glass & Bell/Frost', pos=0.4, blend='spectral', voices=3, detune=8, oct=1, level=0.2).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Fifths').mod('m1', 'at1', 0.5).macro(2, 'Frost').mod('m2', 'blevel', 0.5).macro(3, 'Tilt').mod('m3', 'at3', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('fifths', 'cold', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at3', 0.5)])

p = P('Warm Tape Strings', D).an('a', 'saw', voices=5, detune=16, slop=0.5, level=0.5)
p.off('b', 'c').f1('SEM LP12', 2200, res=0.05, key=0.3).aenv(*PAD).tape(0.25, 0.3, 0.12, 0.55, 1.0) \
 .lfo(1, 0.05, 'drift').mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Tone').mod('m1', 'f1cut', 0.5).macro(2, 'Wow').mod('m2', 'fxwow', 0.5).macro(3, 'Detune').mod('m3', 'adetune', 0.4) \
 .chorus('solina', 0.4, 0.1, 0.6).tags('strings', 'tape', 'solina', 'vintage')
X(p, vib=0.015, press=[('f1cut', 0.35)], slide=[('adetune', 0.4)])

p = P('Sky Burial Pad', D).wt('a', 'Ambient/Magnificent Void', pos=0.3, blend='spectral', voices=4, detune=10, level=0.55)
p.add('b', 'Throat', 'Choir Ah', morph=0.4, partials=48, peak=9, peakamt=0.5, oct=-1, level=0.3).off('c') \
 .f1('SVF LP12', 3800, res=0.05, key=0.2).aenv(*SLOWPAD).curve(a=-0.3) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Void').mod('m1', 'at1', 0.5).macro(2, 'Chant').mod('m2', 'blevel', 0.5).macro(3, 'Overtone').mod('m3', 'bt2', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('tibet', 'vast', 'roach', 'sacred')
X(p, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Pressure Bloom Pad', D).an('a', 'saw', voices=3, detune=12, slop=0.4, level=0.5)
p.wt('b', 'Ambient/Canyon Glow', pos=0.2, blend='spectral', voices=3, detune=8, level=0.35).off('c') \
 .f1('Ladder LP24', 600, res=0.25, key=0.3).aenv(*PAD) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Bloom Depth').mod('m1', 'f1cut', 0.4).macro(2, 'Glow').mod('m2', 'bt1', 0.5).macro(3, 'Res').mod('m3', 'f1res', 0.4) \
 .chorus('ensemble', 0.3, 0.07, 0.5).tags('expressive', 'bloom', 'mpe-showcase')
X(p, pc=0.6, pa=0.6, press=[('f1cut', 0.5), ('bt1', 0.4), ('fxchorus', 0.3)], slide=[('f1res', 0.4)])

p = P('Saturn Ring Halo', D).wt('a', 'Ambient/Night Sky Shimmer', pos=0.3, blend='spectral', voices=5, detune=12, level=0.45)
p.add('b', 'Octaves', 'Glass', morph=0.5, partials=64, shim=0.35, oct=-1, level=0.35).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*SLOWPAD) \
 .lfo(1, 0.011, 'sine').lfo(2, 0.03, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.4).mod('lfo1', 'pan', 0.2) \
 .macro(1, 'Rings').mod('m1', 'at1', 0.5).macro(2, 'Planet').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'bt2', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).width(1.5).tags('space', 'shimmer', 'corbacho')
X(p, press=[('bt2', 0.35)], slide=[('at1', 0.5)])

p = P('Brass Section Swell', D).an('a', 'saw', voices=3, detune=8, slop=0.3, level=0.55)
p.an('b', 'saw', semi=-12, voices=2, detune=6, level=0.3).off('c') \
 .f1('Ladder LP24', 900, res=0.15, key=0.5, env=0.45).fenv(1.8, 2, 0.5, 2).aenv(1.2, 2, 1, 4).curve(a=-0.3, f=-0.2) \
 .macro(1, 'Brass').mod('m1', 'f1cut', 0.5).macro(2, 'Bite').mod('m2', 'f1res', 0.3).macro(3, 'Low').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.1, 0.4).tags('brass', 'swell', 'analog', 'expressive')
X(p, pc=0.6, press=[('f1cut', 0.45)], slide=[('f1res', 0.3)])

p = P('Liquid Crystal Pad', D).wt('a', 'Glass & Bell/Prism', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5, warp='mirror', warpamt=0.2)
p.wt('b', 'Glass & Bell/Crystal Squares', pos=0.3, blend='spectral', voices=2, detune=6, level=0.25, oct=1).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*PAD) \
 .lfo(1, 0.06, 'sine').mod('lfo1', 'at2', 0.4).lfo(2, 0.03, 'drift').mod('lfo2', 'at1', 0.4) \
 .macro(1, 'Prism').mod('m1', 'at1', 0.5).macro(2, 'Mirror').mod('m2', 'at2', 0.5).macro(3, 'Crystal').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('glass', 'crystal', 'bright')
X(p, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Dark Matter Pad', D).wt('a', 'Drone/Deep Murmur', pos=0.3, blend='spectral', voices=3, detune=8, level=0.55)
p.an('b', 'saw', oct=-1, voices=2, detune=7, slop=0.5, level=0.3).sub(0.2).off('c') \
 .f1('Ladder LP24', 600, res=0.3, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'f1cut', 0.35).mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Matter').mod('m1', 'at1', 0.5).macro(2, 'Filter').mod('m2', 'f1cut', 0.5).macro(3, 'Mass').mod('m3', 'sub', 0.4) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('dark', 'roach', 'deep')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Harmonic Cloud Pad', D).add('a', 'Octaves', 'Strings', morph=0.5, partials=96, tilt=-0.1, shim=0.3, srate=0.08, pdet=8, level=0.55)
p.off('b', 'c').f1('SVF LP12', 7000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'tri').mod('lfo1', 'at1', 0.5).lfo(2, 0.05, 'drift', 'voice').mod('lfo2', 'at3', 0.3) \
 .macro(1, 'Morph').mod('m1', 'at1', 0.5).macro(2, 'Tilt').mod('m2', 'at3', 0.5).macro(3, 'Shimmer').mod('m3', 'f1cut', 0.4) \
 .chorus('ensemble', 0.3, 0.05, 0.5).tags('additive', 'cloud', 'corbacho')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Juno Dream Pad', D).an('a', 'pulse', pw=0.35, voices=1, slop=0.3, level=0.55)
p.an('b', 'saw', voices=1, level=0.3).sub(0.3, 'square').off('c') \
 .f1('Ladder LP24', 1800, res=0.15, key=0.4).aenv(*PAD) \
 .lfo(1, 0.35, 'tri').mod('lfo1', 'at1', 0.25) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'PWM').mod('m2', 'at1', 0.4).macro(3, 'Sub').mod('m3', 'sub', 0.5) \
 .chorus('chorus', 0.5, 0.45, 0.6).tags('juno', 'analog', '80s', 'chorus')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.4)])

p = P('Monastery Morning', D).add('a', 'Choir Oo', 'Organ', morph=0.3, partials=56, vowel=0.6, vowelamt=0.5, shim=0.2, level=0.55)
p.gr('b', 'keyed', sample='Voices/Monk Hum Low', pos=0.3, size=0.8, density=10, spray=0.3, level=0.3).off('c') \
 .f1('SVF LP12', 4000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.025, shape=BREATH).mod('lfo1', 'at4', 0.4) \
 .macro(1, 'Vowel').mod('m1', 'at4', 0.5).macro(2, 'Monks').mod('m2', 'blevel', 0.5).macro(3, 'Organ').mod('m3', 'at1', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('choir', 'monks', 'morning', 'roach')
X(p, press=[('blevel', 0.3)], slide=[('at4', 0.6)])

p = P('Warp Bend Pad', D).wt('a', 'Analog/Reso Sweep Soft', pos=0.4, warp='bend+-', warpamt=0.3, voices=3, detune=10, level=0.5)
p.wt('b', 'Analog/Reso Sweep Hollow', pos=0.5, warp='asym', warpamt=0.3, voices=2, detune=7, oct=-1, level=0.3).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*PAD) \
 .lfo(1, 0.05, 'sine').lfo(2, 0.08, 'tri').mod('lfo1', 'at2', 0.4).mod('lfo2', 'bt2', 0.4).mod('lfo1', 'at1', 0.2) \
 .macro(1, 'Warp').mod('m1', 'at2', 0.5).mod('m1', 'bt2', 0.4).macro(2, 'Sweep').mod('m2', 'at1', 0.5).macro(3, 'Low').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.3, 0.07, 0.5).tags('warp', 'wavetable', 'modern')
X(p, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Desert Rose Pad', D).wt('a', 'Ambient/Desert Air', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.an('b', 'tri', semi=7, voices=2, detune=6, level=0.25, slop=0.4).off('c') \
 .f1('SVF BP12', 1500, res=0.15, key=0.3).f2('SVF LP12', 6000).route('parallel', 0.45).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Air').mod('m1', 'at1', 0.5).macro(2, 'Rose').mod('m2', 'blevel', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.2, 0.05, 0.4).tags('desert', 'roach', 'warm')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Ocean Floor Pad', D).wt('a', 'Drone/Tidal Drone', pos=0.3, blend='spectral', voices=3, detune=8, level=0.55)
p.add('b', 'Hollow', 'Choir Oo', morph=0.4, partials=40, shim=0.15, oct=-1, level=0.3).off('c') \
 .f1('Ladder LP24', 900, res=0.2, key=0.2).aenv(*SLOWPAD).phaser(0.25, 0.03, 0.6, 0.4) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Tide').mod('m1', 'at1', 0.5).macro(2, 'Depth').mod('m2', 'f1cut', -0.5).macro(3, 'Voice').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('ocean', 'deep', 'corbacho')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Silver Screen Strings', D).wt('a', 'Strings & Reeds/Viola Body', pos=0.3, voices=5, detune=12, level=0.5)
p.wt('b', 'Strings & Reeds/Cello Body', pos=0.3, voices=3, detune=10, oct=-1, level=0.35).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.3).aenv(1.8, 3, 1, 6).curve(a=-0.2) \
 .lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Bow').mod('m1', 'at1', 0.5).macro(2, 'Celli').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('strings', 'cinematic', 'orchestral')
X(p, vib=0.02, rate=5.5, press=[('f1cut', 0.4)], slide=[('at1', 0.5)])

p = P('Quiet Revolution', D).an('a', 'tri', voices=3, detune=10, slop=0.3, level=0.5)
p.add('b', 'Hollow', 'Organ', morph=0.3, partials=32, shim=0.15, level=0.3).off('c') \
 .f1('SVF LP12', 3000, res=0.05, key=0.3).aenv(*PAD) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.25) \
 .macro(1, 'Warmth').mod('m1', 'f1cut', 0.5).macro(2, 'Organ').mod('m2', 'blevel', 0.5).macro(3, 'Detune').mod('m3', 'adetune', 0.4) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('soft', 'calm', 'intimate')
X(p, press=[('f1cut', 0.35)], slide=[('bt1', 0.4)])

p = P('Glacial Organ Pad', D).wt('a', 'Organs/Organ Celeste', pos=0.3, voices=3, detune=8, level=0.5)
p.wt('b', 'Glass & Bell/Ice Shimmer', pos=0.3, blend='spectral', oct=1, level=0.2, voices=2, detune=6).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Stops').mod('m1', 'at1', 0.5).macro(2, 'Ice').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('organ', 'cold', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Sync Choir Hybrid', D).an('a', 'saw', sync=5, voices=3, detune=10, level=0.45)
p.add('b', 'Choir Ah', 'Choir Oo', morph=0.4, partials=48, shim=0.2, level=0.35).off('c') \
 .f1('SEM LP12', 3000, res=0.1, key=0.3).aenv(*PAD) \
 .lfo(1, 0.04, 'tri').mod('lfo1', 'at2', 0.3) \
 .macro(1, 'Sync').mod('m1', 'at2', 0.5).macro(2, 'Choir').mod('m2', 'blevel', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.07, 0.5).tags('sync', 'choir', 'hybrid')
X(p, press=[('at2', 0.4)], slide=[('bt1', 0.5)])

p = P('Earth Song Pad', D).wt('a', 'Ambient/Earth Pulse', pos=0.3, blend='spectral', voices=3, detune=8, level=0.5)
p.add('b', 'Throat', 'Reed', morph=0.4, partials=48, peak=7, peakamt=0.5, level=0.3).sub(0.2).off('c') \
 .f1('SVF LP12', 3500, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Earth').mod('m1', 'at1', 0.5).macro(2, 'Throat').mod('m2', 'bt2', 0.5).macro(3, 'Low').mod('m3', 'sub', 0.4) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('earthy', 'roach', 'organic')
X(p, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Heaven Gate Pad', D).add('a', 'Choir Ah', 'Octaves', morph=0.5, partials=80, shim=0.3, srate=0.08, pdet=8, level=0.5)
p.wt('b', 'Ambient/Altar of Light', pos=0.3, blend='spectral', voices=4, detune=10, oct=1, level=0.25).off('c') \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*SLOWPAD).curve(a=-0.5) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Choir').mod('m1', 'at1', 0.5).macro(2, 'Light').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'at2', 0.5) \
 .chorus('ensemble', 0.3, 0.05, 0.6).width(1.4).tags('heavenly', 'choir', 'corbacho', 'luminous')
X(p, press=[('blevel', 0.35)], slide=[('at1', 0.5)])

p = P('Retro Sci-Fi Pad', D).an('a', 'pulse', pw=0.15, voices=3, detune=14, level=0.45)
p.wt('b', 'Phase Distortion/PD Saw Resonance', pos=0.3, voices=2, detune=8, level=0.3).off('c') \
 .f1('MS-20 LP12', 1500, res=0.4, key=0.3).aenv(*PAD) \
 .lfo(1, 0.2, 'sine').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', 0.3).lfo(2, 0.03, 'drift').mod('lfo2', 'f1cut', 0.3) \
 .macro(1, 'Filter').mod('m1', 'f1cut', 0.5).macro(2, 'PD').mod('m2', 'bt1', 0.5).macro(3, 'Res').mod('m3', 'f1res', 0.3) \
 .chorus('flanger', 0.2, 0.05, 0.5, 0.3).tags('sci-fi', 'retro', 'analog')
X(p, press=[('f1res', 0.3)], slide=[('bt1', 0.5)])

p = P('Distant Shores Pad', D).wt('a', 'Ambient/Blue Hour', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.add('b', 'Hollow', 'Octaves', morph=0.5, partials=48, shim=0.2, level=0.3).off('c') \
 .f1('SVF LP12', 5500, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Tide').mod('m1', 'at1', 0.5).macro(2, 'Hollow').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('ocean', 'nostalgic', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Midnight Analog Pad', D).an('a', 'saw', voices=2, detune=9, slop=0.4, level=0.5)
p.an('b', 'saw', oct=-1, fine=-6, slop=0.4, level=0.35).off('c') \
 .f1('Ladder LP24', 900, res=0.2, key=0.4, env=0.2).fenv(3, 4, 0.5, 6).aenv(*PAD) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Res').mod('m2', 'f1res', 0.4).macro(3, 'Low').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.08, 0.5).tags('analog', 'dark', 'night', 'moog')
X(p, press=[('f1cut', 0.4)], slide=[('f1res', 0.3)])

p = P('Wind Organ Pad', D).wt('a', 'Organs/Organ Flutes', pos=0.3, voices=3, detune=6, level=0.5)
p.noise(0.15, 0.4).off('b', 'c').f1('SVF LP12', 4500, res=0.05, key=0.2).aenv(*PAD) \
 .lfo(1, 0.05, shape=BREATH).mod('lfo1', 'noise', 0.15).mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Stops').mod('m1', 'at1', 0.5).macro(2, 'Wind').mod('m2', 'noise', 0.4).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('organ', 'breathy', 'wind')
X(p, press=[('noise', 0.3)], slide=[('at1', 0.5)])

p = P('Spectral Sunrise', D).add('a', 'Saw', 'Glass', morph=0.0, partials=80, tilt=-0.5, shim=0.25, level=0.55)
p.off('b', 'c').f1('SVF LP12', 8000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .mseg(1, [(0, 0.0, 0), (20, 1.0, -0.4)], loop=(1, 1), mode='env').mod('mseg1', 'at1', 0.8).mod('mseg1', 'at3', 0.6) \
 .macro(1, 'Sun').mod('m1', 'at1', 0.5).macro(2, 'Tilt').mod('m2', 'at3', 0.5).macro(3, 'Shimmer').mod('m3', 'at2', 0.4) \
 .chorus('ensemble', 0.3, 0.05, 0.5).tags('sunrise', 'additive', 'slow', 'corbacho', 'mseg')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Formant Choir Pad', D).an('a', 'saw', voices=4, detune=12, slop=0.4, level=0.5)
p.off('b', 'c').f1('Formant', 800, res=0.5, key=0.0, shape=0.5).f2('SVF LP12', 5000).aenv(*PAD) \
 .lfo(1, 0.04, 'tri').mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Vowel').mod('m1', 'f1cut', 0.5).macro(2, 'Res').mod('m2', 'f1res', 0.4).macro(3, 'Detune').mod('m3', 'adetune', 0.4) \
 .chorus('ensemble', 0.3, 0.07, 0.5).tags('formant', 'choir', 'analog')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.5)])

p = P('Lunar Tide Pad', D).wt('a', 'Drift Spectra/Drift Tide', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.an('b', 'sine', oct=-1, level=0.25).off('c').f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'at1', 0.6).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Tide').mod('m1', 'at1', 0.5).macro(2, 'Moon').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('moon', 'tide', 'corbacho')
X(p, press=[('f1cut', 0.3)], slide=[('at1', 0.5)])

p = P('Strings of Ether', D).add('a', 'Strings', 'Octaves', morph=0.3, partials=72, shim=0.25, pdet=10, level=0.5)
p.wt('b', 'Strings & Reeds/String Harmonics', pos=0.3, voices=3, detune=8, oct=1, level=0.2).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Ether').mod('m1', 'at1', 0.5).macro(2, 'Harmonics').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'at2', 0.4) \
 .chorus('ensemble', 0.3, 0.05, 0.5).tags('strings', 'ethereal', 'corbacho')
X(p, vib=0.015, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Bitter Cold Pad', D).wt('a', 'Glass & Bell/Frost', pos=0.4, blend='spectral', voices=4, detune=10, level=0.5)
p.noise(0.1, 0.8).an('b', 'tri', oct=-1, level=0.2).off('c').f1('SVF HP12', 200).f2('SVF LP12', 7000).aenv(*PAD) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'noise', 0.1) \
 .macro(1, 'Frost').mod('m1', 'at1', 0.5).macro(2, 'Wind').mod('m2', 'noise', 0.4).macro(3, 'Body').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.2, 0.06, 0.4).tags('cold', 'winter', 'thin')
X(p, press=[('noise', 0.3)], slide=[('at1', 0.5)])

p = P('Golden Hour Pad', D).an('a', 'saw', voices=3, detune=11, slop=0.4, level=0.45)
p.wt('b', 'Chords/Chord Sus2', pos=0.3, blend='spectral', voices=3, detune=8, level=0.3).off('c') \
 .f1('OB-Xa LP12', 2400, res=0.1, key=0.3).aenv(*PAD) \
 .lfo(1, 0.05, 'sine').mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Gold').mod('m1', 'f1cut', 0.5).macro(2, 'Sus').mod('m2', 'blevel', 0.5).macro(3, 'Shine').mod('m3', 'bt1', 0.5) \
 .chorus('ensemble', 0.3, 0.08, 0.5).tags('warm', 'golden', 'uplifting')
X(p, press=[('f1cut', 0.35)], slide=[('bt1', 0.5)])

p = P('Rite of Passage Pad', D).wt('a', 'Ambient/Sacred Ground', pos=0.3, blend='spectral', voices=3, detune=8, level=0.5)
p.add('b', 'Throat', 'Choir Oo', morph=0.3, partials=48, peak=8, peakamt=0.5, oct=-1, level=0.3).sub(0.15).off('c') \
 .f1('SVF LP12', 3500, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.25, shape=HEART).mod('lfo1', 'alevel', 0.1).lfo(2, 0.02, 'drift').mod('lfo2', 'at1', 0.4) \
 .macro(1, 'Ground').mod('m1', 'at1', 0.5).macro(2, 'Chant').mod('m2', 'blevel', 0.5).macro(3, 'Heartbeat').mod('m3', 'alevel', 0.3) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('ritual', 'roach', 'heartbeat', 'tribal')
X(p, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Stratosphere Pad', D).wt('a', 'Ambient/Ancient Light', pos=0.3, blend='spectral', voices=5, detune=12, level=0.45)
p.wt('b', 'Drift Spectra/Drift Halo', pos=0.3, blend='spectral', voices=3, detune=10, oct=1, level=0.25).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*SLOWPAD) \
 .lfo(1, 0.012, 'sine').lfo(2, 0.03, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.5) \
 .macro(1, 'Height').mod('m1', 'at1', 0.5).macro(2, 'Halo').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.04, 0.5).width(1.5).tags('vast', 'airy', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Soft Focus Pad', D).add('a', 'Triangle', 'Hollow', morph=0.4, partials=24, shim=0.1, level=0.55)
p.an('b', 'tri', oct=-1, level=0.25).off('c').f1('SVF LP12', 2500, res=0.05, key=0.3).aenv(*PAD) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Focus').mod('m1', 'f1cut', 0.5).macro(2, 'Hollow').mod('m2', 'at1', 0.5).macro(3, 'Body').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('soft', 'gentle', 'lofi')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Cyber Monk Pad', D).wt('a', 'Vocal/Tuvan Whistle', pos=0.3, blend='spectral', voices=3, detune=8, level=0.45)
p.an('b', 'saw', oct=-1, voices=2, detune=8, level=0.3).off('c').f1('Xpander LP4', 2200, res=0.2, key=0.3).aenv(*PAD) \
 .lfo(1, 0.05, 'sine').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Whistle').mod('m1', 'at1', 0.5).macro(2, 'Body').mod('m2', 'blevel', 0.5).macro(3, 'Filter').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.07, 0.5).crush(12, 1, 0.1).tags('overtone', 'cyber', 'hybrid')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Endless Horizon', D).an('a', 'saw', voices=5, detune=16, slop=0.5, level=0.45)
p.wt('b', 'Ambient/Stillness', pos=0.3, blend='spectral', voices=3, detune=8, oct=1, level=0.25).off('c') \
 .f1('SEM LP12', 2000, res=0.05, key=0.3).aenv(*SLOWPAD) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Horizon').mod('m1', 'f1cut', 0.5).macro(2, 'Stillness').mod('m2', 'blevel', 0.5).macro(3, 'Detune').mod('m3', 'adetune', 0.4) \
 .chorus('ensemble', 0.35, 0.05, 0.6).width(1.4).tags('vast', 'roach', 'endless', 'analog')
X(p, press=[('f1cut', 0.35)], slide=[('adetune', 0.4)])

p = P('Wooden Choir Pad', D).wt('a', 'Organic/Clay Pot', pos=0.3, voices=3, detune=8, level=0.45)
p.add('b', 'Choir Oo', 'Hollow', morph=0.3, partials=40, vowel=0.7, vowelamt=0.5, level=0.35).off('c') \
 .f1('SVF LP12', 4000, res=0.05, key=0.2).aenv(*PAD) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'bt4', 0.4) \
 .macro(1, 'Clay').mod('m1', 'at1', 0.5).macro(2, 'Choir').mod('m2', 'blevel', 0.5).macro(3, 'Vowel').mod('m3', 'bt4', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('organic', 'choir', 'earthy')
X(p, press=[('blevel', 0.3)], slide=[('bt4', 0.6)])

p = P('Nebula Strings', D).wt('a', 'Strings & Reeds/Sul Ponticello', pos=0.3, voices=5, detune=14, level=0.45)
p.wt('b', 'Textures/Aurora', pos=0.3, blend='spectral', voices=3, detune=8, level=0.3).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*SLOWPAD) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'bt1', 0.5).lfo(2, 0.05, 'drift', 'voice').mod('lfo2', 'at1', 0.3) \
 .macro(1, 'Ponticello').mod('m1', 'at1', 0.5).macro(2, 'Nebula').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.05, 0.5).tags('strings', 'space', 'corbacho')
X(p, vib=0.016, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Basalt Organ Pad', D).wt('a', 'Organs/Organ Pedal', pos=0.3, voices=2, detune=5, oct=-1, level=0.5)
p.wt('b', 'Organs/Organ Reed Swell', pos=0.3, voices=3, detune=7, level=0.3).off('c') \
 .f1('SVF LP12', 3500, res=0.05, key=0.2).aenv(*PAD) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Pedal').mod('m1', 'alevel', 0.5).macro(2, 'Swell').mod('m2', 'bt1', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('organ', 'dark', 'cathedral')
X(p, press=[('bt1', 0.35)], slide=[('f1cut', 0.4)])

p = P('Hope Pad', D).an('a', 'saw', voices=3, detune=10, slop=0.3, level=0.45)
p.add('b', 'Octaves', 'Fifths', morph=0.4, partials=48, shim=0.2, oct=1, level=0.25).off('c') \
 .f1('SEM LP12', 3000, res=0.05, key=0.3).aenv(*PAD).curve(a=-0.2) \
 .lfo(1, 0.04, 'sine').mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Light').mod('m1', 'f1cut', 0.5).macro(2, 'Fifths').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'bt2', 0.4) \
 .chorus('ensemble', 0.3, 0.07, 0.5).tags('uplifting', 'hopeful', 'warm')
X(p, press=[('f1cut', 0.35)], slide=[('bt1', 0.5)])
