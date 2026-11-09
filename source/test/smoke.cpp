// Driftfield 3 smoke test: every engine, filter, generator and quality mode, both ISA builds.
#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>

static int bad = 0;
static float render(drift::IEngine* e, int frames, double* rmsOut = nullptr) {
    std::vector<float> L(frames), R(frames);
    double acc = 0; float pk = 0;
    int pos = 0;
    while (pos < frames) { int c = std::min(512, frames - pos); e->process(&L[pos], &R[pos], c); pos += c; }
    for (int i = 0; i < frames; ++i) {
        if (!std::isfinite(L[i]) || !std::isfinite(R[i])) { ++bad; return -1; }
        acc += L[i] * L[i]; pk = std::max(pk, std::fabs(L[i]));
    }
    if (rmsOut) *rmsOut = std::sqrt(acc / frames);
    return pk;
}
static void note(drift::IEngine* e, int n, bool on) { uint8_t m[3] = { (uint8_t)(on ? 0x90 : 0x80), (uint8_t)n, 100 }; e->midi(m, 3); }
static void sp(drift::IEngine* e, const char* sym, float v) { int id = ParamRegistry::get().findSymbol(sym); if (id < 0) { printf("no sym %s\n", sym); ++bad; return; } e->setParam(id, v); }

int main() {
    printf("params %u\n", (unsigned)kParamCount);
    if (ParamRegistry::get().count() != kParamCount) { printf("REGISTRY MISMATCH %u\n", ParamRegistry::get().count()); return 1; }
    drift::Telemetry tel{};
    for (int isa = 0; isa < 2; ++isa) {
        drift::IEngine* e = drift::createEngine(isa == 0);
        e->setSampleRate(48000); e->setTelemetry(&tel);
        e->setTransport(120, false, 0);
        printf("== %s\n", e->isaName());
        const char* types[5] = { "wt", "analog", "add", "phys", "gran" };
        for (int t = 0; t < 5; ++t) {
            for (int o = 0; o < 3; ++o) { e->setParam(oscP(o, ofOn), o == 0 ? 1.f : 0.f); e->setParam(oscP(o, ofType), (float)t); }
            int sub = t == 3 ? kPmModels : (t == 1 ? kAnWaves : (t == 0 ? kWarpModes : (t == 4 ? 3 : 4)));
            for (int k = 0; k < sub; ++k) {
                if (t == 0) { e->setParam(oscP(0, ofWarp), (float)k); e->setParam(oscP(0, ofWarpAmt), 0.6f); e->setParam(oscP(0, ofWtBlend), (float)(k % 3)); }
                if (t == 1) { e->setParam(oscP(0, ofAnWave), (float)k); e->setParam(oscP(0, ofVoices), 3); }
                if (t == 2) { e->setParam(oscP(0, ofAdVowelAmt), 0.7f); e->setParam(oscP(0, ofAdFreeze), (float)(k & 1)); e->setParam(oscP(0, ofAdBlur), k * 0.5f); e->setParam(oscP(0, ofAdSpec1), (float)(11 + k)); }
                if (t == 3) { e->setParam(oscP(0, ofPmModel), (float)k); e->setParam(oscP(0, ofPmExciter), (float)(k % kPmExciters)); e->setParam(oscP(0, ofPmCouple), 0.5f); }
                if (t == 4) { e->setParam(oscP(0, ofGrSource), (float)k); }
                note(e, 48, true); note(e, 55, true);
                double rms; float pk = render(e, 48000, &rms);
                note(e, 48, false); note(e, 55, false); render(e, 4800);
                printf("  %-6s %d  peak %.3f rms %.4f %s\n", types[t], k, pk, rms, (rms < 1e-4 || pk > 4) ? "<-- CHECK" : "");
            }
        }
        // FM / sync matrix
        e->setParam(oscP(0, ofType), 0); e->setParam(oscP(1, ofOn), 1); e->setParam(oscP(1, ofType), 1);
        for (int m = 0; m < 4; ++m) { e->setParam(oscP(0, ofFmSrc), 2); e->setParam(oscP(0, ofFmMode), (float)m); e->setParam(oscP(0, ofFmAmt), 0.7f);
            e->setParam(oscP(1, ofSyncSrc), 1); note(e, 50, true); float pk = render(e, 24000); note(e, 50, false); render(e, 4800); printf("  fm mode %d peak %.3f\n", m, pk); }
        e->setParam(oscP(0, ofFmAmt), 0); e->setParam(oscP(1, ofSyncSrc), 0);
        // filters
        for (int f = 0; f < kFilterTypes; ++f) {
            sp(e, "f1Type", (float)f); sp(e, "f1Type2", (float)((f + 7) % kFilterTypes)); sp(e, "f1Morph", 0.4f); sp(e, "f1Res", 0.8f);
            sp(e, "f1FmSrc", 1); sp(e, "f1FmAmt", 0.3f); sp(e, "f1DriveType", (float)(f % 4)); sp(e, "f1Drive", 0.5f);
            note(e, 45, true); float pk = render(e, 12000); note(e, 45, false); render(e, 2400);
            if (pk < 0 || pk > 6) printf("  filter %s peak %.3f <-- CHECK\n", kFilterNames[f], pk);
        }
        sp(e, "f1Type", 1); sp(e, "f1Morph", 0); sp(e, "f1FmAmt", 0);
        // modulation everywhere
        for (int s = 0; s < kModSlots; ++s) { e->setParam(modSrcParam(s), (float)(1 + s % (kModSrcCount - 1))); e->setParam(modDstParam(s), (float)(1 + (s * 3) % (kModDstCount - 1)));
            e->setParam(modAmtParam(s), 0.5f); e->setParam(modViaParam(s), (float)(s % 5)); e->setParam(modCurveParam(s), (float)(s % 7)); }
        for (int l = 0; l < kLfos; ++l) { e->setParam(lfoP(l, lfWave), (float)(l + 2)); e->setParam(lfoP(l, lfMode), (float)(l % 4)); e->setParam(lfoP(l, lfSync), (float)(l & 1)); }
        // FX all on
        for (int u = 0; u < kFxUnits; ++u) e->setParam(kFxOnParam[u], 1);
        int ord[9] = { 8, 7, 6, 5, 4, 3, 2, 1, 0 }; e->setFxOrder(ord);
        drift::MsegData md; drift::msegDefault(md, 1); e->setMseg(0, md);
        note(e, 60, true); note(e, 64, true); { double r; float pk = render(e, 96000, &r); printf("  full mod+fx peak %.3f rms %.4f\n", pk, r); }
        note(e, 60, false); note(e, 64, false); render(e, 9600);
        // arp + seq with transport
        sp(e, "arpOn", 1); sp(e, "arpMode", 2); sp(e, "arpOct", 2);
        e->setTransport(128, true, 0);
        note(e, 60, true); note(e, 63, true); note(e, 67, true);
        { float pk = render(e, 48000); printf("  arp peak %.3f arpStep %d\n", pk, tel.arpStep); }
        note(e, 60, false); note(e, 63, false); note(e, 67, false); render(e, 4800);
        sp(e, "arpOn", 0); sp(e, "seqMode", 2);
        for (int st = 0; st < 16; ++st) { sp(e, ("seqV" + std::to_string(st + 1)).c_str(), (st % 5) * 0.1f - 0.2f); }
        note(e, 48, true); { float pk = render(e, 48000); printf("  seq notes peak %.3f step %d\n", pk, tel.seqStep); } note(e, 48, false); render(e, 4800);
        sp(e, "seqMode", 1);
        // HQ
        sp(e, "quality", 2); note(e, 52, true); { double r; float pk = render(e, 48000, &r); printf("  HQ peak %.3f rms %.4f\n", pk, r); } note(e, 52, false); render(e, 9600);
        sp(e, "quality", 0); note(e, 52, true); { float pk = render(e, 24000); printf("  Eco peak %.3f\n", pk); } note(e, 52, false);
        printf("  telemetry seq %u active %d\n", tel.seq, tel.activeCount);
        delete e;
    }
    printf("%s (%d bad)\n", bad ? "FAILED" : "OK", bad);
    return bad ? 1 : 0;
}
