# Driftfield 4.0 bank, part A: JUST INTONATION and other tunings (phase A of v4).
# exec'd by gen_presets3.py after the 3.1 banks; Patch / P, X(), field(), LONG / VLONG, TIDE are in scope.
# Every preset sets its tuning with .tune(scale, root); the root is the drone key: play it and its fifth
# for the pure intervals, or automate Tuning Root for drone-root changes (held notes glide by tnGlide).
# MPE layer as in 3.1 (X); Studio One note expression works on all of them without extra wiring.
JI = 'Just Intonation'

def jiX(p, **kw):
    """3.1 MPE layer plus a wider note-expression vibrato for slow JI material"""
    p.nevib(0.35, 4.8)
    return X(p, **kw)

# ------------------------------------------------------------------------------------------- harmonic drones
p = P('Harmonic Throne', JI).tune('JI 7-limit Septimal', glide=1.5)
p.add('a', 'Octaves', 'Primes', morph=0.35, partials=64, tilt=-0.25, shim=0.12, srate=0.04, pdet=2, level=0.55) \
 .add('b', 'Organ', 'Fifths', morph=0.5, partials=48, tilt=-0.1, shim=0.1, srate=0.03, oct=-1, level=0.4) \
 .off('c').sub(0.12).f1('SVF LP12', 7000, res=0.04, key=0.1).aenv(*VLONG).curve(a=-0.4) \
 .lfo(1, 0.009, 'drift').lfo(2, 0.021, 'sine').mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt3', 0.3) \
 .macro(1, 'Primes').mod('m1', 'at1', 0.5).macro(2, 'Low Organ').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'at3', 0.4).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
 .chorus('ensemble', 0.15, 0.03, 0.35).tags('drone', 'harmonic', 'young', 'corbacho')
jiX(p, vib=0, press=[('at2', 0.4)], slide=[('bt1', 0.4)])

p = P('Overtone Cathedral', JI).tune('Harmonic 16-32', glide=2.0)
p.add('a', 'Primes', 'Octaves', morph=0.5, partials=96, tilt=-0.35, shim=0.18, srate=0.05, pdet=1, level=0.5) \
 .wt('b', 'Organs/Organ Cathedral', pos=0.4, voices=3, detune=4, level=0.35) \
 .an('c', 'sine', oct=-1, level=0.3, slop=0.1).f1('SVF LP12', 9000, res=0.02, key=0.0).aenv(10, 4, 1, 24).curve(a=-0.6) \
 .lfo(1, 0.006, 'drift').lfo(2, 0.017, 'drift', 'voice').mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.4) \
 .macro(1, 'Harmonics').mod('m1', 'at1', 0.5).macro(2, 'Organ').mod('m2', 'blevel', 0.45).macro(3, 'Fundamental').mod('m3', 'clevel', 0.5).macro(4, 'Brightness').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.12, 0.03, 0.3).tags('drone', 'harmonic', 'sacred', 'corbacho')
