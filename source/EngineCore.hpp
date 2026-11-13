// Driftfield 3 — DSP engine core. Included by EngineGeneric.cpp and EngineAVX2.cpp with
// DRIFT_NS / DRIFT_DSP_NS set to a per-build namespace.
//
// voice:  Osc A/B/C (Wavetable | Analog | Additive | Physical | Granular, with FM / sync between them)
//         + ring + sub + noise -> Filter 1 / Filter 2 (serial | parallel | split, type morph, audio FM)
//         -> amp env.   Modulation: 3 ADSR, 2 MSEG, 6 LFO, 2 drift generators, 8 macros, step seq,
//         24-slot matrix with via + curve.   Note generators: arpeggiator, step sequencer.
// global: FX chain (9 units, any order), master.  HQ quality renders voices at 2x and decimates.
#pragma once
#include <cmath>
#include <cstring>
#include <atomic>
#include <algorithm>
#include <vector>
#include "SpecData.hpp"
#include "FmAlgos.hpp"

namespace DRIFT_NS {

using drift::WaveTable;
using drift::BuiltinTables;
using drift::SampleBuf;
using drift::MsegData;
using drift::Telemetry;
using drift::SceneData;
using drift::MotionData;
using drift::ModalData;
using drift::kModalMax;
using drift::SpecData;
using drift::wtTable;
using drift::wtMipFor;
using drift::kWtSize;
using drift::kWtFrames;
using drift::kWtBanks;

static constexpr int   kMaxVoices   = 24;
static constexpr int   kMaxUnison   = 7;
static constexpr int   kBlock       = 16;
static constexpr int   kMaxChunk    = 256;
static constexpr int   kMaxChunkI   = kMaxChunk * 2;          // internal (oversampled) chunk
static constexpr int   kBlocksPerChunk = kMaxChunkI / kBlock;
static constexpr int   kStealFade   = 96;
static constexpr int   kBaseModes   = 24;
static constexpr int   kMaxModes    = 48;
static constexpr int   kMaxPartials = 128;
static constexpr int   kMaxGrains   = 32;
static constexpr int   kSpMaxN      = 4096;                   // 5.0 spectral oscillator frame (HQ)
static constexpr int   kSpMaxBins   = kSpMaxN / 2 + 1;
static constexpr int   kWgSize      = 8192;
static constexpr int   kSymSize     = 4096;
static constexpr int   kSymLines    = 4;
static constexpr int   kWinSize     = 512;
static constexpr int   kHbTaps      = 63;                     // half-band decimator
static constexpr float kTwoPi       = 6.28318530717958647692f;
static constexpr float kPiF         = 3.14159265358979323846f;
static_assert(kNumFilterTypes == kFilterTypes, "filter list mismatch");
static_assert(kPartials == kMaxPartials, "partial count mismatch");

static inline float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
static inline float fastSinTurns(float x) {
    x -= std::floor(x);
    const float t = x < 0.5f ? x * 2.f - 0.5f : 1.5f - x * 2.f;
    const float u = t * kPiF;
    const float u2 = u * u;
    return u * (1.f - u2 * (0.16605f - u2 * 0.00761f));
}
// sin & cos of w in [0, pi] (half-angle Taylor + double angle), ~4e-6 abs error
static inline void fastSinCos(float w, float& s, float& c) {
    const float h = 0.5f * w, h2 = h * h;
    const float sh = h * (1.f - h2 * (1.f / 6.f - h2 * (1.f / 120.f - h2 * (1.f / 5040.f - h2 * (1.f / 362880.f)))));
    const float ch = 1.f - h2 * (0.5f - h2 * (1.f / 24.f - h2 * (1.f / 720.f - h2 * (1.f / 40320.f - h2 * (1.f / 3628800.f)))));
    s = 2.f * sh * ch;
    c = 1.f - 2.f * sh * sh;
}
static inline float semisToHz(float s) { return 440.f * fastExp2((s - 69.f) * (1.f / 12.f)); }

struct Rng {
    uint32_t s = 0x9E3779B9u;
    inline uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    inline float uni() { return (float)(next() >> 8) * (1.f / 16777216.f); }
    inline float bi()  { return uni() * 2.f - 1.f; }
};

struct EnvCoefs { float a = 0.f, d = 0.f, r = 0.f, s = 1.f, k = 0.f, aRC = 0.f, rk = 0.f, ra = 0.f, dk = 0.f, da = 0.f; bool classic = false; };
// attack: a timed curve that takes exactly the attack time. k = 0 is an S-shape (gentle start, no
// corner at the peak), k > 0 bends toward a punchy analog rise, k < 0 toward a slow swell.
static inline float attackShape(float p, float k) {
    const float s = p * p * (3.f - 2.f * p);
    if (k >= 0.f) { const float q = 1.f - p; return s + (1.f - q * q * q - s) * k; }
    return s + (p * p * p - s) * (-k);
}
// classic = the 3.0 attack (one-pole toward 1.25, about 1.4x the knob time); kept for 3.0 sessions
// 5.1.3 release curve: the release is drawn in dB from the level at key-up down to -80 dB over the release time.
// rk = 0 is a straight line in dB (the exponential release of every earlier version, computed the old way);
// rk > 0 bends it to drop fast then linger, rk < 0 holds the level and falls away at the end.
static inline float releaseShape(float p, float rk) {
    if (rk > 0.f) { const float q = 1.f - p; return 1.f - std::pow(q, 1.f + 3.f * rk); }
    return std::pow(p, 1.f - 3.f * rk);
}
static inline void envCoefs(EnvCoefs& c, float a, float d, float s, float r, float fs, float k = 0.f, bool classic = false, float rk = 0.f, float dk = 0.f) {
    c.rk = rk < -1.f ? -1.f : (rk > 1.f ? 1.f : rk);
    c.dk = dk < -1.f ? -1.f : (dk > 1.f ? 1.f : dk);           // 6.0 decay curve
    c.da = 1.f / ((d > 0.0005f ? d : 0.0005f) * fs);
    c.ra = 1.f / ((r > 0.0005f ? r : 0.0005f) * fs);
    c.a = 1.f / ((a > 0.0005f ? a : 0.0005f) * fs);
    c.aRC = 1.f - std::exp(-1.f / ((a > 0.0005f ? a : 0.0005f) * fs));
    c.classic = classic;
    c.k = k < -1.f ? -1.f : (k > 1.f ? 1.f : k);
    c.d = std::exp(-6.9078f / ((d > 0.0005f ? d : 0.0005f) * fs));
    c.r = std::exp(-9.2103f / ((r > 0.0005f ? r : 0.0005f) * fs));
    c.s = s;
}
struct Env {
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    int stage = Idle;
    float v = 0.f, p = 0.f, v0 = 0.f;
    void gateOn()  { stage = Attack; p = 0.f; v0 = v; }
    void gateOff() { if (stage != Idle) { stage = Release; p = 0.f; v0 = v; } }
    void reset()   { stage = Idle; v = 0.f; p = 0.f; v0 = 0.f; }
    inline float tick(const EnvCoefs& c) {
        switch (stage) {
        case Attack:
            if (c.classic) { v += (1.25f - v) * c.aRC; if (v >= 1.f) { v = 1.f; stage = Decay; p = 0.f; v0 = 1.f; } break; }
            p += c.a; if (p >= 1.f) { v = 1.f; stage = Decay; p = 0.f; v0 = 1.f; } else v = v0 + (1.f - v0) * attackShape(p, c.k); break;
        case Decay:
            if (c.dk == 0.f) { v = c.s + (v - c.s) * c.d; if (std::fabs(v - c.s) < 1e-4f) { v = c.s; stage = Sustain; } break; }
            // 6.0 curved decay: the distance to the sustain level falls 60 dB over the decay time along
            // releaseShape (curve 0 would be the exponential above)
            p += c.da; if (p >= 1.f) { v = c.s; stage = Sustain; break; }
            v = c.s + (v0 - c.s) * std::exp2(-9.9658f * releaseShape(p, c.dk)); break;
        case Sustain: v = c.s; break;
        case Release:
            if (c.rk == 0.f) { v *= c.r; if (v < 1e-4f) { v = 0.f; stage = Idle; } break; }
            p += c.ra; if (p >= 1.f || v0 < 1e-4f) { v = 0.f; stage = Idle; break; }
            v = v0 * std::exp2(-13.2877f * releaseShape(p, c.rk)); break;
        default: break;
        }
        return v;
    }
};

struct ChannelState { float bend = 0.f, pressure = 0.f, slide = 0.f; };

struct LfoState { float phase = 0.f, sh = 0.f, shPrev = 0.f, rateMul = 1.f, fade = 1.f, smooth = 0.f, rateHz = 0.f; bool done = false; };
static inline float lfoValue(int wave, const LfoState& st, const float* shape) {
    const float ph = st.phase;
    switch (wave) {
    case 0: return fastSinTurns(ph);
    case 1: return ph < 0.5f ? 4.f * ph - 1.f : 3.f - 4.f * ph;
    case 2: return 2.f * ph - 1.f;
    case 3: return 1.f - 2.f * ph;
    case 4: return ph < 0.5f ? 1.f : -1.f;
    case 5: return st.sh;
    case 6: { const float s = ph * ph * (3.f - 2.f * ph); return st.shPrev + (st.sh - st.shPrev) * s; }
    default: {
        const float x = ph * (float)drift::kLfoShapePoints;
        const int i = (int)x; const float f = x - (float)i;
        const float a = shape[i & (drift::kLfoShapePoints - 1)], b = shape[(i + 1) & (drift::kLfoShapePoints - 1)];
        return a + (b - a) * f;
    }
    }
}
// returns true when the phase wrapped
static inline bool lfoAdvance(LfoState& st, float inc, Rng& rng) {
    st.phase += inc;
    if (st.phase >= 1.f) { st.phase -= std::floor(st.phase); st.shPrev = st.sh; st.sh = rng.bi(); return true; }
    return false;
}

struct MsegState { int seg = 1; float t = 0.f, v = 0.f, start = 0.f, time = 0.f, u = 0.f; bool done = false, hold = false; };   // 5.1.6: u = position in segment

struct DriftGen {
    float v = 0.f, from = 0.f, to = 0.f, ph = 1.f;
    inline void tick(float inc, Rng& rng) {
        ph += inc;
        if (ph >= 1.f) { ph -= std::floor(ph); from = to; to = rng.bi(); }
        const float s = ph * ph * (3.f - 2.f * ph);
        v = from + (to - from) * s;
    }
};

static inline int pmModeRatios(int model, float* r) {
    switch (model) {
    case 0: case 7: case 8: case 9: for (int i = 0; i < kBaseModes; ++i) r[i] = (float)(i + 1); return kBaseModes;
    case 1: for (int i = 0; i < 16; ++i) { const float k = (2.f * (i + 1) + 1.f) / 3.f; r[i] = k * k; } return 16;
    case 2: { static const float b[16] = { 0.5f, 1.f, 1.183f, 1.506f, 2.f, 2.514f, 2.662f, 3.011f, 4.166f, 5.433f,
                                           6.796f, 8.215f, 9.7f, 11.2f, 12.8f, 14.5f };
              for (int i = 0; i < 16; ++i) r[i] = b[i]; return 16; }
    case 3: { static const float m[24] = { 1.f, 1.593f, 2.136f, 2.296f, 2.653f, 2.918f, 3.156f, 3.501f, 3.6f, 3.652f,
                                           4.06f, 4.154f, 4.601f, 4.832f, 4.903f, 5.131f, 5.412f, 5.553f, 5.65f,
                                           5.931f, 6.f, 6.209f, 6.536f, 6.749f };
              for (int i = 0; i < 24; ++i) r[i] = m[i]; return 24; }
    case 4: { static const float pl[20] = { 1.f, 2.5f, 2.52f, 4.f, 5.f, 5.03f, 6.5f, 6.53f, 8.5f, 8.52f, 9.f, 10.f,
                                            12.5f, 12.53f, 13.f, 14.5f, 14.53f, 16.f, 17.f, 18.f };
              for (int i = 0; i < 20; ++i) r[i] = pl[i]; return 20; }
    case 5: { static const float g[10] = { 1.f, 2.32f, 4.25f, 6.63f, 9.38f, 12.5f, 16.f, 19.9f, 24.2f, 28.9f };
              for (int i = 0; i < 10; ++i) r[i] = g[i]; return 10; }
    case 6: for (int i = 0; i < 16; ++i) r[i] = (float)(2 * i + 1); return 16;
    case 12: {   // gong / tam-tam: dense, inharmonic, slightly irregular (r must hold kMaxModes)
        for (int i = 0; i < kMaxModes; ++i) r[i] = 1.f + 0.62f * std::pow((float)i, 1.12f) + (i ? 0.07f * std::sin((float)i * 2.39f) : 0.f);
        return kMaxModes; }
    default: return 0;
    }
}

static inline float applyCurve(int c, float x) {
    switch (c) {
    case 1: return x * (x < 0.f ? -x : x);
    case 2: return x < 0.f ? -std::sqrt(-x) : std::sqrt(x);
    case 3: { const float y = clampf(x, -1.f, 1.f); return 1.5f * y - 0.5f * y * y * y; }
    case 4: return 0.5f + 0.5f * x;
    case 5: return -x;
    case 6: return std::floor(x * 4.f + 0.5f) * 0.25f;
    case 7: return 2.f * x - 1.f;                                       // 5.1.6: Bipolar
    default: return x;
    }
}

static inline float driveShape(int type, float x) {
    switch (type) {
    case 1: return satTanh(x + 0.25f) - 0.2449f;                        // tube: biased, asymmetric
    case 2: { const float c = fclamp(x, -1.f, 1.f); return c * 0.85f + 0.15f * satTanh(x * 2.f); }   // diode: hard knee
    case 3: return fastSinTurns(x * 0.22f);                              // wavefolder
    default: return satTanh(x);
    }
}

// 2x oversampled shaper for the filter drive in Normal quality: polyphase IIR half-band
// (2+2 first-order allpasses, ~69 dB image rejection above 0.6 Nyquist, flat to 0.4).
static constexpr float kHb0 = 0.07986642623635751f, kHb1 = 0.28382934487410993f, kHb2 = 0.5453536510711322f, kHb3 = 0.8344118914807379f;
struct AP1 { float x1 = 0.f, y1 = 0.f; inline float run(float c, float x) { const float y = c * (x - y1) + x1; x1 = x; y1 = y; return y; } };
struct Os2Shaper {
    AP1 u0a, u0b, u1a, u1b, d0a, d0b, d1a, d1b;
    inline float run(int type, float x) {
        const float a = driveShape(type, u0b.run(kHb2, u0a.run(kHb0, x)));
        const float b = driveShape(type, u1b.run(kHb3, u1a.run(kHb1, x)));
        return 0.5f * (d0b.run(kHb2, d0a.run(kHb0, b)) + d1b.run(kHb3, d1a.run(kHb1, a)));
    }
    void reset() { *this = Os2Shaper(); }
};

#include "SpecDsp.hpp"
#include "Fx.hpp"

// ------------------------------------------------------------------------------------------------
struct Grain {
    bool  on = false;
    float pos = 0.f, inc = 0.f;          // sample source: index / step;  table source: phase / step
    float age = 0.f, ageInc = 0.f;       // window position 0..1
    float gl = 0.7f, gr = 0.7f;
    int   frame = 0, mip = 0;
};

struct OscVoice {
    // unison (wavetable / analog)
    int   nUni = 1;
    float ph[kMaxUnison] = {}, inc[kMaxUnison] = {}, gL[kMaxUnison] = {}, gR[kMaxUnison] = {}, gM[kMaxUnison] = {};
    float drCur[kMaxUnison] = {}, drTgt[kMaxUnison] = {}, slop[kMaxUnison] = {};
    int   drTimer[kMaxUnison] = {};
    // analog
    float sph[kMaxUnison] = {}, sinc[kMaxUnison] = {}, y1[kMaxUnison] = {};
    float pw = 0.5f; bool sync = false; int wave = 0;
    // wavetable
    float pos = 0.5f, warpAmt = 0.f; int bank = 0, mip = 0, warp = 0, blend = 0;
    // modulation inputs
    int   fmSrc = 0, fmMode = 0, syncSrc = 0;
    float fmAmt = 0.f;
    // additive
    float aS[kMaxPartials] = {}, aC[kMaxPartials] = {}, aRc[kMaxPartials] = {}, aRs[kMaxPartials] = {};
    float aAmp[kMaxPartials] = {}, aDa[kMaxPartials] = {}, aTgt[kMaxPartials] = {};
    float aPl[kMaxPartials] = {}, aPr[kMaxPartials] = {}, aShPh[kMaxPartials] = {}, aShRate[kMaxPartials] = {}, aDet[kMaxPartials] = {};
    float aRatio[kMaxPartials] = {};
    int   aCount = 0;
    float aLastF0 = -1.f, aLastStretch = -9.f, aLastDet = -1.f, aLastPan = -9.f, aLastSpread = -9.f;
    int   aCtl = 0, aPrevN = 0;
    bool  aHave = false;
    // physical: modal
    float mY1[kMaxModes] = {}, mY2[kMaxModes] = {}, mA1[kMaxModes] = {}, mA2[kMaxModes] = {}, mB[kMaxModes] = {};
    float mPl[kMaxModes] = {}, mPr[kMaxModes] = {}, mRatio[kMaxModes] = {};
    int   mCount = 0;
    float excLP = 0.f, strikeEnv = 0.f, strikeDecay = 0.99f, impulse = 0.f, excCoef = 0.5f, bowPh = 0.f, bowInc = 0.f;
    float burstEnv = 0.f, burstRate = 50.f;
    // 4.2 physical 2.0: pluck / mallet pulse, sample exciter, reed / flute state, per-osc controls
    float excT = 1e9f, excLen = 1.f, excAmp = 0.f, smpPos = 0.f, press = 0.5f, noiseAmt = 0.f, nonlin = 0.f;
    float dcX = 0.f, dcY = 0.f, boreLP = 0.f, jetDelay = 10.f, reedOff = 0.7f, reedSlope = -0.3f, bellC = 0.5f, breathS = 0.f;
    int   jetPos = 0;
    float dc2X = 0.f, dc2Y = 0.f;
    // 5.0 spectral: output phase / blurred magnitude per bin, overlap-add rings (pool memory)
    float* spPh = nullptr; float* spMagS = nullptr; float* spFq = nullptr; float* spOlaL = nullptr; float* spOlaR = nullptr;
    int oscIdx = 0;
    // 6.0 FM oscillator: 6 operators x up to 4 unison voices
    float fmPh[4][6] = {}, fmInc[4][6] = {};
    float fmY1[4] = {}, fmY2[4] = {};
    float fmEnv[6] = {}; int fmStage[6] = {};
    float fmE0[6] = {}, fmE1[6] = {}, fmS0[6] = {}, fmS1[6] = {}, fmDc[6] = {}, fmRnd[6] = {};
    float fmFb0 = 0.f, fmFb1 = 0.f, fmHpX[4] = {}, fmHpY[4] = {}, fmHpR = 0.999f;
    int   fmWv[6] = {}, fmAlg = 4;
    bool  fmFresh = true;
    float* pmBuf = nullptr; int pmW = 0; float pmD = 0.f, pmAcc = 0.f; bool pmOn = false;   // 6.0: PM / FM by delay (Additive, Physical, Spectral)
    int   spN = 0, spH = 0, spBins = 0, spRp = 0, spToNext = 0;
    double spOff = 0.0;
    bool  spFresh = true, spFrozen = false;
    float spFrozenPos = 0.f;
    float spRatio = 1.f, spShiftHz = 0.f, spStretch = 0.f, spTilt = 0.f, spFormant = 0.f, spSparse = 0.f, spCross = 0.f, spBlur = 0.f, spPos = 0.f, spSpeed = 1.f, spSpread = 0.5f;
    int   spCrossMode = 0;
    float spPlay = 0.f;                    // playhead 0..1 (telemetry)
    // physical: waveguide (+ tanpura bridge, sympathetic strings)
    float* wg = nullptr;
    float* sym = nullptr;
    int   wgPos = 0, symPos = 0;
    float wgDelay = 100.f, wgFb = 0.99f, wgLP = 0.f, wgBright = 0.5f, wgTap = 0.3f, wgSpread = 0.5f, buzz = 0.f;
    float symDelay[kSymLines] = {}, symFb = 0.99f, symLP[kSymLines] = {};
    float pmSig[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    float mR[kMaxModes] = {}, mAmp[kMaxModes] = {}, mNorm[kMaxModes] = {}, mF0 = -1.f;
    // granular
    Grain gr[kMaxGrains];
    float grTimer = 0.f, grScan = 0.f, grInterval = 2000.f, grGain = 0.3f;
    float grPos = 0.3f, grSpray = 0.f, grSizeS = 4000.f, grPitchRnd = 0.f, grRev = 0.f, grSpread = 0.5f, grBaseInc = 0.f;
    int   grSource = 0, grMax = 24;
    // mix (after modulation)
    bool  on = false;
    int   type = 1;
    float level = 0.f, panL = 0.7071f, panR = 0.7071f, pan = 0.f;
    float hz = 220.f;
};

struct Voice {
    bool  active = false, gate = false, sustained = false, fromGen = false;
    int   ch = 0, note = 60;
    float vel = 1.f, rnd = 0.f, alt = 1.f;
    uint64_t age = 0;
    int   stealCount = 0;
    bool  pending = false;
    int   pCh = 0, pNote = 60; float pVel = 1.f; bool pGen = false;

    float pitch = 60.f, target = 60.f;     // semitones (glide); target is the tuned pitch of the note
    bool  retuning = false;                // target moved while held (tuning change)
    float neTuneS = 0.f, neVibPh = 0.f;    // note-expression tuning (smoothed), vibrato phase
    OscVoice o[3];
    Env   fe, ae, me;
    EnvCoefs ec[3]; bool envMod = false;     // 6.0: per-voice coefficients while an envelope stage is modulated
    LfoState lfo[kLfos];
    MsegState ms[kMsegs];
    DriftGen rndV;
    float bendS = 0.f, pressS = 0.f, slideS = 0.f, polyAT = 0.f;
    float lfoOut[kLfos] = {}, lfoRateMod[kLfos] = {}, msegRateMod[kMsegs] = {};
    Rng   vr;                              // 4.2: per-voice noise source (voices render on any thread)
    bool  restart = false;                 // 4.2: a steal fade finished; the main thread starts the pending note
    int   restartBlk = 0;
    float src[kModSrcCount] = {};
    float md[kModDstCount] = {};

    // filters
    FilterCoefs c1, c2, c1b, c2b;
    float f1Log = 10.f, f2Log = 10.f, f1Prev = 10.f, f2Prev = 10.f, env1 = 0.f, env2 = 0.f;
    float res1 = 0.f, res2 = 0.f, drv1 = 0.f, drv2 = 0.f, balance = 0.5f, morph1 = 0.f, morph2 = 0.f, fmAmt1 = 0.f, fmAmt2 = 0.f;
    int   ft1 = 1, ft2 = 0, ft1b = 0, ft2b = 0, routing = 0, dt1 = 0, dt2 = 0, fmSrc1 = 0, fmSrc2 = 0;
    // mix
    float subL = 0.f, noiseL = 0.f, ringL = 0.f, gain = 1.f, gainPrev = 1.f;
    float subPhase = 0.f, subInc = 0.f; int subMip = 0, subFrame = 0;
    float pink[3] = { 0, 0, 0 };
    bool  firstBlock = true;
    float prevM[3][kBlock] = {}, prevW[3][kBlock] = {};

    // telemetry mirror
    float telT[3][4] = {}, telCut1 = 1000.f, telCut2 = 1000.f;

    MultiFilter f1L, f1R, f2L, f2R, f1bL, f1bR, f2bL, f2bR;
    Os2Shaper os1L, os1R, os2L, os2R;   // oversampled drive (Normal quality)
};

// ------------------------------------------------------------------------------------------------
class Engine final : public drift::IEngine {
public:
    float p[kEngineArr];   // one layer's parameters + the shared layer setup
    float fs = 48000.f;       // internal voice rate (2x in HQ)
    float fsOut = 48000.f;

    Engine() {
        for (uint32_t i = 0; i < kEngineArr; ++i) p[i] = 0.f;
        builtin = &drift::builtinTables();
        const size_t per = (size_t)kWgSize + (size_t)kSymLines * kSymSize;
        wgPool = new float[(size_t)kMaxVoices * 3 * per];
        std::memset(wgPool, 0, sizeof(float) * (size_t)kMaxVoices * 3 * per);
        for (int v = 0; v < kMaxVoices; ++v)
            for (int o = 0; o < 3; ++o) {
                float* base = wgPool + ((size_t)v * 3 + o) * per;
                voices[v].o[o].wg = base;
                voices[v].o[o].sym = base + kWgSize;
            }
        for (int t = 0; t < kBaseSpectra; ++t) DRIFT_DSP_NS::baseSpectrum(t, spec[t]);
        for (int i = 0; i < kMaxPartials; ++i) lg2h[i] = std::log2((float)(i + 1));
        for (int i = 0; i < kMaxModes; ++i) { invSqrt[i] = 1.f / std::sqrt((float)(i + 1)); invPow06[i] = std::pow((float)(i + 1), -0.6f); }
        for (int o = 0; o < 6; ++o) { for (int i = 0; i < kMaxPartials; ++i) custom[o][0][i] = custom[o][1][i] = 1.f / (float)(i + 1); customIdx[o].store(0); }
        for (int l = 0; l < kLfos; ++l) { drift::lfoShapeDefault(lfoShape[l][0], l); std::memcpy(lfoShape[l][1], lfoShape[l][0], sizeof(lfoShape[l][0])); lfoShapeIdx[l].store(0); }
        for (int m = 0; m < kMsegs; ++m) { drift::msegDefault(mseg[m][0], m); mseg[m][1] = mseg[m][0]; msegIdx[m].store(0); }
        for (auto& sl : slots) { sl.pending.store(nullptr); sl.active = nullptr; for (auto& r : sl.retired) r.store(nullptr); }
        for (auto& sl : sslots) { sl.pending.store(nullptr); sl.active = nullptr; for (auto& r : sl.retired) r.store(nullptr); }
        designHalfband();
        fx.setSampleRate(fsOut);
        for (int i = 0; i < kModDstCount; ++i) gmd[i] = 0.f;
        for (int v = 0; v < kMaxVoices; ++v) voices[v].vr.s = 0x9E3779B9u * (uint32_t)(v + 1) + 0x7F4A7C15u;
        motion[0] = new MotionData(); motion[1] = new MotionData();
        // 5.0 spectral
        spScr = new SpScratch[drift::kMaxPoolSlots];
        const size_t spPer = (size_t)kSpMaxBins * 3 + (size_t)kSpMaxN * 2;
        spPool = new float[(size_t)kMaxVoices * 3 * spPer];
        pmPool = new float[(size_t)kMaxVoices * 3 * 3 * kPmBufN]();
        for (int v = 0; v < kMaxVoices; ++v) for (int o = 0; o < 3; ++o) { voices[v].o[o].pmBuf = pmPool + ((size_t)v * 3 + o) * 3 * kPmBufN; voices[v].o[o].oscIdx = o; }
        std::memset(spPool, 0, sizeof(float) * (size_t)kMaxVoices * 3 * spPer);
        for (int v = 0; v < kMaxVoices; ++v)
            for (int o = 0; o < 3; ++o) {
                float* b = spPool + ((size_t)v * 3 + o) * spPer;
                OscVoice& ov = voices[v].o[o];
                ov.spPh = b; ov.spMagS = b + kSpMaxBins; ov.spFq = b + 2 * kSpMaxBins; ov.spOlaL = b + 3 * kSpMaxBins; ov.spOlaR = ov.spOlaL + kSpMaxN;
            }
        for (int k = 0; k < 2; ++k) {
            const int N = k ? 4096 : 2048;
            spFft[k].init(N);
            spWin[k].resize((size_t)N);
            for (int i = 0; i < N; ++i) spWin[k][(size_t)i] = 0.5f - 0.5f * (float)std::cos(6.283185307179586 * i / N);
        }
        for (auto& row : spslots) for (auto& sl : row) { sl.pending.store(nullptr); sl.active = nullptr; for (auto& r : sl.retired) r.store(nullptr); }
        for (int o = 0; o < 3; ++o) { drift::modalDefault(modal[o][0]); modal[o][1] = modal[o][0]; modalIdx[o].store(0); modalGen[o].store(1); }
        for (int m = 0; m < kMacros; ++m) macroEff[m] = 0.f;
    }
    ~Engine() override {
        for (auto& sl : slots) {
            drift::deleteWaveTable(sl.pending.exchange(nullptr));
            drift::deleteWaveTable(sl.active);
            for (auto& r : sl.retired) drift::deleteWaveTable(r.exchange(nullptr));
        }
        for (auto& sl : sslots) {
            drift::deleteSample(sl.pending.exchange(nullptr));
            drift::deleteSample(sl.active);
            for (auto& r : sl.retired) drift::deleteSample(r.exchange(nullptr));
        }
        delete[] wgPool;
        delete motion[0]; delete motion[1];
        for (auto& row : spslots) for (auto& sl : row) {
            drift::deleteSpec(sl.pending.exchange(nullptr));
            drift::deleteSpec(sl.active);
            for (auto& r : sl.retired) drift::deleteSpec(r.exchange(nullptr));
        }
        delete[] spScr; delete[] spPool; delete[] pmPool;
    }

    const char* isaName() const override { return DRIFT_ISA_NAME; }
    int activeVoices() const override { int n = 0; for (const auto& v : voices) n += v.active ? 1 : 0; return n; }

