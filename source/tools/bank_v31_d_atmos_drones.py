# Driftfield 3.1 bank, part D: ATMOSPHERES + DRONES (Roach / Corbacho heavy)
A = 'Atmospheres'
DR = 'Drones'

# ------------------------------------------------------------------------------------------------ ATMOSPHERES
p = P('Origins Unknown', A).wt('a', 'Ambient/Structures from Silence', pos=0.2, blend='spectral', voices=4, detune=9, level=0.5)
p.gr('b', 'keyed', sample='Voices/Overtone Drone', pos=0.3, size=0.9, density=10, spray=0.3, level=0.3, oct=-1).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.008, 'sine').lfo(2, 0.021, 'drift').mod('lfo1', 'at1', 0.6).mod('lfo2', 'bt1', 0.4) \
 .macro(1, 'Silence').mod('m1', 'at1', 0.5).macro(2, 'Overtones').mod('m2', 'blevel', 0.5).macro(3, 'Light').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('roach', 'structures', 'vast', 'slow')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Lunar Seas', A).wt('a', 'Drift Spectra/Drift Tide', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.gr('b', 'fixed', sample='Nature/Ocean Swell', pos=0.3, size=1.2, density=6, level=0.3) \
 .add('c', 'Glass', 'Octaves', morph=0.5, partials=48, shim=0.3, oct=1, level=0.2) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.0133, shape=TIDE).mod('lfo1', 'at1', 0.5).mod('lfo1', 'blevel', 0.3) \
 .macro(1, 'Seas').mod('m1', 'at1', 0.5).macro(2, 'Waves').mod('m2', 'blevel', 0.5).macro(3, 'Moonlight').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('moon', 'ocean', 'corbacho', 'nocturnal')
X(p, press=[('clevel', 0.3)], slide=[('at1', 0.5)])

p = P('Empty Cathedral Air', A).noise(0.25, 0.5).an('a', 'sine', level=0.2)
p.wt('b', 'Organs/Organ Celeste', pos=0.3, voices=3, detune=7, level=0.3).off('c') \
 .f1('Dual Bandpass', 1100, res=0.5, shape=0.6, key=0.5).f2('SVF LP12', 5000).route('parallel', 0.4).aenv(*VLONG) \
 .lfo(1, 0.012, 'drift').mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'noise', 0.15) \
 .macro(1, 'Air').mod('m1', 'noise', 0.5).macro(2, 'Organ Ghost').mod('m2', 'blevel', 0.5).macro(3, 'Resonance').mod('m3', 'f1res', 0.3) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('cathedral', 'air', 'empty', 'corbacho')
X(p, vib=0, press=[('f1res', 0.3)], slide=[('balance', 0.4)])

p = P('Valley of Ten Thousand Stones', A).wt('a', 'Ambient/Sacred Ground', pos=0.3, blend='spectral', voices=3, detune=8, level=0.5)
p.gr('b', 'fixed', sample='Organic/Stone Scrape', pos=0.3, size=0.7, density=8, level=0.25) \
 .ph('c', 'Bar', 'Bow', decay=12, damp=0.18, hard=0.4, couple=0.3, oct=-1, level=0.25) \
 .f1('SVF LP12', 4000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'ct2', 0.3) \
 .macro(1, 'Ground').mod('m1', 'at1', 0.5).macro(2, 'Stones').mod('m2', 'blevel', 0.5).macro(3, 'Bow').mod('m3', 'ct2', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('roach', 'ancient', 'stones', 'desert')
X(p, press=[('ct2', 0.4)], slide=[('at1', 0.5)])

p = P('Incandescent Fog', A).wt('a', 'Drone/Low Fog', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.wt('b', 'Drone/Ember Glow', pos=0.3, blend='spectral', voices=3, detune=8, oct=1, level=0.25).noise(0.08, 0.2).off('c') \
 .f1('SVF LP12', 4500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.015, 'drift').lfo(2, 0.04, 'sine').mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.4) \
 .macro(1, 'Fog').mod('m1', 'at1', 0.5).macro(2, 'Glow').mod('m2', 'blevel', 0.5).macro(3, 'Breath').mod('m3', 'noise', 0.3) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('fog', 'warm', 'corbacho', 'glow')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Celestial Navigation', A).add('a', 'Octaves', 'Glass', morph=0.4, partials=80, shim=0.35, srate=0.1, pdet=8, level=0.5)
p.wt('b', 'Ambient/Night Sky Shimmer', pos=0.3, blend='spectral', voices=4, detune=10, oct=1, level=0.25).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*VLONG) \
 .mseg(1, [(0, 0.2, 0), (12, 0.8, 0.2), (12, 0.4, 0), (12, 1.0, 0.2), (12, 0.2, 0)], loop=(0, 4), mode='free').mod('mseg1', 'at1', 0.7) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Stars').mod('m1', 'at1', 0.5).macro(2, 'Night').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'at2', 0.4) \
 .chorus('ensemble', 0.25, 0.04, 0.5).width(1.5).tags('space', 'stars', 'corbacho', 'mseg')
