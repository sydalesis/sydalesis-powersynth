# Driftfield 4.0 bank, part B: Roach / Corbacho territory in pure tunings, Scala examples, and tuned
# presets for the existing categories.
JI = 'Just Intonation'

HARRISON_PENTA = """! harrison_penta.scl
!
Lou Harrison style 5-limit pentatonic
 5
!
 9/8
 5/4
 3/2
 5/3
 2/1
"""
RILEY_ALBION = """! riley_albion.scl
!
Terry Riley, The Harp of New Albion (5-limit, after the published ratios)
 12
!
 16/15
 9/8
 6/5
 5/4
 4/3
 64/45
 3/2
 8/5
 5/3
 16/9
 15/8
 2/1
"""
SEVEN_LIMIT_PENTA = """! septimal_penta.scl
!
Septimal pentatonic: 1/1 7/6 4/3 3/2 7/4
 5
!
 7/6
 4/3
 3/2
 7/4
 2/1
"""
# white keys only: C D E F G A B carry the 7 degrees, black keys unmapped (they play the neighbour)
WHITE_KEYS_KBM = """! white_keys.kbm
! 7-note scale on the white keys, middle C = degree 0, A4 = 440 Hz reference
12
0
127
60
69
440.0
7
0
x
1
x
2
3
x
4
x
5
x
6
"""
DIATONIC_JI = """! diatonic_ji.scl
!
Just diatonic major (Ptolemy's intense diatonic)
 7
!
 9/8
 5/4
 4/3
 3/2
 5/3
 15/8
 2/1
"""

# ------------------------------------------------------------------------------- slow roach / corbacho beds
def jiPad(name, scale, root, a, b, c=None, cut=4800, cat=JI, glide=1.5, tags=(), lfo=(0.012, 0.031), env=None, chorus=('ensemble', 0.22, 0.05, 0.45), press=(), slide=()):
    p = P(name, cat).tune(scale, root=root, glide=glide)
    a(p); b(p)
    if c: c(p)
    else: p.off('c')
    p.f1('SVF LP12', cut, res=0.05, key=0.15).aenv(*(env or LONG)) \
     .lfo(1, lfo[0], 'drift').lfo(2, lfo[1], 'sine').mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.35).mod('lfo1', 'f1cut', 0.15) \
     .macro(1, 'Colour').mod('m1', 'at1', 0.5).macro(2, 'Layer').mod('m2', 'blevel', 0.45).macro(3, 'Motion').mod('m3', 'lfo1rate', 0.5).macro(4, 'Air').mod('m4', 'f1cut', 0.45) \
     .chorus(*chorus).tags(*tags)
    jiX(p, press=list(press) or [('blevel', 0.3)], slide=list(slide) or [('at1', 0.4)])
    return p

jiPad('Structures in Just Light', 'JI 5-limit Major', 'C',
      lambda p: p.wt('a', 'Ambient/Structures from Silence', pos=0.3, blend='spectral', voices=4, detune=7, level=0.45),
      lambda p: p.add('b', 'Strings', 'Choir Oo', morph=0.3, partials=40, shim=0.18, srate=0.05, level=0.35),
      tags=('roach', 'pad', 'classic', 'silence'), env=VLONG)
jiPad('Magnificent Pure Void', 'JI 7-limit Septimal', 'A',
      lambda p: p.wt('a', 'Ambient/Magnificent Void', pos=0.25, blend='spectral', voices=3, detune=6, oct=-1, level=0.5),
      lambda p: p.an('b', 'sine', level=0.3, slop=0.1),
      lambda p: p.add('c', 'Primes', 'Octaves', morph=0.5, partials=48, shim=0.2, srate=0.04, oct=1, level=0.18),
      cut=3600, tags=('roach', 'void', 'deep', 'drone'), env=VLONG, lfo=(0.007, 0.02))
