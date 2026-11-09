// pluginval-style stress: random values for every parameter, process blocks, time them.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
int main() {
    auto* e = drift::createEngine(); e->setSampleRate(44100);
    std::mt19937 rng(1);
    std::uniform_real_distribution<float> u(0, 1);
    std::vector<float> L(512), R(512);
    double worst = 0, total = 0; int worstIt = -1;
    for (int it = 0; it < 200; ++it) {
        for (uint32_t p = 0; p < kParamCount; ++p) { const ParamDef& d = P(p); float v = d.min + u(rng) * (d.max - d.min); if (d.flags & fInt) v = std::round(v); e->setParam(p, v); }
        if (it % 10 == 0) for (int n = 0; n < 6; ++n) { uint8_t m[3] = { 0x90, (uint8_t)(40 + rng() % 40), 100 }; e->midi(m, 3); }
        e->setTransport(120, (it & 1), it * 0.5);
        const auto t0 = std::chrono::steady_clock::now();
        e->process(L.data(), R.data(), 512);
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        total += ms; if (ms > worst) { worst = ms; worstIt = it; }
        for (float v : L) if (!std::isfinite(v)) { printf("NaN at it %d\n", it); return 1; }
    }
    printf("worst block %.2f ms (budget 11.6 ms) at iteration %d, mean %.2f ms\n", worst, worstIt, total / 200);
}