    void setSampleRate(float sr) override {
        fsOut = sr; os = curQuality == 2 ? 2 : 1; fs = fsOut * (float)os;
        fx.setSampleRate(sr); reset();
    }
    void reset() override {
        for (auto& v : voices) { v.active = false; v.gate = false; v.pending = false; v.ae.reset(); v.fe.reset(); v.me.reset(); }
        for (auto& c : chans) c = ChannelState();
        modWheel = 0.f; expression = 1.f; breath = 0.f; sustain = false; stackN = 0; heldN = 0; latchN = 0;
        std::memset(keyCount, 0, sizeof(keyCount)); lastPolyMode = -1; lastArp = false;
        arpRunning = false; seqRunning = false; arpNoteOn = -1; seqNoteOn = -1;
        fx.reset();
        std::memset(hbL, 0, sizeof(hbL)); std::memset(hbR, 0, sizeof(hbR)); hbPos = 0;
        f1Smooth = std::log2(clampf(p[pF1Cutoff], 16.f, 20000.f));
        f2Smooth = std::log2(clampf(p[pF2Cutoff], 16.f, 20000.f));
        masterS = dbToGain(p[pMaster]);
    }
    void setParam(uint32_t i, float v) override { if (i < kEngineParams || isExtParam(i)) p[i] = v; }
    void setLayer(int index) override { layerIdx = index ? 1 : 0; }
    void setTransport(double bpm, bool playing, double beat) override {
        hostBpm = bpm > 20.0 && bpm < 999.0 ? bpm : 120.0;
        hostPlaying = playing;
        hostBeat = beat;
    }
    void setTelemetry(Telemetry* t) override { tel = t; }
    void setEnvClassic(bool on) override { envClassic.store(on, std::memory_order_relaxed); }
    int pressMode = 0;
    void setPressMode(int mode) override { pressMode = mode < 0 ? 0 : (mode > 2 ? 2 : mode); }
    void setGuardCap(int cap) override { guardCap.store(cap < 1 ? 1 : (cap > kMaxVoices ? kMaxVoices : cap), std::memory_order_relaxed); }

    void postTable(int osc, WaveTable* t) override {
        if (osc < 0 || osc >= 3) { drift::deleteWaveTable(t); return; }
        TSlot& sl = slots[osc];
        for (auto& r : sl.retired) drift::deleteWaveTable(r.exchange(nullptr));
        if (!t) { t = new WaveTable(); t->frames = 0; }
        drift::deleteWaveTable(sl.pending.exchange(t, std::memory_order_acq_rel));
    }
    void postSample(int osc, SampleBuf* s) override {
        if (osc < 0 || osc >= 3) { drift::deleteSample(s); return; }
        SSlot& sl = sslots[osc];
        for (auto& r : sl.retired) drift::deleteSample(r.exchange(nullptr));
        if (!s) { s = new SampleBuf(); s->len = 0; }
        drift::deleteSample(sl.pending.exchange(s, std::memory_order_acq_rel));
    }
    void setCustomSpectrum(int osc, const float* a) override {
        if (osc < 0 || osc >= 6) return;   // 6.0: 3..5 = Custom 2 of osc 0..2
        const int w = 1 - customIdx[osc].load(std::memory_order_acquire);
        for (int i = 0; i < kMaxPartials; ++i) custom[osc][w][i] = clampf(a[i], 0.f, 1.f);
        customIdx[osc].store(w, std::memory_order_release);
        for (auto& v : voices) v.o[osc % 3].aHave = false;    // benign race: only forces a recompute
    }
    void setLfoShape(int l, const float* pts) override {
        if (l < 0 || l >= kLfos) return;
        const int w = 1 - lfoShapeIdx[l].load(std::memory_order_acquire);
        for (int i = 0; i < drift::kLfoShapePoints; ++i) lfoShape[l][w][i] = clampf(pts[i], -1.f, 1.f);
        lfoShapeIdx[l].store(w, std::memory_order_release);
    }
    void setMseg(int m, const MsegData& d) override {
        if (m < 0 || m >= kMsegs) return;
        const int w = 1 - msegIdx[m].load(std::memory_order_acquire);
        mseg[m][w] = d;
        msegIdx[m].store(w, std::memory_order_release);
    }
    void setFxOrder(const int* order) override { fx.setOrder(order); }
    void setPerfAxes(const int* a) override { for (int k = 0; k < 4; ++k) xyAxes[k].store(a[k] < 0 ? 0 : (a[k] > kMacros - 1 ? kMacros - 1 : a[k]), std::memory_order_relaxed); }
    std::atomic<int> xyAxes[4] = { {0}, {1}, {2}, {3} };
    double xyPh[2] = { 0.0, 0.0 };
    // 5.1.1 XY auto-motion: a shape on [-1, 1]^2 at phase ph (0..1); dir mirrors the Y axis (other way round)
    static void xyShape(int mode, double ph, float& x, float& y) {
        const float th = (float)(ph * 6.283185307179586);
        switch (mode) {
        case 1: x = std::cos(th); y = std::sin(th); break;                                   // circle
        case 2: x = std::cos(th); y = 0.45f * std::sin(th); break;                           // ellipse (wide)
        case 3: x = std::sin(th); y = 0.8f * std::sin(2.f * th); break;                      // figure 8
        case 4: x = std::sin(3.f * th); y = std::sin(2.f * th + 0.5f); break;                // Lissajous 3:2
        case 5: { const float r = 0.15f + 0.85f * (0.5f - 0.5f * std::cos((float)(ph / 8.0 * 6.283185307179586)));   // spiral in / out over 8 turns
                  x = r * std::cos(th); y = r * std::sin(th); break; }
        case 6: { const float a = 1.1f * std::sin(th); x = std::sin(a) / std::sin(1.1f);   // pendulum: lowest in the middle of the swing
                  y = -1.f + 2.f * (1.f - std::cos(a)) / (1.f - std::cos(1.1f)); break; }
        case 7: x = 0.62f * std::sin(th) + 0.38f * std::sin(2.31f * th + 1.3f);              // wander: two incommensurate sines per axis
                y = 0.62f * std::sin(1.17f * th + 0.5f) + 0.38f * std::sin(2.73f * th + 2.1f); break;
        default: x = y = 0.f;
        }
    }
    void setScenes(const SceneData& d) override {
        const int w = 1 - sceneIdx.load(std::memory_order_acquire);
        scenes[w] = d;
        sceneIdx.store(w, std::memory_order_release);
    }
    void setMotion(const MotionData& d) override {
        const int w = 1 - motionIdx.load(std::memory_order_acquire);
        MotionData& m = *motion[w];
        m.seconds = d.seconds; m.frames = d.frames; m.recording = d.recording; m.recMask = d.recMask;
        for (int k = 0; k < kMacros; ++k) { m.has[k] = d.has[k]; if (d.has[k] && d.frames > 0) std::memcpy(m.v[k], d.v[k], sizeof(float) * (size_t)d.frames); }
        motionIdx.store(w, std::memory_order_release);
    }
    void setModes(int osc, const ModalData& d) override {
        if (osc < 0 || osc >= 3) return;
        const int w = 1 - modalIdx[osc].load(std::memory_order_acquire);
        modal[osc][w] = d;
        if (modal[osc][w].count <= 0) drift::modalDefault(modal[osc][w]);
        modalIdx[osc].store(w, std::memory_order_release);
        modalGen[osc].fetch_add(1, std::memory_order_release);
    }
    void setWorkerPool(drift::IWorkerPool* p) override { pool.store(p, std::memory_order_release); }
    void postSpectral(int osc, int which, SpecData* d) override {
        if (osc < 0 || osc >= 3 || which < 0 || which > 1) { drift::deleteSpec(d); return; }
        SpSlot& sl = spslots[osc][which];
        for (auto& r : sl.retired) drift::deleteSpec(r.exchange(nullptr));
        if (!d) { d = new SpecData(); d->frames = 0; }
        drift::deleteSpec(sl.pending.exchange(d, std::memory_order_acq_rel));
    }

    // ---- MIDI ------------------------------------------------------------------------------
    void midi(const uint8_t* d, uint32_t size) override {
        if (size < 1) return;
        if (d[0] == 0xF4) { if (size == 12) noteExpression(d); return; }
        const uint8_t st = d[0] & 0xF0;
        const int ch = d[0] & 0x0F;
        const uint8_t d1 = size > 1 ? d[1] : 0, d2 = size > 2 ? d[2] : 0;
        switch (st) {
        case 0x90: if (d2 > 0) { resetNoteExpression(ch, d1); inputNoteOn(ch, d1, d2 / 127.f); } else inputNoteOff(ch, d1); break;
        case 0x80: inputNoteOff(ch, d1); break;
        case 0xA0: for (auto& v : voices) if (v.active && v.ch == ch && v.note == d1) v.polyAT = d2 / 127.f; break;
        case 0xD0: chans[ch].pressure = d1 / 127.f; break;
        case 0xE0: chans[ch].bend = clampf(((int)((d2 << 7) | d1) - 8192) / 8191.f, -1.f, 1.f); break;
        case 0xB0: cc(ch, d1, d2); break;
        default: break;
        }
    }

    // ---- audio -----------------------------------------------------------------------------
    void process(float* outL, float* outR, uint32_t n) override {
        const int q = iparam(pQuality, 0, 2);
        if (q != curQuality) {
            curQuality = q;
            const int nos = q == 2 ? 2 : 1;
            if (nos != os) {
                os = nos; fs = fsOut * (float)os;
                // keep voices; force everything rate-dependent to be recomputed
                for (auto& v : voices) for (auto& ov : v.o) { ov.aLastF0 = -1.f; ov.pmSig[0] = -1.f; for (auto& g : ov.gr) g.on = false; }
                std::memset(hbL, 0, sizeof(hbL)); std::memset(hbR, 0, sizeof(hbR)); hbPos = 0;
            }
        }
        buildMatrix();
        prepCtlK();
        // clock
        const double bpsmp = hostBpm / 60.0 / (double)fsOut;
        if (hostPlaying) {
            if (!wasPlaying || std::fabs(hostBeat - beat) > 0.05) { beat = hostBeat; resyncClock = true; }
        }
        wasPlaying = hostPlaying;
        fx.bpm = hostBpm;
        if (resyncClock) { resyncNoteGen(); resyncClock = false; }

        uint32_t off = 0;
        while (off < n) {
            fireDueEvents();
            uint32_t seg = n - off;
            const double nb = nextEventBeat();
            if (nb < 1e300) {
                const double ds = (nb - beat) / bpsmp;
                uint32_t s = ds <= 0.0 ? 0u : (uint32_t)std::ceil(ds);
                if (s == 0) s = 1;
                if (s < seg) seg = s;
            }
            uint32_t done = 0;
            while (done < seg) {
                const uint32_t c = seg - done < (uint32_t)kMaxChunk ? seg - done : (uint32_t)kMaxChunk;
                processChunk(outL + off + done, outR + off + done, (int)c);
                done += c;
            }
            beat += bpsmp * (double)seg;
            off += seg;
        }
        fireDueEvents();
        if (hostPlaying) hostBeat += bpsmp * (double)n;
        telSamples += (int)n;
        if (tel && telSamples >= (int)(fsOut / 90.f)) { telSamples = 0; writeTelemetry(); }
    }

public:
    // ---- tuning ---------------------------------------------------------------------------------
    void setUserScale(const drift::ScaleDef* sc, const drift::KbmDef* kbm) override {
        const int w = 1 - userIdx.load(std::memory_order_acquire);
        userScl[w] = sc ? *sc : drift::ScaleDef();
        userHasScl[w] = sc != nullptr;
        userKbm[w] = kbm ? *kbm : drift::KbmDef();
        userIdx.store(w, std::memory_order_release);
        userGen.fetch_add(1, std::memory_order_release);
    }
    void setTuningProvider(drift::ITuningProvider* t) override { provider.store(t, std::memory_order_release); }
    const float* tuningTable() const { return tuneTab; }

private:
    drift::ScaleDef userScl[2];
    drift::KbmDef userKbm[2];
    bool userHasScl[2] = { false, false };
    std::atomic<int> userIdx { 0 };
    std::atomic<uint32_t> userGen { 0 };
    std::atomic<drift::ITuningProvider*> provider { nullptr };
    float tuneTab[128];
    int tnKeySrc = -1, tnKeyScale = -1, tnKeyRoot = -1, tnKeySnap = -1; float tnKeyRef = -1.f; uint32_t tnKeyGen = 0xffffffffu;
    int tnSrc = 0; float tnAmt = 1.f, tnRefOff = 0.f; bool tnMts = false;

    // called once per chunk: rebuild the key table when the tuning settings change
    void updateTuning() {
        const int src = iparam(pTnSource, 0, 3), sci = iparam(pTnScale, 0, drift::kBuiltinScales - 1), root = iparam(pTnRoot, 0, 11);
        const int snap = p[pTnSnap] >= 0.5f ? 1 : 0;
        const float ref = clampf(p[pTnRef], 300.f, 600.f);
        const uint32_t gen = userGen.load(std::memory_order_acquire);
        tnSrc = src; tnAmt = clampf(p[pTnAmount], 0.f, 1.f); tnRefOff = 12.f * std::log2(ref / 440.f);
        drift::ITuningProvider* pv = provider.load(std::memory_order_acquire);
        tnMts = src == 3 && pv && pv->connected();
        if (src == tnKeySrc && sci == tnKeyScale && root == tnKeyRoot && snap == tnKeySnap && ref == tnKeyRef && gen == tnKeyGen) return;
        tnKeySrc = src; tnKeyScale = sci; tnKeyRoot = root; tnKeySnap = snap; tnKeyRef = ref; tnKeyGen = gen;
        if (src == 1) drift::buildTuningTable(drift::builtinScales()[sci], nullptr, root, ref, snap != 0, tuneTab);
        else if (src == 2) {
            const int u = userIdx.load(std::memory_order_acquire);
            if (userHasScl[u]) drift::buildTuningTable(userScl[u], userKbm[u].valid ? &userKbm[u] : nullptr, root, ref, snap != 0, tuneTab);
            else for (int k = 0; k < 128; ++k) tuneTab[k] = (float)k + tnRefOff;
        } else for (int k = 0; k < 128; ++k) tuneTab[k] = (float)k + tnRefOff;
    }
    float tunedPitch(int note, int ch) {
        note &= 127;
        const float et = (float)note + tnRefOff;
        float t;
        if (tnSrc == 3) {
            drift::ITuningProvider* pv = provider.load(std::memory_order_relaxed);
            if (!tnMts || !pv) return et;
            t = (float)pv->noteSemis(note, ch);
            const float e0 = (float)note;
            return e0 + (t - e0) * tnAmt;
        }
        if (tnSrc == 0) return et;
        t = tuneTab[note];
        return et + (t - et) * tnAmt;
    }

    // ---- VST3 note expression (0xF4 type ch key + double, see the DPF patch) ----------------------
    struct NeKey { float vol = 1.f, pan = 0.f, tune = 0.f, vib = 0.f, expr = 0.f, bright = 0.f; };
    NeKey neKeys[16][128];
    static bool snapNe(const Voice& v) { return v.firstBlock; }
    void noteExpression(const uint8_t* d) {
        double val; std::memcpy(&val, d + 4, sizeof(double));
        const float x = (float)(val < 0.0 ? 0.0 : (val > 1.0 ? 1.0 : val));
        NeKey& k = neKeys[d[2] & 15][d[3] & 127];
        switch (d[1]) {
        case 0: k.vol = x > 0.f ? 4.f * x : 0.f; if (k.vol > 4.f) k.vol = 4.f; break;     // 0.25 = unity, 1 = +12 dB
        case 1: k.pan = (x - 0.5f) * 2.f; break;
        case 2: k.tune = (x - 0.5f) * 240.f; break;                                      // semitones
        case 3: k.vib = x; break;
        case 4: k.expr = x; break;
        case 5: k.bright = (x - 0.5f) * 2.f; break;
        default: break;
        }
    }
    void resetNoteExpression(int ch, int note) { neKeys[ch & 15][note & 127] = NeKey(); }

    // ---- layers (4.1): this engine is layer 1 or 2 of the patch -------------------------------------
    int layerIdx = 0;
    // position 0 = all layer 1, 1 = all layer 2, for the current mode
    float layerPos(int note, float vel, float press) const {
        const int mode = iparam(pLyMode, 0, 5);
        const float w = p[pLyWidth], m = p[pLyMorph];
        switch (mode) {
        case 2: { if (w < 0.01f) return (float)note >= p[pLySplit] ? 1.f : 0.f;
                  return clampf(((float)note - p[pLySplit] + 0.5f) / w + 0.5f, 0.f, 1.f); }   // split around the split key
        case 3: { const float ww = w / 48.f + 0.02f; return clampf((vel - m) / ww + 0.5f, 0.f, 1.f); }
        case 4: { const float ww = w / 48.f + 0.02f; return clampf((press - m) / ww + 0.5f, 0.f, 1.f); }
        case 5: return m;
        default: return 0.5f;
        }
    }
    float layerGain(int note, float vel, float press) const {
        const int mode = iparam(pLyMode, 0, 5);
        if (mode == 0) return layerIdx == 0 ? 1.f : 0.f;
        if (mode == 1) return 1.f;
        const float s = layerPos(note, vel, press);
        float sa, ca; fastSinCos(s * 0.5f * kPiF, sa, ca);
        return layerIdx == 0 ? ca : sa;                  // equal-power crossfade
    }
    bool layerAccepts(int note, float vel) const {
        const int mode = iparam(pLyMode, 0, 5);
        if (mode == 0) return layerIdx == 0;
        if (mode != 2 && mode != 3) return true;         // pressure / morph can change while the note plays
        const float s = layerPos(note, vel, 0.f);
        return layerIdx == 0 ? s < 0.999f : s > 0.001f;
    }

    std::atomic<bool> envClassic { false };
    std::atomic<int>  guardCap { kMaxVoices };
    Voice voices[kMaxVoices];
    ChannelState chans[16];
    float modWheel = 0.f, expression = 1.f, breath = 0.f;
    bool  sustain = false;
    uint64_t ageCounter = 0;
    Rng rng;
    float lastNoteOnPitch = 60.f;
    float altState = 1.f;
    int   curQuality = 1, os = 1;
    bool  genCtx = false;          // true while a note generator calls noteOn/noteOff

    // mono note stack
    struct HeldNote { int ch, note; float vel; };
    HeldNote stack[32];
    int stackN = 0;

    // note generators (arp / sequencer)
    HeldNote held[32]; int heldN = 0;            // keys physically down, in press order
    // 5.1.1: two keyboards on one channel: a key sounds until every press of it is released
    uint8_t keyCount[16][128] = {};
    // 5.1.1: voice-mode / generator switches seen last block (preset changes while keys are down)
    int lastPolyMode = -1; bool lastArp = false;
    HeldNote latch[32]; int latchN = 0;          // arp hold
    bool   latchArmed = false;
    double hostBpm = 120.0, hostBeat = 0.0, beat = 0.0;
    bool   hostPlaying = false, wasPlaying = false, resyncClock = true;
    bool   arpRunning = false, seqRunning = false;
    double arpOrigin = 0.0, seqOrigin = 0.0;
    long   arpStepIdx = 0, seqStepIdx = 0;
    double arpNextOn = 1e301, arpNextOff = 1e301, seqNextOn = 1e301, seqNextOff = 1e301;
    int    arpNoteOn = -1, seqNoteOn = -1, arpChord[32], arpChordN = 0, arpPatPos = 0;
    int    seqCur = 0;
    float  seqCv = 0.f, seqCvTarget = 0.f;

    const BuiltinTables* builtin = nullptr;
    float* wgPool = nullptr;
    float spec[kBaseSpectra][kMaxPartials];
    float lg2h[kMaxPartials];
    float invSqrt[kMaxModes], invPow06[kMaxModes];
    static constexpr int kPmBufN = 8192;
    float* pmPool = nullptr;
    float custom[6][2][kMaxPartials];          // 6.0: 0-2 Custom, 3-5 Custom 2
    std::atomic<int> customIdx[6];
    float lfoShape[kLfos][2][drift::kLfoShapePoints];
    std::atomic<int> lfoShapeIdx[kLfos];
    MsegData mseg[kMsegs][2];
    std::atomic<int> msegIdx[kMsegs];

    struct TSlot { std::atomic<WaveTable*> pending; WaveTable* active; std::atomic<WaveTable*> retired[4]; };
    struct SSlot { std::atomic<SampleBuf*> pending; SampleBuf* active; std::atomic<SampleBuf*> retired[4]; };
    TSlot slots[3];
    SSlot sslots[3];

    EnvCoefs ecF, ecA, ecM;
    LfoState gLfo[kLfos];
    float gLfoVal[kBlocksPerChunk][kLfos];
    float gLfoLast[kLfos] = {}, gLfoRate[kLfos] = {};
    DriftGen rndG;
    float f1Smooth = 10.f, f2Smooth = 10.f, f1Blk[kBlocksPerChunk], f2Blk[kBlocksPerChunk];
    float masterS = 0.5f;
    float gmd[kModDstCount];
    Rng fmRng;                 // 6.0: FM oscillator randoms (kept apart from rng)
    float tx[kTimbreX] = {};   // 5.1.7: Timbre 5..10 of the oscillator being updated
    FxChain fx;
    float winTab[3][kWinSize + 1];
    float winKey[3] = { -1, -1, -1 };

    // half-band decimator
    float hbC[kHbTaps];
    float hbL[kHbTaps * 2], hbR[kHbTaps * 2];
    int   hbPos = 0;
    float vL[kMaxChunkI], vR[kMaxChunkI];

    // scratch for one block, one set per render thread (4.2)
    struct Scratch {
        float bL[3][kBlock], bR[3][kBlock], bM[3][kBlock], bW[3][kBlock];
        float feB[kBlock], aeB[kBlock], subB[kBlock], noiseB[kBlock];
        float mixPk[6];
    };
    Scratch scr[drift::kMaxPoolSlots];
    // 5.0 spectral work arrays, one set per render thread (heap: large)
    struct SpScratch {
        float re[kSpMaxN], im[kSpMaxN];
        float mag[kSpMaxBins], fhz[kSpMaxBins], phi[kSpMaxBins], envA[kSpMaxBins], envB[kSpMaxBins];
        float nMag[kSpMaxBins], nPh[kSpMaxBins], nPan[kSpMaxBins], nFq[kSpMaxBins], pre[kSpMaxBins + 1];
        int   peaks[kSpMaxBins], lo[kSpMaxBins], hi[kSpMaxBins];
    };
    SpScratch* spScr = nullptr;            // kMaxPoolSlots of them
    float* spPool = nullptr;
    SpecFFT spFft[2];                       // 2048, 4096
    std::vector<float> spWin[2];
    struct SpSlot { std::atomic<SpecData*> pending; SpecData* active; std::atomic<SpecData*> retired[4]; };
    SpSlot spslots[3][2];
    // per-voice output of the current chunk; summed in voice order so any thread count gives the same result
    float vbL[kMaxVoices][kMaxChunkI], vbR[kMaxVoices][kMaxChunkI];
    int   taskList[kMaxVoices], taskCount = 0, taskNb = 0, taskNI = 0;
    std::atomic<drift::IWorkerPool*> pool { nullptr };

    // 4.2 scenes / motion / analysed modes
    SceneData scenes[2];
    std::atomic<int> sceneIdx { 0 };
    MotionData* motion[2] = { nullptr, nullptr };
    std::atomic<int> motionIdx { 0 };
    ModalData modal[3][2];
    std::atomic<int> modalIdx[3];
    std::atomic<uint32_t> modalGen[3];
    float macroEff[kMacros] = {};
    bool  macroLane[kMacros] = {};
    float scPosS = 0.f;
    double motT = 0.0;
    bool  motRestart = false;
    float rsPitch[8] = {};                 // resonator string pitches (semitones)
    bool  rsHaveHeld = false;

    Telemetry* tel = nullptr;
    int telSamples = 0;
    float scopeDecim = 0.f;
    float peakL = 0.f, peakR = 0.f;
    float mixPk[6] = {};

    static float dbToGain(float db) { return std::pow(10.f, db / 20.f); }
    int iparam(uint32_t id, int lo, int hi) const { int v = (int)(p[id] + (p[id] >= 0 ? 0.5f : -0.5f)); return v < lo ? lo : (v > hi ? hi : v); }
    float op(int o, uint32_t f) const { return p[oscP(o, f)]; }
    int  oiparam(int o, uint32_t f, int lo, int hi) const { return iparam(oscP(o, f), lo, hi); }
    int  partialCap() const { return curQuality == 0 ? 32 : (curQuality == 1 ? 64 : 128); }
    int  modeCap() const { return curQuality == 0 ? 12 : 24; }
    int  grainCap() const { return curQuality == 0 ? 12 : (curQuality == 1 ? 24 : 32); }
    int  unisonCap() const { return curQuality == 0 ? 4 : kMaxUnison; }

    void designHalfband() {
        const int M = kHbTaps - 1, c = M / 2;
        const double beta = 9.0;
        auto bessel0 = [](double x) { double s = 1, t = 1; for (int k = 1; k < 30; ++k) { t *= (x / (2 * k)) * (x / (2 * k)); s += t; } return s; };
        double sum = 0;
        for (int i = 0; i <= M; ++i) {
            const int k = i - c;
            const double sinc = k == 0 ? 0.5 : std::sin(3.14159265358979 * k * 0.5) / (3.14159265358979 * k);
            const double r = (double)(2 * i - M) / M;
            const double w = bessel0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / bessel0(beta);
            hbC[i] = (float)(sinc * w);
            sum += hbC[i];
        }
        for (int i = 0; i <= M; ++i) hbC[i] = (float)(hbC[i] / sum);
    }
    // 2:1 decimation of n output samples from 2n inputs
    void decimate(const float* inL, const float* inR, float* outL, float* outR, int n) {
        const int c = (kHbTaps - 1) / 2;
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < 2; ++k) {
                hbL[hbPos] = hbL[hbPos + kHbTaps] = inL[2 * i + k];
                hbR[hbPos] = hbR[hbPos + kHbTaps] = inR[2 * i + k];
                hbPos = hbPos + 1 >= kHbTaps ? 0 : hbPos + 1;
            }
            // hbPos is the oldest sample; history runs hbL[hbPos .. hbPos+kHbTaps-1]
            const float* hl = hbL + hbPos; const float* hr = hbR + hbPos;
            float yl = hbC[c] * hl[c], yr = hbC[c] * hr[c];
            for (int j = 1; j <= c; j += 2) {
                yl += hbC[c - j] * (hl[c - j] + hl[c + j]);
                yr += hbC[c - j] * (hr[c - j] + hr[c + j]);
            }
            outL[i] = yl; outR[i] = yr;
        }
    }

    void consumePending() {
        for (auto& sl : slots) {
            WaveTable* nt = sl.pending.exchange(nullptr, std::memory_order_acq_rel);
            if (!nt) continue;
            WaveTable* old = sl.active;
            sl.active = nt->frames > 0 ? nt : nullptr;
            retireT(sl, old);
            if (nt->frames == 0) retireT(sl, nt);
        }
        for (auto& row : spslots) for (auto& sl : row) {
            SpecData* nd = sl.pending.exchange(nullptr, std::memory_order_acq_rel);
            if (!nd) continue;
            SpecData* old = sl.active;
            sl.active = nd->frames > 0 ? nd : nullptr;
            retireSp(sl, old);
            if (nd->frames == 0) retireSp(sl, nd);
            for (auto& v : voices) for (auto& ov : v.o) ov.spFresh = true;   // restart the resynthesis on the new data
        }
        for (auto& sl : sslots) {
            SampleBuf* ns = sl.pending.exchange(nullptr, std::memory_order_acq_rel);
            if (!ns) continue;
            SampleBuf* old = sl.active;
            sl.active = ns->len > 0 ? ns : nullptr;
            retireS(sl, old);
            if (ns->len == 0) retireS(sl, ns);
        }
    }
    static void retireSp(SpSlot& sl, SpecData* t) { if (!t) return; for (auto& r : sl.retired) { SpecData* e = nullptr; if (r.compare_exchange_strong(e, t)) return; } }
    static void retireT(TSlot& sl, WaveTable* t) { if (!t) return; for (auto& r : sl.retired) { WaveTable* e = nullptr; if (r.compare_exchange_strong(e, t)) return; } }
    static void retireS(SSlot& sl, SampleBuf* t) { if (!t) return; for (auto& r : sl.retired) { SampleBuf* e = nullptr; if (r.compare_exchange_strong(e, t)) return; } }

    // ---- note input & generators ----------------------------------------------------------------
    bool arpOn() const { return p[pArpOn] >= 0.5f && iparam(pSeqMode, 0, 2) != 2; }
    bool seqNotes() const { return iparam(pSeqMode, 0, 2) == 2; }
    bool generatorOwnsNotes() const { return arpOn() || seqNotes(); }

