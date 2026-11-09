#!/usr/bin/env python3
"""Driftfield 3.1 grain-source library (synthesised, no recordings).

Field textures, voices, metals, reeds and machine noises meant as granular material: long, evolving,
loopable (short fades at both ends), mono 48 kHz 16-bit, loudness-normalised like render_samples.cpp
(about -17 dBFS RMS with a soft clip). Usage: gen_samples.py <Samples dir>
"""
import os, sys, struct
import numpy as np
from scipy import signal

SR = 48000
rng = np.random.default_rng(31)
SAMPLES = []


def sample(cat, name, secs=10.0):
    def deco(fn):
        SAMPLES.append((cat, name, secs, fn))
        return fn
    return deco


# --------------------------------------------------------------------------- building blocks
def t_axis(n): return np.arange(n) / SR
def noise(n): return rng.standard_normal(n)
def pink(n):
    w = np.fft.rfft(noise(n)); f = np.fft.rfftfreq(n, 1 / SR); f[0] = f[1]
    return np.fft.irfft(w / np.sqrt(f), n)
def brown(n): x = np.cumsum(noise(n)); return signal.filtfilt(*signal.butter(1, 20 / (SR / 2), 'high'), x)
def bp(x, lo, hi, order=2): return signal.sosfilt(signal.butter(order, [lo / (SR / 2), hi / (SR / 2)], 'band', output='sos'), x)
def lpf(x, fc, order=2): return signal.sosfilt(signal.butter(order, min(fc, SR * 0.45) / (SR / 2), 'low', output='sos'), x)
def hpf(x, fc, order=2): return signal.sosfilt(signal.butter(order, fc / (SR / 2), 'high', output='sos'), x)
def slow(n, rate, seed=None, smooth=3):
    """smooth random control 0..1 with about `rate` changes per second"""
    r = np.random.default_rng(seed)
    k = max(4, int(n / SR * rate) + 3)
    keys = r.random(k)
    x = np.linspace(0, k - 1.001, n)
    i = x.astype(int); fr = x - i; fr = 0.5 - 0.5 * np.cos(np.pi * fr)
    return keys[i] * (1 - fr) + keys[i + 1] * fr
def env_ar(n, a, r):
    e = np.ones(n); na = int(a * SR); nr = int(r * SR)
    if na: e[:na] = np.linspace(0, 1, na) ** 2
    if nr: e[-nr:] *= np.linspace(1, 0, nr) ** 2
    return e
def osc_sine(f, n, ph=0.0):
    f = np.broadcast_to(f, (n,)).astype(float)
    return np.sin(2 * np.pi * np.cumsum(f) / SR + ph)
def resonator(x, f, q):
    """2-pole resonant bandpass (constant 0 dB peak)"""
    b, a = signal.iirpeak(f / (SR / 2), q)
    return signal.lfilter(b, a, x)
def modal_hit(n, freqs, decays, amps, at=0):
    out = np.zeros(n); tt = t_axis(n - at)
    for f, d, a in zip(freqs, decays, amps):
        if f < SR * 0.45: out[at:] += a * np.sin(2 * np.pi * f * tt + rng.random() * 6.28) * np.exp(-tt / d)
    return out
def scatter(n, rate, fn, seed=1):
    """sum of events placed by a Poisson process; fn(length_available) -> event array"""
    r = np.random.default_rng(seed); out = np.zeros(n); t = 0
    while True:
        t += int(r.exponential(SR / rate))
        if t >= n: break
        ev = fn(r)
        m = min(len(ev), n - t); out[t:t + m] += ev[:m]
    return out
def glottal(f0, n, vib=5.2, vibd=0.012, jitter=0.004, seed=0):
    r = np.random.default_rng(seed)
    f = f0 * (1 + vibd * np.sin(2 * np.pi * vib * t_axis(n) + r.random() * 6) + jitter * (slow(n, 3, seed) - 0.5))
    ph = np.cumsum(f) / SR
    saw = 2 * (ph % 1.0) - 1
    return lpf(saw, 5000, 1) * 0.6 + 0.02 * noise(n)
