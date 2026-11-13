// 6.0.5 performance pass: every 6.0-bank preset (or a stride of them) with N held voices at its own quality
// (HQ by default in the bank), single thread. Reports % of one core per preset and writes the render so a
// later build can be null-tested against it.
// build (from source/test): g++ -std=gnu++17 -O3 -ffast-math -fno-finite-math-only -I.. perf605.cpp ../EngineGeneric.cpp ../EngineFactory.cpp ../Platform.cpp ../EngineAVX2.cpp -o perf605 -ldl -lpthread
//   (compile EngineAVX2.cpp with -mavx2 -mfma, as the Makefile does)
// env: DF_STRIDE (1), DF_ONLY (substring), DF_VOICES (24), DF_SECS (1.0), DF_TSV (out), DF_DUMP (dir for renders), DF_CMP (dir to compare)
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../Library.hpp"
#include "../Spectra.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../SpecData.hpp"
#include "../Presets.hpp"
#include "../Pool.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

int main() {
    const int sr = 48000;
    const uint32_t stride = std::getenv("DF_STRIDE") ? (uint32_t)std::max(1, std::atoi(std::getenv("DF_STRIDE"))) : 1u;
    const char* only = std::getenv("DF_ONLY");
    const int voices = std::getenv("DF_VOICES") ? std::atoi(std::getenv("DF_VOICES")) : 24;
    const double secs = std::getenv("DF_SECS") ? std::atof(std::getenv("DF_SECS")) : 1.0;
    const char* dump = std::getenv("DF_DUMP");
    const char* cmp = std::getenv("DF_CMP");
    FILE* tsv = std::fopen(std::getenv("DF_TSV") ? std::getenv("DF_TSV") : "/dev/null", "w");
    float pv[kParamCount];
    const int threads = std::getenv("DF_THREADS") ? std::atoi(std::getenv("DF_THREADS")) : 1;
    drift::WorkerPool* pool = threads > 1 ? new drift::WorkerPool(threads - 1) : nullptr;
    double total = 0; int count = 0; double worstDiff = 0; std::string worstName;
    for (uint32_t pi = 0; pi < kFactoryPresetCount; pi += stride) {
        const FactoryPreset& fp = kFactoryPresets[pi];
        if (fp.bank != 0) continue;
        if (only && !std::strstr(fp.name, only)) continue;
        presetValues(pi, pv);
        if (const char* q = std::getenv("DF_QUALITY")) pv[pQuality] = (float)std::atoi(q);
        const bool layered = pv[pLyMode] >= 0.5f;
        std::string st[kStateCount]; presetStates(pi, st);
        drift::IEngine* eng[2] = { drift::createEngine(), layered ? drift::createEngine() : nullptr };
        for (int layer = 0; layer < (layered ? 2 : 1); ++layer) {
            drift::IEngine* en = eng[layer];
            en->setSampleRate((float)sr); if (layer) en->setLayer(1);
            en->setWorkerPool(pool);
            const uint32_t po = layer ? kLayerBOffset : 0; const int so = layer ? kLayerStates : 0;
            en->reset();
            for (uint32_t i = 0; i < kEngineParams; ++i) en->setParam(i, pv[i < kLayerParams ? i + po : i]);
            for (uint32_t i = pX0; i < pX0 + kExtParams; ++i) en->setParam(i, pv[layer ? layer2Of(i) : i]);
            for (int o = 0; o < 3; ++o) {
                en->postTable(o, st[so + skWtA + o].empty() ? nullptr : drift::loadWavetableFile(drift::lib::resolve(st[so + skWtA + o]).c_str(), false));
                en->postSample(o, st[so + skSmA + o].empty() ? nullptr : drift::loadSampleFile(drift::lib::resolve(st[so + skSmA + o]).c_str()));
                float amps[128]; if (!drift::spectrumFromString(st[so + skSpA + o].c_str(), amps)) drift::spectrumDefault(amps); en->setCustomSpectrum(o, amps);
                { float a2[128]; if (!drift::spectrumFromString(st[so + skSp2A + o].c_str(), a2)) drift::spectrumDefault(a2); en->setCustomSpectrum(3 + o, a2); }
                for (int w = 0; w < 2; ++w) {
                    const std::string& sv = st[so + (w ? skXsrcA : skSrcA) + o];
                    en->postSpectral(o, w, sv.empty() ? nullptr : drift::loadSpecFile(drift::lib::resolve(sv).c_str()));
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
            else en->setUserScale(nullptr, nullptr);
        }
        const int nWarm = (int)(0.4 * sr), nMeas = (int)(secs * sr), n = nWarm + nMeas;
        std::vector<float> L(n), R(n), L2(256), R2(256);
        int pos = 0;
        double t = 0;
        for (int v = 0; v < voices; ++v) {
            uint8_t m[3] = { 0x90, (uint8_t)(33 + (v * 37) % 60), (uint8_t)(70 + (v * 13) % 50) };
            for (int k = 0; k < (layered ? 2 : 1); ++k) eng[k]->midi(m, 3);
        }
        while (pos < n) {
            const int len = std::min(256, n - pos);
            const auto t0 = std::chrono::steady_clock::now();
            for (int k = 0; k < (layered ? 2 : 1); ++k) {
                eng[k]->setTransport(120.0, false, pos / (double)sr * 2.0);
                eng[k]->process(k ? L2.data() : &L[pos], k ? R2.data() : &R[pos], len);
            }
            if (pos >= nWarm) t += std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
            if (layered) for (int i = 0; i < len; ++i) { L[pos + i] += L2[i]; R[pos + i] += R2[i]; }
            pos += len;
        }
        const double cpu = 100.0 * t / secs;
        double e = 0; for (int i = nWarm; i < n; ++i) e += (double)L[i] * L[i] + (double)R[i] * R[i];
        const double rms = std::sqrt(e / (2.0 * nMeas));
        std::string types;
        for (int o = 0; o < 3; ++o) if (pv[oscP(o, ofOn)] >= 0.5f) { if (!types.empty()) types += "+"; types += kOscTypeNames[(int)pv[oscP(o, ofType)]]; }
        double diffDb = -999;
        char fn[512];
        if (dump) { std::snprintf(fn, sizeof fn, "%s/%u.f32", dump, pi); if (FILE* f = std::fopen(fn, "wb")) { std::fwrite(L.data(), 4, n, f); std::fwrite(R.data(), 4, n, f); std::fclose(f); } }
        if (cmp) {
            std::snprintf(fn, sizeof fn, "%s/%u.f32", cmp, pi);
            if (FILE* f = std::fopen(fn, "rb")) {
                std::vector<float> b(2 * (size_t)n);
                if (std::fread(b.data(), 4, b.size(), f) == b.size()) {
                    double d = 0, s = 0;
                    for (int i = 0; i < n; ++i) { const double a = L[i] - b[i], c = R[i] - b[n + i]; d += a * a + c * c; s += (double)b[i] * b[i] + (double)b[n + i] * b[n + i]; }
                    diffDb = 10.0 * std::log10((d + 1e-30) / (s + 1e-30));
                    if (diffDb > worstDiff || worstName.empty()) { worstDiff = diffDb; worstName = fp.name; }
                }
                std::fclose(f);
            }
        }
        std::printf("%-34s %-28s q%d %6.1f%%  rms %.4f%s", fp.name, types.c_str(), (int)pv[pQuality], cpu, rms, layered ? "  [layered]" : "");
        if (cmp) std::printf("  diff %.1f dB", diffDb);
        std::printf("\n"); std::fflush(stdout);
        std::fprintf(tsv, "%u\t%s\t%s\t%d\t%.2f\t%.5f\t%.1f\n", pi, fp.name, types.c_str(), layered ? 1 : 0, cpu, rms, diffDb);
        total += cpu; ++count;
        delete eng[0]; delete eng[1];
    }
    std::printf("presets %d, mean %.1f%% of one core%s\n", count, count ? total / count : 0.0, "");
    if (cmp) std::printf("worst diff %.1f dB (%s)\n", worstDiff, worstName.c_str());
    if (tsv) std::fclose(tsv);
}
