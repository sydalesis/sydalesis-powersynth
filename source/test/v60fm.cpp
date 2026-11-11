// Sydalesis 6.0: FM oscillator (6 operators, 32 algorithms) and the parameter extension block.
// build (from source/test): g++ -std=gnu++17 -O2 -ffast-math -fno-finite-math-only -I.. v60fm.cpp ../EngineGeneric.cpp ../EngineFactory.cpp ../EngineAVX2.cpp ../Platform.cpp -o v60fm -ldl -lpthread
#include "../Params.hpp"
#include "../Shared.hpp"
#include "../EngineIface.hpp"
#include "../FmAlgos.hpp"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <complex>
#include <chrono>
#include <vector>
static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)
static const int SR = 48000;
static int sym(const char* s) { int i = ParamRegistry::get().findSymbol(s); if (i < 0) { std::printf("no symbol %s\n", s); ++fails; return 0; } return i; }
static void set(drift::IEngine* e, const char* s, float v) { uint32_t en; hostToEngine((uint32_t)sym(s), en); e->setParam(en, v); }

static drift::IEngine* fresh(bool hq = false) {
    drift::IEngine* e = drift::createEngine(true); e->setSampleRate((float)SR);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    for (uint32_t i = pX0; i < pX0 + kExtParams; ++i) e->setParam(i, P(i).def);
    set(e, "quality", hq ? 1.f : 0.f);
    set(e, "obOn", 0.f); set(e, "ocOn", 0.f); set(e, "oaOn", 1.f);
    set(e, "oaType", (float)otFm); set(e, "oaVoices", 1.f);
    set(e, "f1Type", 0.f); set(e, "f2Type", 0.f); set(e, "subLevel", 0.f); set(e, "noiseLevel", 0.f);
    set(e, "aeA", 0.001f); set(e, "aeS", 1.f);
    for (int u = 0; u < kFxUnits; ++u) e->setParam(kFxOnParam[u], 0.f);
    return e;
}
static std::vector<float> render(drift::IEngine* e, int note = 57, int secs = 1, int offAt = -1) {
    const uint8_t on[3] = { 0x90, (uint8_t)note, 100 }; e->midi(on, 3);
    const int n = SR * secs;
    std::vector<float> L(n), R(n);
    for (int p = 0; p < n; p += 256) {
        if (offAt >= 0 && p == offAt) { const uint8_t off[3] = { 0x80, (uint8_t)note, 0 }; e->midi(off, 3); }
        e->process(&L[p], &R[p], std::min(256, n - p));
    }
    return L;
}
static double rmsFrom(const std::vector<float>& x, int a, int b) { double s = 0; for (int i = a; i < b; ++i) s += (double)x[i] * x[i]; return std::sqrt(s / (b - a)); }
static bool finite(const std::vector<float>& x) { for (float v : x) if (!std::isfinite(v)) return false; return true; }
static float peak(const std::vector<float>& x) { float m = 0; for (float v : x) m = std::max(m, std::fabs(v)); return m; }
// magnitude spectrum (Hann, 16384) of x from offset
static std::vector<double> spectrum(const std::vector<float>& x, int off) {
    const int N = 16384; std::vector<std::complex<double>> a(N);
    for (int i = 0; i < N; ++i) { const double t = 2 * M_PI * i / N; a[i] = x[off + i] * (0.35875 - 0.48829 * std::cos(t) + 0.14128 * std::cos(2 * t) - 0.01168 * std::cos(3 * t)); }   // Blackman-Harris
    for (int i = 1, j = 0; i < N; ++i) { int bit = N >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) std::swap(a[i], a[j]); }
    for (int len = 2; len <= N; len <<= 1) { const double ang = -2 * M_PI / len; const std::complex<double> wl(std::cos(ang), std::sin(ang));
        for (int i = 0; i < N; i += len) { std::complex<double> w(1); for (int j = 0; j < len / 2; ++j) { auto u = a[i + j], v = a[i + j + len / 2] * w; a[i + j] = u + v; a[i + j + len / 2] = u - v; w *= wl; } } }
    std::vector<double> m(N / 2); for (int i = 0; i < N / 2; ++i) m[i] = std::abs(a[i]); return m;
}
// energy share (dB) of bins that are not within +-3 bins of a harmonic of f0
static double inharmonicDb(const std::vector<float>& x, double f0) {
    auto m = spectrum(x, SR / 2);
    const double bin = (double)SR / 16384;
    double h = 0, o = 0;
    for (int i = 2; i < (int)m.size(); ++i) {
        const double f = i * bin, k = std::round(f / f0);
        const bool near = k >= 1 && std::fabs(f - k * f0) <= 6 * bin;
        (near ? h : o) += m[i] * m[i];
    }
    return 10 * std::log10(o / (h + 1e-30) + 1e-30);
}