jiPad('Desert Septimal Heat', 'JI 7-limit Septimal', 'E',
      lambda p: p.wt('a', 'Ambient/Mirage Heat', pos=0.4, blend='spectral', voices=4, detune=10, level=0.45),
      lambda p: p.an('b', 'saw', voices=3, detune=12, slop=0.5, drift=0.5, oct=-1, level=0.25),
      cut=3000, tags=('roach', 'desert', 'heat', 'analog'), chorus=('solina', 0.25, 0.08, 0.5))
jiPad('Corbacho Pure Horizon', 'JI 5-limit Major', 'F',
      lambda p: p.wt('a', 'Ambient/Blue Hour', pos=0.35, blend='spectral', voices=4, detune=6, level=0.45),
      lambda p: p.add('b', 'Organ', 'Strings', morph=0.4, partials=48, shim=0.15, srate=0.04, level=0.35),
      tags=('corbacho', 'horizon', 'calm', 'pad'), env=VLONG)
jiPad('Ice Cathedral Harmonics', 'Harmonic 16-32', 'D',
      lambda p: p.wt('a', 'Ambient/Ice Cathedral', pos=0.3, blend='spectral', voices=4, detune=6, level=0.45),
      lambda p: p.add('b', 'Glass', 'Primes', morph=0.4, partials=64, shim=0.3, srate=0.08, oct=1, level=0.25),
      cut=7000, tags=('cathedral', 'ice', 'harmonic', 'corbacho'))
jiPad('Altar of Fifths', 'Pythagorean', 'G',
      lambda p: p.wt('a', 'Ambient/Altar of Light', pos=0.3, blend='spectral', voices=3, detune=6, level=0.45),
      lambda p: p.wt('b', 'Chords/Chord Fifths', pos=0.4, voices=2, detune=4, oct=-1, level=0.35),
      tags=('altar', 'fifths', 'sacred', 'roach'))
jiPad('Sacred Ground Pure', 'JI 5-limit Minor', 'D',
      lambda p: p.wt('a', 'Ambient/Sacred Ground', pos=0.3, blend='spectral', voices=4, detune=7, level=0.45),
      lambda p: p.gr('b', 'keyed', sample='Voices/Monk Hum Low', pos=0.3, size=0.9, density=9, spray=0.25, level=0.35),
      cut=3800, tags=('sacred', 'monks', 'minor', 'roach'))
jiPad('Night Sky in Seven', 'JI 7-limit Septimal', 'B',
      lambda p: p.wt('a', 'Ambient/Night Sky Shimmer', pos=0.4, blend='spectral', voices=4, detune=8, level=0.45),
      lambda p: p.add('b', 'Bell', 'Glass', morph=0.3, partials=32, shim=0.35, srate=0.12, oct=1, level=0.2),
      cut=6500, tags=('night', 'stars', 'shimmer', 'corbacho'))
jiPad('Dreamtime Just Reed', 'JI 5-limit Major', 'A',
      lambda p: p.wt('a', 'Ambient/Dreamtime Reed', pos=0.3, blend='spectral', voices=3, detune=6, level=0.45),
      lambda p: p.gr('b', 'keyed', sample='Drones/Didgeridoo', pos=0.2, size=0.8, density=10, spray=0.2, oct=-1, level=0.35),
      cut=3500, tags=('dreamtime', 'didgeridoo', 'roach', 'tribal'))
jiPad('Stillness Overtones', 'Harmonic 16-32', 'C',
      lambda p: p.wt('a', 'Ambient/Stillness', pos=0.3, blend='spectral', voices=3, detune=5, level=0.45),
      lambda p: p.wt('b', 'Vowel Journeys/Tenor Full Circle', pos=0.3, voices=2, detune=4, level=0.3),
      tags=('stillness', 'overtone', 'vocal', 'meditative'), env=VLONG, lfo=(0.006, 0.015))