X(p, press=[('at2', 0.35)], slide=[('at1', 0.5)])

p = P('Bowl of Light', A).ph('a', 'Glass Bowl', 'Bow', decay=20, damp=0.1, hard=0.3, couple=0.6, level=0.5)
p.add('b', 'Glass', 'Octaves', morph=0.5, partials=56, shim=0.4, srate=0.1, level=0.3).off('c') \
 .f1('SVF LP12', 8000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4).mod('lfo1', 'bt1', 0.4) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).macro(2, 'Light').mod('m2', 'blevel', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5) \
 .chorus('ensemble', 0.2, 0.04, 0.5).tags('glass', 'bowl', 'corbacho', 'healing')
X(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Dark Mesa Night', A).wt('a', 'Drone/Night Wind', pos=0.3, blend='spectral', voices=3, detune=8, level=0.5)
p.gr('b', 'fixed', sample='Nature/Night Crickets', pos=0.3, size=0.7, density=8, level=0.25) \
 .an('c', 'tri', oct=-2, slop=0.5, level=0.25).f1('SVF LP12', 3500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Wind').mod('m1', 'at1', 0.5).macro(2, 'Crickets').mod('m2', 'blevel', 0.5).macro(3, 'Earth').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('desert', 'night', 'roach', 'dark')
X(p, press=[('clevel', 0.3)], slide=[('at1', 0.5)])

p = P('Fragile Light', A).add('a', 'Glass', 'Triangle', morph=0.5, partials=40, shim=0.3, srate=0.12, level=0.5)
p.wt('b', 'Glass & Bell/Celesta Tines', pos=0.3, blend='spectral', oct=1, level=0.2).off('c') \
 .f1('SVF LP12', 9000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'sine').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Glass').mod('m1', 'at1', 0.5).macro(2, 'Celesta').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'at2', 0.4) \
 .chorus('ensemble', 0.2, 0.05, 0.4).tags('fragile', 'light', 'corbacho', 'delicate')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Heat Haze Horizon', A).wt('a', 'Ambient/Mirage Heat', pos=0.3, blend='spectral', voices=4, detune=10, level=0.5)
p.gr('b', 'fixed', sample='Organic/Sand Flow', pos=0.3, size=0.5, density=12, level=0.2).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG).tape(0.1, 0.5, 0.05, 0.7, 0.5) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Haze').mod('m1', 'at1', 0.5).macro(2, 'Sand').mod('m2', 'blevel', 0.5).macro(3, 'Wobble').mod('m3', 'fxwow', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('desert', 'heat', 'roach', 'shimmer')
X(p, press=[('fxwow', 0.3)], slide=[('at1', 0.5)])

p = P('Antarctic Dream', A).wt('a', 'Glass & Bell/Ice Shimmer', pos=0.3, blend='spectral', voices=4, detune=10, level=0.45)
p.gr('b', 'keyed', sample='Textures/Frozen Spectrum', pos=0.3, size=1.0, density=8, level=0.3) \
 .gr('c', 'fixed', sample='Nature/Ice Creaks', pos=0.3, size=0.6, density=5, level=0.2) \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Ice').mod('m1', 'at1', 0.5).macro(2, 'Spectrum').mod('m2', 'blevel', 0.5).macro(3, 'Creaks').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('ice', 'arctic', 'corbacho', 'glacial')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Spirit Catcher Night', A).add('a', 'Throat', 'Choir Ah', morph=0.4, partials=64, peak=10, peakamt=0.6, width=0.08, shim=0.2, level=0.5)