    void inputNoteOn(int ch, int note, float vel) {
        if (ch >= 0 && ch < 16 && note >= 0 && note < 128) {
            // self-heal: a count left over from a lost note-off, with nothing sounding for that key, starts again
            if (keyCount[ch][note] > 0 && !generatorOwnsNotes()) {
                bool sounding = false;
                for (const auto& v : voices) if (v.active && v.gate && !v.fromGen && v.ch == ch && v.note == note) sounding = true;
                if (polyMode() != 0) { sounding = false; for (int i = 0; i < stackN; ++i) if (stack[i].ch == ch && stack[i].note == note) sounding = true; }
                if (!sounding) keyCount[ch][note] = 0;
            }
            if (keyCount[ch][note] < 255) ++keyCount[ch][note];
        }
        // keep the held list either way, so switching the arp on mid-chord works
        for (int i = 0; i < heldN; ++i) if (held[i].note == note && held[i].ch == ch) { for (int j = i; j < heldN - 1; ++j) held[j] = held[j + 1]; --heldN; break; }
        if (heldN < 32) held[heldN++] = { ch, note, vel };
        if (!generatorOwnsNotes()) { noteOn(ch, note, vel); return; }
        if (p[pArpHold] >= 0.5f) {
            if (latchArmed) { latchN = 0; latchArmed = false; }
            bool dup = false;
            for (int i = 0; i < latchN; ++i) if (latch[i].note == note) dup = true;
            if (!dup && latchN < 32) latch[latchN++] = { ch, note, vel };
        }
        startGenerators();
    }
    void inputNoteOff(int ch, int note) {
        if (ch >= 0 && ch < 16 && note >= 0 && note < 128) {
            if (keyCount[ch][note] > 1) { --keyCount[ch][note]; return; }   // the same key is still down on the other keyboard
            keyCount[ch][note] = 0;
        }
        for (int i = 0; i < heldN; ++i) if (held[i].note == note && held[i].ch == ch) { for (int j = i; j < heldN - 1; ++j) held[j] = held[j + 1]; --heldN; break; }
        if (!generatorOwnsNotes()) { noteOff(ch, note); return; }
        // keys that started before the generator was switched on still need their release
        for (auto& v : voices) if (v.active && v.gate && !v.fromGen && v.ch == ch && v.note == note) { v.gate = false; if (sustain) v.sustained = true; else releaseVoice(v); }
        if (heldN == 0) latchArmed = true;
        const bool hold = p[pArpHold] >= 0.5f && arpOn();
        if (heldN == 0 && !hold) stopNoteGenerators();
    }
    double stepBeats(uint32_t rateParam) const { return (double)kDivBeats[iparam(rateParam, 0, kDivs - 1)]; }
    double stepStart(double origin, long k, uint32_t rateParam, uint32_t swingParam) const {
        const double L = stepBeats(rateParam);
        return origin + (double)k * L + ((k & 1) ? (double)p[swingParam] * 0.5 * L : 0.0);
    }
    double quantStart(double L) const {
        if (hostPlaying) return std::ceil(beat / L - 1e-9) * L;
        return beat;
    }
    void startGenerators() {
        if (arpOn() && !arpRunning) {
            const double L = stepBeats(pArpRate);
            arpRunning = true; arpPatPos = 0;
            arpOrigin = hostPlaying ? 0.0 : quantStart(L);
            arpStepIdx = hostPlaying ? (long)std::ceil(beat / L - 1e-9) : 0;
            arpNextOn = stepStart(arpOrigin, arpStepIdx, pArpRate, pArpSwing);
        }
        if (seqNotes() && !seqRunning) startSeq();
    }
    void startSeq() {
        const double L = stepBeats(pSeqRate);
        seqRunning = true;
        seqOrigin = hostPlaying ? 0.0 : quantStart(L);
        seqStepIdx = hostPlaying ? (long)std::ceil(beat / L - 1e-9) : 0;
        seqNextOn = stepStart(seqOrigin, seqStepIdx, pSeqRate, pSeqSwing);
    }
    void stopNoteGenerators() {
        if (arpNoteOn >= 0 || arpChordN > 0) releaseArp();
        arpRunning = false; arpNextOn = arpNextOff = 1e301;
        if (seqNotes()) { if (seqNoteOn >= 0) { genOff(seqNoteOn); seqNoteOn = -1; } seqRunning = false; seqNextOn = seqNextOff = 1e301; }
    }
    void genOn(int note, float vel) { genCtx = true; noteOn(0, note, vel); genCtx = false; }
    void genOff(int note) { genCtx = true; noteOff(0, note); genCtx = false; }
    void releaseArp() {
        for (int i = 0; i < arpChordN; ++i) genOff(arpChord[i]);
        arpChordN = 0;
        if (arpNoteOn >= 0) { genOff(arpNoteOn); arpNoteOn = -1; }
    }
    void resyncNoteGen() {
        if (arpRunning) {
            const double L = stepBeats(pArpRate);
            arpOrigin = hostPlaying ? 0.0 : beat;
            arpStepIdx = hostPlaying ? (long)std::ceil(beat / L - 1e-9) : 0;
            arpNextOn = stepStart(arpOrigin, arpStepIdx, pArpRate, pArpSwing);
        }
        if (seqRunning) {
            const double L = stepBeats(pSeqRate);
            seqOrigin = hostPlaying ? 0.0 : beat;
            seqStepIdx = hostPlaying ? (long)std::ceil(beat / L - 1e-9) : 0;
            seqNextOn = stepStart(seqOrigin, seqStepIdx, pSeqRate, pSeqSwing);
        }
    }
    // 5.1.1: a preset change while keys are down must not strand a voice no note-off can reach
    void checkModeSwitches() {
        const int pm = polyMode();
        if (lastPolyMode >= 0 && pm != lastPolyMode) {
            // poly <-> mono: note-offs are routed differently afterwards, so release every gated voice now
            for (auto& v : voices) {
                if (v.pending) v.pending = false;
                if (v.active && v.gate) { v.gate = false; v.sustained = false; releaseVoice(v); }
            }
            stackN = 0;
        }
        lastPolyMode = pm;
        // step sequencer left note mode (to CV or off): its sounding step note is released
        if (!seqNotes() && seqNoteOn >= 0) { genOff(seqNoteOn); seqNoteOn = -1; }
        // arp switched off: forget the latched chord so the next HOLD preset doesn't replay old notes
        const bool ao = arpOn();
        if (lastArp && !ao) latchN = 0;
        lastArp = ao;
    }
    double nextEventBeat() {
        checkModeSwitches();
        // keep generator state consistent with the current switches
        const int sm = iparam(pSeqMode, 0, 2);
        if (sm == 1 && !seqRunning) startSeq();
        if (sm == 0 && seqRunning) { if (seqNoteOn >= 0) { genOff(seqNoteOn); seqNoteOn = -1; } seqRunning = false; seqNextOn = seqNextOff = 1e301; }
        if (!arpOn() && arpRunning) { releaseArp(); arpRunning = false; arpNextOn = arpNextOff = 1e301; }
        double m = 1e301;
        if (arpRunning) { m = std::min(m, arpNextOn); m = std::min(m, arpNextOff); }
        if (seqRunning) { m = std::min(m, seqNextOn); m = std::min(m, seqNextOff); }
        return m;
    }
    void fireDueEvents() {
        for (int guard = 0; guard < 8; ++guard) {
            bool any = false;
            if (arpRunning && arpNextOff <= beat + 1e-9) { releaseArp(); arpNextOff = 1e301; any = true; }
            if (seqRunning && seqNextOff <= beat + 1e-9) { if (seqNoteOn >= 0) { genOff(seqNoteOn); seqNoteOn = -1; } seqNextOff = 1e301; any = true; }
            if (arpRunning && arpNextOn <= beat + 1e-9) { arpStep(); any = true; }
            if (seqRunning && seqNextOn <= beat + 1e-9) { seqStep(); any = true; }
            if (!any) break;
        }
    }
    void arpStep() {
        const HeldNote* src = (p[pArpHold] >= 0.5f && latchN > 0) ? latch : held;
        const int n = (p[pArpHold] >= 0.5f && latchN > 0) ? latchN : heldN;
        const double L = stepBeats(pArpRate);
        const double start = arpNextOn;
        ++arpStepIdx;
        arpNextOn = stepStart(arpOrigin, arpStepIdx, pArpRate, pArpSwing);
        if (n == 0) { arpRunning = false; arpNextOn = arpNextOff = 1e301; return; }
        releaseArp();
        int sorted[32]; float vels[32];
        for (int i = 0; i < n; ++i) { sorted[i] = src[i].note; vels[i] = src[i].vel; }
        const int mode = iparam(pArpMode, 0, 6);
        if (mode != 5) {   // order mode keeps press order
            for (int i = 1; i < n; ++i) { const int k = sorted[i]; const float vk = vels[i]; int j = i - 1; while (j >= 0 && sorted[j] > k) { sorted[j + 1] = sorted[j]; vels[j + 1] = vels[j]; --j; } sorted[j + 1] = k; vels[j + 1] = vk; }
        }
        const int octs = iparam(pArpOct, 1, 4);
        const int total = n * octs;
        float vel = vels[n - 1];
        if (mode == 6) {
            for (int o = 0; o < octs; ++o) for (int i = 0; i < n && arpChordN < 32; ++i) { const int nt = sorted[i] + 12 * o; if (nt <= 127) { genOn(nt, vels[i]); arpChord[arpChordN++] = nt; } }
        } else {
            int idx;
            if (mode == 4) idx = (int)(rng.uni() * (float)total) % total;
            else if (mode == 0 || mode == 5) idx = arpPatPos % total;
            else if (mode == 1) idx = total - 1 - (arpPatPos % total);
            else {
                const int cyc = total > 1 ? 2 * total - 2 : 1;
                const int k = arpPatPos % cyc;
                idx = mode == 2 ? (k < total ? k : cyc - k) : (k < total ? total - 1 - k : k - total + 1);
            }
            ++arpPatPos;
            const int nt = sorted[idx % n] + 12 * (idx / n);
            vel = vels[idx % n];
            if (nt <= 127) { genOn(nt, vel); arpNoteOn = nt; }
        }
        double len = L * (double)p[pArpGate];
        if (start + len > arpNextOn - 1e-4) len = arpNextOn - start - 1e-4;
        arpNextOff = start + len;
        if (tel) { tel->arpStep = arpPatPos; tel->arpNote = arpNoteOn; }
    }
    void seqStep() {
        const int steps = iparam(pSeqSteps, 1, kSeqSteps);
        const double L = stepBeats(pSeqRate);
        const double start = seqNextOn;
        seqCur = (int)(((seqStepIdx % steps) + steps) % steps);
        ++seqStepIdx;
        seqNextOn = stepStart(seqOrigin, seqStepIdx, pSeqRate, pSeqSwing);
        seqCvTarget = p[pSeqV1 + seqCur];
        if (seqNotes()) {
            if (seqNoteOn >= 0) { genOff(seqNoteOn); seqNoteOn = -1; }
            const int n = heldN;
            if (n == 0) { seqRunning = false; seqNextOn = seqNextOff = 1e301; return; }
            if (p[pSeqG1 + seqCur] >= 0.5f) {
                const int base = held[n - 1].note;
                const int nt = base + (int)std::floor(p[pSeqV1 + seqCur] * 24.f + 0.5f);
                if (nt >= 0 && nt <= 127) { genOn(nt, held[n - 1].vel); seqNoteOn = nt; }
                double len = L * (double)p[pSeqGate];
                if (start + len > seqNextOn - 1e-4) len = seqNextOn - start - 1e-4;
                seqNextOff = start + len;
            }
        }
    }

    // ---- voices ------------------------------------------------------------------------------
    int polyMode() const { return iparam(pPolyMode, 0, 2); }
    bool glideOn() const { return p[pGlide] > 0.0015f; }
    int  polyLimit() const { const int l = iparam(pPolyphony, 1, kMaxVoices), g = guardCap.load(std::memory_order_relaxed); return g < l ? (g < 1 ? 1 : g) : l; }
    // CPU guard: fade out voices above the cap, quietest releasing voice first
    void enforceGuard() {
        const int cap = guardCap.load(std::memory_order_relaxed);
        if (cap >= kMaxVoices || polyMode() != 0) return;
        int live = 0;
        for (const auto& v : voices) if (v.active && v.stealCount == 0 && !v.pending) ++live;
        while (live > cap) {
            Voice* best = nullptr;
            for (auto& v : voices) if (v.active && v.stealCount == 0 && !v.pending && !v.gate && (!best || v.ae.v < best->ae.v)) best = &v;
            if (!best) for (auto& v : voices) if (v.active && v.stealCount == 0 && !v.pending && (!best || v.ae.v < best->ae.v)) best = &v;
            if (!best) break;
            best->stealCount = kStealFade; best->gate = false; best->sustained = false;
            --live;
        }
    }

    void noteOn(int ch, int note, float vel) {
        if (!layerAccepts(note, vel)) return;     // split / velocity layers: this layer is silent for the note
        if (polyMode() != 0) { monoNoteOn(ch, note, vel); return; }
        const int lim = polyLimit();
        for (int i = 0; i < lim; ++i) { Voice& v = voices[i]; if (v.active && !v.pending && v.ch == ch && v.note == note && !v.gate) { startVoice(v, ch, note, vel, false); return; } }
        Voice* target = nullptr;
        for (int i = 0; i < lim; ++i) if (!voices[i].active) { target = &voices[i]; break; }
        if (target) { startVoice(*target, ch, note, vel, true); return; }
        Voice* best = nullptr;
        for (int i = 0; i < lim; ++i) { Voice& v = voices[i]; if (!v.gate && !v.pending && (!best || v.ae.v < best->ae.v)) best = &v; }
        if (!best) for (int i = 0; i < lim; ++i) { Voice& v = voices[i]; if (!v.pending && (!best || v.age < best->age)) best = &v; }
        if (!best) best = &voices[0];
        best->pending = true; best->pCh = ch; best->pNote = note; best->pVel = vel; best->pGen = genCtx;
        best->stealCount = kStealFade; best->gate = false; best->sustained = false;
    }
    void noteOff(int ch, int note) {
        if (polyMode() != 0) { monoNoteOff(ch, note); return; }
        for (auto& v : voices) {
            if (v.pending && v.pCh == ch && v.pNote == note && v.pGen == genCtx) { v.pending = false; continue; }
            if (v.active && v.gate && v.ch == ch && v.note == note && v.fromGen == genCtx) {
                v.gate = false;
                if (sustain) v.sustained = true; else releaseVoice(v);
            }
        }
    }
    void releaseVoice(Voice& v) { v.ae.gateOff(); v.fe.gateOff(); v.me.gateOff(); }
    void monoNoteOn(int ch, int note, float vel) {
        for (int i = 0; i < stackN; ++i) if (stack[i].ch == ch && stack[i].note == note) { for (int j = i; j < stackN - 1; ++j) stack[j] = stack[j + 1]; --stackN; break; }
        if (stackN < 32) stack[stackN++] = { ch, note, vel };
        Voice& v = voices[0];
        for (int i = 1; i < kMaxVoices; ++i) if (voices[i].active) { releaseVoice(voices[i]); voices[i].gate = false; }
        const bool legato = polyMode() == 2 && v.active && v.gate && stackN > 1;
        if (legato) { v.ch = ch; v.note = note; v.target = tunedPitch(note, ch); v.bendS = chans[ch].bend; return; }
        startVoice(v, ch, note, vel, !v.active);
    }
    void monoNoteOff(int ch, int note) {
        for (int i = 0; i < stackN; ++i) if (stack[i].ch == ch && stack[i].note == note) { for (int j = i; j < stackN - 1; ++j) stack[j] = stack[j + 1]; --stackN; break; }
        Voice& v = voices[0];
        if (!v.active || v.note != note || v.ch != ch) return;
        if (stackN > 0) { const HeldNote& h = stack[stackN - 1]; v.ch = h.ch; v.note = h.note; v.target = tunedPitch(h.note, h.ch); return; }
        v.gate = false;
        if (sustain) v.sustained = true; else releaseVoice(v);
    }
    void cc(int ch, int num, int val) {
        switch (num) {
        case 1:  modWheel = val / 127.f; break;
        case 2:  breath = val / 127.f; break;
        case 11: expression = val / 127.f; break;
        case 74: chans[ch].slide = clampf((val - 64) / 63.f, -1.f, 1.f); break;
        case 64: {
            const bool on = val >= 64;
            if (sustain && !on)
                for (auto& v : voices) if (v.active && v.sustained) { v.sustained = false; releaseVoice(v); }
            sustain = on;
            break;
        }
        case 120: for (auto& v : voices) { v.active = false; v.pending = false; } stackN = 0; heldN = 0; latchN = 0; std::memset(keyCount, 0, sizeof(keyCount)); stopNoteGenerators(); break;
        case 123: for (auto& v : voices) if (v.active) { v.gate = false; v.sustained = false; releaseVoice(v); } stackN = 0; heldN = 0; latchN = 0; std::memset(keyCount, 0, sizeof(keyCount)); stopNoteGenerators(); break;
        case 121: chans[ch] = ChannelState(); modWheel = 0.f; expression = 1.f; breath = 0.f; break;
        default: break;
        }
    }

    void startVoice(Voice& v, int ch, int note, float vel, bool fresh) {
        { bool held = false; for (const auto& o : voices) if (&o != &v && o.active && o.gate) held = true; if (!held) motRestart = true; }
        const ChannelState& cs = chans[ch];
        if (ch != 0) chans[ch].pressure = 0.f;
        v.active = true; v.gate = true; v.sustained = false; v.pending = false;
        v.ch = ch; v.note = note; v.vel = vel; v.age = ++ageCounter;
        v.rnd = rng.bi();
        altState = -altState; v.alt = altState;
        v.bendS = cs.bend; v.slideS = cs.slide; v.pressS = 0.f; v.polyAT = 0.f;
        v.target = tunedPitch(note, ch);
        v.pitch = glideOn() ? lastNoteOnPitch : v.target;
        v.retuning = false;
        lastNoteOnPitch = v.target;
        v.neVibPh = 0.f;
        for (int i = 0; i < kLfos; ++i) {
            LfoState& l = v.lfo[i];
            const int mode = iparam(lfoP(i, lfMode), 0, 3);
            if (mode >= 2 || fresh) {
                l.phase = mode >= 2 ? p[lfoP(i, lfPhase)] : rng.uni();
                if (mode >= 2 && p[pLRndPh1 + i] > 0.f) { l.phase += p[pLRndPh1 + i] * rng.uni(); l.phase -= std::floor(l.phase); }   // 6.0
                l.sh = rng.bi(); l.shPrev = rng.bi();
                l.rateMul = mode == 1 ? 0.85f + 0.3f * rng.uni() : 1.f;
                l.done = false;
            }
            l.fade = p[lfoP(i, lfFade)] > 0.001f ? 0.f : 1.f;
        }
        for (int m = 0; m < kMsegs; ++m) {
            MsegState& s = v.ms[m];
            const MsegData& d = mseg[m][msegIdx[m].load(std::memory_order_relaxed)];
            s.seg = 1; s.t = 0.f; s.start = fresh ? d.l[0] : s.v; s.v = s.start; s.time = 0.f; s.u = 0.f; s.done = false; s.hold = false;
        }
        v.fromGen = genCtx;
        // 6.0: start phase offsets (A/B/C Phase destinations) from what is known at note-on
        float phMod[3] = { 0.f, 0.f, 0.f };
        {
            bool any = false;
            for (int s = 0; s < kModSlots && !any; ++s) { const int d = iparam(modDstParam(s), 0, kModDstCount - 1); any = d >= mdAPhase && d <= mdCPhase && p[modAmtParam(s)] != 0.f; }
            if (any) {
                float src[kModSrcCount];
                globalSources(src);
                for (int i = 0; i < kLfos; ++i) if (iparam(lfoP(i, lfMode), 0, 3) != 0) src[msLfo1 + i] = lfoValue(iparam(lfoP(i, lfWave), 0, kLfoWaves - 1), v.lfo[i], lfoShape[i][lfoShapeIdx[i].load(std::memory_order_relaxed)]);
                src[msVelocity] = vel; src[msKey] = (v.target - 60.f) / 48.f; src[msNoteRnd] = v.rnd; src[msAlternate] = v.alt;
                src[msRndVoice] = fresh ? 0.f : v.rndV.v; src[msBend] = v.bendS + chans[0].bend; src[msSlide] = v.slideS;
                float md[kModDstCount];
                buildMatrix();      // note-ons can arrive before process() has decoded this call's parameters
                evalMatrix(src, md);
                for (int o = 0; o < 3; ++o) phMod[o] = md[mdAPhase + o];
            }
        }
        auto startPhase = [&](int o) {
            float ph = op(o, ofPhase) + phMod[o];
            const float r = p[osc2P(o, o2PhaseRnd)];
            if (r > 0.f) ph += r * rng.uni();
            return ph - std::floor(ph);
        };
        if (fresh) {
            v.rndV.ph = 1.f; v.rndV.to = rng.bi(); v.rndV.v = v.rndV.to;
            for (int o = 0; o < 3; ++o) {
                OscVoice& ov = v.o[o];
                const bool retrig = op(o, ofRetrig) >= 0.5f;
                const bool plain = phMod[o] == 0.f && p[osc2P(o, o2PhaseRnd)] == 0.f;
                const float ph0 = plain ? op(o, ofPhase) : startPhase(o);
                for (int j = 0; j < kMaxUnison; ++j) {
                    ov.ph[j] = retrig ? (plain || j == 0 ? ph0 : startPhase(o)) : rng.uni(); ov.sph[j] = ov.ph[j]; ov.y1[j] = 0.f;
                    ov.drCur[j] = rng.bi() * 0.5f; ov.drTgt[j] = rng.bi();
                    ov.drTimer[j] = (int)((0.4f + 2.1f * rng.uni()) * fs);
                    ov.slop[j] = rng.bi();
                }
                for (int i = 0; i < kMaxPartials; ++i) {
                    const float ph = retrig ? ph0 * kTwoPi : rng.uni() * kTwoPi;
                    ov.aS[i] = std::sin(ph); ov.aC[i] = std::cos(ph); ov.aAmp[i] = 0.f; ov.aDa[i] = 0.f; ov.aTgt[i] = 0.f;
                    ov.aShPh[i] = rng.uni(); ov.aShRate[i] = 0.4f + 1.2f * rng.uni(); ov.aDet[i] = rng.bi();
                }
                ov.aLastF0 = -1.f; ov.aLastStretch = -9.f; ov.aHave = false;
                for (int k = 0; k < 12; ++k) ov.pmSig[k] = -1.f;
                std::memset(ov.mY1, 0, sizeof(ov.mY1)); std::memset(ov.mY2, 0, sizeof(ov.mY2));
                if (oiparam(o, ofType, 0, kOscTypes - 1) == otPhysical && pmIsWaveguide(oiparam(o, ofPmModel, 0, kPmModels - 1))) {
                    std::memset(ov.wg, 0, sizeof(float) * kWgSize);
                    std::memset(ov.sym, 0, sizeof(float) * kSymLines * kSymSize);
                }
                ov.wgPos = 0; ov.symPos = 0; ov.wgLP = 0.f; ov.excLP = 0.f; ov.bowPh = 0.f; ov.burstEnv = 0.f;
                ov.boreLP = 0.f; ov.dcX = ov.dcY = ov.dc2X = ov.dc2Y = 0.f; ov.jetPos = 0; ov.excT = 1e9f;
                ov.spFresh = true; ov.spOff = 0.0; ov.spFrozen = false;
                { const SampleBuf* sb = sslots[o].active; ov.smpPos = sb ? clampf(op(o, ofGrPos), 0.f, 1.f) * (float)(sb->len > 2 ? sb->len - 2 : 0) : 0.f; }
                for (auto& s : ov.symLP) s = 0.f;
                for (auto& g : ov.gr) g.on = false;
                ov.grTimer = 0.f; ov.grScan = 0.f;
                // 6.0 FM state: its own random source, so every other engine's random sequence stays as in 5.x
                for (int u = 0; u < 4; ++u) { for (int k = 0; k < 6; ++k) ov.fmPh[u][k] = retrig ? 0.f : fmRng.uni(); ov.fmY1[u] = ov.fmY2[u] = ov.fmHpX[u] = ov.fmHpY[u] = 0.f; }
                for (int k = 0; k < 6; ++k) { ov.fmEnv[k] = 0.f; ov.fmRnd[k] = fmRng.bi(); }
                ov.fmFresh = true;
            }
            v.ae.reset(); v.fe.reset(); v.me.reset();
            v.f1L.reset(); v.f1R.reset(); v.f2L.reset(); v.f2R.reset();
            v.f1bL.reset(); v.f1bR.reset(); v.f2bL.reset(); v.f2bR.reset();
            v.os1L.reset(); v.os1R.reset(); v.os2L.reset(); v.os2R.reset();
            v.pink[0] = v.pink[1] = v.pink[2] = 0.f;
            v.subPhase = 0.f;
            std::memset(v.prevM, 0, sizeof(v.prevM));
            for (int o = 0; o < 3; ++o) for (int s = 0; s < kBlock; ++s) v.prevW[o][s] = -1.f;
            v.firstBlock = true;
        }
        for (int o = 0; o < 3; ++o) {
            v.o[o].strikeEnv = vel; v.o[o].impulse = vel;
            for (int k = 0; k < 6; ++k) v.o[o].fmStage[k] = 0;      // FM operator envelopes restart (from their current level)
            if (!fresh && op(o, ofRetrig) >= 0.5f && oiparam(o, ofType, 0, kOscTypes - 1) == otFm)
                for (int u = 0; u < 4; ++u) { for (int k = 0; k < 6; ++k) v.o[o].fmPh[u][k] = 0.f; }
            if (!fresh && op(o, ofRetrig) >= 0.5f) {
                const bool plain = phMod[o] == 0.f && p[osc2P(o, o2PhaseRnd)] == 0.f;
                for (int j = 0; j < kMaxUnison; ++j) { v.o[o].ph[j] = plain ? op(o, ofPhase) : startPhase(o); v.o[o].sph[j] = v.o[o].ph[j]; }
            }
        }
        v.ae.gateOn(); v.fe.gateOn(); v.me.gateOn();
    }

    // ---- modulation --------------------------------------------------------------------------
    // 6.0.5: the matrix slots are decoded once per process() call (parameters only change between calls);
    // every voice then walks the short list of live slots instead of decoding all 24 slots each control block
    struct LiveSlot { int sr, dst, curve, via; float amt; };
    LiveSlot liveSlots[kModSlots];
    int liveSlotCount = 0;
    void buildMatrix() {
        int n = 0;
        for (int s = 0; s < kModSlots; ++s) {
            const int sr = iparam(modSrcParam(s), 0, kModSrcCount - 1);
            const int dst = iparam(modDstParam(s), 0, kModDstCount - 1);
            const float amt = p[modAmtParam(s)];
            if (sr == msNone || dst == mdNone || amt == 0.f) continue;
            liveSlots[n++] = { sr, dst, iparam(modCurveParam(s), 0, 7), iparam(modViaParam(s), 0, kModSrcCount - 1), amt };
        }
        liveSlotCount = n;
    }
    void evalMatrix(const float* src, float* md) const {
        for (int i = 0; i < kModDstCount; ++i) md[i] = 0.f;
        for (int k = 0; k < liveSlotCount; ++k) {
            const LiveSlot& ls = liveSlots[k];
            float x = applyCurve(ls.curve, src[ls.sr]);
            if (ls.via != msNone) x *= src[ls.via];
            md[ls.dst] += x * ls.amt;
        }
    }
    void globalSources(float* src) const {
        for (int i = 0; i < kModSrcCount; ++i) src[i] = 0.f;
        for (int i = 0; i < kLfos; ++i) src[msLfo1 + i] = gLfoVal[0][i];
        src[msModWheel] = modWheel; src[msRndGlobal] = rndG.v;
        for (int m = 0; m < kMacros; ++m) src[msMacro1 + m] = macroEff[m];
        src[msSeq] = seqCv; src[msExpression] = expression; src[msBreath] = breath;
        src[msBend] = chans[0].bend;
    }

    float msegTick(MsegState& s, const MsegData& d, int mode, float dt, bool gate) {
        if (s.done) return s.v;
        s.time += dt;
        if (s.hold) {
            if (gate) return s.v;
            s.hold = false; s.seg += 1; s.start = s.v; s.t = 0.f;
        } else s.t += dt;
        for (int guard = 0; guard < 64; ++guard) {
            if (s.seg >= d.count || s.seg < 1) { s.done = true; s.v = d.l[d.count - 1]; return s.v; }
            const float segT = d.t[s.seg] > 0.0005f ? d.t[s.seg] : 0.0005f;
            if (s.t < segT) break;
            s.t -= segT;
            const int r = s.seg;
            if (r == d.loopEnd && gate && mode == 0) { s.hold = true; s.v = d.l[r]; s.t = 0.f; s.u = 1.f; return s.v; }      // sustain point
            if (r == d.loopEnd && d.loopEnd > d.loopStart && (mode == 2 || (mode == 1 && gate))) { s.seg = d.loopStart + 1; s.start = d.l[d.loopStart]; continue; }
            s.seg = r + 1; s.start = d.l[r];
        }
        const float segT = d.t[s.seg] > 0.0005f ? d.t[s.seg] : 0.0005f;
        const float x = clampf(s.t / segT, 0.f, 1.f);
        s.u = x;
        s.v = s.start + (d.l[s.seg] - s.start) * drift::msegCurve(x, d.c[s.seg]);
        return s.v;
    }

