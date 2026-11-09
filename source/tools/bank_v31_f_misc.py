# Driftfield 3.1 bank, part F: CHOIRS, BELLS, SEQUENCES, KEYS, BASSES, LEADS + flange-filter showcases
CH, BL, SQ, KY, BS, LD = 'Choirs & Voices', 'Bells & Resonators', 'Sequence & Plucks', 'Keys & Organs', 'Basses', 'Leads'

# ------------------------------------------------------------------------------------------------ CHOIRS
p = P('Choir of Ash', CH).gr('a', 'keyed', sample='Voices/Choir Ah', pos=0.3, size=0.9, density=12, spray=0.3, level=0.55)
p.add('b', 'Choir Ah', 'Choir Oo', morph=0.3, partials=64, shim=0.2, pdet=7, level=0.35).off('c') \
 .f1('SVF LP12', 5500, res=0.05, key=0.1).aenv(3, 3, 1, 10) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Choir').mod('m1', 'alevel', 0.5).macro(2, 'Vowel').mod('m2', 'bt1', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.05, 0.5).tags('choir', 'dark', 'roach')
X(p, vib=0.016, press=[('blevel', 0.3)], slide=[('bt4', 0.6)])

p = P('Vowel Shaping Choir', CH).add('a', 'Choir Ah', 'Choir Oo', morph=0.5, partials=64, vowel=0.0, vowelamt=0.9, shim=0.2, pdet=8, level=0.55)
p.off('b', 'c').f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(1.5, 3, 1, 6) \
 .macro(1, 'Vowel').mod('m1', 'at4', 0.8).macro(2, 'Morph').mod('m2', 'at1', 0.5).macro(3, 'Shimmer').mod('m3', 'f1cut', 0.4) \
 .chorus('ensemble', 0.3, 0.06, 0.5).tags('choir', 'vowel', 'expressive', 'mpe-showcase')
X(p, vib=0.018, st=0.0, press=[('at1', 0.4)], slide=[('at4', 0.9)])

p = P('Monk Ensemble', CH).gr('a', 'keyed', sample='Voices/Monk Hum Low', pos=0.3, size=0.9, density=10, spray=0.3, level=0.55)
p.add('b', 'Throat', 'Choir Oo', morph=0.4, partials=48, peak=8, peakamt=0.5, level=0.3).off('c') \
 .f1('SVF LP12', 3500, res=0.05, key=0.1).aenv(2, 3, 1, 8) \
 .macro(1, 'Monks').mod('m1', 'alevel', 0.5).macro(2, 'Overtone').mod('m2', 'bt2', 0.5).macro(3, 'Body').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.5).tags('monks', 'chant', 'low', 'roach')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Whisper Choir', CH).gr('a', 'fixed', sample='Voices/Whisper Air', pos=0.3, size=0.7, density=12, level=0.45)
p.add('b', 'Choir Oo', 'Hollow', morph=0.3, partials=40, shim=0.2, level=0.4).noise(0.1, 0.6).off('c') \
 .f1('Vowel', 900, res=0.4, key=0.0, shape=0.6).aenv(2, 3, 1, 8) \
 .lfo(1, 0.05, 'drift').mod('lfo1', 'f1shape', 0.4) \
 .macro(1, 'Whisper').mod('m1', 'alevel', 0.5).macro(2, 'Tone').mod('m2', 'blevel', 0.5).macro(3, 'Vowel').mod('m3', 'f1shape', 0.6) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('whisper', 'choir', 'breathy', 'eerie')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('f1shape', 0.5)])

p = P('Overtone Duet', CH).wt('a', 'Vocal/Sygyt High', pos=0.3, blend='spectral', level=0.45)
p.wt('b', 'Vocal/Khoomei Low', pos=0.3, blend='spectral', oct=-1, level=0.4).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(1.5, 3, 1, 8) \
 .macro(1, 'Whistle').mod('m1', 'at1', 0.6).macro(2, 'Growl').mod('m2', 'bt1', 0.6).macro(3, 'Balance').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('overtone', 'throat', 'duet', 'roach')
X(p, vib=0.008, press=[('at1', 0.4)], slide=[('bt1', 0.6)])

