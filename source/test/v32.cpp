// Driftfield 3.2 feature checks: classic envelope, CPU-guard voice cap, oversampled drive, MIDI map strings.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
#include <complex>
#include <algorithm>

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { ++bad; std::printf("FAIL: "); std::printf(__VA_ARGS__); std::printf("\n"); } else { std::printf("ok:   "); std::printf(__VA_ARGS__); std::printf("\n"); } } while (0)
static void note(drift::IEngine* e, int n, bool on) { uint8_t m[3] = { (uint8_t)(on ? 0x90 : 0x80), (uint8_t)n, 100 }; e->midi(m, 3); }
static void sp(drift::IEngine* e, const char* sym, float v) { int id = ParamRegistry::get().findSymbol(sym); if (id < 0) { std::printf("no sym %s\n", sym); ++bad; return; } e->setParam(id, v); }
static std::vector<float> render(drift::IEngine* e, int frames, std::vector<float>* R = nullptr) {
    std::vector<float> L(frames), r(frames);
    int pos = 0;
    while (pos < frames) { int c = std::min(256, frames - pos); e->process(&L[pos], &r[pos], c); pos += c; }
    for (int i = 0; i < frames; ++i) if (!std::isfinite(L[i])) { ++bad; std::printf("non-finite\n"); break; }
    if (R) *R = r;
    return L;
}
static drift::IEngine* fresh(bool generic) {
    drift::IEngine* e = drift::createEngine(generic);
    e->setSampleRate(48000); e->setTransport(120, false, 0);
    for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, P(i).def);
    return e;
}
static float timeToFrac(const std::vector<float>& x, float frac) {
    // envelope follower on |x|, time (s) until it reaches frac of its max
    std::vector<float> env(x.size()); float s = 0;
    for (size_t i = 0; i < x.size(); ++i) { const float a = std::fabs(x[i]); s = std::max(a, s * 0.9995f); env[i] = s; }
    const float mx = *std::max_element(env.begin(), env.end());
    for (size_t i = 0; i < env.size(); ++i) if (env[i] >= frac * mx) return (float)i / 48000.f;
    return -1;
}
// power outside the harmonics of f0 relative to total, dB (alias measure)
static double inharmonicDb(const std::vector<float>& x, double f0) {
    const int N = 32768; const int off = (int)x.size() - N;
    std::vector<std::complex<double>> a(N);
    for (int i = 0; i < N; ++i) a[i] = x[off + i] * (0.42 - 0.5 * std::cos(2 * M_PI * i / (N - 1)) + 0.08 * std::cos(4 * M_PI * i / (N - 1)));
    for (int i = 1, j = 0; i < N; ++i) { int b = N >> 1; for (; j & b; b >>= 1) j ^= b; j ^= b; if (i < j) std::swap(a[i], a[j]); }
    for (int len = 2; len <= N; len <<= 1) { const double ang = -2 * M_PI / len; std::complex<double> wl(std::cos(ang), std::sin(ang)); for (int i = 0; i < N; i += len) { std::complex<double> w(1); for (int k = 0; k < len / 2; ++k) { auto u = a[i + k], v = a[i + k + len / 2] * w; a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl; } } }
    double tot = 0, inh = 0;
    for (int k = 2; k < N / 2; ++k) {
        const double f = k * 48000.0 / N; if (f > 18000) break;
        const double p = std::norm(a[k]); tot += p;
        const double h = f / f0; const double d = std::fabs(h - std::round(h)) * f0;
        if (d > 25.0) inh += p;
    }
    return 10 * std::log10(inh / tot + 1e-30);
}