    // ---- control rate -----------------------------------------------------------------------
    // 6.0.5: the per-block smoothing coefficients depend only on the block length and the rate: computed once,
    // not five std::exp calls per voice per control block. ctlK(n)[i] = 1 - exp(-n / (kCtlT[i] * fs)).
    static constexpr int kCtlKs = 5;
    float ctlKTab[kBlock + 1][kCtlKs] = {};
    float ctlKFs = 0.f;
    // filled on the audio thread (process()) before any voice renders: the workers only read it
    void prepCtlK() {
        if (ctlKFs != fs) {
            static const float kCtlT[kCtlKs] = { 0.004f, 0.015f, 0.6f, 0.003f, 0.9f };
            for (int b = 0; b <= kBlock; ++b) for (int i = 0; i < kCtlKs; ++i) ctlKTab[b][i] = 1.f - std::exp(-(float)b / (kCtlT[i] * fs));
            ctlKFs = fs;
        }
    }
    const float* ctlK(int n) const { return ctlKTab[n < 0 ? 0 : (n > kBlock ? kBlock : n)]; }
    void updateControl(Voice& v, int blk, int n) {
        Rng& rng = v.vr;
        const float* ck = ctlK(n);
        const ChannelState& cs = chans[v.ch];
        const bool snap = v.firstBlock;
        const float dtb = (float)n / fs;
        const float sm = snap ? 1.f : ck[0];
        const NeKey& ne = neKeys[v.ch & 15][v.note & 127];
        // 6.0.2: a released note on an MPE member channel keeps the expression it had: the channel now belongs to
        // the next note, whose pressure / slide / bend used to bring the old note's release back
        const bool frozen = !v.gate && v.ch != 0 && !snap;
        if (!frozen) {
            v.bendS  += (cs.bend - v.bendS) * sm;
            v.slideS += (clampf(cs.slide + ne.bright, -1.f, 1.f) - v.slideS) * sm;
        }
        float pressTgt = cs.pressure > v.polyAT ? cs.pressure : v.polyAT;
        if (ne.expr > pressTgt) pressTgt = ne.expr;
        const float smP = snap ? 1.f : ck[1];
        if (pressMode == 2) v.pressS += (0.f - v.pressS) * smP;                      // Off: plays like a keyboard without pressure
        else if (frozen) {}                                                          // released: keeps its pressure
        else if (pressMode == 1 && pressTgt < v.pressS && !snap)                     // Hold: pressure falls slowly, so lifting a
            v.pressS += (pressTgt - v.pressS) * ck[2];   // key (pressure -> 0 before note-off) keeps the note
        else v.pressS += (pressTgt - v.pressS) * smP;

        // LFOs
        const double bps = hostBpm / 60.0;
        for (int i = 0; i < kLfos; ++i) {
            LfoState& l = v.lfo[i];
            const float fade = p[lfoP(i, lfFade)];
            l.fade = fade > 0.001f ? (l.fade + dtb / fade > 1.f ? 1.f : l.fade + dtb / fade) : 1.f;
            const int mode = iparam(lfoP(i, lfMode), 0, 3);
            float val;
            if (mode == 0) val = gLfoVal[blk][i];
            else {
                const int wave = iparam(lfoP(i, lfWave), 0, kLfoWaves - 1);
                val = lfoValue(wave, l, lfoShape[i][lfoShapeIdx[i].load(std::memory_order_relaxed)]);
                float rate = p[lfoP(i, lfSync)] >= 0.5f ? (float)(bps / (double)kDivBeats[iparam(lfoP(i, lfDiv), 0, kDivs - 1)]) : p[lfoP(i, lfRate)];
                rate *= fastExp2(clampf(v.lfoRateMod[i], -1.f, 1.f) * 4.f) * l.rateMul;
                if (!l.done && lfoAdvance(l, rate * dtb, rng) && mode == 3) { l.done = true; l.phase = 0.9999f; }
                l.rateHz = l.done ? 0.f : rate;
                const float smt = p[lfoP(i, lfSmooth)];
                if (smt > 0.001f) { const float k = 1.f - std::exp(-dtb / (smt * smt * 0.5f + 1e-4f)); l.smooth += (val - l.smooth) * k; val = l.smooth; }
            }
            v.lfoOut[i] = val * l.fade;
        }
        // MSEGs
        for (int m = 0; m < kMsegs; ++m) {
            const MsegData& d = mseg[m][msegIdx[m].load(std::memory_order_relaxed)];
            float scale = p[msegP(m, 1)];
            if (p[msegP(m, 2)] >= 0.5f) scale *= (float)(60.0 / hostBpm);     // times become beats
            const float rate = fastExp2(clampf(v.msegRateMod[m], -1.f, 1.f) * 3.f) / (scale > 0.01f ? scale : 0.01f);
            msegTick(v.ms[m], d, iparam(msegP(m, 0), 0, 2), dtb * rate, v.gate || v.sustained);
        }
        v.rndV.tick(p[pRndVRate] * dtb, rng);

        // sources -> matrix
        float* src = v.src;
        src[msNone] = 0.f;
        for (int i = 0; i < kLfos; ++i) src[msLfo1 + i] = v.lfoOut[i];
        src[msFiltEnv] = v.fe.v; src[msAmpEnv] = v.ae.v; src[msModEnv] = v.me.v;
        src[msMseg1] = v.ms[0].v; src[msMseg2] = v.ms[1].v; src[msMseg3] = v.ms[2].v; src[msMseg4] = v.ms[3].v;
        src[msVelocity] = v.vel; src[msPressure] = v.pressS; src[msSlide] = v.slideS; src[msBend] = v.bendS + chans[0].bend;
        src[msModWheel] = modWheel; src[msKey] = (v.target - 60.f) / 48.f;
        src[msNoteRnd] = v.rnd; src[msRndGlobal] = rndG.v; src[msRndVoice] = v.rndV.v; src[msAlternate] = v.alt;
        for (int m = 0; m < kMacros; ++m) src[msMacro1 + m] = macroEff[m];
        src[msSeq] = seqCv; src[msExpression] = expression; src[msBreath] = breath;
        float* md = v.md;
        evalMatrix(src, md);
        for (int i = 0; i < kLfos; ++i) v.lfoRateMod[i] = md[mdLfo1Rate + i];
        v.msegRateMod[0] = md[mdMseg1Rate]; v.msegRateMod[1] = md[mdMseg2Rate]; v.msegRateMod[2] = md[mdMseg3Rate]; v.msegRateMod[3] = md[mdMseg4Rate];
        // 6.0: envelope stages. Times scale exponentially (amount 1 = x16), sustain adds.
        {
            bool any = false;
            for (int d = mdFEnvA; d <= mdMEnvR; ++d) if (md[d] != 0.f) { any = true; break; }
            v.envMod = any;
            if (any) {
                static const uint32_t base[3] = { pFEnvA, pAEnvA, pMEnvA }, ac[3] = { pFEnvCurve, pAEnvCurve, pMEnvCurve },
                                      rc[3] = { pFRelCurve, pARelCurve, pMRelCurve }, dc[3] = { pFDecCurve, pADecCurve, pMDecCurve };
                const bool classic = envClassic.load(std::memory_order_relaxed);
                for (int e = 0; e < 3; ++e) {
                    const float* m4 = md + envDst(e, 0);
                    auto tm = [&](uint32_t pi, float m) { return clampf(p[pi] * fastExp2(clampf(m, -1.f, 1.f) * 4.f), 0.001f, 60.f); };
                    envCoefs(v.ec[e], tm(base[e], m4[0]), tm(base[e] + 1, m4[1]), clampf(p[base[e] + 2] + m4[2], 0.f, 1.f), tm(base[e] + 3, m4[3]),
                             fs, p[ac[e]], classic, p[rc[e]], p[dc[e]]);
                }
            }
        }

        // tuning follows live changes of root / scale / MTS master; held notes glide to the new pitch
        { const float tt = tunedPitch(v.note, v.ch); if (tt != v.target) { v.target = tt; v.retuning = true; } }
        // glide
        if (v.pitch != v.target) {
            const float rg = p[pTnGlide];
            if (glideOn() || (v.retuning && rg > 0.001f)) {
                const float tc = glideOn() ? p[pGlide] : rg;
                const float c = std::exp(-(float)n / (tc * 0.5f * fs));
                v.pitch = v.target + (v.pitch - v.target) * c;
                if (std::fabs(v.pitch - v.target) < 0.001f) { v.pitch = v.target; v.retuning = false; }
            } else { v.pitch = v.target; v.retuning = false; }
        }
        float base = v.pitch + chans[0].bend * 2.f + md[mdPitchAll] * 12.f;
        if (p[pPitch] != 0.f) base += p[pPitch];   // 5.1.6: layer PITCH knob (kept out of the sum at 0: bit-identical to 5.1.5)
        if (v.ch != 0) base += v.bendS * p[pBendRange];
        // VST3 note expression: tuning + vibrato
        {
            const NeKey& ne = neKeys[v.ch & 15][v.note & 127];
            v.neTuneS += (ne.tune - v.neTuneS) * (snapNe(v) ? 1.f : ck[3]);
            base += v.neTuneS;
            if (ne.vib > 0.0005f) {
                v.neVibPh += p[pNeVibRate] * dtb; v.neVibPh -= std::floor(v.neVibPh);
                base += ne.vib * p[pNeVibDepth] * fastSinTurns(v.neVibPh);
            }
        }

        const float spread = p[pSpread];
        const float dCoef = ck[4];
        const float timbreAll = md[mdTimbreAll] + v.slideS * p[pSlideTimbre];
        const float detAll = md[mdDetuneAll] * 50.f;
        static const float panSign[3] = { 1.f, -1.f, 0.6f };

        for (int o = 0; o < 3; ++o) {
            OscVoice& ov = v.o[o];
            ov.on = op(o, ofOn) >= 0.5f;
            ov.type = oiparam(o, ofType, 0, kOscTypes - 1);
            if (!ov.on) { ov.level = 0.f; continue; }
            const int mb = modDstOscBase(o);
            // 5.1.6: OCT / SEMI modulation moves in whole octaves / semitones, FINE in cents (0 mod = unchanged)
            const int tb = modDstTuneBase(o);
            float semis;
            if (md[tb] == 0.f && md[tb + 1] == 0.f && md[tb + 2] == 0.f)
                semis = base + std::round(op(o, ofOct)) * 12.f + std::round(op(o, ofSemi)) + op(o, ofFine) * 0.01f + md[mb] * 12.f;
            else
                semis = base + std::round(op(o, ofOct) + md[tb] * 3.f) * 12.f + std::round(op(o, ofSemi) + md[tb + 1] * 12.f)
                      + (op(o, ofFine) + md[tb + 2] * 50.f) * 0.01f + md[mb] * 12.f;
            ov.level = clampf(op(o, ofLevel) + md[mb + 1], 0.f, 1.f);
            const float pan = clampf(op(o, ofPan) + md[mb + 2] + md[mdPanAll] + neKeys[v.ch & 15][v.note & 127].pan, -1.f, 1.f);
            ov.pan = pan;
            { float sa, ca; fastSinCos((pan + 1.f) * 0.25f * kPiF, sa, ca); ov.panL = ca * 1.4142f; ov.panR = sa * 1.4142f; }
            const float t1 = md[mb + 3] + timbreAll, t2 = md[mb + 4], t3 = md[mb + 5], t4 = md[mb + 6];
            for (int k = 0; k < kTimbreX; ++k) tx[k] = md[modDstXBase(o) + k];   // 5.1.7: Timbre 5..10, read by the engine updates
            ov.fmSrc = oiparam(o, ofFmSrc, 0, 5);
            ov.fmMode = oiparam(o, ofFmMode, 0, 5);
            ov.fmAmt = clampf(op(o, ofFmAmt) + md[mb + 7], 0.f, 1.f);
            if (ov.fmSrc == 1 + o && fmIsPitch(ov.fmMode)) ov.fmSrc = 0;    // self-FM is not offered
            ov.syncSrc = oiparam(o, ofSyncSrc, 0, 3);
            if (ov.syncSrc == 1 + o) ov.syncSrc = 0;
            ov.hz = semisToHz(semis);
            ov.press = clampf(p[osc2P(o, o2Pressure)] + md[mdAPress + o], 0.f, 1.f);
            ov.noiseAmt = p[osc2P(o, o2Noise)];
            ov.nonlin = p[osc2P(o, o2Nonlin)];

            if (ov.type == otWavetable || ov.type == otAnalog) {
                const bool supersaw = ov.type == otAnalog && oiparam(o, ofAnWave, 0, kAnWaves - 1) == 5;
                int nu = supersaw ? 7 : oiparam(o, ofVoices, 1, kMaxUnison);
                if (!supersaw && nu > unisonCap()) nu = unisonCap();
                ov.nUni = nu;
                float detune = op(o, ofDetune) + detAll + md[mb + 8] * 50.f;
                if (detune < 0.f) detune = 0.f;
                const float slopV = ov.type == otAnalog ? clampf(op(o, ofAnSlop) + (ov.type == otAnalog ? t3 : 0.f), 0.f, 1.f) : 0.f;
                const float slopC = slopV * 9.f;
                const float driftCents = (p[pDrift] + (ov.type == otAnalog ? clampf(op(o, ofAnDrift) + tx[0], 0.f, 1.f) : 0.f)) * 14.f + slopV * 10.f;
                const float uniW = clampf(op(o, ofUniWidth) + (ov.type == otWavetable ? t3 : t4), 0.f, 1.f);
                float norm = 1.f / std::sqrt((float)nu);
                float syncRatio = 1.f;
                float sawC = 1.f, sawS = 1.f;
                if (ov.type == otAnalog) {
                    const float syncSt = clampf(op(o, ofAnSync) + t2 * 24.f, 0.f, 48.f);
                    ov.sync = syncSt > 0.05f || ov.syncSrc != 0;
                    syncRatio = fastExp2(syncSt / 12.f);
                    ov.pw = clampf(op(o, ofAnPW) + t1 * 0.45f, 0.03f, 0.97f);
                    ov.wave = oiparam(o, ofAnWave, 0, kAnWaves - 1);
                    if (supersaw) {
                        const float mix = clampf((ov.pw - 0.05f) / 0.9f, 0.f, 1.f);
                        sawC = -0.55366f * mix + 0.99785f;
                        sawS = -0.73764f * mix * mix + 1.2841f * mix + 0.044372f;
                        norm = 1.f / std::sqrt(sawC * sawC + 6.f * sawS * sawS);
                    }
                }
                static const float kSS[7] = { -0.11002313f, -0.06288439f, -0.01952356f, 0.f, 0.01991221f, 0.06216538f, 0.10745242f };
                for (int j = 0; j < nu; ++j) {
                    ov.drTimer[j] -= n;
                    if (ov.drTimer[j] <= 0) { ov.drTgt[j] = rng.bi(); ov.drTimer[j] = (int)((0.4f + 2.1f * rng.uni()) * fs); }
                    ov.drCur[j] += (ov.drTgt[j] - ov.drCur[j]) * dCoef;
                    const float posn = nu == 1 ? 0.f : (supersaw ? kSS[j] / 0.11f : 2.f * j / (nu - 1) - 1.f);
                    const float cents = posn * detune * (supersaw ? 1.2f : 1.f) + ov.drCur[j] * driftCents + ov.slop[j] * slopC;
                    const float hz = semisToHz(semis + cents * 0.01f);
                    ov.inc[j] = clampf(hz / fs, 0.f, 0.45f);
                    ov.sinc[j] = clampf(ov.inc[j] * syncRatio, 0.f, 0.45f);
                    const float pn = clampf(posn * spread * uniW * panSign[o] + pan, -1.f, 1.f);
                    float sa, ca; fastSinCos((pn + 1.f) * 0.25f * kPiF, sa, ca);
                    const float g = norm * (supersaw ? (j == 3 ? sawC : sawS) : 1.f);
                    ov.gL[j] = g * ca * 1.4142f;
                    ov.gR[j] = g * sa * 1.4142f;
                    ov.gM[j] = g;
                }
                if (ov.type == otWavetable) {
                    ov.pos = clampf(op(o, ofPos) + t1 + t4 * 0.1f, 0.f, 1.f);
                    ov.bank = oiparam(o, ofTable, 0, kWtBanks - 1);
                    ov.warp = oiparam(o, ofWarp, 0, kWarpModes - 1);
                    ov.warpAmt = clampf(op(o, ofWarpAmt) + t2, 0.f, 1.f);
                    ov.blend = oiparam(o, ofWtBlend, 0, 2);
                    float mx = 0.f;
                    for (int j = 0; j < nu; ++j) mx = mx > ov.inc[j] ? mx : ov.inc[j];
                    float hf = 1.f;
                    switch (ov.warp) {
                    case 1: case 3: hf = 1.f + 3.f * ov.warpAmt; break;
                    case 2: hf = 1.f + 1.5f * ov.warpAmt; break;
                    case 4: hf = 1.f / (1.f - 0.95f * ov.warpAmt); break;
                    case 5: hf = 1.f + 7.f * ov.warpAmt; break;
                    case 7: hf = 1.f / (1.f - 0.98f * ov.warpAmt); break;
                    case 8: case 9: hf = 1.f + 4.f * ov.warpAmt; break;
                    default: break;
                    }
                    if (ov.fmSrc && fmIsPitch(ov.fmMode)) hf *= 1.f + 6.f * fmHard(ov.fmMode) * ov.fmAmt * ov.fmAmt;
                    ov.mip = wtMipFor(mx * 1.02f * hf);
                    v.telT[o][0] = ov.pos; v.telT[o][1] = ov.warpAmt; v.telT[o][2] = uniW; v.telT[o][3] = ov.pos;
                } else {
                    v.telT[o][0] = ov.pw; v.telT[o][1] = clampf(op(o, ofAnSync) + t2 * 24.f, 0.f, 48.f); v.telT[o][2] = slopV; v.telT[o][3] = uniW;
                }
            } else if (ov.type == otAdditive) {
                updateAdditive(v, o, semis, t1, t2, t3, t4, pan, snap, dtb);
            } else if (ov.type == otPhysical) {
                updatePhysical(v, o, semis, t1, t2, t3, t4, pan);
            } else if (ov.type == otSpectral) {
                updateSpectral(v, o, semis, t1, t2, t3, t4, pan);
            } else if (ov.type == otFm) {
                updateFm(v, o, semis, t1, t2, t3, t4, pan, n);
            } else {
                updateGranular(v, o, semis, t1, t2, t3, t4, pan);
            }
        }

        // sub
        const float subOct = (float)iparam(pSubOct, -2, -1);
        v.subInc = clampf(semisToHz(base + subOct * 12.f) / fs, 0.f, 0.45f);
        v.subMip = wtMipFor(v.subInc);
        v.subFrame = iparam(pSubWave, 0, 3) * (kWtFrames - 1) / 3;
        v.subL   = clampf(p[pSubLevel] + md[mdSub], 0.f, 1.f) * 0.7f;
        v.noiseL = clampf(p[pNoiseLevel] + md[mdNoise] * 0.5f, 0.f, 1.f) * 0.6f;
        v.ringL  = clampf(p[pRingLevel] + md[mdRing], 0.f, 1.f) * 1.2f;

        // filters
        v.ft1 = iparam(pF1Type, 0, kFilterTypes - 1);
        v.ft2 = iparam(pF2Type, 0, kFilterTypes - 1);
        v.ft1b = iparam(pF1Type2, 0, kFilterTypes - 1);
        v.ft2b = iparam(pF2Type2, 0, kFilterTypes - 1);
        v.morph1 = clampf(p[pF1Morph] + md[mdF1Morph], 0.f, 1.f);
        v.morph2 = clampf(p[pF2Morph] + md[mdF2Morph], 0.f, 1.f);
        if (v.ft1b == v.ft1) v.morph1 = 0.f;
        if (v.ft2b == v.ft2) v.morph2 = 0.f;
        v.dt1 = iparam(pF1DriveType, 0, 3); v.dt2 = iparam(pF2DriveType, 0, 3);
        v.fmSrc1 = iparam(pF1FmSrc, 0, 4); v.fmSrc2 = iparam(pF2FmSrc, 0, 4);
        v.fmAmt1 = clampf(p[pF1FmAmt] + md[mdF1Fm], 0.f, 1.f); v.fmAmt2 = clampf(p[pF2FmAmt] + md[mdF2Fm], 0.f, 1.f);
        v.routing = iparam(pFRouting, 0, 2);
        const float noteOct = (v.target - 60.f) / 12.f;
        const float common = v.pressS * p[pPressCut] * 5.f + v.slideS * p[pSlideCut] * 3.f + modWheel * p[pMWCut] * 5.f + md[mdCutoffBoth] * 5.f;
        v.f1Prev = v.f1Log; v.f2Prev = v.f2Log;
        v.f1Log = f1Blk[blk] + p[pF1Key] * noteOct + common + md[mdF1Cutoff] * 5.f;
        v.f2Log = f2Blk[blk] + p[pF2Key] * noteOct + common + md[mdF2Cutoff] * 5.f;
        if (v.firstBlock) { v.f1Prev = v.f1Log; v.f2Prev = v.f2Log; }
        v.env1 = p[pF1Env] * 7.f; v.env2 = p[pF2Env] * 7.f;
        v.res1 = clampf(p[pF1Res] + md[mdF1Res], 0.f, 1.f);
        v.res2 = clampf(p[pF2Res] + md[mdF2Res], 0.f, 1.f);
        v.drv1 = clampf(p[pF1Drive] + md[mdF1Drive], 0.f, 1.f);
        v.drv2 = clampf(p[pF2Drive] + md[mdF2Drive], 0.f, 1.f);
        v.balance = clampf(p[pFBalance] + md[mdBalance], 0.f, 1.f);
        const float sh1 = clampf(p[pF1Shape] + md[mdF1Shape], 0.f, 1.f), sh2 = clampf(p[pF2Shape] + md[mdF2Shape], 0.f, 1.f);
        v.c1.shape = v.c1b.shape = sh1; v.c2.shape = v.c2b.shape = sh2;
        const float fc1 = fastExp2(clampf(v.f1Log + v.fe.v * v.env1, 4.f, 14.4f)), fc2 = fastExp2(clampf(v.f2Log + v.fe.v * v.env2, 4.f, 14.4f));
        if (filterNeedsControlCoefs(v.ft1)) filterControlCoefs(v.ft1, fc1, v.res1, sh1, fs, v.c1);
        if (filterNeedsControlCoefs(v.ft2)) filterControlCoefs(v.ft2, fc2, v.res2, sh2, fs, v.c2);
        if (v.morph1 > 0.f && filterNeedsControlCoefs(v.ft1b)) filterControlCoefs(v.ft1b, fc1, v.res1, sh1, fs, v.c1b);
        if (v.morph2 > 0.f && filterNeedsControlCoefs(v.ft2b)) filterControlCoefs(v.ft2b, fc2, v.res2, sh2, fs, v.c2b);
        v.telCut1 = fc1; v.telCut2 = fc2;

        float g = (1.f - p[pVelAmp]) + p[pVelAmp] * v.vel;
        g *= 1.f + p[pPressAmp] * v.pressS;
        g *= 1.f + md[mdAmp] > 0.f ? 1.f + md[mdAmp] : 0.f;
        g *= neKeys[v.ch & 15][v.note & 127].vol;
        g *= layerGain(v.note, v.vel, v.pressS);
        v.gainPrev = v.firstBlock ? g : v.gain;
        v.gain = g;
        v.firstBlock = false;
    }

    void updateAdditive(Voice& v, int o, float semis, float t1, float t2, float t3, float t4, float pan, bool snap, float dtb) {
        OscVoice& ov = v.o[o];
        const float f0 = semisToHz(semis);
        const int n = std::min(oiparam(o, ofAdPartials, 1, kMaxPartials), partialCap());
        const int s1 = oiparam(o, ofAdSpec1, 0, kSpectra - 1), s2 = oiparam(o, ofAdSpec2, 0, kSpectra - 1);
        auto specOf = [&](int s) -> const float* {
            if (s == kSpecCustom) return custom[o][customIdx[o].load(std::memory_order_acquire)];
            if (s == kSpecCustom2) return custom[o + 3][customIdx[o + 3].load(std::memory_order_acquire)];
            return spec[s];
        };
        const float* A = specOf(s1);
        const float* B = specOf(s2);
        const float morph = clampf(op(o, ofAdMorph) + t1, 0.f, 1.f);
        const float tilt = clampf(op(o, ofAdTilt) + t3, -1.f, 1.f) * 1.5f;
        const float oe = clampf(op(o, ofAdOddEven) + tx[1], -1.f, 1.f);
        const float oddW = oe > 0.f ? 1.f - oe : 1.f, evenW = oe < 0.f ? 1.f + oe : 1.f;
        const float stretch = clampf(op(o, ofAdStretch) + tx[0], -1.f, 1.f);
        const float stretchExp = 1.f + 0.08f * stretch;
        const float peakPos = clampf(op(o, ofAdPeakPos) * fastExp2(clampf(t2, -1.f, 1.f) * 3.f), 1.f, 128.f);
        const float peakAmt = clampf(op(o, ofAdPeakAmt) + tx[2], 0.f, 1.f) * 8.f;
        const float peakW = (0.4f + 6.f * op(o, ofAdPeakWidth)) * (1.f + peakPos * 0.08f);
        const float comb = op(o, ofAdComb), combShift = op(o, ofAdCombShift) + tx[3];   // a phase: wraps
        const float shim = clampf(op(o, ofAdShimmer) + tx[4], 0.f, 1.f) * 0.85f;
        const float shimInc = op(o, ofAdShimRate) * dtb;
        const float det = clampf(op(o, ofAdDetune) + tx[5] * 30.f, 0.f, 30.f);
        const float vowel = clampf(op(o, ofAdVowel) + t4, 0.f, 1.f), vowelAmt = op(o, ofAdVowelAmt);
        const bool freeze = op(o, ofAdFreeze) >= 0.5f;
        const float blur = op(o, ofAdBlur);
        const float spread = p[pSpread];
        const float nyq = 0.45f * fs;
        v.telT[o][0] = morph; v.telT[o][1] = peakPos; v.telT[o][2] = tilt / 1.5f; v.telT[o][3] = vowel;
        const bool reratio = stretch != ov.aLastStretch || det != ov.aLastDet;
        const bool retune = reratio || std::fabs(f0 - ov.aLastF0) > f0 * 2e-5f;
        if (reratio)
            for (int i = 0; i < kMaxPartials; ++i)
                ov.aRatio[i] = fastExp2(stretchExp * lg2h[i] + ov.aDet[i] * det * (1.f / 1200.f));
        if (retune) ov.aLastF0 = f0;
        ov.aLastStretch = stretch; ov.aLastDet = det;
        const float w0 = kTwoPi * f0 / fs;
        // vowel formants (A E I O U), log-frequency bumps
        float vf[3], vg[3] = { 1.f, 0.7f, 0.4f };
        if (vowelAmt > 0.f) {
            static const float F[5][3] = { { 800, 1150, 2900 }, { 400, 1600, 2700 }, { 350, 1700, 2700 }, { 450, 800, 2830 }, { 325, 700, 2530 } };
            const float x = vowel * 3.999f; const int vi = (int)x; const float fr = x - (float)vi;
            for (int k = 0; k < 3; ++k) vf[k] = std::log2(F[vi][k] + (F[vi + 1][k] - F[vi][k]) * fr);
        }
        const float lf0 = std::log2(f0 > 1.f ? f0 : 1.f);
        // spectral shaping runs every 4th control block (~1.3 ms); amplitude ramps stay per block
        const bool heavy = snap || !ov.aHave || ((++ov.aCtl) & 3) == 0 || reratio;
        const bool recompute = (!freeze || !ov.aHave || snap) && heavy;
        // only partials below Nyquist and inside the partial count matter
        int lim = n;
        while (lim > 0 && f0 * ov.aRatio[lim - 1] >= nyq) --lim;
        float sum2 = 0.f;
        if (recompute) {
            for (int i = 0; i < lim; ++i) {
                const float h = (float)(i + 1);
                float a = A[i] + (B[i] - A[i]) * morph;
                if (tilt != 0.f) a *= fastExp2(-tilt * lg2h[i]);
                if (i > 0) a *= (i & 1) ? evenW : oddW;
                if (peakAmt > 0.f) { const float d = (h - peakPos) / peakW; a *= 1.f + peakAmt * fastExp2(-0.7213475f * d * d); }
                if (comb > 0.f) a *= 1.f - comb * (0.5f + 0.5f * fastSinTurns(h * (1.f / 6.f) + combShift + 0.25f));
                if (vowelAmt > 0.f) {
                    const float lf = lf0 + lg2h[i];
                    float w = 0.f;
                    for (int k = 0; k < 3; ++k) { const float d = (lf - vf[k]) * (1.f / 0.2f); w += vg[k] * fastExp2(-0.7213475f * d * d); }
                    a *= (1.f - vowelAmt) + vowelAmt * 3.f * w;
                }
                ov.aShPh[i] += shimInc * 4.f * ov.aShRate[i];
                if (shim > 0.f) a *= 1.f - shim * (0.5f + 0.5f * fastSinTurns(ov.aShPh[i]));
                ov.aTgt[i] = a;
                sum2 += a * a;
            }
            for (int i = lim; i < kMaxPartials; ++i) ov.aTgt[i] = 0.f;
            const float norm = sum2 > 1e-12f ? 0.6f / std::sqrt(sum2) : 0.f;
            for (int i = 0; i < lim; ++i) ov.aTgt[i] *= norm;
            ov.aHave = true;
        } else if (freeze) {
            for (int i = lim; i < kMaxPartials; ++i) ov.aTgt[i] = 0.f;
        }
        const float kb = blur > 0.001f ? 1.f - std::exp(-dtb / blur) : 1.f;
        const int span = std::max(lim, ov.aPrevN);
        ov.aPrevN = lim;
        int cnt = 0;
        const float invB = 1.f / (float)kBlock;
        const bool repan = pan != ov.aLastPan || spread != ov.aLastSpread || snap;
        ov.aLastPan = pan; ov.aLastSpread = spread;
        for (int i = 0; i < span; ++i) {
            const float tgt = ov.aTgt[i];
            if (retune && i < lim) fastSinCos(w0 * ov.aRatio[i], ov.aRs[i], ov.aRc[i]);
            if (tgt > 0.f || ov.aAmp[i] > 1e-6f) cnt = i + 1;
            if (snap) { ov.aAmp[i] = tgt; ov.aDa[i] = 0.f; }
            else { const float goal = ov.aAmp[i] + (tgt - ov.aAmp[i]) * kb; ov.aDa[i] = (goal - ov.aAmp[i]) * invB; }
            if (repan) {
                const float pn = clampf(pan + (i == 0 ? 0.f : spread * ((i & 1) ? 1.f : -1.f) * (i < 3 ? (float)i / 3.f : 1.f)), -1.f, 1.f);
                float sa, ca; fastSinCos((pn + 1.f) * 0.25f * kPiF, sa, ca);
                ov.aPl[i] = ca * 1.4142f; ov.aPr[i] = sa * 1.4142f;
            }
            renormPartial(ov, i);
        }
        // 6.0 fix (see renormPartial): the partials the renderer still runs past `span` are kept normalised too
        for (int i = span; i < ((span + 7) & ~7) && i < kMaxPartials; ++i) renormPartial(ov, i);
        // 6.0 fix: the renderer runs partials in groups of 8, so up to 7 partials past `span` are still played. When
        // the Nyquist limit moves (Stretch modulation, bends, glides) those kept their last amplitude ramp and drifted
        // without bound (NaN). Silence them.
        for (int i = span; i < ((span + 7) & ~7) && i < kMaxPartials; ++i) { ov.aAmp[i] = 0.f; ov.aDa[i] = 0.f; }
        for (int i = span; i < kMaxPartials && repan; ++i) {
            const float pn = clampf(pan + spread * ((i & 1) ? 1.f : -1.f), -1.f, 1.f);
            float sa, ca; fastSinCos((pn + 1.f) * 0.25f * kPiF, sa, ca);
            ov.aPl[i] = ca * 1.4142f; ov.aPr[i] = sa * 1.4142f;
        }
        ov.aCount = cnt;
    }

