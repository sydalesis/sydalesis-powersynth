# Driftfield 4.0 bank, part C: ANALOG — the analog oscillator engine with the analog filter models.
# Families: poly pads, string machines, drones, basses, leads, brass & keys, Berlin / Tangerine sequences,
# arps, sync & cross-mod, noise and S&H effects. Everything is MPE-wired (X) and responds to Studio One
# note expression the same way (pressure / slide). No reverb, no delay.
AN = 'Analog'

def anX(p, pc=0.3, sc=0.2, vib=0.012, press=(), slide=()):
    return X(p, vib=vib, pc=pc, sc=sc, press=list(press), slide=list(slide))

def m4(p, a, b, c, d):
    """four macros: (name, dst, amt) each"""
    for i, (n, dst, amt) in enumerate((a, b, c, d), 1): p.macro(i, n).mod('m%d' % i, dst, amt)
    return p

# ================================================================================================ POLY PADS
def poly_pad(name, wa='saw', wb='saw', fine=7, voices=2, det=10, filt='OB-Xa LP12', cut=1800, res=0.15, env=0.25,
             aenv=(1.2, 2.5, 0.85, 3.5), fenv=(1.5, 3, 0.5, 3), pw=0.5, oct_b=0, chorus=('chorus', 0.3, 0.25, 0.5),
             lfo=0.18, pwm=0.0, sub=0.0, noise=0.0, drift=0.45, tags=(), press=(('f1cut', 0.3),), slide=(('at1', 0.3),)):
    p = P(name, AN).an('a', wa, pw=pw, voices=voices, detune=det, slop=0.4, drift=drift, level=0.5) \
        .an('b', wb, pw=pw, fine=fine, oct=oct_b, voices=voices, detune=det, slop=0.4, drift=drift, level=0.4).off('c') \
        .f1(filt, cut, res=res, key=0.35, env=env).fenv(*fenv).aenv(*aenv) \
        .lfo(1, lfo, 'tri').lfo(2, 0.07, 'drift', 'voice').mod('lfo2', 'f1cut', 0.08)
    if pwm: p.mod('lfo1', 'at1', pwm).mod('lfo1', 'bt1', -pwm)
    if sub: p.sub(sub)
    if noise: p.noise(noise, 0.5)
    p.chorus(*chorus)
    m4(p, ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.35), ('Env', 'f1cut', 0.35), ('Detune', 'detune', 0.35))
    p.tags('pad', 'poly', 'vintage', *tags)
    anX(p, press=press, slide=slide)
    return p

poly_pad('OB Brass Pad', filt='OB-Xa LP12', cut=1400, env=0.45, aenv=(0.5, 2, 0.8, 2.5), fenv=(0.6, 2, 0.4, 2), tags=('oberheim', 'brassy'))
poly_pad('OB Eight Voice Wash', filt='OB-Xa LP24', cut=1100, env=0.3, aenv=(2.5, 3, 0.9, 5), fenv=(3, 4, 0.6, 5), det=14, voices=3, tags=('oberheim', 'wide'))
poly_pad('Jupiter Silk', wb='pulse', pw=0.4, filt='Ladder LP12', cut=2200, res=0.1, pwm=0.25, chorus=('ensemble', 0.35, 0.2, 0.5), tags=('jupiter', 'silky'))
poly_pad('Jupiter Cross Pad', wb='pulse', pw=0.3, filt='Ladder LP24', cut=1600, res=0.2, pwm=0.3, oct_b=1, chorus=('chorus', 0.35, 0.3, 0.55), tags=('jupiter',))
poly_pad('Prophet Velvet', wa='saw', wb='pulse', pw=0.45, filt='Ladder LP24', cut=1300, res=0.25, env=0.35, chorus=('chorus', 0.25, 0.25, 0.4), tags=('prophet', 'warm'))
poly_pad('Prophet Poly-Mod Glass', wb='tri', filt='Ladder LP24', cut=2600, res=0.3, env=0.2, tags=('prophet', 'poly-mod'), chorus=('chorus', 0.2, 0.2, 0.4)) \
    .fm('a', 'b', 0.22, 'fm').mod('fenv', 'afm', 0.25)
