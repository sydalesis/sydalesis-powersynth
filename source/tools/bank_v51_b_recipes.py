# Driftfield 5.1 bank, part B: section recipes, names, and the generation pass.
# Section targets count the presets already in the bank (hand-made ones stay and count).
#
# Style brief: long-form ambient in the line of Steve Roach (Structures-era breathing analog warmth, Mystic
# chord drones, Dreamtime tribal earth, Berlin-school sequences, the vast void), Max Corbacho (luminous, slow
# shimmering harmonic fields, resonant earth, deep space), and Sydalesis (dark, cinematic, nocturnal electronic
# ambient). Slow everything: attacks in seconds, modulation in tens of seconds, nothing static.

# ------------------------------------------------------------------------------------------- recipes
DRONE_SCALES = ['JI 5-limit Major', 'JI 5-limit Minor', 'JI 7-limit Septimal', 'Harmonics 8-16', 'Harmonic 16-32', 'Raga Bhairav',
                'Raga Yaman', '22 Shruti', 'Pythagorean']
DARK_SCALES = ['Raga Todi', 'Raga Marwa', 'Raga Bhairav', 'Maqam Bayati', 'JI 7-limit Septimal', 'JI 5-limit Minor', 'Harmonics 8-16']
TRIBE_SCALES = ['Slendro', 'Pelog', 'Raga Bhairavi', 'Raga Bhairav', 'Maqam Bayati', 'Maqam Rast', 'Harmonics 8-16', '22 Shruti', 'JI 7-limit Septimal']
SPACE_SCALES = ['Harmonic 16-32', 'Harmonics 8-16', 'JI 7-limit Septimal', 'JI 5-limit Major', 'Partch 43']
ALL_JI = DRONE_SCALES + ['Raga Darbari', 'Raga Bhupali', 'Raga Todi', 'Maqam Rast', 'Slendro', 'Pelog', 'Well-Tuned Piano', 'Kirnberger III', 'Meantone 1/4-comma']

FX_AMBIENT = dict(chorus=0.65, phaser=0.12, drive=0.1, tape=0.15, shift=0.05, reso=0.3, sfx=0.4, shimmer=0.1, width=0.45, eq=0.5)

RC = {}
RC['Drones'] = dict(cat='Drones', target=500,
    menuA=[('wt', ['dark', 'warm', 'earth'], 4), ('an', ['warm', 'sine', 'hollow'], 2), ('add', ['organ', 'throat', 'choir', 'hollow'], 3),
           ('sp', ['drones', 'voices', 'strings', 'earthy'], 3), ('ph', ['bowed', 'breath'], 2)],
    menuB=[('wt', ['dark', 'warm', 'earth', 'cosmic'], 3), ('an', ['warm', 'sine', 'hollow'], 2), ('add', ['organ', 'throat', 'choir', 'glass'], 3),
           ('sp', ['drones', 'voices', 'strings', 'bowls'], 2), ('ph', ['bowed', 'breath'], 2), ('gr', ['drones', 'strings', 'voices'], 1)],
    menuC=[('an', ['sine', 'hollow'], 2), ('gr', ['nature', 'hum', 'textures'], 2), ('add', ['glass', 'primes', 'organ'], 2), ('ph', ['bowed', 'rain'], 1),
           ('wt', ['cosmic', 'dark'], 1), ('sp', ['bowls', 'drones'], 1)],
    octs=[[(-1, 3), (0, 3), (-2, 1)], [(0, 2), (-1, 2), (1, 1)], [(-2, 2), (1, 2), (0, 1)]], fifths=0.45, p3=0.65,
    env=((2, 10), (3, 8), (0.85, 1.0), (8, 22)), acurve=(-0.7, 0.0), cut=(300, 3200), ftone=[('warm', 3), ('dark', 2), ('vocal', 1)],
    tune=0.25, scales=DRONE_SCALES, fx=dict(FX_AMBIENT, reso=0.35), routes=(9, 15), scenes=0.4, motion=0.25,
    lfo_rates=(0.005, 0.04), mseg_len=(30, 150), bloom=0.6, bloom_len=(8, 45), sub=0.45, noise=0.15,
    styles=['roach', 'corbacho', 'sydalesis'], moods=['meditative', 'vast', 'dark', 'ritual', 'warm', 'sacred', 'hypnotic', 'deep'], tags=('drone',))

RC['Pads'] = dict(cat='Pads', target=500,
    menuA=[('an', ['warm', 'super', 'hollow'], 4), ('wt', ['warm', 'vocal', 'strings', 'organ'], 4), ('add', ['choir', 'strings', 'organ', 'glass'], 3), ('sp', ['voices', 'strings'], 1)],
    menuB=[('an', ['warm', 'hollow'], 3), ('wt', ['warm', 'cosmic', 'vocal'], 3), ('add', ['choir', 'glass', 'organ'], 2), ('ph', ['bowed'], 1), ('sp', ['voices', 'drones'], 1)],
    menuC=[('add', ['glass', 'choir'], 2), ('wt', ['cosmic', 'warm'], 2), ('an', ['sine'], 1), ('gr', ['voices', 'strings'], 1)],
    octs=[[(0, 3), (-1, 1)], [(0, 2), (1, 1), (-1, 1)], [(1, 2), (0, 1), (-1, 1)]], fifths=0.2, p3=0.55,
    env=((1.0, 6), (2, 6), (0.8, 1.0), (4, 14)), fenv=((1.5, 8), (3, 10), (0.4, 0.8), (4, 12)), fenv_amt=(0.05, 0.3),
    cut=(600, 4500), ftone=[('warm', 4), ('open', 1), ('vocal', 1)], vib=(0.0, 0.008),
    tune=0.08, scales=DRONE_SCALES, fx=dict(FX_AMBIENT, chorus=0.85, sfx=0.3), routes=(9, 14), scenes=0.45, motion=0.2,
    lfo_rates=(0.008, 0.08), mseg_len=(20, 90), bloom=0.5, bloom_len=(4, 25), sub=0.25, noise=0.15,
    styles=['roach', 'corbacho'], moods=['warm', 'luminous', 'breathing', 'lush', 'tender', 'glowing', 'slow', 'nostalgic'], tags=('pad',))

RC['Atmospheres'] = dict(cat='Atmospheres', target=500,
    menuA=[('wt', ['cosmic', 'warm', 'vocal'], 3), ('add', ['glass', 'choir', 'hollow', 'primes'], 3), ('sp', ['voices', 'bowls', 'drones'], 2),
           ('gr', ['voices', 'strings', 'bowls'], 2), ('ph', ['bowed', 'rain'], 1), ('an', ['hollow', 'warm'], 1)],
    menuB=[('wt', ['cosmic', 'warm', 'earth'], 3), ('add', ['glass', 'choir', 'organ'], 2), ('sp', ['voices', 'nature', 'bowls'], 2), ('gr', ['nature', 'voices'], 2), ('ph', ['bowed', 'rain'], 1)],
    menuC=[('gr', ['nature', 'textures'], 3), ('add', ['glass', 'primes'], 2), ('ph', ['rain', 'bowed'], 1), ('an', ['sine'], 1)],
    octs=[[(0, 3), (-1, 1), (1, 1)], [(0, 2), (1, 2), (-1, 1)], [(1, 2), (-1, 1), (0, 1)]], fifths=0.3, p3=0.7,
    env=((3, 12), (4, 10), (0.85, 1.0), (10, 26)), acurve=(-0.8, -0.1), cut=(1200, 9000), ftone=[('open', 3), ('warm', 2), ('thin', 1)],
    tune=0.1, scales=DRONE_SCALES, fx=dict(FX_AMBIENT, sfx=0.45, width=0.6), routes=(10, 15), scenes=0.4, motion=0.25,
    lfo_rates=(0.006, 0.05), mseg_len=(30, 140), bloom=0.55, bloom_len=(8, 40), sub=0.2, noise=0.45, sh=0.4,
    styles=['corbacho', 'roach'], moods=['airy', 'weightless', 'ethereal', 'hazy', 'vast', 'nocturnal', 'dreaming', 'suspended'], tags=('atmosphere',))

