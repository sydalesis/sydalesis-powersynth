// Driftfield 5.1.1 checks: no voice can be stranded by a preset change while keys are down, two keyboards on
// one channel, CC 120 / reset as panic.
#include "../EngineIface.hpp"
#include "../Params.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>

static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)

static const int SR = 48000;
static int sym(const char* s) { return ParamRegistry::get().findSymbol(s); }
static drift::IEngine* fresh() {
    drift::IEngine* e = drift::createEngine();
    e->setSampleRate((float)SR);
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    e->setParam(sym("aeA"), 0.002f); e->setParam(sym("aeR"), 0.05f); e->setParam(sym("aeS"), 1.f);
    e->setParam(sym("quality"), 1.f);
    return e;
}
static void render(drift::IEngine* e, int n, double* rms = nullptr) {
    std::vector<float> L(256), R(256); double acc = 0; int c = 0;
    while (n > 0) { const int k = n < 256 ? n : 256; e->process(L.data(), R.data(), (uint32_t)k); for (int i = 0; i < k; ++i) { acc += L[i] * L[i]; ++c; } n -= k; }
    if (rms) *rms = c ? std::sqrt(acc / c) : 0.0;
}
static void on(drift::IEngine* e, int ch, int n) { const uint8_t m[3] = { (uint8_t)(0x90 | ch), (uint8_t)n, 100 }; e->midi(m, 3); }
static void off(drift::IEngine* e, int ch, int n) { const uint8_t m[3] = { (uint8_t)(0x80 | ch), (uint8_t)n, 0 }; e->midi(m, 3); }
static void cc(drift::IEngine* e, int num, int v) { const uint8_t m[3] = { 0xB0, (uint8_t)num, (uint8_t)v }; e->midi(m, 3); }