int main() {
    // ---- classic envelope: 3.0 attack is a one-pole toward 1.25; reaches 1 at ln(5)≈1.61 x the knob
    for (int isa = 0; isa < 2; ++isa) {
        float t[2];
        for (int cl = 0; cl < 2; ++cl) {
            drift::IEngine* e = fresh(isa == 0);
            e->setParam(pAEnvA, 0.5f); e->setParam(pAEnvS, 1.f);
            e->setEnvClassic(cl == 1);
            drift::Telemetry tel{}; e->setTelemetry(&tel);
            note(e, 57, true);
            t[cl] = -1.f;
            std::vector<float> L(64), R(64);
            for (int pos = 0; pos < 96000; pos += 64) {
                e->process(L.data(), R.data(), 64);
                if (tel.newest >= 0 && tel.v[tel.newest].amp >= 0.98f) { t[cl] = (pos + 64) / 48000.f; break; }
            }
            delete e;
        }
        CHECK(std::fabs(t[0] - 0.5f) < 0.06f, "%s timed attack 0.5 s reaches 98%% at %.3f s", isa ? "avx2" : "generic", t[0]);
        CHECK(t[1] > 0.6f && t[1] < 0.9f, "%s classic attack 0.5 s reaches 98%% at %.3f s (3.0 behaviour)", isa ? "avx2" : "generic", t[1]);
    }

    // ---- guard cap: 10 voices, cap 4 -> the extra voices fade, no click, no NaN
    {
        drift::IEngine* e = fresh(false);
        sp(e, "polyphony", 16); e->setParam(pAEnvA, 0.01f); e->setParam(pAEnvR, 0.5f);
        for (int i = 0; i < 10; ++i) note(e, 48 + i * 2, true);
        render(e, 9600);
        CHECK(e->activeVoices() == 10, "10 voices before the cap (%d)", e->activeVoices());
        e->setGuardCap(4);
        std::vector<float> x = render(e, 4800);
        float jump = 0; for (size_t i = 1; i < x.size(); ++i) jump = std::max(jump, std::fabs(x[i] - x[i - 1]));
        CHECK(e->activeVoices() <= 4, "cap 4 leaves %d voices", e->activeVoices());
        CHECK(jump < 0.08f, "fade-out max sample step %.4f", jump);
        note(e, 90, true); render(e, 2400);
        CHECK(e->activeVoices() <= 4, "new note under cap keeps %d voices", e->activeVoices());
        e->setGuardCap(24);
        for (int i = 0; i < 6; ++i) note(e, 70 + i, true);
        render(e, 2400);
        CHECK(e->activeVoices() > 4, "cap lifted -> %d voices", e->activeVoices());
        delete e;
    }

    // ---- drive oversampling: heavy drive on a bright tone, Normal (2x shaper) vs HQ (2x voices)
    {
        double db[3];
        for (int q = 0; q < 3; ++q) {
            drift::IEngine* e = fresh(false);
            e->setParam(oscP(0, ofType), (float)otAnalog); e->setParam(oscP(0, ofAnWave), 4.f);
            e->setParam(oscP(0, ofVoices), 1.f); e->setParam(oscP(0, ofAnSlop), 0.f); e->setParam(oscP(0, ofAnDrift), 0.f);
            e->setParam(oscP(1, ofOn), 0.f); e->setParam(oscP(2, ofOn), 0.f); sp(e, "chOn", 0); sp(e, "f2Type", 0);
            e->setParam(pAEnvA, 0.001f); e->setParam(pAEnvS, 1.f);
            sp(e, "quality", (float)q);
            sp(e, "f1Type", 0); sp(e, "f1Cutoff", 18000); sp(e, "f1Drive", 1.f); sp(e, "f1Res", 0.f);
            sp(e, "polyphony", 1);
            note(e, 92, true);    // ~1661 Hz
            std::vector<float> x = render(e, 48000);
            db[q] = inharmonicDb(x, 440.0 * std::pow(2.0, (92 - 69) / 12.0));
            delete e;
        }
        std::printf("      inharmonic energy  eco %.1f dB  normal %.1f dB  hq %.1f dB\n", db[0], db[1], db[2]);
        CHECK(db[1] < db[0] - 3.0, "Normal drive aliasing below Eco");
    }

    // ---- MIDI map strings
    {
        int m[128]; for (int& v : m) v = -1;
        m[21] = (int)pF1Cutoff; m[22] = (int)pAEnvCurve; m[74] = (int)pF1Res;   // 74 is not learnable
        const std::string s = drift::midiMapToString(m);
        int r[128]; drift::midiMapFromString(s.c_str(), r);
        CHECK(r[21] == (int)pF1Cutoff && r[22] == (int)pAEnvCurve && r[74] == -1, "map round trip \"%s\"", s.c_str());
        drift::midiMapFromString("21=noSuchParam;x;23=f1Cutoff", r);
        CHECK(r[21] == -1 && r[23] == (int)pF1Cutoff, "map parser ignores junk");
    }
    std::printf(bad ? "\n%d FAILURES\n" : "\nall good\n", bad);
    return bad ? 1 : 0;
}