    // 6.0 fix: one Newton step keeps a partial's rotation at unit length only while it is close to 1. Partials just past
    // the Nyquist limit used to rotate un-normalised and drift; when the limit moved back (Stretch modulation, bends) the
    // Newton step diverged into NaN. Far from 1: exact normalisation (or a fresh phase when it has collapsed).
    static inline void renormPartial(OscVoice& ov, int i) {
        const float m2 = ov.aS[i] * ov.aS[i] + ov.aC[i] * ov.aC[i];
        if (m2 > 0.81f && m2 < 1.21f) { const float m = 1.5f - 0.5f * m2; ov.aS[i] *= m; ov.aC[i] *= m; }
        else if (m2 > 1e-12f && std::isfinite(m2)) { const float m = 1.f / std::sqrt(m2); ov.aS[i] *= m; ov.aC[i] *= m; }
        else { ov.aS[i] = 0.f; ov.aC[i] = 1.f; }
    }
    void updatePhysical(Voice& v, int o, float semis, float t1, float t2, float t3, float t4, float pan) {
        OscVoice& ov = v.o[o];
        const int model = oiparam(o, ofPmModel, 0, kPmModels - 1);
        const int exc = oiparam(o, ofPmExciter, 0, kPmExciters - 1);
        const float f0 = semisToHz(semis);
        const float decay = clampf(op(o, ofPmDecay) * fastExp2(clampf(t3, -1.f, 1.f) * 3.f), 0.02f, 60.f);
        const float damp = clampf(op(o, ofPmDamp) + t4, 0.f, 1.f);
        const float hard = clampf(op(o, ofPmHard) + t2, 0.f, 1.f);
        const float posn = clampf(op(o, ofPmPos) + t1 * 0.5f, 0.02f, 0.98f);
        // 5.1.7: Timbre 5..10. Stretch / Spread / Coupling reshape the modal bank, so moving them rebuilds it
        // (same cost as modulating Decay or Damping)
        const float stretch = 1.f + 0.1f * clampf(op(o, ofPmStretch) + tx[0], -1.f, 1.f);
        const float spread = clampf(op(o, ofPmSpread) + tx[1], 0.f, 1.f);
        const float couple = clampf(op(o, ofPmCouple) + tx[2], 0.f, 1.f);
        const float buzz = clampf(op(o, ofPmBuzz) + tx[3], 0.f, 1.f);
        ov.noiseAmt = clampf(ov.noiseAmt + tx[4], 0.f, 1.f);
        ov.nonlin = clampf(ov.nonlin + tx[5], 0.f, 1.f);
        const float nyq = 0.45f * fs;
        v.telT[o][0] = posn; v.telT[o][1] = hard; v.telT[o][2] = decay; v.telT[o][3] = damp;
        ov.excCoef = 0.02f + 0.9f * hard * hard;
        const float burst = (0.0008f + 0.02f * (1.f - hard)) * fs;
        ov.strikeDecay = std::exp(-1.f / burst);
        const float strikeNorm = 5.f / std::sqrt(burst * ov.excCoef > 1.f ? burst * ov.excCoef : 1.f);
        ov.bowInc = clampf(f0 / fs, 0.f, 0.45f);
        ov.burstRate = exc == 4 ? 30.f + 400.f * hard * hard : 2.f + 70.f * hard;
        ov.buzz = buzz;
        // 4.2 pluck / mallet: contact time from hardness
        ov.excLen = (exc == 6 ? 0.0003f + 0.006f * (1.f - hard) : 0.0005f + 0.008f * (1.f - hard)) * fs;
        if (pmIsWaveguide(model)) {
            ov.mCount = 0;
            const float f = clampf(f0, fs / (kWgSize - 4), nyq * 0.5f);
            if (model == pmReed) {
                // clarinet-like: closed-open bore, the inverted bell reflection doubles the period
                ov.bellC = 0.12f + 0.8f * (1.f - damp);
                ov.wgDelay = clampf(0.5f * fs / f - (1.f - ov.bellC) / ov.bellC - kReedLag, 2.f, (float)kWgSize - 4.f);
                ov.reedOff = 0.62f + 0.18f * posn;
                ov.reedSlope = -0.18f - 0.4f * hard;
                return;
            }
            if (model == pmFlute) {
                // the jet path is part of the loop: bore + jet delays (+ the loss filter) make one period
                ov.bellC = 0.25f + 0.7f * (1.f - damp);
                const float jr = 0.08f + 0.5f * posn;
                const float loop = fs / f - (1.f - ov.bellC) / ov.bellC - kFluteLag;
                ov.wgDelay = clampf(loop / (1.f + jr), 2.f, (float)kWgSize - 4.f);
                ov.jetDelay = clampf(ov.wgDelay * jr, 1.f, (float)kSymSize - 4.f);
                return;
            }
            ov.wgDelay = clampf(fs / f - 0.5f, 2.f, (float)kWgSize - 4.f);
            ov.wgFb = clampf(std::exp(-6.9078f / (decay * f)), 0.f, 0.99995f);
            ov.wgBright = 0.12f + 0.88f * (1.f - damp) * (0.4f + 0.6f * hard);
            ov.wgTap = 0.25f + 0.5f * posn;
            ov.wgSpread = spread;
            if (model == pmSympathetic) {
                static const float ratios[kSymLines] = { 2.f, 1.5f, 4.f / 3.f, 3.f };
                for (int k = 0; k < kSymLines; ++k) {
                    const float fk = clampf(f * ratios[k], fs / (kSymSize - 4), nyq * 0.5f);
                    ov.symDelay[k] = clampf(fs / fk - 0.5f, 2.f, (float)kSymSize - 4.f);
                }
                ov.symFb = clampf(std::exp(-6.9078f / (decay * 1.6f * f * 1.5f)), 0.f, 0.99993f);
            }
            return;
        }
        // full rebuild only when something that shapes the bank moved; pitch alone just retunes it
        const uint32_t gen = model == pmAnalysed ? modalGen[o].load(std::memory_order_acquire) : 0u;
        const float sig[12] = { (float)model, (float)gen, decay, damp, hard, posn, stretch, spread, (float)exc, pan, couple, (float)curQuality };
        bool same = true;
        for (int k = 0; k < 12; ++k) if (sig[k] != ov.pmSig[k]) { same = false; break; }
        if (same) {
            if (std::fabs(f0 - ov.mF0) <= f0 * 5e-5f) return;
            ov.mF0 = f0;
            for (int m = 0; m < ov.mCount; ++m) {
                const float f = f0 * ov.mRatio[m];
                if (f >= nyq) { ov.mB[m] = 0.f; ov.mA1[m] = ov.mA2[m] = 0.f; continue; }
                float sw, cw;
                fastSinCos(kTwoPi * f / fs, sw, cw);
                ov.mA1[m] = 2.f * ov.mR[m] * cw;
                ov.mA2[m] = ov.mR[m] * ov.mR[m];
                ov.mB[m] = ov.mAmp[m] * sw * ov.mNorm[m];
            }
            return;
        }
        for (int k = 0; k < 12; ++k) ov.pmSig[k] = sig[k];
        ov.mF0 = f0;
        float ratios[kMaxModes], amps[kMaxModes], decs[kMaxModes];
        const bool own = model == pmAnalysed;
        int nb;
        if (own) {
            const ModalData& md = modal[o][modalIdx[o].load(std::memory_order_acquire)];
            nb = md.count;
            for (int i = 0; i < nb; ++i) { ratios[i] = md.ratio[i]; amps[i] = md.amp[i]; decs[i] = md.decay[i]; }
        } else nb = pmModeRatios(model, ratios);
        const int cap = (model == pmGong || own) ? modeCap() * 2 : modeCap();
        if (nb > cap) nb = cap;
        const bool coupled = couple > 0.001f;
        const float split = couple * 0.012f;
        ov.mCount = 0;
        const float ratioRef = std::log2(ratios[0] > 0.05f ? ratios[0] : 0.05f);
        const bool impulsive = exc == 0 || exc == 6 || exc == 7;
        const float contNorm = exc >= 4 && exc != 6 && exc != 7 ? 7.f : 2.5f;
        int m = 0;
        for (int i = 0; i < nb; ++i) {
            const float lr = std::log2(ratios[i]) * stretch;
            const float rr = fastExp2(lr);
            float oct = lr - ratioRef; if (oct < 0.f) oct = 0.f;
            const float T = own ? decs[i] * (decay / 5.f) / (1.f + damp * 1.5f * oct * oct) : decay / (1.f + damp * 2.5f * oct * oct);
            // 5.1.1: exact exp2: fastExp2's ~1e-4 relative error, compounded every sample, cut every decay short
            // (Decay 30 s rang ~1.6 s; HQ doubled the error)
            const float r = (float)std::exp2(-9.9658 / ((double)T * (double)fs));
            float a;
            if (own) a = amps[i];
            else if (model == pmString || model == pmTube) a = std::fabs(fastSinTurns(0.5f * (i + 1) * posn)) * invSqrt[i];
            else a = (0.35f + 0.65f * std::fabs(fastSinTurns(0.5f * (i + 1) * posn * 1.37f + 0.0477f))) * invPow06[i];
            const int copies = coupled ? 2 : 1;
            for (int c = 0; c < copies; ++c) {
                const float f = f0 * rr * (coupled ? (c == 0 ? 1.f - split : 1.f + split) : 1.f);
                if (f >= nyq || m >= kMaxModes) continue;
                float sw, cw;
                fastSinCos(kTwoPi * f / fs, sw, cw);
                ov.mA1[m] = 2.f * r * cw;
                ov.mA2[m] = r * r;
                ov.mR[m] = r; ov.mAmp[m] = a;
                ov.mNorm[m] = (impulsive ? strikeNorm : std::sqrt(1.f - r) * contNorm) * (coupled ? 0.7071f : 1.f);
                ov.mB[m] = a * sw * ov.mNorm[m];
                ov.mRatio[m] = rr * (coupled ? (c == 0 ? 1.f - split : 1.f + split) : 1.f);
                const float pn = clampf(pan + spread * ((i & 1) ? 1.f : -1.f) * (i < 3 ? (float)i / 3.f : 1.f) * (c ? -0.6f : 1.f), -1.f, 1.f);
                float sa, ca; fastSinCos((pn + 1.f) * 0.25f * kPiF, sa, ca);
                ov.mPl[m] = ca * 1.4142f; ov.mPr[m] = sa * 1.4142f;
                ++m;
            }
        }
        // silence modes that no longer exist
        for (int k = m; k < ov.mCount; ++k) { ov.mY1[k] = ov.mY2[k] = 0.f; }
        ov.mCount = m;
    }

    // ---- 5.0 spectral oscillator ------------------------------------------------------------------
    void updateSpectral(Voice& v, int o, float semis, float t1, float t2, float t3, float t4, float pan) {
        OscVoice& ov = v.o[o];
        auto q = [&](uint32_t f) { return p[osc2P(o, f)]; };
        ov.spPos = clampf(q(o2SpPos) + t1, 0.f, 1.f);
        ov.spBlur = clampf(q(o2SpBlur) + t2 * 5.f, 0.f, 20.f);
        ov.spShiftHz = clampf(q(o2SpShift) + t3 * 200.f, -2000.f, 2000.f);
        ov.spCross = clampf(q(o2SpCross) + t4, 0.f, 1.f);
        ov.spSpeed = clampf(q(o2SpSpeed) + tx[0] * 2.f, -2.f, 2.f);
        ov.spStretch = clampf(q(o2SpStretch) + tx[1], -1.f, 1.f); ov.spTilt = clampf(q(o2SpTilt) + tx[2], -1.f, 1.f);
        ov.spFormant = clampf(q(o2SpFormant) + tx[3], 0.f, 1.f); ov.spSparse = clampf(q(o2SpSparse) + tx[4], 0.f, 1.f);
        ov.spCrossMode = (int)(q(o2SpCrossMode) + 0.5f);
        ov.spSpread = p[pSpread];
        const SpecData* A = spslots[o][0].active;
        const float root = A ? A->root : 261.6256f;
        if (q(o2SpKeyed) >= 0.5f) ov.spRatio = semisToHz(semis) / root;
        else ov.spRatio = fastExp2((std::round(op(o, ofOct)) * 12.f + std::round(op(o, ofSemi)) + op(o, ofFine) * 0.01f) / 12.f);
        // freeze latches the playhead; while frozen the position knob no longer scrubs
        const bool fz = q(o2SpFreeze) >= 0.5f;
        // a voice that has not run a frame yet (note start, preset saved frozen) has no playhead: freeze at POSITION
        if (fz && !ov.spFrozen) ov.spFrozenPos = ov.spFresh ? ov.spPos : ov.spPlay;
        ov.spFrozen = fz;
        ov.pan = pan;
        v.telT[o][0] = ov.spPos; v.telT[o][1] = ov.spBlur; v.telT[o][2] = ov.spShiftHz; v.telT[o][3] = ov.spCross;
    }

    // one synthesis frame (phase-locked vocoder: every bin follows the phase of its spectral peak)
    void spectralHop(OscVoice& ov, int o, SpScratch& w, const SpecData& A, const SpecData* Bx) {
        const int N = ov.spN, H = ov.spH, B = ov.spBins;
        const int Bin = A.bins;
        const float binIn = A.sr / (float)A.N, binOut = fs / (float)N;
        // playhead (frames of A)
        const float last = (float)std::max(1, A.frames - 1);
        float pos;
        if (ov.spFrozen) pos = ov.spFrozenPos * last;
        else {
            pos = ov.spPos * last + (float)ov.spOff;
            pos -= last * std::floor(pos / last);
            ov.spOff += (double)ov.spSpeed * ((double)H / fs) / ((double)A.hop / A.sr);
            if (ov.spOff > 1e7 || ov.spOff < -1e7) ov.spOff = 0.0;
        }
        ov.spPlay = pos / last;
        int f0 = (int)pos; if (f0 >= A.frames - 1) f0 = std::max(0, A.frames - 2);
        const int f1 = std::min(A.frames - 1, f0 + 1);
        const float fr = clampf(pos - (float)f0, 0.f, 1.f);
        const int fn = fr < 0.5f ? f0 : f1;
        const float* m0 = A.magOf(f0); const float* m1 = A.magOf(f1);
        const int16_t* dv = A.devOf(fn); const int16_t* ph = A.phOf(fn);
        const float devS = 1.f / (float)drift::kSpecDevScale, phS = 3.14159265f / 32767.f;
        const int nb = std::min(Bin, kSpMaxBins);
        float frameMax = 0.f;
        for (int k = 0; k < nb; ++k) {
            w.mag[k] = m0[k] + (m1[k] - m0[k]) * fr;
            w.fhz[k] = ((float)k + (float)dv[k] * devS) * binIn;
            w.phi[k] = (float)ph[k] * phS;
        }
        // cross-synthesis partner at the same relative position
        const bool cross = Bx && Bx->frames > 0 && ov.spCrossMode > 0 && ov.spCross > 0.001f;
        auto envelope = [&](const float* m, float* env) {
            w.pre[0] = 0.f;
            for (int k = 0; k < nb; ++k) w.pre[k + 1] = w.pre[k] + m[k];
            for (int k = 0; k < nb; ++k) {
                const int wd = std::max(2, k / 10);
                const int a = std::max(0, k - wd), b = std::min(nb, k + wd + 1);
                env[k] = (w.pre[b] - w.pre[a]) / (float)(b - a) + 1e-9f;
            }
        };
        if (cross) {
            const SpecData& X = *Bx;
            const float lastB = (float)std::max(1, X.frames - 1);
            const int g0 = std::min(X.frames - 1, (int)(ov.spPlay * lastB));
            const float* mb = X.magOf(g0);
            const float scale = (A.peak > 0.f ? 1.f : 1.f) * (X.peak > 0.f ? A.peak / X.peak : 1.f);
            const float binX = X.sr / (float)X.N;
            // B's magnitudes on A's bin grid
            for (int k = 0; k < nb; ++k) { const int kb = (int)((float)k * binIn / binX + 0.5f); w.envB[k] = kb < X.bins ? mb[kb] * scale : 0.f; }
            const float c = ov.spCross;
            if (ov.spCrossMode == 1) {          // morph: blend the magnitudes; frequencies / phases follow the louder source
                const int16_t* dvb = X.devOf(g0); const int16_t* phb = X.phOf(g0);
                for (int k = 0; k < nb; ++k) {
                    const float mA = w.mag[k] * (1.f - c), mB = w.envB[k] * c;
                    if (mB > mA) { const int kb = std::min(X.bins - 1, (int)((float)k * binIn / binX + 0.5f)); w.fhz[k] = ((float)kb + (float)dvb[kb] * devS) * binX; w.phi[k] = (float)phb[kb] * phS; }
                    w.mag[k] = mA + mB;
                }
            } else {                             // vocode: A's partials through B's spectral envelope
                for (int k = 0; k < nb; ++k) w.nMag[k] = w.envB[k];
                envelope(w.mag, w.envA);
                envelope(w.nMag, w.envB);
                for (int k = 0; k < nb; ++k) w.mag[k] *= std::pow(w.envB[k] / w.envA[k], c);
            }
        }
        for (int k = 0; k < nb; ++k) frameMax = std::max(frameMax, w.mag[k]);
        const bool formant = ov.spFormant > 0.001f && std::fabs(ov.spRatio - 1.f) > 1e-3f;
        if (formant) envelope(w.mag, w.envA);
        // peaks above the floor; Sparse raises it relative to the frame
        const float floorAbs = A.peak * 1e-5f;
        const float thr = std::max(floorAbs, frameMax * std::pow(10.f, (-90.f + 78.f * ov.spSparse) / 20.f));
        int np = 0;
        for (int k = 1; k < nb - 1 && np < kSpMaxBins; ++k)
            if (w.mag[k] > thr && w.mag[k] > w.mag[k - 1] && w.mag[k] >= w.mag[k + 1]) w.peaks[np++] = k;
        // regions: up to the lowest bin between neighbouring peaks
        for (int i = 0; i < np; ++i) {
            int lo = i == 0 ? 1 : w.hi[i - 1] + 1;
            int hi = nb - 2;
            if (i + 1 < np) { hi = w.peaks[i]; for (int k = w.peaks[i]; k < w.peaks[i + 1]; ++k) if (w.mag[k] < w.mag[hi]) hi = k; }
            if (ov.spSparse > 0.001f) { lo = std::max(lo, w.peaks[i] - 3); hi = std::min(hi, w.peaks[i] + 3); }   // sparse: just the peak lobes
            w.lo[i] = lo; w.hi[i] = hi;
        }
        for (int j = 0; j < B; ++j) { w.nMag[j] = 0.f; w.nPan[j] = 0.f; w.nFq[j] = 0.f; }
        const float ratio = ov.spRatio;
        const float f0r = A.root * ratio;
        const float stretchE = 1.f + 0.25f * ov.spStretch;
        const float nyq = 0.45f * fs;
        const float dPh = 6.2831853f * (float)H / fs;
        const float lin = binIn / binOut;        // input bins per output bin (lobe width)
        for (int i = 0; i < np; ++i) {
            const int pk = w.peaks[i];
            float f = w.fhz[pk] * ratio;
            if (ov.spStretch != 0.f && f > 1.f) f = f0r * std::pow(f / f0r, stretchE);
            f += ov.spShiftHz;
            if (f <= 20.f || f >= nyq) continue;
            float g = 1.f;
            if (ov.spTilt != 0.f) g *= std::pow(f / 1000.f, ov.spTilt * 1.5f);
            if (formant) {
                const int ke = std::min(nb - 1, (int)(f / binIn + 0.5f));
                g *= std::pow(w.envA[ke] / w.envA[pk], ov.spFormant);
            }
            g = std::min(g, 16.f);
            const int jp = (int)(f / binOut + 0.5f);
            if (jp < 1 || jp >= B - 1) continue;
            const float phPeak = ov.spFresh ? w.phi[pk] : ov.spPh[jp] + f * dPh;
            const float pan = (i & 1 ? 1.f : -1.f) * ov.spSpread * (i < 3 ? (float)i / 3.f : 1.f);
            for (int k = w.lo[i]; k <= w.hi[i]; ++k) {
                const int j = jp + (int)std::lround((float)(k - pk) * lin);
                if (j < 1 || j >= B - 1) continue;
                const float m = w.mag[k] * g;
                if (m <= w.nMag[j]) continue;
                w.nMag[j] = m;
                w.nPh[j] = phPeak + (w.phi[k] - w.phi[pk]);
                w.nPan[j] = pan;
                w.nFq[j] = f;
            }
        }
        // blur (temporal smoothing of the magnitudes); a bin that lost its partial keeps turning at that
        // partial's frequency, so a blurred tail stays in tune
        const float bl = ov.spBlur > 0.005f ? std::exp(-((float)H / fs) / ov.spBlur) : 0.f;
        for (int j = 0; j < B; ++j) {
            // a bin follows a new partial only where that partial is not just the faint skirt of a region
            if (w.nMag[j] > 0.f && (bl == 0.f || ov.spFresh || w.nMag[j] > ov.spMagS[j] * 0.3f)) { ov.spPh[j] = wrapPi(w.nPh[j]); ov.spFq[j] = w.nFq[j]; }
            else { if (ov.spFresh || ov.spFq[j] <= 0.f) ov.spFq[j] = (float)j * binOut; ov.spPh[j] = wrapPi(ov.spPh[j] + ov.spFq[j] * dPh); }
            ov.spMagS[j] = ov.spFresh ? w.nMag[j] : ov.spMagS[j] * bl + w.nMag[j] * (1.f - bl);
        }
        ov.spFresh = false;
        // pack L + iR into one inverse FFT (both channels are real)
        const float norm = 1.f / ((float)A.N * 1.5f) * 2.f;
        for (int j = 0; j < N; ++j) { w.re[j] = 0.f; w.im[j] = 0.f; }
        for (int j = 1; j < B - 1; ++j) {
            const float m = ov.spMagS[j] * norm;
            if (m <= 0.f) continue;
            float sn, cs; fastSinCos(ov.spPh[j], sn, cs);
            const float pn = clampf(w.nPan[j] + ov.pan, -1.f, 1.f);
            const float gl = std::sqrt(0.5f * (1.f - pn)) * 1.4142f, gr = std::sqrt(0.5f * (1.f + pn)) * 1.4142f;
            const float aL = m * cs * gl, bLm = m * sn * gl, aR = m * cs * gr, bR = m * sn * gr;
            w.re[j] = aL - bR; w.im[j] = bLm + aR;
            w.re[N - j] = aL + bR; w.im[N - j] = -bLm + aR;
        }
        spFft[N == 4096 ? 1 : 0].run(w.re, w.im, true);
        const float* win = spWin[N == 4096 ? 1 : 0].data();
        const int mask = N - 1;
        for (int i = 0; i < N; ++i) {
            const int at = (ov.spRp + i) & mask;
            ov.spOlaL[at] += w.re[i] * win[i] * 0.5f;
            ov.spOlaR[at] += w.im[i] * win[i] * 0.5f;
        }
        (void)o;
    }

    void renderSpectral(OscVoice& ov, int o, Scratch& sc, const float* fm, float* L, float* R, float* M, int n) {
        const SpecData* A = spslots[o][0].active;
        if (!A || A->frames < 1) { for (int s = 0; s < n; ++s) { L[s] = R[s] = M[s] = 0.f; } return; }
        const int N = fs <= 50000.f ? 2048 : 4096;
        if (ov.spN != N) {
            ov.spN = N; ov.spH = N / 4; ov.spBins = N / 2 + 1; ov.spRp = 0; ov.spToNext = 0; ov.spFresh = true;
            std::memset(ov.spOlaL, 0, sizeof(float) * (size_t)kSpMaxN); std::memset(ov.spOlaR, 0, sizeof(float) * (size_t)kSpMaxN);
            std::memset(ov.spMagS, 0, sizeof(float) * (size_t)kSpMaxBins); std::memset(ov.spPh, 0, sizeof(float) * (size_t)kSpMaxBins); std::memset(ov.spFq, 0, sizeof(float) * (size_t)kSpMaxBins);
        }
        if (ov.spFresh) {   // a new note or new data: clear what is still ringing from the last one
            std::memset(ov.spOlaL, 0, sizeof(float) * (size_t)N); std::memset(ov.spOlaR, 0, sizeof(float) * (size_t)N);
            ov.spToNext = 0;
        }
        SpScratch& w = spScr[&sc - scr];
        const SpecData* Bx = spslots[o][1].active;
        const int mask = N - 1;
        for (int s = 0; s < n; ++s) {
            if (ov.spToNext <= 0) { spectralHop(ov, o, w, *A, Bx); ov.spToNext = ov.spH; }
            const int r = ov.spRp;
            L[s] = ov.spOlaL[r]; R[s] = ov.spOlaR[r];
            ov.spOlaL[r] = 0.f; ov.spOlaR[r] = 0.f;
            ov.spRp = (r + 1) & mask;
            --ov.spToNext;
            M[s] = 0.5f * (L[s] + R[s]);
        }
        if (p[pLevelComp] >= 0.5f) {
            // 6.0: resynthesis runs ~4-9 dB under its source; +7 dB plus the source file's own level
            const float g = 2.24f * clampf(0.13f / std::max(A->rmsIn, 1e-4f), 0.25f, 8.f);
            for (int s = 0; s < n; ++s) { L[s] *= g; R[s] *= g; M[s] *= g; }
        }
        pmDelay(ov, fm, L, R, M, n);
        postFm(ov, fm, L, R, M, n);
    }

    void updateGranular(Voice& v, int o, float semis, float t1, float t2, float t3, float t4, float pan) {
        OscVoice& ov = v.o[o];
        ov.grSource = oiparam(o, ofGrSource, 0, 2);
        const float size = clampf(op(o, ofGrSize) * fastExp2(clampf(t2, -1.f, 1.f) * 3.f), 0.005f, 3.f);
        const float dens = clampf(op(o, ofGrDensity) * fastExp2(clampf(t3, -1.f, 1.f) * 3.f), 0.5f, 400.f);
        ov.grPos = clampf(op(o, ofGrPos) + t1, 0.f, 1.f);
        ov.grSpray = clampf(op(o, ofGrSpray) + t4, 0.f, 1.f);
        ov.grSizeS = size * fs;
        ov.grInterval = fs / dens;
        ov.grPitchRnd = clampf(op(o, ofGrPitchRand) + tx[1] * 1200.f, 0.f, 1200.f);
        ov.grRev = clampf(op(o, ofGrReverse) + tx[3], 0.f, 1.f);
        ov.grSpread = clampf(op(o, ofGrSpread) + tx[2], 0.f, 1.f);
        ov.grMax = grainCap();
        const float overlap = dens * size;
        ov.grGain = 0.9f / std::sqrt(overlap > 1.f ? overlap : 1.f);
        if (p[pLevelComp] >= 0.5f) {
            // 6.0 level compensation, fitted to measurements: about +5.5 dB overall, sparse clouds (overlap < 1)
            // lifted toward the level of a dense one (at most +9 dB, so single grains do not turn into peaks),
            // very dense clouds (where grains cancel) a little, the window shape's own loudness evened out,
            // and quiet / loud samples brought to a common RMS
            float c = 1.9f;
            if (overlap < 1.f) c *= std::min(2.8f, 1.f / std::sqrt(std::max(overlap, 0.01f)));
            else if (overlap > 20.f) c *= std::min(1.8f, std::pow(overlap / 20.f, 0.35f));
            const float sh = clampf(op(o, ofGrShape), 0.f, 1.f);
            c *= sh < 0.5f ? dbToGain(4.2f * (0.5f - sh)) : dbToGain(-7.8f * (sh - 0.5f));
            const SampleBuf* lsb = sslots[o].active;
            if (ov.grSource != 0 && lsb && lsb->len > 0) c *= clampf(0.13f / std::max(lsb->rms, 1e-4f), 0.5f, 4.f);
            ov.grGain *= c;
        }
        ov.grScan += clampf(op(o, ofGrScan) + tx[0], -1.f, 1.f) * 0.08f * (float)kBlock / fs;
        ov.grScan -= std::floor(ov.grScan);
        const SampleBuf* sb = sslots[o].active;
        if (ov.grSource == 0 || !sb) ov.grBaseInc = semisToHz(semis) / fs;                       // cycles per sample
        else if (ov.grSource == 1) ov.grBaseInc = semisToHz(semis) / 261.6256f * sb->sr / fs;    // C4 = original pitch
        else ov.grBaseInc = fastExp2((std::round(op(o, ofOct)) * 12.f + std::round(op(o, ofSemi)) + op(o, ofFine) * 0.01f) / 12.f) * sb->sr / fs;
        ov.pan = pan;
        v.telT[o][0] = ov.grPos; v.telT[o][1] = size; v.telT[o][2] = dens; v.telT[o][3] = ov.grSpray;
    }