jiX(p, vib=0, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Dream House Sine Wall', JI).tune('Well-Tuned Piano', root='D#', glide=3.0)
p.an('a', 'sine', level=0.45, slop=0.0, drift=0.0).an('b', 'sine', oct=1, semi=7, level=0.25, slop=0.0, drift=0.0) \
 .an('c', 'sine', oct=-1, level=0.35, slop=0.0, drift=0.0).f1('Off', 20000, drive=0.0).aenv(12, 2, 1, 30).curve(a=-0.7) \
 .lfo(1, 0.004, 'drift').mod('lfo1', 'bpan', 0.4).mod('lfo1', 'apan', -0.3) \
 .macro(1, 'Upper').mod('m1', 'blevel', 0.5).macro(2, 'Lower').mod('m2', 'clevel', 0.5).macro(3, 'Beat').mod('m3', 'bdetune', 0.3).macro(4, 'Space').mod('m4', 'pan', 0.4) \
 .nochorus().width(1.5).tags('drone', 'sine', 'young', 'minimal', 'beating')
jiX(p, vib=0, press=[('blevel', 0.3)], slide=[('bpan', 0.5)])

p = P('Septimal Breath Choir', JI).tune('JI 7-limit Septimal', glide=1.0)
p.add('a', 'Choir Oo', 'Choir Ah', morph=0.3, partials=48, vowel=0.4, vowelamt=0.6, shim=0.2, srate=0.08, level=0.5) \
 .gr('b', 'keyed', sample='Voices/Choir Cloud', pos=0.3, scan=0.02, size=0.8, density=12, spray=0.3, level=0.35) \
 .off('c').noise(0.04, 0.7).f1('Xpander LP2', 3600, res=0.12, key=0.25).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').lfo(2, 0.06, 'sine', 'voice').mod('lfo1', 'at4', 0.4).mod('lfo2', 'bt1', 0.2) \
 .macro(1, 'Vowel').mod('m1', 'at4', 0.6).macro(2, 'Cloud').mod('m2', 'blevel', 0.45).macro(3, 'Breath').mod('m3', 'noise', 0.15).macro(4, 'Open').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.22, 0.05, 0.45).tags('choir', 'vocal', 'roach', 'sacred')
jiX(p, vib=0.01, press=[('at4', 0.35)], slide=[('bt1', 0.4)])

p = P('Pure Fifth Monolith', JI).tune('Pythagorean', glide=2.5)
p.wt('a', 'Drone/Monolith', pos=0.3, blend='spectral', voices=3, detune=5, level=0.5) \
 .wt('b', 'Drone/Monolith', pos=0.6, blend='spectral', semi=7, voices=2, detune=4, level=0.35) \
 .an('c', 'tri', oct=-1, level=0.3, slop=0.3).f1('Ladder LP24', 1400, res=0.1, key=0.3, drive=0.15).aenv(*VLONG) \
 .lfo(1, 0.007, 'drift').lfo(2, 0.019, 'sine').mod('lfo1', 'at1', 0.4).mod('lfo2', 'bt1', 0.4).mod('lfo1', 'f1cut', 0.25) \
 .macro(1, 'Stone').mod('m1', 'at1', 0.5).macro(2, 'Fifth').mod('m2', 'blevel', 0.45).macro(3, 'Depth').mod('m3', 'clevel', 0.5).macro(4, 'Open').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.03, 0.35).tags('drone', 'dark', 'roach', 'fifths')
jiX(p, vib=0, press=[('f1cut', 0.3)], slide=[('at1', 0.4)])

p = P('Prime Number Glass', JI).tune('Harmonic 16-32')
p.add('a', 'Primes', 'Glass', morph=0.4, partials=80, tilt=-0.4, stretch=0.0, shim=0.3, srate=0.12, pdet=2, level=0.45) \
 .ph('b', 'Glass Bowl', 'Bow', decay=12, damp=0.1, hard=0.3, oct=1, level=0.2) \
 .off('c').f1('SVF LP12', 11000, res=0.03, key=0.0).aenv(5, 4, 0.9, 14) \
 .lfo(1, 0.013, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'blevel', 0.2) \
 .macro(1, 'Glass').mod('m1', 'at1', 0.5).macro(2, 'Bowl').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'at3', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).width(1.3).tags('glass', 'harmonic', 'crystalline', 'corbacho')
jiX(p, vib=0, press=[('bt2', 0.4)], slide=[('at1', 0.4)])

