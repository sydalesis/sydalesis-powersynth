// Driftfield 4.1 — audio analysis for the editor (not real-time): pitch detection, harmonic analysis,
// WAV -> 128-partial spectrum, WAV -> 64-frame wavetable, and WAV writing for captures.
#pragma once
#include "Wavetables.hpp"
#include "Library.hpp"
#include "Shared.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <fstream>

namespace drift { namespace ana {

static constexpr double kPi2 = 6.283185307179586;

struct Info { float f0 = 0.f; float seconds = 0.f; int frames = 0; std::string err; };

inline bool loadMono(const char* path, std::vector<float>& x, float& sr, std::string& err) {
    std::vector<uint8_t> file;
    if (!readFileUtf8(path, file)) { err = "cannot read the file"; return false; }
    if (!decodeWavMono(file, x, sr, &err)) return false;
    if (x.size() < 2048 || sr < 8000.f) { err = "the file is too short"; return false; }
    return true;
}

// YIN-style fundamental estimate of x[at .. at+win); 0 when nothing periodic is found
inline float yin(const float* x, int win, float sr, float fmin, float fmax) {
    const int tmin = std::max(2, (int)(sr / fmax)), tmax = std::min(win / 2, (int)(sr / fmin));
    if (tmax <= tmin + 2) return 0.f;
    std::vector<double> d((size_t)tmax + 1, 0.0), cm((size_t)tmax + 1, 1.0);
    for (int t = 1; t <= tmax; ++t) {
        double s = 0.0;
        for (int i = 0; i < win - tmax; ++i) { const double e = (double)x[i] - x[i + t]; s += e * e; }
        d[(size_t)t] = s;
    }
    double run = 0.0;
    for (int t = 1; t <= tmax; ++t) { run += d[(size_t)t]; cm[(size_t)t] = run > 0.0 ? d[(size_t)t] * t / run : 1.0; }
    int best = -1;
    for (int t = tmin; t < tmax; ++t)
        if (cm[(size_t)t] < 0.15 && cm[(size_t)t] <= cm[(size_t)t - 1] && cm[(size_t)t] <= cm[(size_t)t + 1]) { best = t; break; }
    if (best < 0) {   // no dip under the threshold: the global minimum, if it is clear enough
        double m = 1e9; for (int t = tmin; t < tmax; ++t) if (cm[(size_t)t] < m) { m = cm[(size_t)t]; best = t; }
        if (m > 0.4) return 0.f;
    }
    const double a = cm[(size_t)best - 1], b = cm[(size_t)best], c = cm[(size_t)best + 1];
    const double den = a - 2.0 * b + c;
    const double off = std::fabs(den) > 1e-12 ? 0.5 * (a - c) / den : 0.0;
    return (float)(sr / (best + off));
}

// median f0 over the middle of the file
inline float detectF0(const std::vector<float>& x, float sr) {
    const int win = std::min((int)x.size() / 2, (int)(sr * 0.08f));
    std::vector<float> f;
    const int n = (int)x.size();
    for (int k = 0; k < 9; ++k) {
        const int at = (int)((0.2 + 0.6 * k / 8.0) * (n - win));
        const float v = yin(x.data() + at, win, sr, 30.f, 2500.f);
        if (v > 0.f) f.push_back(v);
    }
    if (f.size() < 3) return 0.f;
    std::sort(f.begin(), f.end());
    return f[f.size() / 2];
}

// amplitude and phase of harmonics 1..nh at sample `center`, Hann window of ~4 periods
inline void harmonicsAt(const std::vector<float>& x, float sr, float f0, int center, int nh, float* amp, float* ph) {
    int N = (int)std::lround(4.0 * sr / f0);
    N = std::max(512, std::min(N, 32768));
    const int start = std::max(0, std::min((int)x.size() - N, center - N / 2));
    N = std::min(N, (int)x.size() - start);
    double wsum = 0.0;
    for (int i = 0; i < N; ++i) wsum += 0.5 - 0.5 * std::cos(kPi2 * i / (N - 1));
    for (int h = 1; h <= nh; ++h) {
        const double f = f0 * h;
        if (f >= sr * 0.48) { amp[h - 1] = 0.f; ph[h - 1] = 0.f; continue; }
        const double w = kPi2 * f / sr;
        double re = 0.0, im = 0.0, cr = 1.0, ci = 0.0;
        const double sr1 = std::cos(w), si1 = -std::sin(w);
        for (int i = 0; i < N; ++i) {
            const double v = x[(size_t)(start + i)] * (0.5 - 0.5 * std::cos(kPi2 * i / (N - 1)));
            re += v * cr; im += v * ci;
            const double nr = cr * sr1 - ci * si1; ci = cr * si1 + ci * sr1; cr = nr;
        }
        amp[h - 1] = (float)(2.0 * std::sqrt(re * re + im * im) / wsum);
        ph[h - 1] = (float)std::atan2(im, re);
    }
}

// Any pitched WAV -> 128 harmonic amplitudes (0..1, loudest = 1), averaged over the middle 60 %
inline bool toSpectrum(const char* path, float* amps128, Info& info) {
    std::vector<float> x; float sr;
    if (!loadMono(path, x, sr, info.err)) return false;
    info.seconds = (float)x.size() / sr;
    const float f0 = detectF0(x, sr);
    if (f0 <= 0.f) { info.err = "no clear pitch found (the analysis needs a pitched sound)"; return false; }
    info.f0 = f0;
    std::vector<float> acc(128, 0.f), a(128), p(128);
    const int n = (int)x.size(), steps = 12;
    for (int k = 0; k < steps; ++k) {
        harmonicsAt(x, sr, f0, (int)((0.2 + 0.6 * k / (steps - 1.0)) * n), 128, a.data(), p.data());
        for (int h = 0; h < 128; ++h) acc[(size_t)h] += a[(size_t)h];
    }
    const float mx = *std::max_element(acc.begin(), acc.end());
    if (mx <= 0.f) { info.err = "the file is silent"; return false; }
    for (int h = 0; h < 128; ++h) amps128[h] = acc[(size_t)h] / mx;
    info.frames = steps;
    return true;
}

inline void le16(std::string& s, uint32_t v) { s += (char)(v & 255); s += (char)((v >> 8) & 255); }
inline void le32(std::string& s, uint32_t v) { le16(s, v & 0xFFFF); le16(s, v >> 16); }

// 32-bit float WAV, optional Serum "clm " frame-size chunk
inline bool writeWav(const std::string& path, const std::vector<float>& data, int channels, int sr, int clmFrame = 0) {
    std::string body = "WAVE";
    body += "fmt "; le32(body, 16); le16(body, 3); le16(body, (uint32_t)channels); le32(body, (uint32_t)sr);
    le32(body, (uint32_t)(sr * channels * 4)); le16(body, (uint32_t)(channels * 4)); le16(body, 32);
    if (clmFrame > 0) {
        char c[64]; std::snprintf(c, sizeof c, "<!>%d 10000000 wavetable (Driftfield)", clmFrame);
        std::string cs = c; if (cs.size() & 1) cs += '\0';
        body += "clm "; le32(body, (uint32_t)cs.size()); body += cs;
    }
    body += "data"; le32(body, (uint32_t)(data.size() * 4));
    std::string out = "RIFF"; le32(out, (uint32_t)(body.size() + data.size() * 4)); out += body;
    std::ofstream f(lib::fromU8(path), std::ios::binary);
    if (!f) return false;
    f.write(out.data(), (std::streamsize)out.size());
    f.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)(data.size() * 4));
    return (bool)f;
}

