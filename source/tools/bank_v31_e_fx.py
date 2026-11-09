# Driftfield 3.1 bank, part E: FX & DETAILS (beds, resonant details, risers, impacts, glitches, one-shots)
FXC = 'FX & Details'

def riser(points):
    return points

# ------------------------------------------------------------------------------------------------ noise beds
p = P('Tape Hiss Bed', FXC).noise(0.7, 0.85).off('a', 'b', 'c').f1('SVF HP12', 1500).f2('SVF LP12', 11000).aenv(1.5, 2, 1, 4)
p.tape(0.1, 0.1, 0.2, 0.6, 1.0).lfo(1, 0.07, 'drift').mod('lfo1', 'noise', 0.1) \
 .macro(1, 'Hiss').mod('m1', 'noise', 0.5).macro(2, 'Brightness').mod('m2', 'f2cut', 0.5).macro(3, 'Flutter').mod('m3', 'fxwow', 0.5) \
 .nochorus().tags('noise', 'bed', 'tape', 'background', 'detail')
X(p, vib=0, press=[('noise', 0.3)], slide=[('f2cut', 0.4)])

p = P('Room Tone Air', FXC).noise(0.6, 0.3).off('a', 'b', 'c').f1('SVF LP12', 2500, res=0.0, key=0).f2('SVF HP12', 60).aenv(2, 2, 1, 5)
p.lfo(1, 0.05, 'drift').mod('lfo1', 'f1cut', 0.25).mod('lfo1', 'noise', 0.1) \
 .macro(1, 'Air').mod('m1', 'noise', 0.5).macro(2, 'Room Size').mod('m2', 'f1cut', -0.5).macro(3, 'Movement').mod('m3', 'lfo1rate', 0.5) \
 .nochorus().width(1.6).tags('noise', 'room', 'background', 'detail', 'bed')
X(p, vib=0, press=[('f1cut', 0.3)], slide=[('f1cut', 0.3)])

p = P('Vinyl Dust Bed', FXC).gr('a', 'fixed', sample='Textures/Vinyl Crackle Bed', pos=0.3, scan=0.02, size=0.5, density=16, spray=0.5, level=0.7)
p.noise(0.1, 0.7).off('b', 'c').f1('SVF BP12', 2500, res=0.05, key=0).aenv(0.5, 2, 1, 3) \
 .macro(1, 'Crackle').mod('m1', 'alevel', 0.5).macro(2, 'Hiss').mod('m2', 'noise', 0.3).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .nochorus().tags('vinyl', 'crackle', 'lo-fi', 'background', 'detail')
X(p, vib=0, press=[('alevel', 0.3)], slide=[('f1cut', 0.4)])

p = P('Electric Field Bed', FXC).gr('a', 'fixed', sample='Drones/Electric Hum', pos=0.3, size=0.8, density=10, level=0.6)
p.noise(0.15, 0.9).off('b', 'c').f1('SVF LP12', 6000, res=0.05, key=0).aenv(1, 2, 1, 4) \
 .lfo(1, 0.3, 'sh', smooth=0.5).mod('lfo1', 'noise', 0.15) \
 .macro(1, 'Hum').mod('m1', 'alevel', 0.5).macro(2, 'Sizzle').mod('m2', 'noise', 0.4).macro(3, 'Tone').mod('m3', 'f1cut', 0.5) \
 .nochorus().tags('hum', 'electric', 'background', 'detail', 'bed')
X(p, vib=0, press=[('noise', 0.3)], slide=[('f1cut', 0.4)])

p = P('Distant Wind Bed', FXC).gr('a', 'fixed', sample='Nature/Canyon Wind', pos=0.3, size=1.0, density=8, level=0.6)
p.noise(0.2, 0.2).off('b', 'c').f1('SVF LP12', 1800, res=0.1, key=0).aenv(2, 2, 1, 6) \
 .lfo(1, 0.04, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'noise', 0.15) \
 .macro(1, 'Wind').mod('m1', 'alevel', 0.5).macro(2, 'Gust').mod('m2', 'f1cut', 0.5).macro(3, 'Noise').mod('m3', 'noise', 0.4) \
 .nochorus().width(1.5).tags('wind', 'background', 'detail', 'bed')
X(p, vib=0, press=[('f1cut', 0.35)], slide=[('noise', 0.3)])