    // ---- audio rate -------------------------------------------------------------------------
    static inline float warpPhase(int mode, float p, float a) {
        switch (mode) {
        case 1: { const float k = 8.f * a; return (1.f + k) * p / (1.f + k * p); }
        case 2: { const float k = -0.9f * a; return (1.f + k) * p / (1.f + k * p); }
        case 3: { const float k = 8.f * a;
                  if (p < 0.5f) { const float q = 2.f * p; return 0.5f * (1.f + k) * q / (1.f + k * q); }
                  const float q = 2.f - 2.f * p; return 1.f - 0.5f * (1.f + k) * q / (1.f + k * q); }
        case 4: { const float w = 1.f - 0.95f * a; return p < w ? p / w : 0.f; }
        case 5: { const float q = p * (1.f + 7.f * a); return q - std::floor(q); }
        case 6: { const float m = p < 0.5f ? 2.f * p : 2.f - 2.f * p; return p + (m - p) * a; }
        case 7: { const float k = 0.5f - 0.49f * a; return p < k ? 0.5f * p / k : 0.5f + 0.5f * (p - k) / (1.f - k); }
        case 8: { const float steps = 2.f + (1.f - a) * (1.f - a) * 254.f; return std::floor(p * steps) / steps; }
        default: return p;
        }
    }

    static inline bool fmIsPitch(int m) { return m <= 1 || m >= 4; }          // PM, FM, PM Hard, FM Hard
    static inline float fmHard(int m) { return m >= 4 ? 4.f : 1.f; }
    // 6.0: PM / FM for engines that have no single phase to bend (Additive, Physical, Spectral): the output runs
    // through a short modulated delay. Delay modulation is phase modulation of every partial at once (partial k
    // gets k times the index), the same as bending a wavetable's read phase; FM integrates the modulator first.
    void pmDelay(OscVoice& ov, const float* fm, float* L, float* R, float* M, int n) {
        const int m = ov.fmMode;
        if (!fm || ov.fmAmt <= 0.f || !fmIsPitch(m) || !(iparam(pFmV6, 0, 7) & (1 << ov.oscIdx))) { ov.pmOn = false; return; }
        float* bL = ov.pmBuf; float* bR = bL + kPmBufN; float* bM = bR + kPmBufN;
        if (!ov.pmOn) {
            ov.pmOn = true; ov.pmD = 0.f; ov.pmAcc = 0.f; ov.pmW = 0;
            std::memset(ov.pmBuf, 0, sizeof(float) * 3 * kPmBufN);
        }
        const bool isPm = m == 0 || m == 4;
        const float period = fs / (ov.hz > 10.f ? ov.hz : 10.f);
        const float a2 = ov.fmAmt * ov.fmAmt * fmHard(m);
        const float k = a2 * 8.f;                                   // FM: relative frequency deviation, as on the wavetable
        float D = isPm ? a2 * 2.f * period : k * period * (1.2f / 6.2832f);
        const float dMax = 0.5f * (float)kPmBufN - 8.f;
        if (D > dMax) D = dMax;
        const float cs = 1.f - std::exp(-1.f / (0.01f * fs)), leak = std::exp(-31.4159f / fs);
        const int mask = kPmBufN - 1;
        int w = ov.pmW; float pd = ov.pmD, acc = ov.pmAcc;
        for (int s = 0; s < n; ++s) {
            bL[w] = L[s]; bR[w] = R[s]; bM[w] = M[s];
            pd += (D - pd) * cs;
            float d;
            if (isPm) d = 2.f + pd * (1.f + fm[s]);
            else { acc = acc * leak - k * fm[s]; d = 2.f + pd + acc; }
            d = d < 2.f ? 2.f : (d > (float)kPmBufN - 4.f ? (float)kPmBufN - 4.f : d);
            const float pos = (float)w - d;
            const float fl = std::floor(pos);
            const float t = pos - fl;
            const int i0 = (int)fl;
            const int im = (i0 - 1) & mask, ia = i0 & mask, ib = (i0 + 1) & mask, ic = (i0 + 2) & mask;
            auto herm = [&](const float* b) {
                const float y0 = b[im], y1 = b[ia], y2 = b[ib], y3 = b[ic];
                const float c1 = 0.5f * (y2 - y0), c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3, c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
                return ((c3 * t + c2) * t + c1) * t + y1;
            };
            L[s] = herm(bL); R[s] = herm(bR); M[s] = herm(bM);
            w = (w + 1) & mask;
        }
        ov.pmW = w; ov.pmD = pd; ov.pmAcc = acc;
    }

    void postFm(OscVoice& ov, const float* fm, float* L, float* R, float* M, int n) {
        if (!fm || ov.fmAmt <= 0.f) return;
        if (ov.fmMode == 2) { for (int s = 0; s < n; ++s) { const float g = 1.f + ov.fmAmt * fm[s] * 1.5f; L[s] *= g; R[s] *= g; M[s] *= g; } }
        else if (ov.fmMode == 3) { for (int s = 0; s < n; ++s) { const float g = 1.f - ov.fmAmt + ov.fmAmt * fm[s] * 2.f; L[s] *= g; R[s] *= g; M[s] *= g; } }
    }


    // ---- 6.0 FM oscillator ------------------------------------------------------------------------
    // sine with ~1e-6 error (the operators' purity matters: the modulators' harmonics are multiplied by the index)
    static inline float fmSin(float x) {
        x -= std::floor(x);
        const float t = x < 0.5f ? x * 2.f - 0.5f : 1.5f - x * 2.f;          // triangle fold, [-0.5, 0.5]
        const float u = t * kPiF, u2 = u * u;
        return u * (1.f - u2 * (1.f / 6.f - u2 * (1.f / 120.f - u2 * (1.f / 5040.f - u2 * (1.f / 362880.f)))));
    }
    static inline float fmWave(int w, float ph) {
        ph -= std::floor(ph);
        switch (w) {
        default:
        case 0: return fmSin(ph);
        case 1: { const float s = fmSin(ph); return s > 0.f ? s : 0.f; }                                  // half sine
        case 2: return std::fabs(fmSin(ph));                                                               // abs sine
        case 3: { const float q = ph < 0.5f ? ph : ph - 0.5f; return q < 0.25f ? std::fabs(fmSin(ph)) : 0.f; }   // quarter
        case 4: return ph < 0.5f ? fmSin(ph * 2.f) : 0.f;                                                  // alternating
        case 5: return ph < 0.5f ? std::fabs(fmSin(ph * 2.f)) : 0.f;                                       // camel
        case 6: { const float s = 3.f * fmSin(ph); return s / (1.f + std::fabs(s)) * 1.3333333f; }         // soft square
        case 7: return (fmSin(ph) + 0.5f * fmSin(ph * 2.f) + 0.25f * fmSin(ph * 3.f)) * 0.68f;             // soft saw
        }
    }
    static inline float fmWaveDc(int w) {
        static const float dc[kFmWaves] = { 0.f, 0.3183099f, 0.6366198f, 0.3183099f, 0.f, 0.3183099f, 0.f, 0.f };
        return dc[w < 0 || w >= kFmWaves ? 0 : w];
    }
    float fmPv(int o, int f) const { return p[fmP(o, f)]; }
    float fmOpv(int o, int k, int f) const { return p[fmOpP(o, k, f)]; }

    void updateFm(Voice& v, int o, float semis, float t1, float t2, float t3, float t4, float pan, int n) {
        OscVoice& ov = v.o[o];
        const drift::FmAlgo& A = drift::fmAlgoDef((int)(fmPv(o, fmAlgo) + 0.5f));
        ov.fmAlg = (int)(fmPv(o, fmAlgo) + 0.5f);
        // unison (up to 4), same controls as the wavetable / analog engines
        int nu = oiparam(o, ofVoices, 1, kMaxUnison);
        if (nu > 4) nu = 4;
        if (nu > unisonCap()) nu = unisonCap();
        ov.nUni = nu;
        float detune = op(o, ofDetune) + v.md[mdDetuneAll] * 50.f + v.md[modDstOscBase(o) + 8] * 50.f;
        if (detune < 0.f) detune = 0.f;
        const float uniW = clampf(op(o, ofUniWidth), 0.f, 1.f);
        const float norm = 1.f / std::sqrt((float)nu);
        static const float panSignF[3] = { 1.f, -1.f, 0.6f };
        float uniCents[4];
        for (int j = 0; j < nu; ++j) {
            const float posn = nu == 1 ? 0.f : 2.f * j / (nu - 1) - 1.f;
            uniCents[j] = posn * detune;
            const float pn = clampf(posn * p[pSpread] * uniW * panSignF[o] + pan, -1.f, 1.f);
            float sa, ca; fastSinCos((pn + 1.f) * 0.25f * kPiF, sa, ca);
            ov.gL[j] = norm * ca * 1.4142f; ov.gR[j] = norm * sa * 1.4142f; ov.gM[j] = norm;
        }
        // oscillator-wide
        const float vel = v.vel;
        const float keyOct = (semis - 60.f) / 12.f;
        const float index = clampf(fmPv(o, fmIndex) + t1, 0.f, 3.f);
        const float fbAmt = clampf(fmPv(o, fmFeedback) + t2, 0.f, 1.f);
        const float bright = clampf(fmPv(o, fmBright) + t3, -1.f, 1.f);
        const float spreadC = clampf(fmPv(o, fmSpread) + t4, 0.f, 1.f) * 18.f;
        const float et = clampf(fmPv(o, fmEnvTime), 0.05f, 20.f);
        const float velIdx = fmPv(o, fmVelIdx);
        const float idxScale = index * (1.f - velIdx + velIdx * vel * 1.5f) * fastExp2(-fmPv(o, fmKeyIdx) * keyOct * 0.5f);
        // operators: frequency, envelope, level
        const bool held = v.gate || v.sustained;
        const float dtS = (float)n / fs;
        const float baseHz = semisToHz(semis);
        float incK[6];
        for (int k = 0; k < 6; ++k) {
            float hz;
            if (fmOpv(o, k, foFixed) >= 0.5f) hz = clampf(fmOpv(o, k, foHz), 1.f, 20000.f);
            else {
                const float c = std::floor(fmOpv(o, k, foCoarse) + 0.5f);
                const float ratio = (c < 0.5f ? 0.5f : c) * (1.f + fmOpv(o, k, foFine));
                hz = baseHz * ratio * fastExp2((fmOpv(o, k, foDetune) + ov.fmRnd[k] * spreadC) * (1.f / 1200.f));
            }
            for (int j = 0; j < nu; ++j) ov.fmInc[j][k] = clampf(hz * fastExp2(uniCents[j] * (1.f / 1200.f)) / fs, 0.f, 0.49f);
            incK[k] = ov.fmInc[0][k];
            // envelope (block rate; the render ramps across the block)
            const float atk = fmOpv(o, k, foAttack) * et, dec = fmOpv(o, k, foDecay) * et, sus = clampf(fmOpv(o, k, foSustain), 0.f, 1.f), rel = fmOpv(o, k, foRelease) * et;
            float e = ov.fmEnv[k]; int st = ov.fmStage[k];
            if (!held && st < 3) st = 3;
            if (st == 0) { e += dtS / std::max(atk, 1e-4f); if (e >= 1.f) { e = 1.f; st = 1; } }
            else if (st == 1) { e = sus + (e - sus) * std::exp(-dtS * 4.6f / std::max(dec, 1e-4f)); if (std::fabs(e - sus) < 1e-5f) { e = sus; st = 2; } }
            else if (st == 2) e = sus;
            else e *= std::exp(-dtS * 6.9f / std::max(rel, 1e-4f));
            ov.fmEnv[k] = e; ov.fmStage[k] = st;
            float lv = clampf(fmOpv(o, k, foLevel) + tx[k], 0.f, 1.f);
            lv *= lv;
            const float ov_ = fmOpv(o, k, foVel);
            lv *= 1.f - ov_ + ov_ * vel;
            lv *= fastExp2(-fmOpv(o, k, foKey) * keyOct);
            ov.fmE0[k] = ov.fmFresh ? 0.f : ov.fmE1[k];
            ov.fmE1[k] = e * lv;
            ov.fmWv[k] = (int)clampf(fmOpv(o, k, foWave) + 0.5f, 0.f, (float)(kFmWaves - 1));
            ov.fmDc[k] = (A.carriers >> k) & 1 ? fmWaveDc(ov.fmWv[k]) : 0.f;
        }
        // scales: carriers -> output gain, modulators -> phase deviation in turns (anti-alias tapered)
        // carriers: unity for one full-level carrier, power-normalised when several play (smooth in the levels)
        float cPow = 0.f;
        for (int k = 0; k < 6; ++k) if ((A.carriers >> k) & 1) { float l = clampf(fmOpv(o, k, foLevel) + tx[k], 0.f, 1.f); l *= l; cPow += l * l; }
        const float cS = 0.9f / std::sqrt(std::max(1.f, cPow));
        // operators above Nyquist fade out (an aliased operator is inharmonic in any role)
        float nyq[6];
        for (int k = 0; k < 6; ++k) {
            float mx = 0.f; for (int j = 0; j < nu; ++j) mx = std::max(mx, ov.fmInc[j][k]);
            nyq[k] = clampf((0.47f - mx) * 25.f, 0.f, 1.f);
            ov.fmE1[k] *= nyq[k];
        }
        // raw scales: carriers -> output gain, modulators -> phase deviation in turns
        float sc[6];
        for (int k = 0; k < 6; ++k)
            sc[k] = (A.carriers >> k) & 1 ? cS : 1.6f * idxScale * fastExp2(bright * 1.2f * (float)(A.depth[k] - 1));
        const int fbo = A.fbSrc;
        float fb = 0.32f * fbAmt;
        // band limit: estimate each operator's highest component (Carson's rule through the chains, feedback as
        // extra index) and scale every modulation down together until the carriers stay below 0.45 fs
        float lam = 1.f;
        const float spreadU = 1.f + detune * 0.0006f;
        for (int it = 0; it < 3; ++it) {
            float B[6], worst = 0.f, worstF = 0.f;
            for (int k = 5; k >= 0; --k) {
                const float fk = incK[k] * spreadU;
                float b = fk;
                for (int m = 5; m > k; --m) if ((A.mods[k] >> m) & 1) {
                    const float beta = lam * kTwoPi * sc[m] * std::max(ov.fmE0[m], ov.fmE1[m]);
                    b += (beta + std::min(beta * 4.f, 1.f)) * B[m];
                }
                if (k == A.fbDst) b += lam * fb * std::max(ov.fmE0[fbo], ov.fmE1[fbo]) * 8.f * incK[fbo];
                B[k] = b;
                if ((A.carriers >> k) & 1 && ov.fmE1[k] > 1e-6f && b - fk > worst - worstF) { worst = b; worstF = fk; }
            }
            if (worst <= 0.45f) break;
            const float room = std::max(0.f, 0.45f - worstF), used = worst - worstF;
            lam *= std::max(0.f, room / std::max(used, 1e-9f));
        }
        for (int k = 0; k < 6; ++k) {
            const float v = (A.carriers >> k) & 1 ? sc[k] : sc[k] * lam;
            ov.fmS0[k] = ov.fmFresh ? v : ov.fmS1[k];
            ov.fmS1[k] = v;
        }
        fb *= lam;
        ov.fmFb0 = ov.fmFresh ? fb : ov.fmFb1; ov.fmFb1 = fb;
        ov.fmHpR = 1.f - kTwoPi * 8.f / fs;
        ov.fmFresh = false;
        v.telT[o][0] = index; v.telT[o][1] = fbAmt; v.telT[o][2] = bright; v.telT[o][3] = spreadC / 18.f;
    }

    static inline void fmWaveBlock(int w, const float* x, float* y, int n) {
        switch (w) {
        case 0: for (int s = 0; s < n; ++s) y[s] = fmSin(x[s]); break;
        default: for (int s = 0; s < n; ++s) y[s] = fmWave(w, x[s]); break;
        }
    }
    void renderFm(OscVoice& ov, const float* fm, float* L, float* R, float* M, int n) {
        for (int s = 0; s < n; ++s) { L[s] = 0.f; R[s] = 0.f; M[s] = 0.f; }
        const drift::FmAlgo& A = drift::fmAlgoDef(ov.fmAlg);
        const bool epm = fm && fmIsPitch(ov.fmMode) && ov.fmAmt > 0.f;
        const float epmIdx = ov.fmAmt * ov.fmAmt * 0.6f * fmHard(ov.fmMode);
        const float inv = 1.f / (float)n;
        const int fbS = A.fbSrc, fbD = A.fbDst;
        bool live[6];
        for (int k = 0; k < 6; ++k) live[k] = ov.fmE0[k] > 1e-7f || ov.fmE1[k] > 1e-7f;
        // per-sample envelope x level and scale ramps (shared by the unison voices)
        float E[6][kBlock], S[6][kBlock], FB[kBlock];
        for (int k = 0; k < 6; ++k) {
            if (!live[k]) continue;
            const float dE = (ov.fmE1[k] - ov.fmE0[k]) * inv, dS = (ov.fmS1[k] - ov.fmS0[k]) * inv;
            for (int s = 0; s < n; ++s) { E[k][s] = ov.fmE0[k] + dE * (float)s; S[k][s] = ov.fmS0[k] + dS * (float)s; }
        }
        { const float d = (ov.fmFb1 - ov.fmFb0) * inv; for (int s = 0; s < n; ++s) FB[s] = ov.fmFb0 + d * (float)s; }
        const float hpR = ov.fmHpR;
        for (int j = 0; j < ov.nUni; ++j) {
            float* ph = ov.fmPh[j];
            const float* inc = ov.fmInc[j];
            float acc[kBlock];
            for (int s = 0; s < n; ++s) acc[s] = 0.f;
            if (fbS == fbD) {
                // operator-major: every operator renders its whole block, modulators first (op 6 .. 1)
                float out[6][kBlock];
                for (int k = 5; k >= 0; --k) {
                    const float p0 = ph[k], dp = inc[k];
                    ph[k] = p0 + dp * (float)n; ph[k] -= std::floor(ph[k]);
                    if (!live[k]) { for (int s = 0; s < n; ++s) out[k][s] = 0.f; continue; }
                    const bool car = (A.carriers >> k) & 1;
                    float x[kBlock];
                    for (int s = 0; s < n; ++s) x[s] = p0 + dp * (float)s;
                    if (car && epm) for (int s = 0; s < n; ++s) x[s] += epmIdx * fm[s];
                    const uint8_t mm = A.mods[k];
                    for (int m = 5; m > k; --m) if ((mm >> m) & 1) for (int s = 0; s < n; ++s) x[s] += out[m][s] * S[m][s];
                    float w[kBlock];
                    if (k == fbD && live[fbS]) {
                        float y1 = ov.fmY1[j], y2 = ov.fmY2[j];
                        const int wv = ov.fmWv[k];
                        for (int s = 0; s < n; ++s) {
                            const float v = fmWave(wv, x[s] + FB[s] * 0.5f * (y1 + y2));
                            w[s] = v; y2 = y1; y1 = v * std::min(E[k][s], 1.f);
                        }
                        ov.fmY1[j] = y1; ov.fmY2[j] = y2;
                    } else fmWaveBlock(ov.fmWv[k], x, w, n);
                    for (int s = 0; s < n; ++s) out[k][s] = w[s] * E[k][s];
                    if (car) { const float dc = ov.fmDc[k]; for (int s = 0; s < n; ++s) acc[s] += (w[s] - dc) * E[k][s] * S[k][s]; }
                }
            } else {
                // a feedback loop across operators (algorithms 4 and 6): sample by sample
                float y1 = ov.fmY1[j], y2 = ov.fmY2[j];
                for (int s = 0; s < n; ++s) {
                    float out[6];
                    const float ext = epm ? epmIdx * fm[s] : 0.f;
                    for (int k = 5; k >= 0; --k) {
                        if (!live[k]) { out[k] = 0.f; continue; }
                        float pm = ph[k] + inc[k] * (float)s;
                        const uint8_t mm = A.mods[k];
                        for (int m = 5; m > k; --m) if ((mm >> m) & 1) pm += out[m] * S[m][s];
                        if (k == fbD) pm += FB[s] * 0.5f * (y1 + y2);
                        const bool car = (A.carriers >> k) & 1;
                        if (car) pm += ext;
                        const float w = fmWave(ov.fmWv[k], pm);
                        out[k] = w * E[k][s];
                        if (car) acc[s] += (w - ov.fmDc[k]) * E[k][s] * S[k][s];
                        if (k == fbS) { y2 = y1; y1 = w * std::min(E[k][s], 1.f); }
                    }
                }
                ov.fmY1[j] = y1; ov.fmY2[j] = y2;
                for (int k = 0; k < 6; ++k) { ph[k] += inc[k] * (float)n; ph[k] -= std::floor(ph[k]); }
            }
            // DC blocker (1:1 ratios and the asymmetric waves put energy at 0 Hz)
            float hx = ov.fmHpX[j], hy = ov.fmHpY[j];
            const float gl = ov.gL[j], gr = ov.gR[j], gm = ov.gM[j];
            for (int s = 0; s < n; ++s) { const float y = acc[s] - hx + hpR * hy; hx = acc[s]; hy = y; L[s] += y * gl; R[s] += y * gr; M[s] += y * gm; }
            ov.fmHpX[j] = hx; ov.fmHpY[j] = hy;
        }
        postFm(ov, fm, L, R, M, n);
    }

    void renderWavetable(OscVoice& ov, int o, const float* fm, const float* syncW, float* W, float* L, float* R, float* M, int n) {
        const WaveTable& base = slots[o].active ? *slots[o].active : *builtin->banks[ov.bank];
        const WaveTable& T = (ov.blend == 1 && base.spectral) ? *base.spectral : base;
        const int nf = T.frames;
        const float fpos = ov.pos * (float)(nf - 1);
        int f0 = ov.blend == 2 ? (int)(fpos + 0.5f) : (int)fpos; if (f0 > nf - 2) f0 = nf - 2 > 0 ? nf - 2 : 0;
        const float ff = ov.blend == 2 ? (fpos + 0.5f >= (float)(nf - 1) && nf > 1 ? 1.f : 0.f) : fpos - (float)f0;
        const float* ta = wtTable(T, f0, ov.mip);
        const float* tb = wtTable(T, nf > 1 ? f0 + 1 : f0, ov.mip);
        for (int s = 0; s < n; ++s) { L[s] = 0.f; R[s] = 0.f; M[s] = 0.f; W[s] = -1.f; }
        const int warp = ov.warp; const float wa = ov.warpAmt;
        const bool pm = fm && (ov.fmMode == 0 || ov.fmMode == 4) && ov.fmAmt > 0.f, lfm = fm && (ov.fmMode == 1 || ov.fmMode == 5) && ov.fmAmt > 0.f;
        const float pmIdx = ov.fmAmt * ov.fmAmt * 2.f * fmHard(ov.fmMode), fmIdx = ov.fmAmt * ov.fmAmt * 8.f * fmHard(ov.fmMode);
        for (int j = 0; j < ov.nUni; ++j) {
            float ph = ov.ph[j];
            const float dt = ov.inc[j], gl = ov.gL[j], gr = ov.gR[j], gm = ov.gM[j];
            for (int s = 0; s < n; ++s) {
                if (syncW && syncW[s] >= 0.f) ph = dt * syncW[s];
                float rp = ph;
                if (pm) { rp += pmIdx * fm[s]; rp -= std::floor(rp); }
                if (warp) rp = warpPhase(warp, rp, wa);
                const float x = rp * (float)kWtSize;
                int xi = (int)x; if (xi >= kWtSize) xi = kWtSize - 1; if (xi < 0) xi = 0;
                const float xf = x - (float)xi;
                const float a = ta[xi] + (ta[xi + 1] - ta[xi]) * xf;
                const float b = tb[xi] + (tb[xi + 1] - tb[xi]) * xf;
                float w = a + (b - a) * ff;
                if (warp == 9 && rp >= 1.f - 0.5f * wa) w = -w;
                L[s] += w * gl; R[s] += w * gr; M[s] += w * gm;
                const float step = lfm ? dt * (1.f + fmIdx * fm[s]) : dt;
                ph += step;
                if (ph >= 1.f) { ph -= std::floor(ph); if (j == 0) W[s] = step > 1e-9f ? ph / step : 0.f; }
                else if (ph < 0.f) ph -= std::floor(ph);
            }
            ov.ph[j] = ph;
        }
        postFm(ov, fm, L, R, M, n);
    }

    static inline float anNaive(int wave, float q, float pw) {
        switch (wave) {
        case 0: case 5: return 2.f * q - 1.f;
        case 1: return q < pw ? 1.f : -1.f;
        case 2: return 0.5f * ((2.f * q - 1.f) + (q < pw ? 1.f : -1.f));
        case 3: return q < 0.5f ? 4.f * q - 1.f : 3.f - 4.f * q;
        default: return fastSinTurns(q);
        }
    }

    void renderAnalog(OscVoice& ov, const float* fm, const float* syncW, float* W, float* L, float* R, float* M, int n) {
        for (int s = 0; s < n; ++s) { L[s] = 0.f; R[s] = 0.f; M[s] = 0.f; W[s] = -1.f; }
        const int wave = ov.wave;
        const float pw = ov.pw;
        const bool ext = syncW != nullptr;
        const bool sync = ov.sync && !ext ? true : false;
        const bool lfm = fm && fmIsPitch(ov.fmMode) && ov.fmAmt > 0.f;
        const float fmIdx = ov.fmAmt * ov.fmAmt * 6.f * fmHard(ov.fmMode);
        const bool tri = wave == 3;
        // jump sizes at slave wrap / pw edge, per wave
        const float wrapJ = (wave == 0 || wave == 5) ? -2.f : (wave == 1 ? 2.f : 0.f);
        const float edgeJ = wave == 1 ? -2.f : (wave == 2 ? -1.f : 0.f);
        for (int j = 0; j < ov.nUni; ++j) {
            float p0 = ov.ph[j], q = ov.sph[j], y1 = ov.y1[j];
            const float dtBase = ov.inc[j], sdtBase = (sync || ext) ? ov.sinc[j] : ov.inc[j];
            const float gl = ov.gL[j], gr = ov.gR[j], gm = ov.gM[j];
            for (int s = 0; s < n; ++s) {
                const float fmul = lfm ? clampf(1.f + fmIdx * fm[s], 0.f, 8.f) : 1.f;
                const float dt = clampf(dtBase * fmul, 0.f, 0.45f), sdt = clampf(sdtBase * fmul, 1e-7f, 0.45f);
                float corrCur = 0.f;
                const float qPrev = q;
                q += sdt;
                bool reset = false;
                float d = 0.f;
                if (sync) {
                    p0 += dt;
                    if (p0 >= 1.f) { p0 -= 1.f; d = dt > 1e-9f ? p0 / dt : 0.f; reset = true; if (j == 0) W[s] = d; }
                } else if (ext && syncW[s] >= 0.f) { d = syncW[s]; reset = true; }
                if (reset) {
                    if (d > 1.f) d = 1.f;
                    float qr = q - sdt * d;                  // slave phase at reset instant
                    qr -= std::floor(qr);
                    const float qn = sdt * d;
                    const float J = anNaive(wave, 0.f, pw) - anNaive(wave, qr, pw);
                    y1 += 0.5f * J * d * d;
                    corrCur -= 0.5f * J * (1.f - d) * (1.f - d);
                    q = qn;
                    if (edgeJ != 0.f && q >= pw) { const float d2 = (q - pw) / sdt; y1 += 0.5f * edgeJ * d2 * d2; corrCur -= 0.5f * edgeJ * (1.f - d2) * (1.f - d2); }
                } else {
                    if (q >= 1.f) {
                        q -= 1.f;
                        const float dw = q / sdt;
                        if (!sync && !ext && j == 0) W[s] = dw;
                        if (wrapJ != 0.f) { y1 += 0.5f * wrapJ * dw * dw; corrCur -= 0.5f * wrapJ * (1.f - dw) * (1.f - dw); }
                        if (tri) { const float S = 8.f * sdt; y1 += S * dw * dw * dw * (1.f / 6.f); corrCur += S * (1.f - dw) * (1.f - dw) * (1.f - dw) * (1.f / 6.f); }
                        if (edgeJ != 0.f && q >= pw) {
                            const float d2 = (q - pw) / sdt;
                            y1 += 0.5f * edgeJ * d2 * d2; corrCur -= 0.5f * edgeJ * (1.f - d2) * (1.f - d2);
                        }
                    } else {
                        if (edgeJ != 0.f && qPrev < pw && q >= pw) {
                            const float d2 = (q - pw) / sdt;
                            y1 += 0.5f * edgeJ * d2 * d2; corrCur -= 0.5f * edgeJ * (1.f - d2) * (1.f - d2);
                        }
                        if (tri && qPrev < 0.5f && q >= 0.5f) {
                            const float d2 = (q - 0.5f) / sdt, S = -8.f * sdt;
                            y1 += S * d2 * d2 * d2 * (1.f / 6.f); corrCur += S * (1.f - d2) * (1.f - d2) * (1.f - d2) * (1.f / 6.f);
                        }
                    }
                }
                const float out = y1;                    // one sample of latency lets the BLEP correct the past
                y1 = anNaive(wave, q, pw) + corrCur;
                L[s] += out * gl; R[s] += out * gr; M[s] += out * gm;
            }
            ov.ph[j] = sync ? p0 : q; ov.sph[j] = q; ov.y1[j] = y1;
        }
        postFm(ov, fm, L, R, M, n);
    }

