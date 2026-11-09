// Demo reel: selected v3 presets at their stored levels, cross-faded, no added effects.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../Library.hpp"
#include "../Spectra.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Presets.hpp"
#include <cstdio>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <algorithm>
struct Ev { double t; uint8_t b0, b1, b2; };
int main(int argc, char** argv) {
    const char* names[] = { "Silica Breath", "Overtone Ladder", "Tanpura Field", "Coupled Bowls", "Void Engine", "Basalt Choir",
                            "Rain Temple", "Aurora Granules", "Shifted Monastery", "Eight Voice Strings", "Super Horizon", "Ancestor Hum",
                            "Vowel Choir AEIOU", "Berlin Ostinato", "Gong Bath", "Light Sculpture" };
    const int sr = 48000, seg = 13 * sr, tail = 3 * sr;
    drift::IEngine* e = drift::createEngine(); e->setSampleRate((float)sr);
    std::vector<float> out;
    float pv[kParamCount];
    std::vector<std::string> list(names, names + sizeof(names) / sizeof(names[0]));
    if (argc > 2) list.assign(argv + 2, argv + argc);
    for (const std::string& nms : list) {
        const char* nm = nms.c_str();
        int pi = -1; for (uint32_t i = 0; i < kFactoryPresetCount; ++i) if (!std::strcmp(kFactoryPresets[i].name, nm)) pi = (int)i;
        if (pi < 0) { printf("missing %s\n", nm); continue; }
        presetValues(pi, pv); e->reset(); for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, pv[i]);
        std::string st[kStateCount]; presetStates(pi, st);
        for (int o = 0; o < 3; ++o) {
            e->postTable(o, st[skWtA + o].empty() ? nullptr : drift::loadWavetableFile(drift::lib::resolve(st[skWtA + o]).c_str(), false));
            e->postSample(o, st[skSmA + o].empty() ? nullptr : drift::loadSampleFile(drift::lib::resolve(st[skSmA + o]).c_str()));
            float a[128]; if (!drift::spectrumFromString(st[skSpA + o].c_str(), a)) drift::spectrumDefault(a); e->setCustomSpectrum(o, a);
        }
        for (int l = 0; l < kLfos; ++l) { float p[64]; if (!drift::lfoShapeFromString(st[skLfo1 + l].c_str(), p)) drift::lfoShapeDefault(p, l); e->setLfoShape(l, p); }
        for (int m = 0; m < kMsegs; ++m) { drift::MsegData d; if (!drift::msegFromString(st[skMseg1 + m].c_str(), d)) drift::msegDefault(d, m); e->setMseg(m, d); }
        { int ord[kFxUnits]; if (!drift::fxOrderFromString(st[skFxOrder].c_str(), ord)) drift::fxOrderDefault(ord); e->setFxOrder(ord); }
        const std::string cat = kFactoryPresets[pi].category, tags = kFactoryPresets[pi].tags;
        std::vector<Ev> ev;
        auto chord = [&](std::vector<int> ns, double on, double off) { for (int n : ns) { ev.push_back({on, 0x90, (uint8_t)n, 95}); ev.push_back({off, 0x80, (uint8_t)n, 0}); } };
        if (tags.find("hold-a-note") != std::string::npos) chord({45}, 0, 11.5);
        else if (tags.find("hold-a-chord") != std::string::npos) chord({43, 50, 55}, 0, 11.5);
        else if (cat == "Drones") chord({38, 45, 50}, 0, 10);
        else { chord({50, 57, 62, 65}, 0, 5.5); chord({48, 55, 60, 64}, 5.5, 10.5); }
        std::sort(ev.begin(), ev.end(), [](const Ev& a, const Ev& b) { return a.t < b.t; });
        const int n = seg + tail;
        std::vector<float> L(n), R(n); size_t ei = 0; int pos = 0; double beat = 0;
        while (pos < n) {
            while (ei < ev.size() && (int)(ev[ei].t * sr) <= pos) { uint8_t m[3] = {ev[ei].b0, ev[ei].b1, ev[ei].b2}; e->midi(m, 3); ++ei; }
            int end = std::min(pos + 256, n); if (ei < ev.size()) end = std::min(end, std::max(pos + 1, (int)(ev[ei].t * sr)));
            e->setTransport(112, true, beat);
            e->process(&L[pos], &R[pos], end - pos); beat += (end - pos) / (double)sr * 112.0 / 60.0; pos = end;
        }
        uint8_t off[3] = {0xB0, 120, 0}; e->midi(off, 3);
        const size_t start = out.size() >= (size_t)tail * 2 ? out.size() - tail * 2 : 0;
        out.resize(std::max(out.size(), start + (size_t)n * 2), 0.f);
        for (int i = 0; i < n; ++i) { float g = i >= n - sr ? (float)(n - i) / sr : 1.f; if (i < sr / 4) g *= (float)i / (sr / 4); out[start + 2 * i] += L[i] * g; out[start + 2 * i + 1] += R[i] * g; }
        printf("%s\n", nm);
    }
    FILE* f = fopen(argv[1], "wb"); fwrite(out.data(), 4, out.size(), f); fclose(f);
    delete e;
}