poly_pad('Juno Silver Strings', wa='saw', wb='saw', fine=3, voices=1, filt='SVF LP12', cut=3200, res=0.05, env=0.1, sub=0.15, chorus=('ensemble', 0.45, 0.45, 0.6), tags=('juno', 'strings'))
poly_pad('Juno Pulse Choir', wa='pulse', wb='pulse', pw=0.35, voices=1, filt='SVF LP24', cut=2000, res=0.15, pwm=0.35, chorus=('ensemble', 0.5, 0.5, 0.6), tags=('juno', 'pwm'))
poly_pad('CS Blade Strings', fine=9, voices=3, det=12, filt='SEM LP12', cut=2600, res=0.1, env=0.2, aenv=(1.5, 3, 0.9, 6), chorus=('ensemble', 0.3, 0.15, 0.5), tags=('cs80', 'cinematic'))
poly_pad('CS Dream Swell', wa='saw', wb='saw', fine=11, voices=3, det=14, filt='SEM LP12', cut=900, env=0.55, aenv=(3, 3, 1, 7), fenv=(4, 4, 0.7, 6), tags=('cs80', 'swell', 'vangelis'))
poly_pad('SEM Warm Fifths', wb='saw', oct_b=0, filt='SEM Morph', cut=1700, res=0.2, env=0.25, tags=('sem', 'fifths')).set('obSemi', 7)
poly_pad('Memory Pad', wb='tri', filt='Ladder LP24', cut=1500, res=0.15, env=0.2, aenv=(1.8, 3, 0.9, 4), drift=0.6, tags=('memorymoog', 'drifting'))
poly_pad('Dark Analog Velvet', filt='Diode LP24', cut=700, res=0.2, env=0.35, aenv=(2, 3, 0.9, 5), sub=0.2, tags=('dark',))
poly_pad('Tape Age Pad', filt='OB-Xa LP12', cut=1600, res=0.1, env=0.2, chorus=('solina', 0.3, 0.15, 0.4), tags=('lofi', 'tape')).tape(0.25, 0.4, 0.2, 0.55, 1.0)
poly_pad('Phased Seventies Pad', filt='Ladder LP12', cut=2400, res=0.15, env=0.15, chorus=('chorus', 0.15, 0.2, 0.4), tags=('phaser', 'seventies')).phaser(0.45, 0.06, 0.8, 0.55)
poly_pad('Noise Breath Pad', filt='SVF LP12', cut=2000, res=0.1, env=0.3, noise=0.12, aenv=(2, 3, 0.9, 5), tags=('breath', 'airy'))
poly_pad('Hollow Square Pad', wa='pulse', wb='pulse', pw=0.5, filt='Xpander LP3', cut=1900, res=0.2, env=0.2, tags=('square', 'hollow', 'xpander'))
poly_pad('Xpander Morph Pad', filt='Xpander LP4', cut=1500, res=0.25, env=0.3, tags=('xpander', 'morph'), chorus=('chorus', 0.25, 0.2, 0.45)) \
    .set('f1Type2', FILTERS.index('Xpander BP2')).set('f1Morph', 0.0).mod('lfo1', 'f1morph', 0.4)
poly_pad('MS Scream Pad', filt='MS-20 LP12', cut=1200, res=0.55, env=0.4, tags=('ms20', 'resonant'), chorus=('chorus', 0.2, 0.2, 0.4))
poly_pad('Supersaw Horizon', wa='supersaw', wb='saw', voices=1, filt='OB-Xa LP24', cut=2500, res=0.1, env=0.15, aenv=(1.5, 3, 0.9, 5), tags=('supersaw', 'trance-free', 'wide'))

# ================================================================================================ STRING MACHINES / CHOIRS
def string_machine(name, cut=4200, mode='solina', mix=0.5, octb=1, tags=(), aenv=(0.6, 1.5, 1, 2.2), noise=0.0):
    p = P(name, AN).an('a', 'saw', voices=2, detune=6, slop=0.3, level=0.5).an('b', 'saw', oct=octb, voices=2, detune=6, slop=0.3, level=0.3).off('c') \
        .f1('SVF LP12', cut, res=0.05, key=0.3).f2('SVF HP12', 140).aenv(*aenv).chorus(mode, mix, 0.12, 0.6)
    if noise: p.noise(noise, 0.7)
    m4(p, ('Tone', 'f1cut', 0.5), ('Octave', 'blevel', 0.45), ('Ensemble', 'fxchorus', 0.4), ('Swell', 'amp', 0.2))
    p.tags('strings', 'string-machine', 'vintage', *tags)
    anX(p, press=[('f1cut', 0.3)], slide=[('blevel', 0.3)])
    return p

string_machine('Solina Prime', tags=('solina',))
string_machine('Solina Dusk Section', cut=2600, tags=('solina', 'dark'), aenv=(1.5, 2, 1, 4))
string_machine('Eminent Organ Strings', cut=3600, mode='ensemble', mix=0.45, octb=0, tags=('eminent', 'oxygene'))
string_machine('Crumar Viola Bank', cut=3000, mode='ensemble', mix=0.55, tags=('crumar',))
string_machine('Logan Shimmer Strings', cut=5200, octb=2, tags=('logan', 'bright'))
string_machine('String Machine Choir', cut=2400, mode='solina', mix=0.6, noise=0.05, tags=('choir', 'vocal'), aenv=(1.2, 2, 1, 3)) \
    .f1('Formant', 1200, res=0.2, key=0.3, shape=0.35)

