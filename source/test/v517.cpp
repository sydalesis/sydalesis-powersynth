// Sydalesis 5.1.7: Timbre 5..10 modulation destinations.
// For every engine target: knob at base + delta (no modulation) must sound like knob at base + macro -> Timbre N
// with the matching amount, and both must differ from the unmodulated sound.
// build (from source/test): g++ -std=gnu++17 -O2 -ffast-math -fno-finite-math-only -I.. v517.cpp ../EngineGeneric.cpp ../EngineFactory.cpp ../EngineAVX2.cpp ../Platform.cpp -o v517 -ldl -lpthread
#include "../Params.hpp"
#include "../Shared.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>
static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)
static const int SR = 48000;
static int sym(const char* s) { int i = ParamRegistry::get().findSymbol(s); if (i < 0) { std::printf("no symbol %s\n", s); ++fails; return 0; } return i; }

struct Extra { const char* s; float v; };
static drift::IEngine* fresh(int type, const std::vector<Extra>& ex) {
    drift::IEngine* e = drift::createEngine(true); e->setSampleRate((float)SR);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(sym("quality"), 1.f);
    e->setParam(sym("obOn"), 0.f); e->setParam(sym("ocOn"), 0.f); e->setParam(sym("oaOn"), 1.f);
    e->setParam(sym("oaType"), (float)type); e->setParam(sym("oaVoices"), 1.f);
    e->setParam(sym("f1Type"), 0.f); e->setParam(sym("f2Type"), 0.f); e->setParam(sym("subLevel"), 0.f); e->setParam(sym("noiseLevel"), 0.f);
    e->setParam(sym("aeA"), 0.001f); e->setParam(sym("aeS"), 1.f);
    for (const Extra& x : ex) e->setParam(sym(x.s), x.v);
    return e;
}
static std::vector<float> render(drift::IEngine* e) {
    const uint8_t on[3] = { 0x90, 57, 100 }; e->midi(on, 3);
    const int n = SR;
    std::vector<float> L(n), R(n);
    for (int p = 0; p < n; p += 256) e->process(&L[p], &R[p], std::min(256, n - p));
    std::vector<float> out; for (int i = 0; i < n; ++i) { out.push_back(L[i]); out.push_back(R[i]); }
    return out;
}
static double rms(const std::vector<float>& x) { double s = 0; for (int i = SR / 2; i < (int)x.size(); ++i) s += (double)x[i] * x[i]; return std::sqrt(s / (x.size() - SR / 2)); }
static double diff(const std::vector<float>& a, const std::vector<float>& b) {   // from 0.25 s on (skip the attack)
    double s = 0; for (int i = SR / 2; i < (int)a.size(); ++i) { const double d = (double)a[i] - b[i]; s += d * d; } return std::sqrt(s / (a.size() - SR / 2));
}

struct Case { int type; int k; const char* knob; float base, delta, scale; std::vector<Extra> extra; };