p = P('Rain Window Bed', FXC).gr('a', 'fixed', sample='Nature/Rain Light', pos=0.3, size=0.6, density=14, level=0.6)
p.gr('b', 'fixed', sample='Nature/Rain on Leaves', pos=0.4, size=0.7, density=10, level=0.35).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0).aenv(1.5, 2, 1, 5) \
 .macro(1, 'Drops').mod('m1', 'alevel', 0.5).macro(2, 'Downpour').mod('m2', 'blevel', 0.5).macro(3, 'Muffle').mod('m3', 'f1cut', -0.6) \
 .nochorus().width(1.4).tags('rain', 'background', 'detail', 'bed')
X(p, vib=0, press=[('blevel', 0.4)], slide=[('f1cut', 0.4)])

p = P('Forest Night Bed', FXC).gr('a', 'fixed', sample='Nature/Night Crickets', pos=0.3, size=0.7, density=10, level=0.55)
p.gr('b', 'fixed', sample='Organic/Leaves Rustle', pos=0.3, size=0.6, density=8, level=0.3).off('c') \
 .f1('SVF LP12', 9000, res=0.05, key=0).aenv(1.5, 2, 1, 5) \
 .macro(1, 'Crickets').mod('m1', 'alevel', 0.5).macro(2, 'Leaves').mod('m2', 'blevel', 0.5).macro(3, 'Distance').mod('m3', 'f1cut', -0.6) \
 .nochorus().width(1.5).tags('night', 'forest', 'background', 'detail', 'bed')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('f1cut', 0.4)])

p = P('Ocean Bed', FXC).gr('a', 'fixed', sample='Nature/Ocean Swell', pos=0.3, size=1.3, density=6, spray=0.4, level=0.65)
p.off('b', 'c').f1('SVF LP12', 6000, res=0.05, key=0).aenv(2, 2, 1, 6) \
 .macro(1, 'Waves').mod('m1', 'alevel', 0.5).macro(2, 'Distance').mod('m2', 'f1cut', -0.6).macro(3, 'Grain').mod('m3', 'at2', -0.4) \
 .nochorus().width(1.5).tags('ocean', 'waves', 'background', 'detail', 'bed')
X(p, vib=0, press=[('alevel', 0.3)], slide=[('f1cut', 0.4)])

p = P('Fireplace Bed', FXC).gr('a', 'fixed', sample='Nature/Fire Crackle', pos=0.3, size=0.4, density=16, level=0.65)
p.off('b', 'c').f1('SVF LP12', 8000, res=0.05, key=0).aenv(1, 2, 1, 4) \
 .macro(1, 'Fire').mod('m1', 'alevel', 0.5).macro(2, 'Warmth').mod('m2', 'f1cut', -0.5).macro(3, 'Pops').mod('m3', 'at3', 0.5) \
 .nochorus().tags('fire', 'cozy', 'background', 'detail', 'bed')
X(p, vib=0, press=[('at3', 0.3)], slide=[('f1cut', 0.4)])

# ------------------------------------------------------------------------------------------------ resonant details
p = P('Ghost Resonator Tones', FXC).noise(0.6, 0.4).off('a', 'b', 'c').f1('SVF Peak', 800, res=0.92, key=1.0).aenv(0.3, 3, 0.6, 6)
p.lfo(1, 0.05, 'drift', 'voice').mod('lfo1', 'f1cut', 0.03) \
 .macro(1, 'Ring').mod('m1', 'f1res', 0.08).macro(2, 'Noise').mod('m2', 'noise', 0.4).macro(3, 'Pitch Drift').mod('m3', 'f1cut', 0.05) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('resonant', 'pitched-noise', 'ghost', 'detail')
X(p, vib=0.008, press=[('noise', 0.3)], slide=[('f1res', 0.05)])

p = P('Comb Ping Detail', FXC).noise(1.0, 0.6).off('a', 'b', 'c').f1('Comb +', 400, res=0.85, key=1.0).f2('SVF LP12', 7000).aenv(0.001, 0.25, 0.0, 1.2).curve(a=1)
p.macro(1, 'Ring').mod('m1', 'f1res', 0.1).macro(2, 'Tone').mod('m2', 'f2cut', 0.5).macro(3, 'Length').mod('m3', 'amp', 0.2) \
 .nochorus().tags('comb', 'ping', 'resonant', 'one-shot', 'detail')
X(p, vib=0, press=[('f1res', 0.08)], slide=[('f2cut', 0.4)])

p = P('Glass Ping Detail', FXC).ph('a', 'Glass Bowl', 'Strike', decay=3, damp=0.2, hard=0.8, level=0.7).off('b', 'c')
p.f1('SVF HP12', 300).aenv(0.001, 3, 0.0, 3).curve(a=1) \
 .macro(1, 'Hardness').mod('m1', 'at2', 0.5).macro(2, 'Decay').mod('m2', 'at3', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5) \
 .nochorus().tags('glass', 'ping', 'one-shot', 'detail')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Metal Resonance Hit', FXC).ph('a', 'Plate', 'Strike', decay=6, damp=0.15, hard=0.9, couple=0.4, level=0.7)