jiPad('Canyon Pythagoras', 'Pythagorean', 'E',
      lambda p: p.wt('a', 'Ambient/Canyon Glow', pos=0.35, blend='spectral', voices=4, detune=8, level=0.45),
      lambda p: p.gr('b', 'fixed', sample='Nature/Canyon Wind', pos=0.3, size=1.0, density=8, spray=0.4, level=0.35),
      tags=('canyon', 'wind', 'roach', 'soundscape'))
jiPad('Ghost Strings Septimal', 'JI 7-limit Septimal', 'G',
      lambda p: p.wt('a', 'Ambient/Ghost Strings', pos=0.4, blend='spectral', voices=4, detune=7, level=0.45),
      lambda p: p.ph('b', 'Sympathetic', 'Bow', decay=14, damp=0.06, hard=0.35, oct=-1, level=0.3),
      tags=('strings', 'ghost', 'sympathetic', 'corbacho'))

# ------------------------------------------------------------------------------- drone systems with root moves
p = P('Root Walker Drone', JI).tune('JI 5-limit Major', glide=4.0)
p.add('a', 'Organ', 'Octaves', morph=0.4, partials=64, shim=0.12, srate=0.03, level=0.5) \
 .an('b', 'saw', oct=-1, voices=2, detune=6, slop=0.3, level=0.25).an('c', 'sine', oct=-2, level=0.3, slop=0.1) \
 .f1('Ladder LP24', 2200, res=0.1, key=0.3, drive=0.1).aenv(*VLONG) \
 .lfo(1, 0.008, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'f1cut', 0.2) \
 .macro(1, 'Organ').mod('m1', 'at1', 0.5).macro(2, 'Saw').mod('m2', 'blevel', 0.45).macro(3, 'Sub').mod('m3', 'clevel', 0.5).macro(4, 'Open').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.15, 0.04, 0.35).tags('drone', 'root-change', 'automate-root', 'roach')
jiX(p, vib=0, press=[('f1cut', 0.3)], slide=[('at1', 0.4)])

p = P('Tanpura Root Shift Field', JI).tune('JI 5-limit Major', glide=3.0)
p.ph('a', 'Tanpura', 'Bow', decay=14, damp=0.05, hard=0.45, buzz=0.8, level=0.5) \
 .gr('b', 'keyed', sample='Strings/Tanpura', pos=0.3, size=0.9, density=10, spray=0.3, level=0.3) \
 .an('c', 'sine', oct=-1, level=0.2, slop=0.1).f1('SVF LP12', 6000, res=0.04, key=0.2).aenv(*LONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'at4', 0.3) \
 .macro(1, 'Jawari').mod('m1', 'at4', -0.5).macro(2, 'Grains').mod('m2', 'blevel', 0.45).macro(3, 'Low').mod('m3', 'clevel', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .nochorus().width(1.2).tags('tanpura', 'root-change', 'automate-root', 'physical')
jiX(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Half-Just Twilight', JI).tune('JI 7-limit Septimal', amount=0.5, glide=1.0)
p.wt('a', 'Ambient/Blue Hour', pos=0.4, blend='spectral', voices=4, detune=8, level=0.45) \
 .an('b', 'saw', voices=3, detune=12, slop=0.4, level=0.25).off('c').f1('OB-Xa LP12', 2400, res=0.1, key=0.3).aenv(*LONG) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Colour').mod('m1', 'at1', 0.5).macro(2, 'Saw').mod('m2', 'blevel', 0.45).macro(3, 'Detune').mod('m3', 'detune', 0.3).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('solina', 0.25, 0.07, 0.5).tags('pad', 'half-tempered', 'twilight', 'tuning-amount')
jiX(p, press=[('f1cut', 0.3)], slide=[('at1', 0.4)])

# ------------------------------------------------------------------------------- Scala examples
p = P('Harrison Pentatonic Bells', JI).scala(HARRISON_PENTA, root='D')
p.ph('a', 'Bar', 'Strike', decay=6, damp=0.12, hard=0.7, level=0.5) \
 .ph('b', 'Bell', 'Strike', decay=8, damp=0.08, hard=0.55, oct=1, level=0.25) \
 .off('c').f1('SVF LP12', 9000, res=0.03, key=0.1).aenv(0.002, 6, 0.0, 6).curve(a=1) \
 .macro(1, 'Mallet').mod('m1', 'at2', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.45).macro(3, 'Ring').mod('m3', 'at3', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('harrison', 'pentatonic', 'gamelan', 'just-intonation')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.4)])

