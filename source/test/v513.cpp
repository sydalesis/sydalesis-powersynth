// Sydalesis 5.1.3: release curves. 0 = the old exponential release exactly; +/- end silent at the release time,
// > 0 is below the exponential mid-release, < 0 above it.
#include "../EngineIface.hpp"
#include "../Params.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)
static const int SR = 48000;
static int sym(const char* s) { return ParamRegistry::get().findSymbol(s); }
static std::vector<float> run(float rk, int which) {
    drift::IEngine* e = drift::createEngine(); e->setSampleRate((float)SR);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(sym("quality"), 1.f); e->setParam(sym("aeA"), 0.002f); e->setParam(sym("aeS"), 1.f); e->setParam(sym("aeR"), 1.0f);
    if (which >= 0) e->setParam(which, rk);
    const uint8_t on[3] = { 0x90, 57, 100 }, off[3] = { 0x80, 57, 0 };
    e->midi(on, 3); std::vector<float> L(SR * 2), R(SR * 2);
    e->process(L.data(), R.data(), SR / 2); e->midi(off, 3); e->process(L.data() + SR / 2, R.data() + SR / 2, SR * 3 / 2);
    // rms in 50 ms windows after key-up
    std::vector<float> w; for (int s = SR / 2; s + 2400 <= SR * 2; s += 2400) { double a = 0; for (int i = 0; i < 2400; ++i) a += L[s + i] * L[s + i]; w.push_back((float)std::sqrt(a / 2400)); }
    delete e; return w;
}
int main() {
    const int pa = sym("aRelCurve");
    CHECK(pa == (int)pARelCurve && sym("fRelCurve") == (int)pFRelCurve && sym("mRelCurve") == (int)pMRelCurve, "release-curve parameters registered");
    CHECK(P(pARelCurve).def == 0.f && P(pARelCurve).min == -1.f && P(pARelCurve).max == 1.f, "range -1..1, default 0");
    const std::vector<float> base = run(0.f, -1), zero = run(0.f, pa), fast = run(1.f, pa), hold = run(-1.f, pa);
    bool same = base.size() == zero.size(); for (size_t i = 0; same && i < base.size(); ++i) same = base[i] == zero[i];
    CHECK(same, "curve 0 is bit-identical to the release of earlier versions");
    const size_t mid = 6;   // 300 ms into a 1 s release
    CHECK(fast[mid] < base[mid] * 0.5f && hold[mid] > base[mid] * 2.f, "300 ms in: fast %.4f < exp %.4f < hold %.4f", fast[mid], base[mid], hold[mid]);
    CHECK(fast[21] < 1e-4f && hold[21] < 1e-4f, "both curves silent after the release time (%.2g / %.2g at 1.05 s)", fast[21], hold[21]);
    std::printf(fails ? "%d FAILURES\n" : "all good\n", fails); return fails ? 1 : 0;
}