p = P('Undertone Abyss', JI).tune('JI 5-limit Minor', glide=2.0)
p.wt('a', 'Drone/Abyssal', pos=0.2, blend='spectral', voices=4, detune=7, oct=-1, level=0.5) \
 .add('b', 'Hollow', 'Throat', morph=0.5, partials=32, tilt=0.2, shim=0.1, srate=0.03, oct=-1, level=0.35) \
 .an('c', 'sine', oct=-2, level=0.35, slop=0.2).f1('Diode LP24', 700, res=0.2, key=0.35, drive=0.2).aenv(*VLONG) \
 .lfo(1, 0.005, 'drift').lfo(2, 0.013, 'sine').mod('lfo1', 'f1cut', 0.35).mod('lfo2', 'at1', 0.4) \
 .macro(1, 'Abyss').mod('m1', 'at1', 0.5).macro(2, 'Throat').mod('m2', 'blevel', 0.45).macro(3, 'Sub').mod('m3', 'clevel', 0.5).macro(4, 'Light').mod('m4', 'f1cut', 0.55) \
 .chorus('chorus', 0.1, 0.02, 0.3).tags('drone', 'dark', 'deep', 'roach')
jiX(p, vib=0, press=[('f1cut', 0.35)], slide=[('bt1', 0.4)])

p = P('Just Major Dawn', JI).tune('JI 5-limit Major', glide=1.2)
p.wt('a', 'Ambient/Ancient Light', pos=0.35, blend='spectral', voices=4, detune=8, level=0.45) \
 .add('b', 'Strings', 'Organ', morph=0.3, partials=40, shim=0.2, srate=0.07, level=0.35) \
 .off('c').sub(0.1).f1('SVF LP12', 5200, res=0.06, key=0.15).aenv(*LONG).curve(a=-0.3) \
 .lfo(1, 0.016, shape=TIDE).lfo(2, 0.045, 'drift', 'voice').mod('lfo1', 'at1', 0.35).mod('lfo2', 'bt1', 0.3) \
 .macro(1, 'Light').mod('m1', 'at1', 0.5).macro(2, 'Strings').mod('m2', 'blevel', 0.45).macro(3, 'Depth').mod('m3', 'sub', 0.3).macro(4, 'Air').mod('m4', 'f1cut', 0.4) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('pad', 'warm', 'corbacho', 'major')
jiX(p, press=[('blevel', 0.3)], slide=[('at1', 0.4)])

p = P('Meantone Organ Loft', JI).tune('Meantone 1/4-comma')
p.wt('a', 'Organs/Organ Flutes', pos=0.4, voices=2, detune=3, level=0.5) \
 .wt('b', 'Organs/Organ Mixture', pos=0.3, oct=1, level=0.25) \
 .an('c', 'sine', oct=-1, level=0.25, slop=0.05).f1('SVF LP12', 8000, res=0.03, key=0.1).aenv(0.08, 1, 1, 0.6).curve(a=0.3) \
 .lfo(1, 5.5, 'sine').mod('lfo1', 'alevel', 0.04) \
 .macro(1, 'Mixture').mod('m1', 'blevel', 0.5).macro(2, 'Pedal').mod('m2', 'clevel', 0.5).macro(3, 'Tremulant').mod('m3', 'alevel', 0.1).macro(4, 'Swell').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.4, 0.2).tags('organ', 'renaissance', 'church', 'keys')
jiX(p, vib=0.0, press=[('blevel', 0.3)], slide=[('f1cut', 0.3)])