p.gr('b', 'fixed', sample='Voices/Whisper Air', pos=0.3, size=0.7, density=8, level=0.25).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at2', 0.4).mod('lfo1', 'at1', 0.3) \
 .macro(1, 'Spirit').mod('m1', 'at2', 0.5).macro(2, 'Whispers').mod('m2', 'blevel', 0.5).macro(3, 'Choir').mod('m3', 'at1', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('spirit', 'roach', 'shamanic', 'vocal')
X(p, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Low Cloud Ceiling', A).wt('a', 'Drift Spectra/Drift Nimbus', pos=0.3, blend='spectral', voices=4, detune=9, level=0.5, oct=-1)
p.an('b', 'saw', voices=3, detune=10, slop=0.4, level=0.25).off('c') \
 .f1('Ladder LP24', 900, res=0.15, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.012, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Clouds').mod('m1', 'at1', 0.5).macro(2, 'Ceiling').mod('m2', 'f1cut', 0.5).macro(3, 'Saws').mod('m3', 'blevel', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('overcast', 'dark', 'roach')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Eden Before Dawn', A).add('a', 'Organ', 'Choir Oo', morph=0.4, partials=56, shim=0.2, level=0.45)
p.gr('b', 'fixed', sample='Nature/Night Crickets', pos=0.3, size=0.6, density=8, level=0.2) \
 .gr('c', 'fixed', sample='Nature/Mountain Stream', pos=0.3, size=0.6, density=10, level=0.2) \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Hymn').mod('m1', 'at1', 0.5).macro(2, 'Night').mod('m2', 'blevel', 0.5).macro(3, 'Stream').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('garden', 'dawn', 'corbacho', 'pastoral')
X(p, press=[('at3', 0.3)], slide=[('at1', 0.5)])

p = P('Industrial Cathedral', A).wt('a', 'Drone/Iron Hum', pos=0.3, blend='spectral', voices=3, detune=8, level=0.5)
p.gr('b', 'fixed', sample='Metal/Metal Groan', pos=0.3, size=0.9, density=6, level=0.3) \
 .wt('c', 'Organs/Organ Mixture', pos=0.3, voices=2, detune=6, level=0.25) \
 .f1('Comb +', 200, res=0.4, key=0.6).f2('SVF LP12', 5000).aenv(*VLONG) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Iron').mod('m1', 'at1', 0.5).macro(2, 'Groan').mod('m2', 'blevel', 0.5).macro(3, 'Organ').mod('m3', 'clevel', 0.5) \
 .chorus('flanger', 0.12, 0.02, 0.4, 0.2).tags('industrial', 'dark', 'cathedral', 'cinematic')
X(p, press=[('f1res', 0.3)], slide=[('at1', 0.5)])

p = P('Breathing Glacier', A).wt('a', 'Ambient/Ice Cathedral', pos=0.3, blend='spectral', voices=4, detune=9, level=0.5)
p.noise(0.12, 0.6).off('b', 'c').f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.06, shape=BREATH).mod('lfo1', 'f1cut', 0.3).mod('lfo1', 'noise', 0.12).mod('lfo1', 'at1', 0.2) \
 .macro(1, 'Glacier').mod('m1', 'at1', 0.5).macro(2, 'Breath').mod('m2', 'noise', 0.3).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('ice', 'breath', 'corbacho')
X(p, press=[('noise', 0.3)], slide=[('at1', 0.5)])

p = P('Labyrinth of Echoes', A).wt('a', 'Spectral/Comb Shimmer', pos=0.3, blend='spectral', voices=3, detune=8, level=0.45)
p.add('b', 'Hollow', 'Primes', morph=0.4, partials=48, comb=0.6, cshift=0.4, shim=0.2, level=0.3).off('c') \
 .f1('Comb -', 300, res=0.5, key=0.7).f2('SVF LP12', 5000).aenv(*VLONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'bt2', 0.3) \
 .macro(1, 'Labyrinth').mod('m1', 'at1', 0.5).macro(2, 'Comb').mod('m2', 'f1res', 0.4).macro(3, 'Shift').mod('m3', 'bt2', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('comb', 'mysterious', 'roach')
X(p, press=[('f1res', 0.3)], slide=[('at1', 0.5)])

p = P('Memory of Water', A).gr('a', 'fixed', sample='Nature/Mountain Stream', pos=0.3, size=0.3, density=20, spray=0.5, level=0.3)
p.wt('b', 'Ambient/Ocean of Stars', pos=0.3, blend='spectral', voices=4, detune=10, level=0.45).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*VLONG).tape(0.1, 0.3, 0.05, 0.6, 0.6) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Water').mod('m1', 'alevel', 0.5).macro(2, 'Stars').mod('m2', 'bt1', 0.5).macro(3, 'Memory').mod('m3', 'fxwow', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('water', 'memory', 'corbacho', 'nostalgic')
X(p, press=[('blevel', 0.3)], slide=[('bt1', 0.5)])

