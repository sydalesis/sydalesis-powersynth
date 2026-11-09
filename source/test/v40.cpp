// Driftfield 4.0 phase A checks: tuning tables, Scala parsing, engine tuning, note expression.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Tuning.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { ++bad; std::printf("FAIL: "); } else std::printf("ok:   "); std::printf(__VA_ARGS__); std::printf("\n"); } while (0)
static void note(drift::IEngine* e, int ch, int n, bool on) { uint8_t m[3] = { (uint8_t)((on ? 0x90 : 0x80) | ch), (uint8_t)n, 100 }; e->midi(m, 3); }
static void ne(drift::IEngine* e, int type, int ch, int key, double v) { uint8_t m[12] = { 0xF4, (uint8_t)type, (uint8_t)ch, (uint8_t)key }; std::memcpy(m + 4, &v, 8); e->midi(m, 12); }
static drift::IEngine* fresh(bool generic) {
    drift::IEngine* e = drift::createEngine(generic);
    e->setSampleRate(48000); e->setTransport(120, false, 0);
    for (uint32_t i = 0; i < kParamCount; ++i) e->setParam(i, P(i).def);
    // pure sine voice, no FX, no drift
    e->setParam(oscP(0, ofType), (float)otAnalog); e->setParam(oscP(0, ofAnWave), 4.f); e->setParam(oscP(0, ofVoices), 1.f);
    e->setParam(oscP(0, ofAnSlop), 0.f); e->setParam(oscP(0, ofAnDrift), 0.f);
    e->setParam(oscP(1, ofOn), 0.f); e->setParam(oscP(2, ofOn), 0.f);
    e->setParam(pF1Type, 0); e->setParam(pF2Type, 0); e->setParam(pF1Drive, 0); e->setParam(pF2Drive, 0);
    e->setParam(ParamRegistry::get().findSymbol("chOn"), 0);
    e->setParam(pAEnvA, 0.001f); e->setParam(pAEnvS, 1.f);
    e->setParam(pAEnvR, 0.02f);
    for (int sl = 0; sl < kModSlots; ++sl) e->setParam(pM1Src + sl * kSlotFields + sfAmt, 0.f);
    return e;
}
// frequency of a pure tone by zero crossings with linear interpolation
static double measureHz(drift::IEngine* e, int frames, double* rms = nullptr) {
    std::vector<float> L(frames), R(frames);
    for (int pos = 0; pos < frames; pos += 256) e->process(&L[pos], &R[pos], std::min(256, frames - pos));
    const int s0 = frames / 4;
    double first = -1, last = -1; int n = 0; double acc = 0;
    for (int i = s0 + 1; i < frames; ++i) {
        acc += (double)L[i] * L[i];
        if (L[i - 1] < 0.f && L[i] >= 0.f) {
            const double t = (i - 1) + (double)(-L[i - 1]) / (L[i] - L[i - 1]);
            if (first < 0) first = t; else ++n;
            last = t;
        }
    }
    if (rms) *rms = std::sqrt(acc / (frames - s0));
    return n > 0 ? n * 48000.0 / (last - first) : 0.0;
}
static double hzOf(double semis) { return 440.0 * std::pow(2.0, (semis - 69.0) / 12.0); }