p = P('Children of the Sky', CH).wt('a', 'Vowel Journeys/Soprano Rise', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.add('b', 'Choir Ah', 'Glass', morph=0.4, partials=56, shim=0.3, oct=1, level=0.25).off('c') \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(2.5, 3, 1, 9) \
 .lfo(1, 0.03, 'tri').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Vowel').mod('m1', 'at1', 0.5).macro(2, 'Halo').mod('m2', 'blevel', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.3, 0.05, 0.5).tags('choir', 'soprano', 'heavenly', 'corbacho')
X(p, vib=0.016, press=[('blevel', 0.3)], slide=[('at1', 0.6)])

p = P('Breathing Vocal Pad', CH).gr('a', 'keyed', sample='Voices/Choir Vowel Drift', pos=0.3, scan=0.02, size=0.8, density=12, level=0.55)
p.gr('b', 'fixed', sample='Voices/Breath Cycle', pos=0.3, size=0.9, density=8, level=0.25).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(2.5, 3, 1, 9) \
 .macro(1, 'Drift').mod('m1', 'at1', 0.5).macro(2, 'Breath').mod('m2', 'blevel', 0.5).macro(3, 'Density').mod('m3', 'at3', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('vocal', 'breath', 'granular')
X(p, vib=0.012, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Formant Talker', CH).an('a', 'saw', voices=2, detune=6, level=0.5).off('b', 'c')
p.f1('Vowel', 1000, res=0.6, key=0.2, shape=0.0).aenv(0.05, 1, 1, 0.5) \
 .lfo(1, 2.5, 'sh', 'voice', smooth=0.5).mod('lfo1', 'f1shape', 0.5, via='mw') \
 .macro(1, 'Vowel').mod('m1', 'f1shape', 0.9).macro(2, 'Size').mod('m2', 'f1cut', 0.4).macro(3, 'Talk Speed').mod('m3', 'lfo1rate', 0.5) \
 .chorus('chorus', 0.15, 0.1, 0.3).tags('talking', 'vowel', 'formant', 'mpe-showcase')
X(p, vib=0.012, press=[('f1res', 0.2)], slide=[('f1shape', 0.9)])

# ------------------------------------------------------------------------------------------------ BELLS & RESONATORS
p = P('Temple Bell Cluster', BL).ph('a', 'Bell', 'Strike', decay=10, damp=0.12, hard=0.65, level=0.55)
p.wt('b', 'Metallic/Temple Bell', pos=0.3, blend='spectral', level=0.3).off('c').aenv(0.002, 9, 0.0, 9).curve(a=1) \
 .macro(1, 'Strike').mod('m1', 'at2', 0.5).macro(2, 'Wavetable').mod('m2', 'blevel', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('bell', 'temple', 'ritual')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Gamelan Keys', BL).ph('a', 'Bar', 'Strike', decay=4, damp=0.2, hard=0.7, stretch=0.15, level=0.55)
p.ph('b', 'Bar', 'Strike', decay=4, damp=0.2, hard=0.7, fine=12, level=0.35).off('c').aenv(0.002, 4, 0.0, 4).curve(a=1) \
 .macro(1, 'Shimmer').mod('m1', 'bpitch', 0.004).macro(2, 'Hardness').mod('m2', 'at2', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('gamelan', 'metallophone', 'beating')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Glass Marimba', BL).ph('a', 'Glass Bowl', 'Strike', decay=2.5, damp=0.3, hard=0.6, level=0.55)
p.wt('b', 'Organic/Kalimba Tine', pos=0.3, level=0.25).off('c').aenv(0.002, 2.5, 0.0, 2.5).curve(a=1) \
 .macro(1, 'Glass').mod('m1', 'at2', 0.5).macro(2, 'Tine').mod('m2', 'blevel', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('glass', 'marimba', 'percussive')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Coupled Plates', BL).ph('a', 'Plate', 'Strike', decay=7, damp=0.12, hard=0.75, couple=0.7, level=0.5)
p.ph('b', 'Plate', 'Osc Input', decay=7, damp=0.12, hard=0.5, couple=0.7, semi=5, level=0.35).off('c').aenv(0.002, 7, 0.0, 7).curve(a=1) \
 .macro(1, 'Coupling').mod('m1', 'blevel', 0.5).macro(2, 'Hardness').mod('m2', 'at2', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('plate', 'metallic', 'coupled', 'physical')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Tanpura Flange Pluck', BL).ph('a', 'Tanpura', 'Strike', decay=8, damp=0.05, hard=0.8, buzz=0.7, level=0.6).off('b', 'c')
p.f1('Flg HL6+', 600, res=0.55, key=1.0, shape=0.5).aenv(0.002, 6, 0.5, 6).curve(a=1) \
 .lfo(1, 0.15, 'tri').mod('lfo1', 'f1cut', 0.15) \
 .macro(1, 'Jawari').mod('m1', 'at4', -0.3).macro(2, 'Flange').mod('m2', 'f1res', 0.4).macro(3, 'Sweep').mod('m3', 'lfo1rate', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('tanpura', 'flanger-filter', 'indian', 'roach')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('f1cut', 0.3)])

p = P('Comb Resonator Bells', BL).noise(1.0, 0.6).off('a', 'b', 'c')
p.f1('Cmb HL6+', 440, res=0.9, key=1.0, shape=0.6).aenv(0.001, 3, 0.0, 3).curve(a=1) \
 .macro(1, 'Ring').mod('m1', 'f1res', 0.08).macro(2, 'Tone').mod('m2', 'f1shape', 0.5).macro(3, 'Decay').mod('m3', 'amp', 0.1) \
 .chorus('chorus', 0.15, 0.06, 0.4).tags('comb', 'resonator', 'karplus', 'flanger-filter')
X(p, vib=0, press=[('f1res', 0.08)], slide=[('f1shape', 0.5)])

p = P('Sympathetic Zither', BL).ph('a', 'Sympathetic', 'Strike', decay=6, damp=0.15, hard=0.7, buzz=0.85, level=0.6).off('b', 'c')
p.aenv(0.002, 5, 0.3, 5).curve(a=1) \
 .macro(1, 'Sympathy').mod('m1', 'at4', -0.4).macro(2, 'Hardness').mod('m2', 'at2', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('zither', 'sympathetic', 'plucked')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Bowl Strike Shimmer', BL).ph('a', 'Glass Bowl', 'Strike', decay=14, damp=0.08, hard=0.6, couple=0.4, level=0.55)
p.add('b', 'Bell', 'Glass', morph=0.4, partials=64, stretch=0.2, shim=0.5, level=0.2).off('c').aenv(0.002, 12, 0.0, 12).curve(a=1) \
 .macro(1, 'Shimmer').mod('m1', 'blevel', 0.5).macro(2, 'Hardness').mod('m2', 'at2', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('bowl', 'shimmer', 'meditation', 'corbacho')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

# ------------------------------------------------------------------------------------------------ SEQUENCES & PLUCKS
def berlin(p, notes, rate='1/16', gates=None, glide=0.0):
    return p.seq('notes', rate, len(notes), 0.45, 0.0, glide, values=notes, gates=gates or [1] * len(notes))

p = P('Tangerine Cascade', SQ).an('a', 'saw', voices=2, detune=6, level=0.55).an('b', 'pulse', pw=0.3, oct=-1, level=0.3).off('c')
p.f1('Ladder LP24', 900, res=0.35, key=0.4, env=0.4).fenv(0.001, 0.25, 0.0, 0.2).aenv(0.002, 0.3, 0.0, 0.25).curve(a=1, f=1)
berlin(p, [0, 12, 7, 3, 0, 15, 10, 7, 0, 12, 7, 3, 5, 17, 12, 10])
p.lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Env').mod('m2', 'f1res', 0.3).macro(3, 'Sub').mod('m3', 'blevel', 0.5) \
 .chorus('chorus', 0.15, 0.08, 0.3).tags('berlin', 'sequenced', 'tangerine', 'classic')
X(p, vib=0, press=[('f1cut', 0.4)], slide=[('f1res', 0.3)])

p = P('Glass Sequence Flanger', SQ).wt('a', 'Glass & Bell/Crystal Squares', pos=0.3, level=0.5).off('b', 'c')
p.f1('Flg L6+', 300, res=0.6, key=0.0, shape=0.5).f2('SVF LP12', 7000).aenv(0.002, 0.4, 0.0, 0.3).curve(a=1)
berlin(p, [0, 7, 12, 19, 0, 7, 14, 19, 0, 5, 12, 17, 0, 7, 12, 24], '1/16')
p.lfo(1, 0.07, 'tri').mod('lfo1', 'f1cut', 0.5) \
 .macro(1, 'Flange').mod('m1', 'f1res', 0.4).macro(2, 'Sweep').mod('m2', 'lfo1rate', 0.5).macro(3, 'Glass').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.12, 0.06, 0.3).tags('sequenced', 'glass', 'flanger-filter', 'shimmer')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

p = P('Phaser Pulse Sequence', SQ).an('a', 'pulse', pw=0.4, level=0.55).sub(0.3, 'square').off('b', 'c')
p.f1('Phs 48+', 800, res=0.6, key=0.3, shape=0.5).f2('Ladder LP24', 3000, res=0.2, env=0.3).fenv(0.001, 0.2, 0.0, 0.2).aenv(0.002, 0.25, 0.0, 0.2).curve(a=1)
berlin(p, [0, 0, 12, 0, 10, 0, 7, 0, 0, 0, 12, 0, 15, 0, 7, 10], '1/16')
p.lfo(1, 0.05, 'sine').mod('lfo1', 'f1cut', 0.6) \
 .macro(1, 'Phase').mod('m1', 'f1res', 0.4).macro(2, 'Sweep').mod('m2', 'lfo1rate', 0.5).macro(3, 'Cutoff').mod('m3', 'f2cut', 0.5) \
 .chorus('chorus', 0.1, 0.06, 0.3).tags('sequenced', 'phaser-filter', 'berlin')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

p = P('Ritual Arp Drums', SQ).ph('a', 'Membrane', 'Strike', decay=0.6, damp=0.4, hard=0.6, level=0.6)
p.ph('b', 'Tube', 'Strike', decay=0.4, damp=0.5, hard=0.5, oct=1, level=0.3).off('c').aenv(0.001, 0.5, 0.0, 0.4).curve(a=1) \
 .arp('random', 2, '1/16', 0.4, swing=0.15, hold=True) \
 .lfo(1, 0.5, 'sh', sync=True, div='1/16').mod('lfo1', 'at2', 0.3) \
 .macro(1, 'Skin').mod('m1', 'at2', 0.5).macro(2, 'Wood').mod('m2', 'blevel', 0.5).macro(3, 'Tone').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.08, 0.05, 0.3).tags('tribal', 'drums', 'ritual', 'roach', 'arp')
X(p, vib=0, press=[('at2', 0.3)], slide=[('at1', 0.5)])

p = P('Kalimba Generator', SQ).wt('a', 'Organic/Kalimba Tine', pos=0.2, level=0.55).ph('b', 'Bar', 'Strike', decay=1.5, damp=0.3, hard=0.6, oct=1, level=0.2).off('c')
p.aenv(0.002, 1.5, 0.0, 1.5).curve(a=1).arp('random', 2, '1/8', 0.5, swing=0.2, hold=True) \
 .macro(1, 'Tine').mod('m1', 'at1', 0.5).macro(2, 'Bar').mod('m2', 'blevel', 0.5).macro(3, 'Tone').mod('m3', 'cut', 0.5) \
 .chorus('chorus', 0.12, 0.06, 0.3).tags('kalimba', 'generative', 'arp', 'happy')
X(p, vib=0, press=[('at1', 0.3)], slide=[('bt1', 0.4)])

p = P('Dark Berlin Bass Seq', SQ).an('a', 'saw', level=0.6).an('b', 'saw', fine=-8, level=0.4).off('c')
p.f1('Ladder LP24', 400, res=0.4, key=0.3, env=0.5).fenv(0.001, 0.18, 0.0, 0.15).aenv(0.002, 0.2, 0.0, 0.15).curve(a=1, f=1)
berlin(p, [-12, -12, 0, -12, -9, -12, -5, -12, -12, -12, 0, -12, -2, -12, -5, -7], '1/16')
p.lfo(1, 0.025, 'drift').mod('lfo1', 'f1cut', 0.5) \
 .macro(1, 'Cutoff').mod('m1', 'f1cut', 0.5).macro(2, 'Res').mod('m2', 'f1res', 0.4).macro(3, 'Env').mod('m3', 'f1drive', 0.5) \
 .drive(0.15).nochorus().tags('berlin', 'bass', 'sequenced', 'dark', 'roach')
X(p, vib=0, press=[('f1cut', 0.4)], slide=[('f1res', 0.3)])

p = P('Steel Drum Pluck', SQ).ph('a', 'Plate', 'Strike', decay=1.2, damp=0.3, hard=0.55, stretch=0.1, level=0.6).off('b', 'c')
p.aenv(0.002, 1.2, 0.0, 1.2).curve(a=1).f1('SVF LP12', 6000) \
 .macro(1, 'Hardness').mod('m1', 'at2', 0.5).macro(2, 'Decay').mod('m2', 'at3', 0.5).macro(3, 'Position').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('steel-drum', 'pluck', 'caribbean')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Comb Pluck Harp', SQ).noise(1.0, 0.5).an('a', 'saw', level=0.2).off('b', 'c')
p.f1('Cmb L6+', 400, res=0.92, key=1.0, shape=0.4).aenv(0.001, 2, 0.0, 2).curve(a=1) \
 .macro(1, 'Ring').mod('m1', 'f1res', 0.06).macro(2, 'Brightness').mod('m2', 'f1shape', 0.5).macro(3, 'Body').mod('m3', 'alevel', 0.5) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('pluck', 'harp', 'comb', 'karplus', 'flanger-filter')
X(p, vib=0, press=[('f1res', 0.05)], slide=[('f1shape', 0.5)])

# ------------------------------------------------------------------------------------------------ KEYS & ORGANS
p = P('Drawbar Cathedral', KY).wt('a', 'Organs/Organ Cathedral', pos=0.3, voices=2, detune=4, level=0.55).off('b', 'c')
p.aenv(0.02, 1, 1, 1.2).lfo(1, 6.2, 'sine').mod('lfo1', 'apitch', 0.002).mod('lfo1', 'alevel', 0.05) \
 .macro(1, 'Stops').mod('m1', 'at1', 0.6).macro(2, 'Leslie').mod('m2', 'lfo1rate', -0.6).macro(3, 'Drive').mod('m3', 'fxdrive', 0.5) \
 .drive(0.1).chorus('chorus', 0.3, 0.8, 0.4).tags('organ', 'drawbar', 'church')
X(p, vib=0, press=[('at1', 0.4)], slide=[('lfo1rate', 0.5)])

p = P('Rhodes in the Rain', KY).an('a', 'sine', level=0.6).an('b', 'sine', oct=2, level=0.0).fm('a', 'b', 0.25, 'pm').off('c')
p.gr('c', 'fixed', sample='Nature/Rain Light', pos=0.3, size=0.6, density=12, level=0.2) \
 .aenv(0.002, 3, 0.4, 1.2).curve(a=1).menv(0.001, 1.2, 0.0, 1).mod('menv', 'afm', 0.4).mod('vel', 'afm', 0.3) \
 .macro(1, 'Bell').mod('m1', 'afm', 0.4).macro(2, 'Rain').mod('m2', 'clevel', 0.5).macro(3, 'Tremolo').mod('m3', 'fxphaser', 0.5) \
 .phaser(0.2, 0.3, 0.5, 0.3).chorus('chorus', 0.2, 0.3, 0.4).tags('electric-piano', 'fm', 'rain', 'lo-fi')
X(p, vib=0, press=[('afm', 0.3)], slide=[('fxphaser', 0.4)])

p = P('Reed Organ Flange', KY).wt('a', 'Organs/Organ Reed Swell', pos=0.4, voices=2, detune=5, level=0.55).off('b', 'c')
p.f1('Flg +', 220, res=0.5, key=0.0, shape=0.5).aenv(0.03, 1, 1, 1) \
 .lfo(1, 0.12, 'tri').mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Flange').mod('m1', 'f1res', 0.4).macro(2, 'Rate').mod('m2', 'lfo1rate', 0.5).macro(3, 'Swell').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('organ', 'flanger-filter', 'psychedelic')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

# ------------------------------------------------------------------------------------------------ BASSES
p = P('Sub Weight Bass', BS).wt('a', 'Bass/Sub Weight', pos=0.3, level=0.65).an('b', 'saw', level=0.2).off('c')
p.f1('Ladder LP24', 600, res=0.15, key=0.4, env=0.3).fenv(0.001, 0.4, 0.2, 0.3).aenv(0.003, 1, 0.8, 0.3).curve(a=1) \
 .voice('legato', 0.03).macro(1, 'Weight').mod('m1', 'at1', 0.5).macro(2, 'Grit').mod('m2', 'blevel', 0.5).macro(3, 'Cutoff').mod('m3', 'f1cut', 0.5) \
 .nochorus().tags('bass', 'sub', 'mono')
X(p, vib=0, press=[('f1cut', 0.4)], slide=[('at1', 0.5)])

p = P('Growl Formant Bass', BS).wt('a', 'Bass/Growl Formant', pos=0.3, level=0.6).off('b', 'c').sub(0.3)
p.f1('Ladder LP24', 1200, res=0.2, key=0.4).aenv(0.003, 1, 0.8, 0.25).curve(a=1) \
 .lfo(1, 2, 'sine', sync=True, div='1/8').mod('lfo1', 'at1', 0.4, via='mw') \
 .voice('mono', 0.02).macro(1, 'Growl').mod('m1', 'at1', 0.6).macro(2, 'Wobble').mod('m2', 'lfo1rate', 0.5).macro(3, 'Sub').mod('m3', 'sub', 0.5) \
 .drive(0.2).nochorus().tags('bass', 'growl', 'formant', 'wobble')
X(p, vib=0, press=[('at1', 0.5)], slide=[('f1cut', 0.4)])

p = P('Moog Sweep Bass', BS).wt('a', 'Bass/Moog Saw Sweep', pos=0.1, level=0.6).an('b', 'saw', oct=-1, level=0.3).off('c')
p.f1('Ladder LP24', 900, res=0.3, key=0.4, env=0.3).fenv(0.001, 0.3, 0.1, 0.3).aenv(0.003, 1, 0.8, 0.25).curve(a=1, f=1) \
 .menv(0.001, 0.4, 0.0, 0.3).mod('menv', 'at1', 0.6).voice('mono', 0.04) \
 .macro(1, 'Sweep').mod('m1', 'at1', 0.5).macro(2, 'Cutoff').mod('m2', 'f1cut', 0.5).macro(3, 'Res').mod('m3', 'f1res', 0.4) \
 .nochorus().tags('bass', 'moog', 'analog', 'mono')
X(p, vib=0, press=[('f1cut', 0.4)], slide=[('at1', 0.5)])

p = P('Drone Bass Flanged', BS).an('a', 'saw', voices=2, detune=6, oct=-1, level=0.55).sub(0.3).off('b', 'c')
p.f1('Flg H6+', 120, res=0.6, key=0.5, shape=0.4).f2('Ladder LP24', 1200, res=0.2).aenv(0.01, 2, 1, 1) \
 .lfo(1, 0.06, 'tri').mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Flange').mod('m1', 'f1res', 0.4).macro(2, 'Sweep').mod('m2', 'lfo1rate', 0.5).macro(3, 'Cutoff').mod('m3', 'f2cut', 0.5) \
 .nochorus().tags('bass', 'drone', 'flanger-filter', 'roach')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

# ------------------------------------------------------------------------------------------------ LEADS (MPE-first)
p = P('Expressive Cello Lead', LD).wt('a', 'Strings & Reeds/Cello Body', pos=0.3, voices=2, detune=5, level=0.55).off('b', 'c')
p.f1('SVF LP12', 3500, res=0.1, key=0.4).aenv(0.15, 2, 0.9, 0.6).curve(a=-0.3).voice('legato', 0.06) \
 .macro(1, 'Bow').mod('m1', 'at1', 0.5).macro(2, 'Brightness').mod('m2', 'f1cut', 0.5).macro(3, 'Vibrato').mod('m3', 'lfo6rate', 0.3) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('lead', 'cello', 'expressive', 'mpe-showcase')
X(p, vib=0.025, rate=5.3, pa=0.6, pc=0.4, press=[('at1', 0.4)], slide=[('f1cut', 0.4)])

p = P('Shakuhachi Lead', LD).wt('a', 'Organic/Shakuhachi', pos=0.3, level=0.55).noise(0.12, 0.4).off('b', 'c')
p.f1('SVF LP12', 5000, res=0.05, key=0.4).aenv(0.08, 1, 0.9, 0.4).voice('legato', 0.05) \
 .macro(1, 'Breath').mod('m1', 'noise', 0.4).macro(2, 'Overblow').mod('m2', 'at1', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.1, 0.05, 0.3).tags('lead', 'flute', 'japan', 'expressive')
X(p, vib=0.02, rate=4.8, pa=0.6, press=[('noise', 0.3), ('at1', 0.4)], slide=[('f1cut', 0.3)])

p = P('Virus Sync Lead', LD).an('a', 'saw', sync=12, voices=3, detune=10, level=0.55).an('b', 'saw', oct=-1, level=0.3).off('c')
p.f1('Ladder LP24', 3000, res=0.2, key=0.4, env=0.2).fenv(0.001, 0.6, 0.4, 0.4).aenv(0.003, 1, 0.9, 0.3).curve(a=1) \
 .menv(0.001, 0.8, 0.2, 0.5).mod('menv', 'at2', 0.5).voice('legato', 0.04) \
 .macro(1, 'Sync').mod('m1', 'at2', 0.5).macro(2, 'Cutoff').mod('m2', 'f1cut', 0.5).macro(3, 'Low').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.2, 0.1, 0.4).tags('lead', 'sync', 'virus', 'screaming')
X(p, vib=0.02, press=[('at2', 0.5)], slide=[('f1cut', 0.4)])

p = P('Desert Throat Lead', LD).wt('a', 'Vocal/Overtone Singer', pos=0.3, blend='spectral', level=0.55).off('b', 'c')
p.f1('SVF LP12', 6000, res=0.05, key=0.3).aenv(0.1, 1, 0.9, 0.6).voice('legato', 0.08) \
 .macro(1, 'Overtone').mod('m1', 'at1', 0.6).macro(2, 'Air').mod('m2', 'f1cut', 0.5).macro(3, 'Vibrato').mod('m3', 'lfo6rate', 0.3) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('lead', 'overtone', 'roach', 'expressive')
X(p, vib=0.02, press=[('at1', 0.4)], slide=[('at1', 0.6)])

p = P('Phaser Glide Lead', LD).an('a', 'sawpulse', pw=0.4, voices=2, detune=8, level=0.55).off('b', 'c')
p.f1('Phs 48HL6+', 900, res=0.65, key=0.4, shape=0.5).f2('Ladder LP24', 4000, res=0.15).aenv(0.01, 1, 0.9, 0.4).voice('legato', 0.12) \
 .lfo(1, 0.2, 'tri').mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Phase').mod('m1', 'f1res', 0.4).macro(2, 'Sweep').mod('m2', 'lfo1rate', 0.5).macro(3, 'Cutoff').mod('m3', 'f2cut', 0.5) \
 .chorus('chorus', 0.12, 0.08, 0.3).tags('lead', 'phaser-filter', 'glide', 'retro')
X(p, vib=0.018, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

# ------------------------------------------------------------------------------------------------ flange-filter textures / pads
p = P('Barber Pole Drone', 'Drones').an('a', 'saw', voices=4, detune=12, slop=0.4, level=0.5).noise(0.08, 0.5).off('b', 'c')
p.f1('FPhs 12HL6+', 400, res=0.7, key=0.2, shape=0.5).f2('SVF LP12', 6000).aenv(*LONG) \
 .lfo(1, 0.02, 'sawup').mod('lfo1', 'f1cut', 0.6) \
 .macro(1, 'Feedback').mod('m1', 'f1res', 0.3).macro(2, 'Speed').mod('m2', 'lfo1rate', 0.6).macro(3, 'Air').mod('m3', 'noise', 0.3) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('flanger-filter', 'barber-pole', 'drone', 'psychedelic')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.3)])

p = P('Jet Stream Pad', 'Pads').an('a', 'saw', voices=5, detune=16, level=0.5).off('b', 'c')
p.f1('Flg -', 600, res=0.7, key=0.0, shape=0.5).f2('SEM LP12', 4000).aenv(*PAD) \
 .lfo(1, 0.05, 'tri').mod('lfo1', 'f1cut', 0.8) \
 .macro(1, 'Jet').mod('m1', 'f1res', 0.3).macro(2, 'Speed').mod('m2', 'lfo1rate', 0.6).macro(3, 'Tone').mod('m3', 'f2cut', 0.5) \
 .chorus('ensemble', 0.25, 0.06, 0.5).tags('flanger-filter', 'jet', 'pad', '70s')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.5)])

