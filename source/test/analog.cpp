#include "../Params.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <vector>
#include <algorithm>
int main() {
    auto* e = drift::createEngine(); e->setSampleRate(48000);
    std::vector<float> out;
    struct Case { int wave; float sync; int note; bool sweepSync; bool sweepPW; };
    Case cs[] = { {0, 0, 84, false, false}, {1, 0, 84, false, true}, {2, 0, 72, false, true}, {3, 0, 84, false, false}, {0, 0, 60, true, false}, {1, 0, 60, true, false} };
    for (auto& c : cs) {
        for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, P(i).def);
        e->setParam(oscP(1, ofOn), 0); e->setParam(pF1Type, 0); e->setParam(pChMix, 0); e->setParam(pAEnvA, 0.005f); e->setParam(pAEnvS, 1);
        e->setParam(oscP(0, ofAnWave), c.wave); e->setParam(oscP(0, ofAnSlop), 0); e->setParam(pDrift, 0);
        uint8_t on[3] = {0x90, (uint8_t)c.note, 100}; e->midi(on, 3);
        const int n = 48000 * 3;
        std::vector<float> L(n), R(n);
        for (int i = 0; i < n; i += 256) {
            const float t = (float)i / n;
            if (c.sweepSync) e->setParam(oscP(0, ofAnSync), 36 * t);
            if (c.sweepPW) e->setParam(oscP(0, ofAnPW), 0.5f - 0.45f * t);
            // pitch sweep via bend on master channel for non-sync cases
            if (!c.sweepSync && !c.sweepPW) { int v = 8192 + (int)(t * 8191); uint8_t pb[3] = {0xE0, (uint8_t)(v & 127), (uint8_t)(v >> 7)}; e->midi(pb, 3); }
            e->process(&L[i], &R[i], std::min(256, n - i));
        }
        uint8_t off[3] = {0xB0, 120, 0}; e->midi(off, 3);
        uint8_t pb0[3] = {0xE0, 0, 64}; e->midi(pb0, 3);
        out.insert(out.end(), L.begin(), L.end());
    }
    FILE* f = fopen("analog.raw", "wb"); fwrite(out.data(), 4, out.size(), f); fclose(f);
}