p = P('New Albion Harp', JI).scala(RILEY_ALBION, root='C#')
p.ph('a', 'String', 'Strike', decay=6, damp=0.15, hard=0.75, pos=0.1, level=0.5) \
 .ph('b', 'Sympathetic', 'Osc Input', decay=12, damp=0.05, hard=0.3, level=0.25) \
 .off('c').f1('SVF LP12', 7500, res=0.04, key=0.25).aenv(0.002, 6, 0.0, 6).curve(a=1) \
 .macro(1, 'Hammer').mod('m1', 'at2', 0.5).macro(2, 'Sympathy').mod('m2', 'blevel', 0.45).macro(3, 'Position').mod('m3', 'at1', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.35).tags('riley', 'piano', 'just-intonation', 'physical')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.5)])

p = P('Septimal Pentatonic Drift', JI).scala(SEVEN_LIMIT_PENTA, root='A', glide=1.0)
p.add('a', 'Choir Oo', 'Throat', morph=0.4, partials=40, vowel=0.5, vowelamt=0.5, shim=0.2, srate=0.06, level=0.5) \
 .an('b', 'tri', oct=-1, level=0.3, slop=0.3).off('c').f1('SVF LP12', 4200, res=0.06, key=0.2).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at4', 0.4) \
 .macro(1, 'Vowel').mod('m1', 'at4', 0.5).macro(2, 'Low').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'at3', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.4).tags('septimal', 'pentatonic', 'choir', 'scala')
jiX(p, vib=0.01, press=[('at4', 0.35)], slide=[('at1', 0.4)])

p = P('White Keys Just Diatonic', JI).scala(DIATONIC_JI, WHITE_KEYS_KBM)
p.add('a', 'Strings', 'Organ', morph=0.35, partials=48, shim=0.15, srate=0.06, level=0.5) \
 .an('b', 'saw', voices=3, detune=10, slop=0.4, level=0.2).off('c').f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(0.8, 3, 0.9, 4) \
 .macro(1, 'Organ').mod('m1', 'at1', 0.5).macro(2, 'Saw').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'at3', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.22, 0.05, 0.45).tags('diatonic', 'white-keys', 'kbm', 'scala')
jiX(p, press=[('blevel', 0.3)], slide=[('at1', 0.4)])

# ------------------------------------------------------------------------------- tuned presets in other categories
p = P('Just Fifths Drone', 'Drones').tune('Pythagorean', glide=2.0)
p.an('a', 'saw', voices=3, detune=8, slop=0.4, drift=0.5, level=0.4).an('b', 'saw', semi=7, voices=3, detune=8, slop=0.4, drift=0.5, level=0.3) \
 .an('c', 'sine', oct=-1, level=0.3, slop=0.1).f1('Ladder LP24', 1200, res=0.15, key=0.3, drive=0.2).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'f1cut', 0.35) \
 .macro(1, 'Open').mod('m1', 'f1cut', 0.5).macro(2, 'Fifth').mod('m2', 'blevel', 0.45).macro(3, 'Sub').mod('m3', 'clevel', 0.5).macro(4, 'Drift').mod('m4', 'detune', 0.3) \
 .chorus('solina', 0.2, 0.06, 0.45).tags('drone', 'fifths', 'analog', 'roach', 'microtonal')
jiX(p, vib=0, press=[('f1cut', 0.35)], slide=[('balance', 0.3)])

