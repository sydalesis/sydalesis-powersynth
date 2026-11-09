// Renders every factory preset with a category-appropriate phrase; reports level, peak and
// suggested master gain. Writes ../tools/preset_levels.json and an optional raw render.
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
#include <algorithm>

struct Ev { double t; uint8_t b0, b1, b2; };
static std::vector<Ev> phrase(const std::string& cat, double& dur, double& m0, double& m1) {
    std::vector<Ev> e;
    auto chord = [&](std::vector<int> ns, double on, double off) { for (int n : ns) { e.push_back({on, 0x90, (uint8_t)n, 90}); e.push_back({off, 0x80, (uint8_t)n, 0}); } };
    if (cat == "Drones") { chord({38, 45}, 0.0, 8.0); dur = 12; m0 = 2; m1 = 8; }
    else if (cat == "Basses") { int ns[8] = {36, 36, 43, 41, 36, 39, 43, 34}; for (int i = 0; i < 8; ++i) chord({ns[i]}, i * 0.5, i * 0.5 + 0.4); dur = 5; m0 = 0; m1 = 4; }
    else if (cat == "Leads") { int ns[8] = {60, 62, 63, 67, 65, 63, 62, 60}; for (int i = 0; i < 8; ++i) chord({ns[i]}, i * 0.6, i * 0.6 + 0.55); dur = 6; m0 = 0.3; m1 = 4.8; }
    else if (cat == "Bells & Resonators" || cat == "Keys & Organs" || cat == "Sequence & Plucks") {
        int ns[8] = {57, 60, 64, 67, 69, 64, 60, 72}; for (int i = 0; i < 8; ++i) chord({ns[i]}, i * 0.5, i * 0.5 + 0.45); dur = 7; m0 = 0; m1 = 4.5; }
    else { chord({50, 57, 62, 65, 69}, 0.0, 7.0); dur = 10; m0 = 2.5; m1 = 7; }
    std::sort(e.begin(), e.end(), [](const Ev& a, const Ev& b) { return a.t < b.t; });
    return e;
}

int main(int argc, char** argv) {
    const bool writeRaw = argc > 1;
    std::vector<float> raw;
    const int sr = 48000;
    drift::IEngine* e = drift::createEngine();
    e->setSampleRate((float)sr);
    FILE* js = fopen("../tools/preset_levels.json", "w");
    fprintf(js, "{\n");
    int problems = 0;
    float pv[kParamCount];
    for (uint32_t pi = 0; pi < kFactoryPresetCount; ++pi) {
        const FactoryPreset& fp = kFactoryPresets[pi];
        presetValues(pi, pv);
        const float masterSet = pv[pMaster];
        pv[pMaster] = -6.f;   // measure at a reference gain
        e->reset();
        for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, pv[i]);
        std::string st[6]; presetStates(pi, st);
        bool missing = false;
        for (int o = 0; o < 3; ++o) {
            if (!st[o].empty()) {
                drift::WaveTable* t = drift::loadWavetableFile(drift::lib::resolve(st[o]).c_str(), false);
                if (!t) missing = true;
                e->postTable(o, t);
            } else e->postTable(o, nullptr);
            float amps[64];
            if (!st[3 + o].empty() && drift::spectrumFromString(st[3 + o].c_str(), amps)) e->setCustomSpectrum(o, amps);
            else { for (int i = 0; i < 64; ++i) amps[i] = 1.f / (i + 1); e->setCustomSpectrum(o, amps); }
        }
        double dur, m0, m1;
        auto ev = phrase(fp.category, dur, m0, m1);
        const int n = (int)(dur * sr);
        std::vector<float> L(n), R(n);
        size_t ei = 0; int pos = 0;
        while (pos < n) {
            while (ei < ev.size() && (int)(ev[ei].t * sr) <= pos) { uint8_t m[3] = {ev[ei].b0, ev[ei].b1, ev[ei].b2}; e->midi(m, 3); ++ei; }
            int end = std::min(pos + 256, n);
            if (ei < ev.size()) end = std::min(end, std::max(pos + 1, (int)(ev[ei].t * sr)));
            e->process(&L[pos], &R[pos], end - pos);
            pos = end;
        }
        uint8_t off[3] = {0xB0, 120, 0}; e->midi(off, 3);
        float pk = 0; double rms = 0; int nan = 0; int cnt = 0;
        for (int i = 0; i < n; ++i) {
            if (!std::isfinite(L[i]) || !std::isfinite(R[i])) { ++nan; continue; }
            pk = std::max(pk, std::max(std::fabs(L[i]), std::fabs(R[i])));
            if (i >= m0 * sr && i < m1 * sr) { rms += 0.5 * (L[i] * L[i] + R[i] * R[i]); ++cnt; }
        }
        const double rmsDb = 10 * std::log10(rms / std::max(1, cnt) + 1e-12);
        const bool plucky = std::string(fp.category) == "Bells & Resonators" || std::string(fp.category) == "Sequence & Plucks" || std::string(fp.category) == "Keys & Organs";
        const double target = plucky ? -22.0 : (std::string(fp.category) == "Basses" || std::string(fp.category) == "Leads" ? -19.0 : -20.0);
        double adj = target - rmsDb;
        // keep peaks under -3 dBFS after adjustment
        const double pkDb = 20 * std::log10(pk + 1e-9);
        if (pkDb + adj > -3.0) adj = -3.0 - pkDb;
        adj = std::max(-24.0, std::min(18.0, adj));
        const double master = std::round((-6.0 + adj) * 2.0) / 2.0;
        const bool bad = nan || rmsDb < -60 || missing;
        if (bad) ++problems;
        printf("%-3u %-24s %-20s rms %6.1f dB  peak %6.1f dB  -> master %+5.1f dB%s%s%s\n", pi, fp.name, fp.category, rmsDb, pkDb, master,
               nan ? "  NaN!" : "", rmsDb < -60 ? "  SILENT!" : "", missing ? "  MISSING TABLE!" : "");
        fprintf(js, "  \"%s\": %.1f%s\n", fp.name, std::max(-36.0, std::min(6.0, master)), pi + 1 < kFactoryPresetCount ? "," : "");
        (void)masterSet;
        if (writeRaw) { const int keep = std::min(n, 8 * sr); for (int i = 0; i < keep; ++i) { raw.push_back(L[i] * std::pow(10.f, (float)(adj / 20.0))); raw.push_back(R[i] * std::pow(10.f, (float)(adj / 20.0))); } }
    }
    fprintf(js, "}\n"); fclose(js);
    printf("%d problems\n", problems);
    if (writeRaw) { FILE* f = fopen(argv[1], "wb"); fwrite(raw.data(), 4, raw.size(), f); fclose(f); }
    delete e;
}
