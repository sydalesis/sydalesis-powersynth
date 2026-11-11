#include <cstring>
#include <cstdlib>
// 6.0: presetcheck6 = presetcheck3 for the 6.0 bank only (bank 0); Custom 2 spectra loaded.
// Renders every factory preset with a category-appropriate phrase; reports level, peak, CPU and
// suggested master gain. Writes ../tools/preset_levels3.json; optional raw render (argv[1]).
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../Library.hpp"
#include "../Spectra.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../SpecData.hpp"
#include "../Presets.hpp"
#include "../Analysis.hpp"
#include <fstream>
#include <set>
#include <cstdio>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <algorithm>

struct Ev { double t; uint8_t b0, b1, b2; };
static std::vector<Ev> phrase(const FactoryPreset& fp, double& dur, double& m0, double& m1, double atk) {
    // sustained categories hold long enough to measure after the amp attack has finished
    const double hl = atk + 5.0 > 8.0 ? atk + 5.0 : 8.0, ms = atk + 0.5 > 3.0 ? atk + 0.5 : 3.0;
    std::vector<Ev> e;
    const std::string cat = fp.category, tags = fp.tags;
    auto chord = [&](std::vector<int> ns, double on, double off) { for (int n : ns) { e.push_back({on, 0x90, (uint8_t)n, 90}); e.push_back({off, 0x80, (uint8_t)n, 0}); } };
    const bool hold = tags.find("hold-a") != std::string::npos || tags.find("arp") != std::string::npos || tags.find("sequenced") != std::string::npos;
    if (hold) { if (tags.find("hold-a-note") != std::string::npos) chord({45}, 0.0, 7.0); else chord({45, 52, 57, 60}, 0.0, 7.0); dur = 9; m0 = 1; m1 = 7; }
    else if (cat == "Drones") { chord({38, 45}, 0.0, hl + 1); dur = hl + 4; m0 = ms; m1 = hl + 1; }
    else if (cat == "Basses") { int ns[8] = {36, 36, 43, 41, 36, 39, 43, 34}; for (int i = 0; i < 8; ++i) chord({ns[i]}, i * 0.5, i * 0.5 + 0.4); dur = 5; m0 = 0; m1 = 4; }
    else if (cat == "Leads") { int ns[8] = {60, 62, 63, 67, 65, 63, 62, 60}; for (int i = 0; i < 8; ++i) chord({ns[i]}, i * 0.6, i * 0.6 + 0.55); dur = 6; m0 = 0.3; m1 = 4.8; }
    else if (cat == "FX & Details") { chord({57, 64}, 0.0, hl); dur = hl + 3; m0 = 0.0; m1 = hl; }
    else if (cat == "Bells & Resonators" || cat == "Keys & Organs" || cat == "Sequence & Plucks") {
        int ns[8] = {57, 60, 64, 67, 69, 64, 60, 72}; for (int i = 0; i < 8; ++i) chord({ns[i]}, i * 0.5, i * 0.5 + 0.45); dur = 7; m0 = 0; m1 = 4.5; }
    else { chord({50, 57, 62, 65, 69}, 0.0, hl); dur = hl + 3; m0 = ms; m1 = hl; }
    std::sort(e.begin(), e.end(), [](const Ev& a, const Ev& b) { return a.t < b.t; });
    return e;
}

