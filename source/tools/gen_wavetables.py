#!/usr/bin/env python3
"""Driftfield factory wavetable library generator.

Writes Serum/Vital-compatible wavetables: mono 32-bit float WAV, 2048-sample frames,
'clm ' chunk marking the frame size. Every frame is band-limited to 1023 harmonics,
DC-free and peak-normalised. Usage: gen_wavetables.py <output dir>
"""
import os, sys, struct
import numpy as np

N = 2048            # samples per frame
F = 64              # frames per table
H = 1023            # max harmonic
OS = 8              # oversampling for time-domain generators
h = np.arange(1, H + 1, dtype=np.float64)
TABLES = []


def table(cat, name):
    def deco(fn):
        TABLES.append((cat, name, fn))
        return fn
    return deco


# --------------------------------------------------------------------------- helpers
def spec_to_wave(amp, phase=None):
    """amp[h-1] = amplitude of sin(2*pi*h*x + phase)."""
    X = np.zeros(N // 2 + 1, dtype=np.complex128)
    ph = np.zeros(H) if phase is None else phase
    X[1:H + 1] = amp * (N / 2) * (-1j) * np.exp(1j * ph)
    return np.fft.irfft(X, n=N)


def time_to_wave(x):
    """Band-limit an oversampled single cycle (length N*OS) to H harmonics, return N samples."""
    X = np.fft.rfft(x)
    Y = np.zeros(N // 2 + 1, dtype=np.complex128)
    Y[1:H + 1] = X[1:H + 1] / OS
    return np.fft.irfft(Y, n=N)


P = np.arange(N * OS) / (N * OS)       # phase 0..1 for time-domain generators


def lerp(a, b, x):
    return a + (b - a) * x


def lp(c, order=4):
    return 1.0 / np.sqrt(1.0 + (h / max(c, 1e-3)) ** order)


def bump(centre, width):
    return np.exp(-0.5 * ((h - centre) / max(width, 1e-3)) ** 2)


SAW = 1.0 / h
SQR = np.where(h % 2 == 1, 1.0 / h, 0.0)
TRI = np.where(h % 2 == 1, ((-1.0) ** ((h - 1) // 2)) / h ** 2, 0.0)


def formants(fund, fl, amps, bws):
    hz = h * fund
    a = np.zeros(H)
    for f, g, b in zip(fl, amps, bws):
        a += g * np.exp(-0.5 * ((hz - f) / b) ** 2)
    return a


def smooth_noise(seed, n_keys, length):
    """Smooth random curve(s) via cosine interpolation of random keys."""
    rng = np.random.default_rng(seed)
    keys = rng.random((n_keys,) + length)
    x = np.linspace(0, n_keys - 1, F)
    i0 = np.floor(x).astype(int).clip(0, n_keys - 2)
    fr = (x - i0)[:, None] if len(length) else (x - i0)
    fr = 0.5 - 0.5 * np.cos(np.pi * fr)
    return keys[i0] * (1 - fr) + keys[i0 + 1] * fr


# Vowel formants (Hz, dB, bandwidth) — classic Csound formant tables
VOW = {
    'bass':    {'a': ([600, 1040, 2250, 2450, 2750], [0, -7, -9, -9, -20], [60, 70, 110, 120, 130]),
                'e': ([400, 1620, 2400, 2800, 3100], [0, -12, -9, -12, -18], [40, 80, 100, 120, 120]),
                'i': ([250, 1750, 2600, 3050, 3340], [0, -30, -16, -22, -28], [60, 90, 100, 120, 120]),
                'o': ([400, 750, 2400, 2600, 2900], [0, -11, -21, -20, -40], [40, 80, 100, 120, 120]),
                'u': ([350, 600, 2400, 2675, 2950], [0, -20, -32, -28, -36], [40, 80, 100, 120, 120])},
    'tenor':   {'a': ([650, 1080, 2650, 2900, 3250], [0, -6, -7, -8, -22], [80, 90, 120, 130, 140]),
                'e': ([400, 1700, 2600, 3200, 3580], [0, -14, -12, -14, -20], [70, 80, 100, 120, 120]),
                'i': ([290, 1870, 2800, 3250, 3540], [0, -15, -18, -20, -30], [40, 90, 100, 120, 120]),
                'o': ([400, 800, 2600, 2800, 3000], [0, -10, -12, -12, -26], [40, 80, 100, 120, 120]),
                'u': ([350, 600, 2700, 2900, 3300], [0, -20, -17, -14, -26], [40, 60, 100, 120, 120])},
    'alto':    {'a': ([800, 1150, 2800, 3500, 4950], [0, -4, -20, -36, -60], [80, 90, 120, 130, 140]),
                'e': ([400, 1600, 2700, 3300, 4950], [0, -24, -30, -35, -60], [60, 80, 120, 150, 200]),
                'i': ([350, 1700, 2700, 3700, 4950], [0, -20, -30, -36, -60], [50, 100, 120, 150, 200]),
                'o': ([450, 800, 2830, 3500, 4950], [0, -9, -16, -28, -55], [70, 80, 100, 130, 135]),
                'u': ([325, 700, 2530, 3500, 4950], [0, -12, -30, -40, -64], [50, 60, 170, 180, 200])},
    'soprano': {'a': ([800, 1150, 2900, 3900, 4950], [0, -6, -32, -20, -50], [80, 90, 120, 130, 140]),
                'e': ([350, 2000, 2800, 3600, 4950], [0, -20, -15, -40, -56], [60, 100, 120, 150, 200]),
                'i': ([270, 2140, 2950, 3900, 4950], [0, -12, -26, -26, -44], [60, 90, 100, 120, 120]),
                'o': ([450, 800, 2830, 3800, 4950], [0, -11, -22, -22, -50], [70, 80, 100, 130, 135]),
                'u': ([325, 700, 2700, 3800, 4950], [0, -16, -35, -40, -60], [50, 60, 170, 180, 200])},
}
FUND = {'bass': 98.0, 'tenor': 147.0, 'alto': 220.0, 'soprano': 330.0}


def vowel_amp(voice, seq, t, tilt=0.6):
    x = t * (len(seq) - 1)
    i = min(int(x), len(seq) - 2)
    fr = x - i
    fa, ga, ba = VOW[voice][seq[i]]
    fb, gb, bb = VOW[voice][seq[i + 1]]
    fl = [lerp(a, b, fr) for a, b in zip(fa, fb)]
    gl = [10 ** (lerp(a, b, fr) / 20) for a, b in zip(ga, gb)]
    bl = [lerp(a, b, fr) for a, b in zip(ba, bb)]
    glott = h ** -tilt * np.exp(-h * FUND[voice] / 9000.0)
    return glott * (0.02 + formants(FUND[voice], fl, gl, bl))


# =========================================================================== Analog
@table('Analog', 'Saw to Square')
def _(t): return lerp(SAW, SQR, t)

@table('Analog', 'Sine to Saw')
def _(t): return lerp(np.where(h == 1, 1.0, 0.0), SAW, t ** 0.7)

@table('Analog', 'Triangle to Saw')
def _(t):
    k = lerp(0.5, 0.995, t)
    x = np.where(P < k, P / k, (1 - P) / (1 - k)) * 2 - 1
    return time_to_wave(x)

@table('Analog', 'PWM 50 to 2')
def _(t):
    w = lerp(0.5, 0.02, t)
    return time_to_wave(np.where(P < w, 1.0, -1.0))

@table('Analog', 'Saw Lowpass Sweep')
def _(t): return SAW * lp(1.0 + 120 * t ** 2.2)

@table('Analog', 'Square Lowpass Sweep')
def _(t): return SQR * lp(1.0 + 120 * t ** 2.2)

@table('Analog', 'Sync Saw 1-8')
def _(t): return time_to_wave(2 * ((P * (1 + 7 * t)) % 1.0) - 1)

@table('Analog', 'Sync Square 1-6')
def _(t): return time_to_wave(np.where(((P * (1 + 5 * t)) % 1.0) < 0.5, 1.0, -1.0))

@table('Analog', 'Sync Sine 1-5')
def _(t): return time_to_wave(np.sin(2 * np.pi * ((P * (1 + 4 * t)) % 1.0)))

@table('Analog', 'Trapezoid Drive')
def _(t):
    tri = 1 - 4 * np.abs(P - 0.5)
    return time_to_wave(np.clip(tri * (1 + 9 * t), -1, 1))

@table('Analog', 'Ramp Fold')
def _(t):
    x = (2 * P - 1) * (1 + 3 * t)
    return time_to_wave(np.sin(np.pi / 2 * x))

@table('Analog', 'Double Saw Phase')
def _(t):
    a = 2 * P - 1
    b = 2 * ((P + 0.5 * t) % 1.0) - 1
    return time_to_wave(a + b)

# =========================================================================== Spectral
@table('Spectral', 'Resonant LP Sweep')
def _(t):
    c = 1.5 + 100 * t ** 2
    return SAW * lp(c) + 0.9 * SAW * bump(c, 0.6 + c * 0.06) * np.sqrt(h)

@table('Spectral', 'Resonant BP Sweep')
def _(t):
    c = 1.5 + 100 * t ** 2
    return 0.05 * SAW + bump(c, 0.8 + c * 0.12) / np.sqrt(h)

@table('Spectral', 'Resonant HP Sweep')
def _(t):
    c = 1 + 80 * t ** 2
    return SAW / np.sqrt(1 + (c / h) ** 4) + 0.6 * SAW * bump(c, 0.5 + c * 0.08) * np.sqrt(h)

@table('Spectral', 'Comb Sweep')
def _(t): return SAW * (0.5 + 0.5 * np.cos(2 * np.pi * h * lerp(0.0, 0.25, t))) * lp(200)

@table('Spectral', 'Phaser Notches')
def _(t):
    g = np.ones(H)
    for k in range(4):
        c = (3 + 12 * k) * (0.5 + t)
        g *= 1 - 0.95 * bump(c, 1 + c * 0.08)
    return SAW * g

@table('Spectral', 'Odd to Even')
def _(t):
    return SAW * np.where(h % 2 == 1, 1 - t, t) * np.where(h == 1, 1.0, 1.0) + np.where(h == 1, 0.6, 0.0)

@table('Spectral', 'Tilt Dark to Bright')
def _(t): return h ** -lerp(2.6, 0.6, t)

@table('Spectral', 'Spectral Stripes')
def _(t): return SAW * (0.5 + 0.5 * np.sign(np.sin(2 * np.pi * (h / 8.0 + t))))

@table('Spectral', 'Harmonic Window')
def _(t):
    c = lerp(1, 60, t ** 1.5)
    return bump(c, 1.5 + c * 0.25) / np.sqrt(h) + 0.02 * SAW

@table('Spectral', 'Spectral Mirror')
def _(t):
    c1, c2 = lerp(2, 30, t), lerp(90, 30, t)
    return (bump(c1, 2 + c1 * 0.2) + bump(c2, 3 + c2 * 0.2)) / np.sqrt(h)

@table('Spectral', 'Formant Glide')
def _(t):
    c = lerp(200, 4200, t ** 1.4)
    return SAW ** 0.6 * (0.03 + formants(110, [c], [1], [c * 0.12]))

@table('Spectral', 'Brightness Bloom')
def _(t):
    return SAW * lp(2 + 150 * t ** 2.5) / np.sqrt(1 + (lerp(6, 1, t) / h) ** 4)

# =========================================================================== Vocal
@table('Vocal', 'Bass Vowels U-O-A-E-I')
def _(t): return vowel_amp('bass', 'uoaei', t)

@table('Vocal', 'Tenor Vowels A-E-I-O-U')
def _(t): return vowel_amp('tenor', 'aeiou', t)

@table('Vocal', 'Alto Vowels O-A-E')
def _(t): return vowel_amp('alto', 'oae', t)

@table('Vocal', 'Soprano Vowels U-A-I')
def _(t): return vowel_amp('soprano', 'uai', t)

@table('Vocal', 'Choir Aah to Ooh')
def _(t): return vowel_amp('alto', 'aou', t, tilt=0.8)

@table('Vocal', 'Choir Ooh to Eeh')
def _(t): return vowel_amp('soprano', 'uoe', t, tilt=0.8)

@table('Vocal', 'Throat Drone')
def _(t): return vowel_amp('bass', 'uo', t, tilt=0.4)

@table('Vocal', 'Ethereal Ah')
def _(t): return vowel_amp('soprano', 'aoa', t, tilt=1.0) * lp(lerp(8, 20, t))

@table('Vocal', 'Overtone Singing')
def _(t):
    c = lerp(5, 16, 0.5 - 0.5 * np.cos(np.pi * t))
    base = vowel_amp('bass', 'u', 0.0, tilt=0.5)
    return base * 0.35 + 1.2 * bump(c, 0.45) * h ** -0.3

@table('Vocal', 'Monk Chant')
def _(t):
    c = lerp(6, 12, t)
    return vowel_amp('bass', 'oau', t, tilt=0.5) + 0.5 * bump(c, 0.5) / h

@table('Vocal', 'Whisper Air')
def _(t):
    rng = np.random.default_rng(7 + int(t * 16))
    return vowel_amp('alto', 'aei', t, tilt=0.2) * (0.4 + 0.6 * rng.random(H))

@table('Vocal', 'Vocal Morph Wide')
def _(t): return vowel_amp('tenor', 'iaoue', t, tilt=0.5)

# =========================================================================== Additive
@table('Additive', 'Harmonic Walk 1-32')
def _(t):
    x = 1 + 31 * t
    i = int(np.floor(x)); fr = x - i
    a = np.zeros(H); a[i - 1] = 1 - fr
    if i < H: a[i] = fr
    a[0] += 0.5
    return a

@table('Additive', 'Harmonic Build')
def _(t):
    n = 1 + 47 * t
    return SAW * np.clip(n - h + 1, 0, 1)

@table('Additive', 'Odd Build')
def _(t):
    n = 1 + 63 * t
    return SQR * np.clip(n - h + 1, 0, 1)

@table('Additive', 'Octave Stack')
def _(t):
    a = np.zeros(H)
    for k, hh in enumerate([1, 2, 4, 8, 16, 32, 64]):
        a[hh - 1] = np.clip(t * 7 - k + 1, 0, 1) * (0.8 ** k) if k else 1.0
    return a

@table('Additive', 'Fifths Stack')
def _(t):
    a = np.zeros(H)
    for k, hh in enumerate([1, 3, 9, 27, 81]):
        a[hh - 1] = (np.clip(t * 5 - k + 1, 0, 1) if k else 1.0) * 0.75 ** k
    return a

@table('Additive', 'Prime Partials')
def _(t):
    primes = [p for p in range(2, 200) if all(p % d for d in range(2, int(p ** 0.5) + 1))]
    a = np.zeros(H); a[0] = 1
    for p in primes: a[p - 1] = 1 / np.sqrt(p)
    return a * lp(lerp(3, 150, t ** 1.5))

@table('Additive', 'Fibonacci Glow')
def _(t):
    a = np.zeros(H)
    for f in [1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610]: a[f - 1] = f ** -0.5
    return a * lp(lerp(4, 400, t ** 2))

@table('Additive', 'Square Numbers')
def _(t):
    a = np.zeros(H)
    for k in range(1, 32): a[k * k - 1] = k ** -lerp(1.8, 0.5, t)
    return a

@table('Additive', 'Drawbars Gospel')
def _(t):
    regs = [[8, 8, 0, 0, 0, 0, 0, 0, 0], [8, 8, 8, 0, 0, 0, 0, 0, 0], [8, 8, 8, 8, 0, 0, 0, 0, 0],
            [8, 6, 8, 6, 4, 0, 3, 0, 0], [8, 8, 8, 8, 8, 8, 8, 8, 8]]
    bars = [0.5, 1, 1.5, 2, 3, 4, 5, 6, 8]  # 16' 8' 5 1/3' ... in units of the 8' fundamental
    x = t * (len(regs) - 1); i = min(int(x), len(regs) - 2); fr = x - i
    a = np.zeros(H)
    for k, b in enumerate(bars):
        hh = int(round(b * 2))           # table fundamental = 16' sub, so 8' = harmonic 2
        a[hh - 1] += lerp(regs[i][k], regs[i + 1][k], fr) / 8
    return a

@table('Additive', 'Pipe Flute')
def _(t):
    a = np.where(h == 1, 1.0, 0.0) + np.where(h == 2, lerp(0.05, 0.3, t), 0) + np.where(h == 3, lerp(0.02, 0.15, t), 0)
    return a + 0.004 * SAW * lp(30)

@table('Additive', 'Cathedral Organ')
def _(t):
    a = np.zeros(H)
    for hh, g in [(1, 1), (2, 0.8), (4, 0.6), (3, 0.4), (6, 0.35), (8, 0.3), (12, 0.25), (16, 0.2)]:
        a[hh - 1] = g
    mix = [(12, 0.3), (16, 0.3), (24, 0.25), (32, 0.2)]
    for hh, g in mix: a[hh - 1] += g * t
    return a

@table('Additive', 'Octave Down Morph')
def _(t):
    even = np.where(h % 2 == 0, SAW * 2, 0)     # tone an octave up (period/2)
    odd = np.where(h % 2 == 1, SAW, 0)          # sub-octave content
    return even + odd * t

# =========================================================================== FM
def fm(c, m, imax, curve=1.0):
    def f(t):
        I = imax * t ** curve
        return time_to_wave(np.sin(2 * np.pi * c * P + I * np.sin(2 * np.pi * m * P)))
    return f

for (c, m, imax) in [(1, 1, 6), (1, 2, 6), (1, 3, 5), (2, 1, 5), (1, 4, 4), (3, 2, 5), (1, 7, 3), (2, 5, 4)]:
    table('FM', f'FM {c}-{m} Index Sweep')(fm(c, m, imax))

@table('FM', 'FM Feedback')
def _(t):
    beta = 1.6 * t
    y = np.zeros(N * OS); prev = 0.0
    for _ in range(2):  # two passes so the cycle is periodic-ish
        for i in range(N * OS):
            prev = np.sin(2 * np.pi * P[i] + beta * prev)
            y[i] = prev
    return time_to_wave(y)

@table('FM', 'PM Saw Modulator')
def _(t): return time_to_wave(np.sin(2 * np.pi * P + 3 * t * (2 * ((2 * P) % 1.0) - 1)))

@table('FM', 'FM Two-Op Chain')
def _(t):
    m2 = np.sin(2 * np.pi * 5 * P) * 2 * t
    m1 = np.sin(2 * np.pi * 2 * P + m2) * 3 * t
    return time_to_wave(np.sin(2 * np.pi * P + m1))

@table('FM', 'FM Index Wobble')
def _(t): return time_to_wave(np.sin(2 * np.pi * P + (2 + 2 * np.sin(2 * np.pi * 2 * t)) * np.sin(2 * np.pi * 3 * P)))

# =========================================================================== Phase Distortion
def pd_res(window):
    def f(t):
        r = 1 + 15 * t
        return time_to_wave(window(P) * np.cos(2 * np.pi * ((P * r) % 1.0)) )
    return f

table('Phase Distortion', 'PD Saw Resonance')(pd_res(lambda p: 1 - p))
table('Phase Distortion', 'PD Triangle Resonance')(pd_res(lambda p: 1 - np.abs(2 * p - 1)))
table('Phase Distortion', 'PD Trapezoid Resonance')(pd_res(lambda p: np.clip(2 - 4 * np.abs(p - 0.5) * 2, 0, 1)))

@table('Phase Distortion', 'PD Bend')
def _(t):
    d = lerp(0.5, 0.02, t)
    ph = np.where(P < d, 0.5 * P / d, 0.5 + 0.5 * (P - d) / (1 - d))
    return time_to_wave(-np.cos(2 * np.pi * ph))

@table('Phase Distortion', 'PD Pulse')
def _(t):
    d = lerp(0.5, 0.05, t)
    ph = np.where(P < d, 0.5 * P / d, np.where(P < 0.5 + d, 0.5, 0.5 + 0.5 * (P - 0.5 - d) / max(1e-6, 0.5 - d)))
    ph = np.clip(ph, 0, 1)
    return time_to_wave(-np.cos(2 * np.pi * ph))

@table('Phase Distortion', 'PD Double Sine')
def _(t):
    d = lerp(0.5, 0.1, t)
    q = (2 * P) % 1.0
    ph = np.where(q < d, 0.5 * q / d, 0.5 + 0.5 * (q - d) / (1 - d))
    return time_to_wave(-np.cos(2 * np.pi * ph) * np.where(P < 0.5, 1, lerp(1, 0.3, t)))

# =========================================================================== Digital
@table('Digital', 'Bitcrush Sine')
def _(t):
    lv = 2 ** lerp(8, 1.2, t)
    return time_to_wave(np.round(np.sin(2 * np.pi * P) * lv) / lv)

@table('Digital', 'Bitcrush Saw')
def _(t):
    lv = 2 ** lerp(8, 1.2, t)
    return time_to_wave(np.round((2 * P - 1) * lv) / lv)

@table('Digital', 'Decimate Sine')
def _(t):
    steps = int(round(2 ** lerp(9, 2.5, t)))
    return time_to_wave(np.sin(2 * np.pi * np.floor(P * steps) / steps))

@table('Digital', 'Sine Drive')
def _(t): return time_to_wave(np.tanh(np.sin(2 * np.pi * P) * lerp(1, 30, t ** 2)))

@table('Digital', 'Wavefolder Sine')
def _(t): return time_to_wave(np.sin(np.sin(2 * np.pi * P) * lerp(1, 9, t) * np.pi / 2))

@table('Digital', 'Wavefolder Triangle')
def _(t):
    x = (1 - 4 * np.abs(P - 0.5)) * lerp(1, 7, t)
    return time_to_wave(np.abs(((x + 1) % 4) - 2) - 1)

@table('Digital', 'Hard Clip Saw')
def _(t): return time_to_wave(np.clip((2 * P - 1) * lerp(1, 12, t), -1, 1))

@table('Digital', 'Digital Stairs')
def _(t):
    steps = int(round(lerp(64, 3, t)))
    return time_to_wave(np.floor(P * steps) / (steps - 1) * 2 - 1)

@table('Digital', 'Bitwise Shuffle')
def _(t):
    i = (P * 256).astype(int)
    k = int(t * 255)
    return time_to_wave(((i ^ k) & 255) / 127.5 - 1)

@table('Digital', 'Sine Ring Ratio')
def _(t):
    k = 1 + 11 * t
    k0 = int(np.floor(k)); fr = k - k0
    s = np.sin(2 * np.pi * P)
    return time_to_wave(s * ((1 - fr) * np.sin(2 * np.pi * k0 * P) + fr * np.sin(2 * np.pi * (k0 + 1) * P)))

# =========================================================================== Drone
def random_spectrum(seed, tilt, nharm=64, keys=6, floor=0.0):
    curves = smooth_noise(seed, keys, (H,))
    def f(t):
        i = int(round(t * (F - 1)))
        return (floor + curves[i]) * h ** -tilt * lp(nharm, 6)
    return f

table('Drone', 'Deep Murmur')(random_spectrum(11, 1.5, 24))
table('Drone', 'Night Wind')(random_spectrum(12, 0.9, 80, floor=0.2))
table('Drone', 'Slow Storm')(random_spectrum(13, 0.7, 160, keys=8))

@table('Drone', 'Abyssal')
def _(t): return h ** -lerp(2.4, 1.6, t) * lp(lerp(6, 14, t))

@table('Drone', 'Ember Glow')
def _(t): return SQR ** 1.3 * (1 + 3 * bump(lerp(3, 15, t), 2.5)) * lp(40)

@table('Drone', 'Iron Hum')
def _(t):
    return np.where(h <= 3, 1.0 / h, 0) + lerp(0.05, 0.25, t) * bump(12, 1.5) * np.where(h % 2 == 1, 1, 0.3)

@table('Drone', 'Low Fog')
def _(t):
    return np.where(h == 1, 1.0, 0) + 0.2 * (0.5 + 0.5 * np.sin(2 * np.pi * (t + h / 5))) * np.where((h > 1) & (h < 7), 1 / h, 0)

@table('Drone', 'Black Glass')
def _(t):
    a = h ** -2.2
    for hh in [17, 23, 31, 41]: a[hh - 1] += lerp(0.002, 0.03, t)
    return a

@table('Drone', 'Monolith')
def _(t): return SQR * lp(20) * (1 - t * 0.9 * np.where((h > 2) & (h < 12), 1, 0))

@table('Drone', 'Undertow')
def _(t):
    c = lerp(30, 3, t)
    return 0.3 * h ** -1.8 + bump(c, 1 + c * 0.2) / h

@table('Drone', 'Tidal Drone')
def _(t):
    return SAW * lp(30) * (0.3 + 0.7 * (0.5 + 0.5 * np.sin(2 * np.pi * (1.3 * t + h / 12))))

@table('Drone', 'Cave Resonance')
def _(t):
    return SAW * lp(25) * (0.4 + 0.6 * np.cos(np.pi * h * lerp(0.05, 0.2, t)) ** 2) + formants(110, [300], [0.4], [60])

# =========================================================================== Glass & Bell
@table('Glass & Bell', 'Crystal Squares')
def _(t):
    a = np.zeros(H)
    for k in range(1, 32): a[k * k - 1] = k ** -lerp(1.5, 0.6, t)
    return a

@table('Glass & Bell', 'Glass Harmonica')
def _(t):
    a = np.zeros(H)
    for hh, g in [(1, 1), (3, 0.5), (5, 0.25), (8, 0.15), (12, 0.08)]: a[hh - 1] = g
    return a * lp(lerp(4, 20, t))

@table('Glass & Bell', 'Ice Shimmer')
def _(t):
    c = lerp(25, 90, t)
    return np.where(h == 1, 0.8, 0) + 0.4 * bump(c, 4) / np.sqrt(h / 20)

@table('Glass & Bell', 'Celesta Tines')
def _(t):
    a = np.zeros(H)
    for hh, g in [(1, 1), (2, 0.15), (4, 0.4), (7, 0.25 * t + 0.05), (11, 0.15 * t), (16, 0.1 * t)]: a[hh - 1] = g
    return a

@table('Glass & Bell', 'Bell Cluster')
def _(t):
    # bell-like partial ratios mapped onto harmonics of a low table fundamental
    a = np.zeros(H)
    for r, g in [(4, 0.5), (8, 1), (10, 0.6), (12, 0.5), (16, 0.4), (20, 0.25), (24, 0.2), (31, 0.12)]:
        a[r - 1] = g * (1 if r <= 12 else lerp(0.3, 1, t))
    return a

@table('Glass & Bell', 'Chime Cluster')
def _(t): return np.where((h >= 8) & (h <= 16), 1.0, 0.0) * bump(lerp(8, 16, t), 2.5) + np.where(h == 1, 0.4, 0)

@table('Glass & Bell', 'Starfield')
def _(t):
    rng = np.random.default_rng(100 + int(t * 12))
    a = np.where(h == 1, 0.6, 0.0)
    idx = rng.choice(np.arange(8, 120), 10, replace=False)
    a[idx] = 0.08 + 0.12 * rng.random(10)
    return a

@table('Glass & Bell', 'Prism')
def _(t):
    return np.where(h == 1, 0.7, 0) + sum(bump(c * (1 + t), 1.2) / c for c in [5, 11, 19, 29])

@table('Glass & Bell', 'Frost')
def _(t):
    rng = np.random.default_rng(55)
    return np.where(h == 1, 1.0, 0) + lerp(0.0, 0.03, t) * rng.random(H) * lp(300) * np.where(h > 20, 1, 0)

@table('Glass & Bell', 'Halo')
def _(t):
    return np.where(h == 1, 1.0, 0) + 0.15 * bump(lerp(10, 40, t), 1.5) + 0.1 * bump(lerp(60, 25, t), 2)

# =========================================================================== Strings & Reeds
def body(fund, peaks):
    return formants(fund, [p for p, _ in peaks], [g for _, g in peaks], [p * 0.12 for p, _ in peaks])

@table('Strings & Reeds', 'Bowed Body')
def _(t):
    return SAW * lp(lerp(12, 60, t)) * (0.3 + body(196, [(280, 1), (460, 0.8), (700, 0.6), (1100, 0.5), (2500, 0.4)]))

@table('Strings & Reeds', 'Cello Body')
def _(t):
    return SAW * lp(lerp(10, 45, t)) * (0.3 + body(65.4, [(200, 1), (400, 0.7), (650, 0.5), (1300, 0.35)]))

@table('Strings & Reeds', 'Viola Body')
def _(t):
    return SAW * lp(lerp(12, 50, t)) * (0.3 + body(130.8, [(350, 1), (600, 0.6), (1100, 0.5), (2300, 0.3)]))

@table('Strings & Reeds', 'Ensemble Soft')
def _(t): return SAW * lp(lerp(5, 16, t)) * (1 + 0.3 * (h % 2))

@table('Strings & Reeds', 'Sul Ponticello')
def _(t): return SAW * (0.2 + bump(lerp(10, 30, t), 6)) * lp(80)

@table('Strings & Reeds', 'Flautando')
def _(t): return np.where(h == 1, 1.0, 0) + SAW * lp(lerp(2, 6, t)) * 0.4

@table('Strings & Reeds', 'String Harmonics')
def _(t):
    n = int(round(lerp(2, 5, t)))
    return np.where(h % n == 0, SAW * n, 0.0) * lp(60) + np.where(h == 1, 0.05, 0)

@table('Strings & Reeds', 'Reed Breath')
def _(t): return np.where(h % 2 == 1, 1.0 / h, lerp(0.0, 0.6, t) / h) * lp(lerp(10, 40, t))

# =========================================================================== Textures
for k, (tilt, nh) in enumerate([(1.0, 64), (0.6, 200), (1.4, 40), (0.8, 120)]):
    table('Textures', f'Random Walk Spectrum {k + 1}')(random_spectrum(200 + k, tilt, nh, keys=10))

@table('Textures', 'Granular Phase Drift')
def _(t):
    rng = np.random.default_rng(9)
    return SAW * lp(60), rng.random(H) * 2 * np.pi * t

@table('Textures', 'Dust Spectrum')
def _(t):
    rng = np.random.default_rng(300 + int(t * F))
    return SAW * lp(80) * (rng.random(H) < 0.35)

@table('Textures', 'Tape Saw')
def _(t): return time_to_wave(np.tanh((2 * P - 1) * lerp(0.8, 4, t)) + 0.05 * np.sin(2 * np.pi * 3 * P))

@table('Textures', 'Shortwave')
def _(t):
    rng = np.random.default_rng(77)
    return SAW * lp(150) * (0.5 + 0.5 * np.cos(2 * np.pi * h * lerp(0.02, 0.11, t))) * (0.5 + rng.random(H))

@table('Textures', 'Spectral Freeze')
def _(t):
    a = smooth_noise(400, 8, (H,))[int(round(t * (F - 1)))]
    rng = np.random.default_rng(401)
    return a * h ** -0.8 * lp(250), rng.random(H) * 2 * np.pi

@table('Textures', 'Aurora')
def _(t):
    c1, c2, c3 = 4 + 3 * np.sin(2 * np.pi * t), 14 + 8 * np.sin(2 * np.pi * 2 * t + 1), 40 + 20 * np.sin(2 * np.pi * 3 * t + 2)
    return 0.2 * h ** -1.5 + (bump(c1, 1.5) + 0.6 * bump(c2, 3) + 0.3 * bump(c3, 6)) / np.sqrt(h)

@table('Textures', 'Breathing Haze')
def _(t):
    rng = np.random.default_rng(500 + int(t * 20))
    return (h ** -1.1) * lp(lerp(20, 70, 0.5 - 0.5 * np.cos(2 * np.pi * t))) * (0.6 + 0.4 * rng.random(H))


# --------------------------------------------------------------------------- writer
def render(fn):
    frames = []
    for f in range(F):
        t = f / (F - 1)
        out = fn(t)
        if isinstance(out, tuple):
            w = spec_to_wave(np.asarray(out[0], dtype=np.float64), np.asarray(out[1]))
        elif len(out) == H:
            w = spec_to_wave(np.asarray(out, dtype=np.float64))
        else:
            w = out
        w = w - w.mean()
        pk = np.max(np.abs(w))
        frames.append(w * (0.9 / pk) if pk > 1e-12 else w)
    return np.concatenate(frames).astype(np.float32)


def write_wav(path, data):
    clm = b'<!>2048 10000000 wavetable (Driftfield)'
    clm += b'\0' * (len(clm) & 1)
    fmt = struct.pack('<HHIIHH', 3, 1, 48000, 48000 * 4, 4, 32)
    body = (b'WAVE' + b'fmt ' + struct.pack('<I', 16) + fmt
            + b'clm ' + struct.pack('<I', len(clm)) + clm
            + b'data' + struct.pack('<I', data.nbytes) + data.tobytes())
    with open(path, 'wb') as f:
        f.write(b'RIFF' + struct.pack('<I', len(body)) + body)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'Wavetables'
    for cat, name, fn in TABLES:
        d = os.path.join(out, cat)
        os.makedirs(d, exist_ok=True)
        data = render(fn)
        assert np.all(np.isfinite(data)), name
        write_wav(os.path.join(d, name + '.wav'), data)
    print(f'{len(TABLES)} wavetables written to {out}')


if __name__ == '__main__':
    main()
