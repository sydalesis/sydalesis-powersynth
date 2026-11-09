// Driftfield 3 — additive base spectra (128 partial amplitudes each, peak = 1).
#pragma once
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>

// The engine builds (generic / AVX2) set DRIFT_DSP_NS so their copies of this code never merge.
#ifndef DRIFT_DSP_NS
#define DRIFT_DSP_NS drift
#endif
namespace DRIFT_DSP_NS {

static constexpr int kPartials = 128;
static constexpr int kBaseSpectra = 15;   // + Custom

static inline void baseSpectrum(int type, float* a) {
    for (int i = 0; i < kPartials; ++i) a[i] = 0.f;
    auto bump = [](float hz, float fc, float bw) { const float d = (hz - fc) / bw; return std::exp(-0.5f * d * d); };
    auto fbump = [](float h, float c, float w) { const float d = (h - c) / w; return std::exp(-0.5f * d * d); };
    for (int i = 0; i < kPartials; ++i) {
        const int h = i + 1;
        const float fh = (float)h;
        const bool odd = (h & 1) != 0;
        float v = 0.f;
        switch (type) {
        case 0: v = 1.f / fh; break;                                  // saw
        case 1: v = odd ? 1.f / fh : 0.f; break;                      // square
        case 2: v = odd ? 1.f / (fh * fh) : 0.f; break;               // triangle
        case 3: {                                                     // organ drawbars
            static const int hs[9] = { 1, 2, 3, 4, 6, 8, 10, 12, 16 };
            static const float g[9] = { 1.f, 0.8f, 0.6f, 0.5f, 0.35f, 0.3f, 0.15f, 0.12f, 0.1f };
            for (int k = 0; k < 9; ++k) if (hs[k] == h) v = g[k];
            break;
        }
        case 4: { const float hz = fh * 160.f;                        // choir "ah"
                  v = std::pow(fh, -0.6f) * (0.03f + bump(hz, 700, 110) + 0.6f * bump(hz, 1150, 130) + 0.25f * bump(hz, 2800, 220)); break; }
        case 5: { const float hz = fh * 160.f;                        // choir "oo"
                  v = std::pow(fh, -0.6f) * (0.03f + bump(hz, 300, 80) + 0.5f * bump(hz, 870, 110) + 0.15f * bump(hz, 2240, 200)); break; }
        case 6: {                                                     // bell (harmonic approximation)
            static const int hs[10] = { 1, 2, 3, 5, 6, 8, 12, 16, 20, 27 };
            static const float g[10] = { 0.5f, 1.f, 0.55f, 0.7f, 0.35f, 0.45f, 0.3f, 0.22f, 0.15f, 0.1f };
            for (int k = 0; k < 10; ++k) if (hs[k] == h) v = g[k];
            if (v == 0.f) v = 0.01f / fh;
            break;
        }
        case 7: {                                                     // glass (fibonacci)
            static const int hs[10] = { 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
            for (int k = 0; k < 10; ++k) if (hs[k] == h) v = 1.f / std::sqrt(fh);
            break;
        }
        case 8: v = h == 1 ? 1.f : (odd && h >= 5 ? 0.9f * std::pow(fh, -0.7f) : 0.f); break;   // hollow
        case 9: { const float hz = fh * 196.f;                        // strings (bowed body)
                  v = (1.f / fh) * (0.35f + bump(hz, 280, 50) + 0.8f * bump(hz, 460, 70) + 0.6f * bump(hz, 700, 90)
                                    + 0.5f * bump(hz, 1100, 140) + 0.45f * bump(hz, 2500, 300)); break; }
        case 10: v = (odd ? 1.f : 0.25f) / fh * (1.f + 1.5f * fbump(fh, 10.f, 3.f)); break;     // reed
        case 11: v = std::pow(fh, -0.9f) * 0.08f + fbump(fh, 1, 0.6f) + 0.6f * fbump(fh, 3, 0.8f)   // throat
                   + 0.7f * fbump(fh, 6, 1.f) + 0.3f * fbump(fh, 12, 1.5f) + 0.15f * fbump(fh, 24, 2.5f); break;
        case 12: { int k = 0; for (int x = 1; x <= kPartials; x *= 2, ++k) if (x == h) v = 1.f / (1.f + 0.6f * k); break; }   // octaves
        case 13: { int k = 0; for (int x = 1; x <= kPartials; x *= 3, ++k) if (x == h) v = 1.f / (1.f + 0.4f * k);           // fifths (3^n)
                   if (h == 2 || h == 6 || h == 18 || h == 54) v = 0.35f; break; }
        case 14: {                                                    // primes
            bool prime = h >= 2;
            for (int d = 2; d * d <= h && prime; ++d) if (h % d == 0) prime = false;
            if (h == 1) v = 1.f; else if (prime) { int k = 0; for (int x = 2; x < h; ++x) { bool p2 = true; for (int d = 2; d * d <= x; ++d) if (x % d == 0) { p2 = false; break; } if (p2) ++k; } v = 1.f / (1.f + 0.25f * (k + 1)); }
            break;
        }
        default: v = 1.f / fh; break;
        }
        a[i] = v;
    }
    float mx = 0.f;
    for (int i = 0; i < kPartials; ++i) mx = mx > a[i] ? mx : a[i];
    if (mx > 0.f) for (int i = 0; i < kPartials; ++i) a[i] /= mx;
}

// Precomputed table of all non-custom spectra
struct SpectrumTable {
    float a[kBaseSpectra][kPartials];
    SpectrumTable() { for (int t = 0; t < kBaseSpectra; ++t) baseSpectrum(t, a[t]); }
    static const SpectrumTable& get() { static SpectrumTable t; return t; }
};

// Custom spectrum <-> state string (up to 128 values 0..1000, comma separated). Shorter strings
// (v2 used 64) leave the rest at zero.
static inline std::string spectrumToString(const float* a) {
    int last = 0;
    for (int i = 0; i < kPartials; ++i) if ((int)(a[i] * 1000.f + 0.5f) > 0) last = i;
    std::string s; char b[16];
    for (int i = 0; i <= last; ++i) {
        int v = (int)(a[i] * 1000.f + 0.5f);
        if (v < 0) v = 0;
        if (v > 1000) v = 1000;
        std::snprintf(b, sizeof(b), i ? ",%d" : "%d", v);
        s += b;
    }
    return s;
}
static inline bool spectrumFromString(const char* s, float* a) {
    if (!s || !*s) return false;
    int i = 0;
    while (*s && i < kPartials) {
        char* end = nullptr;
        const long v = std::strtol(s, &end, 10);
        if (end == s) break;
        a[i++] = (float)(v < 0 ? 0 : (v > 1000 ? 1000 : v)) / 1000.f;
        s = end;
        if (*s == ',') ++s;
    }
    if (i == 0) return false;
    for (; i < kPartials; ++i) a[i] = 0.f;
    return true;
}
static inline void spectrumDefault(float* a) { for (int i = 0; i < kPartials; ++i) a[i] = 1.f / (float)(i + 1); }

} // namespace DRIFT_DSP_NS