    // 6.0.5: W partials (W/8 groups of 8) advance together. One group alone is a serial chain per sample
    // (each rotation waits for the previous one), so the CPU sat idle; several independent groups fill it.
    template <int W>
    static inline void additiveGroups(OscVoice& ov, int g, int n, float (*accL)[8], float (*accR)[8], float (*accM)[8]) {
        float sv[W], cv[W], av[W], rc[W], rs[W], da[W], pl[W], pr[W];
        for (int k = 0; k < W; ++k) { sv[k] = ov.aS[g + k]; cv[k] = ov.aC[g + k]; av[k] = ov.aAmp[g + k]; rc[k] = ov.aRc[g + k]; rs[k] = ov.aRs[g + k]; da[k] = ov.aDa[g + k]; pl[k] = ov.aPl[g + k]; pr[k] = ov.aPr[g + k]; }
        for (int t = 0; t < n; ++t) {
            float xl[8] = {}, xr[8] = {}, xm[8] = {};
            for (int j = 0; j < W; j += 8)
                for (int k = 0; k < 8; ++k) {
                    const int i = j + k;
                    const float sn = sv[i] * rc[i] + cv[i] * rs[i];
                    const float cn = cv[i] * rc[i] - sv[i] * rs[i];
                    sv[i] = sn; cv[i] = cn;
                    av[i] += da[i];
                    const float x = sn * av[i];
                    xl[k] += x * pl[i]; xr[k] += x * pr[i]; xm[k] += x;
                }
            for (int k = 0; k < 8; ++k) { accL[t][k] += xl[k]; accR[t][k] += xr[k]; accM[t][k] += xm[k]; }
        }
        for (int k = 0; k < W; ++k) { ov.aS[g + k] = sv[k]; ov.aC[g + k] = cv[k]; ov.aAmp[g + k] = av[k]; }
    }
    void renderAdditive(OscVoice& ov, const float* fm, float* L, float* R, float* M, int n) {
        // 8 partials per vector lane group; per-sample accumulators keep the loops vectorisable
        const int cnt = (ov.aCount + 7) & ~7;
        float accL[kBlock][8], accR[kBlock][8], accM[kBlock][8];
        for (int t = 0; t < n; ++t) for (int k = 0; k < 8; ++k) { accL[t][k] = 0.f; accR[t][k] = 0.f; accM[t][k] = 0.f; }
        int g = 0;
        for (; g + 32 <= cnt; g += 32) additiveGroups<32>(ov, g, n, accL, accR, accM);
        if (g + 16 <= cnt) { additiveGroups<16>(ov, g, n, accL, accR, accM); g += 16; }
        if (g + 8 <= cnt) { additiveGroups<8>(ov, g, n, accL, accR, accM); g += 8; }
        for (int t = 0; t < n; ++t) {
            float l = 0.f, r = 0.f, m = 0.f;
            for (int k = 0; k < 8; ++k) { l += accL[t][k]; r += accR[t][k]; m += accM[t][k]; }
            L[t] = l; R[t] = r; M[t] = m;
        }
        pmDelay(ov, fm, L, R, M, n);
        postFm(ov, fm, L, R, M, n);
    }

    static constexpr float kPulseGain = 0.9f;
    static constexpr float kReedLag = 0.f, kFluteLag = -0.2f, kBloom = 0.3f;
    void renderPhysical(OscVoice& ov, int o, Rng& rng, const float* excIn, const float* fm, float* L, float* R, float* M, int n) {
        const int model = oiparam(o, ofPmModel, 0, kPmModels - 1);
        const int exc = oiparam(o, ofPmExciter, 0, kPmExciters - 1);
        const bool wg = pmIsWaveguide(model);
        const float hard = op(o, ofPmHard);
        const float burstP = ov.burstRate / fs;
        const float burstDecay = exc == 4 ? std::exp(-1.f / (0.002f * fs)) : std::exp(-1.f / (0.0008f * fs));
        const float pr = 2.f * ov.press;             // continuous exciters: 1 at the default pressure
        const float nz = ov.noiseAmt;
        const SampleBuf* sb = exc == 8 ? sslots[o].active : nullptr;
        const float smpInc = sb ? sb->sr / fs : 0.f;
        if ((exc == 6 || exc == 7) && ov.impulse > 0.f) { ov.excT = 0.f; ov.excAmp = ov.impulse; ov.impulse = 0.f; }   // note on: start the pulse
        const float invLen = 1.f / (ov.excLen > 1.f ? ov.excLen : 1.f);
        const float pulseK = kPulseGain * 2.f * invLen * (wg ? 40.f : 60.f);
        float excB[kBlock];
        for (int s = 0; s < n; ++s) {
            const float noise = rng.bi();
            ov.excLP += (noise - ov.excLP) * ov.excCoef;
            float e;
            switch (exc) {
            case 0: e = ov.excLP * ov.strikeEnv + (wg ? 0.f : ov.impulse * (0.08f + 0.25f * hard)); ov.strikeEnv *= ov.strikeDecay; ov.impulse = 0.f; break;   // strings: no raw spike
            case 1: { ov.bowPh += ov.bowInc; if (ov.bowPh >= 1.f) ov.bowPh -= 1.f;
                      e = ov.excLP * 0.9f * (0.55f + 0.45f * fastSinTurns(ov.bowPh)) * pr; break; }
            case 2: e = ((noise - ov.excLP) * 0.3f + ov.excLP * 0.2f) * pr; break;
            case 3: e = excIn[s] * 0.5f * pr; break;
            case 4: { if (rng.uni() < burstP) ov.burstEnv = 0.6f + 0.4f * rng.uni(); ov.burstEnv *= burstDecay;
                      e = ov.excLP * (0.25f + ov.burstEnv) * 1.1f * pr; break; }
            case 5: { if (rng.uni() < burstP) ov.burstEnv = (0.3f + 0.7f * rng.uni()) * (rng.uni() < 0.5f ? -1.f : 1.f); ov.burstEnv *= burstDecay;
                      e = (ov.burstEnv * 1.6f + ov.excLP * 0.02f) * pr; break; }
            case 6: {   // pluck: one raised-cosine displacement pulse, shorter = brighter
                      e = 0.f;
                      if (ov.excT < ov.excLen) { e = ov.excAmp * (0.5f - 0.5f * fastSinTurns(ov.excT * invLen + 0.25f)) * pulseK; ov.excT += 1.f; }
                      break; }
            case 7: {   // felt mallet: half-sine contact force plus a little felt noise
                      e = 0.f;
                      if (ov.excT < ov.excLen) { e = ov.excAmp * (fastSinTurns(0.5f * ov.excT * invLen) + 0.15f * (1.f - hard) * noise) * pulseK * 0.64f; ov.excT += 1.f; }
                      break; }
            default: {  // the oscillator's grain sample, looped, drives the resonator
                      e = 0.f;
                      if (sb) {
                          const float lim = (float)(sb->len - 2);
                          float pp = ov.smpPos; if (pp >= lim || pp < 0.f) pp = 0.f;
                          const int i0 = (int)pp; const float f = pp - (float)i0;
                          e = (sb->data[i0] + (sb->data[i0 + 1] - sb->data[i0]) * f) * 0.8f * pr;
                          ov.smpPos = pp + smpInc;
                      }
                      break; }
            }
            if (nz > 0.f) e += noise * nz * 0.25f;
            if (model == pmReed) {
                const float breath = (0.2f + 0.75f * ov.press) * (1.f + nz * 0.25f * noise) + e * 0.3f;   // chokes above ~0.88 (overblown)
                float rp = (float)ov.wgPos - ov.wgDelay; if (rp < 0.f) rp += (float)kWgSize;
                const int i0 = (int)rp; const float fr = rp - (float)i0;
                const float y = ov.wg[i0 & (kWgSize - 1)] * (1.f - fr) + ov.wg[(i0 + 1) & (kWgSize - 1)] * fr;
                ov.boreLP += (-0.95f * y - ov.boreLP) * ov.bellC;          // bell: inverted, lowpassed reflection
                const float pd = ov.boreLP - breath;
                const float rt = clampf(ov.reedOff + ov.reedSlope * pd, -1.f, 1.f);
                ov.wg[ov.wgPos] = clampf(breath + pd * rt, -2.f, 2.f);
                ov.wgPos = (ov.wgPos + 1) & (kWgSize - 1);
                ov.dcY = y - ov.dcX + 0.995f * ov.dcY; ov.dcX = y;
                const float out = ov.dcY * 1.6f;
                L[s] = out * ov.panL; R[s] = out * ov.panR; M[s] = out;
            } else if (model == pmFlute) {
                const float breath = (0.35f + 0.75f * ov.press) * (1.f + (0.06f + nz * 0.3f) * noise) + e * 0.3f;
                float rp = (float)ov.wgPos - ov.wgDelay; if (rp < 0.f) rp += (float)kWgSize;
                const int i0 = (int)rp; const float fr = rp - (float)i0;
                const float y = ov.wg[i0 & (kWgSize - 1)] * (1.f - fr) + ov.wg[(i0 + 1) & (kWgSize - 1)] * fr;
                ov.boreLP += (y - ov.boreLP) * ov.bellC;
                const float tv = -ov.boreLP;
                ov.dcY = tv - ov.dcX + 0.995f * ov.dcY; ov.dcX = tv;
                const float temp = ov.dcY;
                float* jet = ov.sym;
                jet[ov.jetPos] = breath - 0.5f * temp;
                float jr = (float)ov.jetPos - ov.jetDelay; if (jr < 0.f) jr += (float)kSymSize;
                const int j0 = (int)jr; const float jf = jr - (float)j0;
                const float jv = jet[j0 & (kSymSize - 1)] * (1.f - jf) + jet[(j0 + 1) & (kSymSize - 1)] * jf;
                ov.jetPos = (ov.jetPos + 1) & (kSymSize - 1);
                const float jt = clampf(jv * (jv * jv - 1.f), -1.f, 1.f);   // jet deflection nonlinearity
                ov.wg[ov.wgPos] = clampf(jt + 0.5f * temp, -2.f, 2.f);
                ov.wgPos = (ov.wgPos + 1) & (kWgSize - 1);
                ov.dc2Y = y - ov.dc2X + 0.995f * ov.dc2Y; ov.dc2X = y;
                const float out = ov.dc2Y * 16.f;
                L[s] = out * ov.panL; R[s] = out * ov.panR; M[s] = out;
            } else if (wg) {
                float dly = ov.wgDelay;
                if (model == pmTanpura) {
                    // tanpura: where the string grazes the curved bridge it is effectively shorter, so the
                    // delay shrinks with displacement. Energy is kept; the phase modulation makes the jawari buzz.
                    const float a = std::fabs(ov.wgLP);
                    const float ex = a > 0.015f ? a - 0.015f : 0.f;
                    dly -= fclamp(ov.buzz * ov.buzz * 60.f * ex, 0.f, ov.wgDelay * 0.2f);
                }
                if (ov.nonlin > 0.f) dly *= 1.f - ov.nonlin * 0.03f * fclamp(std::fabs(ov.wgLP) * 2.f, 0.f, 1.f);   // tension modulation: louder = sharper
                float rp = (float)ov.wgPos - dly;
                if (rp < 0.f) rp += (float)kWgSize;
                const int i0 = (int)rp; const float fr = rp - (float)i0;
                const float y = ov.wg[i0 & (kWgSize - 1)] * (1.f - fr) + ov.wg[(i0 + 1) & (kWgSize - 1)] * fr;
                ov.wgLP += (y - ov.wgLP) * ov.wgBright;
                float fbv = ov.wgLP * ov.wgFb;
                ov.wg[ov.wgPos] = satTanh(e * (exc == 0 ? 7.f : 1.2f) + fbv);
                ov.wgPos = (ov.wgPos + 1) & (kWgSize - 1);
                const float tap = ov.wg[(ov.wgPos + kWgSize - 1 - (int)(ov.wgDelay * ov.wgTap)) & (kWgSize - 1)];
                const float sp = ov.wgSpread;
                float out = y;
                float symL = 0.f, symR = 0.f;
                if (model == pmSympathetic) {
                    for (int k = 0; k < kSymLines; ++k) {
                        float* line = ov.sym + k * kSymSize;
                        float r2 = (float)ov.symPos - ov.symDelay[k];
                        if (r2 < 0.f) r2 += (float)kSymSize;
                        const int j0 = (int)r2; const float f2 = r2 - (float)j0;
                        const float ys = line[j0 & (kSymSize - 1)] * (1.f - f2) + line[(j0 + 1) & (kSymSize - 1)] * f2;
                        ov.symLP[k] += (ys - ov.symLP[k]) * 0.6f;
                        line[ov.symPos] = ov.symLP[k] * ov.symFb + y * 0.04f;
                        if (k & 1) symR += ys; else symL += ys;
                    }
                    ov.symPos = (ov.symPos + 1) & (kSymSize - 1);
                    symL *= ov.buzz * 1.4f; symR *= ov.buzz * 1.4f;
                    out += (symL + symR) * 0.5f;
                }
                const float pl = (y * (1.f - 0.5f * sp) + tap * 0.5f * sp) * 4.4f + symL * 4.4f;
                const float pr2 = (y * (1.f - 0.5f * sp) - tap * 0.5f * sp) * 4.4f + symR * 4.4f;
                L[s] = pl * ov.panL; R[s] = pr2 * ov.panR; M[s] = out * 2.2f;
            } else {
                excB[s] = e;    // modal banks run below, vectorised across modes
            }
        }
        if (!wg) renderModes(ov, excB, L, R, M, n);
        pmDelay(ov, fm, L, R, M, n);
        postFm(ov, fm, L, R, M, n);
    }
    // 6.0.5: W modes (W/8 groups) run together, as in renderAdditive: independent resonators fill the CPU's pipeline
    template <int W>
    static inline void modeGroups(OscVoice& ov, int g, const float* ex, int n, float (*accL)[8], float (*accR)[8], float (*accM)[8]) {
        float y1[W], y2[W], a1[W], a2[W], b[W], pl[W], pr[W];
        for (int k = 0; k < W; ++k) {
            const bool live = g + k < ov.mCount;
            y1[k] = ov.mY1[g + k]; y2[k] = ov.mY2[g + k]; a1[k] = live ? ov.mA1[g + k] : 0.f; a2[k] = live ? ov.mA2[g + k] : 0.f;
            b[k] = live ? ov.mB[g + k] : 0.f; pl[k] = ov.mPl[g + k]; pr[k] = ov.mPr[g + k];
        }
        for (int t = 0; t < n; ++t) {
            const float e = ex[t];
            float xl[8] = {}, xr[8] = {}, xm[8] = {};
            for (int j = 0; j < W; j += 8)
                for (int k = 0; k < 8; ++k) {
                    const int i = j + k;
                    const float y = b[i] * e + a1[i] * y1[i] - a2[i] * y2[i];
                    y2[i] = y1[i]; y1[i] = y;
                    xl[k] += y * pl[i]; xr[k] += y * pr[i]; xm[k] += y;
                }
            for (int k = 0; k < 8; ++k) { accL[t][k] += xl[k]; accR[t][k] += xr[k]; accM[t][k] += xm[k]; }
        }
        for (int k = 0; k < W; ++k) {
            // 6.0.5 guard: a mode driven past any audible level (the bowed / blown feedback models can run away when
            // rounding tips them over the edge) or gone NaN restarts silent instead of taking the voice down with it
            const float m = std::fabs(y1[k]) + std::fabs(y2[k]);
            if (!(m < 1e4f)) { y1[k] = 0.f; y2[k] = 0.f; }
            ov.mY1[g + k] = y1[k]; ov.mY2[g + k] = y2[k];
        }
    }
    void renderModes(OscVoice& ov, const float* exc, float* L, float* R, float* M, int n) {
        const int cnt = (ov.mCount + 7) & ~7;
        float accL[kBlock][8], accR[kBlock][8], accM[kBlock][8];
        for (int t = 0; t < n; ++t) for (int k = 0; k < 8; ++k) { accL[t][k] = 0.f; accR[t][k] = 0.f; accM[t][k] = 0.f; }
        const float nl = ov.nonlin;
        float exc2[kBlock];
        const float* ex = exc;
        int g = 0;
        if (nl > 0.f && cnt > 8) {
            modeGroups<8>(ov, 0, ex, n, accL, accR, accM);
            // 4.2 nonlinearity: the upper modes are also driven by the square of the lowest eight
            // (energy cascades upward, the gong / tam-tam bloom)
            for (int t = 0; t < n; ++t) { float ls = 0.f; for (int k = 0; k < 8; ++k) ls += accM[t][k]; exc2[t] = exc[t] + nl * kBloom * satTanh(ls * ls * 3.f); }
            ex = exc2; g = 8;
        }
        for (; g + 32 <= cnt; g += 32) modeGroups<32>(ov, g, ex, n, accL, accR, accM);
        if (g + 16 <= cnt) { modeGroups<16>(ov, g, ex, n, accL, accR, accM); g += 16; }
        if (g + 8 <= cnt) { modeGroups<8>(ov, g, ex, n, accL, accR, accM); g += 8; }
        for (int t = 0; t < n; ++t) {
            float l = 0.f, r = 0.f, m = 0.f;
            for (int k = 0; k < 8; ++k) { l += accL[t][k]; r += accR[t][k]; m += accM[t][k]; }
            L[t] = satTanh(l * 0.6f) * 1.3f; R[t] = satTanh(r * 0.6f) * 1.3f; M[t] = satTanh(m * 0.6f) * 1.3f;
        }
    }

    void prepareWindows() {
        for (int o = 0; o < 3; ++o) {
            const float sh = op(o, ofGrShape);
            if (sh == winKey[o]) continue;
            winKey[o] = sh;
            for (int i = 0; i <= kWinSize; ++i) {
                const float x = (float)i / kWinSize;
                const float hann = 0.5f - 0.5f * std::cos(kTwoPi * x);
                float w;
                if (sh < 0.5f) w = std::pow(hann, 1.f + 4.f * (0.5f - sh));           // narrower, gaussian-like
                else {                                                               // tukey: flat top widens
                    const float a = 1.f - (sh - 0.5f) * 1.8f;                         // taper fraction
                    if (a <= 0.001f) w = 1.f;
                    else if (x < a * 0.5f) w = 0.5f - 0.5f * std::cos(kTwoPi * x / a);
                    else if (x > 1.f - a * 0.5f) w = 0.5f - 0.5f * std::cos(kTwoPi * (1.f - x) / a);
                    else w = 1.f;
                }
                winTab[o][i] = w;
            }
            winTab[o][kWinSize] = 0.f;
        }
    }

    void spawnGrain(OscVoice& ov, int o, Rng& rng) {
        int slot = -1, active = 0;
        for (int g = 0; g < kMaxGrains; ++g) { if (ov.gr[g].on) ++active; else if (slot < 0) slot = g; }
        if (slot < 0 || active >= ov.grMax) return;
        Grain& g = ov.gr[slot];
        const SampleBuf* sb = sslots[o].active;
        const bool sample = ov.grSource != 0 && sb;
        float pos = ov.grPos + ov.grScan + (rng.bi()) * ov.grSpray * 0.5f;
        pos -= std::floor(pos);
        const float pr = ov.grPitchRnd > 0.f ? fastExp2(rng.bi() * ov.grPitchRnd * (1.f / 1200.f)) : 1.f;
        const bool rev = rng.uni() < ov.grRev;
        g.on = true; g.age = 0.f;
        const float len = ov.grSizeS > 16.f ? ov.grSizeS : 16.f;
        g.ageInc = 1.f / len;
        const float pn = clampf(ov.pan + rng.bi() * ov.grSpread, -1.f, 1.f);
        const float ang = (pn + 1.f) * 0.25f * kPiF;
        g.gl = std::cos(ang) * 1.4142f; g.gr = std::sin(ang) * 1.4142f;
        if (sample) {
            const float span = (float)(sb->len - 2);
            g.pos = pos * span;
            g.inc = ov.grBaseInc * pr * (rev ? -1.f : 1.f);
        } else {
            const WaveTable& T = slots[o].active ? *slots[o].active : *builtin->banks[oiparam(o, ofTable, 0, kWtBanks - 1)];
            g.frame = (int)(pos * (float)(T.frames - 1) + 0.5f);
            g.pos = rng.uni();
            g.inc = ov.grBaseInc * pr;
            g.mip = wtMipFor(std::fabs(g.inc) * 1.02f);
        }
    }

    void renderGranular(OscVoice& ov, int o, Rng& rng, const float* fm, float* L, float* R, float* M, int n) {
        for (int s = 0; s < n; ++s) { L[s] = 0.f; R[s] = 0.f; M[s] = 0.f; }
        const SampleBuf* sb = sslots[o].active;
        const bool sample = ov.grSource != 0 && sb;
        const WaveTable& T = slots[o].active ? *slots[o].active : *builtin->banks[oiparam(o, ofTable, 0, kWtBanks - 1)];
        const float* win = winTab[o];
        const bool lfm = fm && fmIsPitch(ov.fmMode) && ov.fmAmt > 0.f;
        const float fmIdx = ov.fmAmt * ov.fmAmt * 6.f * fmHard(ov.fmMode);
        // scheduling, sample-accurate within the block
        for (int s = 0; s < n; ++s) {
            ov.grTimer -= 1.f;
            if (ov.grTimer <= 0.f) { spawnGrain(ov, o, rng); ov.grTimer += ov.grInterval * (0.75f + 0.5f * rng.uni()); }
        }
        // the grains just spawned start at the block start (<= 16 samples of jitter is inaudible here)
        const float gg = ov.grGain;
        for (int k = 0; k < kMaxGrains; ++k) {
            Grain& g = ov.gr[k];
            if (!g.on) continue;
            if (sample) {
                const float* d = sb->data; const float lim = (float)(sb->len - 2);
                for (int s = 0; s < n; ++s) {
                    float x = g.age * (float)kWinSize; int xi = (int)x; if (xi >= kWinSize) xi = kWinSize - 1;
                    const float w = win[xi] + (win[xi + 1] - win[xi]) * (x - (float)xi);
                    float pp = g.pos;
                    if (pp < 0.f) pp += lim; if (pp >= lim) pp -= lim;
                    const int i0 = (int)pp; const float f = pp - (float)i0;
                    const float y = (d[i0] + (d[i0 + 1] - d[i0]) * f) * w * gg;
                    L[s] += y * g.gl; R[s] += y * g.gr; M[s] += y;
                    g.pos = pp + g.inc * (lfm ? 1.f + fmIdx * fm[s] : 1.f);
                    g.age += g.ageInc;
                    if (g.age >= 1.f) { g.on = false; break; }
                }
            } else {
                const float* tab = wtTable(T, g.frame < T.frames ? g.frame : T.frames - 1, g.mip);
                for (int s = 0; s < n; ++s) {
                    float x = g.age * (float)kWinSize; int xi = (int)x; if (xi >= kWinSize) xi = kWinSize - 1;
                    const float w = win[xi] + (win[xi + 1] - win[xi]) * (x - (float)xi);
                    const float tx = g.pos * (float)kWtSize; int ti = (int)tx; if (ti >= kWtSize) ti = kWtSize - 1;
                    const float y = (tab[ti] + (tab[ti + 1] - tab[ti]) * (tx - (float)ti)) * w * gg;
                    L[s] += y * g.gl; R[s] += y * g.gr; M[s] += y;
                    g.pos += g.inc * (lfm ? 1.f + fmIdx * fm[s] : 1.f);
                    g.pos -= std::floor(g.pos);
                    g.age += g.ageInc;
                    if (g.age >= 1.f) { g.on = false; break; }
                }
            }
        }
        postFm(ov, fm, L, R, M, n);
    }

    inline float processFilter(Voice& v, int which, int ch, float x, float g) {
        const bool f1 = which == 0;
        FilterCoefs& c = f1 ? v.c1 : v.c2;
        FilterCoefs& cb = f1 ? v.c1b : v.c2b;
        MultiFilter& fa = f1 ? (ch ? v.f1R : v.f1L) : (ch ? v.f2R : v.f2L);
        MultiFilter& fb = f1 ? (ch ? v.f1bR : v.f1bL) : (ch ? v.f2bR : v.f2bL);
        const int ta = f1 ? v.ft1 : v.ft2, tb = f1 ? v.ft1b : v.ft2b;
        const float res = f1 ? v.res1 : v.res2, mo = f1 ? v.morph1 : v.morph2;
        c.g = g; cb.g = g;
        if (mo <= 0.001f) return fa.process(ta, x, res, c);
        if (mo >= 0.999f) return fb.process(tb, x, res, cb);
        const float ya = fa.process(ta, x, res, c), yb = fb.process(tb, x, res, cb);
        return ya + (yb - ya) * mo;
    }