p = P('Werckmeister Chamber', JI).tune('Werckmeister III')
p.ph('a', 'String', 'Strike', decay=4, damp=0.3, hard=0.7, pos=0.13, level=0.55) \
 .ph('b', 'String', 'Strike', decay=4, damp=0.3, hard=0.7, pos=0.13, fine=3, level=0.35, pan=0.3) \
 .off('c').f1('SVF LP12', 6500, res=0.05, key=0.3).aenv(0.002, 4, 0.0, 3).curve(a=1) \
 .macro(1, 'Pluck').mod('m1', 'at2', 0.5).macro(2, 'Position').mod('m2', 'at1', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('harpsichord', 'baroque', 'plucked', 'physical')
jiX(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Kirnberger Glass Harmonica', JI).tune('Kirnberger III')
p.ph('a', 'Glass Bowl', 'Bow', decay=8, damp=0.12, hard=0.35, level=0.5) \
 .add('b', 'Glass', 'Bell', morph=0.3, partials=24, shim=0.2, srate=0.2, level=0.3) \
 .off('c').f1('SVF LP12', 9000, res=0.03, key=0.1).aenv(0.6, 3, 0.85, 4) \
 .lfo(1, 4.2, 'sine', 'voice', fade=1.0).mod('lfo1', 'pitch', 0.006) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).macro(2, 'Glass').mod('m2', 'blevel', 0.45).macro(3, 'Wet Finger').mod('m3', 'at4', -0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.35).tags('glass', 'harmonica', 'historical', 'physical')
jiX(p, vib=0.01, press=[('at2', 0.45)], slide=[('at1', 0.4)])

# ------------------------------------------------------------------------------------------- ragas
for name, scale, root, extra in [
        ('Yaman Evening Tanpura', 'Raga Yaman', 'C', ('evening',)),
        ('Bhairav Dawn Tanpura', 'Raga Bhairav', 'C#', ('dawn',)),
        ('Todi Morning Strings', 'Raga Todi', 'D', ('morning',)),
        ('Darbari Night Drone', 'Raga Darbari', 'A', ('night',)),
        ('Marwa Sunset Swarm', 'Raga Marwa', 'D', ('sunset',)),
        ('Bhupali Open Sky', 'Raga Bhupali', 'E', ('pentatonic',))]:
    p = P(name, JI).tune(scale, root=root, snap=True, glide=1.5)
    p.ph('a', 'Tanpura', 'Strike', decay=14, damp=0.05, hard=0.78, buzz=0.78, level=0.5) \
     .ph('b', 'Sympathetic', 'Bow', decay=16, damp=0.05, hard=0.35, oct=-1, level=0.3, pan=0.3) \
     .an('c', 'saw', oct=-1, level=0.12, slop=0.3, drift=0.4).f1('SVF LP12', 6000, res=0.05, key=0.2).aenv(0.01, 6, 0.85, 10) \
     .lfo(1, 0.011, 'drift').mod('lfo1', 'at4', 0.3).mod('lfo1', 'blevel', 0.2) \
     .macro(1, 'Jawari').mod('m1', 'at4', -0.5).macro(2, 'Sympathy').mod('m2', 'blevel', 0.5).macro(3, 'Reed').mod('m3', 'clevel', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
     .nochorus().width(1.2).tags('tanpura', 'raga', 'physical', *extra)
    jiX(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Bhairavi Bowed Sarangi', JI).tune('Raga Bhairavi', snap=True)
p.ph('a', 'Waveguide', 'Bow', decay=9, damp=0.2, hard=0.55, pos=0.11, level=0.55) \
 .ph('b', 'Sympathetic', 'Osc Input', decay=12, damp=0.06, hard=0.3, level=0.3) \
 .off('c').f1('Formant', 1600, res=0.2, key=0.3, shape=0.4).aenv(0.25, 2, 0.95, 2.5).voice('legato', 0.08) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).macro(2, 'Sympathy').mod('m2', 'blevel', 0.45).macro(3, 'Vowel').mod('m3', 'f1shape', 0.5).macro(4, 'Position').mod('m4', 'at1', 0.4) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('sarangi', 'raga', 'bowed', 'expressive', 'solo')
jiX(p, vib=0.028, press=[('at2', 0.45)], slide=[('f1shape', 0.5)])

p = P('Shruti Box Breath', JI).tune('JI 5-limit Major', glide=0.8)
p.gr('a', 'keyed', sample='Drones/Shruti Box', pos=0.3, scan=0.01, size=0.9, density=10, spray=0.2, level=0.5) \
 .wt('b', 'Strings & Reeds/Reed Breath', pos=0.4, voices=2, detune=5, level=0.3) \
 .off('c').f1('SVF LP12', 4500, res=0.05, key=0.2).aenv(1.2, 3, 1, 4) \
 .lfo(1, 0.25, 'sine').mod('lfo1', 'alevel', 0.08).mod('lfo1', 'blevel', 0.06) \
 .macro(1, 'Bellows').mod('m1', 'alevel', 0.2).macro(2, 'Reed').mod('m2', 'blevel', 0.45).macro(3, 'Grain').mod('m3', 'at3', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('shruti', 'indian', 'reed', 'granular')
jiX(p, vib=0, press=[('alevel', 0.25)], slide=[('at1', 0.3)])

p = P('22 Shruti Glissando Field', JI).tune('22 Shruti', glide=0.4)
p.add('a', 'Strings', 'Throat', morph=0.4, partials=56, shim=0.25, srate=0.06, level=0.5) \
 .gr('b', 'keyed', sample='Strings/Sympathetic Drone', pos=0.25, size=0.7, density=12, spray=0.35, level=0.35) \
 .off('c').f1('SVF LP12', 5000, res=0.05, key=0.2).aenv(*LONG).voice('legato', 0.25) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Morph').mod('m1', 'at1', 0.5).macro(2, 'Drone').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'at3', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.4).tags('shruti', 'microtonal', 'glide', 'legato')
jiX(p, vib=0.015, press=[('at4', 0.4)], slide=[('at1', 0.4)])

# ------------------------------------------------------------------------------------------- maqam
p = P('Rast Desert Oud', JI).tune('Maqam Rast', root='C', snap=True)
p.ph('a', 'String', 'Strike', decay=3.5, damp=0.35, hard=0.55, pos=0.2, level=0.55) \
 .ph('b', 'Membrane', 'Osc Input', decay=0.6, damp=0.5, hard=0.3, oct=-1, level=0.2) \
 .off('c').f1('SVF LP12', 5000, res=0.08, key=0.3).aenv(0.002, 3, 0.0, 2.2).curve(a=1) \
 .macro(1, 'Pluck').mod('m1', 'at2', 0.5).macro(2, 'Body').mod('m2', 'blevel', 0.45).macro(3, 'Position').mod('m3', 'at1', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('oud', 'maqam', 'plucked', 'desert')
jiX(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Bayati Ney Wind', JI).tune('Maqam Bayati', root='D', snap=True)
p.ph('a', 'Tube', 'Breath', decay=4, damp=0.3, hard=0.45, pos=0.3, level=0.55).noise(0.08, 0.6) \
 .off('b', 'c').f1('SVF BP12', 1400, res=0.2, key=0.5).f2('SVF LP12', 7000).aenv(0.18, 2, 0.95, 1.6).voice('legato', 0.06) \
 .macro(1, 'Breath').mod('m1', 'at2', 0.5).macro(2, 'Air').mod('m2', 'noise', 0.15).macro(3, 'Tube').mod('m3', 'at1', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.4) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('ney', 'maqam', 'flute', 'breath', 'solo')
jiX(p, vib=0.03, press=[('at2', 0.5)], slide=[('at1', 0.4)])

# ------------------------------------------------------------------------------------------- gamelan
p = P('Slendro Bronze Garden', JI).tune('Slendro', root='D')
p.ph('a', 'Bar', 'Strike', decay=7, damp=0.1, hard=0.75, stretch=0.2, level=0.5) \
 .ph('b', 'Bell', 'Strike', decay=9, damp=0.08, hard=0.6, oct=1, level=0.25, pan=0.3) \
 .off('c').f1('SVF LP12', 9000, res=0.03, key=0.1).aenv(0.002, 7, 0.0, 7).curve(a=1) \
 .macro(1, 'Mallet').mod('m1', 'at2', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.45).macro(3, 'Ring').mod('m3', 'at3', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.18, 0.08, 0.45).tags('gamelan', 'metallophone', 'mallet', 'physical')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.4)])

p = P('Pelog Gong Ritual', JI).tune('Pelog', root='C')
p.ph('a', 'Plate', 'Strike', decay=14, damp=0.05, hard=0.5, level=0.45) \
 .ph('b', 'Bar', 'Strike', decay=6, damp=0.1, hard=0.7, oct=1, level=0.3) \
 .gr('c', 'fixed', sample='Metal/Gamelan Pulse', pos=0.3, size=0.6, density=6, spray=0.3, level=0.2) \
 .f1('SVF LP12', 7000, res=0.03, key=0.1).aenv(0.002, 12, 0.0, 12).curve(a=1) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'at4', 0.3) \
 .macro(1, 'Gong').mod('m1', 'alevel', 0.4).macro(2, 'Saron').mod('m2', 'blevel', 0.45).macro(3, 'Pulse').mod('m3', 'clevel', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('gamelan', 'gong', 'ritual', 'physical')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.4)])

