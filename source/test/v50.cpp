// Driftfield 5.0 checks: spectral oscillator (fidelity, pitch, speed, freeze, shift, cross, formant,
// sparse, blur), Spectral FX (transparency, freeze, shift), multicore determinism.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../SpecData.hpp"
#include "../Pool.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
#include <cstring>
#include <memory>
#include <functional>

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { ++bad; std::printf("FAIL: "); } else std::printf("ok:   "); std::printf(__VA_ARGS__); std::printf("\n"); } while (0)
static const int SR = 48000;
static void note(drift::IEngine* e, int n, int vel, bool on) { uint8_t m[3] = { (uint8_t)(on ? 0x90 : 0x80), (uint8_t)n, (uint8_t)vel }; e->midi(m, 3); }
static drift::IEngine* fresh() {
    drift::IEngine* e = drift::createEngine(true);
    e->setSampleRate((float)SR); e->setTransport(120, false, 0); e->setLayer(0);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(pAEnvA, 0.001f); e->setParam(pAEnvS, 1.f); e->setParam(pAEnvR, 0.05f);
    e->setParam(pChOn, 0.f); e->setParam(pF1Type, 0.f); e->setParam(pF2Type, 0.f);
    e->setParam(oscP(1, ofOn), 0.f); e->setParam(pDrift, 0.f); e->setParam(pF1Drive, 0.f);
    return e;
}
static void spec(drift::IEngine* e) { e->setParam(oscP(0, ofType), (float)otSpectral); e->setParam(pSpread, 0.f); }
static void render(drift::IEngine* e, int frames, std::vector<float>* outL = nullptr, std::vector<float>* outR = nullptr) {
    std::vector<float> L(256), R(256);
    for (int pos = 0; pos < frames; pos += 256) {
        const int n = std::min(256, frames - pos);
        e->process(L.data(), R.data(), (uint32_t)n);
        if (outL) outL->insert(outL->end(), L.begin(), L.begin() + n);
        if (outR) outR->insert(outR->end(), R.begin(), R.begin() + n);
    }
}
static double rmsOf(const std::vector<float>& x, size_t a, size_t b) { double s = 0; b = std::min(b, x.size()); for (size_t i = a; i < b; ++i) s += x[i] * x[i]; return std::sqrt(s / std::max<size_t>(1, b - a)); }
static double goertzel(const std::vector<float>& x, size_t a, size_t b, double f) {
    const double w = 2 * M_PI * f / SR, c = 2 * std::cos(w); double s1 = 0, s2 = 0;
    for (size_t i = a; i < b; ++i) { const double s0 = x[i] + c * s1 - s2; s2 = s1; s1 = s0; }
    return 2 * std::sqrt(std::max(0.0, s1 * s1 + s2 * s2 - c * s1 * s2)) / (double)(b - a);
}
static double peakFreq(const std::vector<float>& x, size_t a, size_t len, double lo, double hi) {
    double best = 0, bf = 0;
    for (double f = lo; f <= hi; f *= 1.0015) { const double g = goertzel(x, a, a + len, f); if (g > best) { best = g; bf = f; } }
    return bf;
}
static drift::SpecData* make(std::function<float(double)> sig, double secs) {
    std::vector<float> x((size_t)(secs * SR));
    for (size_t i = 0; i < x.size(); ++i) x[i] = sig((double)i / SR);
    drift::SpecData* d = new drift::SpecData();
    drift::analyseSpec(x.data(), (int)x.size(), (float)SR, *d);
    return d;
}
static double sine(double t, double f, double a = 0.8) { return a * std::sin(2 * M_PI * f * t); }

