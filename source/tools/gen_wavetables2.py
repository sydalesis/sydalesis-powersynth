#!/usr/bin/env python3
"""Driftfield 3.1 wavetable expansion.

Adds ~120 tables on top of gen_wavetables.py. Same format (Serum/Vital compatible, 2048-sample
frames, 'clm ' chunk) but written as 16-bit PCM to keep the download small.
Usage: gen_wavetables2.py <output dir>
"""
import os, sys, struct
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_wavetables as G
from gen_wavetables import N, F, H, h, P, lerp, lp, bump, SAW, SQR, TRI, spec_to_wave, time_to_wave, formants, VOW, FUND

TABLES = []


def table(cat, name):
    def deco(fn):
        TABLES.append((cat, name, fn))
        return fn
    return deco


def frame_of(t):
    return int(round(t * (F - 1)))


SINE = np.where(h == 1, 1.0, 0.0)


def harmset(ratios, rolloff=1.0, maxh=H):
    """Amplitudes for a chord expressed as harmonic numbers (and their multiples)."""
    a = np.zeros(H)
    for r in ratios:
        k = np.arange(r, maxh + 1, r)
        a[k - 1] += 1.0 / (k / r) ** rolloff
    return a


def drift_keys(seed, keys, shape):
    rng = np.random.default_rng(seed)
    return rng.random((keys,) + shape)


def cos_interp(keys, t):
    x = t * (len(keys) - 1)
    i = min(int(x), len(keys) - 2)
    fr = 0.5 - 0.5 * np.cos(np.pi * (x - i))
    return keys[i] * (1 - fr) + keys[i + 1] * fr


def step_interp(keys, t):
    return keys[min(int(t * len(keys)), len(keys) - 1)]


# =========================================================================== Drift Spectra (slow random walks)
for i in range(12):
    seed = 300 + i
    tilt = 0.7 + 0.08 * (i % 5)
    bright = [8, 14, 22, 36][i % 4]
    oddness = (i % 3) * 0.35

    def mk(seed=seed, tilt=tilt, bright=bright, oddness=oddness):
        keys = drift_keys(seed, 7, (H,))
        def fn(t):
            k = cos_interp(keys, t)
            base = h ** -tilt * lp(bright * (0.7 + 0.6 * np.sin(np.pi * t) ** 2), 3)
            odd = np.where(h % 2 == 1, 1.0, 1.0 - oddness)
            return base * odd * (0.25 + 1.5 * k ** 2)
        return fn
    names = ['Ember', 'Haze', 'Lumen', 'Dune', 'Cinder', 'Nimbus', 'Tide', 'Ochre', 'Vapour', 'Lichen', 'Halo', 'Ash']
    table('Drift Spectra', f'Drift {names[i]}')(mk())

# =========================================================================== Chord tables (harmonic stacks)
CHORDS = {
    'Fifths': [2, 3], 'Major': [4, 5, 6], 'Minor': [10, 12, 15], 'Sus2': [8, 9, 12], 'Sus4': [6, 8, 9],
    'Major 7': [8, 10, 12, 15], 'Minor 9': [10, 12, 15, 18, 22], 'Quartal': [9, 12, 16], 'Overtone 7': [4, 5, 6, 7],
    'Cluster': [16, 17, 18, 19],
}
for name, rs in CHORDS.items():
    def mk(rs=rs):
        a = harmset(rs, 1.0)
        def fn(t):
            return a * lp(lerp(rs[0] * 1.5, 120, t ** 1.6), 3) + SINE * 0.3 * (1 - t)
        return fn
    table('Chords', f'Chord {name}')(mk())

# =========================================================================== Vowel journeys
JOURNEYS = [('bass', 'aou', 'Bass A-O-U'), ('bass', 'uoa', 'Bass U-O-A'), ('tenor', 'iea', 'Tenor I-E-A'),
            ('tenor', 'aeiou', 'Tenor Full Circle'), ('alto', 'uoaei', 'Alto Bloom'), ('alto', 'oa', 'Alto O-A'),
            ('soprano', 'aei', 'Soprano Rise'), ('soprano', 'uoau', 'Soprano Breath')]
for voice, seq, name in JOURNEYS:
    def mk(voice=voice, seq=seq):
        return lambda t: G.vowel_amp(voice, list(seq), t)
    table('Vowel Journeys', name)(mk())

