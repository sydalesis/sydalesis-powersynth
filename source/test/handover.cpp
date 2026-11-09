// Thread check for 3.2 background loading: one thread renders, another decodes files and hands
// tables / samples to the engine (as the plug-in's loader thread does). Build with -fsanitize=thread.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Library.hpp"
#include <thread>
#include <atomic>
#include <cstdio>
#include <vector>

int main() {
    drift::IEngine* e = drift::createEngine(true);
    e->setSampleRate(48000); e->setTransport(120, false, 0);
    drift::Telemetry tel{}; e->setTelemetry(&tel);
    for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, P(i).def);
    e->setParam(oscP(0, ofType), (float)otWavetable);
    e->setParam(oscP(1, ofType), (float)otGranular);
    e->setParam(oscP(1, ofOn), 1.f);
    auto tables = drift::lib::scan();
    auto samples = drift::lib::scanSamples();
    std::printf("%zu tables, %zu samples\n", tables.size(), samples.size());
    if (tables.empty() || samples.empty()) return 1;
    std::atomic<bool> stop { false };
    std::atomic<int> blocks { 0 };
    std::thread audio([&] {
        std::vector<float> L(256), R(256);
        uint8_t on[3] = { 0x90, 50, 100 }, off[3] = { 0x80, 50, 0 };
        while (!stop.load()) {
            if ((blocks % 40) == 0) { on[1] = off[1] = (uint8_t)(40 + blocks % 30); e->midi(on, 3); }
            if ((blocks % 40) == 30) e->midi(off, 3);
            e->process(L.data(), R.data(), 256);
            for (float x : L) if (!std::isfinite(x)) { std::printf("NaN\n"); std::exit(1); }
            ++blocks;
        }
    });
    for (int i = 0; i < 120; ++i) {
        const auto& t = tables[(size_t)(i * 7) % tables.size()];
        e->postTable(i % 3, (i % 11) == 0 ? nullptr : drift::loadWavetableFile(t.path.c_str(), false));
        const auto& s = samples[(size_t)(i * 5) % samples.size()];
        e->postSample(i % 3, (i % 13) == 0 ? nullptr : drift::loadSampleFile(s.path.c_str()));
    }
    stop.store(true); audio.join();
    std::printf("rendered %d blocks while handing over 240 files\n", blocks.load());
    delete e;
    return 0;
}