p.ph('b', 'Bell', 'Osc Input', decay=5, damp=0.2, hard=0.6, oct=-1, level=0.3).off('c').aenv(0.001, 6, 0.0, 6).curve(a=1) \
 .macro(1, 'Hardness').mod('m1', 'at2', 0.5).macro(2, 'Sympathy').mod('m2', 'blevel', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .nochorus().tags('metal', 'impact', 'resonant', 'one-shot', 'detail')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Pipe Resonance Detail', FXC).ph('a', 'Tube', 'Breath', decay=2, damp=0.3, hard=0.4, level=0.6).off('b', 'c')
p.noise(0.08, 0.3).f1('SVF LP12', 5000).aenv(0.3, 2, 0.6, 3) \
 .macro(1, 'Breath').mod('m1', 'at2', 0.5).macro(2, 'Air').mod('m2', 'noise', 0.3).macro(3, 'Length').mod('m3', 'at3', 0.5) \
 .nochorus().tags('pipe', 'breath', 'resonant', 'detail')
X(p, vib=0.01, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Formant Whisper Tones', FXC).noise(1.0, 0.5).off('a', 'b', 'c').f1('Vowel', 900, res=0.7, key=0.6, shape=0.3).aenv(0.4, 2, 0.7, 3)
p.lfo(1, 0.3, 'sh', 'voice', smooth=0.7).mod('lfo1', 'f1shape', 0.4) \
 .macro(1, 'Vowel').mod('m1', 'f1shape', 0.6).macro(2, 'Chatter').mod('m2', 'lfo1rate', 0.6).macro(3, 'Resonance').mod('m3', 'f1res', 0.2) \
 .chorus('chorus', 0.15, 0.05, 0.4).tags('whisper', 'formant', 'eerie', 'detail')
X(p, vib=0, press=[('f1shape', 0.4)], slide=[('f1cut', 0.3)])

p = P('Singing Wire Detail', FXC).ph('a', 'Waveguide', 'Bow', decay=8, damp=0.05, hard=0.6, level=0.6).off('b', 'c')
p.f1('SVF HP12', 200).aenv(0.8, 3, 0.7, 4).lfo(1, 0.07, 'drift', 'voice').mod('lfo1', 'at2', 0.3) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).macro(2, 'Damping').mod('m2', 'at4', 0.5).macro(3, 'Pitch').mod('m3', 'pitch', 0.1) \
 .chorus('chorus', 0.12, 0.05, 0.3).tags('wire', 'bowed', 'resonant', 'eerie', 'detail')
X(p, vib=0.01, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Tinnitus Sine', FXC).an('a', 'sine', oct=2, level=0.35, slop=0.0).off('b', 'c').aenv(3, 2, 1, 5)
p.lfo(1, 0.02, 'drift').mod('lfo1', 'apitch', 0.002).mod('lfo1', 'alevel', 0.2) \
 .macro(1, 'Level').mod('m1', 'alevel', 0.5).macro(2, 'Beat').mod('m2', 'apitch', 0.01).macro(3, 'Pan').mod('m3', 'pan', 0.6) \
 .nochorus().tags('sine', 'tension', 'detail', 'tone')
X(p, vib=0, press=[('alevel', 0.3)], slide=[('pan', 0.5)])

# ------------------------------------------------------------------------------------------------ risers / falls / swells
p = P('Spectral Riser', FXC).wt('a', 'Spectral/Partial Rise', pos=0.0, blend='spectral', voices=5, detune=20, level=0.5)
p.noise(0.3, 0.6).off('b', 'c').f1('SVF LP12', 400, res=0.2, key=0.2).aenv(0.01, 2, 1, 2) \
 .mseg(1, [(0, 0.0, 0), (8, 1.0, 0.5)], loop=(1, 1), mode='env').mod('mseg1', 'at1', 1.0).mod('mseg1', 'f1cut', 0.9).mod('mseg1', 'pitch', 0.5).mod('mseg1', 'noise', 0.3) \
 .macro(1, 'Length').mod('m1', 'mseg1rate', -0.6).macro(2, 'Pitch Rise').mod('m2', 'pitch', 0.4).macro(3, 'Noise').mod('m3', 'noise', 0.4) \
 .chorus('ensemble', 0.2, 0.2, 0.5).width(1.5).tags('riser', 'build', 'transition', 'mseg', 'detail')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('mseg1rate', 0.5)])

