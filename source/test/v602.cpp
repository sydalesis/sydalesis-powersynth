// 6.0.2: MPE pressure modes; released notes keep their expression (Osmose: pressure -> 0 before note-off, channel reuse)
#include "../Params.hpp"
#include "../Shared.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)
static const int SR = 48000;
static void set(drift::IEngine* e, const char* s, float v) { uint32_t en; hostToEngine((uint32_t)ParamRegistry::get().findSymbol(s), en); e->setParam(en, v); }
static double tone(const std::vector<float>& x, int a, int b, double f) {   // Goertzel amplitude
    double s1 = 0, s2 = 0, c = 2 * std::cos(2 * M_PI * f / SR);
    for (int i = a; i < b; ++i) { double s0 = x[i] + c * s1 - s2; s2 = s1; s1 = s0; }
    return std::sqrt(s1 * s1 + s2 * s2 - c * s1 * s2) * 2 / (b - a);
}
static std::vector<float> run(int mode) {
    drift::IEngine* e = drift::createEngine(true); e->setSampleRate((float)SR);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    for (uint32_t i = pX0; i < pX0 + kExtParams; ++i) e->setParam(i, P(i).def);
    set(e, "obOn", 0); set(e, "ocOn", 0); set(e, "oaType", (float)otAnalog); set(e, "oaAnWave", 4); set(e, "oaVoices", 1);
    set(e, "f1Type", 0); set(e, "f2Type", 0); set(e, "subLevel", 0); set(e, "noiseLevel", 0);
    set(e, "aeA", 0.001f); set(e, "aeS", 1); set(e, "aeR", 3.f); set(e, "pressAmp", 1.f); set(e, "pressCut", 0.f);
    for (int u = 0; u < kFxUnits; ++u) e->setParam(kFxOnParam[u], 0.f);
    e->setPressMode(mode);
    std::vector<float> L(SR * 3), R(SR * 3);
    auto ev = [&](uint8_t a, uint8_t b, uint8_t c, uint32_t n) { const uint8_t m[3] = { a, b, c }; e->midi(m, n); };
    for (int p = 0; p < SR * 3; p += 240) {
        const double t = (double)p / SR;
        if (p == 0) { ev(0xD1, 0, 0, 2); ev(0x91, 57, 100, 3); }
        if (t < 1.0) ev(0xD1, (uint8_t)std::min(127.0, t * 400), 0, 2);                // press in
        else if (t < 1.1) ev(0xD1, (uint8_t)std::max(0.0, 127 * (1.1 - t) / 0.1), 0, 2);   // lift: pressure falls first
        if (p == (int)(1.1 * SR)) { ev(0xD1, 0, 0, 2); ev(0x81, 57, 0, 3); }
        if (p == (int)(1.6 * SR)) ev(0x91, 64, 100, 3);                                // next note, same channel
        if (t > 1.6) ev(0xD1, (uint8_t)std::min(127.0, (t - 1.6) * 300), 0, 2);
        e->process(&L[p], &R[p], 240);
    }
    delete e; return L;
}
int main() {
    if (ParamRegistry::get().findSymbol("pressAmp") < 0) { std::printf("no prsAmp symbol\n"); }
    double H[3], Rl[3], B[3];
    for (int mode = 0; mode < 3; ++mode) {
        auto x = run(mode);
        H[mode] = tone(x, SR * 9 / 10, SR, 220.0); Rl[mode] = tone(x, (int)(1.25 * SR), (int)(1.55 * SR), 220.0); B[mode] = tone(x, (int)(2.3 * SR), (int)(2.6 * SR), 220.0);
        std::printf("      mode %d: held %.4f  release %.4f (%.2f of held)  old note after the next note %.4f\n", mode, H[mode], Rl[mode], Rl[mode] / H[mode], B[mode]);
    }
    CHECK(B[0] <= Rl[0] * 1.05 && B[1] <= Rl[1] * 1.05, "the next note's pressure on the same channel no longer brings a released note back");
    CHECK(Rl[1] / H[1] > 1.4 * Rl[0] / H[0] && Rl[1] / H[1] > 0.85 * Rl[2] / H[2], "Hold: lifting the key keeps the pressed level into the release");
    CHECK(H[2] < H[0] * 0.6, "Off: pressure is ignored");
    std::printf(fails ? "\n%d FAILED\n" : "\nall passed\n", fails); return fails ? 1 : 0;
}