p = P('Slendro Shimmer Pad', JI).tune('Slendro', root='A', glide=1.0)
p.wt('a', 'Glass & Bell/Ice Shimmer', pos=0.4, blend='spectral', voices=4, detune=9, level=0.45) \
 .add('b', 'Bell', 'Glass', morph=0.4, partials=32, shim=0.35, srate=0.15, level=0.3) \
 .off('c').f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.018, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt3', 0.3) \
 .macro(1, 'Glass').mod('m1', 'at1', 0.5).macro(2, 'Bells').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'bt4', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.45).tags('gamelan', 'pad', 'shimmer')
jiX(p, press=[('blevel', 0.3)], slide=[('at1', 0.4)])

# ------------------------------------------------------------------------------------------- Partch / BP / xenharmonic
p = P('Partch Cloud Chamber Bowls', JI).tune('Partch 43', root='G')
p.ph('a', 'Glass Bowl', 'Strike', decay=10, damp=0.06, hard=0.55, level=0.5) \
 .ph('b', 'Bell', 'Strike', decay=8, damp=0.08, hard=0.5, oct=-1, level=0.3, pan=-0.3) \
 .off('c').f1('SVF LP12', 9000, res=0.03, key=0.1).aenv(0.002, 10, 0.0, 10).curve(a=1) \
 .macro(1, 'Mallet').mod('m1', 'at2', 0.5).macro(2, 'Low Bowl').mod('m2', 'blevel', 0.45).macro(3, 'Ring').mod('m3', 'at3', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('partch', 'bowls', 'microtonal', 'physical')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.4)])