p = P('Noise Sweep Up', FXC).noise(0.8, 0.5).off('a', 'b', 'c').f1('SVF BP12', 300, res=0.5, key=0).aenv(0.01, 2, 1, 1.5)
p.mseg(1, [(0, 0.0, 0), (6, 1.0, 0.6)], loop=(1, 1), mode='env').mod('mseg1', 'f1cut', 1.0).mod('mseg1', 'f1res', 0.2) \
 .macro(1, 'Length').mod('m1', 'mseg1rate', -0.6).macro(2, 'Resonance').mod('m2', 'f1res', 0.4).macro(3, 'Width').mod('m3', 'fxwidth', 0.5) \
 .phaser(0.4, 0.2, 0.7, 0.5).nochorus().width(1.5).tags('riser', 'noise', 'sweep', 'transition', 'detail')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('mseg1rate', 0.5)])

p = P('Sub Downlifter', FXC).an('a', 'sine', level=0.7).an('b', 'tri', level=0.25).off('c').aenv(0.001, 4, 0.0, 4).curve(a=1)
p.mseg(1, [(0, 1.0, 0), (3, 0.0, -0.5)], loop=(1, 1), mode='env').mod('mseg1', 'pitch', 1.0).f1('SVF LP12', 1500) \
 .macro(1, 'Drop').mod('m1', 'pitch', 0.5).macro(2, 'Length').mod('m2', 'mseg1rate', -0.5).macro(3, 'Grit').mod('m3', 'fxdrive', 0.5) \
 .drive(0.1).nochorus().tags('downlifter', 'sub', 'drop', 'transition', 'one-shot', 'detail')
X(p, vib=0, press=[('fxdrive', 0.3)], slide=[('mseg1rate', 0.4)])

p = P('Reverse Swell Pad', FXC).add('a', 'Choir Ah', 'Glass', morph=0.4, partials=56, shim=0.3, level=0.55).off('b', 'c')
p.f1('SVF LP12', 1200, res=0.1, key=0.2).aenv(4, 0.1, 1.0, 0.15).curve(a=-1).fenv(4, 0.1, 1.0, 0.2).set('f1Env', 0.6) \
 .macro(1, 'Swell Time').mod('m1', 'amp', 0.1).macro(2, 'Brightness').mod('m2', 'f1cut', 0.5).macro(3, 'Glass').mod('m3', 'at1', 0.5) \
 .chorus('ensemble', 0.25, 0.08, 0.5).tags('reverse', 'swell', 'transition', 'detail')
X(p, vib=0, press=[('f1cut', 0.3)], slide=[('at1', 0.5)])

p = P('Tension Riser', FXC).an('a', 'saw', sync=0, voices=5, detune=25, level=0.45).an('b', 'saw', semi=1, voices=3, detune=20, level=0.3).off('c')
p.f1('Ladder LP24', 500, res=0.35, key=0.3).aenv(0.5, 2, 1, 2) \
 .mseg(1, [(0, 0.0, 0), (10, 1.0, 0.3)], loop=(1, 1), mode='env').mod('mseg1', 'f1cut', 0.8).mod('mseg1', 'at2', 0.6).mod('mseg1', 'adetune', 0.4) \
 .lfo(1, 2, 'sine', 'voice').mod('lfo1', 'pitch', 0.01, via='mseg1') \
 .macro(1, 'Length').mod('m1', 'mseg1rate', -0.6).macro(2, 'Dissonance').mod('m2', 'blevel', 0.5).macro(3, 'Sync').mod('m3', 'at2', 0.5) \
 .chorus('ensemble', 0.25, 0.15, 0.5).tags('riser', 'tension', 'cinematic', 'transition', 'detail')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('mseg1rate', 0.5)])

p = P('Wind Gust Whoosh', FXC).noise(0.8, 0.4).off('a', 'b', 'c').f1('SVF BP12', 600, res=0.3, key=0).aenv(1.2, 1.5, 0.0, 1.5).curve(a=-0.4)
p.fenv(1.2, 1.5, 0.0, 1.5).set('f1Env', 0.6).lfo(1, 0.4, 'sine').mod('lfo1', 'pan', 0.6) \
 .macro(1, 'Speed').mod('m1', 'amp', 0.1).macro(2, 'Brightness').mod('m2', 'f1cut', 0.5).macro(3, 'Pan Sweep').mod('m3', 'pan', 0.4) \
 .nochorus().width(1.5).tags('whoosh', 'wind', 'transition', 'one-shot', 'detail')
X(p, vib=0, press=[('f1cut', 0.3)], slide=[('pan', 0.5)])