p = P('Lost Radio Signals', A).wt('a', 'Textures/Shortwave', pos=0.3, blend='spectral', voices=3, detune=8, level=0.45)
p.gr('b', 'fixed', sample='Drones/Shortwave Radio', pos=0.3, size=0.4, density=14, level=0.25).off('c') \
 .f1('SVF BP12', 1500, res=0.2, key=0.1).f2('SVF LP12', 5000).route('parallel', 0.4).aenv(*VLONG).shifter(0.4, 0.2, 0.4, 0.8) \
 .lfo(1, 0.03, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Signal').mod('m1', 'at1', 0.5).macro(2, 'Static').mod('m2', 'blevel', 0.5).macro(3, 'Shift').mod('m3', 'fxshiftmix', 0.5) \
 .chorus('chorus', 0.15, 0.04, 0.4).tags('radio', 'lost', 'shifter', 'eerie')
X(p, press=[('f1res', 0.3)], slide=[('f1cut', 0.4)])

p = P('Altar Candles', A).wt('a', 'Ambient/Altar of Light', pos=0.3, blend='spectral', voices=3, detune=8, level=0.5)
p.gr('b', 'fixed', sample='Nature/Fire Crackle', pos=0.3, size=0.4, density=10, level=0.2) \
 .add('c', 'Choir Oo', 'Organ', morph=0.3, partials=40, shim=0.15, oct=-1, level=0.25) \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.4) \
 .macro(1, 'Altar').mod('m1', 'at1', 0.5).macro(2, 'Candles').mod('m2', 'blevel', 0.5).macro(3, 'Choir').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('sacred', 'warm', 'candle', 'roach')
X(p, press=[('clevel', 0.3)], slide=[('at1', 0.5)])

# ------------------------------------------------------------------------------------------------ DRONES
p = P('Magnificent Void Drone', DR).wt('a', 'Ambient/Magnificent Void', pos=0.3, blend='spectral', voices=4, detune=8, oct=-1, level=0.55)
p.an('b', 'saw', oct=-2, voices=2, detune=6, slop=0.5, level=0.25).sub(0.2).off('c') \
 .f1('Ladder LP24', 700, res=0.2, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.008, 'drift').mod('lfo1', 'at1', 0.5).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Void').mod('m1', 'at1', 0.5).macro(2, 'Opening').mod('m2', 'f1cut', 0.5).macro(3, 'Mass').mod('m3', 'sub', 0.4) \
 .chorus('chorus', 0.12, 0.03, 0.4).tags('roach', 'void', 'magnificent', 'dark')
X(p, press=[('f1cut', 0.35)], slide=[('at1', 0.5)])

