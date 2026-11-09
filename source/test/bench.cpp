#include "../Params.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <vector>
#include <chrono>
#include <functional>
static double run(drift::IEngine* e, std::function<void(drift::IEngine*)> setup) {
    for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, P(i).def);
    e->setParam(oscP(1, ofOn), 0); e->setParam(oscP(2, ofOn), 0); e->setParam(pAEnvA, 0.01f);
    setup(e);
    uint8_t off[3] = {0xB0, 120, 0}; e->midi(off, 3);
    for (int i = 0; i < 16; ++i) { uint8_t m[3] = {0x90, (uint8_t)(36 + i * 3), 100}; e->midi(m, 3); }
    std::vector<float> a(512), b(512);
    for (int i = 0; i < 20; ++i) e->process(a.data(), b.data(), 512);
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 48000 * 3 / 512; ++i) e->process(a.data(), b.data(), 512);
    return 100.0 * std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count() / 3.0;
}
int main() {
    for (int g = 0; g < 2; ++g) {
        auto* e = drift::createEngine(g); e->setSampleRate(48000);
        printf("[%s] 16 voices, one osc:\n", e->isaName());
        printf("  analog saw x1          %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otAnalog); }));
        printf("  analog saw x7          %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otAnalog); e->setParam(oscP(0, ofVoices), 7); }));
        printf("  analog sync x7         %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otAnalog); e->setParam(oscP(0, ofVoices), 7); e->setParam(oscP(0, ofAnSync), 9); }));
        printf("  wavetable x7           %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otWavetable); e->setParam(oscP(0, ofVoices), 7); }));
        printf("  additive 64            %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otAdditive); e->setParam(oscP(0, ofAdPartials), 64); }));
        printf("  physical modal         %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otPhysical); e->setParam(oscP(0, ofPmModel), 3); }));
        printf("  physical waveguide     %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofType), otPhysical); e->setParam(oscP(0, ofPmModel), 7); }));
        printf("  osc off (filter only)  %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(oscP(0, ofLevel), 0.f); }));
        printf("  + F2 ladder parallel   %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(pF2Type, 1); e->setParam(pFRouting, 1); }));
        printf("  phaser8 F1             %5.1f%%\n", run(e, [](drift::IEngine* e) { e->setParam(pF1Type, 33); }));
        delete e;
    }
}
// appended: v1.3-comparable load
int main2() { return 0; }