p = P('Partch Adapted Viola', JI).tune('Partch 43', root='G')
p.ph('a', 'String', 'Bow', decay=7, damp=0.22, hard=0.5, pos=0.14, level=0.55) \
 .off('b', 'c').f1('SVF LP12', 6000, res=0.05, key=0.3).aenv(0.3, 2, 0.95, 2).voice('legato', 0.1) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).macro(2, 'Position').mod('m2', 'at1', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('partch', 'viola', 'bowed', 'solo')
jiX(p, vib=0.022, press=[('at2', 0.45)], slide=[('at1', 0.5)])

p = P('Bohlen-Pierce Clarinet Choir', JI).tune('Bohlen-Pierce', root='C')
p.add('a', 'Square', 'Reed', morph=0.4, partials=32, oe=-0.9, shim=0.1, srate=0.05, level=0.5) \
 .add('b', 'Square', 'Hollow', morph=0.3, partials=24, oe=-0.9, oct=-1, level=0.3, pan=0.3) \
 .off('c').noise(0.03, 0.6).f1('SVF LP12', 4200, res=0.08, key=0.35).aenv(0.2, 2, 0.95, 2.5) \
 .lfo(1, 0.03, 'drift', 'voice').mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Reed').mod('m1', 'at1', 0.5).macro(2, 'Low').mod('m2', 'blevel', 0.45).macro(3, 'Breath').mod('m3', 'noise', 0.1).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.4).tags('bohlen-pierce', 'clarinet', 'tritave', 'xenharmonic')
jiX(p, vib=0.012, press=[('f1cut', 0.35)], slide=[('at1', 0.4)])

p = P('Harmonic Ladder Pluck', JI).tune('Harmonics 8-16', root='C')
p.ph('a', 'String', 'Strike', decay=3, damp=0.35, hard=0.6, pos=0.2, level=0.55) \
 .add('b', 'Octaves', 'Primes', morph=0.5, partials=24, shim=0.15, srate=0.2, oct=1, level=0.2) \
 .off('c').f1('SVF LP12', 7000, res=0.05, key=0.3).aenv(0.002, 3, 0.0, 2.5).curve(a=1) \
 .macro(1, 'Pluck').mod('m1', 'at2', 0.5).macro(2, 'Halo').mod('m2', 'blevel', 0.45).macro(3, 'Position').mod('m3', 'at1', 0.5).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('harmonic', 'pluck', 'ladder', 'arpeggio-friendly')
jiX(p, vib=0, press=[('at2', 0.35)], slide=[('at1', 0.5)])

p = P('Quarter-Tone Ghost Strings', JI).tune('24-EDO', root='C', glide=0.3)
p.wt('a', 'Ambient/Ghost Strings', pos=0.4, blend='spectral', voices=3, detune=7, level=0.5) \
 .an('b', 'saw', voices=2, detune=10, level=0.25, slop=0.4).off('c').f1('OB-Xa LP12', 2600, res=0.1, key=0.3).aenv(*LONG) \
 .lfo(1, 0.025, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Ghost').mod('m1', 'at1', 0.5).macro(2, 'Saw').mod('m2', 'blevel', 0.45).macro(3, 'Detune').mod('m3', 'detune', 0.3).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.45).tags('quarter-tone', 'strings', 'xenharmonic', 'eerie')
jiX(p, press=[('f1cut', 0.3)], slide=[('at1', 0.4)])

p = P('Nineteen Tone Organum', JI).tune('19-EDO', root='C')
p.add('a', 'Organ', 'Choir Oo', morph=0.3, partials=40, shim=0.1, srate=0.05, level=0.5) \
 .an('b', 'sine', oct=-1, level=0.3, slop=0.1).off('c').f1('SVF LP12', 6000, res=0.04, key=0.2).aenv(1.5, 3, 1, 5) \
 .macro(1, 'Choir').mod('m1', 'at1', 0.5).macro(2, 'Low').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'at3', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.4).tags('19-edo', 'organ', 'xenharmonic')
jiX(p, vib=0, press=[('at1', 0.3)], slide=[('at3', 0.4)])

p = P('Carlos Alpha Drift', JI).tune('Carlos Alpha', root='C', glide=0.2)
p.wt('a', 'Glass & Bell/Prism', pos=0.3, blend='spectral', voices=3, detune=6, level=0.45) \
 .add('b', 'Glass', 'Bell', morph=0.4, partials=24, shim=0.3, srate=0.2, level=0.3) \
 .off('c').f1('SVF LP12', 7000, res=0.05, key=0.2).aenv(0.6, 3, 0.8, 5) \
 .lfo(1, 0.05, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Prism').mod('m1', 'at1', 0.5).macro(2, 'Bell').mod('m2', 'blevel', 0.45).macro(3, 'Shimmer').mod('m3', 'bt4', 0.4).macro(4, 'Tone').mod('m4', 'f1cut', 0.5) \
 .chorus('chorus', 0.2, 0.06, 0.4).tags('carlos', 'alpha', 'xenharmonic', 'non-octave')
jiX(p, press=[('blevel', 0.3)], slide=[('at1', 0.4)])