int main(int argc, char** argv) {
    const bool writeRaw = argc > 1;
    std::vector<float> raw;
    const int sr = 48000;
    drift::IEngine* e = drift::createEngine();
    e->setSampleRate((float)sr);
    drift::IEngine* e2 = drift::createEngine();     // layer 2 of layered presets
    e2->setSampleRate((float)sr); e2->setLayer(1);
    std::vector<float> L2(256), R2(256);
    const char* only = std::getenv("DF_ONLY");
    std::set<std::string> nameSet;
    if (const char* nf = std::getenv("DF_NAMES")) { std::ifstream in(nf); std::string ln; while (std::getline(in, ln)) if (!ln.empty()) nameSet.insert(ln); }
    char tsvPath[256]; std::snprintf(tsvPath, sizeof tsvPath, "%s", std::getenv("DF_TSV") ? std::getenv("DF_TSV") : "/dev/null");
    FILE* tsv = fopen(tsvPath, "w");
    const int part = std::getenv("DF_PART") ? std::atoi(std::getenv("DF_PART")) : -1, parts = std::getenv("DF_PARTS") ? std::max(1, std::atoi(std::getenv("DF_PARTS"))) : 1;
    char jsPath[256]; std::snprintf(jsPath, sizeof jsPath, part >= 0 ? "../tools/preset_levels6.part%d.json" : "../tools/preset_levels6.json", part);
    FILE* js = fopen(only || !nameSet.empty() ? "/dev/null" : jsPath, "w");

    fprintf(js, "{\n");
    int jsCount = 0;
    int problems = 0;
    float pv[kParamCount];
    double cpuMax = 0; std::string cpuMaxName;
    const uint32_t stride = std::getenv("DF_STRIDE") ? (uint32_t)std::max(1, std::atoi(std::getenv("DF_STRIDE"))) : 1u;
    for (uint32_t pi = 0; pi < kFactoryPresetCount; pi += stride) {
        const FactoryPreset& fp = kFactoryPresets[pi];
        if (only && !std::strstr(fp.name, only)) continue;
        if (!nameSet.empty() && !nameSet.count(fp.name)) continue;
        if (part >= 0 && (int)(pi % (uint32_t)parts) != part) continue;
        if (kFactoryPresets[pi].bank != 0) continue;
        // fresh engines per preset: no tails / voice / random state carried over from the previous one
        delete e; delete e2;
        e = drift::createEngine(); e->setSampleRate((float)sr);
        e2 = drift::createEngine(); e2->setSampleRate((float)sr); e2->setLayer(1);
        presetValues(pi, pv);
        if (const char* q = std::getenv("DF_QUALITY")) pv[pQuality] = (float)std::atoi(q);   // fast passes at Normal
        const bool layered = pv[pLyMode] >= 0.5f;
        pv[kLayerBOffset + pMaster] = -6.f + (pv[kLayerBOffset + pMaster] - pv[pMaster]);   // keep layer 2's balance
        pv[pMaster] = -6.f;
        std::string st[kStateCount]; presetStates(pi, st);
        bool missing = false;
        auto setup = [&](drift::IEngine* en, int layer) {
            const uint32_t po = layer ? kLayerBOffset : 0; const int so = layer ? kLayerStates : 0;
            en->reset();
            for (uint32_t i = 0; i < kEngineParams; ++i) en->setParam(i, pv[i < kLayerParams ? i + po : i]);
            for (int o = 0; o < 3; ++o) {
                if (!st[so + skWtA + o].empty()) { drift::WaveTable* t = drift::loadWavetableFile(drift::lib::resolve(st[so + skWtA + o]).c_str(), false); if (!t) missing = true; en->postTable(o, t); }
                else en->postTable(o, nullptr);
                if (!st[so + skSmA + o].empty()) { drift::SampleBuf* sb = drift::loadSampleFile(drift::lib::resolve(st[so + skSmA + o]).c_str()); if (!sb) missing = true; en->postSample(o, sb); }
                else en->postSample(o, nullptr);
                float amps[128]; if (!drift::spectrumFromString(st[so + skSpA + o].c_str(), amps)) drift::spectrumDefault(amps); en->setCustomSpectrum(o, amps);
                { float a2[128]; if (!drift::spectrumFromString(st[so + skSp2A + o].c_str(), a2)) drift::spectrumDefault(a2); en->setCustomSpectrum(3 + o, a2); }
                for (int w = 0; w < 2; ++w) {
                    const std::string& sv = st[so + (w ? skXsrcA : skSrcA) + o];
                    if (sv.empty()) { en->postSpectral(o, w, nullptr); continue; }
                    drift::SpecData* sd = drift::loadSpecFile(drift::lib::resolve(sv).c_str());
                    if (!sd) missing = true;
                    en->postSpectral(o, w, sd);
                }
            }
            for (int l = 0; l < kLfos; ++l) { float pts[64]; if (!drift::lfoShapeFromString(st[so + skLfo1 + l].c_str(), pts)) drift::lfoShapeDefault(pts, l); en->setLfoShape(l, pts); }
            for (int m = 0; m < kMsegs; ++m) { const int k = m < 2 ? skMseg1 + m : skMseg3 + m - 2; drift::MsegData d; if (!drift::msegFromString(st[so + k].c_str(), d)) drift::msegDefault(d, m); en->setMseg(m, d); }
            for (int o = 0; o < 3; ++o) { drift::ModalData md; if (!drift::modalFromString(st[so + skModesA + o].c_str(), md)) drift::modalDefault(md); en->setModes(o, md); }
            { drift::SceneData sd; drift::scenesFromString(st[so + skScenes].c_str(), sd); en->setScenes(sd); }
            { static drift::MotionData* mo = new drift::MotionData(); drift::motionFromString(st[so + skMotion].c_str(), *mo); en->setMotion(*mo); }
            { int ord[kFxUnits]; if (!drift::fxOrderFromString(st[so + skFxOrder].c_str(), ord)) drift::fxOrderDefault(ord); en->setFxOrder(ord); }
            drift::ScaleDef sc; drift::KbmDef km;
            const std::string scl = drift::tnUnescape(st[so + skScl]), kbm = drift::tnUnescape(st[so + skKbm]);
            if (!scl.empty() && drift::parseScl(scl.c_str(), sc)) en->setUserScale(&sc, (!kbm.empty() && drift::parseKbm(kbm.c_str(), km)) ? &km : nullptr);
            else { if (!scl.empty()) { std::printf("BAD SCALA in %s\n", fp.name); ++problems; } en->setUserScale(nullptr, nullptr); }
        };
        setup(e, 0);
        if (layered) setup(e2, 1);
        double dur, m0, m1;
        auto ev = phrase(fp, dur, m0, m1, pv[pAEnvA] * 1.05);
        const int n = (int)(dur * sr);
        std::vector<float> L(n), R(n);
        size_t ei = 0; int pos = 0;
        double beat = 0;
        const auto t0 = std::chrono::steady_clock::now();
        while (pos < n) {
            while (ei < ev.size() && (int)(ev[ei].t * sr) <= pos) { uint8_t m[3] = {ev[ei].b0, ev[ei].b1, ev[ei].b2}; e->midi(m, 3); if (layered) e2->midi(m, 3); ++ei; }
            int end = std::min(pos + 256, n);
            if (ei < ev.size()) end = std::min(end, std::max(pos + 1, (int)(ev[ei].t * sr)));
            e->setTransport(120.0, false, beat);
            e->process(&L[pos], &R[pos], end - pos);
            if (layered) { e2->setTransport(120.0, false, beat); e2->process(L2.data(), R2.data(), end - pos); for (int i = 0; i < end - pos; ++i) { L[pos + i] += L2[i]; R[pos + i] += R2[i]; } }
            beat += (end - pos) / (double)sr * 2.0;
            pos = end;
        }
        const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        const double cpu = secs / dur;
        if (cpu > cpuMax) { cpuMax = cpu; cpuMaxName = fp.name; }
        uint8_t off[3] = {0xB0, 120, 0}; e->midi(off, 3); e2->midi(off, 3);
        float pk = 0; double rms = 0; int nan = 0; int cnt = 0;
        for (int i = 0; i < n; ++i) {
            if (!std::isfinite(L[i]) || !std::isfinite(R[i])) { ++nan; continue; }
            pk = std::max(pk, std::max(std::fabs(L[i]), std::fabs(R[i])));
            if (i >= m0 * sr && i < m1 * sr) { rms += 0.5 * (L[i] * L[i] + R[i] * R[i]); ++cnt; }
        }
        const double rmsDb = 10 * std::log10(rms / std::max(1, cnt) + 1e-12);
        const std::string cat = fp.category;
        const bool plucky = cat == "Bells & Resonators" || cat == "Sequence & Plucks" || cat == "Keys & Organs";
        const double target = plucky ? -22.0 : (cat == "Basses" || cat == "Leads" ? -19.0 : -20.0);
        double adj = target - rmsDb;
        const double pkDb = 20 * std::log10(pk + 1e-9);
        const double pkCap = cat == "FX & Details" ? -6.0 : -3.0;
        if (pkDb + adj > pkCap) adj = pkCap - pkDb;
        adj = std::max(-24.0, std::min(24.0, adj));
        const double master = std::round((-6.0 + adj) * 2.0) / 2.0;
        const bool bad = nan || (rmsDb < -60 && pkDb < -40) || missing;
        if (bad) ++problems;
        // audit metrics over the measure window (mono): brightness, low / high share, frame-to-frame change,
        // long-term evolution (first vs last third), stereo correlation
        double cen = 0, lo = 0, hi = 0, flux = 0, evo = 0, corr = 0;
        {
            const int N = 4096, i0 = (int)(m0 * sr), i1 = std::min(n, (int)(m1 * sr));
            std::vector<double> win(N); for (int k = 0; k < N; ++k) win[k] = 0.5 - 0.5 * std::cos(6.283185307 * k / N);
            std::vector<std::vector<double>> mags;
            double eTot = 0, eLo = 0, eHi = 0, cw = 0;
            for (int s0 = i0; s0 + N <= i1; s0 += N) {
                std::vector<double> re(N), im(N, 0.0);
                for (int k = 0; k < N; ++k) re[k] = 0.5 * (L[s0 + k] + R[s0 + k]) * win[k];
                drift::ana::fft(re, im);
                std::vector<double> m(N / 2);
                double fe = 0, fc = 0;
                for (int b = 1; b < N / 2; ++b) {
                    const double e2 = re[b] * re[b] + im[b] * im[b], hz = b * (double)sr / N;
                    m[b] = std::sqrt(e2); fe += e2; fc += hz * e2;
                    if (hz < 100) eLo += e2; else if (hz > 5000) eHi += e2;
                }
                eTot += fe; if (fe > 0) { cen += fc; cw += fe; }
                mags.push_back(m);
            }
            if (cw > 0) cen /= cw;
            if (eTot > 0) { lo = eLo / eTot; hi = eHi / eTot; }
            double fmax = 0; for (auto& m : mags) { double t = 0; for (int b = 1; b < N / 2; ++b) t += m[b] * m[b]; fmax = std::max(fmax, t); }
            int fc2 = 0;
            for (size_t f = 1; f < mags.size(); ++f) {
                double d = 0, t = 0;
                for (int b = 1; b < N / 2; ++b) { const double x = mags[f][b] - mags[f - 1][b]; d += x * x; t += mags[f][b] * mags[f][b]; }
                if (t > fmax * 1e-4) { flux += std::sqrt(d / t); ++fc2; }
            }
            if (fc2) flux /= fc2;
            if (mags.size() >= 3) {
                const size_t th = mags.size() / 3;
                std::vector<double> a(N / 2, 0.0), b2(N / 2, 0.0);
                for (size_t f = 0; f < th; ++f) for (int b = 1; b < N / 2; ++b) a[b] += mags[f][b];
                for (size_t f = mags.size() - th; f < mags.size(); ++f) for (int b = 1; b < N / 2; ++b) b2[b] += mags[f][b];
                double d = 0, t = 0; for (int b = 1; b < N / 2; ++b) { d += (a[b] - b2[b]) * (a[b] - b2[b]); t += a[b] * a[b] + b2[b] * b2[b]; }
                if (t > 0) evo = std::sqrt(d / t);
            }
            double sll = 0, srr = 0, slr = 0;
            for (int k = i0; k < i1; ++k) { sll += L[k] * L[k]; srr += R[k] * R[k]; slr += L[k] * R[k]; }
            if (sll > 0 && srr > 0) corr = slr / std::sqrt(sll * srr);
        }
        fprintf(tsv, "%s\t%s\t%.1f\t%.1f\t%.1f\t%.4f\t%.0f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n", fp.name, fp.category, rmsDb, pkDb, master, cpu, cen, lo, hi, flux, evo, corr);
        fflush(tsv);
        printf("%-3u %-26s %-19s rms %6.1f  pk %6.1f -> %+5.1f dB  cpu %4.1f%%%s%s%s\n", pi, fp.name, fp.category, rmsDb, pkDb, master, cpu * 100,
               nan ? "  NaN!" : "", (rmsDb < -60 && pkDb < -40) ? "  SILENT!" : "", missing ? "  MISSING FILE!" : "");
        fprintf(js, "%s  \"%s\": %.1f", jsCount++ ? ",\n" : "", fp.name, std::max(-36.0, std::min(24.0, master)));
        if (writeRaw) { const int keep = std::min(n, 8 * sr); const float g = std::pow(10.f, (float)(adj / 20.0)); for (int i = 0; i < keep; ++i) { raw.push_back(L[i] * g); raw.push_back(R[i] * g); } }
    }
    fprintf(js, "\n}\n"); fclose(js);
    printf("%d problems; heaviest %s %.1f%%\n", problems, cpuMaxName.c_str(), cpuMax * 100);
    if (writeRaw) { FILE* f = fopen(argv[1], "wb"); fwrite(raw.data(), 4, raw.size(), f); fclose(f); }
    delete e; delete e2;
}
