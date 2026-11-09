#include "../Params.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <vector>
#include <chrono>
int main() {
    for (int g = 0; g < 2; ++g) {
        auto* e = drift::createEngine(g); e->setSampleRate(48000);
        e->setParam(oscP(0, ofType), otWavetable); e->setParam(oscP(0, ofVoices), 7);
        e->setParam(oscP(1, ofType), otWavetable); e->setParam(oscP(1, ofVoices), 7);
        e->setParam(oscP(2, ofOn), 1); e->setParam(oscP(2, ofType), otAdditive); e->setParam(oscP(2, ofAdPartials), 64);
        e->setParam(pF1Type, 33); e->setParam(pAEnvA, 0.01f);
        for (int s = 0; s < 8; ++s) { e->setParam(modSrcParam(s), 1 + s % 4); e->setParam(modDstParam(s), 1 + s * 3); e->setParam(modAmtParam(s), 0.3f); }
        for (int i = 0; i < 16; ++i) { uint8_t m[3] = {0x90, (uint8_t)(36 + i * 3), 100}; e->midi(m, 3); }
        std::vector<float> a(512), b(512);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 48000 * 5 / 512; ++i) e->process(a.data(), b.data(), 512);
        printf("[%s] v1.3-comparable worst case: %.0f%% of one core (v1.3: 40%%)\n", e->isaName(), 100.0 * std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count() / 5.0);
        delete e;
    }
}