FORM = {'a': [(730, 80, 1.0), (1090, 90, 0.5), (2440, 120, 0.25)], 'o': [(570, 70, 1.0), (840, 80, 0.45), (2410, 120, 0.12)],
        'u': [(300, 50, 1.0), (870, 80, 0.2), (2240, 120, 0.06)], 'e': [(530, 60, 1.0), (1840, 100, 0.4), (2480, 120, 0.3)],
        'i': [(270, 50, 1.0), (2290, 100, 0.25), (3010, 140, 0.2)]}
def vowel(x, v):
    return sum(g * resonator(x, f, f / bw) for f, bw, g in FORM[v])


# =========================================================================== Nature
@sample('Nature', 'Rain Light', 12)
def _(n):
    drops = scatter(n, 180, lambda r: modal_hit(int(0.03 * SR), [r.uniform(1800, 6000)], [r.uniform(0.002, 0.008)], [r.uniform(0.2, 1)]), 2)
    bed = hpf(pink(n), 900) * 0.25
    return drops + bed

@sample('Nature', 'Rain on Leaves', 12)
def _(n):
    taps = scatter(n, 600, lambda r: bp(noise(int(0.012 * SR)), r.uniform(600, 2500), r.uniform(3000, 9000)) * np.hanning(int(0.012 * SR)) * r.uniform(0.2, 1), 3)
    return taps + lpf(pink(n), 3000) * 0.3 * (0.6 + 0.4 * slow(n, 0.3, 4))

@sample('Nature', 'Mountain Stream', 12)
def _(n):
    bubbles = scatter(n, 90, lambda r: (lambda m, f0: np.sin(2 * np.pi * np.cumsum(f0 * (1 + 0.8 * np.linspace(0, 1, m))) / SR) * np.exp(-np.linspace(0, 6, m)))(int(r.uniform(0.01, 0.05) * SR), r.uniform(500, 2200)) * r.uniform(0.2, 1), 5)
    return bubbles * 0.7 + bp(pink(n), 200, 4000) * 0.5 * (0.7 + 0.3 * slow(n, 1.0, 6))

@sample('Nature', 'Ocean Swell', 16)
def _(n):
    tt = t_axis(n)
    wave = (0.5 - 0.5 * np.cos(2 * np.pi * tt / 7.5)) ** 2
    cut = 300 + 3000 * wave
    x = pink(n)
    # time-varying lowpass in short blocks
    out = np.zeros(n); blk = 2048
    for i in range(0, n, blk):
        out[i:i + blk] = lpf(x[max(0, i - 4096):i + blk], cut[i])[-len(x[i:i + blk]):]
    return out * (0.25 + wave) + brown(n) * 0.02

@sample('Nature', 'Distant Thunder', 14)
def _(n):
    rumble = lpf(brown(n), 180, 4) * 0.02
    claps = scatter(n, 0.35, lambda r: lpf(noise(int(4 * SR)), r.uniform(120, 400), 2) * np.exp(-np.linspace(0, r.uniform(3, 6), int(4 * SR))) * r.uniform(0.6, 1.2), 7)
    return rumble + claps

@sample('Nature', 'Fire Crackle', 12)
def _(n):
    pops = scatter(n, 25, lambda r: hpf(noise(int(0.004 * SR)), 1500) * np.exp(-np.linspace(0, 5, int(0.004 * SR))) * r.uniform(0.3, 2.5), 8)
    roar = lpf(brown(n), 400) * 0.004 * (0.7 + 0.3 * slow(n, 0.6, 9))
    hiss = bp(noise(n), 2000, 7000) * 0.05
    return pops + roar + hiss

@sample('Nature', 'Cicadas', 12)
def _(n):
    out = np.zeros(n); tt = t_axis(n)
    for k in range(5):
        f = rng.uniform(4200, 7200); am = 0.5 + 0.5 * np.sign(np.sin(2 * np.pi * rng.uniform(40, 120) * tt))
        swell = slow(n, 0.25, 20 + k) ** 2
        out += bp(noise(n), f * 0.92, f * 1.08) * am * swell
    return out

