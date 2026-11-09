// Driftfield 4.1 checks: layer gains / note acceptance per mode, WAV analysis to partials and wavetable.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Analysis.hpp"
#include <cstdio>
#include <cmath>
#include <vector>

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { ++bad; std::printf("FAIL: "); } else std::printf("ok:   "); std::printf(__VA_ARGS__); std::printf("\n"); } while (0)
static void note(drift::IEngine* e, int n, int vel, bool on) { uint8_t m[3] = { (uint8_t)(on ? 0x90 : 0x80), (uint8_t)n, (uint8_t)vel }; e->midi(m, 3); }
static drift::IEngine* fresh(int layer) {
    drift::IEngine* e = drift::createEngine(true);
    e->setSampleRate(48000); e->setTransport(120, false, 0); e->setLayer(layer);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(pAEnvA, 0.001f); e->setParam(pAEnvS, 1.f); e->setParam(pAEnvR, 0.02f);
    return e;
}
static double rms(drift::IEngine* e, int frames) {
    std::vector<float> L(256), R(256); double acc = 0; int n = 0;
    for (int pos = 0; pos < frames; pos += 256) { e->process(L.data(), R.data(), 256); if (pos > frames / 2) for (float v : L) { acc += v * v; ++n; } }
    return std::sqrt(acc / std::max(1, n));
}
static double playRms(int layer, int mode, int key, int vel, float morph = 0.5f, float split = 60, float width = 0) {
    drift::IEngine* e = fresh(layer);
    e->setParam(pLyMode, (float)mode); e->setParam(pLySplit, split); e->setParam(pLyWidth, width); e->setParam(pLyMorph, morph);
    note(e, key, vel, true);
    const double r = rms(e, 24000);
    delete e;
    return r;
}

int main() {
    std::printf("params: %u per layer, %u total (layer 2 at %u)\n", (unsigned)kLayerParams, (unsigned)kParamCount, (unsigned)kLayerBOffset);
    CHECK(ParamRegistry::get().count() == kParamCount, "registry holds %u params", ParamRegistry::get().count());
    CHECK(std::string(P(kLayerBOffset + pF1Cutoff).symbol) == "b_f1Cutoff" && P(kLayerBOffset + pF1Cutoff).def == P(pF1Cutoff).def, "layer 2 copy of f1Cutoff");
    CHECK(std::string(kStateKeys[kLayerStates + skWtA]) == "b_wtA", "layer 2 state keys");

    const double a = playRms(0, 0, 60, 100), b = playRms(1, 0, 60, 100);
    CHECK(a > 1e-3 && b == 0.0, "mode Off: layer 1 plays (%.4f), layer 2 silent", a);
    const double la = playRms(0, 1, 60, 100), lb = playRms(1, 1, 60, 100);
    CHECK(std::fabs(la - a) < 1e-6 && std::fabs(lb - a) < 1e-6, "mode Layer: both play at full level");
    CHECK(playRms(0, 2, 48, 100) > 1e-3 && playRms(1, 2, 48, 100) == 0.0, "Split: key 48 -> layer 1 only");
    CHECK(playRms(0, 2, 72, 100) == 0.0 && playRms(1, 2, 72, 100) > 1e-3, "Split: key 72 -> layer 2 only");
    {
        const double x = playRms(0, 2, 60, 100, 0.5f, 60, 12), y = playRms(1, 2, 60, 100, 0.5f, 60, 12);
        CHECK(x > 0.3 * a && y > 0.3 * a && std::fabs(x * x + y * y - a * a) < 0.1 * a * a, "Split with 12 st crossfade: both at the split, equal power (%.3f / %.3f of %.3f)", x, y, a);
    }
    CHECK(playRms(1, 3, 60, 20) == 0.0 && playRms(1, 3, 60, 127) > 1e-3 && playRms(0, 3, 60, 127) == 0.0, "Velocity XF: soft -> layer 1, hard -> layer 2");
    {
        const double m0a = playRms(0, 5, 60, 100, 0.f), m0b = playRms(1, 5, 60, 100, 0.f), m1b = playRms(1, 5, 60, 100, 1.f);
        CHECK(m0a > 0.99 * a && m0b < 1e-4 && m1b > 0.99 * a, "Morph 0 = layer 1, morph 1 = layer 2");
    }

    // analysis: a 196 Hz band-limited saw with slow vibrato
    {
        const int sr = 48000, n = sr * 2;
        std::vector<float> x((size_t)n);
        double ph = 0;
        for (int i = 0; i < n; ++i) {
            const double f = 196.0 * (1.0 + 0.003 * std::sin(6.283 * 5 * i / sr));
            ph += f / sr;
            double v = 0; for (int h = 1; h <= 60; ++h) v += std::sin(6.283185307 * h * ph) / h;
            x[(size_t)i] = (float)(0.3 * v);
        }
        const std::string wav = "/tmp/claude-0/v41_saw.wav", wt = "/tmp/claude-0/v41_saw_table.wav";
        drift::ana::writeWav(wav, x, 1, sr);
        float amps[128]; drift::ana::Info info;
        const bool ok = drift::ana::toSpectrum(wav.c_str(), amps, info);
        CHECK(ok && std::fabs(info.f0 - 196.f) < 1.5f, "pitch detection %.2f Hz (%s)", info.f0, info.err.c_str());
        CHECK(ok && std::fabs(amps[1] - 0.5f) < 0.06f && std::fabs(amps[4] - 0.2f) < 0.04f && amps[100] < 0.02f, "saw partials 1, %.3f, ... %.3f (expect 0.5, 0.2)", amps[1], amps[4]);
        drift::ana::Info i2;
        const bool ok2 = drift::ana::toWavetable(wav.c_str(), wt, i2);
        std::string err;
        std::unique_ptr<drift::WaveTable> t(ok2 ? drift::loadWavetableFile(wt.c_str(), false, &err) : nullptr);
        CHECK(ok2 && t && t->frames == 64, "wavetable written and loaded: %d frames (%s %s)", t ? t->frames : 0, i2.err.c_str(), err.c_str());
        std::vector<float> noise((size_t)n); uint32_t s = 1; for (float& v : noise) { s = s * 1664525u + 1013904223u; v = ((s >> 9) / 4194304.f - 1.f) * 0.3f; }
        drift::ana::writeWav("/tmp/claude-0/v41_noise.wav", noise, 1, sr);
        drift::ana::Info i3;
        CHECK(!drift::ana::toSpectrum("/tmp/claude-0/v41_noise.wav", amps, i3), "noise is refused: %s", i3.err.c_str());
    }
    std::printf(bad ? "\n%d FAILURES\n" : "\nall good\n", bad);
    return bad ? 1 : 0;
}
