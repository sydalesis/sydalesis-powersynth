// Sydalesis 5.1.6: OCT / SEMI / FINE / PITCH modulation, Bipolar curve, tape wow/flutter speed, MSEG playhead position.
#include "../Params.hpp"
#include "../Shared.hpp"
#include "../EngineIface.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)
static const int SR = 48000;
static int sym(const char* s) { int i = ParamRegistry::get().findSymbol(s); if (i < 0) { std::printf("no symbol %s\n", s); ++fails; } return i; }
static drift::IEngine* fresh() {
    drift::IEngine* e = drift::createEngine(true); e->setSampleRate((float)SR);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(sym("quality"), 1.f);
    // one plain analog saw, no filter colour, no unison
    e->setParam(sym("obOn"), 0.f); e->setParam(sym("ocOn"), 0.f); e->setParam(sym("oaOn"), 1.f);
    e->setParam(sym("oaType"), (float)otAnalog); e->setParam(sym("oaVoices"), 1.f); e->setParam(sym("oaAnDrift"), 0.f); e->setParam(sym("oaAnSlop"), 0.f);
    e->setParam(sym("f1Type"), 0.f); e->setParam(sym("f2Type"), 0.f); e->setParam(sym("drift"), 0.f); e->setParam(sym("subLevel"), 0.f); e->setParam(sym("noiseLevel"), 0.f);
    e->setParam(sym("aeA"), 0.001f); e->setParam(sym("aeS"), 1.f);
    return e;
}
// YIN pitch of the left channel over [a, b) seconds
static double yin(const std::vector<float>& x, int a, int len) {
    const int maxTau = SR / 50; std::vector<double> d(maxTau, 0.0);
    for (int tau = 1; tau < maxTau; ++tau) { double s = 0; for (int i = 0; i < len; ++i) { const double df = x[a + i] - x[a + i + tau]; s += df * df; } d[tau] = s; }
    double run = 0; for (int tau = 1; tau < maxTau; ++tau) { run += d[tau]; const double cm = d[tau] * tau / (run > 0 ? run : 1);
        if (tau > SR / 4000 && cm < 0.12) { while (tau + 1 < maxTau && d[tau + 1] < d[tau]) ++tau;
            const double y0 = d[tau - 1], y1 = d[tau], y2 = d[tau + 1], den = y0 - 2 * y1 + y2; const double off = den != 0 ? 0.5 * (y0 - y2) / den : 0; return SR / (tau + off); } }
    return 0.0;
}
static double freqOf(drift::IEngine* e, double a, double b, int note = 57) {
    const uint8_t on[3] = { 0x90, (uint8_t)note, 100 };
    e->midi(on, 3);
    const int n = (int)(b * SR) + SR / 10;
    std::vector<float> L(n), R(n);
    for (int p = 0; p < n; p += 256) e->process(&L[p], &R[p], std::min(256, n - p));
    return yin(L, (int)(a * SR), (int)((b - a) * SR * 0.25));
}
static void route(drift::IEngine* e, int slot, int src, int dst, float amt, int curve = 0) {
    e->setParam(modSrcParam(slot), (float)src); e->setParam(modDstParam(slot), (float)dst); e->setParam(modAmtParam(slot), amt); e->setParam(modCurveParam(slot), (float)curve);
}
int main() {
    if (ParamRegistry::get().count() != kParamCount) { std::printf("REGISTRY MISMATCH\n"); return 1; }
    CHECK(sym("tpWowRate") == (int)pTpWowRate && sym("tpFlutRate") == (int)pTpFlutRate && sym("pitch") == (int)pPitch, "new parameter ids line up with the registry");
    CHECK(sym("b_pitch") == (int)(kLayerBOffset + pPitch), "layer 2 copies exist (b_pitch)");
    CHECK(pPitch < pRsv1 && kLayerParams == 700, "new params come out of the reserve; layer block size unchanged");
    const double f0 = [&] { auto* e = fresh(); double f = freqOf(e, 0.2, 0.6); delete e; return f; }();
    CHECK(std::fabs(f0 - 220.0) < 1.0, "reference A3 = %.2f Hz", f0);
    // macro 1 -> A Octave: amount 1/3 at macro 1.0 = +1 octave
    { auto* e = fresh(); e->setParam(sym("macro1"), 1.f); route(e, 0, msMacro1, mdAOct, 1.f / 3.f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - 2.0) < 0.01, "macro -> A Octave (+1/3) = %.3fx", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(sym("macro1"), 1.f); route(e, 0, msMacro1, mdAOct, 0.25f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - 2.0) < 0.01, "octave mod is quantised: 0.75 oct -> %.3fx (whole octave)", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(sym("macro1"), 1.f); route(e, 0, msMacro1, mdASemi, 7.f / 12.f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - std::pow(2.0, 7.0 / 12.0)) < 0.005, "macro -> A Semi (+7) = %.4fx", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(sym("macro1"), 1.f); route(e, 0, msMacro1, mdAFine, 1.f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - std::pow(2.0, 0.5 / 12.0)) < 0.002, "macro -> A Fine (full) = +50 ct (%.4fx)", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(sym("macro1"), 1.f); route(e, 0, msMacro1, mdBOct, 1.f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - 1.0) < 0.003, "B Octave leaves osc A alone (%.4fx)", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(pPitch, 12.f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - 2.0) < 0.01, "PITCH +12 st = %.3fx", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(pPitch, -3.5f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - std::pow(2.0, -3.5 / 12.0)) < 0.004, "PITCH -3.5 st = %.4fx (continuous)", f / f0); delete e; }
    { auto* e = fresh(); e->setParam(sym("macro1"), 1.f); route(e, 0, msMacro1, mdPitchAll, 0.5f); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - std::sqrt(2.0)) < 0.01, "Pitch All (the PITCH knob destination) +0.5 = +6 st (%.3fx)", f / f0); delete e; }
    // Bipolar curve: macro at 0 through Bipolar = -1 -> -1 octave with amount 1/3
    { auto* e = fresh(); e->setParam(sym("macro1"), 0.f); route(e, 0, msMacro1, mdAOct, 1.f / 3.f, 7); const double f = freqOf(e, 0.2, 0.6); CHECK(std::fabs(f / f0 - 0.5) < 0.01, "Bipolar curve maps macro 0 to -1 (%.3fx)", f / f0); delete e; }
    // LFO -> A Octave through Unipolar: never goes below the base pitch
    {
        auto* e = fresh(); e->setParam(lfoP(0, lfRate), 2.f); e->setParam(lfoP(0, lfWave), 4.f); route(e, 0, msLfo1, mdAOct, 1.f / 3.f, 4);
        const uint8_t on[3] = { 0x90, 57, 100 }; e->midi(on, 3);
        std::vector<float> L(SR * 2), R(SR * 2);
        for (int p = 0; p < SR * 2; p += 256) e->process(&L[p], &R[p], 256);
        double fLo = 1e9, fHi = 0;
        for (int w = SR / 10; w + SR / 20 < SR * 2 - SR / 50; w += SR / 40) { const double f = yin(L, w, SR / 100); if (f > 0) { fLo = std::min(fLo, f); fHi = std::max(fHi, f); } }
        CHECK(fLo > f0 * 0.97 && fHi < f0 * 2.06 && fHi > f0 * 1.9, "square LFO -> octave (unipolar) toggles %.1f .. %.1f Hz", fLo, fHi);
        delete e;
    }
    // tape: speeds change the output; defaults equal the old fixed speeds
    {
        auto run = [&](float wr, float fr) {
            auto* e = fresh(); e->setParam(pTpOn, 1.f); e->setParam(pTpWow, 1.f); e->setParam(pTpFlutter, 1.f); e->setParam(pTpWowRate, wr); e->setParam(pTpFlutRate, fr);
            const uint8_t on[3] = { 0x90, 57, 100 }; e->midi(on, 3);
            std::vector<float> L(SR * 3), R(SR * 3);
            for (int p = 0; p < SR * 3; p += 256) e->process(&L[p], &R[p], std::min(256, SR * 3 - p));
            // instantaneous period track -> how fast the pitch wanders (count direction changes of the period)
            std::vector<int> per; int last = -1;
            for (int i = SR / 2; i < SR * 3; ++i) if (L[i - 1] < 0.f && L[i] >= 0.f) { if (last > 0) per.push_back(i - last); last = i; }
            double dev = 0; for (size_t k = 1; k < per.size(); ++k) dev += std::fabs(per[k] - per[k - 1]);
            delete e; return dev / per.size();
        };
        const double slow = run(0.1f, 2.f), def = run(0.45f, 7.3f), fast = run(4.f, 30.f);
        CHECK(fast > def * 1.5 && def > slow * 1.2, "tape wander speed follows the speed knobs (period jitter %.3f / %.3f / %.3f)", slow, def, fast);
        CHECK(std::fabs(P(pTpWowRate).def - 0.45f) < 1e-6f && std::fabs(P(pTpFlutRate).def - 7.3f) < 1e-6f, "tape speed defaults are the old fixed 0.45 / 7.3 Hz");
    }
    // MSEG playhead: telemetry segment position follows time inside the segment, also on a bent segment
    {
        auto* e = fresh();
        static drift::Telemetry tel; e->setTelemetry(&tel);
        drift::MsegData d; d.count = 2; d.t[0] = 0.f; d.l[0] = 0.f; d.t[1] = 1.f; d.l[1] = 1.f; d.c[1] = 0.9f; d.loopStart = 0; d.loopEnd = 0;
        e->setMseg(0, d); e->setParam(msegP(0, 0), 0.f); e->setParam(msegP(0, 1), 1.f);
        route(e, 0, msMseg1, mdF1Cutoff, 0.1f);
        const uint8_t on[3] = { 0x90, 57, 100 }; e->midi(on, 3);
        std::vector<float> L(256), R(256);
        bool ok = true; float worst = 0.f; int samples = 0;
        for (int b = 0; b < SR / 256; ++b) {
            e->process(L.data(), R.data(), 256);
            const double t = (b + 1) * 256.0 / SR;
            const int nv = tel.newest; if (nv < 0) continue;
            const drift::TelVoice& v = tel.v[nv];
            if (t > 0.05 && t < 0.95) {
                ++samples;
                const float err = std::fabs(v.msegU[0] - (float)t);
                worst = std::max(worst, err);
                const float valFromU = drift::msegCurve(v.msegU[0], d.c[1]);
                if (std::fabs(valFromU - v.mseg[0]) > 0.02f) ok = false;
                // the old UI solved u from the value as if the segment were straight: far off on this curve
            }
        }
        CHECK(samples > 100 && worst < 0.02f && ok, "MSEG segment position tracks time on a curved segment (worst %.4f over %d reads)", worst, samples);
        delete e;
    }
    std::printf(fails ? "%d FAILED\n" : "all good\n", fails);
    return fails ? 1 : 0;
}