p = P('Phase Cathedral Pad', 'Pads').wt('a', 'Organs/Organ Mixture', pos=0.3, voices=3, detune=7, level=0.5).off('b', 'c')
p.f1('Phs 36+', 700, res=0.6, key=0.0, shape=0.5).aenv(*SLOWPAD) \
 .lfo(1, 0.03, 'sine').mod('lfo1', 'f1cut', 0.7) \
 .macro(1, 'Phase').mod('m1', 'f1res', 0.3).macro(2, 'Speed').mod('m2', 'lfo1rate', 0.6).macro(3, 'Stops').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('phaser-filter', 'organ', 'cathedral')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.5)])

p = P('Comb Wind Texture', 'Textures').noise(0.7, 0.5).off('a', 'b', 'c')
p.f1('Cmb HL6-', 180, res=0.85, key=1.0, shape=0.5).f2('SVF LP12', 5000).aenv(*LONG) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'f1shape', 0.4).mod('lfo1', 'noise', 0.15) \
 .macro(1, 'Pitch Ring').mod('m1', 'f1res', 0.1).macro(2, 'Tone').mod('m2', 'f1shape', 0.5).macro(3, 'Air').mod('m3', 'f2cut', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).width(1.4).tags('comb', 'wind', 'pitched-noise', 'flanger-filter')