int main() {
    const drift::ScaleDef* lib = drift::builtinScales();
    std::printf("%d built-in scales\n", drift::kBuiltinScales);
    for (int i = 0; i < drift::kBuiltinScales; ++i) {
        const auto& s = lib[i];
        bool mono = true; for (int k = 1; k <= s.n; ++k) if (s.c[k] <= s.c[k - 1] && !s.twelve) mono = false;
        if (s.c[s.n] <= s.c[s.n - 1] && s.n > 1) mono = false;
        CHECK(s.n > 0 && mono && (!s.twelve || s.n == 12), "%-22s n=%-3d period %.2f%s", s.name, s.n, s.c[s.n], s.twelve ? " (12-key)" : "");
    }
    float t[128]; bool mem[128];
    drift::buildTuningTable(lib[0], nullptr, 0, 440.0, false, t, mem);
    CHECK(std::fabs(t[60] - 60.f) < 1e-4f && std::fabs(t[64] - 63.8631f) < 1e-3f && std::fabs(t[67] - 67.0196f) < 1e-3f && std::fabs(t[72] - 72.f) < 1e-4f,
          "JI major on C: C4 %.4f E4 %.4f G4 %.4f C5 %.4f", t[60], t[64], t[67], t[72]);
    drift::buildTuningTable(lib[0], nullptr, 2, 432.0, false, t);
    CHECK(std::fabs(t[62] - (62.f + 12.f * std::log2(432.f / 440.f))) < 1e-3f && std::fabs(t[69] - (62.f + 12.f * std::log2(432.f / 440.f) + 7.01955f)) < 1e-3f,
          "JI major on D, A=432: D4 %.4f A4 %.4f", t[62], t[69]);
    drift::buildTuningTable(lib[15], nullptr, 0, 440.0, true, t, mem);   // Bhupali, snap
    CHECK(!mem[61] && t[61] == t[60] && mem[62] && t[66] == t[67] && t[65] == t[64], "Bhupali snap: C# -> C, F# -> G, F -> E");
    drift::buildTuningTable(lib[18], nullptr, 9, 440.0, false, t);       // Slendro on A
    CHECK(std::fabs(t[69] - 69.f) < 1e-4f && std::fabs(t[74] - 81.f) < 1e-4f && std::fabs(t[70] - 71.31f) < 1e-3f, "Slendro linear: 5 keys per octave (A4 %.2f, +5 keys %.2f)", t[69], t[74]);

    // Scala parsing
    const char* scl = "! test.scl\n!\nJust test scale\n 5\n!\n 9/8\n 5/4 the third\n 701.955\n 5/3\n 2\n";
    drift::ScaleDef sc; std::string err;
    CHECK(drift::parseScl(scl, sc, &err) && sc.n == 5 && std::fabs(sc.c[3] - 701.955) < 1e-6 && std::fabs(sc.c[5] - 1200.0) < 1e-6, "parse .scl (%s) n=%d", err.c_str(), sc.n);
    CHECK(!drift::parseScl("bad\n3\n9/8\n", sc, &err), "rejects short .scl: %s", err.c_str());
    const char* kbm = "! map\n12\n0\n127\n60\n69\n432.0\n5\n0\nx\n1\nx\n2\n3\nx\n4\nx\n5\nx\nx\n";
    drift::KbmDef km; drift::ScaleDef sc5; drift::parseScl(scl, sc5);
    CHECK(drift::parseKbm(kbm, km, &err) && km.size == 12 && km.map[1] == -1 && km.map[2] == 1 && std::fabs(km.refFreq - 432.0) < 1e-9, "parse .kbm (%s)", err.c_str());
    drift::buildTuningTable(sc5, &km, 0, 440.0, false, t, mem);
    CHECK(std::fabs(t[69] - (69.f + 12.f * std::log2(432.f / 440.f))) < 1e-3f && !mem[61] && mem[62], "kbm reference: key 69 = 432 Hz (%.3f Hz)", hzOf(t[69]));
    CHECK(drift::tnUnescape(drift::tnEscape(scl)) == scl && drift::tnEscape(scl).find('\n') == std::string::npos, "scala text escaping round trip");

    // engine
    for (int isa = 0; isa < 2; ++isa) {
        const char* nm = isa ? "avx2" : "generic";
        drift::IEngine* e = fresh(isa == 0);
        note(e, 0, 64, true);
        double hz = measureHz(e, 48000);
        CHECK(std::fabs(hz - hzOf(64)) < 0.3, "%s equal E4 %.2f Hz (expect %.2f)", nm, hz, hzOf(64));
        note(e, 0, 64, false); measureHz(e, 9600);
        e->setParam(pTnSource, 1); e->setParam(pTnScale, 0); e->setParam(pTnRoot, 0);
        note(e, 0, 64, true);
        hz = measureHz(e, 48000);
        CHECK(std::fabs(hz - hzOf(63.8631)) < 0.3, "%s JI E4 %.2f Hz (expect %.2f, 5/4 of C)", nm, hz, hzOf(63.8631));
        // live root change: held E4 becomes the root of D-major JI? (E is the 9/8 of D) -> glides
        e->setParam(pTnRoot, 2);
        hz = measureHz(e, 48000);
        CHECK(std::fabs(hz - hzOf(62.0 + 2.0391)) < 0.3, "%s held E4 retunes to 9/8 over D: %.2f Hz", nm, hz);
        e->setParam(pTnAmount, 0.f);
        hz = measureHz(e, 24000);
        CHECK(std::fabs(hz - hzOf(64)) < 0.3, "%s amount 0 = equal: %.2f Hz", nm, hz);
        note(e, 0, 64, false); measureHz(e, 9600);
        // Scala source
        e->setParam(pTnAmount, 1.f); e->setParam(pTnSource, 2); e->setParam(pTnRoot, 0);
        e->setUserScale(&sc5, nullptr);
        note(e, 0, 62, true);   // 2 keys above C in a 5-note scale = 5/4
        hz = measureHz(e, 48000);
        CHECK(std::fabs(hz - hzOf(60 + 3.8631)) < 0.3, "%s Scala key D4 = degree 2 (5/4): %.2f Hz", nm, hz);
        note(e, 0, 62, false); measureHz(e, 9600);
        // note expression
        e->setParam(pTnSource, 0);
        note(e, 0, 57, true);
        double r0; measureHz(e, 48000, &r0);
        ne(e, 0, 0, 57, 0.125);                       // -6 dB
        double r1; measureHz(e, 48000, &r1);
        ne(e, 2, 0, 57, 0.5 + 2.0 / 240.0);          // +2 semitones
        hz = measureHz(e, 48000);
        CHECK(std::fabs(hz - hzOf(59)) < 0.3, "%s note-expression tuning +2 st: %.2f Hz", nm, hz);
        CHECK(std::fabs(20 * std::log10(r1 / r0) + 6.02) < 0.3, "%s note-expression volume -6 dB: %+.2f dB", nm, 20 * std::log10(r1 / r0));
        note(e, 0, 57, false); measureHz(e, 9600);
        note(e, 0, 57, true);   // new note on the same key starts with neutral expression
        hz = measureHz(e, 24000);
        CHECK(std::fabs(hz - hzOf(57)) < 0.3, "%s next note on the key resets expression: %.2f Hz", nm, hz);
        delete e;
    }
    std::printf(bad ? "\n%d FAILURES\n" : "\nall good\n", bad);
    return bad ? 1 : 0;
}