p = P('Breath Swell Detail', FXC).gr('a', 'fixed', sample='Voices/Breath Cycle', pos=0.17, scan=0.0, size=0.5, density=18, spray=0.05, level=0.8).off('b', 'c')
p.f1('SVF LP12', 6000).aenv(1.5, 2, 0.6, 2).curve(a=-0.5) \
 .macro(1, 'Breath').mod('m1', 'alevel', 0.5).macro(2, 'Air').mod('m2', 'f1cut', 0.5).macro(3, 'Position').mod('m3', 'at1', 0.5) \
 .nochorus().tags('breath', 'human', 'detail', 'one-shot')
X(p, vib=0, press=[('alevel', 0.3)], slide=[('at1', 0.5)])

# ------------------------------------------------------------------------------------------------ impacts
p = P('Sub Boom Impact', FXC).an('a', 'sine', oct=-1, level=0.8).noise(0.3, 0.1).off('b', 'c').aenv(0.001, 3, 0.0, 3).curve(a=1)
p.mseg(1, [(0, 1.0, 0), (0.4, 0.0, -0.7)], loop=(1, 1), mode='env').mod('mseg1', 'pitch', 0.8).mod('mseg1', 'noise', 0.4) \
 .f1('SVF LP12', 2000).macro(1, 'Punch').mod('m1', 'pitch', 0.4).macro(2, 'Noise').mod('m2', 'noise', 0.4).macro(3, 'Drive').mod('m3', 'fxdrive', 0.6) \
 .drive(0.2).nochorus().tags('impact', 'boom', 'sub', 'cinematic', 'one-shot', 'detail')
X(p, vib=0, press=[('fxdrive', 0.3)], slide=[('f1cut', 0.4)])

p = P('Bowl Impact Bloom', FXC).ph('a', 'Glass Bowl', 'Strike', decay=12, damp=0.1, hard=0.85, couple=0.5, level=0.6)
p.ph('b', 'Plate', 'Strike', decay=8, damp=0.15, hard=0.7, oct=-1, level=0.35).off('c').aenv(0.001, 10, 0.0, 10).curve(a=1) \
 .macro(1, 'Hardness').mod('m1', 'at2', 0.5).macro(2, 'Plate').mod('m2', 'blevel', 0.5).macro(3, 'Decay').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('impact', 'bowl', 'cinematic', 'one-shot', 'detail')
X(p, vib=0, press=[('at4', -0.3)], slide=[('at1', 0.5)])

p = P('Thunder Roll One-Shot', FXC).gr('a', 'fixed', sample='Nature/Distant Thunder', pos=0.18, scan=0.06, size=0.9, density=12, spray=0.2, level=0.7)
p.an('b', 'sine', oct=-2, level=0.3).off('c').aenv(0.01, 5, 0.0, 5).curve(a=0.6).f1('SVF LP12', 3000) \
 .macro(1, 'Thunder').mod('m1', 'alevel', 0.5).macro(2, 'Sub').mod('m2', 'blevel', 0.5).macro(3, 'Distance').mod('m3', 'f1cut', -0.5) \
 .nochorus().tags('thunder', 'impact', 'nature', 'one-shot', 'detail')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('f1cut', 0.4)])

p = P('Wood Knock Detail', FXC).ph('a', 'Bar', 'Strike', decay=0.6, damp=0.5, hard=0.7, level=0.7).off('b', 'c').aenv(0.001, 0.6, 0.0, 0.6).curve(a=1)
p.macro(1, 'Hardness').mod('m1', 'at2', 0.5).macro(2, 'Ring').mod('m2', 'at3', 0.5).macro(3, 'Position').mod('m3', 'at1', 0.5) \
 .nochorus().tags('wood', 'knock', 'percussion', 'one-shot', 'detail')
X(p, vib=0, press=[('at2', 0.3)], slide=[('at1', 0.5)])

p = P('Water Drop Ping', FXC).ph('a', 'Tube', 'Strike', decay=0.5, damp=0.6, hard=0.3, level=0.7).off('b', 'c').aenv(0.001, 0.5, 0.0, 0.5).curve(a=1)
p.mseg(1, [(0, 0.0, 0), (0.15, 1.0, -0.5)], loop=(1, 1), mode='env').mod('mseg1', 'pitch', 0.25) \
 .macro(1, 'Drop Size').mod('m1', 'pitch', -0.3).macro(2, 'Ring').mod('m2', 'at3', 0.5).macro(3, 'Blip').mod('m3', 'at2', 0.5) \
 .nochorus().tags('water', 'drop', 'ping', 'one-shot', 'detail')
X(p, vib=0, press=[('at2', 0.3)], slide=[('pitch', 0.2)])