@sample('Nature', 'Night Crickets', 12)
def _(n):
    out = np.zeros(n); tt = t_axis(n)
    for k in range(7):
        f = rng.uniform(3800, 5200); rate = rng.uniform(1.2, 3.5); ph = rng.random()
        gate = ((tt * rate + ph) % 1.0 < 0.18).astype(float)
        trill = 0.5 + 0.5 * np.sin(2 * np.pi * 30 * tt)
        out += osc_sine(f, n) * lpf(gate, 200) * trill * rng.uniform(0.3, 1)
    return out + lpf(pink(n), 500) * 0.15

@sample('Nature', 'Canyon Wind', 16)
def _(n):
    x = pink(n); out = np.zeros(n)
    for k in range(3):
        c = 250 + 900 * slow(n, 0.15, 30 + k)
        blk = 2048
        for i in range(0, n, blk):
            seg = x[max(0, i - 4096):i + blk]
            out[i:i + blk] += resonator(seg, c[i], 6)[-len(x[i:i + blk]):] * (0.4 + 0.6 * slow(n, 0.2, 40 + k)[i])
    return out + lpf(x, 400) * 0.3

@sample('Nature', 'Wind Whistle', 14)
def _(n):
    f = 700 + 500 * slow(n, 0.25, 50) + 60 * slow(n, 2, 51)
    tone = osc_sine(f, n) * slow(n, 0.4, 52) ** 2
    return tone * 0.6 + bp(pink(n), 300, 2500) * 0.4

@sample('Nature', 'Bubbling Spring', 10)
def _(n):
    return scatter(n, 40, lambda r: (lambda m, f0: np.sin(2 * np.pi * np.cumsum(f0 * (1 + 1.5 * np.linspace(0, 1, m) ** 2)) / SR) * np.sin(np.pi * np.linspace(0, 1, m)))(int(r.uniform(0.02, 0.08) * SR), r.uniform(200, 900)) * r.uniform(0.3, 1), 53) + lpf(pink(n), 800) * 0.1

@sample('Nature', 'Ice Creaks', 12)
def _(n):
    def creak(r):
        m = int(r.uniform(0.2, 0.9) * SR); tt = np.arange(m) / SR
        rate = r.uniform(30, 160) * (1 + 0.5 * np.sin(2 * np.pi * r.uniform(1, 4) * tt))
        pulses = (np.diff(np.floor(np.cumsum(rate) / SR), prepend=0) > 0).astype(float)
        return modal_hit(m, [r.uniform(300, 900), r.uniform(1200, 2600)], [0.01, 0.005], [1, 0.5]) * 0 + signal.lfilter(*signal.iirpeak(r.uniform(400, 1800) / (SR / 2), 8), pulses) * np.hanning(m) * 6
    return scatter(n, 1.5, creak, 54) + bp(noise(n), 3000, 9000) * 0.02

# =========================================================================== Organic
@sample('Organic', 'Wood Creak', 12)
def _(n):
    def creak(r):
        m = int(r.uniform(0.6, 1.8) * SR); tt = np.arange(m) / SR
        f = r.uniform(25, 70) * (1 + 0.6 * np.sin(np.pi * tt / tt[-1]))
        pulses = (np.diff(np.floor(np.cumsum(f) / SR), prepend=0) > 0).astype(float)
        body = sum(signal.lfilter(*signal.iirpeak(fc / (SR / 2), 12), pulses) for fc in (r.uniform(180, 320), r.uniform(600, 900), r.uniform(1500, 2200)))
        return body * np.hanning(m) * 3
    return scatter(n, 0.9, creak, 60)

@sample('Organic', 'Stone Scrape', 10)
def _(n):
    g = slow(n, 0.8, 61) ** 1.5
    return bp(noise(n), 300, 5000) * g * (0.6 + 0.4 * (np.random.default_rng(62).random(n) > 0.97)) + resonator(noise(n) * g, 420, 10) * 0.5

@sample('Organic', 'Seed Rattle', 10)
def _(n):
    shakes = (0.5 + 0.5 * np.sin(2 * np.pi * 2.2 * t_axis(n))) ** 4
    grains = scatter(n, 900, lambda r: hpf(noise(64), 2500) * np.hanning(64) * r.uniform(0.1, 1), 63)
    return grains * (0.1 + shakes)