# =========================================================================== Overtone singing (narrow formant through the harmonics)
for i, (lo, hi_, q, nm) in enumerate([(5, 12, 0.7, 'Overtone Singer'), (6, 16, 0.5, 'Tuvan Whistle'),
                                      (4, 9, 1.0, 'Khoomei Low'), (8, 20, 0.6, 'Sygyt High')]):
    def mk(lo=lo, hi_=hi_, q=q):
        def fn(t):
            c = lerp(lo, hi_, 0.5 - 0.5 * np.cos(np.pi * t))
            body = h ** -1.1 * lp(30, 2)
            return body * 0.35 + 2.2 * np.exp(-0.5 * ((h - c) / q) ** 2) + SINE * 0.5
        return fn
    table('Vocal', nm)(mk())

# =========================================================================== Formant shift families
for i, (src, nm) in enumerate([(SAW, 'Formant Shift Saw'), (SQR, 'Formant Shift Square'),
                               (harmset([2, 3]), 'Formant Shift Fifths'), (TRI * 0 + h ** -0.6, 'Formant Shift Bright')]):
    def mk(src=src):
        def fn(t):
            sh = lerp(0.6, 2.4, t)
            return np.abs(src) * (0.08 + formants(110.0, [700 * sh, 1200 * sh, 2600 * sh], [1.0, 0.6, 0.3], [120 * sh, 160 * sh, 260 * sh]))
        return fn
    table('Spectral', nm)(mk())

# =========================================================================== PPG-style stepped tables
for i in range(6):
    def mk(seed=500 + i, steps=[8, 12, 16, 10, 24, 6][i]):
        keys = drift_keys(seed, steps, (H,))
        cut = np.random.default_rng(seed + 7).integers(6, 60, steps)
        def fn(t):
            j = min(int(t * steps), steps - 1)
            return (keys[j] ** 3) * h ** -0.5 * lp(cut[j], 6)
        return fn
    table('Stepped', ['Wave Steps Glass', 'Wave Steps Digital', 'Wave Steps Choir', 'Wave Steps Metal', 'Wave Steps Dense', 'Wave Steps Hollow'][i])(mk())

# =========================================================================== Comb sweeps
for i, (src, sp, nm) in enumerate([(SAW, (2, 12), 'Comb Sweep Saw'), (SAW, (12, 2), 'Comb Fall Saw'),
                                   (SQR, (3, 9), 'Comb Sweep Square'), (h ** -0.8, (1.5, 6), 'Comb Shimmer')]):
    def mk(src=src, sp=sp):
        def fn(t):
            period = lerp(sp[0], sp[1], t)
            return np.abs(src) * (0.15 + 0.85 * (0.5 + 0.5 * np.cos(2 * np.pi * h / period)) ** 2)
        return fn
    table('Spectral', nm)(mk())

# =========================================================================== Organ registrations
DRAW = [0.5, 1, 1.5, 2, 3, 4, 5, 6, 8]       # footage harmonics (16', 8', 5 1/3', 4', 2 2/3', 2', 1 3/5', 1 1/3', 1')
def drawbar(levels):
    a = np.zeros(H)
    for hm, lv in zip(DRAW, levels):
        k = int(round(hm * 2))     # an octave below the sub so 16' is harmonic 1
        if 1 <= k <= H: a[k - 1] += lv / 8.0
    return a
REG = {
    'Organ Flutes': ([8, 8, 0, 0, 0, 0, 0, 0, 0], [8, 8, 8, 6, 0, 0, 0, 0, 0]),
    'Organ Cathedral': ([8, 6, 4, 6, 2, 4, 0, 0, 2], [8, 8, 8, 8, 6, 8, 4, 4, 6]),
    'Organ Reed Swell': ([4, 8, 0, 6, 0, 4, 4, 0, 0], [6, 8, 4, 8, 4, 8, 6, 6, 4]),
    'Organ Celeste': ([0, 8, 0, 4, 0, 2, 0, 0, 1], [2, 8, 2, 6, 2, 6, 2, 2, 4]),
    'Organ Pedal': ([8, 4, 2, 0, 0, 0, 0, 0, 0], [8, 8, 6, 4, 2, 2, 0, 0, 0]),
    'Organ Mixture': ([0, 6, 0, 6, 4, 6, 4, 4, 6], [4, 8, 4, 8, 8, 8, 8, 8, 8]),
}
for nm, (a, b) in REG.items():
    def mk(a=a, b=b):
        return lambda t: drawbar([lerp(x, y, t) for x, y in zip(a, b)]) + 1e-4 * SAW
    table('Organs', nm)(mk())