    void renderBlock(Voice& v, Scratch& sc, float* outL, float* outR, int n, int blk) {
        Rng& rng = v.vr;
        float (&bL)[3][kBlock] = sc.bL; float (&bR)[3][kBlock] = sc.bR; float (&bM)[3][kBlock] = sc.bM; float (&bW)[3][kBlock] = sc.bW;
        float* feB = sc.feB; float* aeB = sc.aeB; float* subB = sc.subB; float* noiseB = sc.noiseB; float* mixPk = sc.mixPk;
        updateControl(v, blk, n);
        const EnvCoefs& cF = v.envMod ? v.ec[0] : ecF; const EnvCoefs& cA = v.envMod ? v.ec[1] : ecA; const EnvCoefs& cM = v.envMod ? v.ec[2] : ecM;
        for (int s = 0; s < n; ++s) { feB[s] = v.fe.tick(cF); aeB[s] = v.ae.tick(cA); }
        for (int s = 0; s < n; ++s) v.me.tick(cM);

        // sub + noise first: both can be FM sources
        const WaveTable& st = *builtin->banks[0];
        const float* stab = wtTable(st, v.subFrame, v.subMip);
        for (int s = 0; s < n; ++s) {
            const float x = v.subPhase * (float)kWtSize;
            const int xi = (int)x;
            subB[s] = stab[xi] + (stab[xi + 1] - stab[xi]) * (x - (float)xi);
            v.subPhase += v.subInc; if (v.subPhase >= 1.f) v.subPhase -= 1.f;
            noiseB[s] = rng.bi();
        }

        bool done[3] = { false, false, false };
        const int ringSrc = iparam(pRingSource, 0, 3);
        static const bool kRingUse[4][3] = { { 1, 1, 0 }, { 1, 0, 1 }, { 0, 1, 1 }, { 1, 1, 1 } };
        bool ringOn = v.ringL > 0.f;
        for (int o = 0; o < 3; ++o) if (kRingUse[ringSrc][o] && !v.o[o].on) ringOn = false;
        bool needed[3] = { false, false, false };
        for (int o = 0; o < 3; ++o) {
            const OscVoice& ov = v.o[o];
            if (!ov.on) continue;
            if (ov.level > 0.f || (ringOn && kRingUse[ringSrc][o])) needed[o] = true;
            if (ov.type == otPhysical && oiparam(o, ofPmExciter, 0, kPmExciters - 1) == 3) for (int k = 0; k < 3; ++k) if (k != o) needed[k] = true;
            if (ov.fmSrc >= 1 && ov.fmSrc <= 3 && ov.fmAmt > 0.f) needed[ov.fmSrc - 1] = true;
            if (ov.syncSrc) needed[ov.syncSrc - 1] = true;
        }
        if (v.fmSrc1 >= 1 && v.fmSrc1 <= 3 && v.fmAmt1 > 0.f) needed[v.fmSrc1 - 1] = true;
        if (v.fmSrc2 >= 1 && v.fmSrc2 <= 3 && v.fmAmt2 > 0.f) needed[v.fmSrc2 - 1] = true;
        auto srcBuf = [&](int k) -> const float* { return done[k] ? bM[k] : v.prevM[k]; };
        auto wrapBuf = [&](int k) -> const float* { return done[k] ? bW[k] : v.prevW[k]; };
        auto fmBuf = [&](const OscVoice& ov) -> const float* {
            if (!ov.fmSrc || ov.fmAmt <= 0.f) return nullptr;
            if (ov.fmSrc <= 3) return srcBuf(ov.fmSrc - 1);
            return ov.fmSrc == 4 ? subB : noiseB;
        };
        for (int o = 0; o < 3; ++o) {
            OscVoice& ov = v.o[o];
            for (int s = 0; s < n; ++s) bW[o][s] = -1.f;
            if (!ov.on || ov.type == otPhysical) continue;
            if (!needed[o]) { for (int s = 0; s < n; ++s) bL[o][s] = bR[o][s] = bM[o][s] = 0.f; done[o] = true; continue; }
            const float* fm = fmBuf(ov);
            const float* sw = ov.syncSrc && (ov.type == otWavetable || ov.type == otAnalog) ? wrapBuf(ov.syncSrc - 1) : nullptr;
            if (ov.type == otWavetable) renderWavetable(ov, o, fm, sw, bW[o], bL[o], bR[o], bM[o], n);
            else if (ov.type == otAnalog) renderAnalog(ov, fm, sw, bW[o], bL[o], bR[o], bM[o], n);
            else if (ov.type == otAdditive) renderAdditive(ov, fm, bL[o], bR[o], bM[o], n);
            else if (ov.type == otSpectral) renderSpectral(ov, o, sc, fm, bL[o], bR[o], bM[o], n);
            else if (ov.type == otFm) renderFm(ov, fm, bL[o], bR[o], bM[o], n);
            else renderGranular(ov, o, rng, fm, bL[o], bR[o], bM[o], n);
            done[o] = true;
        }
        for (int o = 0; o < 3; ++o) {
            OscVoice& ov = v.o[o];
            if (!ov.on) { for (int s = 0; s < n; ++s) bL[o][s] = bR[o][s] = bM[o][s] = 0.f; done[o] = true; continue; }
            if (ov.type != otPhysical) continue;
            float exc[kBlock];
            for (int s = 0; s < n; ++s) { float e = 0.f; for (int k = 0; k < 3; ++k) if (k != o && v.o[k].on) e += srcBuf(k)[s]; exc[s] = e; }
            renderPhysical(ov, o, rng, exc, fmBuf(ov), bL[o], bR[o], bM[o], n);
            done[o] = true;
        }
        for (int o = 0; o < 3; ++o) for (int s = 0; s < n; ++s) { v.prevM[o][s] = bM[o][s]; v.prevW[o][s] = bW[o][s]; }

        // mix into buses (bus 2 is only used by split routing: B + C)
        float aL[kBlock], aR[kBlock], cL[kBlock], cR[kBlock];
        const bool split = v.routing == 2;
        for (int s = 0; s < n; ++s) { aL[s] = aR[s] = cL[s] = cR[s] = 0.f; }
        for (int o = 0; o < 3; ++o) {
            const OscVoice& ov = v.o[o];
            if (!ov.on || ov.level <= 0.f) continue;
            float* dl = (split && o > 0) ? cL : aL;
            float* dr = (split && o > 0) ? cR : aR;
            const float lv = ov.level;
            float pk = 0.f;
            for (int s = 0; s < n; ++s) { dl[s] += bL[o][s] * lv; dr[s] += bR[o][s] * lv; pk = std::max(pk, std::fabs(bL[o][s]) + std::fabs(bR[o][s])); }
            mixPk[o] = std::max(mixPk[o], pk * 0.5f * lv * v.ae.v);
        }
        if (ringOn) {
            for (int s = 0; s < n; ++s) {
                float r = v.ringL;
                for (int o = 0; o < 3; ++o) if (kRingUse[ringSrc][o]) r *= bM[o][s];
                if (ringSrc == 3) r *= 2.f;
                float* dl = split ? cL : aL; float* dr = split ? cR : aR;
                dl[s] += r; dr[s] += r;
                mixPk[3] = std::max(mixPk[3], std::fabs(r) * v.ae.v);
            }
        }
        if (v.subL > 0.f) { for (int s = 0; s < n; ++s) { const float y = subB[s] * v.subL; aL[s] += y; aR[s] += y; } mixPk[4] = std::max(mixPk[4], v.subL * v.ae.v); }
        if (v.noiseL > 0.f) {
            const float col = p[pNoiseColor];
            mixPk[5] = std::max(mixPk[5], v.noiseL * 0.5f * v.ae.v);
            for (int s = 0; s < n; ++s) {
                const float white = noiseB[s];
                v.pink[0] = 0.99765f * v.pink[0] + white * 0.0990460f;
                v.pink[1] = 0.96300f * v.pink[1] + white * 0.2965164f;
                v.pink[2] = 0.57000f * v.pink[2] + white * 1.0526913f;
                const float pink = (v.pink[0] + v.pink[1] + v.pink[2] + white * 0.1848f) * 0.25f;
                const float w2 = rng.bi();
                const float nl = (pink * (1.f - col) + white * 0.5f * col) * v.noiseL;
                const float nr = (pink * (1.f - col) + w2 * 0.5f * col) * v.noiseL;
                aL[s] += nl + 0.1f * w2 * v.noiseL; aR[s] += nr - 0.1f * w2 * v.noiseL;
            }
        }

        // filters + amp
        const float invN = 1.f / (float)n;
        const float piFs = kPiF / fs, nyqLog = std::log2(0.45f * fs);
        const float k1 = 1.f + v.drv1 * 5.f, kc1 = 1.4f / (1.f + v.drv1 * 1.5f);
        const float k2 = 1.f + v.drv2 * 5.f, kc2 = 1.4f / (1.f + v.drv2 * 1.5f);
        const bool d1 = v.drv1 > 0.001f, d2 = v.drv2 > 0.001f;
        const bool osDrv = curQuality == 1;   // HQ already runs voices at 2x; Eco stays cheap
        const float bal = v.balance;
        const float g1 = split ? (bal < 0.5f ? 1.f : 2.f * (1.f - bal)) : 1.f - bal;
        const float g2 = split ? (bal > 0.5f ? 1.f : 2.f * bal) : bal;
        const float* fmF1 = v.fmAmt1 > 0.f && v.fmSrc1 ? (v.fmSrc1 <= 3 ? srcBuf(v.fmSrc1 - 1) : noiseB) : nullptr;
        const float* fmF2 = v.fmAmt2 > 0.f && v.fmSrc2 ? (v.fmSrc2 <= 3 ? srcBuf(v.fmSrc2 - 1) : noiseB) : nullptr;
        const float fmD1 = v.fmAmt1 * v.fmAmt1 * 6.f, fmD2 = v.fmAmt2 * v.fmAmt2 * 6.f;
        const bool on1 = v.ft1 != ftOff || (v.morph1 > 0.f && v.ft1b != ftOff), on2 = v.ft2 != ftOff || (v.morph2 > 0.f && v.ft2b != ftOff);
        for (int s = 0; s < n; ++s) {
            const float t = (float)(s + 1) * invN;
            float g1c = 0.f, g2c = 0.f;
            if (on1) {
                float lg = v.f1Prev + (v.f1Log - v.f1Prev) * t + feB[s] * v.env1;
                if (fmF1) lg += fmD1 * fmF1[s];
                g1c = fastTan(fastExp2(clampf(lg, 4.f, nyqLog)) * piFs);
            }
            if (on2) {
                float lg = v.f2Prev + (v.f2Log - v.f2Prev) * t + feB[s] * v.env2;
                if (fmF2) lg += fmD2 * fmF2[s];
                g2c = fastTan(fastExp2(clampf(lg, 4.f, nyqLog)) * piFs);
            }
            float xl = aL[s], xr = aR[s], yl, yr;
            if (v.routing == 0) {                         // serial
                if (d1) { if (osDrv) { xl = v.os1L.run(v.dt1, xl * k1) * kc1; xr = v.os1R.run(v.dt1, xr * k1) * kc1; } else { xl = driveShape(v.dt1, xl * k1) * kc1; xr = driveShape(v.dt1, xr * k1) * kc1; } }
                yl = on1 ? processFilter(v, 0, 0, xl, g1c) : xl;
                yr = on1 ? processFilter(v, 0, 1, xr, g1c) : xr;
                if (on2) {
                    if (d2) { if (osDrv) { yl = v.os2L.run(v.dt2, yl * k2) * kc2; yr = v.os2R.run(v.dt2, yr * k2) * kc2; } else { yl = driveShape(v.dt2, yl * k2) * kc2; yr = driveShape(v.dt2, yr * k2) * kc2; } }
                    yl = processFilter(v, 1, 0, yl, g2c);
                    yr = processFilter(v, 1, 1, yr, g2c);
                }
            } else {
                float al1 = xl, ar1 = xr;
                float bl2 = split ? cL[s] : xl, br2 = split ? cR[s] : xr;
                if (d1) { if (osDrv) { al1 = v.os1L.run(v.dt1, al1 * k1) * kc1; ar1 = v.os1R.run(v.dt1, ar1 * k1) * kc1; } else { al1 = driveShape(v.dt1, al1 * k1) * kc1; ar1 = driveShape(v.dt1, ar1 * k1) * kc1; } }
                if (d2) { if (osDrv) { bl2 = v.os2L.run(v.dt2, bl2 * k2) * kc2; br2 = v.os2R.run(v.dt2, br2 * k2) * kc2; } else { bl2 = driveShape(v.dt2, bl2 * k2) * kc2; br2 = driveShape(v.dt2, br2 * k2) * kc2; } }
                if (on1) { al1 = processFilter(v, 0, 0, al1, g1c); ar1 = processFilter(v, 0, 1, ar1, g1c); }
                if (on2) { bl2 = processFilter(v, 1, 0, bl2, g2c); br2 = processFilter(v, 1, 1, br2, g2c); }
                yl = al1 * g1 + bl2 * g2; yr = ar1 * g1 + br2 * g2;
            }
            float gain = aeB[s] * (v.gainPrev + (v.gain - v.gainPrev) * t) * 0.35f;
            if (v.pending || v.stealCount > 0) {
                gain *= (float)v.stealCount / (float)kStealFade;
                if (--v.stealCount <= 0) {
                    if (v.pending) { v.restart = true; return; }   // the main thread starts the stolen note (startVoice is not thread-safe)
                    v.active = false; return;
                }
            }
            outL[s] += yl * gain; outR[s] += yr * gain;
        }
        if (v.ae.stage == Env::Idle && !v.pending) v.active = false;
    }

    // ---- 4.2 voice rendering on the worker pool --------------------------------------------------
    bool rendered[kMaxVoices] = {};
    void renderVoiceChunk(Voice& v, Scratch& sc, float* L, float* R, int b0) {
        for (int b = b0; b < taskNb && v.active; ++b) {
            const int bn = (b == taskNb - 1) ? taskNI - b * kBlock : kBlock;
            renderBlock(v, sc, L + b * kBlock, R + b * kBlock, bn, b);
            if (v.restart) { v.restartBlk = b + 1; return; }
        }
    }
    void renderTask(int task, int slot) {
        const int vi = taskList[task];
        std::memset(vbL[vi], 0, sizeof(float) * (size_t)taskNI);
        std::memset(vbR[vi], 0, sizeof(float) * (size_t)taskNI);
        renderVoiceChunk(voices[vi], scr[slot < drift::kMaxPoolSlots ? slot : 0], vbL[vi], vbR[vi], 0);
    }
    static void voiceTask(void* ctx, int task, int slot) { static_cast<Engine*>(ctx)->renderTask(task, slot); }
    void renderVoices(int nb, int nI) {
        taskNb = nb; taskNI = nI; taskCount = 0;
        for (int i = 0; i < kMaxVoices; ++i) { rendered[i] = voices[i].active; if (rendered[i]) taskList[taskCount++] = i; }
        for (auto& sc : scr) for (float& k : sc.mixPk) k = 0.f;
        drift::IWorkerPool* pl = pool.load(std::memory_order_acquire);
        if (pl && pl->workers() > 0 && taskCount >= 2) pl->run(taskCount, &voiceTask, this);
        else for (int t = 0; t < taskCount; ++t) renderTask(t, 0);
        // a stolen voice finished its fade: start the pending note here, in voice order, and render the rest of the chunk
        for (int t = 0; t < taskCount; ++t) {
            Voice& v = voices[taskList[t]];
            for (int guard = 0; v.restart && guard < 4; ++guard) {
                v.restart = false;
                genCtx = v.pGen; startVoice(v, v.pCh, v.pNote, v.pVel, true); genCtx = false;
                renderVoiceChunk(v, scr[0], vbL[taskList[t]], vbR[taskList[t]], v.restartBlk);
            }
            v.restart = false;
        }
        for (const auto& sc : scr) for (int k = 0; k < 6; ++k) mixPk[k] = std::max(mixPk[k], sc.mixPk[k]);
    }

    // ---- 4.2 effective macros: scene morph, then recorded motion ---------------------------------
    void updateMacros(int n) {
        const float dt = (float)n / fsOut;
        float base[kMacros];
        for (int m = 0; m < kMacros; ++m) base[m] = p[pMacro1 + m];
        const float tgt = clampf(p[pScPos], 0.f, (float)(kScenes - 1));
        if (p[pScOn] >= 0.5f) {
            const float gl = p[pScGlide];
            scPosS = gl > 0.001f ? scPosS + (tgt - scPosS) * (1.f - std::exp(-dt / (gl * 0.5f))) : tgt;
            const SceneData& sd = scenes[sceneIdx.load(std::memory_order_acquire)];
            int i0 = (int)std::floor(scPosS); if (i0 < 0) i0 = 0; if (i0 > kScenes - 1) i0 = kScenes - 1;
            const int i1 = i0 + 1 < kScenes ? i0 + 1 : i0;
            const float fr = clampf(scPosS - (float)i0, 0.f, 1.f);
            for (int m = 0; m < kMacros; ++m) {
                const float a = sd.used[i0] ? sd.v[i0][m] : base[m], b = sd.used[i1] ? sd.v[i1][m] : base[m];
                base[m] = a + (b - a) * fr;
            }
        } else scPosS = tgt;
        for (bool& l : macroLane) l = false;
        const int mode = iparam(pMoMode, 0, 3);
        const MotionData& md = *motion[motionIdx.load(std::memory_order_acquire)];
        // 6.0: while overdubbing only the recorded macros pause; the other lanes keep playing in time
        const int recMask = md.recording ? md.recMask : 0;
        if (mode != 0 && md.frames > 1 && recMask != 0xFF && md.seconds > 0.f) {
            const double len = md.seconds, sp = p[pMoSpeed];
            if (mode == 3) { if (hostPlaying) motT = beat * 60.0 / hostBpm * sp; }
            else { if (mode == 2 && motRestart) motT = 0.0; motT += (double)dt * sp; }
            motT = std::fmod(motT, len); if (motT < 0.0) motT += len;
            const float fpos = (float)(motT / len * (double)md.frames);
            int i0 = (int)fpos; if (i0 >= md.frames) i0 = md.frames - 1;
            const int i1 = (i0 + 1) % md.frames;
            const float fr = fpos - (float)i0;
            for (int m = 0; m < kMacros; ++m) if (md.has[m] && !(recMask & (1 << m))) { base[m] = md.v[m][i0] + (md.v[m][i1] - md.v[m][i0]) * fr; macroLane[m] = true; }
        }
        motRestart = false;
        // 5.1.1: XY pad auto-motion moves the pad's two macros around where the pad sits
        for (int pd = 0; pd < 2; ++pd) {
            const uint32_t o = pd ? (uint32_t)(pXy2Mode - pXy1Mode) : 0u;
            const int mode = iparam(pXy1Mode + o, 0, kXyShapes - 1);
            if (mode == 0) continue;
            if (p[pXy1Sync + o] >= 0.5f) {
                const double L = (double)kDivBeats[iparam(pXy1Div + o, 0, kDivs - 1)];
                if (hostPlaying) xyPh[pd] = beat / L;
                else xyPh[pd] += (double)dt * hostBpm / 60.0 / L;
            } else xyPh[pd] += (double)dt * (double)p[pXy1Rate + o];
            if (xyPh[pd] > 1e6) xyPh[pd] = std::fmod(xyPh[pd], 8.0);
            float x, y; xyShape(mode, xyPh[pd], x, y);
            if (p[pXy1Dir + o] >= 0.5f) y = -y;
            const float sz = 0.5f * clampf(p[pXy1Size + o], 0.f, 1.f);
            const int ax = xyAxes[pd * 2].load(std::memory_order_relaxed), ay = xyAxes[pd * 2 + 1].load(std::memory_order_relaxed);
            base[ax] = clampf(base[ax] + sz * x, 0.f, 1.f);
            if (ay != ax) base[ay] = clampf(base[ay] + sz * y, 0.f, 1.f);
        }
        for (int m = 0; m < kMacros; ++m) macroEff[m] = base[m];
    }

    // ---- 4.2 Resonator: string pitches from the tuning (and the held notes) ----------------------
    void updateResonator() {
        if (p[pRsOn] < 0.5f) return;
        const int mode = iparam(pRsMode, 0, 5), root = iparam(pRsRoot, 0, 11), oct = iparam(pRsOct, 1, 5);
        const int base = 12 * (oct + 1) + root;
        static const float iv[5][8] = { { 0, 7, 12, 19, 24, 31, 36, 43 }, { 0, 12, 24, 36, 48, 12.04f, 23.96f, 36.03f },
                                        { 0, 4, 7, 11, 12, 14, 16, 19 }, { 0, 3, 7, 10, 12, 14, 15, 19 }, { 0, 7, 12, 12.05f, 19, 24, 0.03f, 7.02f } };
        auto tuned = [&](float x) { int ni = (int)std::floor(x); const float fr = x - (float)ni; if (ni < 0) ni = 0; if (ni > 127) ni = 127; return tunedPitch(ni, 0) + fr; };
        if (mode < 5) { for (int k = 0; k < 8; ++k) rsPitch[k] = tuned((float)base + iv[mode][k]); fx.setResonator(rsPitch); return; }
        int held[8], nh = 0;
        for (const auto& v : voices) {
            if (!v.active || !v.gate || nh >= 8) continue;
            bool dup = false; for (int i = 0; i < nh; ++i) if (held[i] == v.note) dup = true;
            if (!dup) held[nh++] = v.note;
        }
        if (nh == 0) { if (!rsHaveHeld) { for (int k = 0; k < 8; ++k) rsPitch[k] = tuned((float)base + iv[0][k]); } fx.setResonator(rsPitch); return; }
        std::sort(held, held + nh);
        for (int k = 0; k < 8; ++k) {
            int note = held[k % nh];
            while (note >= base + 12) note -= 12;       // lowest octave of each held note at or above the root octave
            while (note < base) note += 12;
            note += 12 * (k / nh);
            while (note > base + 48) note -= 12;
            rsPitch[k] = tuned((float)note);
        }
        rsHaveHeld = true;
        fx.setResonator(rsPitch);
    }

    void processChunk(float* L, float* R, int n) {
        consumePending();
        updateTuning();
        enforceGuard();
        updateMacros(n);
        updateResonator();
        const int nI = n * os;
        envCoefs(ecF, p[pFEnvA], p[pFEnvD], p[pFEnvS], p[pFEnvR], fs, p[pFEnvCurve], envClassic.load(std::memory_order_relaxed), p[pFRelCurve], p[pFDecCurve]);
        envCoefs(ecA, p[pAEnvA], p[pAEnvD], p[pAEnvS], p[pAEnvR], fs, p[pAEnvCurve], envClassic.load(std::memory_order_relaxed), p[pARelCurve], p[pADecCurve]);
        envCoefs(ecM, p[pMEnvA], p[pMEnvD], p[pMEnvS], p[pMEnvR], fs, p[pMEnvCurve], envClassic.load(std::memory_order_relaxed), p[pMRelCurve], p[pMDecCurve]);
        prepareWindows();
        const int nb = (nI + kBlock - 1) / kBlock;
        const float t1 = std::log2(clampf(p[pF1Cutoff], 16.f, 20000.f)), t2 = std::log2(clampf(p[pF2Cutoff], 16.f, 20000.f));
        const double bps = hostBpm / 60.0;
        for (int b = 0; b < nb; ++b) {
            const int bn = (b == nb - 1) ? nI - b * kBlock : kBlock;
            const float sm = 1.f - std::exp(-(float)bn / (0.02f * fs));
            f1Smooth += (t1 - f1Smooth) * sm; f2Smooth += (t2 - f2Smooth) * sm;
            f1Blk[b] = f1Smooth; f2Blk[b] = f2Smooth;
            for (int i = 0; i < kLfos; ++i) {
                const int wave = iparam(lfoP(i, lfWave), 0, kLfoWaves - 1);
                const float* shp = lfoShape[i][lfoShapeIdx[i].load(std::memory_order_relaxed)];
                const bool sync = p[lfoP(i, lfSync)] >= 0.5f;
                if (sync && hostPlaying) {
                    // phase locked to the song position
                    const double div = (double)kDivBeats[iparam(lfoP(i, lfDiv), 0, kDivs - 1)];
                    const double bb = beat + (double)b * kBlock / (double)fs * bps;
                    const float ph = (float)(bb / div - std::floor(bb / div)) + p[lfoP(i, lfPhase)];
                    const float np = ph - std::floor(ph);
                    if (np < gLfo[i].phase - 0.5f) { gLfo[i].shPrev = gLfo[i].sh; gLfo[i].sh = rng.bi(); }
                    gLfo[i].phase = np;
                    gLfoVal[b][i] = lfoValue(wave, gLfo[i], shp);
                    gLfoRate[i] = (float)(bps / div);
                } else {
                    gLfoVal[b][i] = lfoValue(wave, gLfo[i], shp);
                    float rate = sync ? (float)(bps / (double)kDivBeats[iparam(lfoP(i, lfDiv), 0, kDivs - 1)]) : p[lfoP(i, lfRate)];
                    rate *= fastExp2(clampf(gmd[mdLfo1Rate + i], -1.f, 1.f) * 4.f);
                    lfoAdvance(gLfo[i], rate * (float)bn / fs, rng);
                    gLfoRate[i] = rate;
                }
                const float smt = p[lfoP(i, lfSmooth)];
                if (smt > 0.001f) { const float k = 1.f - std::exp(-((float)bn / fs) / (smt * smt * 0.5f + 1e-4f)); gLfo[i].smooth += (gLfoVal[b][i] - gLfo[i].smooth) * k; gLfoVal[b][i] = gLfo[i].smooth; }
            }
            for (int i = 0; i < kLfos; ++i) gLfoLast[i] = gLfoVal[b][i];
            rndG.tick(p[pRndGRate] * (float)bn / fs, rng);
        }
        // sequencer CV glide
        {
            const float gl = p[pSeqGlide];
            const float k = gl > 0.001f ? 1.f - std::exp(-((float)n / fsOut) / (gl * gl * 0.5f)) : 1.f;
            seqCv += (seqCvTarget - seqCv) * k;
        }
        float* bufL = os == 2 ? vL : L;
        float* bufR = os == 2 ? vR : R;
        for (int i = 0; i < nI; ++i) { bufL[i] = 0.f; bufR[i] = 0.f; }
        renderVoices(nb, nI);
        for (int vi = 0; vi < kMaxVoices; ++vi) {
            if (!rendered[vi]) continue;
            const float* a = vbL[vi]; const float* b = vbR[vi];
            for (int i = 0; i < nI; ++i) { bufL[i] += a[i]; bufR[i] += b[i]; }
        }
        if (os == 2) decimate(vL, vR, L, R, n);
        // global modulation for the FX (voice-only sources follow the newest voice)
        {
            float src[kModSrcCount];
            globalSources(src);
            const Voice* nv = newestVoice();
            if (nv) for (int i = 0; i < kModSrcCount; ++i) if (!isGlobalSrc(i)) src[i] = nv->src[i];
            evalMatrix(src, gmd);
        }
        fx.process(L, R, n, p, gmd);
        const float mTgt = dbToGain(p[pMaster]);
        const float mSm = 1.f - std::exp(-1.f / (0.01f * fsOut));
        for (int i = 0; i < n; ++i) {
            masterS += (mTgt - masterS) * mSm; L[i] *= masterS; R[i] *= masterS;
            const float al = std::fabs(L[i]), ar = std::fabs(R[i]);
            if (al > peakL) peakL = al;
            if (ar > peakR) peakR = ar;
        }
        if (tel) {
            // scope: stereo, full rate (every 2nd sample above 60 kHz)
            int w = tel->scopeWrite;
            const int step = fsOut > 60000.f ? 2 : 1;
            tel->scopeRate = fsOut / (float)step;
            for (int i = 0; i < n; ++i) {
                scopeDecim += 1.f;
                if (scopeDecim >= (float)step) { scopeDecim = 0.f; tel->scope[w] = L[i]; tel->scopeR[w] = R[i]; w = (w + 1) & (drift::kScopeSize - 1); }
            }
            __atomic_store_n(&tel->scopeWrite, w, __ATOMIC_RELEASE);
        }
    }

    static bool isGlobalSrc(int s) {
        return (s >= msLfo1 && s <= msLfo6) || s == msModWheel || s == msRndGlobal || (s >= msMacro1 && s <= msMacro8) || s == msSeq || s == msExpression || s == msBreath || s == msNone;
    }
    const Voice* newestVoice() const {
        const Voice* best = nullptr;
        for (const auto& v : voices) if (v.active && (!best || (v.gate && !best->gate) || (v.gate == best->gate && v.age > best->age))) best = &v;
        return best;
    }

    // ---- telemetry --------------------------------------------------------------------------
    void writeTelemetry() {
        Telemetry& t = *tel;
        drift::telBegin(t);
        const Voice* nv = newestVoice();
        t.newest = nv ? (int)(nv - voices) : -1;
        t.activeCount = activeVoices();
        t.quality = curQuality;
        t.bpm = (float)hostBpm; t.beat = beat; t.playing = hostPlaying ? 1 : 0;
        for (int i = 0; i < kMaxVoices && i < drift::kTelVoices; ++i) {
            const Voice& v = voices[i];
            drift::TelVoice& tv = t.v[i];
            tv.active = v.active ? 1 : 0; tv.gate = v.gate ? 1 : 0; tv.note = v.note; tv.age = (int32_t)(v.age & 0x7fffffff);
            if (!v.active) continue;
            tv.amp = v.ae.v; tv.envF = v.fe.v; tv.envM = v.me.v; tv.pitch = v.pitch;
            tv.ch = v.ch; tv.vel = v.vel; tv.press = v.pressS; tv.slide = v.slideS;
            tv.bend = chans[0].bend * 2.f + (v.ch != 0 ? v.bendS * p[pBendRange] : 0.f) + v.neTuneS;   // 5.1.2: + note-expression tuning (Studio One MPE)
            {   // 5.1.2: a channel's pressure is per-note on an MPE member channel carrying one note, otherwise whole-keyboard aftertouch
                int onCh = 0;
                for (const Voice& w : voices) if (w.active && w.gate && w.ch == v.ch) ++onCh;
                const float cp = chans[v.ch & 15].pressure;
                const bool perNoteCh = v.ch != 0 && onCh <= 1;
                const NeKey& ne = neKeys[v.ch & 15][v.note & 127];
                tv.notePress = std::max(std::max(v.polyAT, ne.expr), perNoteCh ? cp : 0.f);
                tv.chanAT = perNoteCh ? 0.f : cp;
            }
            for (int o = 0; o < 3; ++o) { for (int k = 0; k < 4; ++k) tv.t[o][k] = v.telT[o][k]; tv.level[o] = v.o[o].level; tv.fm[o] = v.o[o].fmAmt; }
            tv.cut1 = v.telCut1; tv.cut2 = v.telCut2; tv.res1 = v.res1; tv.res2 = v.res2;
            tv.shape1 = v.c1.shape; tv.shape2 = v.c2.shape; tv.morph1 = v.morph1; tv.morph2 = v.morph2;
            for (int l = 0; l < kLfos; ++l) {
                const bool g = iparam(lfoP(l, lfMode), 0, 3) == 0;
                tv.lfo[l] = v.lfoOut[l]; tv.lfoPh[l] = g ? gLfo[l].phase : v.lfo[l].phase;
                tv.lfoRate[l] = g ? gLfoRate[l] : v.lfo[l].rateHz; tv.lfoFade[l] = v.lfo[l].fade;
            }
            for (int m = 0; m < kMsegs; ++m) { tv.mseg[m] = v.ms[m].v; tv.msegT[m] = v.ms[m].time; tv.msegSeg[m] = v.ms[m].seg; tv.msegU[m] = v.ms[m].u; }
            tv.rndV = v.rndV.v;
        }
        for (int l = 0; l < kLfos; ++l) { t.lfoG[l] = gLfoLast[l]; t.lfoGPh[l] = gLfo[l].phase; t.lfoGRate[l] = gLfoRate[l]; }
        for (int m = 0; m < kMacros; ++m) { t.macro[m] = macroEff[m]; t.motionOn[m] = macroLane[m] ? 1 : 0; }
        t.scenePos = scPosS;
        { const MotionData& md = *motion[motionIdx.load(std::memory_order_acquire)]; t.motionLen = md.frames > 1 ? md.seconds : 0.f; t.motionT = (float)motT; }
        t.rndG = rndG.v; t.seqCv = seqCv; t.seqStep = seqRunning ? seqCur : -1;
        if (nv) {
            for (int s = 0; s < kModSlots; ++s) {
                const int sr = iparam(modSrcParam(s), 0, kModSrcCount - 1);
                const int via = iparam(modViaParam(s), 0, kModSrcCount - 1);
                float x = applyCurve(iparam(modCurveParam(s), 0, 7), nv->src[sr]);
                if (via != msNone) x *= nv->src[via];
                t.slotSrc[s] = nv->src[sr];
                t.slotOut[s] = sr == msNone ? 0.f : x * p[modAmtParam(s)];
            }
            for (int d = 0; d < kModDstCount; ++d) t.md[d] = isGlobalDest(d) ? gmd[d] : nv->md[d];
            for (int o = 0; o < 3; ++o) {
                const OscVoice& ov = nv->o[o];
                t.specPos[o] = ov.spPlay;
                for (int i = 0; i < drift::kTelPartials; ++i) t.partials[o][i] = ov.aTgt[i];
                int mc = ov.mCount > drift::kTelModes ? drift::kTelModes : ov.mCount;
                t.modeCount[o] = mc;
                for (int i = 0; i < mc; ++i) {
                    const float r = std::sqrt(ov.mA2[i] > 0.f ? ov.mA2[i] : 0.f);
                    const float cw = r > 1e-6f ? ov.mA1[i] / (2.f * r) : 1.f;
                    const float sw2 = 1.f - cw * cw;
                    const float y1 = ov.mY1[i], y2 = ov.mY2[i] / (r > 1e-6f ? r : 1.f);
                    const float a2 = (y1 * y1 + y2 * y2 - 2.f * y1 * y2 * cw) / (sw2 > 1e-6f ? sw2 : 1e-6f);
                    t.modes[o][i] = std::sqrt(a2 > 0.f ? a2 : 0.f);
                    t.modeRatio[o][i] = ov.mRatio[i];
                }
                for (int g = 0; g < drift::kTelGrains; ++g) {
                    const Grain& gr = ov.gr[g];
                    if (!gr.on) { t.grainAmp[o][g] = 0.f; continue; }
                    const SampleBuf* sb = sslots[o].active;
                    float pos;
                    if (ov.grSource != 0 && sb) pos = gr.pos / (float)(sb->len > 2 ? sb->len - 2 : 1);
                    else { const WaveTable& T = slots[o].active ? *slots[o].active : *builtin->banks[oiparam(o, ofTable, 0, kWtBanks - 1)]; pos = T.frames > 1 ? (float)gr.frame / (float)(T.frames - 1) : 0.f; }
                    t.grainPos[o][g] = pos;
                    t.grainAmp[o][g] = std::sin(kPiF * clampf(gr.age, 0.f, 1.f));
                }
            }
        } else {
            for (int s = 0; s < kModSlots; ++s) { t.slotOut[s] = 0.f; t.slotSrc[s] = 0.f; }
            for (int d = 0; d < kModDstCount; ++d) t.md[d] = isGlobalDest(d) ? gmd[d] : 0.f;
            for (int o = 0; o < 3; ++o) { t.modeCount[o] = 0; for (int g = 0; g < drift::kTelGrains; ++g) t.grainAmp[o][g] = 0.f; }
        }
        t.peakL = peakL; t.peakR = peakR; peakL *= 0.0f; peakR *= 0.0f;
        for (int k = 0; k < 6; ++k) { t.mixPk[k] = mixPk[k]; mixPk[k] = 0.f; }
        drift::telEnd(t);
    }
};

} // namespace DRIFT_NS
