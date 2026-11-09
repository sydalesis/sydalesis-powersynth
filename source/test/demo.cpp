// Renders a demo reel of selected factory presets (stored master gain) to raw stereo float.
#include "../Params.hpp"
#include "../Presets.hpp"
#include "../Wavetables.hpp"
#include "../Library.hpp"
#include "../Spectra.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <algorithm>
struct Ev { double t; uint8_t b0, b1, b2; };
int main(int argc, char** argv) {
    const char* names[] = { "Gravity Well", "Throat of the World", "Magnificent Void", "Red Rock Resonance", "Oberheim Sunset",
                            "Glass Cathedral", "Overtone Singer", "Singing Bowl Choir", "Vespers of Stone", "Velvet PWM", "Bronze Gong", "Berlin Pulse" };
    const int sr = 48000, seg = 13 * sr, tail = 3 * sr;
    drift::IEngine* e = drift::createEngine(); e->setSampleRate((float)sr);
    std::vector<float> out;
    float pv[kParamCount];
    for (const char* nm : names) {
        int pi = -1; for (uint32_t i = 0; i < kFactoryPresetCount; ++i) if (!std::strcmp(kFactoryPresets[i].name, nm)) pi = (int)i;
        if (pi < 0) { printf("missing %s\n", nm); continue; }
        const std::string cat = kFactoryPresets[pi].category;
        presetValues(pi, pv); e->reset(); for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, pv[i]);
        std::string st[6]; presetStates(pi, st);
        for (int o = 0; o < 3; ++o) {
            e->postTable(o, st[o].empty() ? nullptr : drift::loadWavetableFile(drift::lib::resolve(st[o]).c_str(), false));
            float a[64]; if (st[3+o].empty() || !drift::spectrumFromString(st[3+o].c_str(), a)) for (int i = 0; i < 64; ++i) a[i] = 1.f/(i+1);
            e->setCustomSpectrum(o, a);
        }
        std::vector<Ev> ev;
        auto chord = [&](std::vector<int> ns, double on, double off) { for (int n : ns) { ev.push_back({on,0x90,(uint8_t)n,95}); ev.push_back({off,0x80,(uint8_t)n,0}); } };
        if (cat == "Drones") chord({38, 45, 50}, 0, 10);
        else if (cat == "Bells & Resonators") { int ns[10] = {57,64,69,72,67,64,62,69,60,57}; for (int i = 0; i < 10; ++i) chord({ns[i]}, i*1.0, i*1.0+0.8); }
        else if (cat == "Sequence & Plucks") { int ns[16] = {45,57,52,57,48,57,52,55,45,57,52,57,43,55,50,55}; for (int r = 0; r < 3; ++r) for (int i = 0; i < 16; ++i) chord({ns[i]}, (r*16+i)*0.2, (r*16+i)*0.2+0.15); }
        else { chord({50,57,62,65}, 0, 5); chord({48,55,60,64}, 5, 10); }
        std::sort(ev.begin(), ev.end(), [](const Ev& a, const Ev& b){ return a.t < b.t; });
        const int n = seg + tail;
        std::vector<float> L(n), R(n); size_t ei = 0; int pos = 0;
        while (pos < n) {
            while (ei < ev.size() && (int)(ev[ei].t*sr) <= pos) { uint8_t m[3] = {ev[ei].b0, ev[ei].b1, ev[ei].b2}; e->midi(m, 3); ++ei; }
            int end = std::min(pos+256, n); if (ei < ev.size()) end = std::min(end, std::max(pos+1, (int)(ev[ei].t*sr)));
            e->process(&L[pos], &R[pos], end-pos); pos = end;
        }
        uint8_t off[3] = {0xB0,120,0}; e->midi(off,3);
        // overlap-add with crossfade: new segment starts `tail` before end of previous
        const size_t start = out.size() >= (size_t)tail*2 ? out.size() - tail*2 : 0;
        out.resize(std::max(out.size(), start + (size_t)n*2), 0.f);
        for (int i = 0; i < n; ++i) {
            float g = i >= n - sr ? (float)(n - i) / sr : 1.f;
            out[start+2*i] += L[i]*g; out[start+2*i+1] += R[i]*g;
        }
        printf("%s\n", nm);
    }
    FILE* f = fopen(argv[1], "wb"); fwrite(out.data(), 4, out.size(), f); fclose(f);
    delete e;
}