RC['Textures'] = dict(cat='Textures', target=500,
    menuA=[('gr', ['nature', 'organic', 'textures', 'metalfx', 'voices'], 4), ('sp', ['nature', 'textures', 'organic', 'metalfx', 'pitched'], 3),
           ('ph', ['rain', 'scrape', 'bowed'], 2), ('wt', ['motion', 'metal', 'earth'], 2), ('add', ['glass', 'primes', 'hollow'], 1)],
    menuB=[('gr', ['nature', 'organic', 'textures', 'metalfx'], 3), ('sp', ['nature', 'textures', 'metalfx', 'bowls'], 2), ('ph', ['rain', 'scrape'], 2),
           ('wt', ['motion', 'metal', 'cosmic'], 2), ('add', ['glass', 'primes'], 1)],
    menuC=[('an', ['sine', 'hollow'], 1), ('wt', ['dark', 'cosmic'], 1), ('add', ['glass'], 1), ('gr', ['nature', 'textures'], 1)],
    octs=[[(0, 3), (1, 1), (-1, 1)], [(0, 2), (1, 1), (-1, 1)], [(-1, 2), (1, 1)]], fifths=0.2, p3=0.55,
    env=((0.3, 6), (2, 8), (0.7, 1.0), (4, 16)), cut=(1500, 12000), ftone=[('motion', 3), ('open', 2), ('thin', 1), ('warm', 1)],
    fx=dict(FX_AMBIENT, phaser=0.25, crush=0.06, sfx=0.4), routes=(11, 16), scenes=0.35, motion=0.3, sh=0.7,
    lfo_rates=(0.01, 0.2), mseg_len=(8, 60), bloom=0.4, bloom_len=(3, 20), sub=0.1, noise=0.3, sp_shift=0.15, sp_stretch=0.3,
    styles=['corbacho', 'roach', 'sydalesis'], moods=['granular', 'organic', 'glitter', 'crackling', 'dusty', 'restless', 'tactile', 'microscopic'], tags=('texture',))

RC['Soundscapes'] = dict(cat='Soundscapes', target=500,
    menuA=[('gr', ['nature', 'organic'], 5), ('sp', ['nature'], 1)],
    menuB=[('wt', ['warm', 'earth', 'cosmic', 'dark'], 3), ('add', ['choir', 'organ', 'glass', 'throat'], 2), ('sp', ['voices', 'drones', 'strings', 'bowls'], 2), ('an', ['warm', 'hollow'], 1)],
    menuC=[('ph', ['bowed', 'rain', 'breath'], 2), ('gr', ['nature', 'organic', 'metalfx'], 2), ('add', ['glass'], 1), ('wt', ['cosmic'], 1)],
    octs=[[(0, 1)], [(0, 2), (-1, 2), (1, 1)], [(0, 2), (1, 1), (-1, 1)]], fifths=0.3, p3=0.7,
    env=((4, 14), (4, 10), (0.85, 1.0), (12, 28)), acurve=(-0.8, -0.2), cut=(1500, 9000), ftone=[('open', 3), ('warm', 2)],
    tune=0.12, scales=DRONE_SCALES, fx=dict(FX_AMBIENT, sfx=0.4, reso=0.3, width=0.55), routes=(10, 15), scenes=0.5, motion=0.25,
    lfo_rates=(0.005, 0.04), mseg_len=(40, 160), bloom=0.6, bloom_len=(10, 45), sub=0.15, noise=0.2,
    styles=['roach', 'corbacho', 'sydalesis'], moods=['place', 'field-recording', 'journey', 'landscape', 'weather', 'night', 'dawn', 'wilderness'], tags=('soundscape',))

RC['Spectral'] = dict(cat='Spectral', target=500,
    menuA=[('sp', ['voices', 'drones', 'strings', 'bowls', 'earthy', 'pitched'], 5), ('sp', ['nature', 'textures', 'organic', 'metalfx', 'hum'], 2)],
    menuB=[('sp', ['voices', 'bowls', 'drones', 'nature', 'strings'], 3), ('add', ['glass', 'choir'], 1), ('wt', ['cosmic', 'warm'], 1), ('an', ['sine'], 1)],
    menuC=[('add', ['glass'], 1), ('an', ['sine'], 1), ('wt', ['dark', 'cosmic'], 1)],
    octs=[[(0, 3), (-1, 2)], [(0, 2), (1, 1), (-1, 1)], [(-1, 2), (1, 1)]], fifths=0.3, p3=0.35,
    env=((1.5, 8), (3, 8), (0.85, 1.0), (6, 18)), cut=(1500, 12000), ftone=[('open', 3), ('warm', 2), ('vocal', 1)],
    sp_cross=0.35, sp_freeze=0.25, sp_stretch=0.25, sp_shift=0.1, cross_sets=['voices', 'nature', 'bowls', 'drones', 'textures', 'earthy'],
    tune=0.08, scales=DRONE_SCALES, fx=dict(FX_AMBIENT, sfx=0.45), routes=(9, 14), scenes=0.4, motion=0.3, macro_core=['smear', 'position'],
    lfo_rates=(0.006, 0.06), mseg_len=(20, 120), bloom=0.5, bloom_len=(5, 30), sub=0.2, noise=0.1,
    styles=['corbacho', 'roach', 'sydalesis'], moods=['resynthesis', 'smeared', 'frozen', 'spectral', 'ghostly', 'time-stretched', 'liquid', 'haunted'], tags=('spectral',))

def _sweep_post(c):
    r = c.r
    kind = _wch(r, [('lfo', 3), ('mseg', 2)])
    if kind == 'lfo':
        if _p(r, 0.5): c.p.lfo(5, 0.1, _ch(r, ['tri', 'sine']), sync=True, div=_ch(r, ['8 bars', '4 bars', '4 bars', '2 bars']))
        else: c.p.lfo(5, _t(_lg(r, 0.02, 0.15), 3), _ch(r, ['tri', 'sine', 'sawup', 'sawdown']))
        _mod(c, 'lfo5', 'f1cut', _u(r, 0.5, 0.8))
        if _p(r, 0.5): _mod(c, 'lfo5', _ch(r, ['f1res', 'f1shape', 'f1morph']), _u(r, 0.15, 0.35))
    else:
        rise = _lg(r, 4, 24)
        c.p.mseg(4, [(0, 0.0, 0), (_t(rise, 2), 1.0, _t(_u(r, -0.3, 0.5))), (_t(rise * _u(r, 0.6, 1.4), 2), 0.0, _t(_u(r, -0.5, 0.3)))], loop=(0, 2), mode=_ch(r, ['free', 'loop']))
        _mod(c, 'mseg4', 'f1cut', _u(r, 0.5, 0.85))
        if _p(r, 0.5): _mod(c, 'mseg4', _ch(r, ['f1res', 'f1shape', 'f1morph', 'at1']), _u(r, 0.15, 0.35))
RC['Sweeps'] = dict(cat='Sweeps', target=500,
    menuA=[('an', ['warm', 'bright', 'super'], 4), ('wt', ['warm', 'analog', 'motion', 'vocal'], 3), ('add', ['strings', 'organ', 'choir', 'hollow'], 2), ('sp', ['drones', 'voices'], 1)],
    menuB=[('an', ['warm', 'hollow', 'bright'], 3), ('wt', ['warm', 'analog', 'cosmic'], 2), ('add', ['glass', 'organ'], 1), ('gr', ['nature', 'textures'], 1)],
    menuC=[('an', ['sine', 'hollow'], 1), ('add', ['glass'], 1), ('wt', ['dark'], 1)],
    octs=[[(0, 3), (-1, 1)], [(0, 2), (-1, 2), (1, 1)], [(-1, 2), (1, 1)]], fifths=0.2, p3=0.45,
    env=((0.5, 5), (2, 6), (0.8, 1.0), (3, 10)), cut=(250, 2500), res=(0.2, 0.55), cut_mod=(0.3, 0.6), fmorph=0.35,
    ftone=[('motion', 3), ('warm', 3), ('vocal', 2), ('thin', 1), ('dark', 1)], post=_sweep_post,
    fx=dict(FX_AMBIENT, phaser=0.3, drive=0.2, sfx=0.25), routes=(8, 12), scenes=0.35, motion=0.3,
    lfo_rates=(0.01, 0.1), mseg_len=(16, 90), bloom=0.4, bloom_len=(4, 20), sub=0.3, noise=0.15,
    styles=['roach', 'berlin-school', 'sydalesis'], moods=['sweep', 'filter', 'resonant', 'moving', 'cyclic', 'phasing', 'tidal', 'hypnotic'], tags=('sweep',))