int main() {
    {   // poly chord held, preset switches to mono, keys released -> silence
        drift::IEngine* e = fresh();
        on(e, 0, 48); on(e, 0, 55); on(e, 0, 60); render(e, SR / 4);
        const int before = e->activeVoices();
        e->setParam(pPolyMode, 1.f); render(e, 256);
        off(e, 0, 48); off(e, 0, 55); off(e, 0, 60); render(e, SR);
        CHECK(before == 3 && e->activeVoices() == 0, "poly chord across a switch to mono releases (%d voices before, %d left)", before, e->activeVoices());
        delete e;
    }
    {   // mono note held, preset switches to poly
        drift::IEngine* e = fresh(); e->setParam(pPolyMode, 2.f);
        on(e, 0, 48); on(e, 0, 52); render(e, SR / 4);
        e->setParam(pPolyMode, 0.f); render(e, 256);
        off(e, 0, 48); off(e, 0, 52); render(e, SR);
        CHECK(e->activeVoices() == 0, "legato notes across a switch to poly release (%d left)", e->activeVoices());
        delete e;
    }
    {   // note-sequencer playing, preset switches the sequencer to CV, key released
        drift::IEngine* e = fresh();
        e->setParam(pSeqMode, 2.f); e->setParam(sym("seqGate"), 1.f);
        e->setTransport(120.0, false, 0.0);
        on(e, 0, 48); render(e, SR / 3);
        e->setParam(pSeqMode, 1.f); render(e, 256);
        off(e, 0, 48); render(e, SR);
        CHECK(e->activeVoices() == 0, "sequencer note -> CV switch leaves nothing sounding (%d left)", e->activeVoices());
        delete e;
    }
    {   // arp HOLD latch: arp off, then another HOLD preset must not replay the old chord
        drift::IEngine* e = fresh();
        e->setParam(pArpOn, 1.f); e->setParam(pArpHold, 1.f);
        e->setTransport(120.0, false, 0.0);
        on(e, 0, 60); on(e, 0, 64); render(e, SR / 4); off(e, 0, 60); off(e, 0, 64); render(e, SR / 4);
        const int latched = e->activeVoices();
        e->setParam(pArpOn, 0.f); render(e, SR / 2);
        e->setParam(pArpOn, 1.f); render(e, SR);
        CHECK(latched > 0 && e->activeVoices() == 0, "latched arp chord is forgotten when the arp goes off (%d while latched, %d after)", latched, e->activeVoices());
        delete e;
    }
    {   // two keyboards, same channel, same key: it sounds until both are up
        drift::IEngine* e = fresh();
        on(e, 0, 57); render(e, SR / 8); on(e, 0, 57); render(e, SR / 8);
        off(e, 0, 57); render(e, SR / 2);
        double r1; render(e, SR / 8, &r1);
        const int still = e->activeVoices();
        off(e, 0, 57); render(e, SR);
        CHECK(still >= 1 && r1 > 1e-3 && e->activeVoices() == 0, "same key on two keyboards: sounds until both are released (%d voices / rms %.3f after the first release, %d after both)", still, r1, e->activeVoices());
        delete e;
    }
    {   // a lost note-off does not leave the key unreleasable: a fresh press of a silent key starts the count again
        drift::IEngine* e = fresh();
        on(e, 0, 62); render(e, SR / 8); on(e, 0, 62); render(e, SR / 8);     // count 2
        cc(e, 123, 0); render(e, SR);                                          // all notes off: count cleared
        on(e, 0, 62); render(e, SR / 8); off(e, 0, 62); render(e, SR);
        CHECK(e->activeVoices() == 0, "CC 123 clears the key counts (%d left)", e->activeVoices());
        delete e;
    }
    {   // panic = reset: everything silent at once, nothing comes back
        drift::IEngine* e = fresh(); e->setParam(sym("aeR"), 20.f);
        e->setParam(pArpOn, 1.f); e->setParam(pArpHold, 1.f); e->setTransport(120.0, false, 0.0);
        on(e, 0, 50); on(e, 0, 57); render(e, SR / 4); off(e, 0, 50); off(e, 0, 57);
        e->reset();
        double r; render(e, SR, &r);
        CHECK(e->activeVoices() == 0 && r < 1e-6, "panic silences a latched arp with a 20 s release (rms %.2g)", r);
        delete e;
    }
    {   // XY auto-motion: circle on pad 1 traces radius 0.25 (size 0.5) around the macro knobs; axes follow the PERFORM layout
        drift::IEngine* e = fresh();
        static drift::Telemetry t; e->setTelemetry(&t);
        e->setParam(pMacro1 + 4, 0.5f); e->setParam(pMacro1 + 6, 0.5f);
        const int ax[4] = { 4, 6, 2, 3 }; e->setPerfAxes(ax);
        e->setParam(pXy1Mode, 1.f); e->setParam(pXy1Rate, 1.f); e->setParam(pXy1Size, 0.5f);
        on(e, 0, 48);
        double rmin = 9, rmax = 0, xmin = 9, xmax = -9; float other = 0;
        for (int k = 0; k < 60; ++k) {
            render(e, SR / 60);
            const double dx = t.macro[4] - 0.5, dy = t.macro[6] - 0.5, rr = std::sqrt(dx * dx + dy * dy);
            rmin = std::min(rmin, rr); rmax = std::max(rmax, rr); xmin = std::min(xmin, (double)t.macro[4]); xmax = std::max(xmax, (double)t.macro[4]);
            other = std::max(other, std::fabs(t.macro[0]));
        }
        CHECK(rmin > 0.23 && rmax < 0.27 && xmin < 0.27 && xmax > 0.73 && other < 1e-6f, "circle on macros 5 / 7: radius %.3f..%.3f, x %.2f..%.2f, macro 1 untouched", rmin, rmax, xmin, xmax);
        delete e;
    }
    {   // modal decay follows the Decay knob, the same at Normal and HQ (fastExp2 error fixed)
        double drop[3] = {0, 0, 0};
        for (int q = 1; q <= 2; ++q) {
            drift::IEngine* e = fresh(); e->setParam(sym("quality"), (float)q);
            e->setParam(oscP(0, ofType), (float)otPhysical); e->setParam(oscP(0, ofPmModel), (float)pmAnalysed); e->setParam(oscP(0, ofPmExciter), 0.f);
            e->setParam(oscP(1, ofOn), 0.f); e->setParam(pF1Type, 0.f); e->setParam(pF2Type, 0.f); e->setParam(sym("aeR"), 0.05f);
            drift::ModalData md; md.count = 1; md.ratio[0] = 1; md.amp[0] = 1; md.decay[0] = 4; e->setModes(0, md);
            render(e, 256);
            on(e, 0, 60);
            double a, b; render(e, SR / 4); render(e, SR / 4, &a); render(e, SR * 3 / 4); render(e, SR / 4, &b);
            drop[q] = 20 * std::log10(b / a);
            delete e;
        }
        CHECK(std::fabs(drop[1] + 15) < 4 && std::fabs(drop[2] - drop[1]) < 1.5, "a 4 s mode drops %.1f dB (Normal) / %.1f dB (HQ) in 1 s (expected -15)", drop[1], drop[2]);
    }
    {   // 5.1.2 keyboard telemetry: note-expression tuning shows as bend; per-note pressure vs whole-keyboard aftertouch
        drift::IEngine* e = fresh(); static drift::Telemetry t; e->setTelemetry(&t);
        on(e, 3, 60);                                             // an MPE note alone on channel 4
        uint8_t ne[12] = { 0xF4, 2, 3, 60 }; double tune = 0.5 + 2.0 / 240.0; std::memcpy(ne + 4, &tune, 8); e->midi(ne, 12);   // +2 st
        const uint8_t cp3[2] = { 0xD3, 100 }; e->midi(cp3, 2);   // channel pressure on its own channel = per-note pressure
        on(e, 0, 48); on(e, 0, 52);                               // two notes on channel 1 (plain keyboard)
        const uint8_t cp0[2] = { 0xD0, 64 }; e->midi(cp0, 2);     // channel aftertouch = whole keyboard
        render(e, SR / 4);
        float b60 = 0, p60 = 0, a60 = 0, p48 = 0, a48 = 0;
        for (int i = 0; i < drift::kTelVoices; ++i) { const drift::TelVoice& v = t.v[i]; if (!v.active) continue;
            if (v.note == 60) { b60 = v.bend; p60 = v.notePress; a60 = v.chanAT; } if (v.note == 48) { p48 = v.notePress; a48 = v.chanAT; } }
        CHECK(std::fabs(b60 - 2.f) < 0.05f && p60 > 0.75f && a60 < 0.01f && p48 < 0.01f && std::fabs(a48 - 64.f / 127.f) < 0.02f,
              "note-expression bend %.2f st; MPE note: pressure %.2f / aftertouch %.2f; keyboard chord: pressure %.2f / aftertouch %.2f", b60, p60, a60, p48, a48);
        delete e;
    }
    std::printf(fails ? "%d FAILURES\n" : "all good\n", fails);
    return fails ? 1 : 0;
}