int main() {
    // ---- fidelity and pitch
    {
        drift::IEngine* e = fresh(); spec(e);
        drift::SpecData* d = make([](double t) { return (float)sine(t, 220.0); }, 3.0);
        std::printf("analysis: %d frames x %d bins, root %.2f Hz (%s)\n", d->frames, d->bins, d->root, d->pitched ? "pitched" : "unpitched");
        CHECK(std::fabs(d->root - 220.f) < 1.f, "root detected at %.2f Hz", d->root);
        e->postSpectral(0, 0, d);
        render(e, 256);
        note(e, 57, 127, true);
        std::vector<float> L; render(e, SR * 2, &L);
        const double f = peakFreq(L, SR / 2, SR, 200, 240);
        // reference: the same note on an analog sine, same level
        drift::IEngine* r = fresh(); r->setParam(oscP(0, ofType), (float)otAnalog); r->setParam(oscP(0, ofAnWave), 4.f);
        note(r, 57, 127, true);
        std::vector<float> Lr; render(r, SR * 2, &Lr);
        const double a = rmsOf(L, SR / 2, SR * 2), b = rmsOf(Lr, SR / 2, SR * 2) * 0.8;
        CHECK(std::fabs(f - 220.0) < 0.5, "keyed A3 plays the source at 220 Hz (%.2f)", f);
        CHECK(std::fabs(20 * std::log10(a / b)) < 1.0, "level matches the source (%.2f dB vs a 0.8 sine)", 20 * std::log10(a / b));
        // purity: energy away from 220 Hz
        double tot = 0, at = goertzel(L, SR / 2, SR * 2, 220.0);
        tot = rmsOf(L, SR / 2, SR * 2) * std::sqrt(2.0);
        CHECK(at / tot > 0.97, "resynthesis is clean (220 Hz holds %.1f %% of the amplitude)", at / tot * 100);
        note(e, 57, 0, false); render(e, SR / 4);
        note(e, 69, 127, true);
        std::vector<float> L2; render(e, SR, &L2);
        const double f2 = peakFreq(L2, SR / 2, SR / 2, 400, 480);
        CHECK(std::fabs(f2 - 440.0) < 1.0, "A4 shifts it an octave up (%.2f Hz)", f2);
        delete e; delete r;
    }
    // ---- speed, position, freeze, shift on a two-tone file (300 Hz then 600 Hz, 1 s each)
    auto twoTone = []() { return make([](double t) { return (float)(t < 1.0 ? sine(t, 300.0) : sine(t, 600.0)); }, 2.0); };
    auto run = [&](std::function<void(drift::IEngine*)> setup, std::vector<float>& L, int secs = 2) {
        drift::IEngine* e = fresh(); spec(e);
        e->setParam(osc2P(0, o2SpKeyed), 0.f);
        setup(e);
        e->postSpectral(0, 0, twoTone());
        render(e, 256);
        note(e, 60, 127, true);
        render(e, SR * secs, &L);
        delete e;
    };
    {
        std::vector<float> L; run([](drift::IEngine*) {}, L);
        const double a = peakFreq(L, SR / 4, SR / 2, 250, 700), b = peakFreq(L, SR * 5 / 4, SR / 2, 250, 700);
        CHECK(std::fabs(a - 300) < 2 && std::fabs(b - 600) < 2, "speed 1 plays through the file (%.1f then %.1f Hz)", a, b);
        std::vector<float> L2; run([](drift::IEngine* e) { e->setParam(osc2P(0, o2SpSpeed), 0.5f); }, L2);
        const double c = peakFreq(L2, SR * 5 / 4, SR / 2, 250, 700);
        CHECK(std::fabs(c - 300) < 2, "speed 0.5 is still in the first tone at 1.5 s (%.1f Hz): time-stretch without pitch change", c);
        std::vector<float> L3; run([](drift::IEngine* e) { e->setParam(osc2P(0, o2SpSpeed), 0.f); e->setParam(osc2P(0, o2SpPos), 0.75f); }, L3);
        const double d = peakFreq(L3, SR / 2, SR, 250, 700);
        CHECK(std::fabs(d - 600) < 2, "speed 0 holds position 0.75 (%.1f Hz)", d);
        std::vector<float> L4; run([](drift::IEngine* e) { e->setParam(osc2P(0, o2SpShift), 100.f); }, L4);
        const double s = peakFreq(L4, SR / 4, SR / 2, 250, 700);
        CHECK(std::fabs(s - 400) < 3, "shift +100 Hz moves 300 to %.1f Hz", s);
        // freeze after half a second
        std::vector<float> L5;
        {
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f);
            e->postSpectral(0, 0, twoTone()); render(e, 256);
            note(e, 60, 127, true);
            render(e, SR / 2, &L5);
            e->setParam(osc2P(0, o2SpFreeze), 1.f);
            render(e, SR * 3 / 2, &L5);
            delete e;
        }
        const double fz = peakFreq(L5, SR * 3 / 2, SR / 2, 250, 700);
        const double lv = rmsOf(L5, SR * 3 / 2, SR * 2) / rmsOf(L5, SR / 4, SR / 2);
        CHECK(std::fabs(fz - 300) < 2 && lv > 0.8 && lv < 1.25, "freeze holds the first tone (%.1f Hz at 1.5 s, level %.2fx)", fz, lv);
        // saved frozen (freeze on before the note): holds POSITION, at full level — also on a reused voice
        {
            std::vector<float> L6, Lr;
            run([](drift::IEngine* e) { e->setParam(osc2P(0, o2SpFreeze), 1.f); e->setParam(osc2P(0, o2SpPos), 0.75f); }, L6);
            run([](drift::IEngine* e) { e->setParam(osc2P(0, o2SpSpeed), 0.f); e->setParam(osc2P(0, o2SpPos), 0.75f); }, Lr);
            const double f6 = peakFreq(L6, SR / 2, SR, 250, 700), l6 = rmsOf(L6, SR / 2, SR * 3 / 2) / rmsOf(Lr, SR / 2, SR * 3 / 2);
            CHECK(std::fabs(f6 - 600) < 2 && l6 > 0.8 && l6 < 1.25, "freeze on from note start holds position 0.75 (%.1f Hz, level %.2fx of unfrozen)", f6, l6);
        }
        // blur: the old tone lingers after the switch
        std::vector<float> L6; run([](drift::IEngine* e) { e->setParam(osc2P(0, o2SpBlur), 2.f); }, L6);
        const double old = goertzel(L6, SR * 11 / 10, SR * 13 / 10, 300), nw = goertzel(L6, SR * 11 / 10, SR * 13 / 10, 600);
        std::vector<float> L7; run([](drift::IEngine*) {}, L7);
        const double old0 = goertzel(L7, SR * 11 / 10, SR * 13 / 10, 300);
        CHECK(old > nw * 0.5 && old > old0 * 10, "blur 2 s smears the first tone past the switch (300 Hz %.3f vs %.4f unblurred)", old, old0);
    }
    // ---- cross-synthesis
    {
        auto cross = [&](int mode, float amt, std::vector<float>& L, std::function<float(double)> a, std::function<float(double)> b) {
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f);
            e->setParam(osc2P(0, o2SpCrossMode), (float)mode); e->setParam(osc2P(0, o2SpCross), amt);
            e->postSpectral(0, 0, make(a, 2.0)); e->postSpectral(0, 1, make(b, 2.0));
            render(e, 256); note(e, 60, 127, true); render(e, SR * 3 / 2, &L); delete e;
        };
        auto s300 = [](double t) { return (float)sine(t, 300.0); };
        auto s900 = [](double t) { return (float)sine(t, 900.0); };
        std::vector<float> a0, a1, ah;
        cross(1, 0.f, a0, s300, s900); cross(1, 1.f, a1, s300, s900); cross(1, 0.5f, ah, s300, s900);
        const double r0 = goertzel(a0, SR / 2, SR, 900) / goertzel(a0, SR / 2, SR, 300), r1 = goertzel(a1, SR / 2, SR, 300) / goertzel(a1, SR / 2, SR, 900);
        const double h3 = goertzel(ah, SR / 2, SR, 300), h9 = goertzel(ah, SR / 2, SR, 900);
        const double full = goertzel(a0, SR / 2, SR, 300);
        CHECK(r0 < 0.02 && r1 < 0.02 && h3 > 0.3 * full && h3 < 0.7 * full && h9 > 0.3 * full && h9 < 0.7 * full, "morph: 0 = source (300 Hz), 1 = partner (900 Hz), 0.5 = half of each (%.2f / %.2f of %.2f; leak %.3f %.3f)", h3, h9, full, r0, r1);
        // vocode: a harmonic-rich source takes the partner's bright spectral envelope
        auto saw = [](double t) { double v = 0; for (int h = 1; h <= 40; ++h) v += std::sin(2 * M_PI * 110 * h * t) / h; return (float)(0.4 * v); };
        auto hi = [](double t) { double v = 0; for (int h = 18; h <= 22; ++h) v += std::sin(2 * M_PI * 100 * h * t); return (float)(0.15 * v); };
        std::vector<float> v0, v1;
        cross(2, 0.f, v0, saw, hi); cross(2, 1.f, v1, saw, hi);
        auto band = [&](const std::vector<float>& x, double lo, double hi2) { double s = 0; for (int h = 1; h <= 40; ++h) { const double f = 110.0 * h; if (f >= lo && f <= hi2) s += std::pow(goertzel(x, SR / 2, SR, f), 2); } return s; };
        const double before = band(v0, 1700, 2300) / band(v0, 100, 600), after = band(v1, 1700, 2300) / band(v1, 100, 600);
        CHECK(after > before * 20, "vocode: energy around 2 kHz rises %.0fx relative to the low partials", after / before);
    }
    // ---- formant keeping and sparse
    {
        // a 110 Hz pulse train through a resonance at 800 Hz, played an octave up
        auto voice = [](double t) { double v = 0; for (int h = 1; h <= 40; ++h) { const double f = 110.0 * h; const double g = 1.0 / (1.0 + std::pow((f - 800.0) / 150.0, 2)); v += g * std::sin(2 * M_PI * f * t); } return (float)(0.3 * v); };
        auto play = [&](float formant, std::vector<float>& L) {
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f); e->setParam(oscP(0, ofOct), 1.f);
            e->setParam(osc2P(0, o2SpFormant), formant);
            e->postSpectral(0, 0, make(voice, 2.0)); render(e, 256); note(e, 60, 127, true); render(e, SR * 3 / 2, &L); delete e;
        };
        std::vector<float> a, b; play(0.f, a); play(1.f, b);
        auto peakOf = [&](const std::vector<float>& x) { double best = 0, bf = 0; for (int h = 1; h <= 20; ++h) { const double f = 220.0 * h; const double g = goertzel(x, SR / 2, SR, f); if (g > best) { best = g; bf = f; } } return bf; };
        const double pa = peakOf(a), pb = peakOf(b);
        CHECK(std::fabs(pa - 1540) < 230 && std::fabs(pb - 880) < 230, "formant: an octave up the resonance moves to %.0f Hz; with Formant 1 it stays at %.0f Hz", pa, pb);
        // sparse: noise under a sine; Sparse keeps the sine
        auto noisy = [](double t) { static uint32_t r = 1; r = r * 1664525u + 1013904223u; return (float)(sine(t, 500.0, 0.4) + ((r >> 9) / 4194304.0 - 1.0) * 0.9); };
        auto sp = [&](float amt, std::vector<float>& L) {
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f); e->setParam(osc2P(0, o2SpSparse), amt);
            e->postSpectral(0, 0, make(noisy, 2.0)); render(e, 256); note(e, 60, 127, true); render(e, SR * 3 / 2, &L); delete e;
        };
        std::vector<float> s0, s1; sp(0.f, s0); sp(1.f, s1);
        const double q0 = goertzel(s0, SR / 2, SR, 500) / (rmsOf(s0, SR / 2, SR) * 1.4142), q1 = goertzel(s1, SR / 2, SR, 500) / (rmsOf(s1, SR / 2, SR) * 1.4142);
        CHECK(q1 > q0 * 1.3 && q1 > 0.8, "sparse keeps the sine and drops the noise (sine share %.2f -> %.2f)", q0, q1);
    }
    // ---- HQ (N = 4096) and heavy modulation stay finite
    {
        drift::IEngine* e = fresh(); spec(e); e->setParam(pQuality, 2.f);
        e->setParam(osc2P(0, o2SpStretch), 0.7f); e->setParam(osc2P(0, o2SpTilt), -0.6f); e->setParam(osc2P(0, o2SpBlur), 0.4f);
        e->setParam(slotP(0, sfSrc), (float)msLfo1); e->setParam(slotP(0, sfDst), (float)(mdAT1)); e->setParam(slotP(0, sfAmt), 0.8f);
        e->setParam(lfoP(0, lfRate), 3.f);
        auto saw = [](double t) { double v = 0; for (int h = 1; h <= 30; ++h) v += std::sin(2 * M_PI * 130 * h * t) / h; return (float)(0.4 * v); };
        e->postSpectral(0, 0, make(saw, 3.0)); render(e, 256);
        for (int n : { 36, 48, 60, 72, 84, 96 }) note(e, n, 100, true);
        std::vector<float> L; render(e, SR * 3, &L);
        bool fin = true; float pk = 0; for (float v : L) { fin = fin && std::isfinite(v); pk = std::max(pk, std::fabs(v)); }
        CHECK(fin && pk < 4.f && rmsOf(L, SR, SR * 3) > 1e-3, "HQ, 6 notes, stretch / tilt / blur / LFO on position: finite, peak %.2f", pk);
        delete e;
    }
    // ---- Spectral FX
    {
        auto fx = [&](std::function<void(drift::IEngine*)> setup, std::vector<float>& L, std::vector<float>& dry, std::function<float(double)> src) {
            // the source goes through an analog-free path: a spectral osc playing `src` at speed 1 is the input
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f);
            e->postSpectral(0, 0, make(src, 3.0)); render(e, 256);
            drift::IEngine* d = fresh(); spec(d); d->setParam(osc2P(0, o2SpKeyed), 0.f);
            d->postSpectral(0, 0, make(src, 3.0)); render(d, 256);
            e->setParam(pSxOn, 1.f); e->setParam(pSxMix, 1.f); e->setParam(pSxBlur, 0.f); e->setParam(pSxSpread, 0.f);
            setup(e);
            note(e, 60, 127, true); note(d, 60, 127, true);
            render(e, SR * 2, &L); render(d, SR * 2, &dry);
            delete e; delete d;
        };
        auto two = [](double t) { return (float)(t < 1.0 ? sine(t, 300.0) : sine(t, 600.0)); };
        std::vector<float> L, D;
        fx([](drift::IEngine*) {}, L, D, two);
        double err = 0, ref = 0;
        for (int i = SR / 2; i < SR * 3 / 2; ++i) { const double x = D[(size_t)(i - 4096)]; err += (L[(size_t)i] - x) * (L[(size_t)i] - x); ref += x * x; }
        CHECK(10 * std::log10(err / ref) < -60, "neutral Spectral FX is transparent (one frame later): error %.1f dB", 10 * std::log10(err / ref));
        std::vector<float> L2, D2;
        fx([](drift::IEngine* e) { e->setParam(pSxShift, 12.f); }, L2, D2, two);
        const double s = peakFreq(L2, SR / 2, SR / 3, 250, 1300);
        CHECK(std::fabs(s - 600) < 4, "shift +12 st: 300 Hz in, %.1f Hz out", s);
        std::vector<float> L3, D3;
        {
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f);
            e->postSpectral(0, 0, make(two, 3.0)); render(e, 256);
            e->setParam(pSxOn, 1.f); e->setParam(pSxMix, 1.f); e->setParam(pSxBlur, 0.f); e->setParam(pSxSpread, 0.f);
            note(e, 60, 127, true);
            render(e, SR * 2 / 3, &L3);
            e->setParam(pSxFreeze, 1.f);
            render(e, SR * 4 / 3, &L3);
            delete e;
        }
        const double fzf = peakFreq(L3, SR * 3 / 2, SR / 3, 250, 700);
        CHECK(std::fabs(fzf - 300) < 3, "FX freeze holds 300 Hz after the input moved to 600 (%.1f Hz)", fzf);
        // saved frozen: freeze on before any sound captures the first audible frame, not silence
        std::vector<float> L4;
        {
            drift::IEngine* e = fresh(); spec(e); e->setParam(osc2P(0, o2SpKeyed), 0.f);
            e->postSpectral(0, 0, make(two, 3.0));
            e->setParam(pSxOn, 1.f); e->setParam(pSxMix, 1.f); e->setParam(pSxBlur, 0.f); e->setParam(pSxSpread, 0.f); e->setParam(pSxFreeze, 1.f);
            render(e, SR / 2);
            note(e, 60, 127, true);
            render(e, SR * 2, &L4);
            delete e;
        }
        const double f4 = peakFreq(L4, SR * 3 / 2, SR / 3, 250, 700), r4 = rmsOf(L4, SR * 3 / 2, SR * 2);
        CHECK(std::fabs(f4 - 300) < 3 && r4 > 1e-3, "FX saved frozen: holds the first sound (%.1f Hz, rms %.4f)", f4, r4);
    }
    // ---- multicore: spectral voices give identical output on 1 and 3 threads
    {
        auto go = [&](drift::IWorkerPool* pool, std::vector<float>& L) {
            drift::IEngine* e = fresh(); spec(e); e->setWorkerPool(pool);
            auto saw = [](double t) { double v = 0; for (int h = 1; h <= 20; ++h) v += std::sin(2 * M_PI * 150 * h * t) / h; return (float)(0.4 * v); };
            e->postSpectral(0, 0, make(saw, 2.0)); render(e, 256);
            for (int n : { 48, 55, 60, 64 }) note(e, n, 100, true);
            render(e, SR, &L); delete e;
        };
        std::vector<float> a, b; drift::WorkerPool p3(2);
        go(nullptr, a); go(&p3, b);
        CHECK(a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * 4) == 0, "spectral voices: 1 and 3 threads bit-identical");
    }
    std::printf(bad ? "\n%d FAILURES\n" : "\nall good\n", bad);
    return bad ? 1 : 0;
}