# =========================================================================== Resonant filter sweeps (saw/square through a resonant LP)
for i, (src, q, nm) in enumerate([(SAW, 6, 'Reso Sweep Saw'), (SAW, 14, 'Reso Sweep Saw Acid'), (SQR, 8, 'Reso Sweep Square'),
                                  (SAW, 3, 'Reso Sweep Soft'), (np.where(h % 3 == 0, 0.0, 1.0 / h), 8, 'Reso Sweep Hollow'),
                                  (SAW * (1 + 0.5 * np.sin(h)), 10, 'Reso Sweep Grit')]):
    def mk(src=src, q=q):
        def fn(t):
            c = 1.5 * 2 ** (t * 6.2)
            res = 1.0 + q * np.exp(-0.5 * ((h - c) / max(0.6, c * 0.08)) ** 2)
            return np.abs(src) * lp(c, 4) * res
        return fn
    table('Analog', nm)(mk())

# =========================================================================== Wavefolders
for i, (shape, nm) in enumerate([('sin', 'Fold Sine Deep'), ('tri', 'Fold Triangle Deep'), ('saw', 'Fold Saw'), ('asym', 'Fold Asymmetric')]):
    def mk(shape=shape):
        def fn(t):
            g = lerp(1.0, 9.0, t ** 1.3)
            if shape == 'sin': x = np.sin(2 * np.pi * P)
            elif shape == 'tri': x = 1 - 4 * np.abs(P - 0.5)
            elif shape == 'saw': x = 2 * P - 1
            else: x = np.sin(2 * np.pi * P) + 0.4
            return time_to_wave(np.sin(np.pi * 0.5 * g * x))
        return fn
    table('Digital', nm)(mk())

# =========================================================================== Ambient (Roach / Corbacho territory)
NOISE = drift_keys(901, 9, (H,))
def airy(t, amount):
    return cos_interp(NOISE, t) * (h > 6) * h ** -0.9 * amount

@table('Ambient', 'Desert Air')
def _(t): return SAW * lp(lerp(3, 9, t), 3) + airy(t, 0.35 * t)

@table('Ambient', 'Dreamtime Reed')
def _(t): return np.where(h % 2 == 1, 1.0 / h, 0.25 / h) * lp(lerp(6, 18, t)) * (1 + bump(lerp(5, 9, t), 1.5))

@table('Ambient', 'Structures from Silence')
def _(t): return SINE + harmset([2, 3], 1.6) * 0.5 * t + harmset([5], 1.8) * 0.2 * t ** 2

@table('Ambient', 'Magnificent Void')
def _(t): return h ** -1.4 * lp(lerp(2.5, 10, t), 4) * (1 + 0.6 * np.sin(h * 0.9 + 6 * t))

@table('Ambient', 'Canyon Glow')
def _(t): return harmset([2, 3, 4], 1.3) * lp(lerp(8, 40, t), 2) + SINE * 0.6

@table('Ambient', 'Night Sky Shimmer')
def _(t): return SINE * 0.7 + (h > 8) * np.abs(np.sin(h * 1.37 + 9 * t)) * h ** -0.7 * lp(lerp(40, 160, t), 2)

@table('Ambient', 'Stillness')
def _(t): return SINE + np.where(h == 2, 0.25 * t, 0.0) + np.where(h == 3, 0.12 * t, 0.0)

@table('Ambient', 'Ritual Breath')
def _(t): return (0.2 + formants(98, [lerp(400, 700, t), 1100, 2600], [1, 0.5, 0.25], [90, 140, 220])) * h ** -0.8 + airy(t, 0.25)

@table('Ambient', 'Altar of Light')
def _(t): return harmset([4, 5, 6], 1.2) * lp(lerp(10, 70, t), 2) * 0.6 + SINE * 0.5

@table('Ambient', 'Ocean of Stars')
def _(t): return cos_interp(drift_keys(911, 6, (H,)), t) ** 2 * h ** -0.6 * lp(lerp(12, 60, t), 3)

@table('Ambient', 'Mirage Heat')
def _(t): return SAW * lp(10, 2) * (1 + 0.8 * np.sin(h * lerp(0.2, 1.4, t)))