# ================================================================================================ ANALOG DRONES (Roach / Schulze)
def an_drone(name, cut=600, res=0.3, filt='Ladder LP24', fine=5, oct_b=-1, rate=0.012, drv=0.2, tags=(), sub=0.2, semi_b=0, wave_b='saw', chorus=None):
    p = P(name, AN).an('a', 'saw', voices=3, detune=8, slop=0.6, drift=0.7, level=0.45) \
        .an('b', wave_b, fine=fine, oct=oct_b, semi=semi_b, voices=2, detune=6, slop=0.6, drift=0.7, level=0.35) \
        .an('c', 'tri', oct=-2, level=0.3, slop=0.4).f1(filt, cut, res=res, key=0.2, drive=drv).aenv(6, 4, 1, 16).curve(a=-0.4) \
        .lfo(1, rate, 'drift').lfo(2, rate * 2.7, 'sine').lfo(3, 0.05, 'drift', 'voice') \
        .mod('lfo1', 'f1cut', 0.45).mod('lfo2', 'f1res', 0.15).mod('lfo3', 'balance', 0.2).mod('lfo2', 'bpan', 0.4)
    if sub: p.sub(sub)
    if chorus: p.chorus(*chorus)
    else: p.chorus('chorus', 0.12, 0.05, 0.35)
    m4(p, ('Open', 'f1cut', 0.5), ('Resonance', 'f1res', 0.4), ('Motion', 'lfo1rate', 0.5), ('Low', 'clevel', 0.4))
    p.tags('drone', 'analog-drone', 'roach', *tags)
    anX(p, vib=0, press=[('f1cut', 0.35)], slide=[('f1res', 0.3)])
    return p

an_drone('Structures of Saw', tags=('structures', 'classic'))
an_drone('Modular Monolith', cut=450, res=0.45, drv=0.35, tags=('modular', 'dark'))
an_drone('Desert Night Saws', cut=800, res=0.25, semi_b=7, oct_b=0, tags=('desert', 'fifths'), chorus=('solina', 0.2, 0.06, 0.4))
an_drone('Schulze Mirage Bed', cut=1100, res=0.35, filt='SEM LP12', rate=0.02, tags=('schulze', 'berlin'), chorus=('ensemble', 0.25, 0.06, 0.5))
an_drone('Resonant Tide Drone', cut=520, res=0.62, rate=0.008, tags=('resonant', 'tidal'))
an_drone('Diode Undercurrent', cut=380, res=0.4, filt='Diode LP24', drv=0.4, tags=('diode', 'deep'))
an_drone('MS-20 Howl Drone', cut=700, res=0.7, filt='MS-20 LP12', drv=0.3, tags=('ms20', 'howl'))
an_drone('Pulse Width Cavern', cut=650, res=0.3, wave_b='pulse', tags=('pwm', 'cavern')).mod('lfo2', 'bt1', 0.35)
an_drone('Twin Sawtooth Sky', cut=1500, res=0.15, oct_b=0, fine=11, sub=0.0, tags=('bright', 'sky'), chorus=('ensemble', 0.3, 0.05, 0.55))
an_drone('Oberheim Night Drone', cut=900, res=0.3, filt='OB-Xa LP24', tags=('oberheim', 'night'))
an_drone('Xpander Breathing Drone', cut=800, res=0.35, filt='Xpander BP2', tags=('xpander', 'breathing'))
an_drone('Tri Sub Meditation', cut=1300, res=0.1, wave_b='tri', tags=('soft', 'meditative'))

# ================================================================================================ BASSES
def an_bass(name, wa='saw', wb='pulse', pw=0.4, filt='Ladder LP24', cut=380, res=0.25, env=0.55, fenv=(0.002, 0.35, 0.15, 0.2),
            aenv=(0.002, 0.5, 0.8, 0.15), glide=0.02, mode='mono', drv=0.25, dtype='soft', sub=0.0, oct_b=-1, tags=(), fine=0):
    p = P(name, AN).an('a', wa, pw=pw, level=0.5, slop=0.15).an('b', wb, pw=pw, oct=oct_b, fine=fine, level=0.4, slop=0.15).off('c') \
        .f1(filt, cut, res=res, key=0.4, env=env, drive=drv, dtype=dtype).fenv(*fenv).aenv(*aenv).curve(a=0.6).voice(mode, glide).nochorus()
    if sub: p.sub(sub)
    m4(p, ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.4), ('Env', 'f1cut', 0.4), ('Drive', 'f1drive', 0.4))
    p.tags('bass', 'mono' if mode != 'poly' else 'poly', *tags)
    anX(p, vib=0, pc=0.35, press=[('f1cut', 0.3)], slide=[('f1res', 0.3)])
    return p

an_bass('Model D Thump', tags=('moog', 'classic'))
an_bass('Model D Round Low', wb='tri', cut=260, res=0.1, env=0.4, tags=('moog', 'round'))
an_bass('Taurus Pedal', wa='saw', wb='saw', fine=5, cut=300, res=0.2, env=0.45, aenv=(0.005, 1, 0.9, 0.6), sub=0.25, tags=('taurus', 'huge'))
an_bass('SEM Rubber', filt='SEM LP12', cut=500, res=0.35, env=0.5, tags=('sem', 'rubbery'))
an_bass('MS-20 Growl', filt='MS-20 LP12', cut=450, res=0.55, env=0.5, drv=0.45, dtype='diode', tags=('ms20', 'aggressive'))
an_bass('Acid Ladder Squelch', wa='saw', wb='saw', oct_b=0, cut=320, res=0.72, env=0.75, fenv=(0.002, 0.18, 0.0, 0.1), aenv=(0.002, 0.3, 0.7, 0.08), glide=0.06, mode='legato', tags=('acid', '303-ish'))
an_bass('Acid Square Bite', wa='pulse', wb='pulse', oct_b=0, pw=0.5, filt='Diode LP24', cut=300, res=0.7, env=0.7, fenv=(0.002, 0.2, 0.0, 0.1), glide=0.06, mode='legato', tags=('acid', 'square'))
an_bass('Oberheim Stab Bass', filt='OB-Xa LP24', cut=420, res=0.3, env=0.6, mode='poly', glide=0.001, tags=('oberheim', 'stab'))
an_bass('Sync Bass Ripper', wa='saw', wb='saw', oct_b=0, cut=900, res=0.2, env=0.4, drv=0.35, tags=('sync', 'aggressive')) \
    .hardsync('b', 'a').set('obAnSync', 12).mod('fenv', 'bt2', 0.4)
