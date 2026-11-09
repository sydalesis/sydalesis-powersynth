#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../Library.hpp"
#include "../Spectra.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Presets.hpp"
#include <cstdio>
#include <cstring>
#include <vector>
#include <chrono>
int main(int argc, char** argv) {
    auto* e = drift::createEngine(argc > 3); e->setSampleRate(48000);
    int pi = -1; for (uint32_t i = 0; i < kFactoryPresetCount; ++i) if (!strcmp(kFactoryPresets[i].name, argv[1])) pi = i;
    if (pi < 0) { printf("no preset\n"); return 1; }
    float pv[kParamCount]; presetValues(pi, pv); for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, pv[i]);
    std::string st[kStateCount]; presetStates(pi, st);
    for (int o = 0; o < 3; ++o) {
        if (!st[skWtA + o].empty()) e->postTable(o, drift::loadWavetableFile(drift::lib::resolve(st[skWtA + o]).c_str(), false));
        if (!st[skSmA + o].empty()) e->postSample(o, drift::loadSampleFile(drift::lib::resolve(st[skSmA + o]).c_str()));
        float a[128]; if (!drift::spectrumFromString(st[skSpA + o].c_str(), a)) drift::spectrumDefault(a); e->setCustomSpectrum(o, a);
    }
    for (int m = 0; m < kMsegs; ++m) { drift::MsegData d; if (drift::msegFromString(st[skMseg1 + m].c_str(), d)) e->setMseg(m, d); }
    const int voices = atoi(argv[2]);
    for (int v = 0; v < voices; ++v) { uint8_t m[3] = { 0x90, (uint8_t)(40 + v * 3), 100 }; e->midi(m, 3); }
    std::vector<float> L(256), R(256);
    for (int i = 0; i < (getenv("BLOCKS") ? 20 : 200); ++i) e->process(L.data(), R.data(), 256);   // warm-up past attacks
    const int blocks = getenv("BLOCKS") ? atoi(getenv("BLOCKS")) : 48000 * 4 / 256;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < blocks; ++i) e->process(L.data(), R.data(), 256);
    const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    printf("%-22s %2d voices %s: %.1f%% of one core\n", argv[1], voices, e->isaName(), s / (blocks * 256.0 / 48000.0) * 100);
}
