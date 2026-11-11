// Driftfield 3 — data shared between plugin, engine builds and UI:
// MSEG / LFO-shape / FX-order formats, granular sample buffers, and the live telemetry block
// the engine fills for the interface (read through DPF direct access).
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <atomic>
#include <vector>
#include "Params.hpp"

namespace drift {

// ------------------------------------------------------------------------------------------------
// MSEG: up to 16 points. Point 0 sits at time 0; point i (i>0) is reached t[i] seconds after i-1.
// c[i] bends the segment arriving at point i (-1 .. 1). loopStart/loopEnd are point indices.
static constexpr int kMsegMaxPoints = 16;
struct MsegData {
    int count = 4;
    int loopStart = 1, loopEnd = 2;
    float t[kMsegMaxPoints] = { 0.f, 1.f, 1.f, 2.f };
    float l[kMsegMaxPoints] = { 0.f, 1.f, 0.6f, 0.f };
    float c[kMsegMaxPoints] = { 0.f, 0.f, 0.f, 0.f };
};
static inline void msegDefault(MsegData& m, int which) {
    m = MsegData();
    if (which == 1) {   // slow looping wave
        m.count = 5; m.loopStart = 0; m.loopEnd = 4;
        const float t[5] = { 0.f, 2.f, 1.5f, 2.5f, 2.f }, l[5] = { 0.f, 1.f, 0.35f, 0.8f, 0.f }, c[5] = { 0.f, 0.4f, -0.4f, 0.3f, -0.3f };
        for (int i = 0; i < 5; ++i) { m.t[i] = t[i]; m.l[i] = l[i]; m.c[i] = c[i]; }
    } else if (which == 2) {   // long swell: rises over 8 s, sways while held
        m.count = 5; m.loopStart = 2; m.loopEnd = 4;
        const float t[5] = { 0.f, 8.f, 3.f, 4.f, 5.f }, l[5] = { 0.f, 1.f, 0.7f, 0.95f, 0.7f }, c[5] = { 0.f, 0.2f, -0.3f, 0.3f, -0.3f };
        for (int i = 0; i < 5; ++i) { m.t[i] = t[i]; m.l[i] = l[i]; m.c[i] = c[i]; }
    } else if (which == 3) {   // slow stepped pattern
        m.count = 7; m.loopStart = 0; m.loopEnd = 6;
        const float t[7] = { 0.f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f }, l[7] = { 0.2f, 0.8f, 0.45f, 1.f, 0.1f, 0.6f, 0.2f }, c[7] = { 0.f, 0.9f, -0.9f, 0.9f, -0.9f, 0.9f, -0.9f };
        for (int i = 0; i < 7; ++i) { m.t[i] = t[i]; m.l[i] = l[i]; m.c[i] = c[i]; }
    }
}
static inline std::string msegToString(const MsegData& m) {
    std::string s;
    char b[64];
    std::snprintf(b, sizeof(b), "%d,%d", m.loopStart, m.loopEnd);
    s = b;
    for (int i = 0; i < m.count; ++i) { std::snprintf(b, sizeof(b), ";%.4g,%.4g,%.3g", m.t[i], m.l[i], m.c[i]); s += b; }
    return s;
}
static inline bool msegFromString(const char* str, MsegData& m) {
    if (!str || !*str) return false;
    MsegData r; r.count = 0;
    char* end = nullptr;
    r.loopStart = (int)std::strtol(str, &end, 10); if (end == str) return false; str = end; if (*str == ',') ++str;
    r.loopEnd = (int)std::strtol(str, &end, 10); if (end == str) return false; str = end;
    while (*str == ';' && r.count < kMsegMaxPoints) {
        ++str;
        float v[3] = { 0, 0, 0 };
        for (int k = 0; k < 3; ++k) { v[k] = std::strtof(str, &end); if (end == str) return false; str = end; if (*str == ',') ++str; }
        r.t[r.count] = r.count == 0 ? 0.f : (v[0] < 0.001f ? 0.001f : (v[0] > 60.f ? 60.f : v[0]));
        r.l[r.count] = v[1] < 0.f ? 0.f : (v[1] > 1.f ? 1.f : v[1]);
        r.c[r.count] = v[2] < -1.f ? -1.f : (v[2] > 1.f ? 1.f : v[2]);
        ++r.count;
    }
    if (r.count < 2) return false;
    if (r.loopStart < 0) r.loopStart = 0;
    if (r.loopEnd >= r.count) r.loopEnd = r.count - 1;
    if (r.loopStart > r.loopEnd) r.loopStart = r.loopEnd;
    m = r;
    return true;
}
// segment shape: x in 0..1, c in -1..1 (positive = slow start, negative = fast start)
static inline float msegCurve(float x, float c) {
    if (c > 0.001f) { float e = 1.f + 5.f * c; float r = 1.f; float b = x; int n = (int)e; for (int i = 0; i < n; ++i) r *= b; const float fr = e - (float)n; return r * (1.f - fr + fr * b); }
    if (c < -0.001f) { const float y = 1.f - x; float e = 1.f - 5.f * c; float r = 1.f; int n = (int)e; for (int i = 0; i < n; ++i) r *= y; const float fr = e - (float)n; return 1.f - r * (1.f - fr + fr * y); }
    return x;
}

// ------------------------------------------------------------------------------------------------
// custom LFO shape: 64 points, -1..1, stored as ints -1000..1000
static constexpr int kLfoShapePoints = 64;
static inline void lfoShapeDefault(float* a, int which) {
    for (int i = 0; i < kLfoShapePoints; ++i) {
        const float x = (float)i / kLfoShapePoints;
        switch (which % 3) {
        case 0: a[i] = x < 0.5f ? -1.f + 4.f * x * x * 2.f * (x < 0.25f ? 1.f : 1.f) : 1.f - 2.f * (x - 0.5f) * 2.f; break;
        case 1: a[i] = (float)((i / 8) % 2 ? 0.6 : -0.6) + 0.4f * std::sin(6.2831853f * x * 4.f) * 0.5f; break;
        default: a[i] = std::sin(6.2831853f * x) * 0.7f + 0.3f * std::sin(6.2831853f * x * 3.f); break;
        }
        if (a[i] > 1.f) a[i] = 1.f;
        if (a[i] < -1.f) a[i] = -1.f;
    }
}
static inline std::string lfoShapeToString(const float* a) {
    std::string s; char b[16];
    for (int i = 0; i < kLfoShapePoints; ++i) { int v = (int)(a[i] * 1000.f + (a[i] >= 0 ? 0.5f : -0.5f)); std::snprintf(b, sizeof(b), i ? ",%d" : "%d", v); s += b; }
    return s;
}
static inline bool lfoShapeFromString(const char* s, float* a) {
    if (!s || !*s) return false;
    int i = 0; char* end = nullptr;
    while (*s && i < kLfoShapePoints) {
        const long v = std::strtol(s, &end, 10);
        if (end == s) break;
        a[i++] = (float)(v < -1000 ? -1000 : (v > 1000 ? 1000 : v)) / 1000.f;
        s = end; if (*s == ',') ++s;
    }
    if (i == 0) return false;
    for (int k = i; k < kLfoShapePoints; ++k) a[k] = a[i - 1];
    return true;
}

// ------------------------------------------------------------------------------------------------
static inline void fxOrderDefault(int* o) { for (int i = 0; i < kFxUnits; ++i) o[i] = kFxDefaultOrder[i]; }
static inline bool fxOrderFromString(const char* s, int* o) {
    if (!s || !*s) return false;
    int tmp[kFxUnits]; bool used[kFxUnits] = {}; int n = 0; char* end = nullptr;
    while (*s && n < kFxUnits) {
        const long v = std::strtol(s, &end, 10);
        if (end == s) break;
        if (v < 0 || v >= kFxUnits || used[v]) return false;
        used[v] = true; tmp[n++] = (int)v;
        s = end; if (*s == ',') ++s;
    }
    for (int u = 0; u < kFxUnits && n < kFxUnits; ++u) if (!used[u]) tmp[n++] = u;   // append missing units (4.2: Resonator, Space)
    for (int i = 0; i < kFxUnits; ++i) o[i] = tmp[i];
    return true;
}
static inline std::string fxOrderToString(const int* o) {
    std::string s; char b[8];
    for (int i = 0; i < kFxUnits; ++i) { std::snprintf(b, sizeof(b), i ? ",%d" : "%d", o[i]); s += b; }
    return s;
}

// ------------------------------------------------------------------------------------------------
// 4.2 scenes: up to 8 snapshots of the 8 macros. "-" = empty slot; else 8 values 0..1000.
struct SceneData {
    bool used[kScenes] = {};
    float v[kScenes][kMacros] = {};
};
static inline std::string scenesToString(const SceneData& d) {
    std::string s; char b[16];
    for (int i = 0; i < kScenes; ++i) {
        if (i) s += ";";
        if (!d.used[i]) { s += "-"; continue; }
        for (int m = 0; m < kMacros; ++m) { std::snprintf(b, sizeof(b), m ? ",%d" : "%d", (int)(d.v[i][m] * 1000.f + 0.5f)); s += b; }
    }
    return s;
}
static inline bool scenesFromString(const char* s, SceneData& d) {
    d = SceneData();
    if (!s || !*s) return false;
    for (int i = 0; i < kScenes && *s; ++i) {
        if (*s == '-') { ++s; }
        else {
            char* end = nullptr; int m = 0;
            for (; m < kMacros; ++m) {
                const long v = std::strtol(s, &end, 10);
                if (end == s) break;
                d.v[i][m] = (float)(v < 0 ? 0 : (v > 1000 ? 1000 : v)) / 1000.f;
                s = end; if (*s == ',') ++s;
            }
            d.used[i] = m == kMacros;
        }
        while (*s && *s != ';') ++s;
        if (*s == ';') ++s;
    }
    return true;
}

// 4.2 motion recorder: macro movements sampled at kMotionRate, looped. Lanes exist only for the
// macros that moved while recording. Text: "<seconds>;<macro>:<v>,<v>,...;<macro>:..." (v 0..1000)
static constexpr int kMotionRate = 50;
static constexpr int kMotionMaxSec = 120;
static constexpr int kMotionMax = kMotionRate * kMotionMaxSec;
struct MotionData {
    float seconds = 0.f;          // lane length
    int frames = 0;               // samples per lane (seconds * kMotionRate)
    bool has[kMacros] = {};
    bool recording = false;       // the editor is recording: playback paused
    int recMask = 0xFF;           // 6.0 overdub: only these macros pause (the other lanes keep playing)
    float v[kMacros][kMotionMax];
};
static inline std::string motionToString(const MotionData& d) {
    if (d.frames <= 0) return d.recording ? "rec" : "";
    char mb[16]; std::snprintf(mb, sizeof mb, ";rec%d", d.recMask);
    std::string s; char b[24];
    std::snprintf(b, sizeof(b), "%.3f", d.seconds); s = b;
    for (int m = 0; m < kMacros; ++m) {
        if (!d.has[m]) continue;
        std::snprintf(b, sizeof(b), ";%d:", m); s += b;
        for (int i = 0; i < d.frames; ++i) { std::snprintf(b, sizeof(b), i ? ",%d" : "%d", (int)(d.v[m][i] * 1000.f + 0.5f)); s += b; }
    }
    if (d.recording) s += d.recMask == 0xFF ? std::string(";rec") : std::string(mb);   // 6.0: ";rec<mask>" while overdubbing
    return s;
}
static inline bool motionFromString(const char* s, MotionData& d) {
    d.seconds = 0.f; d.frames = 0; d.recording = false; d.recMask = 0xFF;
    for (bool& h : d.has) h = false;
    if (!s || !*s) return false;
    if (std::strncmp(s, "rec", 3) == 0) { d.recording = true; return true; }
    char* end = nullptr;
    const float sec = std::strtof(s, &end);
    if (end == s || !(sec > 0.f)) return false;
    d.seconds = sec > (float)kMotionMaxSec ? (float)kMotionMaxSec : sec;
    d.frames = (int)(d.seconds * kMotionRate + 0.5f);
    if (d.frames < 2) d.frames = 2;
    if (d.frames > kMotionMax) d.frames = kMotionMax;
    s = end;
    while (*s == ';') {
        ++s;
        if (std::strncmp(s, "rec", 3) == 0) {
            d.recording = true; s += 3;
            if (*s >= '0' && *s <= '9') { d.recMask = (int)std::strtol(s, &end, 10) & 0xFF; s = end; }
            continue;
        }
        const long m = std::strtol(s, &end, 10);
        if (end == s || *end != ':') break;
        s = end + 1;
        if (*s == '~') {
            // 5.1 keyframe form "~step:k0,k1,...": key j at frame j*step, the last key at the last frame,
            // linear in between (factory presets; saved sessions always use the frame-by-frame form)
            const long step = std::strtol(s + 1, &end, 10);
            if (end == s + 1 || *end != ':' || step < 1) break;
            s = end + 1;
            float keys[kMotionMax]; int nk = 0;
            while (*s && *s != ';') {
                const long v = std::strtol(s, &end, 10);
                if (end == s) { ++s; continue; }
                if (nk < kMotionMax) keys[nk++] = (float)(v < 0 ? 0 : (v > 1000 ? 1000 : v)) / 1000.f;
                s = end; if (*s == ',') ++s;
            }
            if (m >= 0 && m < kMacros && nk > 0) {
                auto pos = [&](int j) { const int q = j == nk - 1 ? d.frames - 1 : j * (int)step; return q < d.frames - 1 ? q : d.frames - 1; };
                for (int i = 0; i < d.frames; ++i) d.v[m][i] = keys[nk - 1];
                for (int j = 0; j + 1 < nk; ++j) {
                    const int a0 = pos(j), a1 = pos(j + 1);
                    for (int i = a0; i < a1; ++i) d.v[m][i] = keys[j] + (keys[j + 1] - keys[j]) * (float)(i - a0) / (float)(a1 - a0);
                }
                d.has[m] = true;
            }
            continue;
        }
        int i = 0;
        float last = 0.f;
        while (*s && *s != ';') {
            const long v = std::strtol(s, &end, 10);
            if (end == s) { ++s; continue; }
            last = (float)(v < 0 ? 0 : (v > 1000 ? 1000 : v)) / 1000.f;
            if (m >= 0 && m < kMacros && i < d.frames) d.v[m][i++] = last;
            s = end; if (*s == ',') ++s;
        }
        if (m >= 0 && m < kMacros && i > 0) { for (; i < d.frames; ++i) d.v[m][i] = last; d.has[m] = true; }
    }
    return true;
}

// 4.2 analysed modal bank (physical model "Analysed"): frequency ratios relative to the detected
// fundamental, amplitudes 0..1 and decay times in seconds. Text: "r,a,d;r,a,d;..."
static constexpr int kModalMax = 48;
struct ModalData {
    int count = 0;
    float ratio[kModalMax] = {}, amp[kModalMax] = {}, decay[kModalMax] = {};
};
static inline void modalDefault(ModalData& d) {   // a struck bowl until something is analysed
    static const float r[8] = { 1.f, 2.71f, 4.95f, 7.69f, 10.9f, 14.6f, 18.8f, 23.4f };
    d.count = 8;
    for (int i = 0; i < 8; ++i) { d.ratio[i] = r[i]; d.amp[i] = 1.f / (1.f + i * 0.8f); d.decay[i] = 12.f / (1.f + i * 0.7f); }
}
static inline std::string modalToString(const ModalData& d) {
    std::string s; char b[48];
    for (int i = 0; i < d.count; ++i) { std::snprintf(b, sizeof(b), i ? ";%.5g,%.4g,%.4g" : "%.5g,%.4g,%.4g", d.ratio[i], d.amp[i], d.decay[i]); s += b; }
    return s;
}
static inline bool modalFromString(const char* s, ModalData& d) {
    d = ModalData();
    if (!s || !*s) return false;
    char* end = nullptr;
    while (*s && d.count < kModalMax) {
        float v[3];
        int k = 0;
        for (; k < 3; ++k) { v[k] = std::strtof(s, &end); if (end == s) break; s = end; if (*s == ',') ++s; }
        if (k < 3) break;
        if (v[0] > 0.05f && v[0] < 200.f && v[1] > 0.f) {
            d.ratio[d.count] = v[0]; d.amp[d.count] = v[1] > 1.f ? 1.f : v[1];
            d.decay[d.count] = v[2] < 0.02f ? 0.02f : (v[2] > 120.f ? 120.f : v[2]);
            ++d.count;
        }
        if (*s == ';') ++s;
    }
    return d.count > 0;
}

// ------------------------------------------------------------------------------------------------
// Granular source sample (mono, file sample rate). Allocated/freed in generic code only.
struct SampleBuf {
    float* data = nullptr;     // len + 4 guard samples
    int len = 0;
    float sr = 48000.f;
    float rms = 0.13f;         // 6.0: RMS after the peak normalisation (level compensation)
    std::string name;
};

// ------------------------------------------------------------------------------------------------
// How a matrix destination maps onto a knob (used by the UI to draw modulation rings).
// kind 0: param += amt * scale (param units); kind 1: param *= 2^(amt * scale) (log params)
struct DestMap { int param; int kind; float scale; };
static inline DestMap destMap(int d, const int* oscType) {
    auto oscMap = [&](int o, int k) -> DestMap {
        const int t = oscType[o];
        switch (k) {
        case 1: return { (int)oscP(o, ofLevel), 0, 1.f };
        case 2: return { (int)oscP(o, ofPan), 0, 1.f };
        case 7: return { (int)oscP(o, ofFmAmt), 0, 1.f };
        case 8: return { (int)oscP(o, ofDetune), 0, 50.f };
        case 3: case 4: case 5: case 6: {
            const int ti = k - 3;
            static const uint32_t wt[4] = { ofPos, ofWarpAmt, ofUniWidth, ofPos };
            static const float wtS[4] = { 1.f, 1.f, 1.f, 0.1f };
            static const uint32_t an[4] = { ofAnPW, ofAnSync, ofAnSlop, ofUniWidth };
            static const float anS[4] = { 0.45f, 24.f, 1.f, 1.f };
            static const uint32_t ad[4] = { ofAdMorph, ofAdPeakPos, ofAdTilt, ofAdVowel };
            static const uint32_t pm[4] = { ofPmPos, ofPmHard, ofPmDecay, ofPmDamp };
            static const float pmS[4] = { 0.5f, 1.f, 3.f, 1.f };
            static const uint32_t gr[4] = { ofGrPos, ofGrSize, ofGrDensity, ofGrSpray };
            switch (t) {
            case otWavetable: return { (int)oscP(o, wt[ti]), 0, wtS[ti] };
            case otAnalog:    return { (int)oscP(o, an[ti]), 0, anS[ti] };
            case otAdditive:  return { (int)oscP(o, ad[ti]), ti == 1 ? 1 : 0, ti == 1 ? 3.f : 1.f };
            case otPhysical:  return { (int)oscP(o, pm[ti]), ti == 2 ? 1 : 0, pmS[ti] };
            case otSpectral: { static const uint32_t sp[4] = { o2SpPos, o2SpBlur, o2SpShift, o2SpCross }; static const float spS[4] = { 1.f, 5.f, 200.f, 1.f };
                               return { (int)osc2P(o, sp[ti]), 0, spS[ti] }; }
            case otFm: { static const int fm[4] = { fmIndex, fmFeedback, fmBright, fmSpread }; return { (int)fmP(o, fm[ti]), 0, 1.f }; }
            default:          return { (int)oscP(o, gr[ti]), (ti == 1 || ti == 2) ? 1 : 0, (ti == 1 || ti == 2) ? 3.f : 1.f };
            }
        }
        default: return { -1, 0, 0.f };
        }
    };
    switch (d) {
    case mdF1Cutoff: return { (int)pF1Cutoff, 1, 5.f };
    case mdF1Res: return { (int)pF1Res, 0, 1.f };
    case mdF1Drive: return { (int)pF1Drive, 0, 1.f };
    case mdF1Shape: return { (int)pF1Shape, 0, 1.f };
    case mdF1Morph: return { (int)pF1Morph, 0, 1.f };
    case mdF1Fm: return { (int)pF1FmAmt, 0, 1.f };
    case mdF2Cutoff: return { (int)pF2Cutoff, 1, 5.f };
    case mdF2Res: return { (int)pF2Res, 0, 1.f };
    case mdF2Drive: return { (int)pF2Drive, 0, 1.f };
    case mdF2Shape: return { (int)pF2Shape, 0, 1.f };
    case mdF2Morph: return { (int)pF2Morph, 0, 1.f };
    case mdF2Fm: return { (int)pF2FmAmt, 0, 1.f };
    case mdBalance: return { (int)pFBalance, 0, 1.f };
    case mdRing: return { (int)pRingLevel, 0, 1.f };
    case mdSub: return { (int)pSubLevel, 0, 1.f };
    case mdNoise: return { (int)pNoiseLevel, 0, 0.5f };
    case mdFxPhaser: return { (int)pPhMix, 0, 1.f };
    case mdFxChorus: return { (int)pChMix, 0, 1.f };
    case mdFxDelay: return { (int)pDlMix, 0, 1.f };
    case mdFxShift: return { (int)pFsShift, 0, 200.f };
    case mdFxShiftMix: return { (int)pFsMix, 0, 1.f };
    case mdFxWow: return { (int)pTpWow, 0, 1.f };
    case mdFxCrush: return { (int)pCrMix, 0, 1.f };
    case mdFxWidth: return { (int)pWdWidth, 0, 1.f };
    case mdFxDrive: return { (int)pDrvAmt, 0, 1.f };
    default: break;
    }
    if (d >= mdLfo1Rate && d <= mdLfo6Rate) return { (int)lfoP(d - mdLfo1Rate, lfRate), 1, 4.f };
    if (d == mdMseg1Rate) return { (int)msegP(0, 1), 1, -3.f };
    if (d == mdMseg2Rate) return { (int)msegP(1, 1), 1, -3.f };
    if (d == mdMseg3Rate) return { (int)msegP(2, 1), 1, -3.f };
    if (d == mdMseg4Rate) return { (int)msegP(3, 1), 1, -3.f };
    if (d >= mdAPress && d <= mdCPress) return { (int)osc2P(d - mdAPress, o2Pressure), 0, 1.f };
    if (d == mdFxReso) return { (int)pRsMix, 0, 1.f };
    if (d == mdFxSpace) return { (int)pSpMix, 0, 1.f };
    if (d == mdFxSpecMix) return { (int)pSxMix, 0, 1.f };
    if (d == mdFxSpecBlur) return { (int)pSxBlur, 0, 10.f };
    if (d == mdPitchAll) return { (int)pPitch, 0, 12.f };                 // 5.1.6: PITCH knob (amount 1 = 12 st)
    if (d >= mdFEnvA && d <= mdMEnvR) {                                   // 6.0: envelope stages (times x16 at amount 1)
        static const uint32_t base[3] = { pFEnvA, pAEnvA, pMEnvA };
        const int e = (d - mdFEnvA) / 4, k = (d - mdFEnvA) % 4;
        return k == 2 ? DestMap{ (int)(base[e] + 2), 0, 1.f } : DestMap{ (int)(base[e] + k), 1, 4.f };
    }
    if (d >= mdAPhase && d <= mdCPhase) return { (int)oscP(d - mdAPhase, ofPhase), 0, 1.f };
    switch (d) {                                                          // 6.0 FX parameters
    case mdFxPhRate: return { (int)pPhRate, 1, 3.f };
    case mdFxPhFb: return { (int)pPhFb, 0, 1.f };
    case mdFxChRate: return { (int)pChRate, 1, 3.f };
    case mdFxChDepth: return { (int)pChDepth, 0, 1.f };
    case mdFxDlTime: return { (int)pDlTime, 1, 2.f };
    case mdFxDlFb: return { (int)pDlFb, 0, 1.f };
    case mdFxDlDamp: return { (int)pDlDamp, 0, 1.f };
    case mdFxFsFb: return { (int)pFsFb, 0, 1.f };
    case mdFxTpFlutter: return { (int)pTpFlutter, 0, 1.f };
    case mdFxTpTone: return { (int)pTpTone, 0, 1.f };
    case mdFxRsDecay: return { (int)pRsDecay, 1, 3.f };
    case mdFxRsBright: return { (int)pRsBright, 0, 1.f };
    case mdFxSpSize: return { (int)pSpSize, 0, 1.f };
    case mdFxSpDecay: return { (int)pSpDecay, 1, 3.f };
    case mdFxSpDamp: return { (int)pSpDamp, 0, 1.f };
    case mdFxSxShift: return { (int)pSxShift, 0, 24.f };
    case mdFxSxTilt: return { (int)pSxTilt, 0, 1.f };
    default: break;
    }
    if (d >= mdAOct && d <= mdCFine) {                                    // 5.1.6: OCT / SEMI / FINE knobs
        const int o = (d - mdAOct) / 3, k = (d - mdAOct) % 3;
        static const uint32_t f[3] = { ofOct, ofSemi, ofFine };
        static const float sc[3] = { 3.f, 12.f, 50.f };                   // amount 1 = the knob's full range
        return { (int)oscP(o, f[k]), 0, sc[k] };
    }
    if (d >= mdAPitch && d < mdAPitch + 3 * kOscDstStride) { const int o = (d - mdAPitch) / kOscDstStride; return oscMap(o, (d - mdAPitch) % kOscDstStride); }
    if (d >= mdAT5 && d < mdAT5 + 3 * kTimbreX) {                         // 5.1.7: Timbre 5..10 (scales match the engine)
        const int o = (d - mdAT5) / kTimbreX, k = (d - mdAT5) % kTimbreX;
        struct X { uint32_t f; bool second; float scale; };
        static const X an[kTimbreX] = { { ofAnDrift, false, 1.f }, {}, {}, {}, {}, {} };
        static const X ad[kTimbreX] = { { ofAdStretch, false, 1.f }, { ofAdOddEven, false, 1.f }, { ofAdPeakAmt, false, 1.f },
                                        { ofAdCombShift, false, 1.f }, { ofAdShimmer, false, 1.f }, { ofAdDetune, false, 30.f } };
        static const X pm[kTimbreX] = { { ofPmStretch, false, 1.f }, { ofPmSpread, false, 1.f }, { ofPmCouple, false, 1.f },
                                        { ofPmBuzz, false, 1.f }, { o2Noise, true, 1.f }, { o2Nonlin, true, 1.f } };
        static const X gr[kTimbreX] = { { ofGrScan, false, 1.f }, { ofGrPitchRand, false, 1200.f }, { ofGrSpread, false, 1.f },
                                        { ofGrReverse, false, 1.f }, {}, {} };
        static const X sp[kTimbreX] = { { o2SpSpeed, true, 2.f }, { o2SpStretch, true, 1.f }, { o2SpTilt, true, 1.f },
                                        { o2SpFormant, true, 1.f }, { o2SpSparse, true, 1.f }, {} };
        const X* t = nullptr;
        switch (oscType[o]) {
        case otAnalog: t = an; break; case otAdditive: t = ad; break; case otPhysical: t = pm; break;
        case otGranular: t = gr; break; case otSpectral: t = sp; break; default: break;
        }
        if (oscType[o] == otFm) return { (int)fmOpP(o, k, foLevel), 0, 1.f };      // 6.0: operator levels
        if (!t || t[k].scale == 0.f) return { -1, 0, 0.f };
        return { (int)(t[k].second ? osc2P(o, t[k].f) : oscP(o, t[k].f)), 0, t[k].scale };
    }
    return { -1, 0, 0.f };
}

// ------------------------------------------------------------------------------------------------
// Live telemetry. The engine writes it a few hundred times a second; the UI copies it out under a
// sequence counter (odd = being written) and retries if it changed mid-copy.
static constexpr int kTelVoices = 24;
static constexpr int kTelPartials = 128;
static constexpr int kTelModes = 48;
static constexpr int kTelGrains = 32;
static constexpr int kScopeSize = 4096;      // per channel, full rate up to 60 kHz (half rate above)

struct TelVoice {
    int32_t active, gate, note, age;
    int32_t ch;                      // 5.1.1: MIDI channel (keyboard strip)
    float vel, press, slide;         // 5.1.1: velocity, pressure (channel / poly AT / note expression), slide
    float bend;                      // 5.1.1: semitones (global wheel + per-note MPE bend + VST3 note-expression tuning)
    float notePress, chanAT;         // 5.1.2: per-note pressure (MPE / poly AT / note expression) vs whole-keyboard aftertouch
    float amp;                       // amp envelope
    float envF, envM;
    float pitch;                     // semitones incl. bend/glide
    float t[3][4];                   // modulated engine values (param units), per osc
    float level[3], fm[3];
    float cut1, cut2, res1, res2, shape1, shape2, morph1, morph2;   // filter values (Hz incl. env)
    float lfo[kLfos];                // per-voice LFO output (global LFOs mirrored)
    float lfoPh[kLfos];
    float lfoRate[kLfos], lfoFade[kLfos];   // cycles per second, fade-in gain (for UI extrapolation)
    float mseg[kMsegs], msegT[kMsegs];   // value, time since start (s)
    int32_t msegSeg[kMsegs];
    float rndV;
    float msegU[kMsegs];             // 5.1.6: position inside the current segment (0..1), for the playhead x
};

struct Telemetry {
    uint32_t seq;
    int32_t newest;                  // index of the most recently started active voice, -1 if none
    int32_t activeCount;
    int32_t quality;
    float cpu;                       // 0..1 of the real-time budget
    float bpm;
    double beat;
    int32_t playing;
    TelVoice v[kTelVoices];
    float lfoG[kLfos], lfoGPh[kLfos], lfoGRate[kLfos];   // global LFO value / phase / rate (Hz)
    float macro[kMacros];                   // effective macro values (after scene morph and motion)
    float scenePos;                         // smoothed scene morph position
    float motionT, motionLen;               // motion playback position / length (s); len 0 = no lanes
    int32_t motionOn[kMacros];              // 1 where a lane is driving the macro
    float slotOut[kModSlots];               // curve(src) * via * amt for the newest voice
    float slotSrc[kModSlots];
    float md[kModDstCount];                 // summed modulation per destination (newest voice; FX = global)
    float rndG, seqCv;
    int32_t seqStep, arpStep, arpNote;
    float partials[3][kTelPartials];        // newest voice additive amplitudes (normalised)
    float modes[3][kTelModes];              // newest voice resonator energy
    float modeRatio[3][kTelModes];
    int32_t modeCount[3];
    float grainPos[3][kTelGrains], grainAmp[3][kTelGrains];
    float specPos[3];                       // 5.0 spectral playhead (0..1) of the newest voice, per osc
    float scope[kScopeSize], scopeR[kScopeSize];   // L (or mono) and R
    int32_t scopeWrite;
    float scopeRate;                        // samples per second in the scope buffers
    float mixPk[6];                         // pre-filter peaks: osc A, B, C, ring, sub, noise (amp-env weighted)
    float peakL, peakR;
};

static inline void telBegin(Telemetry& t) { __atomic_store_n(&t.seq, t.seq + 1, __ATOMIC_RELEASE); __atomic_thread_fence(__ATOMIC_RELEASE); }
static inline void telEnd(Telemetry& t) { __atomic_thread_fence(__ATOMIC_RELEASE); __atomic_store_n(&t.seq, t.seq + 1, __ATOMIC_RELEASE); }
// copy out; returns false if the writer was active (caller keeps its previous copy)
static inline bool telRead(const Telemetry& src, Telemetry& dst) {
    for (int tries = 0; tries < 4; ++tries) {
        const uint32_t a = __atomic_load_n(&src.seq, __ATOMIC_ACQUIRE);
        if (a & 1u) continue;
        std::memcpy(&dst, &src, sizeof(Telemetry));
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        const uint32_t b = __atomic_load_n(&src.seq, __ATOMIC_ACQUIRE);
        if (a == b) return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// MIDI learn map: CC number -> parameter index (-1 = none). Stored as "cc=symbol;cc=symbol".
static constexpr int kMidiMapSize = 128;
static inline bool ccLearnable(int cc) { return cc >= 0 && cc < 120 && cc != 64 && cc != 74 && cc != 0 && cc != 32 && cc != 6 && cc != 38 && cc != 98 && cc != 99 && cc != 100 && cc != 101; }
static inline int paramIndexOfSymbol(const char* sym, size_t n) {
    for (uint32_t i = 0; i < kParamCount; ++i) { const char* s = P(i).symbol; if (std::strlen(s) == n && std::strncmp(s, sym, n) == 0) return (int)i; }
    return -1;
}
template <class Map> static inline std::string midiMapToString(const Map& m) {
    std::string s; char b[16];
    for (int c = 0; c < kMidiMapSize; ++c) {
        const int p = m[c];
        if (p < 0 || p >= (int)kParamCount) continue;
        std::snprintf(b, sizeof(b), "%d=", c);
        if (!s.empty()) s += ";";
        s += b; s += P((uint32_t)p).symbol;
    }
    return s;
}
// fills out[128]; returns false on an empty string
static inline bool midiMapFromString(const char* str, int* out) {
    for (int c = 0; c < kMidiMapSize; ++c) out[c] = -1;
    if (!str || !*str) return false;
    while (*str) {
        const char* e = str;
        while (*e && *e != ';') ++e;               // one "cc=symbol" token; junk tokens are skipped
        char* end = nullptr;
        const long cc = std::strtol(str, &end, 10);
        if (end != str && end < e && *end == '=') {
            const char* sym = end + 1;
            const int p = paramIndexOfSymbol(sym, (size_t)(e - sym));
            if (cc >= 0 && cc < kMidiMapSize && p >= 0 && ccLearnable((int)cc)) out[cc] = p;
        }
        str = *e ? e + 1 : e;
    }
    return true;
}

// 4.1 resample: the plug-in writes its stereo output here (interleaved, ring); the editor copies the
// last N seconds out for CAPTURE. Copy-while-writing is benign: at worst the oldest few samples of a
// capture are already overwritten.
struct CaptureRing {
    std::vector<float> data;               // size * 2, interleaved L R
    int size = 0;
    float sr = 48000.f;
    std::atomic<int> write { 0 }, filled { 0 };
};

// Object shared between the plugin and its UI through DPF direct access.
struct SharedBlock {
    Telemetry tel;                          // layer 1 (and the whole plug-in's cpu / scope when layers are off)
    Telemetry tel2;                         // layer 2
    std::atomic<CaptureRing*> capture { nullptr };
    std::atomic<int> uiOpen { 0 };
    // CPU guard
    std::atomic<int> guardOn { 1 };        // user option (Setup page)
    std::atomic<int> guardCap { 0 };       // 0 = idle, else the current voice cap (header indicator)
    std::atomic<int> guardRealtime { 0 };  // 1 when the host reported real-time processing (VST3)
    // legacy 3.0 envelope model for this instance
    std::atomic<int> envClassic { 0 };
    // MIDI learn
    std::atomic<int> learnArm { -1 };      // parameter waiting for a CC, -1 = none
    std::atomic<int> midiMap[kMidiMapSize];
    std::atomic<float> ccValue[kMidiMapSize];
    std::atomic<uint32_t> ccSeq { 0 };     // bumps whenever a mapped CC moved a value or the map changed
    // background file loading
    std::atomic<int> loadsPending { 0 };
    // 4.0 tuning status for the editor
    std::atomic<int> mtsConnected { 0 };
    std::atomic<uint32_t> mtsSeq { 0 };    // odd while the plug-in rewrites the fields below
    char mtsScaleName[64] = {};
    float mtsPitch[128] = {};              // absolute semitones per key from the MTS-ESP master
    std::atomic<int> neSeen { 0 };         // VST3 note-expression events received (count)
    // 5.1.1: PANIC button (editor sets, audio thread clears both layers at the next block)
    std::atomic<int> panicReq { 0 };
    // 5.1.1: on-screen keyboard -> audio thread: single-producer (editor) / single-consumer (audio) MIDI ring
    static constexpr uint32_t kUiMidiSize = 512;
    std::atomic<uint32_t> uiMidiW { 0 }, uiMidiR { 0 };
    uint32_t uiMidi[kUiMidiSize] = {};     // status | d1 << 8 | d2 << 16
    bool pushUiMidi(uint8_t st, uint8_t d1, uint8_t d2) {
        const uint32_t w = uiMidiW.load(std::memory_order_relaxed), r = uiMidiR.load(std::memory_order_acquire);
        if (w - r >= kUiMidiSize) return false;
        uiMidi[w % kUiMidiSize] = (uint32_t)st | ((uint32_t)d1 << 8) | ((uint32_t)d2 << 16);
        uiMidiW.store(w + 1, std::memory_order_release);
        return true;
    }
    // 5.1.1: a MIDI program change loaded a factory preset in the plug-in: the editor refreshes from it
    std::atomic<uint32_t> pcSerial { 0 };
    std::atomic<int> pcIndex { -1 };
    // 5.1.1: safety limiter gain reduction (dB, >= 0) for the meter
    std::atomic<float> limGr { 0.f };
    std::atomic<int> sclError { 0 };       // 1 when the stored Scala text failed to parse
    std::atomic<int> threadsActive { 1 };  // 4.2: render threads in use (1 = audio thread only)
    SharedBlock() { for (int c = 0; c < kMidiMapSize; ++c) { midiMap[c].store(-1); ccValue[c].store(0.f); } }
    std::string mapString() const { int m[kMidiMapSize]; for (int c = 0; c < kMidiMapSize; ++c) m[c] = midiMap[c].load(); return midiMapToString(m); }
    void setMap(const int* m) { for (int c = 0; c < kMidiMapSize; ++c) midiMap[c].store(m[c]); ccSeq.fetch_add(1); }
};

} // namespace drift

// defined in DriftfieldPlugin.cpp: maps the DPF instance pointer to the shared block
drift::SharedBlock* driftSharedFromInstance(void* pluginInstance);