// Any pitched WAV -> a 64-frame, 2048-sample wavetable resynthesised from its harmonics through the
// file (phase-locked to the fundamental so the frames morph without phasing). Writes outPath.
inline bool toWavetable(const char* path, const std::string& outPath, Info& info, int frames = 64) {
    std::vector<float> x; float sr;
    if (!loadMono(path, x, sr, info.err)) return false;
    info.seconds = (float)x.size() / sr;
    const float f0 = detectF0(x, sr);
    if (f0 <= 0.f) { info.err = "no clear pitch found (the analysis needs a pitched sound)"; return false; }
    info.f0 = f0;
    const int nh = std::min(256, (int)(sr * 0.48f / f0));
    const int W = kWtSize, n = (int)x.size();
    const int pad = std::min(n / 4, (int)(4.0 * sr / f0));
    std::vector<float> table((size_t)frames * W, 0.f), a((size_t)nh), p((size_t)nh);
    for (int fr = 0; fr < frames; ++fr) {
        const int center = pad + (int)((double)fr / std::max(1, frames - 1) * (n - 2 * pad));
        // local pitch, kept within 3 semitones of the file's
        float lf = yin(x.data() + std::max(0, std::min(n - 4096, center - 2048)), std::min(4096, n), sr, f0 / 1.19f, f0 * 1.19f);
        if (lf <= 0.f) lf = f0;
        harmonicsAt(x, sr, lf, center, nh, a.data(), p.data());
        float* cyc = table.data() + (size_t)fr * W;
        for (int h = 1; h <= nh; ++h) {
            const float am = a[(size_t)h - 1];
            if (am < 1e-6f) continue;
            const double phase = p[(size_t)h - 1] - h * p[0];
            for (int i = 0; i < W; ++i) cyc[i] += am * (float)std::cos(kPi2 * h * i / W + phase);
        }
    }
    float mx = 0.f; for (float v : table) mx = std::max(mx, std::fabs(v));
    if (mx <= 0.f) { info.err = "the file is silent"; return false; }
    for (float& v : table) v *= 0.95f / mx;
    info.frames = frames;
    std::error_code ec;
    std::filesystem::create_directories(lib::fromU8(outPath).parent_path(), ec);
    if (!writeWav(outPath, table, 1, 48000, W)) { info.err = "could not write the wavetable file"; return false; }
    return true;
}