RC['Dark Ambient'] = dict(cat='Dark Ambient', target=200,
    menuA=[('wt', ['dark'], 4), ('an', ['warm', 'sine'], 2), ('sp', ['drones', 'voices', 'bowls', 'hum', 'earthy'], 3), ('ph', ['bowed', 'scrape'], 2), ('add', ['throat', 'organ', 'hollow'], 2)],
    menuB=[('wt', ['dark', 'earth'], 3), ('sp', ['hum', 'nature', 'bowls', 'voices'], 2), ('ph', ['bowed', 'scrape'], 2), ('gr', ['hum', 'nature', 'textures'], 2), ('an', ['warm', 'sine'], 1), ('add', ['throat', 'hollow'], 1)],
    menuC=[('gr', ['hum', 'nature', 'textures', 'metalfx'], 3), ('ph', ['scrape', 'bowed', 'rain'], 2), ('sp', ['metalfx', 'hum', 'nature'], 1), ('an', ['sine'], 1)],
    octs=[[(-1, 3), (-2, 2), (0, 1)], [(-1, 2), (0, 2), (-2, 1)], [(-2, 1), (0, 1), (1, 1)]], fifths=0.3, p3=0.75,
    env=((3, 12), (4, 10), (0.85, 1.0), (10, 26)), acurve=(-0.8, -0.2), cut=(280, 1800), ftone=[('dark', 4), ('warm', 2), ('motion', 1)],
    tune=0.2, scales=DARK_SCALES, fx=dict(FX_AMBIENT, shift=0.2, tape=0.25, crush=0.05, sfx=0.5, reso=0.25, eq_high=(-8, 0)),
    routes=(10, 15), scenes=0.4, motion=0.3, lfo_rates=(0.005, 0.05), mseg_len=(30, 160), bloom=0.6, bloom_len=(10, 50),
    sub=0.6, noise=0.35, ring=0.1, sp_shift=0.3, macro_core=['depth', 'smear'],
    styles=['sydalesis', 'roach', 'dark-ambient'], moods=['dark', 'abyss', 'ominous', 'cinematic', 'void', 'night', 'industrial', 'cavernous'], tags=('dark',))

def _ritual_post(c):
    """a rhythmic heart for most ritual patches: host-synced tribal shape or a CV step pattern on amp / filter / a layer"""
    r = c.r
    if not _p(r, 0.65): return
    if _p(r, 0.5):
        c.p.lfo(5, 0.1, shape=_ch(r, [SHAPES['tribal'], SHAPES['heart'], _rand_shape(r)]), sync=True, div=_ch(r, ['1/4', '1/2', '1 bar', '1/4.', '1/8']))
        src = 'lfo5'
    else:
        pat = [_ch(r, [0.0, 0.2, 0.5, 1.0]) for _ in range(16)]
        for k in (0, 8): pat[k] = 1.0
        c.p.seq('cv', _ch(r, ['1/8', '1/16', '1/8T', '1/16']), _ch(r, [8, 12, 16, 16]), 0.5, _t(_u(r, 0.0, 0.25)), 0.0, values=[_t(v) for v in pat])
        src = 'seq'
    c.flags.add('pulse')
    _mod(c, src, _ch(r, ['amp', 'f1cut', 'f1cut', 'alevel', 'blevel']), _u(r, 0.15, 0.35))
    if _p(r, 0.5): _mod(c, src, _ch(r, ['f1shape', 'at2', 'noise']), _u(r, 0.1, 0.3))
RC['Ritual & Tribal'] = dict(cat='Ritual & Tribal', target=200,
    menuA=[('sp', ['earthy', 'voices'], 3), ('add', ['throat', 'reed', 'choir'], 3), ('wt', ['earth', 'vocal'], 3), ('ph', ['breath', 'bowed'], 2), ('an', ['warm', 'hollow'], 1)],
    menuB=[('wt', ['earth', 'dark'], 2), ('add', ['throat', 'reed'], 2), ('gr', ['organic', 'nature'], 2), ('sp', ['earthy'], 1), ('ph', ['breath', 'bowed'], 1)],
    menuC=[('gr', ['organic', 'nature'], 3), ('ph', ['membrane', 'rain', 'strike'], 2), ('sp', ['earthy', 'organic'], 1), ('an', ['sine'], 1)],
    octs=[[(-1, 3), (0, 2), (-2, 1)], [(0, 2), (-1, 2), (1, 1)], [(0, 2), (-1, 1), (1, 1)]], fifths=0.5, p3=0.75,
    env=((0.5, 5), (3, 8), (0.85, 1.0), (6, 18)), cut=(300, 3000), ftone=[('vocal', 3), ('warm', 2), ('dark', 1)],
    tune=0.3, scales=TRIBE_SCALES, fx=dict(FX_AMBIENT, tape=0.2, drive=0.15, reso=0.4), post=_ritual_post,
    routes=(9, 13), scenes=0.35, motion=0.3, lfo_rates=(0.008, 0.08), mseg_len=(20, 90), bloom=0.5, bloom_len=(4, 25),
    sub=0.4, noise=0.2, macro_core=['pulse', 'texture'], strike_decay=(0.6, 4),
    styles=['roach', 'tribal', 'corbacho'], moods=['tribal', 'ritual', 'desert', 'dreamtime', 'earth', 'trance', 'ancestral', 'primal'], tags=('ritual', 'tribal'))

# Berlin-school sequences ------------------------------------------------------------------------
MODES = {'aeolian': [0, 2, 3, 5, 7, 8, 10], 'phrygian': [0, 1, 3, 5, 7, 8, 10], 'dorian': [0, 2, 3, 5, 7, 9, 10],
         'minpenta': [0, 3, 5, 7, 10], 'harmminor': [0, 2, 3, 5, 7, 8, 11], 'sus': [0, 2, 5, 7, 10]}