an_bass('Pulse Width Sub', wa='pulse', wb='sine', pw=0.25, cut=600, res=0.1, env=0.3, sub=0.2, tags=('pwm', 'sub')).lfo(1, 0.3, 'tri').mod('lfo1', 'at1', 0.25)
an_bass('Pluck Bass Seventy', cut=500, res=0.3, env=0.6, aenv=(0.002, 0.6, 0.0, 0.3), fenv=(0.002, 0.3, 0.0, 0.2), mode='poly', glide=0.001, tags=('pluck', 'funk'))
an_bass('Analog Sub Drone Bass', wa='tri', wb='sine', cut=250, res=0.1, env=0.2, aenv=(0.02, 1, 1, 1.2), sub=0.3, tags=('sub', 'soft'))
an_bass('Diode Detune Bass', wa='saw', wb='saw', oct_b=0, fine=12, filt='Diode LP24', cut=420, res=0.3, env=0.5, drv=0.3, tags=('detuned', 'fat'))
an_bass('Xpander Hollow Bass', filt='Xpander LP2', cut=600, res=0.4, env=0.45, tags=('xpander', 'hollow'))

# ================================================================================================ LEADS
def an_lead(name, wa='saw', wb='saw', fine=6, oct_b=0, filt='Ladder LP24', cut=1400, res=0.3, env=0.4, glide=0.06, mode='legato',
            vib=0.022, drv=0.2, tags=(), aenv=(0.01, 1, 0.9, 0.4), chorus=('chorus', 0.15, 0.3, 0.3)):
    p = P(name, AN).an('a', wa, level=0.5, slop=0.2).an('b', wb, fine=fine, oct=oct_b, level=0.4, slop=0.2).off('c') \
        .f1(filt, cut, res=res, key=0.5, env=env, drive=drv).fenv(0.01, 0.6, 0.4, 0.5).aenv(*aenv).voice(mode, glide).chorus(*chorus)
    m4(p, ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.4), ('Glide', 'pitch', 0.0), ('Drive', 'f1drive', 0.4))
    p.tags('lead', *tags)
    anX(p, vib=vib, pc=0.4, press=[('f1cut', 0.35)], slide=[('f1res', 0.3)])
    return p

an_lead('Minimoog Solo', tags=('moog', 'classic'))
an_lead('Minimoog Flute', wa='tri', wb='tri', fine=3, cut=1800, res=0.1, env=0.2, tags=('moog', 'flute'))
an_lead('Prophet Sync Lead', cut=2600, res=0.2, env=0.3, tags=('prophet', 'sync')).hardsync('b', 'a').set('obAnSync', 10).mod('fenv', 'bt2', 0.5)
an_lead('Oberheim Brass Lead', filt='OB-Xa LP12', cut=1600, res=0.2, env=0.5, tags=('oberheim', 'brass'))
an_lead('MS-20 Screamer', filt='MS-20 LP12', cut=1200, res=0.65, env=0.45, drv=0.45, tags=('ms20', 'screaming'))
an_lead('Square Lead Seventy', wa='pulse', wb='pulse', fine=4, cut=2000, res=0.15, env=0.25, tags=('square', 'retro'))
an_lead('CS Bright Lead', fine=9, filt='SEM LP12', cut=2400, res=0.2, env=0.35, vib=0.03, tags=('cs80', 'vangelis'), chorus=('ensemble', 0.2, 0.2, 0.4))
an_lead('Tangerine Arc Lead', cut=1700, res=0.35, env=0.35, glide=0.12, vib=0.025, tags=('tangerine', 'glide'), chorus=('chorus', 0.2, 0.2, 0.4)).phaser(0.3, 0.08, 0.7, 0.5)
an_lead('Theremin Ghost', wa='sine', wb='tri', fine=2, cut=4000, res=0.05, env=0.0, glide=0.25, vib=0.04, tags=('theremin', 'eerie'), aenv=(0.15, 1, 1, 0.8))
an_lead('Fifth Stack Lead', oct_b=0, cut=1500, res=0.25, env=0.4, tags=('fifths', 'power')).set('obSemi', 7)
an_lead('Diode Bite Lead', filt='Diode LP24', cut=1300, res=0.5, env=0.5, drv=0.4, tags=('diode', 'biting'))
an_lead('Soft Pulse Whistle', wa='pulse', wb='sine', fine=0, oct_b=1, cut=3000, res=0.1, env=0.1, vib=0.03, tags=('whistle', 'soft'), aenv=(0.08, 1, 1, 0.6)).set('oaAnPW', 0.2)