# ------------------------------------------------------------------------------------------------ glitch / sci-fi
p = P('Data Chirps', FXC).an('a', 'pulse', pw=0.2, oct=1, level=0.45).off('b', 'c').f1('SVF BP12', 2500, res=0.3, key=0.5).aenv(0.001, 0.08, 0.0, 0.1).curve(a=1)
p.arp('random', 3, '1/32', 0.4, hold=True).lfo(1, 8, 'sh', 'voice').mod('lfo1', 'pitch', 0.2).mod('lfo1', 'f1cut', 0.4) \
 .macro(1, 'Chaos').mod('m1', 'pitch', 0.2).macro(2, 'Band').mod('m2', 'f1cut', 0.5).macro(3, 'Bits').mod('m3', 'fxcrush', 0.6) \
 .crush(8, 3, 0.3).nochorus().tags('glitch', 'data', 'computer', 'arp', 'detail')
X(p, vib=0, press=[('fxcrush', 0.4)], slide=[('f1cut', 0.4)])

p = P('Alien Chatter', FXC).an('a', 'saw', level=0.5).off('b', 'c').f1('Vowel', 900, res=0.6, key=0.4, shape=0.5).aenv(0.01, 0.3, 0.6, 0.3)
p.lfo(1, 7, 'sh', 'voice').lfo(2, 3, 'sh', 'voice', smooth=0.4).mod('lfo1', 'f1shape', 0.6).mod('lfo2', 'pitch', 0.1) \
 .macro(1, 'Speech Rate').mod('m1', 'lfo1rate', 0.6).macro(2, 'Inflection').mod('m2', 'pitch', 0.1).macro(3, 'Mouth').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.15, 0.2, 0.4).tags('alien', 'voice', 'sci-fi', 'detail')
X(p, vib=0, press=[('lfo1rate', 0.4)], slide=[('f1shape', 0.5)])

p = P('UFO Hover', FXC).an('a', 'sine', level=0.5).an('b', 'sine', semi=7, level=0.0).fm('a', 'b', 0.5, 'fm').off('c')
p.aenv(1, 2, 1, 3).lfo(1, 0.4, 'sine').lfo(2, 6, 'sine').mod('lfo1', 'pitch', 0.05).mod('lfo2', 'afm', 0.2).mod('lfo1', 'pan', 0.4) \
 .macro(1, 'Wobble').mod('m1', 'lfo2rate', 0.5).macro(2, 'FM').mod('m2', 'afm', 0.4).macro(3, 'Hover').mod('m3', 'pitch', 0.1) \
 .phaser(0.3, 0.3, 0.6, 0.4).nochorus().tags('ufo', 'sci-fi', 'fm', 'detail')
X(p, vib=0, press=[('afm', 0.3)], slide=[('lfo2rate', 0.5)])

p = P('Laser Zap', FXC).an('a', 'saw', oct=1, level=0.5).off('b', 'c').f1('SVF LP12', 6000, res=0.3).aenv(0.001, 0.35, 0.0, 0.3).curve(a=1)
p.mseg(1, [(0, 1.0, 0), (0.3, 0.0, -0.6)], loop=(1, 1), mode='env').mod('mseg1', 'pitch', 1.0).mod('mseg1', 'f1cut', 0.4) \
 .macro(1, 'Zap Depth').mod('m1', 'pitch', 0.5).macro(2, 'Speed').mod('m2', 'mseg1rate', 0.6).macro(3, 'Resonance').mod('m3', 'f1res', 0.4) \
 .nochorus().tags('laser', 'sci-fi', 'retro', 'one-shot', 'detail')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('mseg1rate', 0.5)])

p = P('Morse Code Signal', FXC).an('a', 'sine', oct=1, level=0.45).noise(0.1, 0.7).off('b', 'c').f1('SVF BP12', 1500, res=0.2, key=0.2).aenv(0.003, 0.1, 0.9, 0.02).curve(a=1)
p.arp('order', 1, '1/16', 0.35, hold=True).lfo(1, 0.5, 'sh').mod('lfo1', 'alevel', 0.3) \
 .macro(1, 'Signal').mod('m1', 'alevel', 0.5).macro(2, 'Static').mod('m2', 'noise', 0.4).macro(3, 'Tune').mod('m3', 'f1cut', 0.4) \
 .nochorus().tags('morse', 'radio', 'signal', 'arp', 'detail')
X(p, vib=0, press=[('noise', 0.3)], slide=[('f1cut', 0.4)])

