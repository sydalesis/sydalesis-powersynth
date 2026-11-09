// Driftfield 4.2 checks: MSEG 3/4, slots 25-32, scenes, motion, physical 2.0 models and exciters,
// WAV -> modes, Resonator, Space, multicore determinism, FX order compatibility.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Analysis.hpp"
#include "../Pool.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
#include <memory>
#include <cstring>

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { ++bad; std::printf("FAIL: "); } else std::printf("ok:   "); std::printf(__VA_ARGS__); std::printf("\n"); } while (0)
static const int SR = 48000;
static void note(drift::IEngine* e, int n, int vel, bool on) { uint8_t m[3] = { (uint8_t)(on ? 0x90 : 0x80), (uint8_t)n, (uint8_t)vel }; e->midi(m, 3); }
static drift::IEngine* fresh() {
    drift::IEngine* e = drift::createEngine(true);
    e->setSampleRate((float)SR); e->setTransport(120, false, 0); e->setLayer(0);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(pAEnvA, 0.001f); e->setParam(pAEnvS, 1.f); e->setParam(pAEnvR, 0.05f);
    e->setParam(pChOn, 0.f);
    return e;
}
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
static float peakOf(const std::vector<float>& x) { float p = 0; for (float v : x) p = std::max(p, std::fabs(v)); return p; }
static bool finiteAll(const std::vector<float>& x) { for (float v : x) if (!std::isfinite(v)) return false; return true; }
// pitch by normalised autocorrelation over [a, a+len)
static double pitchOf(const std::vector<float>& x, size_t a, size_t len, double fmin = 50, double fmax = 1500) {
    const int tmin = (int)(SR / fmax), tmax = (int)(SR / fmin);
    double best = -1; int bt = 0;
    std::vector<double> r((size_t)tmax + 2, 0.0);
    for (int t = tmin; t <= tmax + 1; ++t) {
        double s = 0, e1 = 0, e2 = 0;
        for (size_t i = a; i < a + len; ++i) { s += x[i] * x[i + t]; e1 += x[i] * x[i]; e2 += x[i + t] * x[i + t]; }
        r[(size_t)t] = s / std::sqrt(e1 * e2 + 1e-20);
    }
    // first peak above 0.85 of the maximum (avoids octave errors)
    double mx = 0; for (int t = tmin; t <= tmax; ++t) mx = std::max(mx, r[(size_t)t]);
    for (int t = tmin + 1; t <= tmax; ++t) if (r[(size_t)t] >= 0.85 * mx && r[(size_t)t] >= r[(size_t)t - 1] && r[(size_t)t] >= r[(size_t)t + 1]) { best = r[(size_t)t]; bt = t; break; }
    if (bt == 0) return 0;
    const double y0 = r[(size_t)bt - 1], y1 = r[(size_t)bt], y2 = r[(size_t)bt + 1];
    const double den = y0 - 2 * y1 + y2;
    const double off = std::fabs(den) > 1e-12 ? 0.5 * (y0 - y2) / den : 0.0;
    (void)best;
    return SR / (bt + off);
}
static double goertzel(const std::vector<float>& x, size_t a, size_t b, double f) {
    const double w = 2 * M_PI * f / SR, c = 2 * std::cos(w); double s1 = 0, s2 = 0;
    for (size_t i = a; i < b; ++i) { const double s0 = x[i] + c * s1 - s2; s2 = s1; s1 = s0; }
    return std::sqrt(s1 * s1 + s2 * s2 - c * s1 * s2) / (double)(b - a);
}
static double cents(double f, double ref) { return 1200.0 * std::log2(f / ref); }