X(p, vib=0, press=[('f1res', 0.06)], slide=[('f1shape', 0.5)])

p = P('Flanged Rain Choir', 'Textures').gr('a', 'keyed', sample='Voices/Choir Oo', pos=0.3, size=0.8, density=12, level=0.5)
p.gr('b', 'fixed', sample='Nature/Rain Light', pos=0.3, size=0.6, density=12, level=0.3).off('c') \
 .f1('Flg HL6+', 350, res=0.55, key=0.0, shape=0.5).aenv(*LONG) \
 .lfo(1, 0.04, 'sine').mod('lfo1', 'f1cut', 0.6) \
 .macro(1, 'Flange').mod('m1', 'f1res', 0.4).macro(2, 'Rain').mod('m2', 'blevel', 0.5).macro(3, 'Speed').mod('m3', 'lfo1rate', 0.5) \
 .chorus('ensemble', 0.2, 0.05, 0.4).tags('choir', 'rain', 'flanger-filter', 'psychedelic')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.5)])

p = P('Cicada Phaser Field', 'Soundscapes').gr('a', 'fixed', sample='Nature/Cicadas', pos=0.3, size=0.8, density=10, level=0.5)
p.wt('b', 'Ambient/Desert Air', pos=0.3, blend='spectral', voices=3, detune=8, level=0.35).off('c') \
 .f1('Phs 48L6+', 1200, res=0.6, key=0.0, shape=0.5).aenv(*LONG) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'f1cut', 0.7) \
 .macro(1, 'Phase').mod('m1', 'f1res', 0.4).macro(2, 'Desert').mod('m2', 'blevel', 0.5).macro(3, 'Speed').mod('m3', 'lfo1rate', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('summer', 'phaser-filter', 'roach', 'heat')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.5)])

p = P('Metallic Flange Swell', 'FX & Details').gr('a', 'keyed', sample='Metal/Bowed Cymbal', pos=0.3, size=0.6, density=16, level=0.55).off('b', 'c')
p.f1('Flg HL6-', 500, res=0.75, key=0.0, shape=0.6).aenv(3, 0.5, 1.0, 2).curve(a=-0.6) \
 .mseg(1, [(0, 0.0, 0), (3, 1.0, 0.3)], loop=(1, 1), mode='env').mod('mseg1', 'f1cut', 0.8) \
 .macro(1, 'Feedback').mod('m1', 'f1res', 0.2).macro(2, 'Swell').mod('m2', 'mseg1rate', -0.5).macro(3, 'Scan').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('metal', 'swell', 'flanger-filter', 'transition', 'detail')
X(p, vib=0, press=[('f1res', 0.15)], slide=[('at1', 0.5)])