# ================================================================================================ BRASS & KEYS
def an_brass(name, filt='OB-Xa LP12', cut=900, res=0.15, env=0.6, fenv=(0.08, 0.8, 0.4, 0.6), aenv=(0.03, 0.8, 0.85, 0.5), fine=7, tags=(), chorus=('chorus', 0.2, 0.25, 0.4)):
    p = P(name, AN).an('a', 'saw', voices=2, detune=8, slop=0.3, level=0.5).an('b', 'saw', fine=fine, voices=2, detune=8, slop=0.3, level=0.4).off('c') \
        .f1(filt, cut, res=res, key=0.4, env=env).fenv(*fenv).aenv(*aenv).chorus(*chorus)
    m4(p, ('Brightness', 'f1cut', 0.5), ('Env', 'f1cut', 0.4), ('Resonance', 'f1res', 0.35), ('Detune', 'detune', 0.3))
    p.tags('brass', 'poly', *tags)
    anX(p, press=[('f1cut', 0.35)], slide=[('f1res', 0.25)])
    return p

an_brass('OB Poly Brass', tags=('oberheim', 'jump-free'))
an_brass('CS Brass Section', filt='SEM LP12', cut=800, env=0.65, fenv=(0.2, 1.2, 0.5, 0.8), aenv=(0.12, 1, 0.9, 0.8), fine=9, tags=('cs80', 'cinematic'), chorus=('ensemble', 0.25, 0.2, 0.45))
an_brass('Prophet Brass Stab', filt='Ladder LP24', cut=700, env=0.7, fenv=(0.01, 0.4, 0.2, 0.3), aenv=(0.005, 0.5, 0.6, 0.3), tags=('prophet', 'stab'))
an_brass('Soft Horn Swell', filt='Ladder LP12', cut=600, env=0.5, fenv=(0.6, 1.5, 0.6, 1.2), aenv=(0.5, 1.2, 0.9, 1.4), tags=('horn', 'soft'))
an_brass('Blade Fanfare', filt='OB-Xa LP24', cut=1000, res=0.2, env=0.6, fenv=(0.05, 1, 0.5, 0.8), fine=11, tags=('fanfare', 'cinematic'), chorus=('ensemble', 0.3, 0.2, 0.5))

def an_keys(name, wa='pulse', wb='saw', pw=0.4, cut=2200, res=0.1, env=0.4, decay=1.5, tags=(), chorus=('chorus', 0.25, 0.4, 0.4), phaser=False):
    p = P(name, AN).an('a', wa, pw=pw, level=0.5, slop=0.2).an('b', wb, oct=1, level=0.25, slop=0.2).off('c') \
        .f1('Ladder LP12', cut, res=res, key=0.5, env=env).fenv(0.002, decay * 0.6, 0.2, 0.5).aenv(0.002, decay, 0.35, 0.5).curve(a=0.8).chorus(*chorus)
    if phaser: p.phaser(0.4, 0.15, 0.6, 0.4)
    m4(p, ('Tone', 'f1cut', 0.5), ('Bell', 'blevel', 0.4), ('Decay', 'f1cut', 0.3), ('Chorus', 'fxchorus', 0.4))
    p.tags('keys', 'poly', *tags)
    anX(p, vib=0, press=[('f1cut', 0.3)], slide=[('blevel', 0.3)])
    return p

an_keys('Poly Clav Pulse', pw=0.25, cut=3000, env=0.5, decay=0.8, tags=('clav', 'funky'), phaser=True)
an_keys('Analog Electric Piano', wa='tri', wb='sine', cut=2600, env=0.3, decay=2.5, tags=('epiano', 'soft'), chorus=('chorus', 0.3, 0.5, 0.4))
an_keys('Synth Harp Pluck', wa='saw', wb='tri', cut=1800, env=0.6, decay=1.8, tags=('harp', 'pluck'))
an_keys('Organ Square Combo', wa='pulse', wb='pulse', pw=0.5, cut=4000, env=0.0, decay=6, tags=('organ', 'combo'), chorus=('chorus', 0.2, 1.5, 0.3)) \
    .aenv(0.005, 0.2, 1.0, 0.1)
an_keys('Glass Chime Poly', wa='tri', wb='pulse', pw=0.15, cut=5000, env=0.3, decay=3, tags=('chime', 'bell'))

# ================================================================================================ SEQUENCES & ARPS (Berlin school)
def an_seq(name, notes, gates, rate='1/16', cut=650, res=0.45, filt='Ladder LP24', wb='pulse', tags=(), env=0.55, decay=0.25, chorus=None, phaser=False, swing=0.0):
    p = P(name, AN).an('a', 'saw', level=0.6, slop=0.2).an('b', wb, pw=0.3, oct=-1, level=0.35, slop=0.2).off('c') \
        .f1(filt, cut, res=res, key=0.4, env=env).fenv(0.002, decay, 0.1, 0.2).aenv(0.002, decay + 0.05, 0.5, 0.25) \
        .seq('notes', rate, len(notes), 0.45, swing, 0.0, notes, gates).voice('mono', 0.01) \
        .lfo(1, 0.1, 'sine', sync=True, div='4 bars').lfo(2, 0.1, 'tri', sync=True, div='2 bars').mod('lfo1', 'f1cut', 0.35).mod('lfo2', 'f1res', 0.12).mod('alt', 'f1cut', 0.08)
    if chorus: p.chorus(*chorus)
    else: p.nochorus()
    if phaser: p.phaser(0.35, 0.05, 0.7, 0.5)
    m4(p, ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.4), ('Decay', 'f1cut', 0.4), ('Pulse', 'blevel', 0.4))
    p.tags('berlin', 'sequenced', 'hold-a-note', *tags)
    anX(p, vib=0, press=[('f1cut', 0.35)], slide=[('f1res', 0.3)])
    return p