@sample('Organic', 'Bamboo Knocks', 12)
def _(n):
    return scatter(n, 3, lambda r: modal_hit(int(0.6 * SR), np.array([1, 2.76, 5.4]) * r.choice([180, 240, 270, 320, 360]), [0.12, 0.05, 0.02], [1, 0.35, 0.15]), 64)

@sample('Organic', 'Leaves Rustle', 12)
def _(n):
    gust = slow(n, 0.4, 65) ** 2
    return scatter(n, 400, lambda r: bp(noise(200), 1500, 9000) * np.hanning(200) * r.uniform(0.1, 1), 66) * (0.15 + gust) + bp(pink(n), 500, 3000) * gust * 0.4

@sample('Organic', 'Sand Flow', 12)
def _(n):
    return hpf(noise(n), 1800) * (0.4 + 0.6 * slow(n, 0.3, 67)) + scatter(n, 2000, lambda r: noise(16) * 0.4, 68)

# =========================================================================== Voices
def choir(n, v, notes, voices=4, seed=0):
    out = np.zeros(n)
    for k, f in enumerate(notes):
        for j in range(voices):
            g = glottal(f * (1 + 0.004 * (j - voices / 2)), n, vib=rng.uniform(4.6, 5.8), vibd=0.01, seed=seed + 10 * k + j)
            out += vowel(g, v)
    return out + hpf(noise(n), 3000) * 0.02 * len(notes)

@sample('Voices', 'Choir Ah', 12)
def _(n): return choir(n, 'a', [147, 220, 294])

@sample('Voices', 'Choir Oo', 12)
def _(n): return choir(n, 'u', [110, 165, 220], seed=3)

@sample('Voices', 'Choir Vowel Drift', 14)
def _(n):
    g = sum(glottal(f, n, seed=i) for i, f in enumerate([131, 196, 262, 330]))
    seq = 'aoueia'; out = np.zeros(n); seg = n // (len(seq) - 1)
    for i in range(len(seq) - 1):
        a = vowel(g, seq[i]); b = vowel(g, seq[i + 1])
        x = np.clip((np.arange(n) - i * seg) / seg, 0, 1)
        w = ((np.arange(n) >= i * seg) & (np.arange(n) < (i + 1) * seg)).astype(float)
        out += (a * (1 - x) + b * x) * w
    return out

@sample('Voices', 'Monk Hum Low', 12)
def _(n): return choir(n, 'o', [65.4, 98], voices=3, seed=7) + lpf(glottal(32.7, n, seed=9), 300) * 0.3

@sample('Voices', 'Overtone Drone', 14)
def _(n):
    g = glottal(98, n, vibd=0.003, seed=11)
    c = 98 * (6 + 6 * (0.5 - 0.5 * np.cos(2 * np.pi * t_axis(n) / 14)))
    out = np.zeros(n); blk = 1024
    for i in range(0, n, blk):
        seg = g[max(0, i - 4096):i + blk]
        out[i:i + blk] = resonator(seg, c[i], 40)[-len(g[i:i + blk]):]
    return out * 2.5 + vowel(g, 'o') * 0.4

@sample('Voices', 'Whisper Air', 10)
def _(n):
    x = bp(noise(n), 600, 8000)
    seq = 'ahoahi'; out = np.zeros(n); seg = n // len(seq)
    for i, v in enumerate(seq):
        if v == 'h': continue
        out[i * seg:(i + 1) * seg] = vowel(x, v)[i * seg:(i + 1) * seg] * np.hanning(seg)
    return out + x * 0.05

@sample('Voices', 'Breath Cycle', 12)
def _(n):
    br = (0.5 - 0.5 * np.cos(2 * np.pi * t_axis(n) / 4.0)) ** 2
    return bp(noise(n), 400, 4000) * br + vowel(noise(n), 'o') * br * 0.3