p = P('Septimal Seventh Drone', 'Drones').tune('JI 7-limit Septimal', glide=2.0)
p.add('a', 'Organ', 'Hollow', morph=0.4, partials=48, shim=0.1, srate=0.04, level=0.45) \
 .add('b', 'Organ', 'Hollow', morph=0.4, partials=48, shim=0.1, srate=0.04, semi=10, level=0.3) \
 .an('c', 'sine', oct=-1, level=0.3, slop=0.1).f1('SVF LP12', 3800, res=0.05, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.008, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Colour').mod('m1', 'at1', 0.5).macro(2, 'Seventh').mod('m2', 'blevel', 0.45).macro(3, 'Sub').mod('m3', 'clevel', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.15, 0.04, 0.35).tags('drone', 'septimal', 'harmonic-seventh', 'microtonal')
jiX(p, vib=0, press=[('at1', 0.3)], slide=[('blevel', 0.3)])

p = P('Just Choir Aah', 'Choirs & Voices').tune('JI 5-limit Major', glide=0.5)
p.add('a', 'Choir Ah', 'Choir Oo', morph=0.2, partials=48, vowel=0.3, vowelamt=0.6, shim=0.2, srate=0.08, level=0.5) \
 .gr('b', 'keyed', sample='Voices/Choir Ah', pos=0.3, size=0.7, density=12, spray=0.3, level=0.3) \
 .off('c').f1('SVF LP12', 5500, res=0.05, key=0.2).aenv(1.5, 3, 1, 5) \
 .macro(1, 'Vowel').mod('m1', 'at4', 0.5).macro(2, 'Cloud').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'at3', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.45).tags('choir', 'just-intonation', 'microtonal', 'vocal')
jiX(p, vib=0.012, press=[('at4', 0.35)], slide=[('bt1', 0.4)])

p = P('Overtone Singer JI', 'Choirs & Voices').tune('Harmonic 16-32', glide=0.4)
p.wt('a', 'Vowel Journeys/Bass U-O-A', pos=0.3, voices=1, level=0.5) \
 .add('b', 'Throat', 'Primes', morph=0.5, partials=64, peak=12, peakamt=0.7, width=0.15, level=0.35) \
 .off('c').f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(0.6, 2, 1, 3).voice('legato', 0.1) \
 .lfo(1, 0.05, 'sine').mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Vowel').mod('m1', 'at1', 0.5).macro(2, 'Whistle').mod('m2', 'bt2', 0.5).macro(3, 'Throat').mod('m3', 'blevel', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .nochorus().tags('overtone', 'throat-singing', 'harmonic', 'microtonal')
jiX(p, vib=0.01, press=[('bt2', 0.5)], slide=[('at1', 0.5)])

p = P('Gamelan Bell Tree', 'Bells & Resonators').tune('Pelog', root='E')
p.ph('a', 'Bell', 'Strike', decay=9, damp=0.06, hard=0.65, level=0.5) \
 .gr('b', 'fixed', sample='Metal/Bell Tree', pos=0.3, size=0.4, density=8, spray=0.5, level=0.2) \
 .off('c').f1('SVF LP12', 10000, res=0.03, key=0.1).aenv(0.002, 9, 0.0, 9).curve(a=1) \
 .macro(1, 'Mallet').mod('m1', 'at2', 0.5).macro(2, 'Tree').mod('m2', 'blevel', 0.45).macro(3, 'Ring').mod('m3', 'at3', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('gamelan', 'bells', 'microtonal')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.4)])

p = P('Just Kalimba', 'Sequence & Plucks').tune('JI 5-limit Major', root='G')
p.ph('a', 'Bar', 'Strike', decay=2.5, damp=0.25, hard=0.6, stretch=0.1, level=0.55) \
 .off('b', 'c').f1('SVF LP12', 8000, res=0.04, key=0.2).aenv(0.002, 2.5, 0.0, 2).curve(a=1) \
 .arp('up', 2, '1/16', 0.5).macro(1, 'Tine').mod('m1', 'at2', 0.5).macro(2, 'Position').mod('m2', 'at1', 0.5).macro(3, 'Ring').mod('m3', 'at3', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.08, 0.3).tags('kalimba', 'arp', 'just-intonation', 'microtonal')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.4)])