// ---- 4.2: WAV -> modal bank (physical model "Analysed") ------------------------------------------
// in-place radix-2 FFT, n a power of two
inline void fft(std::vector<double>& re, std::vector<double>& im) {
    const size_t n = re.size();
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (size_t len = 2; len <= n; len <<= 1) {
        const double a = -kPi2 / (double)len, wr = std::cos(a), wi = std::sin(a);
        for (size_t i = 0; i < n; i += len) {
            double cr = 1.0, ci = 0.0;
            for (size_t k = 0; k < len / 2; ++k) {
                const size_t u = i + k, v = i + k + len / 2;
                const double tr = re[v] * cr - im[v] * ci, ti = re[v] * ci + im[v] * cr;
                re[v] = re[u] - tr; im[v] = im[u] - ti; re[u] += tr; im[u] += ti;
                const double nr = cr * wr - ci * wi; ci = cr * wi + ci * wr; cr = nr;
            }
        }
    }
}
// Hann-windowed magnitude spectrum of x[at .. at+N), zero beyond the end
inline void magSpectrum(const std::vector<float>& x, int at, int N, std::vector<double>& mag) {
    std::vector<double> re((size_t)N, 0.0), im((size_t)N, 0.0);
    for (int i = 0; i < N; ++i) {
        const int k = at + i;
        if (k >= 0 && k < (int)x.size()) re[(size_t)i] = x[(size_t)k] * (0.5 - 0.5 * std::cos(kPi2 * i / (N - 1)));
    }
    fft(re, im);
    mag.assign((size_t)N / 2, 0.0);
    for (int i = 0; i < N / 2; ++i) mag[(size_t)i] = std::sqrt(re[(size_t)i] * re[(size_t)i] + im[(size_t)i] * im[(size_t)i]);
}
// A struck or bowed recording (bell, bowl, gong, plate, glass...) -> up to 48 resonant modes:
// frequency ratios to the lowest strong peak, relative amplitudes, and decay times from how much
// each peak falls between an early and a later window. info.f0 = frequency of ratio 1.
inline bool toModes(const char* path, ModalData& out, Info& info) {
    std::vector<float> x; float sr;
    if (!loadMono(path, x, sr, info.err)) return false;
    info.seconds = (float)x.size() / sr;
    float pk = 0.f; for (float v : x) pk = std::max(pk, std::fabs(v));
    if (pk <= 1e-5f) { info.err = "the file is silent"; return false; }
    int onset = 0; while (onset < (int)x.size() && std::fabs(x[(size_t)onset]) < 0.1f * pk) ++onset;
    int N = 1; while (N < (int)(sr * 0.17f)) N <<= 1;               // ~6 Hz resolution at 48 kHz
    const int a1 = onset + (int)(0.02f * sr);
    const int remain = (int)x.size() - a1 - N;
    if (remain < (int)(0.05f * sr)) { info.err = "the file is too short to measure decays (needs about half a second)"; return false; }
    const int gap = std::min(remain, (int)(0.6f * sr));
    const int a2 = a1 + gap;
    const double dt = (double)gap / sr;
    std::vector<double> m1, m2;
    magSpectrum(x, a1, N, m1);
    magSpectrum(x, a2, N, m2);
    const double binHz = sr / (double)N;
    double mx = 0.0; for (size_t i = 2; i < m1.size(); ++i) mx = std::max(mx, m1[i]);
    if (mx <= 0.0) { info.err = "the file is silent"; return false; }
    struct Pk { double f, a, d; };
    std::vector<Pk> peaks;
    const int lo = std::max(2, (int)(25.0 / binHz)), hi = std::min((int)m1.size() - 2, (int)(0.45 * sr / binHz));
    for (int i = lo; i < hi; ++i) {
        const double y = m1[(size_t)i];
        if (y < mx * 0.003 || y < m1[(size_t)i - 1] || y <= m1[(size_t)i + 1] || y < m1[(size_t)i - 2] || y < m1[(size_t)i + 2]) continue;
        const double ya = m1[(size_t)i - 1], yb = m1[(size_t)i + 1];
        const double den = ya - 2.0 * y + yb;
        const double off = std::fabs(den) > 1e-12 ? 0.5 * (ya - yb) / den : 0.0;
        const double f = (i + off) * binHz;
        // decay: the same peak (+-1 bin) in the later window
        double l2 = 0.0; for (int k = -1; k <= 1; ++k) l2 = std::max(l2, m2[(size_t)(i + k)]);
        double t60 = 60.0;
        if (l2 > 1e-12 && l2 < y) t60 = 6.9078 * dt / std::log(y / l2);
        peaks.push_back({ f, y / mx, std::min(60.0, std::max(0.05, t60)) });
    }
    if (peaks.empty()) { info.err = "no resonant peaks found"; return false; }
    std::sort(peaks.begin(), peaks.end(), [](const Pk& a, const Pk& b) { return a.a > b.a; });
    if (peaks.size() > (size_t)kModalMax) peaks.resize((size_t)kModalMax);
    std::sort(peaks.begin(), peaks.end(), [](const Pk& a, const Pk& b) { return a.f < b.f; });
    // ratio 1 = the lowest peak at least 20 % of the strongest
    double f0 = peaks[0].f;
    for (const auto& q : peaks) if (q.a >= 0.2) { f0 = q.f; break; }
    out = ModalData();
    for (const auto& q : peaks) {
        if (out.count >= kModalMax) break;
        const double r = q.f / f0;
        if (r < 0.06 || r > 190.0) continue;
        out.ratio[out.count] = (float)r; out.amp[out.count] = (float)q.a; out.decay[out.count] = (float)q.d;
        ++out.count;
    }
    info.f0 = (float)f0;
    info.frames = out.count;
    return out.count > 0;
}

}} // namespace drift::ana