int main() {
    if (ParamRegistry::get().count() != kParamCount) { std::printf("REGISTRY MISMATCH %zu vs %u\n", (size_t)ParamRegistry::get().count(), kParamCount); return 1; }
    // ---- parameter layout
    CHECK(pX0 == kParamCountV5, "extension block starts after every 5.x parameter");
    CHECK(std::strcmp(P(fmP(0, fmAlgo)).symbol, "oaFmAlgo") == 0 && std::strcmp(P(fmOpP(2, 5, foKey)).symbol, "ocFm6Key") == 0, "FM symbols (layer 1)");
    CHECK(std::strcmp(P(layer2Of(fmP(1, fmIndex))).symbol, "b_obFmIndex") == 0, "FM symbols (layer 2 copy)");
    { uint32_t e; CHECK(hostToEngine(layer2Of(fmOpP(1, 3, foLevel)), e) == 1 && e == fmOpP(1, 3, foLevel), "layer-2 FM parameter routes to engine 2, same engine index"); }
    { uint32_t e; CHECK(hostToEngine(pLyMode, e) == -1 && hostToEngine(pMaster, e) == 0 && hostToEngine(pMaster + kLayerBOffset, e) == 1 && e == pMaster, "5.x routing unchanged"); }
    { int n = 0; forEachLayerParam([&](uint32_t) { ++n; }); CHECK(n == (int)(kLayerParams + kExtParams), "forEachLayerParam covers both blocks"); }
    // ---- algorithm table
    {
        const drift::FmAlgo& a1 = drift::fmAlgoDef(0);
        CHECK(a1.carriers == 0x05 && a1.mods[0] == 0x02 && a1.mods[2] == 0x08 && a1.mods[4] == 0x20 && a1.depth[5] == 3, "algorithm 1: 2>1, 6>5>4>3, carriers 1+3");
        CHECK(drift::fmAlgoDef(31).carriers == 0x3f, "algorithm 32: six carriers");
        CHECK(drift::fmAlgoDef(3).fbSrc == 3 && drift::fmAlgoDef(3).fbDst == 5, "algorithm 4: loop 4 -> 6");
        bool order = true;
        for (int i = 0; i < 32; ++i) for (int k = 0; k < 6; ++k) for (int m = 0; m < 6; ++m) if ((drift::fmAlgoDef(i).mods[k] >> m) & 1 && m <= k) order = false;
        CHECK(order, "every modulator has a higher number than its target (one pass from op 6 down)");
    }
    // ---- reference levels
    {
        auto* e = fresh(); set(e, "oaType", (float)otAnalog); set(e, "oaAnWave", 0.f); auto a = render(e); delete e;
        e = fresh(); set(e, "oaType", (float)otAnalog); set(e, "oaAnWave", 4.f); auto b = render(e); delete e;
        std::printf("      reference: analog saw %.1f dB, analog sine %.1f dB, sine inharmonic %.1f dB\n", 20 * std::log10(rmsFrom(a, SR / 4, SR)), 20 * std::log10(rmsFrom(b, SR / 4, SR)), inharmonicDb(b, 220.0));
    }
    // ---- init sound
    {
        auto* e = fresh(); auto x = render(e); delete e;
        const double r = rmsFrom(x, SR / 4, SR);
        std::printf("      init FM rms %.1f dB, inharmonic %.1f dB\n", 20 * std::log10(r), inharmonicDb(x, 220.0));
        CHECK(finite(x) && r > 0.02 && r < 0.6, "init FM patch sounds at a normal level");
        CHECK(inharmonicDb(x, 220.0) < -70, "init FM patch is harmonic and alias-free");
    }
    // ---- a single carrier is a clean sine
    {
        auto* e = fresh(); set(e, "oaFm2Level", 0.f); auto x = render(e); delete e;
        auto m = spectrum(x, SR / 2);
        const int b1 = (int)std::round(220.0 / ((double)SR / 16384));
        double f = 0, h = 0; for (int i = 2; i < (int)m.size(); ++i) { if (std::abs(i - b1) <= 6) f += m[i] * m[i]; else h += m[i] * m[i]; }
        std::printf("      carrier sine THD+N %.1f dB\n", 10 * std::log10(h / f));
        CHECK(10 * std::log10(h / f) < -90, "carrier sine is pure (< -90 dB)");
    }
    // ---- index moves the spectrum, modulator envelope shapes it over time
    {
        auto* e = fresh(); set(e, "oaFmIndex", 0.f); auto a = render(e); delete e;
        e = fresh(); set(e, "oaFmIndex", 2.f); auto b = render(e); delete e;
        auto ma = spectrum(a, SR / 2), mb = spectrum(b, SR / 2);
        auto cent = [](const std::vector<double>& m) { double s = 0, w = 0; for (int i = 1; i < (int)m.size(); ++i) { s += i * m[i]; w += m[i]; } return s / w * SR / 16384; };
        std::printf("      centroid index 0: %.0f Hz, index 2: %.0f Hz\n", cent(ma), cent(mb));
        CHECK(cent(mb) > cent(ma) * 2.5, "index brightens");
        e = fresh(); set(e, "oaFm2Dec", 1.5f); set(e, "oaFm2Sus", 0.f); set(e, "oaFmIndex", 2.f); auto c = render(e, 57, 2); delete e;
        auto m0 = spectrum(c, 2400), m1 = spectrum(c, SR + SR / 2);
        CHECK(cent(m1) < cent(m0) * 0.6, "a decaying modulator envelope darkens the note (%.0f -> %.0f Hz)", cent(m0), cent(m1));
    }
    // ---- carrier envelope and release
    {
        auto* e = fresh(); set(e, "aeR", 0.05f); set(e, "oaFm1Rel", 0.2f); auto x = render(e, 57, 2, 24064); delete e;
        CHECK(rmsFrom(x, 24064 - 4800, 24064) > 0.01 && rmsFrom(x, SR + SR / 2, 2 * SR) < 1e-4, "note off: operator release closes the note");
    }
    // ---- every algorithm, hard settings, high and low notes: finite, bounded, little aliasing
    {
        bool ok = true; float worstPk = 0; double worstAl = -200; int worstA = -1;
        for (int a = 0; a < 32; ++a) for (int note : { 36, 84, 103 }) {
            auto* e = fresh();
            set(e, "oaFmAlgo", (float)a); set(e, "oaFmIndex", 3.f); set(e, "oaFmFeedback", 1.f); set(e, "oaFmBright", 1.f);
            const char* lv[6] = { "oaFm1Level", "oaFm2Level", "oaFm3Level", "oaFm4Level", "oaFm5Level", "oaFm6Level" };
            const char* rt[6] = { "oaFm1Coarse", "oaFm2Coarse", "oaFm3Coarse", "oaFm4Coarse", "oaFm5Coarse", "oaFm6Coarse" };
            static const float ratios[6] = { 1, 3, 1, 7, 2, 14 };
            for (int k = 0; k < 6; ++k) { set(e, lv[k], 1.f); set(e, rt[k], ratios[k]); }
            auto x = render(e, note); delete e;
            const float pk = peak(x);
            if (!finite(x) || pk > 3.f) { ok = false; std::printf("      algo %d note %d: peak %.2f finite %d\n", a + 1, note, pk, (int)finite(x)); }
            worstPk = std::max(worstPk, pk);
            if (note == 103) {
                // all ratios are integers -> everything that is not a harmonic is aliasing (or feedback noise)
                const double f0 = 440.0 * std::pow(2.0, (note - 69) / 12.0);
                const double al = inharmonicDb(x, f0);
                if (al > worstAl) { worstAl = al; worstA = a + 1; }
            }
        }
        std::printf("      worst peak %.2f, worst aliasing at G7 %.1f dB (algorithm %d)\n", worstPk, worstAl, worstA);
        CHECK(ok, "32 algorithms x full levels, index 3, feedback 1: finite and bounded");
        CHECK(worstAl < -20, "anti-alias taper keeps G7 at full index mostly harmonic");
    }
    // ---- feedback: a single op with feedback goes saw-like (more harmonics), stays finite
    {
        auto* e = fresh(); set(e, "oaFmAlgo", 31.f); set(e, "oaFm2Level", 0.f); set(e, "oaFm6Level", 1.f); set(e, "oaFm1Level", 0.f); set(e, "oaFmFeedback", 0.f); auto a = render(e); delete e;
        e = fresh(); set(e, "oaFmAlgo", 31.f); set(e, "oaFm2Level", 0.f); set(e, "oaFm6Level", 1.f); set(e, "oaFm1Level", 0.f); set(e, "oaFmFeedback", 1.f); auto b = render(e); delete e;
        auto ma = spectrum(a, SR / 2), mb = spectrum(b, SR / 2);
        const int b2 = (int)std::round(440.0 / ((double)SR / 16384));
        double h2a = 0, h2b = 0; for (int i = b2 - 3; i <= b2 + 3; ++i) { h2a += ma[i] * ma[i]; h2b += mb[i] * mb[i]; }
        CHECK(h2b > h2a * 1000 && finite(b), "op-6 feedback adds harmonics");
    }
    // ---- waves differ and are DC-free at the output
    {
        std::vector<float> w0;
        bool differ = true, dcOk = true;
        for (int w = 0; w < kFmWaves; ++w) {
            auto* e = fresh(); set(e, "oaFm2Level", 0.f); set(e, "oaFm1Wave", (float)w); auto x = render(e); delete e;
            double dc = 0; for (int i = SR / 2; i < SR; ++i) dc += x[i]; dc /= SR / 2;
            if (std::fabs(dc) > 0.01 * rmsFrom(x, SR / 2, SR) + 1e-4) { dcOk = false; std::printf("      wave %d dc %.4f\n", w, dc); }
            if (w == 0) w0 = x; else { double d = 0; for (int i = SR / 2; i < SR; ++i) d += std::fabs(x[i] - w0[i]); if (d < 10) differ = false; }
        }
        CHECK(differ, "8 operator waves differ");
        CHECK(dcOk, "carrier waves are DC-corrected");
    }
    // ---- timbre destinations reach the FM engine
    {
        auto* e = fresh(); auto a = render(e); delete e;
        e = fresh(); set(e, "macro1", 1.f); e->setParam(modSrcParam(0), (float)msMacro1); e->setParam(modDstParam(0), (float)mdAT1); e->setParam(modAmtParam(0), 1.f);
        auto b = render(e); delete e;
        e = fresh(); set(e, "macro1", 1.f); e->setParam(modSrcParam(0), (float)msMacro1); e->setParam(modDstParam(0), (float)(mdAT5 + 2)); e->setParam(modAmtParam(0), 1.f);
        auto c = render(e); delete e;
        double d1 = 0, d2 = 0; for (int i = SR / 2; i < SR; ++i) { d1 += std::fabs(a[i] - b[i]); d2 += std::fabs(a[i] - c[i]); }
        CHECK(d1 > 100 && d2 > 100, "Timbre 1 (index) and Timbre 7 (op 3 level) modulate the FM oscillator");
    }
    // ---- HQ
    {
        auto* e = fresh(true); auto x = render(e); delete e;
        CHECK(finite(x) && rmsFrom(x, SR / 4, SR) > 0.01, "HQ renders FM");
    }
    // ---- CPU: 16 voices, 4 unison, 3 FM oscillators
    {
        auto* e = fresh();
        set(e, "obOn", 1.f); set(e, "ocOn", 1.f); set(e, "obType", (float)otFm); set(e, "ocType", (float)otFm);
        for (const char* s : { "oaVoices", "obVoices", "ocVoices" }) set(e, s, 4.f);
        for (const char* s : { "oaFmAlgo", "obFmAlgo", "ocFmAlgo" }) set(e, s, 0.f);
        for (int o = 0; o < 3; ++o) for (int k = 0; k < 6; ++k) e->setParam(fmOpP(o, k, foLevel), 0.7f);
        for (int k = 0; k < 16; ++k) { const uint8_t on[3] = { 0x90, (uint8_t)(40 + k * 3), 100 }; e->midi(on, 3); }
        std::vector<float> L(SR + 256), R(SR + 256);
        auto t0 = std::chrono::steady_clock::now();
        for (int p = 0; p < SR * 2; p += 256) e->process(&L[0], &R[0], 256);
        const double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        std::printf("      16 voices x 3 FM osc x 4 unison: %.1f%% of one core (%s)\n", sec / 2 * 100, e->isaName());
        delete e;
        CHECK(sec / 2 < 1.5, "FM CPU acceptable (worst case: 288 operator streams)");
    }
    std::printf(fails ? "\n%d FAILED\n" : "\nall passed\n", fails);
    return fails ? 1 : 0;
}