p = P('Computer Memory Beeps', FXC).an('a', 'pulse', pw=0.5, oct=1, level=0.4).off('b', 'c').f1('SVF LP12', 4000).aenv(0.001, 0.12, 0.0, 0.08).curve(a=1)
p.arp('random', 3, '1/16', 0.5, hold=True) \
 .macro(1, 'Pulse').mod('m1', 'at1', 0.5).macro(2, 'Tone').mod('m2', 'f1cut', 0.5).macro(3, 'Crush').mod('m3', 'fxcrush', 0.5) \
 .crush(10, 2, 0.2).nochorus().tags('computer', 'retro', 'beeps', 'arp', 'detail')
X(p, vib=0, press=[('fxcrush', 0.4)], slide=[('at1', 0.5)])

p = P('Bit Rain Glitch', FXC).gr('a', 'fixed', sample='Textures/Data Stream', pos=0.3, size=0.03, density=150, spray=1.0, level=0.5)
p.off('b', 'c').f1('SVF HP12', 600).aenv(0.2, 2, 1, 2).crush(6, 4, 0.4) \
 .macro(1, 'Density').mod('m1', 'at3', 0.5).macro(2, 'Bits').mod('m2', 'fxcrush', 0.5).macro(3, 'Spray').mod('m3', 'at4', 0.5) \
 .nochorus().width(1.5).tags('glitch', 'digital', 'rain', 'detail')
X(p, vib=0, press=[('at3', 0.4)], slide=[('fxcrush', 0.4)])

p = P('Shortwave Tuning Detail', FXC).gr('a', 'fixed', sample='Drones/Shortwave Radio', pos=0.3, scan=0.08, size=0.3, density=20, level=0.6)
p.off('b', 'c').f1('SVF BP12', 1500, res=0.3, key=0).aenv(0.2, 2, 1, 2).lfo(1, 0.15, 'drift').mod('lfo1', 'f1cut', 0.5) \
 .macro(1, 'Tuning').mod('m1', 'at1', 0.6).macro(2, 'Band').mod('m2', 'f1cut', 0.5).macro(3, 'Shift').mod('m3', 'fxshiftmix', 0.5) \
 .shifter(0.2, 0.0, 0.2, 0.5).nochorus().tags('radio', 'tuning', 'lo-fi', 'detail')
X(p, vib=0, press=[('fxshiftmix', 0.4)], slide=[('at1', 0.6)])

# ------------------------------------------------------------------------------------------------ sprinkles & tails
p = P('Chime Sprinkle', FXC).ph('a', 'Bell', 'Strike', decay=3, damp=0.2, hard=0.7, level=0.55).off('b', 'c').f1('SVF HP12', 400).aenv(0.001, 3, 0.0, 3).curve(a=1)
p.arp('random', 3, '1/32', 0.3, hold=True).lfo(1, 0.3, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Sparkle').mod('m1', 'at2', 0.5).macro(2, 'Position').mod('m2', 'at1', 0.5).macro(3, 'Ring').mod('m3', 'at3', 0.5) \
 .chorus('chorus', 0.12, 0.08, 0.3).width(1.5).tags('chimes', 'sprinkle', 'sparkle', 'arp', 'detail')
X(p, vib=0, press=[('at2', 0.3)], slide=[('at1', 0.5)])

p = P('Bell Shimmer Tail', FXC).add('a', 'Bell', 'Glass', morph=0.5, partials=80, stretch=0.2, shim=0.6, srate=0.5, level=0.55).off('b', 'c')
p.f1('SVF HP12', 500).aenv(0.01, 8, 0.0, 8).curve(a=0.8) \
 .macro(1, 'Shimmer').mod('m1', 'at2', 0.5).macro(2, 'Stretch').mod('m2', 'at3', 0.5).macro(3, 'Glass').mod('m3', 'at1', 0.5) \
 .chorus('ensemble', 0.2, 0.06, 0.5).tags('bell', 'tail', 'shimmer', 'one-shot', 'detail')
X(p, vib=0, press=[('at2', 0.3)], slide=[('at1', 0.5)])

p = P('Granular Freeze Scatter', FXC).gr('a', 'keyed', sample='Voices/Choir Cloud', pos=0.4, scan=0.0, size=0.04, density=160, spray=0.8, prand=30, level=0.5)
p.off('b', 'c').f1('SVF LP12', 9000).aenv(0.5, 2, 1, 3).width(1.6) \
 .macro(1, 'Scatter').mod('m1', 'at4', 0.5).macro(2, 'Pitch Spray').mod('m2', 'at3', 0.5).macro(3, 'Position').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.15, 0.1, 0.4).tags('granular', 'freeze', 'scatter', 'detail')
X(p, vib=0, press=[('at4', 0.4)], slide=[('at1', 0.6)])

