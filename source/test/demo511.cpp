// Driftfield 5.1.1 demo reel: factory presets at their stored levels, both layers, every state (wavetables,
// samples, spectral sources, modes, MSEGs, scenes, motion, tuning, FX order, XY axes), cross-faded.
// No added effects. "Name@xy" plays the preset with XY pad 1 auto-motion (circle) switched on.
// usage: demo511 out.raw "Preset A" "Preset B@xy" ...   (raw = float32 stereo interleaved, 48 kHz)
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../Library.hpp"
#include "../Spectra.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../SpecData.hpp"
#include "../Presets.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

struct Ev { double t; uint8_t b0, b1, b2; };

static void setup(drift::IEngine* en, int layer, const float* pv, const std::string* st) {
    const uint32_t po = layer ? kLayerBOffset : 0; const int so = layer ? kLayerStates : 0;
    en->reset();
    for (uint32_t i = 0; i < kEngineParams; ++i) en->setParam(i, pv[i < kLayerParams ? i + po : i]);
    for (int o = 0; o < 3; ++o) {
        en->postTable(o, st[so + skWtA + o].empty() ? nullptr : drift::loadWavetableFile(drift::lib::resolve(st[so + skWtA + o]).c_str(), false));
        en->postSample(o, st[so + skSmA + o].empty() ? nullptr : drift::loadSampleFile(drift::lib::resolve(st[so + skSmA + o]).c_str()));
        float amps[128]; if (!drift::spectrumFromString(st[so + skSpA + o].c_str(), amps)) drift::spectrumDefault(amps); en->setCustomSpectrum(o, amps);
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
    { int a[4] = { 0, 1, 2, 3 }; if (std::sscanf(st[so + skPerf].c_str(), "%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3]) != 4) { a[0] = 0; a[1] = 1; a[2] = 2; a[3] = 3; } en->setPerfAxes(a); }
    drift::ScaleDef sc; drift::KbmDef km;
    const std::string scl = drift::tnUnescape(st[so + skScl]), kbm = drift::tnUnescape(st[so + skKbm]);
    if (!scl.empty() && drift::parseScl(scl.c_str(), sc)) en->setUserScale(&sc, (!kbm.empty() && drift::parseKbm(kbm.c_str(), km)) ? &km : nullptr);
    else en->setUserScale(nullptr, nullptr);
}

int main(int argc, char** argv) {
    if (argc < 3) { std::printf("usage: demo511 out.raw names...\n"); return 2; }
    const int sr = 48000, seg = 16 * sr, tail = 4 * sr;
    std::vector<float> out;
    static float pv[kParamCount];
    for (int a = 2; a < argc; ++a) {
        std::string nm = argv[a];
        bool xy = false;
        if (nm.size() > 3 && nm.substr(nm.size() - 3) == "@xy") { xy = true; nm = nm.substr(0, nm.size() - 3); }
        int pi = -1; for (uint32_t i = 0; i < kFactoryPresetCount; ++i) if (nm == kFactoryPresets[i].name) pi = (int)i;
        if (pi < 0) { std::printf("missing %s\n", nm.c_str()); continue; }
        presetValues((uint32_t)pi, pv);
        if (xy) { pv[pXy1Mode] = 1.f; pv[pXy1Rate] = 0.2f; pv[pXy1Size] = 1.0f; pv[pMacro1] = 0.5f; pv[pMacro1 + 1] = 0.5f; }
        if (std::getenv("DEMO_WET")) {
            // demo only (factory presets stay dry): shimmer from the Spectral FX (octave up, smeared), a dotted-eighth
            // ping-pong delay and a long Space reverb, on both layers
            for (int l = 0; l < 2; ++l) {
                const uint32_t o = l ? kLayerBOffset : 0;
                if (pv[o + pSxOn] < 0.5f) { pv[o + pSxOn] = 1; pv[o + pSxMix] = 0.32f; pv[o + pSxShift] = 12; pv[o + pSxBlur] = 4; pv[o + pSxSpread] = 0.7f; pv[o + pSxTilt] = -0.25f; pv[o + pSxGate] = 0.1f; pv[o + pSxFreeze] = 0; }
                pv[o + pDlOn] = 1; pv[o + pDlMix] = 0.22f; pv[o + pDlSync] = 1; pv[o + pDlDiv] = 9; pv[o + pDlFb] = 0.45f; pv[o + pDlDamp] = 0.55f; pv[o + pDlMode] = 1;
                pv[o + pSpOn] = 1; pv[o + pSpMix] = 0.38f; pv[o + pSpSize] = 0.92f; pv[o + pSpDecay] = 11; pv[o + pSpDamp] = 0.45f; pv[o + pSpPre] = 0.04f; pv[o + pSpMod] = 0.45f; pv[o + pSpWidth] = 1;
            }
        }
        std::string st[kStateCount]; presetStates((uint32_t)pi, st);
        drift::IEngine* e = drift::createEngine(); e->setSampleRate((float)sr);
        drift::IEngine* e2 = drift::createEngine(); e2->setSampleRate((float)sr); e2->setLayer(1);
        const bool layered = pv[pLyMode] >= 0.5f;
        setup(e, 0, pv, st); if (layered) setup(e2, 1, pv, st);
        const std::string cat = kFactoryPresets[pi].category, tags = kFactoryPresets[pi].tags;
        std::vector<Ev> ev;
        auto chord = [&](std::vector<int> ns, double on, double off, int vel = 92) { for (int n : ns) { ev.push_back({on, 0x90, (uint8_t)n, (uint8_t)vel}); ev.push_back({off, 0x80, (uint8_t)n, 0}); } };
        if (tags.find("hold-a-note") != std::string::npos) chord({45}, 0, 14);
        else if (tags.find("hold-a") != std::string::npos || tags.find("sequenced") != std::string::npos || cat == "Berlin Sequences") chord({45, 52, 57}, 0, 14);
        else if (cat == "Drones" || cat == "Dark Ambient") chord({38, 45, 50}, 0, 14);
        else if (cat == "Bells & Resonators") { const int ns[10] = { 57, 64, 69, 72, 76, 74, 69, 64, 62, 57 }; for (int i = 0; i < 10; ++i) chord({ ns[i] }, i * 1.25, i * 1.25 + 1.0, 70 + (i * 7) % 30); }
        else { chord({50, 57, 62, 65, 69}, 0, 7.2); chord({46, 53, 58, 62, 65}, 7.0, 14); }
        std::sort(ev.begin(), ev.end(), [](const Ev& x, const Ev& y) { return x.t < y.t; });
        const int n = seg + tail;
        std::vector<float> L(n), R(n), L2(256), R2(256);
        size_t ei = 0; int pos = 0; double beat = 0;
        while (pos < n) {
            while (ei < ev.size() && (int)(ev[ei].t * sr) <= pos) { uint8_t m[3] = { ev[ei].b0, ev[ei].b1, ev[ei].b2 }; e->midi(m, 3); if (layered) e2->midi(m, 3); ++ei; }
            int end = std::min(pos + 256, n); if (ei < ev.size()) end = std::min(end, std::max(pos + 1, (int)(ev[ei].t * sr)));
            e->setTransport(100, true, beat); e->process(&L[pos], &R[pos], (uint32_t)(end - pos));
            if (layered) { e2->setTransport(100, true, beat); e2->process(L2.data(), R2.data(), (uint32_t)(end - pos)); for (int i = 0; i < end - pos; ++i) { L[pos + i] += L2[i]; R[pos + i] += R2[i]; } }
            beat += (end - pos) / (double)sr * 100.0 / 60.0; pos = end;
        }
        const size_t start = out.size() >= (size_t)tail * 2 ? out.size() - (size_t)tail * 2 : 0;
        out.resize(std::max(out.size(), start + (size_t)n * 2), 0.f);
        for (int i = 0; i < n; ++i) {
            float g = i >= n - tail ? (float)(n - i) / tail : 1.f;
            if (i < sr / 3) g *= (float)i / (sr / 3);
            out[start + 2 * (size_t)i] += L[i] * g; out[start + 2 * (size_t)i + 1] += R[i] * g;
        }
        std::printf("%6.1f s  %-28s %s%s\n", start / 2.0 / sr, nm.c_str(), cat.c_str(), xy ? "  (XY auto-motion: circle)" : "");
        delete e; delete e2;
    }
    // safety limiter like the plug-in's (transparent below -1 dBFS)
    float env = 0.f; const float thr = 0.8912509f, att = 1.f - std::exp(-1.f / (0.001f * sr)), rel = 1.f - std::exp(-1.f / (0.15f * sr));
    for (size_t i = 0; i + 1 < out.size(); i += 2) {
        const float pk = std::max(std::fabs(out[i]), std::fabs(out[i + 1]));
        env += (pk - env) * (pk > env ? att : rel);
        const float g = env > thr ? thr / env : 1.f;
        for (int c = 0; c < 2; ++c) { float x = out[i + c] * g; const float ax = std::fabs(x); if (ax > thr) { const float o = thr + (1.f - thr) * std::tanh((ax - thr) / (1.f - thr)); x = x < 0 ? -o : o; } out[i + c] = x; }
    }
    FILE* f = std::fopen(argv[1], "wb"); std::fwrite(out.data(), 4, out.size(), f); std::fclose(f);
    return 0;
}