# =========================================================================== Metal
@sample('Metal', 'Singing Bowl Pair', 14)
def _(n):
    out = np.zeros(n)
    for f0, at in [(220.0, 0), (233.5, int(4 * SR))]:
        ratios = np.array([1, 2.71, 5.15, 8.3]); out += modal_hit(n, list(f0 * ratios) + list(f0 * ratios * 1.003), [9, 5, 3, 2] * 2, [1, 0.5, 0.25, 0.12] * 2, at)
    return out

@sample('Metal', 'Wind Chimes', 14)
def _(n):
    tones = [523, 587, 698, 784, 880, 1047]
    return scatter(n, 3.5, lambda r: modal_hit(int(4 * SR), np.array([1, 2.76, 5.4, 8.9]) * r.choice(tones), [2.5, 1.2, 0.6, 0.3], [1, 0.4, 0.2, 0.1]) * r.uniform(0.3, 1), 70)

@sample('Metal', 'Bowed Cymbal', 12)
def _(n):
    fr = np.sort(rng.uniform(800, 9000, 40))
    out = sum(osc_sine(f * (1 + 0.002 * slow(n, 0.5, int(f))), n) * slow(n, 0.6, int(f) + 1) ** 3 / (1 + f / 3000) for f in fr)
    return out * env_ar(n, 2.5, 3)

@sample('Metal', 'Bell Tree', 10)
def _(n):
    out = np.zeros(n)
    for i, f in enumerate(np.geomspace(2000, 6000, 18)):
        out += modal_hit(n, [f, f * 2.3], [1.5, 0.6], [1, 0.3], int(i * 0.08 * SR))
    for i, f in enumerate(np.geomspace(6000, 2000, 18)):
        out += modal_hit(n, [f, f * 2.3], [1.5, 0.6], [0.7, 0.2], int((5 + i * 0.1) * SR))
    return out

@sample('Metal', 'Gamelan Pulse', 12)
def _(n):
    scale = [293, 330, 392, 440, 523]; out = np.zeros(n)
    for i in range(24):
        f = scale[[0, 2, 1, 3, 2, 4, 3, 1][i % 8]]
        out += modal_hit(n, [f, f * 2.0, f * 2.74, f * 5.3], [1.6, 0.7, 0.4, 0.2], [1, 0.3, 0.25, 0.1], int(i * 0.48 * SR))
        out += modal_hit(n, [f * 1.006, f * 2.01], [1.6, 0.7], [0.7, 0.2], int(i * 0.48 * SR))
    return out

@sample('Metal', 'Metal Groan', 12)
def _(n):
    f = 55 * (1 + 0.15 * slow(n, 0.3, 75))
    src = lpf(np.sign(osc_sine(f, n)) * (0.6 + 0.4 * slow(n, 2, 76)), 3000)
    return sum(resonator(src, fc, 30) for fc in (310, 742, 1390, 2213, 3311)) * env_ar(n, 1.5, 2)

# =========================================================================== Strings
@sample('Strings', 'Bowed Cello Drone', 14)
def _(n):
    f = 65.4 * (1 + 0.003 * np.sin(2 * np.pi * 5 * t_axis(n)) * slow(n, 0.3, 80))
    ph = np.cumsum(f) / SR; saw = 2 * (ph % 1) - 1
    bow = saw * (0.8 + 0.2 * slow(n, 3, 81)) + 0.03 * noise(n)
    body = sum(g * resonator(bow, fc, 6) for fc, g in [(230, 1.0), (470, 0.7), (900, 0.4), (2200, 0.2)])
    return body * env_ar(n, 1.2, 1.5)

@sample('Strings', 'Harmonic Glissando', 12)
def _(n):
    out = np.zeros(n); seg = n // 16
    for i in range(16):
        k = [2, 3, 4, 5, 6, 8, 6, 5, 4, 3, 5, 6, 8, 10, 12, 8][i]
        out += modal_hit(n, [110 * k], [1.8], [1.0 / np.sqrt(k)], i * seg)
    return out