@table('Ambient', 'Sacred Ground')
def _(t): return h ** -1.2 * lp(6, 4) + bump(lerp(8, 4, t), 0.8) * 0.6

@table('Ambient', 'Monsoon Haze')
def _(t): return airy(t, 1.0) + SAW * lp(4, 3) * 0.8

@table('Ambient', 'Blue Hour')
def _(t): return np.where(h % 2 == 1, 1.0, 0.0) * h ** -1.5 * lp(lerp(4, 16, t), 3) + SINE * 0.4

@table('Ambient', 'Ancient Light')
def _(t): return harmset([3, 4], 1.5) * lp(lerp(6, 30, t), 3) + harmset([7], 2.0) * 0.3 * t

@table('Ambient', 'Ice Cathedral')
def _(t): return harmset([2, 4, 8], 0.9) * lp(lerp(30, 200, t), 2) * (1 + 0.4 * np.cos(h * 0.3))

@table('Ambient', 'Earth Pulse')
def _(t): return SINE + SAW * lp(lerp(2, 5, t), 6) * 0.5

@table('Ambient', 'Celestial Fold')
def _(t): return time_to_wave(np.sin(2 * np.pi * P + lerp(0.2, 3.5, t) * np.sin(2 * np.pi * 2 * P)))

@table('Ambient', 'Distant Choir')
def _(t): return G.vowel_amp('alto', list('ooa'), t) * lp(18, 3) + airy(t, 0.12)

@table('Ambient', 'Ghost Strings')
def _(t): return SAW * lp(lerp(5, 14, t), 3) * (0.5 + bump(lerp(14, 22, t), 4)) + airy(t, 0.2)

# =========================================================================== Organic winds and wood
@table('Organic', 'Wood Flute')
def _(t): return SINE + np.where(h == 2, 0.25, 0) + np.where(h == 3, 0.1 + 0.2 * t, 0) + airy(t, 0.15)

@table('Organic', 'Bamboo Breath')
def _(t): return np.where(h % 2 == 1, 1.0 / h ** 1.8, 0.0) + airy(t, 0.35)

@table('Organic', 'Ocarina')
def _(t): return SINE + np.where(h == 3, 0.06, 0) + np.where(h == 2, 0.03 * t, 0)

@table('Organic', 'Shakuhachi')
def _(t): return SINE + np.where(h == 2, 0.4 * t, 0) + np.where(h == 3, 0.2, 0) + airy(t, 0.4 + 0.3 * t)

@table('Organic', 'Didgeridoo Morph')
def _(t): return h ** -0.7 * (0.1 + formants(65, [lerp(300, 900, t), lerp(1400, 2200, t)], [1, 0.6], [80, 160]))

@table('Organic', 'Clay Pot')
def _(t): return SINE + bump(lerp(4, 7, t), 0.7) * 0.5

@table('Organic', 'Hang Drum')
def _(t): return SINE + np.where(h == 2, 0.5, 0) + np.where(h == 3, 0.3 * (1 - t), 0) + np.where(h == 5, 0.15, 0)

@table('Organic', 'Kalimba Tine')
def _(t): return SINE + np.where(h == 6, lerp(0.5, 0.05, t), 0) + np.where(h == 13, lerp(0.25, 0.0, t), 0)

# =========================================================================== Metallic (harmonic approximations of bells and plates)
for i, (cent, spread, nm) in enumerate([(9, 3, 'Gamelan Glow'), (14, 5, 'Bowl Ring'), (20, 8, 'Plate Shimmer'), (6, 2, 'Gong Body'),
                                        (30, 10, 'Cymbal Air'), (11, 2.5, 'Temple Bell'), (17, 4, 'Bronze Bars'), (24, 6, 'Steel Rain')]):
    def mk(cent=cent, spread=spread, seed=700 + i):
        keys = drift_keys(seed, 5, (H,))
        def fn(t):
            k = cos_interp(keys, t)
            cluster = np.exp(-0.5 * ((h - cent * lerp(0.8, 1.25, t)) / spread) ** 2)
            return SINE * 0.5 + (k > 0.72) * cluster * 1.4 + h ** -1.5 * 0.2
        return fn
    table('Metallic', nm)(mk())