int main() {
    // ---- registry
    std::printf("params: %u per layer (reserve from %u), %u total\n", (unsigned)kLayerParams, (unsigned)pRsv1, (unsigned)kParamCount);
    CHECK(ParamRegistry::get().count() == kParamCount, "registry holds %u params", ParamRegistry::get().count());
    CHECK(std::string(P(542).symbol) == "neVibRate" && std::string(P(pMacro1).symbol) == "macro1" && std::string(P(slotP(23, 4)).symbol) == "mod24Curve", "4.1 layer-1 parameter ids unchanged");
    CHECK(std::string(P(slotP(24, 0)).symbol) == "mod25Src" && std::string(P(slotP(31, 3)).symbol) == "mod32Amt" && std::string(P(msegP(3, 1)).symbol) == "mseg4Scale", "slots 25-32 and MSEG 3/4 map to their block");
    CHECK(std::string(P(kLayerBOffset + osc2P(1, o2Pressure)).symbol) == "b_obPmPress" && (P(pRsv1).flags & fHidden), "layer 2 copy and hidden reserves");
    {
        int o[kFxUnits];
        drift::fxOrderFromString("0,1,2,3,4,5,6,7,8", o);
        CHECK(o[9] == fxReso && o[10] == fxSpace, "a 4.1 FX order gets Resonator and Space appended");
        drift::fxOrderDefault(o);
        CHECK(o[4] == fxReso && o[3] == fxSpectral && o[kFxUnits - 1] == fxSpace, "new patches: Spectral, Resonator before the delay, Space last");
        drift::fxOrderFromString("0,1,2,9,3,4,5,6,7,8,10", o);
        CHECK(o[10] == fxSpace && o[11] == fxSpectral, "a 4.2 FX order gets Spectral appended");
    }

    std::unique_ptr<drift::Telemetry> tel(new drift::Telemetry());
    // ---- MSEG 3/4 and slot 30
    {
        drift::IEngine* e = fresh(); e->setTelemetry(tel.get());
        drift::MsegData m3; drift::msegDefault(m3, 2); e->setMseg(2, m3);
        drift::MsegData m4; drift::msegDefault(m4, 3); e->setMseg(3, m4);
        note(e, 60, 100, true);
        render(e, SR * 4);
        const int nv = tel->newest;
        const float a = nv >= 0 ? tel->v[nv].mseg[2] : -1.f, b = nv >= 0 ? tel->v[nv].mseg[3] : -1.f;
        CHECK(a > 0.1f && a < 0.95f, "MSEG 3 (8 s swell) at 4 s = %.2f", a);
        CHECK(b >= 0.f && b <= 1.f && nv >= 0 && tel->v[nv].msegSeg[3] >= 1, "MSEG 4 runs (%.2f, segment %d)", b, nv >= 0 ? tel->v[nv].msegSeg[3] : -1);
        delete e;
        auto lvl = [&](float macro) {
            drift::IEngine* x = fresh();
            x->setParam(slotP(29, sfSrc), (float)msMacro1); x->setParam(slotP(29, sfDst), (float)mdAmp); x->setParam(slotP(29, sfAmt), -1.f);
            x->setParam(pMacro1, macro);
            note(x, 60, 100, true);
            std::vector<float> L; render(x, SR / 2, &L);
            delete x;
            return rmsOf(L, SR / 4, SR / 2);
        };
        const double l0 = lvl(0.f), l1 = lvl(1.f);
        CHECK(l0 > 1e-3 && l1 < l0 * 0.01, "slot 30: Macro 1 -> Amp -100 %% silences (%.4f -> %.6f)", l0, l1);
    }
    // ---- scenes
    {
        drift::IEngine* e = fresh(); e->setTelemetry(tel.get());
        drift::SceneData sd;
        sd.used[0] = sd.used[1] = true;
        for (int m = 0; m < kMacros; ++m) { sd.v[0][m] = 0.2f; sd.v[1][m] = 0.8f; }
        const std::string txt = drift::scenesToString(sd);
        drift::SceneData back; drift::scenesFromString(txt.c_str(), back);
        CHECK(back.used[0] && back.used[1] && !back.used[2] && std::fabs(back.v[1][3] - 0.8f) < 1e-3f, "scene text round trip (%s)", txt.substr(0, 40).c_str());
        e->setScenes(back);
        e->setParam(pScOn, 1.f); e->setParam(pScGlide, 0.f); e->setParam(pScPos, 0.5f); e->setParam((pMacro1 + 2), 0.33f);
        render(e, SR / 10);
        CHECK(std::fabs(tel->macro[0] - 0.5f) < 0.01f, "scene morph 0.5 between 0.2 and 0.8 -> %.3f", tel->macro[0]);
        e->setParam(pScPos, 2.f); render(e, SR / 10);
        CHECK(std::fabs(tel->macro[2] - 0.33f) < 0.01f, "an empty scene keeps the knob value (%.3f)", tel->macro[2]);
        e->setParam(pScGlide, 2.f); e->setParam(pScPos, 1.f); render(e, SR / 10);
        CHECK(tel->scenePos > 1.05f && tel->scenePos < 1.95f, "scene glide moves gradually (%.2f)", tel->scenePos);
        delete e;
    }
    // ---- motion
    {
        std::unique_ptr<drift::MotionData> md(new drift::MotionData());
        md->seconds = 2.f; md->frames = 100; md->has[2] = true;
        for (int i = 0; i < 100; ++i) md->v[2][i] = (float)i / 100.f;
        const std::string txt = drift::motionToString(*md);
        std::unique_ptr<drift::MotionData> back(new drift::MotionData());
        drift::motionFromString(txt.c_str(), *back);
        CHECK(back->frames == 100 && back->has[2] && !back->has[0] && std::fabs(back->v[2][50] - 0.5f) < 1e-3f, "motion text round trip (%zu chars)", txt.size());
        drift::IEngine* e = fresh(); e->setTelemetry(tel.get());
        e->setMotion(*back);
        e->setParam((pMacro1 + 2), 0.9f);
        render(e, SR / 10);
        CHECK(std::fabs(tel->macro[2] - 0.9f) < 1e-3f, "playback Off: the knob value (%.2f)", tel->macro[2]);
        e->setParam(pMoMode, 1.f);
        render(e, SR);   // ~1 s into the loop
        CHECK(std::fabs(tel->macro[2] - 0.5f) < 0.06f && tel->motionOn[2] == 1 && tel->motionOn[0] == 0, "Loop: macro 3 follows its lane (%.2f at ~1 s)", tel->macro[2]);
        render(e, SR * 3 / 2);
        CHECK(std::fabs(tel->macro[2] - 0.25f) < 0.06f, "Loop wraps at 2 s (%.2f at ~2.5 s)", tel->macro[2]);
        e->setParam(pMoSpeed, 2.f); render(e, SR / 4);
        CHECK(std::fabs(tel->macro[2] - 0.5f) < 0.07f, "speed 2x (%.2f)", tel->macro[2]);
        back->recording = true; e->setMotion(*back); render(e, SR / 10);
        CHECK(std::fabs(tel->macro[2] - 0.9f) < 1e-3f, "recording pauses playback (%.2f)", tel->macro[2]);
        back->recording = false; e->setMotion(*back);
        e->setParam(pMoMode, 3.f); e->setParam(pMoSpeed, 1.f); e->setTransport(120, true, 1.0);   // beat 1 = 0.5 s
        render(e, 1024);
        CHECK(std::fabs(tel->macro[2] - 0.25f) < 0.05f, "Host: song position 0.5 s -> %.2f", tel->macro[2]);
        e->setParam(pMoMode, 2.f); e->setTransport(120, false, 0);
        render(e, SR / 2); note(e, 60, 100, true); render(e, SR / 5);
        CHECK(tel->macro[2] < 0.15f, "Note: a new phrase restarts the lane (%.2f)", tel->macro[2]);
        delete e;
    }
    // ---- physical models
    {
        struct Case { int model, exc; const char* name; bool pitch; };
        const Case cases[] = {
            { pmString, 6, "String + Pluck", true }, { pmString, 7, "String + Mallet", true }, { pmBar, 7, "Bar + Mallet", false },
            { pmWaveguide, 6, "Waveguide + Pluck", true }, { pmTanpura, 6, "Tanpura + Pluck", false }, { pmSympathetic, 1, "Sympathetic + Bow", false },
            { pmReed, 2, "Reed + Breath", true }, { pmFlute, 2, "Flute + Breath", true }, { pmGong, 7, "Gong + Mallet", false },
            { pmAnalysed, 0, "Analysed + Strike", true }, { pmBowl, 8, "Bowl + Sample (none loaded)", false }, { pmPlate, 0, "Plate + Strike", false } };
        for (const Case& c : cases) {
            drift::IEngine* e = fresh();
            e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)c.model); e->setParam(oscP(0, ofPmExciter), (float)c.exc);
            e->setParam(oscP(1, ofOn), 0.f); e->setParam(pF1Type, 0.f); e->setParam(pF2Type, 0.f);
            if (c.model == pmAnalysed) { drift::ModalData md; md.count = 4; const float r[4] = { 1, 2, 3, 4 }; for (int i = 0; i < 4; ++i) { md.ratio[i] = r[i]; md.amp[i] = 1.f / (i + 1); md.decay[i] = 4.f; } e->setModes(0, md); }
            note(e, 60, 110, true);
            std::vector<float> L; render(e, SR * 2, &L);
            const double r = rmsOf(L, SR / 2, SR * 2);
            bool ok = finiteAll(L) && peakOf(L) < 4.f && (c.exc == 8 || r > 3e-4);
            if (c.exc == 8) ok = ok && peakOf(L) < 1e-3f;   // no sample -> silent, not noise
            char extra[64] = "";
            if (c.pitch && ok) {
                const double f = pitchOf(L, SR / 2, 4096, 100, 900);
                const double cc = cents(f, 261.6256);
                std::snprintf(extra, sizeof extra, ", %.1f Hz (%+.0f ct)", f, cc);
                const double tol = (c.model == pmReed || c.model == pmFlute) ? 30.0 : 12.0;
                ok = ok && std::fabs(cc) < tol;
            }
            CHECK(ok, "%-28s rms %.4f peak %.2f%s", c.name, r, peakOf(L), extra);
            delete e;
        }
        // reed / flute pitch across the range
        for (int model : { (int)pmReed, (int)pmFlute }) {
            double worst = 0; int worstN = 0;
            for (int n : { 48, 55, 67, 74, 79 }) {
                drift::IEngine* e = fresh();
                e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)model); e->setParam(oscP(0, ofPmExciter), 2.f);
                e->setParam(oscP(1, ofOn), 0.f); e->setParam(pF1Type, 0.f);
                note(e, n, 110, true);
                std::vector<float> L; render(e, SR, &L);
                const double ref = 440.0 * std::pow(2.0, (n - 69) / 12.0);
                const double f = pitchOf(L, SR / 2, 4096, ref * 0.7, ref * 1.4);
                const double cc = f > 0 ? cents(f, ref) : 9999;
                if (std::fabs(cc) > std::fabs(worst)) { worst = cc; worstN = n; }
                delete e;
            }
            CHECK(std::fabs(worst) < 30.0, "%s tracks pitch over 3 octaves (worst %+.0f ct at note %d)", model == pmReed ? "Reed" : "Flute", worst, worstN);
        }
        // pressure: no breath, no tone (reed needs pressure above the threshold)
        {
            drift::IEngine* e = fresh();
            e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)pmReed); e->setParam(oscP(0, ofPmExciter), 2.f);
            e->setParam(oscP(1, ofOn), 0.f); e->setParam(osc2P(0, o2Pressure), 0.f);
            note(e, 60, 110, true);
            std::vector<float> L; render(e, SR, &L);
            const double lo = rmsOf(L, SR / 2, SR);
            delete e;
            e = fresh();
            e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)pmReed); e->setParam(oscP(0, ofPmExciter), 2.f);
            e->setParam(oscP(1, ofOn), 0.f); e->setParam(osc2P(0, o2Pressure), 0.7f);
            note(e, 60, 110, true);
            std::vector<float> L2; render(e, SR, &L2);
            const double hi = rmsOf(L2, SR / 2, SR);
            delete e;
            CHECK(hi > lo * 4, "Reed pressure: %.4f at 0, %.4f at 0.7", lo, hi);
        }
        // gong: nonlinearity pushes energy upward
        {
            auto hiShare = [&](float nl) {
                drift::IEngine* e = fresh();
                e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)pmGong); e->setParam(oscP(0, ofPmExciter), 7.f);
                e->setParam(oscP(1, ofOn), 0.f); e->setParam(pF1Type, 0.f); e->setParam(osc2P(0, o2Nonlin), nl); e->setParam(pQuality, 2.f);
                e->setParam(oscP(0, ofPmDamp), 0.1f); e->setParam(oscP(0, ofPmDecay), 8.f);
                note(e, 40, 127, true);
                std::vector<float> L; render(e, SR * 2, &L);
                delete e;
                double hi = 0, all = 0;
                for (double f = 100; f < 6000; f *= 1.03) { const double g = goertzel(L, SR / 10, SR, f); all += g * g; if (f > 1500) hi += g * g; }
                return hi / (all + 1e-20);
            };
            const double a = hiShare(0.f), b = hiShare(1.f);
            CHECK(b > a * 1.15, "Gong nonlinearity: energy above 1.5 kHz %.1f %% -> %.1f %%", a * 100, b * 100);
        }
        // sample exciter: a loaded sample drives the bowl
        {
            drift::IEngine* e = fresh();
            e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)pmBowl); e->setParam(oscP(0, ofPmExciter), 8.f);
            e->setParam(oscP(1, ofOn), 0.f);
            drift::SampleBuf* sb = new drift::SampleBuf();
            sb->len = SR; sb->sr = (float)SR; sb->data = new float[(size_t)SR + 4];
            uint32_t r = 7; for (int i = 0; i < SR + 4; ++i) { r = r * 1664525u + 1013904223u; sb->data[i] = ((r >> 9) / 4194304.f - 1.f) * 0.3f; }
            e->postSample(0, sb);
            render(e, 256);
            note(e, 60, 110, true);
            std::vector<float> L; render(e, SR * 2, &L);
            CHECK(rmsOf(L, SR, SR * 2) > 1e-3 && finiteAll(L), "Sample exciter drives the bowl (rms %.4f)", rmsOf(L, SR, SR * 2));
            delete e;
        }
    }
    // ---- analysis: WAV -> modes
    {
        const int n = SR * 2;
        std::vector<float> x((size_t)n);
        const double f[3] = { 200, 523, 977 }, t60[3] = { 4, 2, 1 }, a[3] = { 0.5, 0.3, 0.2 };
        for (int i = 0; i < n; ++i) { double v = 0; const double t = (double)i / SR; for (int k = 0; k < 3; ++k) v += a[k] * std::exp(-6.9078 * t / t60[k]) * std::sin(2 * M_PI * f[k] * t); x[(size_t)i] = (float)v; }
        drift::ana::writeWav("/tmp/claude-0/v42_bell.wav", x, 1, SR);
        drift::ModalData md; drift::ana::Info info;
        const bool ok = drift::ana::toModes("/tmp/claude-0/v42_bell.wav", md, info);
        bool good = ok && md.count >= 3;
        std::string desc;
        for (int i = 0; i < md.count && i < 4; ++i) { char b[64]; std::snprintf(b, sizeof b, " %.3f/%.2f/%.2fs", md.ratio[i], md.amp[i], md.decay[i]); desc += b; }
        if (good) {
            good = std::fabs(info.f0 - 200.f) < 1.f && std::fabs(md.ratio[1] - 2.615f) < 0.02f && std::fabs(md.ratio[2] - 4.885f) < 0.03f;
            for (int k = 0; k < 3 && good; ++k) good = std::fabs(md.decay[k] - (float)t60[k]) < 0.3f * (float)t60[k];
        }
        CHECK(good, "toModes: f0 %.1f Hz, %d modes:%s (%s)", info.f0, md.count, desc.c_str(), info.err.c_str());
        const std::string txt = drift::modalToString(md);
        drift::ModalData back; drift::modalFromString(txt.c_str(), back);
        CHECK(back.count == md.count && std::fabs(back.ratio[1] - md.ratio[1]) < 1e-3f, "modal text round trip");
    }
    // ---- Resonator
    {
        drift::IEngine* e = fresh();
        e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)pmBar); e->setParam(oscP(0, ofPmExciter), 0.f);
        e->setParam(oscP(0, ofPmDecay), 0.1f); e->setParam(oscP(1, ofOn), 0.f);
        e->setParam(pAEnvR, 0.01f);
        e->setParam(pRsOn, 1.f); e->setParam(pRsMix, 1.f); e->setParam(pRsDecay, 6.f); e->setParam(pRsBright, 0.9f);
        e->setParam(pRsMode, 0.f); e->setParam(pRsRoot, 0.f); e->setParam(pRsOct, 2.f);
        note(e, 72, 127, true); render(e, SR / 20); note(e, 72, 0, false);
        std::vector<float> L; render(e, SR * 2, &L);
        const double on = goertzel(L, SR / 2, SR * 2, 65.406), off = goertzel(L, SR / 2, SR * 2, 73.0);
        const double tail = rmsOf(L, SR, SR * 2);
        CHECK(finiteAll(L) && tail > 1e-4 && on > off * 4, "Resonator rings on after the note (tail rms %.4f), C2 string %.1fx the level between strings", tail, on / (off + 1e-20));
        delete e;
        // held notes mode follows the chord
        e = fresh();
        e->setParam(oscP(1, ofOn), 0.f); e->setParam(oscP(0, ofOn), 0.f); e->setParam(pNoiseLevel, 1.f); e->setParam(pF1Type, 0.f);
        e->setParam(pRsOn, 1.f); e->setParam(pRsMix, 1.f); e->setParam(pRsMode, 5.f); e->setParam(pRsOct, 2.f); e->setParam(pRsDecay, 8.f);
        note(e, 57, 100, true);   // A3 -> strings on A (folded into C2..C6)
        render(e, SR / 2); note(e, 57, 0, false);
        std::vector<float> L2; render(e, SR, &L2);
        const double a = goertzel(L2, SR / 4, SR, 110.0), b = goertzel(L2, SR / 4, SR, 98.0);
        CHECK(a > b * 3, "Held Notes: strings tuned to the held A (110 Hz %.1fx the level at 98 Hz)", a / (b + 1e-20));
        delete e;
    }
    // ---- Space
    {
        drift::IEngine* e = fresh();
        e->setParam(oscP(1, ofOn), 0.f); e->setParam(oscP(0, ofType), (float)otAnalog);
        e->setParam(pAEnvR, 0.005f);
        e->setParam(pSpOn, 1.f); e->setParam(pSpMix, 1.f); e->setParam(pSpDecay, 2.f); e->setParam(pSpPre, 0.f);
        note(e, 60, 110, true); render(e, SR / 10); note(e, 60, 0, false);
        std::vector<float> L; render(e, SR * 3, &L);
        const double a = rmsOf(L, SR / 2, SR * 6 / 10), b = rmsOf(L, SR * 3 / 2, SR * 16 / 10);
        const double db = 20 * std::log10(a / (b + 1e-20));
        CHECK(finiteAll(L) && db > 22 && db < 40, "Space decay 2 s: %.1f dB lost over 1 s (expect ~30)", db);
        delete e;
        e = fresh();
        e->setParam(oscP(1, ofOn), 0.f); e->setParam(pNoiseLevel, 1.f); e->setParam(oscP(0, ofOn), 0.f);
        e->setParam(pSpOn, 1.f); e->setParam(pSpMix, 1.f); e->setParam(pSpDecay, 60.f); e->setParam(pSpSize, 1.f); e->setParam(pSpDamp, 0.f);
        note(e, 60, 127, true);
        std::vector<float> L2; render(e, SR * 10, &L2);
        CHECK(finiteAll(L2) && peakOf(L2) < 4.f, "Space at 60 s decay with constant noise stays bounded (peak %.2f)", peakOf(L2));
        delete e;
    }
    // ---- multicore: identical output for any thread count (incl. voice steals)
    {
        auto run = [&](drift::IWorkerPool* pool, std::vector<float>& L, std::vector<float>& R) {
            drift::IEngine* e = fresh();
            e->setWorkerPool(pool);
            e->setParam(pPolyphony, 4.f);
            e->setParam(oscP(0, ofVoices), 5.f); e->setParam(pNoiseLevel, 0.2f); e->setParam(pDrift, 0.5f);
            e->setParam(oscP(2, ofOn), 1.f); e->setParam(oscP(2, ofType), (float)otPhysical); e->setParam(oscP(2, ofPmModel), (float)pmSympathetic);
            e->setParam(pAEnvR, 0.3f);
            const int notes[6] = { 48, 55, 60, 64, 67, 71 };
            for (int k = 0; k < 6; ++k) { note(e, notes[k], 90, true); render(e, SR / 8, &L, &R); }
            for (int k = 0; k < 6; ++k) note(e, notes[k], 0, false);
            render(e, SR / 2, &L, &R);
            delete e;
        };
        std::vector<float> L1, R1, L2, R2, L4, R4;
        run(nullptr, L1, R1);
        drift::WorkerPool p2(1), p4(3);
        run(&p2, L2, R2);
        run(&p4, L4, R4);
        const bool same2 = L1.size() == L2.size() && std::memcmp(L1.data(), L2.data(), L1.size() * 4) == 0 && std::memcmp(R1.data(), R2.data(), R1.size() * 4) == 0;
        const bool same4 = L1.size() == L4.size() && std::memcmp(L1.data(), L4.data(), L1.size() * 4) == 0 && std::memcmp(R1.data(), R4.data(), R1.size() * 4) == 0;
        CHECK(same2 && same4 && rmsOf(L1, 0, L1.size()) > 1e-3, "1, 2 and 4 threads give bit-identical output (%zu samples, 6 notes on 4 voices)", L1.size());
    }
    std::printf(bad ? "\n%d FAILURES\n" : "\nall good\n", bad);
    return bad ? 1 : 0;
}