@sample('Strings', 'Zither Swarm', 12)
def _(n):
    notes = [146.8, 220, 293.7, 329.6, 440, 587.3]
    return scatter(n, 12, lambda r: modal_hit(int(3 * SR), [r.choice(notes) * m for m in (1, 2, 3, 4)], [2, 1.2, 0.7, 0.4], [1, 0.5, 0.3, 0.15]) * r.uniform(0.2, 0.8), 85)

# =========================================================================== Drones
@sample('Drones', 'Didgeridoo', 14)
def _(n):
    f = 69 * (1 + 0.004 * slow(n, 1, 90))
    ph = np.cumsum(f) / SR
    lip = np.maximum(0, np.sin(2 * np.pi * ph)) ** 3
    form = 400 + 1400 * slow(n, 0.7, 91)
    out = np.zeros(n); blk = 1024
    for i in range(0, n, blk):
        seg = lip[max(0, i - 4096):i + blk]
        out[i:i + blk] = (resonator(seg, form[i], 5) + resonator(seg, form[i] * 2.4, 8) * 0.5)[-len(lip[i:i + blk]):]
    return out + lpf(lip, 300) * 0.8

@sample('Drones', 'Shruti Box', 14)
def _(n):
    out = np.zeros(n)
    for f in (130.8, 196, 261.6):
        for d in (-0.6, 0.6):
            ph = np.cumsum(np.full(n, f * 2 ** (d / 1200))) / SR
            reed = np.tanh(3 * np.sin(2 * np.pi * ph)) + 0.4 * np.sin(4 * np.pi * ph)
            out += lpf(reed, 2400)
    bellows = 0.75 + 0.25 * (0.5 - 0.5 * np.cos(2 * np.pi * t_axis(n) / 3.5))
    return out * bellows + bp(noise(n), 1000, 4000) * 0.03

@sample('Drones', 'Harmonium Reed', 12)
def _(n):
    out = np.zeros(n)
    for f in (110, 164.8, 220, 277.2):
        ph = np.cumsum(np.full(n, f)) / SR; sq = np.sign(np.sin(2 * np.pi * ph)) * 0.5 + (2 * (ph % 1) - 1) * 0.5
        out += lpf(sq, 1800)
    return out * env_ar(n, 0.8, 1)

@sample('Drones', 'Tape Loop Pad', 14)
def _(n):
    tt = t_axis(n)
    wow = 1 + 0.004 * np.sin(2 * np.pi * 0.55 * tt) + 0.0015 * np.sin(2 * np.pi * 6.3 * tt)
    out = np.zeros(n)
    for f in (130.8, 164.8, 196, 246.9):
        ph = np.cumsum(f * wow) / SR
        out += lpf(2 * (ph % 1) - 1, 1600) * (0.7 + 0.3 * slow(n, 0.3, int(f)))
    return np.tanh(out * 0.8) + hpf(noise(n), 4000) * 0.02 + lpf(noise(n), 60) * 0.03

@sample('Drones', 'Shortwave Radio', 12)
def _(n):
    tt = t_axis(n)
    car = osc_sine(900 + 700 * slow(n, 0.4, 95) + 40 * np.sin(2 * np.pi * 0.3 * tt), n)
    morse = (np.random.default_rng(96).random(int(n / 2400) + 1) > 0.55).repeat(2400)[:n].astype(float)
    static = bp(noise(n), 300, 5000) * (0.4 + 0.6 * slow(n, 1.2, 97))
    return car * lpf(morse, 80) * 0.6 + static * 0.5 + osc_sine(50, n) * 0.05

@sample('Drones', 'Sub Rumble Field', 14)
def _(n):
    return lpf(brown(n), 120, 4) * 0.02 + osc_sine(41.2 * (1 + 0.01 * slow(n, 0.2, 98)), n) * 0.5 * slow(n, 0.15, 99)

@sample('Drones', 'Organ Cluster', 12)
def _(n):
    out = np.zeros(n)
    for f in (130.8, 146.8, 164.8, 174.6, 196):
        for k, g in ((1, 1), (2, 0.5), (4, 0.25), (3, 0.2)):
            out += osc_sine(f * k * (1 + 0.0007 * rng.standard_normal()), n) * g
    return out * env_ar(n, 1.5, 1.5)

