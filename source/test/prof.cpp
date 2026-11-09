#include "../Params.hpp"
#include "../EngineIface.hpp"
#include <vector>
int main() {
    auto* e = drift::createEngine(true); e->setSampleRate(48000);
    e->setParam(oscP(1, ofOn), 0); e->setParam(oscP(0, ofLevel), 0.f); e->setParam(pAEnvA, 0.01f);
    for (int i = 0; i < 16; ++i) { uint8_t m[3] = {0x90, (uint8_t)(36 + i * 3), 100}; e->midi(m, 3); }
    std::vector<float> a(512), b(512);
    for (int i = 0; i < 48000 / 512; ++i) e->process(a.data(), b.data(), 512);
    delete e;
}