an_seq('Phaedra Drift Seq', [0, 7, 12, 7, 3, 7, 10, 7, 0, 7, 12, 15, 12, 10, 7, 3], [1] * 16, cut=560, res=0.5, tags=('tangerine', 'phaedra'), phaser=True)
an_seq('Rubycon Pulse', [0, 0, 12, 0, 10, 0, 7, 0, 0, 0, 12, 0, 15, 12, 10, 7], [1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1], cut=480, tags=('tangerine', 'dark'))
an_seq('Timewind Arc', [0, 3, 7, 12, 15, 12, 7, 3], [1] * 8, rate='1/8', cut=900, res=0.35, tags=('schulze',), chorus=('ensemble', 0.25, 0.15, 0.4))
an_seq('Night Highway Seq', [0, 12, 0, 12, 7, 19, 7, 19, 5, 17, 5, 17, 3, 15, 3, 15], [1] * 16, cut=700, res=0.4, tags=('driving', 'night'))
an_seq('Oxygene Drip', [12, 7, 3, 0, 12, 7, 3, 0, 10, 7, 3, 0, 10, 7, 5, 3], [1] * 16, cut=1300, res=0.3, filt='SVF LP12', wb='tri', tags=('jarre', 'melodic'), chorus=('chorus', 0.25, 0.3, 0.4))
an_seq('Stratosfear Run', [0, 12, 10, 12, 7, 12, 5, 12, 3, 12, 5, 12, 7, 12, 10, 12], [1] * 16, cut=620, res=0.55, tags=('tangerine', 'fast'), decay=0.18)
an_seq('Mirage Sequence', [0, 7, 0, 10, 0, 12, 0, 15, 0, 12, 0, 10, 0, 7, 0, 5], [1] * 16, cut=800, res=0.45, filt='SEM LP12', tags=('schulze', 'mirage'), chorus=('ensemble', 0.2, 0.1, 0.4))
an_seq('Diode Ostinato', [0, 0, 7, 0, 0, 10, 0, 12], [1, 1, 1, 0, 1, 1, 0, 1], rate='1/16', cut=420, res=0.6, filt='Diode LP24', tags=('dark', 'driving'))
an_seq('Swing Analog Pulse', [0, 12, 7, 12, 0, 15, 7, 12], [1] * 8, rate='1/16', cut=700, res=0.35, tags=('swing', 'groove'), swing=0.18)
an_seq('Cathedral Sequencer', [0, 7, 12, 19, 24, 19, 12, 7, 0, 5, 12, 17, 24, 17, 12, 5], [1] * 16, rate='1/8', cut=1100, res=0.25, filt='OB-Xa LP24', tags=('majestic', 'roach'), chorus=('ensemble', 0.3, 0.1, 0.5), decay=0.4)
an_seq('Bubbling Resonance Seq', [0, 12, 3, 15, 7, 19, 10, 22], [1] * 8, rate='1/16', cut=380, res=0.72, tags=('resonant', 'bubbling'), env=0.7, decay=0.15)
an_seq('Slow Berlin Tide', [0, 7, 10, 12, 10, 7, 3, 5], [1] * 8, rate='1/4', cut=900, res=0.35, tags=('slow', 'roach'), decay=0.9, chorus=('chorus', 0.2, 0.08, 0.4))

def an_arp(name, mode='up', octs=2, rate='1/16', cut=1500, res=0.3, wa='pulse', pw=0.3, tags=(), decay=0.3, chorus=('chorus', 0.2, 0.3, 0.4), gate=0.5):
    p = P(name, AN).an('a', wa, pw=pw, level=0.55, slop=0.2).an('b', 'saw', fine=5, level=0.3, slop=0.2).off('c') \
        .f1('Ladder LP24', cut, res=res, key=0.5, env=0.45).fenv(0.002, decay, 0.15, 0.2).aenv(0.002, decay + 0.1, 0.4, 0.3) \
        .arp(mode, octs, rate, gate).chorus(*chorus).lfo(1, 0.1, 'sine', sync=True, div='2 bars').mod('lfo1', 'f1cut', 0.3)
    m4(p, ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.4), ('Decay', 'f1cut', 0.35), ('Saw', 'blevel', 0.4))
    p.tags('arp', 'hold-a-chord', *tags)
    anX(p, vib=0, press=[('f1cut', 0.35)], slide=[('f1res', 0.3)])
    return p