@sample('Drones', 'Electric Hum', 10)
def _(n):
    tt = t_axis(n)
    hum = sum(np.sin(2 * np.pi * 60 * k * tt) / k ** 0.8 for k in range(1, 14))
    return np.tanh(hum * 0.7) * (0.8 + 0.2 * slow(n, 0.5, 100)) + bp(noise(n), 2000, 6000) * 0.05 * slow(n, 4, 101)

# =========================================================================== Textures
@sample('Textures', 'Granular Dust', 10)
def _(n):
    return scatter(n, 300, lambda r: osc_sine(r.uniform(300, 4000), 480) * np.hanning(480) * r.uniform(0.1, 1), 110)

@sample('Textures', 'Vinyl Crackle Bed', 10)
def _(n):
    clicks = scatter(n, 18, lambda r: hpf(noise(30), 800) * np.exp(-np.linspace(0, 4, 30)) * r.uniform(0.5, 3), 111)
    return clicks + lpf(hpf(pink(n), 200), 6000) * 0.08

@sample('Textures', 'Glass Harmonics', 12)
def _(n):
    out = np.zeros(n)
    for k, f in enumerate([880, 1320, 1760, 2217, 2637, 3520]):
        out += osc_sine(f * (1 + 0.0015 * np.sin(2 * np.pi * (0.2 + 0.05 * k) * t_axis(n))), n) * slow(n, 0.35, 120 + k) ** 3
    return out

@sample('Textures', 'Data Stream', 10)
def _(n):
    bits = (np.random.default_rng(121).random(int(n / 120) + 1) > 0.5).repeat(120)[:n] * 2.0 - 1
    return lpf(bits, 4000) * (slow(n, 3, 122) > 0.4) * 0.6 + osc_sine(1200 + 2000 * (slow(n, 6, 123) > 0.5), n) * 0.2

@sample('Textures', 'Frozen Spectrum', 12)
def _(n):
    fr = np.fft.rfftfreq(SR, 1 / SR)
    mag = np.exp(-0.5 * ((np.log(fr + 1) - np.log(1500)) / 0.9) ** 2) * (1 + 3 * (np.random.default_rng(124).random(len(fr)) > 0.995))
    out = np.zeros(n); hop = SR // 4; win = np.hanning(SR)
    for i in range(0, n - SR, hop):
        ph = np.random.default_rng(125 + i).random(len(fr)) * 2 * np.pi
        out[i:i + SR] += np.fft.irfft(mag * np.exp(1j * ph), SR) * win
    return out


# --------------------------------------------------------------------------- output
def write(path, x):
    x = np.asarray(x, dtype=np.float64)
    x = x - np.mean(x)
    f = SR // 50
    x[:f] *= np.linspace(0, 1, f); x[-f:] *= np.linspace(1, 0, f)
    rms = np.sqrt(np.mean(x ** 2))
    g = 0.14 / rms if rms > 1e-9 else 1.0
    y = np.tanh(x * g * 1.1) / 1.1 * 0.95
    pcm = np.clip(np.round(y * 32767), -32767, 32767).astype('<i2')
    fmt = struct.pack('<HHIIHH', 1, 1, SR, SR * 2, 2, 16)
    body = b'WAVE' + b'fmt ' + struct.pack('<I', 16) + fmt + b'data' + struct.pack('<I', pcm.nbytes) + pcm.tobytes()
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as fh:
        fh.write(b'RIFF' + struct.pack('<I', len(body)) + body)
    return 20 * np.log10(np.sqrt(np.mean(y ** 2)) + 1e-12), 20 * np.log10(np.max(np.abs(y)) + 1e-12)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'Samples'
    only = sys.argv[2:]
    for cat, name, secs, fn in SAMPLES:
        if only and name not in only: continue
        n = int(secs * SR)
        x = fn(n)
        assert len(x) == n and np.all(np.isfinite(x)), name
        r, p = write(os.path.join(out, cat, name + '.wav'), x)
        print(f'{cat + "/" + name:34s} rms {r:6.1f} dB  peak {p:5.1f} dB')
    print(len(SAMPLES), 'samples')


if __name__ == '__main__':
    main()