def _berlin_pattern(r):
    mode = MODES[_ch(r, list(MODES))]
    n = _ch(r, [8, 12, 16, 16, 16])
    style = _ch(r, ['pedal', 'pedal', 'arc', 'octaves', 'walk'])
    deg = lambda k: mode[k % len(mode)] + 12 * (k // len(mode))
    notes = []
    k = 0
    for i in range(n):
        if style == 'pedal': notes.append(0 if i % 2 == 0 else deg(_ch(r, [2, 3, 4, 5, 6, 7, 9])))
        elif style == 'octaves': notes.append(_ch(r, [0, 12]) if i % 2 == 0 else deg(_ch(r, [2, 4, 5, 7])))
        elif style == 'arc': notes.append(deg(int(round(4 + 4 * _mm.sin(_mm.pi * 2 * i / n)))) - 12)
        else:
            k = max(-2, min(9, k + _ch(r, [-2, -1, 1, 1, 2])))
            notes.append(deg(k))
    notes = [max(-12, min(24, x)) for x in notes]
    gates = [1 if _p(r, 0.88) else 0 for _ in range(n)]; gates[0] = 1
    return notes, gates
def _berlin_builder(c):
    r = c.r
    _layers(c)
    notes, gates = _berlin_pattern(r)
    c.p.seq('notes', _wch(r, [('1/16', 6), ('1/8', 2), ('1/16T', 1), ('1/8T', 1)]), len(notes), _t(_u(r, 0.3, 0.6)), _t(_u(r, 0.0, 0.2)) if _p(r, 0.3) else 0.0,
            _t(_u(r, 0.05, 0.3)) if _p(r, 0.2) else 0.0, notes, gates)
    c.p.voice('mono', 0.005)
    c.p.lfo(5, 0.1, _ch(r, ['tri', 'sine']), sync=True, div=_ch(r, ['8 bars', '4 bars', '4 bars', '2 bars']))
    _mod(c, 'lfo5', 'f1cut', _u(r, 0.25, 0.5))
    _mod(c, 'alt', 'f1cut', _u(r, 0.05, 0.15))
    c.p.tags('sequenced', 'hold-a-note')
RC['Berlin Sequences'] = dict(cat='Berlin Sequences', target=200, builder=_berlin_builder,
    menuA=[('an', ['warm', 'bright', 'hollow'], 5), ('wt', ['analog'], 2)],
    menuB=[('an', ['warm', 'hollow', 'sine'], 3), ('wt', ['analog', 'warm'], 1)],
    menuC=[('an', ['sine'], 1), ('wt', ['analog'], 1)],
    octs=[[(0, 2), (-1, 2)], [(-1, 2), (0, 1), (1, 1)], [(-1, 1), (1, 1)]], fifths=0.1, p3=0.35, an_voices=[1, 1, 2, 2],
    env=((0.001, 0.006), (0.15, 0.5), (0.0, 0.5), (0.1, 0.5)), acurve=(0.3, 1.0),
    fenv=((0.001, 0.005), (0.12, 0.5), (0.0, 0.3), (0.1, 0.4)), fenv_amt=(0.3, 0.7),
    cut=(250, 1500), res=(0.25, 0.6), cut_mod=(0.15, 0.35), ftone=[('warm', 2), ('dark', 3)],
    fx=dict(chorus=0.5, phaser=0.3, tape=0.3, drive=0.3, sfx=0.15, width=0.4, eq=0.4, reso=0.1), routes=(5, 9), scenes=0.4, motion=0.3,
    lfo_rates=(0.01, 0.1), mseg_len=(16, 64), bloom=0.0, sub=0.35, noise=0.05, vel=(0.2, 0.4), macro_core=['resonance', 'speed'],
    styles=['roach', 'berlin-school', 'tangerine'], moods=['sequencer', 'hypnotic', 'pulsing', 'motorik', 'retro', 'cosmic', 'driving', 'nocturnal'], tags=('berlin',))

RC['Deep Space'] = dict(cat='Deep Space', target=200,
    menuA=[('wt', ['cosmic'], 4), ('add', ['glass', 'primes', 'choir'], 3), ('sp', ['bowls', 'voices', 'drones'], 2), ('gr', ['bowls', 'voices', 'strings'], 1), ('ph', ['bowed', 'rain'], 1)],
    menuB=[('wt', ['cosmic', 'warm'], 3), ('add', ['glass', 'primes'], 2), ('sp', ['bowls', 'textures'], 1), ('an', ['sine', 'hollow'], 1)],
    menuC=[('an', ['sine'], 2), ('wt', ['dark', 'cosmic'], 2), ('gr', ['textures', 'nature'], 1)],
    octs=[[(0, 3), (1, 2), (-1, 1)], [(1, 2), (0, 2), (2, 1)], [(-2, 2), (-1, 2)]], fifths=0.35, p3=0.8,
    env=((4, 14), (5, 10), (0.85, 1.0), (12, 28)), acurve=(-0.9, -0.2), cut=(900, 4500), ftone=[('warm', 3), ('open', 1), ('dark', 1)],
    tune=0.2, scales=SPACE_SCALES, fx=dict(FX_AMBIENT, sfx=0.6, shimmer=0.35, shift=0.25, width=0.7, chorus=0.7, reso=0.25),
    routes=(10, 15), scenes=0.4, motion=0.3, lfo_rates=(0.005, 0.03), mseg_len=(40, 180), bloom=0.65, bloom_len=(12, 50), sub=0.35, noise=0.15,
    macro_core=['shimmer', 'space'], styles=['corbacho', 'roach', 'sydalesis'],
    moods=['cosmic', 'vast', 'stellar', 'weightless', 'luminous', 'nebula', 'orbital', 'infinite'], tags=('space',))

# top-ups for the smaller sections ------------------------------------------------------------------
RC['Choirs & Voices'] = dict(cat='Choirs & Voices', target=150,
    menuA=[('add', ['choir', 'throat'], 3), ('wt', ['vocal'], 3), ('sp', ['voices'], 3), ('gr', ['voices'], 1)],
    menuB=[('add', ['choir'], 2), ('wt', ['vocal', 'warm'], 2), ('sp', ['voices'], 1), ('an', ['hollow'], 1)],
    menuC=[('add', ['glass', 'choir'], 1), ('wt', ['cosmic'], 1), ('an', ['sine'], 1)],
    p3=0.5, fifths=0.25, env=((0.8, 5), (2, 6), (0.85, 1.0), (4, 12)), cut=(1000, 7000), ftone=[('vocal', 3), ('warm', 2), ('open', 1)],
    vib=(0.004, 0.012), tune=0.15, scales=DRONE_SCALES, fx=FX_AMBIENT, routes=(9, 13), scenes=0.4, motion=0.2,
    styles=['roach', 'corbacho'], moods=['choir', 'sacred', 'chant', 'vocal', 'monastic', 'ethereal'], tags=('choir', 'voice'))
RC['Bells & Resonators'] = dict(cat='Bells & Resonators', target=150,
    menuA=[('ph', ['strike'], 5), ('add', ['glass', 'primes'], 2), ('wt', ['metal'], 2)],
    menuB=[('ph', ['strike', 'rain'], 2), ('add', ['glass', 'primes'], 2), ('wt', ['metal', 'cosmic'], 1)],
    menuC=[('an', ['sine'], 1), ('add', ['glass'], 1)],
    p3=0.4, fifths=0.35, env=((0.001, 0.01), (3, 10), (0.0, 0.15), (4, 12)), acurve=(0.5, 1.0), strike_decay=(3, 14),
    cut=(2500, 14000), ftone=[('open', 3), ('warm', 1)], tune=0.15, scales=ALL_JI, fx=dict(FX_AMBIENT, reso=0.6, sfx=0.3),
    routes=(7, 11), scenes=0.25, motion=0.2, bloom=0.2, styles=['corbacho', 'roach'], moods=['bell', 'metallic', 'glass', 'gamelan', 'temple', 'ringing'], tags=('bell',))
RC['Physical Models'] = dict(cat='Physical Models', target=150,
    menuA=[('ph', ['bowed', 'breath', 'rain', 'scrape', 'strike', 'pluck'], 6), ('add', ['strings', 'reed'], 1)],
    menuB=[('ph', ['bowed', 'breath', 'rain', 'strike'], 3), ('add', ['glass', 'strings'], 1), ('wt', ['strings', 'earth'], 1)],
    menuC=[('ph', ['bowed', 'rain'], 1), ('an', ['sine'], 1)],
    p3=0.4, fifths=0.3, env=((0.2, 4), (2, 8), (0.6, 1.0), (3, 12)), cut=(1500, 10000), ftone=[('open', 2), ('warm', 2)],
    tune=0.12, scales=ALL_JI, fx=dict(FX_AMBIENT, reso=0.4), routes=(8, 12), scenes=0.3, motion=0.2, strike_decay=(2, 10),
    styles=['roach', 'corbacho'], moods=['physical', 'acoustic', 'bowed', 'resonant', 'woody', 'organic'], tags=('physical',))
RC['Winds & Reeds'] = dict(cat='Winds & Reeds', target=100,
    menuA=[('ph', ['breath'], 5), ('add', ['reed', 'throat'], 2), ('sp', ['earthy'], 1)],
    menuB=[('add', ['reed', 'throat', 'hollow'], 2), ('sp', ['earthy'], 1), ('wt', ['earth'], 1), ('ph', ['breath'], 1)],
    menuC=[('gr', ['organic', 'nature'], 1), ('an', ['sine'], 1)],
    p3=0.35, fifths=0.3, env=((0.1, 1.5), (1, 4), (0.8, 1.0), (1, 5)), cut=(1200, 8000), ftone=[('vocal', 2), ('open', 2), ('warm', 1)],
    vib=(0.005, 0.012), tune=0.2, scales=TRIBE_SCALES, fx=FX_AMBIENT, routes=(7, 11), scenes=0.25, motion=0.2, bloom=0.3,
    styles=['roach', 'tribal'], moods=['breath', 'flute', 'reed', 'desert', 'ancient', 'wooden'], tags=('wind',))
RC['Just Intonation'] = dict(RC['Drones'], cat='Just Intonation', target=150, tune=1.0, scales=ALL_JI, variant='ji',
    styles=['roach', 'corbacho'], moods=['just-intonation', 'pure', 'harmonic', 'sacred', 'resonant', 'still'], tags=('tuned',))
RC['FX & Details'] = dict(RC['Textures'], cat='FX & Details', target=150, variant='fx', env=((0.5, 6), (3, 10), (0.5, 1.0), (6, 20)),
    bloom=0.9, bloom_len=(3, 15), cut=(400, 12000), fx=dict(FX_AMBIENT, sfx=0.6, shift=0.3, crush=0.1), macro_core=['bloom', 'smear'],
    moods=['swell', 'riser', 'dissolve', 'impact', 'transition', 'cinematic'], tags=('fx',))
RC['Keys & Organs'] = dict(cat='Keys & Organs', target=60,
    menuA=[('add', ['organ'], 3), ('wt', ['organ'], 3), ('an', ['hollow', 'warm'], 2), ('ph', ['pluck', 'strike'], 2)],
    menuB=[('add', ['organ', 'glass'], 2), ('wt', ['organ', 'warm'], 1), ('an', ['sine'], 1)],
    menuC=[('an', ['sine'], 1)],
    p3=0.3, env=((0.003, 0.05), (1.5, 5), (0.2, 0.8), (1, 4)), cut=(1200, 8000), ftone=[('warm', 2), ('open', 2)],
    fx=dict(FX_AMBIENT, sfx=0.15, phaser=0.25), routes=(6, 10), scenes=0.2, motion=0.15, bloom=0.2,
    styles=['roach', 'corbacho'], moods=['keys', 'organ', 'warm', 'vintage', 'sacred'], tags=('keys',))
def _mono(c, glide=(0.02, 0.15), mode='legato'):
    c.p.voice(mode, _t(_u(c.r, *glide)))
def _bass_builder(c): _layers(c); _mono(c, (0.005, 0.08), _ch(c.r, ['mono', 'legato']))
def _lead_builder(c): _layers(c); _mono(c, (0.03, 0.18), 'legato')
RC['Basses'] = dict(cat='Basses', target=60, builder=_bass_builder,
    menuA=[('an', ['warm', 'sine', 'hollow'], 3), ('wt', ['dark', 'analog'], 2), ('add', ['organ', 'hollow'], 1)],
    menuB=[('an', ['sine', 'warm'], 2), ('wt', ['dark'], 1)], menuC=[('an', ['sine'], 1)],
    octs=[[(-1, 2), (-2, 1)], [(-1, 1), (-2, 1), (0, 1)], [(-2, 1)]], p3=0.3, env=((0.002, 0.03), (0.4, 2), (0.5, 1.0), (0.3, 1.5)),
    fenv=((0.001, 0.02), (0.2, 1.0), (0.1, 0.5), (0.2, 1.0)), fenv_amt=(0.2, 0.5), cut=(150, 900), res=(0.1, 0.35), ftone=[('dark', 2), ('warm', 2)],
    fx=dict(chorus=0.3, drive=0.3, tape=0.2, eq=0.4, width=0.1), routes=(5, 8), scenes=0.2, motion=0.15, bloom=0.1, sub=0.5,
    styles=['roach', 'sydalesis'], moods=['bass', 'deep', 'analog', 'dark', 'pulse'], tags=('bass',))
RC['Leads'] = dict(cat='Leads', target=60, builder=_lead_builder,
    menuA=[('an', ['warm', 'bright'], 3), ('wt', ['vocal', 'warm', 'strings'], 2), ('add', ['reed', 'choir', 'hollow'], 2), ('ph', ['breath', 'bowed'], 1)],
    menuB=[('an', ['warm', 'hollow'], 2), ('wt', ['warm'], 1), ('add', ['glass'], 1)], menuC=[('an', ['sine'], 1)],
    octs=[[(0, 2), (1, 1)], [(0, 1), (-1, 1), (1, 1)], [(-1, 1)]], p3=0.3, env=((0.01, 0.2), (0.5, 2), (0.7, 1.0), (0.4, 2)),
    fenv=((0.01, 0.2), (0.5, 2), (0.3, 0.8), (0.4, 2)), fenv_amt=(0.1, 0.35), cut=(800, 5000), ftone=[('warm', 3), ('vocal', 1)],
    vib=(0.006, 0.014), fx=dict(FX_AMBIENT, sfx=0.25, reso=0.15), routes=(6, 10), scenes=0.2, motion=0.15, bloom=0.2,
    styles=['roach', 'sydalesis'], moods=['lead', 'lyrical', 'singing', 'expressive', 'gliding'], tags=('lead',))
def _pluck_builder(c):
    _layers(c)
    if _p(c.r, 0.5):
        notes, gates = _berlin_pattern(c.r)
        c.p.seq('notes', _ch(c.r, ['1/16', '1/8', '1/16T']), len(notes), _t(_u(c.r, 0.3, 0.6)), 0.0, 0.0, notes, gates).voice('mono', 0.005)
        c.p.tags('sequenced', 'hold-a-note')
RC['Sequence & Plucks'] = dict(cat='Sequence & Plucks', target=80, builder=_pluck_builder,
    menuA=[('ph', ['pluck', 'strike', 'membrane'], 3), ('an', ['warm', 'hollow'], 2), ('wt', ['metal', 'analog', 'earth'], 2), ('add', ['glass'], 1)],
    menuB=[('ph', ['strike', 'pluck'], 1), ('an', ['sine', 'hollow'], 1), ('add', ['glass'], 1)], menuC=[('an', ['sine'], 1)],
    p3=0.3, env=((0.001, 0.005), (0.3, 1.5), (0.0, 0.2), (0.3, 2)), acurve=(0.5, 1.0), strike_decay=(0.5, 4),
    fenv=((0.001, 0.005), (0.2, 0.8), (0.0, 0.3), (0.2, 1.0)), fenv_amt=(0.2, 0.5), cut=(600, 5000), ftone=[('warm', 2), ('open', 2)],
    fx=dict(FX_AMBIENT, sfx=0.2, reso=0.3, phaser=0.2), routes=(6, 10), scenes=0.2, motion=0.25, bloom=0.0,
    styles=['roach', 'berlin-school'], moods=['pluck', 'sequence', 'percussive', 'rhythmic', 'kalimba'], tags=('pluck',))
RC['Performance'] = dict(RC['Atmospheres'], cat='Performance', target=60, variant='perf', scenes=0.6, motion=1.0,
    moods=['performance', 'scenes', 'motion', 'journey', 'evolving'], tags=('performance',))

# ------------------------------------------------------------------------------------------- names
# 5.1.2: grit words are enforced in gen_presets3.emit() (grit presets removed, Burnished / Burning renamed); listed here for reference
_GRIT_WORDS = {'acid', 'distorted', 'overdriven', 'fuzz', 'saturated', 'crushed', 'lo-fi', 'lofi', 'broken', 'gritty', 'grit', 'burn', 'burnt', 'burning', 'burnished', 'dirt', 'dirty', 'grunge'}
_BLOCK = {'structures from silence', 'dreamtime return', 'magnificent void', 'mystic chords', 'western spaces', 'quiet music', 'skeleton keys',
          'world\'s edge', 'origins', 'the serpent\'s lair', 'nocturnes', 'ars lucis', 'splendour', 'the talisman\'s sleep', 'vapor', 'arcanum',
          'the resonant earth', 'lunar phases', 'early man', 'empetus', 'now', 'trance spaces'}
W_COMMON_A = ['Slow', 'Ancient', 'Hollow', 'Distant', 'Silent', 'Endless', 'Faded', 'Veiled', 'Burnished', 'Amber', 'Cobalt', 'Ashen', 'Lucid',
              'Sunken', 'Glacial', 'Molten', 'Pale', 'Velvet', 'Sacred', 'Quiet', 'Long', 'Inner', 'Ember', 'Tidal', 'Sleeping', 'Wandering',
              'Drifting', 'Patient', 'Weathered', 'Low', 'Golden', 'Iron', 'Salt', 'Stone', 'Smoke', 'Night', 'Dawn', 'Dusk', 'Winter', 'Summer']
W = {
 'Drones': (['Basalt', 'Obsidian', 'Earthen', 'Monolithic', 'Ochre', 'Cathedral', 'Resonant', 'Unbroken', 'Primordial', 'Gravity', 'Solar', 'Lunar', 'Bronze', 'Granite', 'Deep'],
            ['Hymn', 'Psalm', 'Litany', 'Drone', 'Vigil', 'Mantra', 'Pillar', 'Field', 'Bell Floor', 'Root', 'Fundamental', 'Chord', 'Monolith', 'Choir', 'Engine', 'Breath', 'Furnace', 'Altar', 'Current', 'Horizon', 'Meridian', 'Keel', 'Spire', 'Ground', 'Hum', 'Tone', 'Column', 'Crypt', 'Well', 'Pulse'],
            ['the Deep', 'Salt', 'Embers', 'Stillness', 'Iron', 'the Old Earth', 'Basalt', 'the Plain', 'Long Shadows', 'Cedar', 'Ash', 'the Fifth', 'the Root', 'Slow Light', 'Mountains'],
            ['Sonoran', 'Atacama', 'Altiplano', 'Kalahari', 'Gobi', 'Mojave', 'Nullarbor', 'Tibetan', 'Andean', 'Saharan', 'Nordic', 'Arctic', 'Magellan', 'Pyrenean', 'Iberian']),
 'Pads': (['Warm', 'Luminous', 'Breathing', 'Tender', 'Soft', 'Honeyed', 'Glowing', 'Sunlit', 'Gentle', 'Rose', 'Silken', 'Lantern', 'Woven', 'Halcyon', 'Mellow'],
          ['Pad', 'Haze', 'Veil', 'Glow', 'Embrace', 'Cloud', 'Weave', 'Bloom', 'Tapestry', 'Canopy', 'Shelter', 'Linen', 'Harbour', 'Garden', 'Memory', 'Lullaby', 'Promise', 'Warmth', 'Light', 'Hours', 'Room', 'Window', 'Season', 'Letter', 'Morning', 'Afternoon', 'Hearth', 'Cradle', 'Orchard', 'Meadow'],
          ['Light', 'Summer', 'Honey', 'Home', 'Small Hours', 'Rain', 'Kindness', 'Late Sun', 'Old Photographs', 'Distant Rooms', 'Calm', 'Velvet', 'Peace', 'Afterglow', 'Evenings'],
          ['Analog', 'Vintage', 'Polyphonic', 'Celestial', 'Pastoral', 'Coastal', 'Northern', 'Southern', 'Morning', 'Evening', 'Twilight', 'Sunday', 'October', 'April', 'Midsummer']),
 'Atmospheres': (['Ethereal', 'Weightless', 'Suspended', 'Vaporous', 'Translucent', 'Airborne', 'Nebulous', 'Hazy', 'Misted', 'Floating', 'Diaphanous', 'Gossamer', 'Phantom', 'Spectral', 'Elysian'],
                 ['Air', 'Mist', 'Fog', 'Vapour', 'Ether', 'Aura', 'Halo', 'Breath', 'Drift', 'Sky', 'Cirrus', 'Stratus', 'Updraft', 'Thermal', 'Shroud', 'Gauze', 'Reverie', 'Dream', 'Hover', 'Glide', 'Expanse', 'Openness', 'Void', 'Whisper', 'Murmur', 'Sigh', 'Lull', 'Wake', 'Trance', 'Veil'],
                 ['the Upper Air', 'Mist', 'Clouds', 'Weightlessness', 'Distance', 'High Places', 'Thin Air', 'Morning Fog', 'Silence', 'Altitude', 'the Valley', 'the Pass', 'Snowfields', 'Lakes', 'Heights'],
                 ['Alpine', 'Highland', 'Cloud', 'Sky', 'Horizon', 'Valley', 'Lake', 'Glacier', 'Plateau', 'Summit', 'Coastline', 'Island', 'Tundra', 'Fjord', 'Steppe']),
 'Textures': (['Granular', 'Brittle', 'Crackling', 'Dusty', 'Fractured', 'Rustling', 'Tactile', 'Scattered', 'Splintered', 'Glittering', 'Frayed', 'Woven', 'Gritty', 'Shimmering', 'Restless'],
              ['Dust', 'Grain', 'Static', 'Sand', 'Gravel', 'Ash Fall', 'Pollen', 'Frost', 'Lichen', 'Bark', 'Splinters', 'Embers', 'Mica', 'Filament', 'Fibre', 'Rust', 'Patina', 'Moss', 'Shards', 'Sparks', 'Crystals', 'Seeds', 'Threads', 'Beads', 'Flakes', 'Particles', 'Fragments', 'Glints', 'Rattles', 'Scratches'],
              ['Glass', 'Sand', 'Rust', 'Lichen', 'Small Things', 'Insects', 'Ice', 'Paper', 'Copper', 'Wire', 'Bone', 'Dry Leaves', 'Seeds', 'Mica', 'Static'],
              ['Micro', 'Copper', 'Glass', 'Paper', 'Clay', 'Bamboo', 'Reed', 'Tin', 'Brass', 'Quartz', 'Cedar', 'Willow', 'Flint', 'Silica', 'Graphite']),
 'Soundscapes': (['Windswept', 'Rain-soaked', 'Moonlit', 'Sunbaked', 'Forgotten', 'Abandoned', 'Overgrown', 'Flooded', 'Frozen', 'Midnight', 'Desert', 'Coastal', 'Forest', 'Canyon', 'Highland'],
                 ['Shoreline', 'Canyon', 'Basin', 'Mesa', 'Arroyo', 'Delta', 'Estuary', 'Rainforest', 'Monastery', 'Ruins', 'Harbour', 'Cavern', 'Riverbed', 'Plateau', 'Dunes', 'Marsh', 'Grove', 'Valley', 'Crossing', 'Outpost', 'Temple', 'Lighthouse', 'Quarry', 'Railyard', 'Cistern', 'Orchard', 'Glade', 'Ridge', 'Inlet', 'Oasis'],
                 ['the Dunes', 'Rain', 'the River', 'Wolves', 'the Monsoon', 'Night Birds', 'the Tide', 'Cicadas', 'Thunder', 'the Old Road', 'Lanterns', 'the Pilgrims', 'Smoke', 'Falling Water', 'the Herons'],
                 ['Sonoran', 'Mekong', 'Amazon', 'Kyoto', 'Andalusian', 'Laurentian', 'Saharan', 'Patagonian', 'Hebridean', 'Anatolian', 'Kimberley', 'Yukon', 'Bengal', 'Mongolian', 'Baltic']),
 'Spectral': (['Smeared', 'Frozen', 'Stretched', 'Liquid', 'Ghosted', 'Melted', 'Dissolved', 'Spectral', 'Haunted', 'Inverted', 'Resynthesised', 'Mirrored', 'Suspended', 'Bleeding', 'Shadowed'],
              ['Choir', 'Bell', 'Voice', 'Prism', 'Spectrum', 'Partials', 'Afterimage', 'Imprint', 'Echo Form', 'Residue', 'Apparition', 'Mirage', 'Hologram', 'Phantom', 'Overtones', 'Breath', 'Remnant', 'Trace', 'Shadow', 'Relic', 'Fresco', 'Negative', 'Palimpsest', 'Lens', 'Refraction', 'Halo', 'Ghost', 'Bloom', 'Wraith', 'Lattice'],
              ['Light', 'Voices', 'Glass', 'Time', 'Memory', 'Bells', 'Breath', 'a Choir', 'Water', 'Ice', 'Smoke', 'Distance', 'Bronze', 'Strings', 'Dust'],
              ['Spectral', 'Frozen', 'Prismatic', 'Inverse', 'Phase', 'Fourier', 'Harmonic', 'Partial', 'Vocoded', 'Morphed', 'Formant', 'Sparse', 'Blurred', 'Stretched', 'Slowed']),
 'Sweeps': (['Rising', 'Turning', 'Circling', 'Spiralling', 'Sweeping', 'Cycling', 'Phasing', 'Orbiting', 'Wheeling', 'Swaying', 'Revolving', 'Ebbing', 'Surging', 'Tidal', 'Pendulum'],
            ['Sweep', 'Arc', 'Tide', 'Cycle', 'Spiral', 'Wheel', 'Orbit', 'Gyre', 'Sway', 'Crest', 'Swell', 'Wave', 'Current', 'Turn', 'Phase', 'Vortex', 'Pendulum', 'Helix', 'Loop', 'Rotation', 'Ascent', 'Descent', 'Filter Sea', 'Motion', 'Sunrise', 'Sundown', 'Storm Front', 'Comet Tail', 'Lighthouse', 'Rotor'],
             ['Tides', 'Light', 'Seasons', 'the Moon', 'Wind', 'Planets', 'the Sea', 'Dunes', 'Clouds', 'Hours', 'Orbits', 'Breath', 'Weather', 'Comets', 'Shadows'],
            ['Analog', 'Resonant', 'Ladder', 'Comb', 'Phaser', 'Formant', 'Vowel', 'Notch', 'Cosmic', 'Berlin', 'Slow', 'Wide', 'Deep', 'High', 'Lunar']),
 'Dark Ambient': (['Abyssal', 'Sunless', 'Black', 'Hollow', 'Buried', 'Drowned', 'Ruined', 'Sepulchral', 'Lightless', 'Cold', 'Leaden', 'Sunken', 'Nocturnal', 'Funereal', 'Brooding'],
                  ['Abyss', 'Void', 'Chasm', 'Crypt', 'Vault', 'Undertow', 'Mausoleum', 'Catacomb', 'Bunker', 'Furnace', 'Reactor', 'Hull', 'Shaft', 'Trench', 'Pit', 'Machine', 'Ossuary', 'Tunnel', 'Wreck', 'Cistern', 'Rift', 'Maw', 'Gate', 'Nadir', 'Eclipse', 'Omen', 'Requiem', 'Dread', 'Pressure', 'Signal'],
                  ['the Deep', 'Iron', 'No Return', 'the Buried', 'Old Machines', 'Ash', 'the Drowned', 'Black Water', 'the Last Light', 'Rust', 'Concrete', 'Static', 'Cold Stars', 'Lead', 'the Void'],
                  ['Subterranean', 'Industrial', 'Arctic', 'Orbital', 'Mariana', 'Siberian', 'Chernobyl', 'Midnight', 'Basement', 'Tunnel', 'Polar', 'Blackwater', 'Undersea', 'Derelict', 'Hadal']),
 'Ritual & Tribal': (['Tribal', 'Ancestral', 'Earthen', 'Ceremonial', 'Shamanic', 'Totemic', 'Ochre', 'Painted', 'Burning', 'Sacred', 'Nomad', 'Primal', 'Dreaming', 'Red', 'Rattling'],
                     ['Ritual', 'Ceremony', 'Rite', 'Fire Circle', 'Totem', 'Songline', 'Dreaming', 'Trance', 'Procession', 'Invocation', 'Offering', 'Initiation', 'Gathering', 'Dance', 'Drum Spirit', 'Ochre', 'Bone Flute', 'Rattle', 'Smoke Signal', 'Vision', 'Journey', 'Summoning', 'Ancestors', 'Hearth', 'Medicine', 'Ember Song', 'Kiva', 'Petroglyph', 'Canyon Song', 'Heartbeat'],
                     ['the Ancestors', 'Fire', 'the Red Earth', 'Smoke', 'the Clan', 'Rain', 'the Serpent', 'the Hunt', 'Bones', 'Stars', 'Ochre', 'the Spirit', 'the Old Ones', 'Dust', 'the Mesa'],
                     ['Kimberley', 'Arnhem', 'Sonoran', 'Hopi', 'Anasazi', 'Kalahari', 'Sahel', 'Andean', 'Mayan', 'Tuvan', 'Saami', 'Berber', 'Navajo', 'Uluru', 'Altai']),
 'Berlin Sequences': (['Motorik', 'Analog', 'Pulsing', 'Electric', 'Midnight', 'Neon', 'Chrome', 'Circuit', 'Kinetic', 'Phosphor', 'Rotating', 'Cascading', 'Mechanical', 'Nightbound', 'Restless'],
                      ['Sequence', 'Ostinato', 'Pulse', 'Circuit', 'Transit', 'Autobahn', 'Pattern', 'Cascade', 'Machine', 'Relay', 'Run', 'Grid', 'Motor', 'Signal', 'Lattice', 'Spindle', 'Dynamo', 'Turbine', 'Shuttle', 'Corridor', 'Convoy', 'Express', 'Orbit', 'Clockwork', 'Velocity', 'Momentum', 'Arpeggio', 'Loop', 'Network', 'Station'],
                      ['Steel', 'Neon', 'Night Trains', 'Lights', 'the City', 'Voltage', 'Chrome', 'Wires', 'the Grid', 'Satellites', 'Glass', 'Motion', 'Signals', 'Engines', 'Stars'],
                      ['Berlin', 'Teutonic', 'Kosmische', 'Ferro', 'Quartz', 'Carbon', 'Tangent', 'Kobalt', 'Nacht', 'Hamburg', 'Dusseldorf', 'Cologne', 'Leipzig', 'Dresden', 'Potsdam']),
 'Deep Space': (['Stellar', 'Interstellar', 'Galactic', 'Orbital', 'Cosmic', 'Celestial', 'Luminous', 'Infinite', 'Radiant', 'Boundless', 'Silver', 'Starborn', 'Polar', 'Zenith', 'Ionic'],
                ['Nebula', 'Pulsar', 'Quasar', 'Aurora', 'Corona', 'Horizon', 'Orbit', 'Eclipse', 'Solstice', 'Equinox', 'Stardust', 'Comet', 'Meridian', 'Heliosphere', 'Constellation', 'Galaxy', 'Starfield', 'Event Horizon', 'Parallax', 'Perihelion', 'Lightyear', 'Zenith', 'Cosmos', 'Ion Storm', 'Magnetar', 'Redshift', 'Exoplanet', 'Lagrange', 'Halo Ring', 'Starlight'],
                ['the Stars', 'Light', 'Andromeda', 'Orion', 'the Pleiades', 'Saturn', 'Distant Suns', 'Cold Light', 'Vega', 'the Milky Way', 'Lyra', 'Cygnus', 'Silence', 'Infinity', 'Neptune'],
                ['Andromeda', 'Orion', 'Cygnus', 'Lyra', 'Vega', 'Sirius', 'Polaris', 'Rigel', 'Altair', 'Arcturus', 'Deneb', 'Antares', 'Capella', 'Kepler', 'Europa']),
}
# smaller sections share pools
W['Choirs & Voices'] = (['Monastic', 'Cloistered', 'Chanting', 'Vesper', 'Choral', 'Distant', 'Hushed'], ['Choir', 'Chant', 'Voices', 'Psalm', 'Canticle', 'Antiphon', 'Plainsong', 'Vespers', 'Matins', 'Hymnal', 'Chorale', 'Cantor'], ['Monks', 'Voices', 'the Cloister', 'Candles', 'Stone'], ['Gregorian', 'Byzantine', 'Tibetan', 'Bulgarian', 'Tuvan', 'Georgian', 'Celtic'])
W['Bells & Resonators'] = (['Bronze', 'Temple', 'Singing', 'Struck', 'Ringing', 'Tuned', 'Glass'], ['Bells', 'Gongs', 'Bowl', 'Chimes', 'Carillon', 'Tines', 'Gamelan', 'Bars', 'Resonator', 'Peal', 'Toll', 'Kettle'], ['Bronze', 'Glass', 'the Temple', 'Morning', 'Silver'], ['Balinese', 'Javanese', 'Tibetan', 'Kyoto', 'Nepalese', 'Burmese', 'Thai'])
W['Physical Models'] = (['Bowed', 'Plucked', 'Breathed', 'Struck', 'Scraped', 'Hollow', 'Wooden'], ['Strings', 'Plate', 'Tube', 'Membrane', 'Bow', 'Body', 'Soundboard', 'Bridge', 'Monochord', 'Bar', 'Box', 'Cylinder'], ['Wood', 'Gut', 'Brass', 'Cane', 'Hide'], ['Acoustic', 'Resonant', 'Sympathetic', 'Modal', 'Coupled', 'Damped', 'Nonlinear'])
W['Winds & Reeds'] = (['Breathy', 'Hollow', 'Desert', 'Bamboo', 'Reed', 'Cane', 'Bone'], ['Flute', 'Reed', 'Pipe', 'Shawm', 'Ney', 'Duduk', 'Whistle', 'Panpipe', 'Ocarina', 'Horn', 'Bansuri', 'Didge'], ['the Wind', 'Breath', 'Cane', 'the Canyon', 'Dusk'], ['Andean', 'Persian', 'Armenian', 'Japanese', 'Bedouin', 'Celtic', 'Hindustani'])
W['Just Intonation'] = (['Pure', 'Just', 'Harmonic', 'Septimal', 'Otonal', 'Natural', 'Untempered'], ['Ratios', 'Fifths', 'Lattice', 'Overtones', 'Series', 'Monochord', 'Consonance', 'Intervals', 'Partials', 'Tonality', 'Diamond', 'Spiral'], ['Ratios', 'Pure Fifths', 'Overtones', 'Seven', 'the Series'], ['Pythagorean', 'Ptolemaic', 'Septimal', 'Undecimal', 'Harmonic', 'Otonal', 'Utonal'])
W['FX & Details'] = (['Rising', 'Falling', 'Shattered', 'Dissolving', 'Distant', 'Sudden', 'Slow'], ['Swell', 'Riser', 'Impact', 'Fall', 'Dissolve', 'Transition', 'Wash', 'Flare', 'Crack', 'Bloom', 'Collapse', 'Surge'], ['Light', 'Glass', 'Thunder', 'Static', 'Dust'], ['Cinematic', 'Spectral', 'Granular', 'Reverse', 'Tidal', 'Seismic', 'Solar'])
W['Keys & Organs'] = (['Warm', 'Chapel', 'Velvet', 'Faded', 'Tape', 'Wooden', 'Soft'], ['Organ', 'Keys', 'Harmonium', 'Celesta', 'Piano Ghost', 'Reed Organ', 'Clavier', 'Positive', 'Manual', 'Stops', 'Register', 'Console'], ['the Chapel', 'Wood', 'Dust', 'Evening', 'Candles'], ['Baroque', 'Gothic', 'Parlour', 'Vintage', 'Chapel', 'Cathedral', 'Village'])
W['Basses'] = (['Deep', 'Sub', 'Heavy', 'Low', 'Dark', 'Round', 'Warm'], ['Bass', 'Floor', 'Foundation', 'Undertone', 'Keel', 'Bedrock', 'Pedal', 'Ballast', 'Anchor', 'Root', 'Trench', 'Hull'], ['the Deep', 'Iron', 'Earth', 'Stone', 'Night'], ['Analog', 'Moog', 'Subterranean', 'Tectonic', 'Seismic', 'Abyssal', 'Granite'])
W['Leads'] = (['Singing', 'Lyrical', 'Lonely', 'Gliding', 'Soaring', 'Plaintive', 'Silver'], ['Lead', 'Line', 'Voice', 'Song', 'Call', 'Cry', 'Melody', 'Solo', 'Aria', 'Lament', 'Thread', 'Flight'], ['the Canyon', 'Dusk', 'Longing', 'the Hawk', 'Rain'], ['Analog', 'Western', 'Desert', 'Lyric', 'Sustained', 'Portamento', 'Legato'])
W['Sequence & Plucks'] = (['Plucked', 'Tumbling', 'Rippling', 'Bright', 'Glassy', 'Wooden', 'Tiny'], ['Plucks', 'Ripples', 'Kalimba', 'Steps', 'Cascade', 'Pattern', 'Drops', 'Tines', 'Patter', 'Spiral', 'Rain Steps', 'Mallets'], ['Glass', 'Rain', 'Wood', 'Bells', 'Pebbles'], ['Kalimba', 'Gamelan', 'Marimba', 'Celesta', 'Koto', 'Harp', 'Mbira'])
W['Performance'] = (['Shifting', 'Living', 'Turning', 'Evolving', 'Unfolding', 'Changing', 'Morphing'], ['Journey', 'Passage', 'Scenes', 'Seasons', 'Weather', 'Voyage', 'Chapters', 'Movements', 'Pilgrimage', 'Crossing', 'Migration', 'Odyssey'], ['Seasons', 'Light', 'Weather', 'Tides', 'Time'], ['Morph', 'Scene', 'Motion', 'Macro', 'Live', 'Gesture', 'Performer'])

EXISTING_NAMES = set(p.name for p in PRESETS)
def make_names(cat, n, used):
    A, N, O, PL = W[cat]
    A = A + W_COMMON_A
    r = _R('names', cat)
    out = []
    tries = 0
    while len(out) < n and tries < n * 400:
        tries += 1
        f = _wch(r, [(0, 45), (1, 22), (2, 20), (3, 13)])
        if f == 0: nm = '%s %s' % (_ch(r, A), _ch(r, N))
        elif f == 1: nm = '%s of %s' % (_ch(r, N), _ch(r, O))
        elif f == 2: nm = '%s %s' % (_ch(r, PL), _ch(r, N))
        else: nm = '%s %s %s' % (_ch(r, A), _ch(r, PL), _ch(r, N))
        words = nm.split()
        if len(nm) > 25 or len(set(w.lower() for w in words)) < len(words): continue
        if nm.lower() in _BLOCK or any(b in nm.lower() for b in _BLOCK if len(b) > 6): continue
        if nm in used: continue
        used.add(nm); out.append(nm)
    assert len(out) == n, (cat, len(out), n)
    return out

# ------------------------------------------------------------------------------------------- generate
_GEN_ORDER = ['Drones', 'Pads', 'Atmospheres', 'Textures', 'Soundscapes', 'Spectral', 'Sweeps', 'Dark Ambient', 'Ritual & Tribal', 'Berlin Sequences',
              'Deep Space', 'Choirs & Voices', 'Bells & Resonators', 'Physical Models', 'Winds & Reeds', 'Just Intonation', 'FX & Details',
              'Keys & Organs', 'Basses', 'Leads', 'Sequence & Plucks', 'Performance']
_USED = set(EXISTING_NAMES)
import json as _js
_rp = _os.path.join(HERE, 'reroll51.json')
_REROLL = _js.load(open(_rp)) if _os.path.exists(_rp) else {}
GEN51 = []
from collections import Counter as _Counter
_have = _Counter(p.cat for p in PRESETS)
for _cat in _GEN_ORDER:
    _rc = RC[_cat]
    _n = max(0, _rc['target'] - _have.get(_cat, 0))
    for _i, _nm in enumerate(make_names(_cat, _n, _USED)):
        # presets the audit rejected (harsh / static / phasey / too heavy) are re-rolled under the same name
        GEN51.append(compose(_rc, _nm, _i + 100000 * _REROLL.get(_nm, 0)))