p = P('Meantone Harpsichord', 'Keys & Organs').tune('Meantone 1/4-comma')
p.ph('a', 'String', 'Strike', decay=3, damp=0.35, hard=0.9, pos=0.07, level=0.5) \
 .ph('b', 'String', 'Strike', decay=3, damp=0.35, hard=0.9, pos=0.07, oct=1, level=0.25) \
 .off('c').f1('SVF HP12', 120, res=0.02, key=0.0).f2('SVF LP12', 9000).aenv(0.001, 3, 0.0, 0.8).curve(a=1) \
 .macro(1, 'Quill').mod('m1', 'at2', 0.5).macro(2, '4-foot').mod('m2', 'blevel', 0.45).macro(3, 'Lute Stop').mod('m3', 'at4', 0.5).macro(4, 'Tone').mod('m4', 'f2cut', 0.5) \
 .chorus('chorus', 0.08, 0.05, 0.25).tags('harpsichord', 'meantone', 'baroque', 'microtonal')
jiX(p, vib=0, press=[('at2', 0.3)], slide=[('at1', 0.4)])

p = P('Well-Tuned Piano Hall', 'Keys & Organs').tune('Well-Tuned Piano', root='D#')
p.ph('a', 'String', 'Strike', decay=8, damp=0.12, hard=0.7, pos=0.12, level=0.5) \
 .ph('b', 'Sympathetic', 'Osc Input', decay=14, damp=0.05, hard=0.3, level=0.25) \
 .off('c').f1('SVF LP12', 6500, res=0.03, key=0.3).aenv(0.002, 8, 0.0, 8).curve(a=1) \
 .macro(1, 'Hammer').mod('m1', 'at2', 0.5).macro(2, 'Sympathy').mod('m2', 'blevel', 0.45).macro(3, 'Damping').mod('m3', 'at4', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('piano', 'young', 'just-intonation', 'microtonal')
jiX(p, vib=0, press=[('at2', 0.3)], slide=[('at1', 0.4)])

p = P('Bhairav Raga Lead', 'Leads').tune('Raga Bhairav', root='C', snap=True)
p.an('a', 'saw', voices=1, slop=0.3, level=0.5).an('b', 'pulse', pw=0.35, fine=6, level=0.3).off('c') \
 .f1('Ladder LP24', 2000, res=0.25, key=0.5, env=0.3, drive=0.2).fenv(0.01, 0.8, 0.4, 1).aenv(0.02, 1, 0.9, 0.6).voice('legato', 0.12) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Pulse').mod('m2', 'blevel', 0.45).macro(3, 'Res').mod('m3', 'f1res', 0.4).macro(4, 'Drive').mod('m4', 'f1drive', 0.4) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('lead', 'raga', 'snap', 'microtonal')
jiX(p, vib=0.03, press=[('f1cut', 0.4)], slide=[('f1res', 0.3)])

p = P('Just Sub Bass', 'Basses').tune('JI 5-limit Minor', root='E')
p.an('a', 'saw', level=0.45, slop=0.2).an('b', 'sine', oct=-1, level=0.45, slop=0.05).off('c') \
 .f1('Ladder LP24', 600, res=0.2, key=0.4, env=0.35, drive=0.3).fenv(0.005, 0.4, 0.2, 0.3).aenv(0.005, 1, 0.9, 0.3).voice('mono', 0.03) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Sub').mod('m2', 'blevel', 0.45).macro(3, 'Env').mod('m3', 'f1cut', 0.3).macro(4, 'Drive').mod('m4', 'f1drive', 0.4) \
 .nochorus().tags('bass', 'mono', 'just-intonation', 'microtonal')
jiX(p, vib=0, press=[('f1cut', 0.35)], slide=[('f1res', 0.3)])