# =========================================================================== Basses
@table('Bass', 'Sub Weight')
def _(t): return SINE + np.where(h == 2, 0.35 * t, 0) + np.where(h == 3, 0.12 * t, 0)

@table('Bass', 'Growl Formant')
def _(t): return SAW * (0.2 + formants(55, [lerp(250, 700, t), lerp(900, 1400, t)], [1, 0.6], [70, 120]))

@table('Bass', 'Round Square')
def _(t): return SQR * lp(lerp(3, 25, t), 4)

@table('Bass', 'Pulse Body')
def _(t): return time_to_wave(np.where(P < lerp(0.5, 0.12, t), 1.0, -1.0)) * 1.0

@table('Bass', 'Moog Saw Sweep')
def _(t): return SAW * lp(lerp(2, 40, t ** 2), 4) * (1 + 3 * np.exp(-0.5 * ((h - lerp(2, 40, t ** 2)) / 1.2) ** 2))

@table('Bass', 'Fat Fold')
def _(t): return time_to_wave(np.tanh(lerp(1, 6, t) * np.sin(2 * np.pi * P)) + 0.3 * np.sin(4 * np.pi * P))

# =========================================================================== Spectral misc
@table('Spectral', 'Odd Even Crossfade')
def _(t): return np.where(h % 2 == 1, 1 - t, t) / h ** 0.9 * lp(60)

@table('Spectral', 'Tilt Sweep')
def _(t): return h ** -lerp(2.2, 0.3, t) * lp(200)

@table('Spectral', 'Partial Rise')
def _(t): return np.exp(-0.5 * ((h - lerp(1, 48, t)) / lerp(1, 6, t)) ** 2) + SINE * 0.15

@table('Spectral', 'Harmonic Sieve')
def _(t):
    k = int(lerp(1, 7, t)) + 1
    return np.where(h % k == 0, 1.0, 0.15) / h ** 0.8 * lp(80)

@table('Spectral', 'Spectral Smear')
def _(t):
    a = SAW * lp(30)
    w = int(lerp(1, 25, t))
    ker = np.ones(w) / w
    return np.convolve(a, ker, mode='same') + SINE * 0.2

@table('Spectral', 'Inverse Saw Spectrum')
def _(t): return (h / H) ** lerp(0.2, 1.0, t) * lp(lerp(20, 300, t), 4) + SINE * 0.4

@table('Spectral', 'Bright Bell Spectrum')
def _(t): return np.where(np.isin(h, [1, 2, 3, 5, 8, 13, 21, 34, 55]), 1.0, 0.0) * h ** -lerp(0.2, 1.2, t)

@table('Spectral', 'Twin Peaks')
def _(t): return bump(lerp(3, 12, t), 1.2) + bump(lerp(20, 8, t), 1.8) + SINE * 0.3

@table('Spectral', 'Vocoder Bands')
def _(t): return SAW * (0.1 + np.abs(np.sin(h * 0.25 + 4 * t)) ** 4)

@table('Spectral', 'Phase Scatter')
def _(t):
    rng = np.random.default_rng(1234)
    ph = rng.random(H) * 2 * np.pi * t
    return (SAW * lp(40), ph)


# --------------------------------------------------------------------------- output
def write_wav16(path, data):
    pcm = np.clip(np.round(data * 32767.0), -32768, 32767).astype('<i2')
    clm = b'<!>2048 10000000 wavetable (Driftfield)'
    clm += b'\0' * (len(clm) & 1)
    fmt = struct.pack('<HHIIHH', 1, 1, 48000, 48000 * 2, 2, 16)
    body = (b'WAVE' + b'fmt ' + struct.pack('<I', 16) + fmt
            + b'clm ' + struct.pack('<I', len(clm)) + clm
            + b'data' + struct.pack('<I', pcm.nbytes) + pcm.tobytes())
    with open(path, 'wb') as f:
        f.write(b'RIFF' + struct.pack('<I', len(body)) + body)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'Wavetables'
    names = set()
    for cat, name, fn in TABLES:
        assert (cat, name) not in names, name
        names.add((cat, name))
        d = os.path.join(out, cat)
        os.makedirs(d, exist_ok=True)
        data = G.render(fn)
        assert np.all(np.isfinite(data)), name
        write_wav16(os.path.join(d, name + '.wav'), data)
    print(f'{len(TABLES)} wavetables written to {out}')


if __name__ == '__main__':
    main()