p = P('Reverse Grain Swell', FXC).gr('a', 'keyed', sample='Metal/Singing Bowl Pair', pos=0.5, scan=-0.05, size=0.5, density=20, spray=0.3, rev=1.0, level=0.6)
p.off('b', 'c').f1('SVF LP12', 7000).aenv(3, 0.1, 1.0, 0.3).curve(a=-1) \
 .macro(1, 'Scan').mod('m1', 'at1', 0.5).macro(2, 'Grain Size').mod('m2', 'at2', 0.5).macro(3, 'Brightness').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.05, 0.4).tags('reverse', 'swell', 'granular', 'transition', 'detail')
X(p, vib=0, press=[('f1cut', 0.3)], slide=[('at1', 0.5)])

p = P('Ice Crack Details', FXC).gr('a', 'fixed', sample='Nature/Ice Creaks', pos=0.3, size=0.3, density=8, spray=0.9, level=0.65)
p.ph('b', 'Bar', 'Rain', decay=1.2, damp=0.3, hard=0.8, spread=1.0, oct=1, level=0.25).off('c').f1('SVF HP12', 300).aenv(0.2, 2, 1, 3) \
 .macro(1, 'Cracks').mod('m1', 'alevel', 0.5).macro(2, 'Tinkle').mod('m2', 'blevel', 0.5).macro(3, 'Density').mod('m3', 'at3', 0.5) \
 .nochorus().width(1.5).tags('ice', 'crack', 'cold', 'detail')
X(p, vib=0, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Wood Creak Details', FXC).gr('a', 'fixed', sample='Organic/Wood Creak', pos=0.3, size=0.6, density=5, spray=0.8, level=0.7)
p.off('b', 'c').f1('SVF LP12', 6000).aenv(0.2, 2, 1, 3) \
 .macro(1, 'Creaks').mod('m1', 'at3', 0.5).macro(2, 'Size').mod('m2', 'at2', 0.5).macro(3, 'Tone').mod('m3', 'f1cut', 0.5) \
 .nochorus().tags('wood', 'creak', 'house', 'detail')
X(p, vib=0, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Bamboo Wind Chimes', FXC).gr('a', 'fixed', sample='Organic/Bamboo Knocks', pos=0.3, size=0.3, density=6, spray=0.9, level=0.6)
p.gr('b', 'fixed', sample='Metal/Wind Chimes', pos=0.3, size=0.6, density=4, spray=0.9, level=0.35).off('c').aenv(0.2, 2, 1, 4) \
 .macro(1, 'Bamboo').mod('m1', 'alevel', 0.5).macro(2, 'Chimes').mod('m2', 'blevel', 0.5).macro(3, 'Breeze').mod('m3', 'at3', 0.5) \
 .nochorus().width(1.5).tags('chimes', 'bamboo', 'garden', 'detail')
X(p, vib=0, press=[('at3', 0.4)], slide=[('at1', 0.5)])

p = P('Dark Matter Pulse', FXC).an('a', 'sine', oct=-2, level=0.7).off('b', 'c').aenv(0.01, 0.6, 0.0, 0.6).curve(a=0.8)
p.arp('order', 1, '1/2', 0.3, hold=True) \
 .mseg(1, [(0, 1.0, 0), (0.15, 0.0, -0.5)], loop=(1, 1), mode='env').mod('mseg1', 'pitch', 0.3) \
 .macro(1, 'Punch').mod('m1', 'pitch', 0.3).macro(2, 'Drive').mod('m2', 'fxdrive', 0.6).macro(3, 'Tone').mod('m3', 'cut', 0.5) \
 .drive(0.15).nochorus().tags('pulse', 'heartbeat', 'sub', 'cinematic', 'arp', 'detail')
X(p, vib=0, press=[('fxdrive', 0.3)], slide=[('pitch', 0.2)])

p = P('Heartbeat Low', FXC).an('a', 'sine', oct=-2, level=0.7).off('b', 'c').aenv(0.005, 0.3, 0.0, 0.3).curve(a=1)
p.mseg(1, [(0, 1.0, 0), (0.1, 0.0, -0.5)], loop=(1, 1), mode='env').mod('mseg1', 'pitch', 0.2) \
 .arp('order', 1, '1/4', 0.2, hold=True) \
 .macro(1, 'Thump').mod('m1', 'pitch', 0.2).macro(2, 'Warmth').mod('m2', 'fxdrive', 0.5).macro(3, 'Muffle').mod('m3', 'cut', -0.4) \
 .nochorus().tags('heartbeat', 'pulse', 'tension', 'arp', 'detail')
X(p, vib=0, press=[('fxdrive', 0.3)], slide=[('pitch', 0.2)])