an_arp('Jupiter Arp Cascade', 'updown', 2, tags=('jupiter', 'classic'))
an_arp('Juno Chord Arp', 'up', 1, '1/8', cut=2200, res=0.15, tags=('juno', 'gentle'), chorus=('ensemble', 0.45, 0.5, 0.6))
an_arp('Random Pulse Arp', 'random', 3, '1/16', cut=1200, res=0.45, tags=('random', 'sparkle'))
an_arp('Triplet Glass Arp', 'up', 2, '1/8T', cut=2600, res=0.2, wa='tri', tags=('triplet', 'glass'))
an_arp('Down Spiral Arp', 'down', 3, '1/16', cut=1000, res=0.5, tags=('descending', 'hypnotic'))
an_arp('Order Pluck Arp', 'order', 2, '1/16', cut=1800, res=0.3, wa='saw', tags=('as-played', 'pluck'), decay=0.2)

# ================================================================================================ SYNC, CROSS-MOD, FX
p = P('Sync Sweep Pad', AN).an('a', 'saw', level=0.45, slop=0.3).an('b', 'saw', level=0.4, slop=0.3).off('c') \
    .hardsync('b', 'a').set('obAnSync', 6).f1('OB-Xa LP12', 2600, res=0.15, key=0.3).aenv(1, 2, 0.9, 3) \
    .lfo(1, 0.05, 'tri').mod('lfo1', 'bt2', 0.6).mod('fenv', 'bt2', 0.3).fenv(1, 3, 0.4, 2).chorus('ensemble', 0.3, 0.2, 0.5)
m4(p, ('Sync', 'bt2', 0.5), ('Cutoff', 'f1cut', 0.5), ('Sweep Rate', 'lfo1rate', 0.5), ('Detune', 'detune', 0.3)).tags('sync', 'pad', 'sweep')
anX(p, press=[('bt2', 0.4)], slide=[('f1cut', 0.3)])

p = P('Ring Mod Bells', AN).an('a', 'sine', level=0.3, slop=0.0).an('b', 'sine', semi=7, fine=13, level=0.0, slop=0.0).off('c') \
    .ring(0.6, 'ab').f1('SVF LP12', 7000, res=0.05, key=0.2).aenv(0.002, 2.5, 0.0, 2.5).curve(a=1).chorus('chorus', 0.2, 0.2, 0.4)
m4(p, ('Ring', 'ring', 0.4), ('Ratio', 'bpitch', 0.2), ('Body', 'alevel', 0.4), ('Tone', 'f1cut', 0.5)).tags('ring-mod', 'bells', 'metallic')
anX(p, vib=0, press=[('ring', 0.3)], slide=[('bpitch', 0.1)])

p = P('Cross-Mod Growl Pad', AN).an('a', 'saw', voices=2, detune=8, level=0.45, slop=0.3).an('b', 'tri', oct=-1, level=0.35, slop=0.3).off('c') \
    .fm('a', 'b', 0.25, 'fm').f1('Ladder LP24', 1100, res=0.3, key=0.3, env=0.3).fenv(1, 2, 0.5, 2).aenv(0.8, 2, 0.9, 3) \
    .lfo(1, 0.08, 'tri').mod('lfo1', 'afm', 0.2).chorus('chorus', 0.2, 0.2, 0.4)
m4(p, ('Cross-Mod', 'afm', 0.4), ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.4), ('Motion', 'lfo1rate', 0.5)).tags('cross-mod', 'pad', 'growl')
anX(p, press=[('afm', 0.35)], slide=[('f1cut', 0.3)])

p = P('Sample and Hold Bleeps', AN).an('a', 'pulse', pw=0.25, level=0.5, slop=0.1).off('b', 'c') \
    .f1('Ladder LP24', 1200, res=0.65, key=0.4, env=0.3).fenv(0.002, 0.15, 0.0, 0.1).aenv(0.002, 0.2, 0.5, 0.2) \
    .lfo(1, 8, 'sh', sync=True, div='1/16').mod('lfo1', 'f1cut', 0.6).mod('lfo1', 'apitch', 0.0).chorus('chorus', 0.15, 0.3, 0.3)
m4(p, ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.3), ('S&H Depth', 'f1cut', 0.4), ('Rate', 'lfo1rate', 0.5)).tags('sample-and-hold', 'bleeps', 'fx', 'seventies')
anX(p, vib=0, press=[('f1res', 0.3)], slide=[('f1cut', 0.3)])

p = P('Noise Wind Sweep', AN).off('a', 'b', 'c').noise(0.8, 0.5).f1('SVF BP12', 900, res=0.55, key=0.6).aenv(1.5, 2, 1, 4) \
    .lfo(1, 0.07, 'drift').lfo(2, 0.21, 'sine').mod('lfo1', 'f1cut', 0.6).mod('lfo2', 'pan', 0.4)
m4(p, ('Pitch', 'f1cut', 0.5), ('Whistle', 'f1res', 0.4), ('Gust', 'lfo1rate', 0.5), ('Width', 'pan', 0.4)).tags('noise', 'wind', 'fx', 'texture')
anX(p, vib=0, press=[('f1res', 0.35)], slide=[('f1cut', 0.4)])