p = P('Tibetan Throat Drone', DR).wt('a', 'Vocal/Khoomei Low', pos=0.3, blend='spectral', voices=2, detune=5, level=0.5)
p.add('b', 'Throat', 'Throat', partials=64, peak=9, peakamt=0.9, width=0.05, oct=0, level=0.35) \
 .gr('c', 'keyed', sample='Voices/Monk Hum Low', pos=0.3, size=0.9, density=8, level=0.25) \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG) \
 .mseg(1, [(0, 0.2, 0), (5, 0.5, 0), (5, 0.3, 0), (5, 0.7, 0), (5, 0.2, 0)], loop=(0, 4), mode='free').mod('mseg1', 'bt2', 0.6).mod('mseg1', 'at1', 0.4) \
 .macro(1, 'Overtone').mod('m1', 'bt2', 0.5).macro(2, 'Throat').mod('m2', 'at1', 0.5).macro(3, 'Monks').mod('m3', 'clevel', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('tibet', 'throat', 'roach', 'overtone')
X(p, vib=0, press=[('bt2', 0.4)], slide=[('at1', 0.5)])

p = P('Shruti Infinite', DR).gr('a', 'keyed', sample='Drones/Shruti Box', pos=0.3, scan=0.01, size=1.0, density=12, spray=0.3, level=0.55)
p.add('b', 'Reed', 'Fifths', morph=0.4, partials=48, shim=0.1, level=0.3).off('c') \
 .f1('SVF LP12', 5000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.29, shape=BREATH).mod('lfo1', 'alevel', 0.12).lfo(2, 0.02, 'drift').mod('lfo2', 'bt1', 0.4) \
 .macro(1, 'Bellows').mod('m1', 'alevel', 0.4).macro(2, 'Reed').mod('m2', 'blevel', 0.5).macro(3, 'Fifths').mod('m3', 'bt1', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('india', 'shruti', 'drone', 'roach')
X(p, vib=0, press=[('blevel', 0.3)], slide=[('bt1', 0.5)])

p = P('Cello Drone Choir', DR).gr('a', 'keyed', sample='Strings/Bowed Cello Drone', pos=0.3, size=1.0, density=10, spray=0.3, level=0.5)
p.gr('b', 'keyed', sample='Voices/Choir Oo', pos=0.3, size=0.9, density=10, spray=0.3, level=0.3).off('c') \
 .f1('SVF LP12', 4500, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo1', 'bt1', 0.3) \
 .macro(1, 'Cello').mod('m1', 'alevel', 0.5).macro(2, 'Choir').mod('m2', 'blevel', 0.5).macro(3, 'Scan').mod('m3', 'at1', 0.5) \
 .chorus('ensemble', 0.25, 0.05, 0.5).tags('strings', 'choir', 'drone', 'cinematic')
X(p, vib=0.012, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Didgeridoo Circle', DR).gr('a', 'keyed', sample='Drones/Didgeridoo', pos=0.3, size=0.9, density=10, level=0.55, oct=-1)
p.wt('b', 'Organic/Didgeridoo Morph', pos=0.3, level=0.3).off('c') \
 .f1('Vowel', 500, res=0.4, key=0.0, shape=0.4).aenv(*LONG) \
 .lfo(1, 0.08, 'drift').mod('lfo1', 'f1shape', 0.4).mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Mouth').mod('m1', 'f1shape', 0.6).macro(2, 'Drone').mod('m2', 'alevel', 0.5).macro(3, 'Formant').mod('m3', 'bt1', 0.5) \
 .chorus('chorus', 0.1, 0.04, 0.3).tags('didgeridoo', 'roach', 'dreamtime', 'ritual')
X(p, vib=0, press=[('f1shape', 0.4)], slide=[('bt1', 0.5)])

p = P('Electric Om', DR).wt('a', 'Vocal/Overtone Singer', pos=0.3, blend='spectral', voices=3, detune=6, level=0.5)
p.an('b', 'saw', oct=-1, voices=2, detune=6, level=0.3).off('c') \
 .f1('Xpander LP3', 1800, res=0.2, key=0.2).aenv(*LONG).phaser(0.25, 0.03, 0.6, 0.5) \
 .lfo(1, 0.03, 'sine').mod('lfo1', 'at1', 0.6) \
 .macro(1, 'Om').mod('m1', 'at1', 0.5).macro(2, 'Saw').mod('m2', 'blevel', 0.5).macro(3, 'Phase').mod('m3', 'fxphaser', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('om', 'overtone', 'electric', 'roach')
X(p, vib=0, press=[('fxphaser', 0.4)], slide=[('at1', 0.5)])

p = P('Planetary Hum', DR).an('a', 'sine', oct=-1, level=0.5, slop=0.2)
p.wt('b', 'Ambient/Earth Pulse', pos=0.3, blend='spectral', voices=3, detune=6, level=0.35).sub(0.3) \
 .gr('c', 'fixed', sample='Drones/Sub Rumble Field', pos=0.3, size=1.0, density=6, level=0.25) \
 .f1('SVF LP12', 2500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'bt1', 0.5) \
 .macro(1, 'Planet').mod('m1', 'bt1', 0.5).macro(2, 'Rumble').mod('m2', 'clevel', 0.5).macro(3, 'Mass').mod('m3', 'sub', 0.4) \
 .chorus('chorus', 0.1, 0.03, 0.3).tags('sub', 'planet', 'roach', 'deep')
X(p, vib=0, press=[('clevel', 0.3)], slide=[('bt1', 0.5)])

p = P('Bronze Gong Drone', DR).ph('a', 'Plate', 'Bow', decay=25, damp=0.08, hard=0.6, couple=0.4, oct=-1, level=0.5)
p.gr('b', 'keyed', sample='Metal/Gong Swell', pos=0.4, size=1.0, density=8, level=0.35, oct=-1).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.015, 'drift').mod('lfo1', 'at1', 0.3).mod('lfo1', 'at2', 0.3) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).macro(2, 'Gong').mod('m2', 'blevel', 0.5).macro(3, 'Position').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.12, 0.03, 0.4).tags('gong', 'metallic', 'roach', 'ritual')
X(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Harmonium Pilgrimage', DR).gr('a', 'keyed', sample='Drones/Harmonium Reed', pos=0.3, size=0.9, density=10, level=0.5)
p.wt('b', 'Organs/Organ Reed Swell', pos=0.3, voices=3, detune=6, level=0.3).off('c') \
 .f1('SVF LP12', 4000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.25, shape=BREATH).mod('lfo1', 'alevel', 0.1).lfo(2, 0.02, 'drift').mod('lfo2', 'bt1', 0.4) \
 .macro(1, 'Reed').mod('m1', 'alevel', 0.5).macro(2, 'Swell').mod('m2', 'bt1', 0.5).macro(3, 'Air').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('harmonium', 'pilgrimage', 'drone')
X(p, vib=0, press=[('bt1', 0.35)], slide=[('f1cut', 0.4)])

p = P('Black Glass Monolith', DR).wt('a', 'Drone/Black Glass', pos=0.3, blend='spectral', voices=3, detune=8, oct=-1, level=0.55)
p.wt('b', 'Drone/Monolith', pos=0.3, blend='spectral', voices=2, detune=6, level=0.3).off('c') \
 .f1('Xpander BP2', 600, res=0.3, key=0.2).f2('SVF LP12', 4000).route('parallel', 0.5).aenv(*VLONG) \
 .lfo(1, 0.011, 'drift').lfo(2, 0.027, 'sine').mod('lfo1', 'at1', 0.5).mod('lfo2', 'bt1', 0.5).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Glass').mod('m1', 'at1', 0.5).macro(2, 'Monolith').mod('m2', 'bt1', 0.5).macro(3, 'Band').mod('m3', 'f1cut', 0.5) \
 .chorus('chorus', 0.12, 0.03, 0.4).tags('dark', 'monolith', 'roach')
X(p, press=[('f1res', 0.3)], slide=[('at1', 0.5)])

p = P('Fifths of Eternity', DR).add('a', 'Fifths', 'Fifths', partials=64, tilt=-0.2, shim=0.15, srate=0.05, level=0.5)
p.an('b', 'saw', semi=7, oct=-1, voices=2, detune=6, slop=0.4, level=0.25).an('c', 'sine', oct=-2, level=0.3) \
 .f1('SEM LP12', 2500, res=0.05, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'at3', 0.4).mod('lfo1', 'f1cut', 0.3) \
 .macro(1, 'Tilt').mod('m1', 'at3', 0.5).macro(2, 'Fifth Saw').mod('m2', 'blevel', 0.5).macro(3, 'Root').mod('m3', 'clevel', 0.5) \
 .chorus('ensemble', 0.2, 0.04, 0.5).tags('fifths', 'eternal', 'roach', 'drone')
X(p, press=[('at3', 0.4)], slide=[('f1cut', 0.4)])

p = P('Cicada Drone Mantra', DR).add('a', 'Throat', 'Octaves', morph=0.3, partials=56, peak=12, peakamt=0.7, width=0.06, level=0.5)
p.gr('b', 'fixed', sample='Nature/Cicadas', pos=0.3, size=0.7, density=8, level=0.25).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.07, 'tri').mod('lfo1', 'at2', 0.35) \
 .macro(1, 'Mantra').mod('m1', 'at2', 0.5).macro(2, 'Cicadas').mod('m2', 'blevel', 0.5).macro(3, 'Octaves').mod('m3', 'at1', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('mantra', 'summer', 'overtone', 'roach')
X(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Coupled Strings Drone', DR).ph('a', 'String', 'Bow', decay=12, damp=0.15, hard=0.45, couple=0.7, level=0.5)
p.ph('b', 'String', 'Bow', decay=12, damp=0.15, hard=0.45, couple=0.7, semi=7, level=0.35) \
 .ph('c', 'Sympathetic', 'Breath', decay=10, damp=0.2, buzz=0.8, oct=-1, level=0.25) \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.03, 'drift', 'voice').mod('lfo1', 'at2', 0.4).mod('lfo1', 'bt2', 0.4) \
 .macro(1, 'Bow').mod('m1', 'at2', 0.5).mod('m1', 'bt2', 0.5).macro(2, 'Sympathy').mod('m2', 'clevel', 0.5).macro(3, 'Damping').mod('m3', 'at4', 0.5) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('physical', 'strings', 'coupled', 'corbacho')
X(p, vib=0, press=[('at2', 0.4), ('bt2', 0.3)], slide=[('at1', 0.5)])

p = P('Subterranean River', DR).an('a', 'tri', oct=-1, slop=0.5, drift=0.5, level=0.45)
p.gr('b', 'fixed', sample='Nature/Mountain Stream', pos=0.3, size=0.5, density=10, level=0.25) \
 .wt('c', 'Drone/Undertow', pos=0.3, blend='spectral', voices=2, detune=6, level=0.3).sub(0.25) \
 .f1('Ladder LP24', 600, res=0.2, key=0.2).aenv(*VLONG) \
 .lfo(1, 0.01, 'drift').mod('lfo1', 'f1cut', 0.4).mod('lfo1', 'ct1', 0.4) \
 .macro(1, 'River').mod('m1', 'blevel', 0.5).macro(2, 'Undertow').mod('m2', 'ct1', 0.5).macro(3, 'Depth').mod('m3', 'sub', 0.4) \
 .chorus('chorus', 0.1, 0.03, 0.4).tags('underground', 'water', 'roach', 'dark')
X(p, press=[('f1cut', 0.35)], slide=[('ct1', 0.5)])

p = P('Sky Lantern Drone', DR).wt('a', 'Ambient/Canyon Glow', pos=0.3, blend='spectral', voices=3, detune=7, level=0.5)
p.add('b', 'Octaves', 'Glass', morph=0.5, partials=48, shim=0.3, oct=1, level=0.25).off('c') \
 .f1('SVF LP12', 7000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'sine').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Glow').mod('m1', 'at1', 0.5).macro(2, 'Lanterns').mod('m2', 'blevel', 0.5).macro(3, 'Shimmer').mod('m3', 'bt2', 0.4) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('warm', 'glow', 'corbacho')
X(p, press=[('blevel', 0.3)], slide=[('at1', 0.5)])

p = P('Wind Harp Drone', DR).ph('a', 'Sympathetic', 'Breath', decay=12, damp=0.15, hard=0.3, buzz=0.9, level=0.5)
p.gr('b', 'fixed', sample='Nature/Canyon Wind', pos=0.3, size=1.0, density=7, level=0.3).off('c') \
 .f1('SVF LP12', 6000, res=0.05, key=0.1).aenv(*LONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at2', 0.4).mod('lfo1', 'blevel', 0.3) \
 .macro(1, 'Wind').mod('m1', 'blevel', 0.5).mod('m1', 'at2', 0.3).macro(2, 'Strings').mod('m2', 'at4', -0.4).macro(3, 'Buzz').mod('m3', 'at3', 0.4) \
 .chorus('chorus', 0.12, 0.04, 0.3).tags('aeolian', 'harp', 'wind', 'corbacho')
X(p, vib=0, press=[('at2', 0.4)], slide=[('at1', 0.5)])

p = P('Grain Cathedral Drone', DR).gr('a', 'wt', lib='Organs/Organ Cathedral', pos=0.3, scan=0.02, size=0.6, density=24, spray=0.4, prand=5, level=0.5)
p.gr('b', 'keyed', sample='Drones/Organ Cluster', pos=0.3, size=0.9, density=10, level=0.3).off('c') \
 .f1('SVF LP12', 4500, res=0.05, key=0.1).aenv(*VLONG) \
 .lfo(1, 0.02, 'drift').mod('lfo1', 'at1', 0.5) \
 .macro(1, 'Stops').mod('m1', 'at1', 0.5).macro(2, 'Cluster').mod('m2', 'blevel', 0.5).macro(3, 'Grains').mod('m3', 'at3', 0.5) \
 .chorus('ensemble', 0.25, 0.04, 0.5).tags('organ', 'granular', 'cathedral', 'roach')
X(p, press=[('at3', 0.4)], slide=[('at1', 0.5)])