int main() {
    if (ParamRegistry::get().count() != kParamCount) { std::printf("REGISTRY MISMATCH\n"); return 1; }
    CHECK(mdCT10 == mdCFine + 18, "18 Timbre 5-10 destinations follow C Fine (old preset ids unchanged)");
    CHECK(mdAT5 == mdCFine + 1 && modDstXBase(2) + 5 == mdCT10, "Timbre 5..10 block layout");
    CHECK(std::strcmp(kModDstNames[mdCT10], "C Timbre 10") == 0, "names table length matches");

    const std::vector<Case> cases = {
        { otAnalog,    0, "oaAnDrift",     0.1f,  0.6f,   1.f,  { { "oaAnWave", 1.f } } },
        { otAdditive,  0, "oaAdStretch",   0.f,   0.7f,   1.f,  {} },
        { otAdditive,  1, "oaAdOddEven",   0.f,   0.8f,   1.f,  {} },
        { otAdditive,  2, "oaAdPeakAmt",   0.f,   0.7f,   1.f,  {} },
        { otAdditive,  3, "oaAdCombShift", 0.1f,  0.4f,   1.f,  { { "oaAdComb", 0.8f } } },
        { otAdditive,  4, "oaAdShimmer",   0.f,   0.8f,   1.f,  {} },
        { otAdditive,  5, "oaAdDetune",    3.f,   20.f,  30.f,  {} },
        { otPhysical,  0, "oaPmStretch",   0.f,   0.8f,   1.f,  { { "oaPmModel", 1.f } } },
        { otPhysical,  1, "oaPmSpread",    0.1f,  0.8f,   1.f,  { { "oaPmModel", 1.f } } },
        { otPhysical,  2, "oaPmCouple",    0.f,   0.8f,   1.f,  { { "oaPmModel", 1.f } } },
        { otPhysical,  3, "oaPmBuzz",      0.f,   0.9f,   1.f,  { { "oaPmModel", (float)pmTanpura } } },
        { otPhysical,  4, "oaPmNoise",     0.f,   0.8f,   1.f,  { { "oaPmModel", 1.f } } },
        { otPhysical,  5, "oaPmNonlin",    0.f,   0.9f,   1.f,  { { "oaPmModel", (float)pmWaveguide } } },
        { otGranular,  0, "oaGrScan",      0.f,   0.9f,   1.f,  {} },
        { otGranular,  1, "oaGrPitchRand", 0.f, 600.f, 1200.f,  {} },
        { otGranular,  2, "oaGrSpread",    0.f,   0.9f,   1.f,  {} },
        { otGranular,  3, "oaGrReverse",   0.f,   1.f,    1.f,  { { "oaGrSize", 0.4f }, { "oaGrPitchRand", 300.f } } },
    };
    for (const Case& c : cases) {
        auto withKnob = [&](float v, bool mod) {
            auto* e = fresh(c.type, c.extra);
            e->setParam(sym(c.knob), v);
            if (mod) {
                e->setParam(sym("macro1"), 1.f);
                e->setParam(modSrcParam(0), (float)msMacro1); e->setParam(modDstParam(0), (float)(mdAT5 + c.k));
                e->setParam(modAmtParam(0), c.delta / c.scale);
            }
            auto out = render(e); delete e; return out;
        };
        const auto plain = withKnob(c.base, false), moved = withKnob(c.base + c.delta, false), modded = withKnob(c.base, true);
        const double level = rms(moved), changed = diff(plain, moved), err = diff(moved, modded);
        const bool sampleOnly = c.type == otGranular && c.k == 3;   // Reverse acts on sample sources only (none loaded here)
        CHECK(level > 1e-4 && (sampleOnly ? err < 1e-6 : (changed > 0.05 * level && err < 0.1 * changed)),
              "%-9s T%d %-14s  knob move changes %.4f (rms %.4f); modulated vs knob %.5f",
              kOscTypeNames[c.type], 5 + c.k, kTimbreXNames[c.type][c.k], changed, level, err);
    }
    // destinations without a target on an engine are silent no-ops
    {
        auto run = [&](bool mod) {
            auto* e = fresh(otWavetable, {});
            if (mod) { e->setParam(sym("macro1"), 1.f); e->setParam(modSrcParam(0), (float)msMacro1); e->setParam(modDstParam(0), (float)mdAT5); e->setParam(modAmtParam(0), 1.f); }
            auto out = render(e); delete e; return out;
        };
        const auto a = run(false), b = run(true);
        CHECK(diff(a, b) < 1e-6, "Wavetable: Timbre 5 has no target and changes nothing (%.2g)", diff(a, b));
    }
    // UI mapping: right-drag on the PHYS Stretch knob finds A Timbre 5
    {
        const int types[3] = { otPhysical, otAnalog, otWavetable };
        const drift::DestMap m = drift::destMap(mdAT5, types);
        CHECK(m.param == (int)oscP(0, ofPmStretch) && m.scale == 1.f, "destMap(A Timbre 5, Physical) = Stretch knob");
        const drift::DestMap w = drift::destMap(mdCT5, types);
        CHECK(w.param < 0, "destMap(C Timbre 5, Wavetable) = none");
        const int t2[3] = { otSpectral, otGranular, otAdditive };
        CHECK(drift::destMap(mdAT6, t2).param == (int)osc2P(0, o2SpStretch), "destMap(A Timbre 6, Spectral) = Spectral Stretch");
        CHECK(drift::destMap(mdCT5, t2).param == (int)oscP(2, ofAdStretch), "destMap(C Timbre 5, Additive) = Stretch");
    }
    std::printf(fails ? "%d FAILED\n" : "all good\n", fails);
    return fails ? 1 : 0;
}