p = P('Laser Zap Kit', AN).an('a', 'saw', level=0.5, slop=0.0).off('b', 'c').f1('Ladder LP24', 3000, res=0.6, key=0.3, env=0.6).fenv(0.001, 0.25, 0.0, 0.1) \
    .aenv(0.001, 0.4, 0.0, 0.2).menv(0.001, 0.3, 0.0, 0.1).mod('menv', 'apitch', 0.8).nochorus()
m4(p, ('Sweep', 'apitch', 0.5), ('Cutoff', 'f1cut', 0.5), ('Resonance', 'f1res', 0.3), ('Length', 'amp', 0.2)).tags('fx', 'zap', 'laser', 'one-shot')
anX(p, vib=0, press=[('f1res', 0.3)], slide=[('apitch', 0.2)])

p = P('Analog Thunder Rumble', AN).an('a', 'saw', oct=-2, voices=3, detune=20, level=0.3, slop=0.8, drift=0.9).off('b', 'c').noise(0.5, 0.1) \
    .f1('Ladder LP24', 300, res=0.3, key=0.0, drive=0.4).aenv(0.5, 4, 0.6, 6).lfo(1, 0.15, 'drift').mod('lfo1', 'f1cut', 0.5).mod('lfo1', 'noise', 0.2)
m4(p, ('Rumble', 'f1cut', 0.5), ('Noise', 'noise', 0.3), ('Motion', 'lfo1rate', 0.5), ('Grit', 'f1drive', 0.4)).tags('fx', 'thunder', 'rumble', 'texture')
anX(p, vib=0, press=[('f1cut', 0.35)], slide=[('noise', 0.2)])

p = P('Phaser Jet Wash', AN).an('a', 'saw', voices=3, detune=10, level=0.45, slop=0.4).an('b', 'saw', oct=1, voices=2, detune=10, level=0.25, slop=0.4).off('c').noise(0.06, 0.8) \
    .f1('SVF LP12', 5000, res=0.1, key=0.3).aenv(2, 3, 1, 5).phaser(0.6, 0.04, 0.9, 0.7).nochorus()
m4(p, ('Jet', 'fxphaser', 0.4), ('Air', 'noise', 0.15), ('Octave', 'blevel', 0.4), ('Tone', 'f1cut', 0.5)).tags('phaser', 'jet', 'wash', 'seventies')
anX(p, press=[('fxphaser', 0.3)], slide=[('f1cut', 0.3)])

p = P('Resonant Ping Rain', AN).an('a', 'pulse', pw=0.3, level=0.8, slop=0.1).off('b', 'c').f1('SVF BP12', 2000, res=0.6, key=0.6, env=0.4).fenv(0.001, 0.08, 0.0, 0.05) \
    .aenv(0.001, 0.15, 0.0, 0.15).arp('random', 3, '1/32', 0.3).lfo(1, 0.2, 'drift').mod('lfo1', 'f1cut', 0.4).chorus('chorus', 0.2, 0.3, 0.4)
m4(p, ('Pitch', 'f1cut', 0.5), ('Ring', 'f1res', 0.2), ('Density', 'lfo1rate', 0.4), ('Width', 'fxchorus', 0.4)).tags('fx', 'pings', 'rain', 'arp')
anX(p, vib=0, press=[('f1res', 0.2)], slide=[('f1cut', 0.3)])

p = P('Detuned Organ Drone', AN).an('a', 'pulse', pw=0.5, voices=2, detune=4, level=0.4, slop=0.2).an('b', 'pulse', pw=0.5, oct=1, voices=2, detune=4, level=0.3, slop=0.2) \
    .an('c', 'tri', oct=-1, level=0.3, slop=0.2).f1('SVF LP12', 3000, res=0.05, key=0.2).aenv(0.5, 2, 1, 3).lfo(1, 6, 'sine').mod('lfo1', 'alevel', 0.04).chorus('chorus', 0.25, 1.2, 0.35)
m4(p, ('Mixture', 'blevel', 0.4), ('Pedal', 'clevel', 0.4), ('Tremulant', 'alevel', 0.1), ('Tone', 'f1cut', 0.5)).tags('organ', 'drone', 'square')
anX(p, vib=0, press=[('blevel', 0.3)], slide=[('f1cut', 0.3)])

p = P('Analog Heartbeat', AN).an('a', 'sine', oct=-1, level=0.6, slop=0.0).off('b', 'c').f1('Ladder LP24', 400, res=0.2, key=0.3, env=0.4).fenv(0.001, 0.12, 0.0, 0.1) \
    .aenv(0.001, 0.35, 0.0, 0.3).menv(0.001, 0.08, 0.0, 0.05).mod('menv', 'apitch', 0.25).seq('notes', '1/8', 4, 0.3, 0.0, 0.0, [0, 0, 0, 0], [1, 1, 0, 0]).voice('mono', 0.001).nochorus()
m4(p, ('Punch', 'apitch', 0.4), ('Tone', 'f1cut', 0.5), ('Body', 'f1res', 0.3), ('Length', 'amp', 0.2)).tags('pulse', 'heartbeat', 'sequenced', 'fx')
anX(p, vib=0, press=[('f1cut', 0.3)], slide=[('apitch', 0.1)])
