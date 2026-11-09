// Driftfield 3 — 87-type multimode filter (per voice, per channel).
#pragma once
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>

// The engine builds (generic / AVX2) set DRIFT_DSP_NS so their copies of this code never merge.
#ifndef DRIFT_DSP_NS
#define DRIFT_DSP_NS drift
#endif
namespace DRIFT_DSP_NS {

enum FilterType {
    ftOff,
    ftLadLP24, ftLadLP18, ftLadLP12, ftLadLP6, ftLadHP24, ftLadHP18, ftLadHP12, ftLadHP6,
    ftLadBP24, ftLadBP12, ftLadNotch, ftDiodeLP24,
    ftSvfLP12, ftSvfHP12, ftSvfBP12, ftSvfNotch, ftSvfPeak, ftSvfLP24, ftSvfHP24, ftSvfBP24,
    ftMs20LP, ftMs20HP, ftSemLP, ftSemBP, ftSemHP, ftSemNotch, ftSemMorph,
    ftObLP12, ftObLP24, ftObBP, ftObHP,
    ftXpLP4, ftXpLP3, ftXpLP2, ftXpLP1, ftXpHP3, ftXpHP2, ftXpHP1, ftXpBP4, ftXpBP2, ftXpN2, ftXpPH3,
    ftXpHP2LP1, ftXpHP3LP1, ftXpN2LP1, ftXpPH3LP1,
    ftDualBP, ftDualNotch, ftCombPos, ftCombNeg, ftPhaser4, ftPhaser8, ftFormant, ftVowel,
    // 3.1 "Flanges" family: comb / flanger / phaser / flange-phaser with filtered feedback loops
    ftFlgFirst,
    ftFlgLast = ftFlgFirst + 31,
    kNumFilterTypes
};

static inline float fclamp(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
static inline float satTanh(float x) {
    x = fclamp(x, -3.f, 3.f);
    const float x2 = x * x;
    return x * (27.f + x2) / (27.f + 9.f * x2);
}
// tan(x) for 0 <= x < ~1.5 (Pade 7/6), accurate well past the 0.45*fs cutoff limit
static inline float fastTan(float x) {
    const float x2 = x * x;
    return x * (135135.f - x2 * (17325.f - x2 * (378.f - x2))) / (135135.f - x2 * (62370.f - x2 * (3150.f - 28.f * x2)));
}
// 2^x, |x| < 30, ~1e-4 relative error
static inline float fastExp2(float x) {
    x = fclamp(x, -30.f, 30.f);
    const float fl = std::floor(x);
    const float f = x - fl;
    const float p = 1.f + f * (0.6931472f + f * (0.2402265f + f * (0.0555041f + f * (0.0096181f + f * 0.0013334f))));
    const int32_t e = ((int32_t)fl + 127) << 23;
    float scale;
    std::memcpy(&scale, &e, sizeof(scale));
    return p * scale;
}

struct Svf {
    float ic1 = 0.f, ic2 = 0.f;
    float a1 = 0.f, a2 = 0.f, a3 = 0.f, k = 2.f;
    void reset() { ic1 = ic2 = 0.f; }
    inline void set(float g, float kk) {
        k = kk;
        a1 = 1.f / (1.f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    inline void tick(float v0, float& lo, float& bp, float& hi) {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.f * v1 - ic1;
        ic2 = 2.f * v2 - ic2;
        lo = v2; bp = v1; hi = v0 - k * v1 - v2;
    }
};

static constexpr int kCombSize = 4096;
static constexpr int kMaxPhaserStages = 8;

struct FilterCoefs {
    float g = 0.1f;                          // per-sample single cutoff
    float gBand[kMaxPhaserStages] = {};      // control-rate bands
    float gainBand[4] = { 1, 0.6f, 0.35f, 0.2f };
    float kBand = 1.f;
    float combDelay = 100.f, combFb = 0.f, combDamp = 1.f;
    float shape = 0.5f;
    // flanges family
    float lpK = 1.f, hpK = 0.f, mix = 0.5f;
    float apA[kMaxPhaserStages] = {};
};

// kind: 0 comb, 1 flanger, 2 phaser, 3 flange-phaser. stages = first-order allpass count (phasers).
struct FlangeDef { int8_t kind, sign, lp, hp, stages; };
static const FlangeDef kFlangeDefs[32] = {
    {0, 1,0,0,0}, {0,-1,0,0,0}, {0, 1,1,0,0}, {0,-1,1,0,0}, {0, 1,0,1,0}, {0,-1,0,1,0}, {0, 1,1,1,0}, {0,-1,1,1,0},
    {1, 1,0,0,0}, {1,-1,0,0,0}, {1, 1,1,0,0}, {1,-1,1,0,0}, {1, 1,0,1,0}, {1,-1,0,1,0}, {1, 1,1,1,0}, {1,-1,1,1,0},
    {2, 1,0,0,2}, {2,-1,0,0,2}, {2, 1,0,0,4}, {2,-1,0,0,4}, {2, 1,0,0,6}, {2,-1,0,0,6}, {2, 1,0,0,8}, {2,-1,0,0,8},
    {2, 1,1,0,8}, {2,-1,1,0,8}, {2, 1,0,1,8}, {2,-1,0,1,8}, {2, 1,1,1,8}, {2,-1,1,1,8},
    {3, 1,1,1,2}, {3,-1,1,1,2} };

static inline bool filterNeedsControlCoefs(int t) { return t >= ftDualBP; }

struct MultiFilter {
    float s[4] = { 0, 0, 0, 0 };
    Svf   svf[kMaxPhaserStages];
    float phFb = 0.f, combLp = 0.f, obFb = 0.f;
    float lpS = 0.f, hpS = 0.f, apX[kMaxPhaserStages] = {}, apY[kMaxPhaserStages] = {};
    float comb[kCombSize];
    int   combPos = 0;

    MultiFilter() { reset(); }
    void reset() {
        std::memset(s, 0, sizeof(s));
        for (auto& f : svf) f.reset();
        phFb = 0.f; combLp = 0.f; obFb = 0.f; lpS = 0.f; hpS = 0.f;
        for (int i = 0; i < kMaxPhaserStages; ++i) { apX[i] = 0.f; apY[i] = 0.f; }
        std::memset(comb, 0, sizeof(comb));
        combPos = 0;
    }

    inline void ladder(float x, float g, float k, float* y) {
        const float a  = 1.f / (1.f + g);
        const float G  = g * a;
        const float G2 = G * G, G3 = G2 * G, G4 = G2 * G2;
        const float sigma = (G3 * s[0] + G2 * s[1] + G * s[2] + s[3]) * a;
        float u = (x * (1.f + 0.5f * k) - k * sigma) / (1.f + k * G4);
        u = satTanh(u);
        y[0] = u;
        float in = u;
        for (int i = 0; i < 4; ++i) {
            const float v = (in - s[i]) * G;
            const float o = v + s[i];
            s[i] = o + v;
            y[i + 1] = o;
            in = o;
        }
    }

    inline float diode(float x, float g, float k) {
        const float G = g / (1.f + g);
        float in = satTanh(x * (1.f + 0.4f * k) - k * satTanh(s[3] * 1.1f));
        for (int i = 0; i < 4; ++i) {
            const float v = (satTanh(in) - s[i]) * G;
            const float o = v + s[i];
            s[i] = o + v;
            in = o;
        }
        return in;
    }

    inline float process(int type, float x, float res, const FilterCoefs& c) {
        float lo, bp, hi;
        switch (type) {
        case ftOff: return x;
        case ftLadLP24: case ftLadLP18: case ftLadLP12: case ftLadLP6: case ftLadHP24: case ftLadHP18:
        case ftLadHP12: case ftLadHP6: case ftLadBP24: case ftLadBP12: case ftLadNotch: {
            const float k = res * 3.85f;
            float y[5];
            ladder(x, c.g, type <= ftLadLP6 ? k : k * 0.92f, y);
            switch (type) {
            case ftLadLP24: return y[4];
            case ftLadLP18: return y[3];
            case ftLadLP12: return y[2];
            case ftLadLP6:  return y[1];
            case ftLadHP24: return y[0] - 4.f * y[1] + 6.f * y[2] - 4.f * y[3] + y[4];
            case ftLadHP18: return y[0] - 3.f * y[1] + 3.f * y[2] - y[3];
            case ftLadHP12: return y[0] - 2.f * y[1] + y[2];
            case ftLadHP6:  return y[0] - y[1];
            case ftLadBP24: return 2.5f * (y[2] - 2.f * y[3] + y[4]);
            case ftLadBP12: return 2.f * (y[1] - y[2]);
            default:        return y[0] - 2.f * y[1] + 2.f * y[2];
            }
        }
        case ftDiodeLP24: return diode(x, c.g, res * 3.6f) * 1.3f;
        case ftSvfLP12: case ftSvfHP12: case ftSvfBP12: case ftSvfNotch: case ftSvfPeak: {
            const float k = 2.f - 1.96f * res;
            svf[0].set(c.g, k);
            svf[0].tick(x, lo, bp, hi);
            switch (type) {
            case ftSvfLP12: return lo;
            case ftSvfHP12: return hi;
            case ftSvfBP12: return bp * k;
            case ftSvfNotch: return lo + hi;
            default: return lo - hi;
            }
        }
        case ftSvfLP24: case ftSvfHP24: case ftSvfBP24: {
            const float k = 2.f - 1.9f * res, k2 = 1.414f;
            svf[0].set(c.g, k); svf[1].set(c.g, k2);
            svf[0].tick(x, lo, bp, hi);
            const float m1 = type == ftSvfLP24 ? lo : (type == ftSvfHP24 ? hi : bp * k);
            svf[1].tick(m1, lo, bp, hi);
            return type == ftSvfLP24 ? lo : (type == ftSvfHP24 ? hi : bp * k2);
        }
        case ftMs20LP: case ftMs20HP: {
            const float k = 2.f - 1.99f * res;
            svf[0].set(c.g, k);
            svf[0].tick(satTanh(x * 1.2f), lo, bp, hi);
            svf[0].ic1 = satTanh(svf[0].ic1 * 0.8f) * 1.25f;
            return type == ftMs20LP ? lo : hi;
        }
        case ftSemLP: case ftSemBP: case ftSemHP: case ftSemNotch: case ftSemMorph: {
            // Oberheim SEM character: gentle 12 dB, soft input stage, resonance stops short of self-oscillation
            const float k = 2.f - 1.82f * res;
            svf[0].set(c.g, k);
            svf[0].tick(satTanh(x * 0.9f) * 1.11f, lo, bp, hi);
            svf[0].ic2 = satTanh(svf[0].ic2 * 0.6f) * 1.6667f;
            switch (type) {
            case ftSemLP: return lo;
            case ftSemBP: return bp * k * 1.2f;
            case ftSemHP: return hi;
            case ftSemNotch: return lo + hi;
            default: {
                const float m = c.shape;   // 0 LP -> 0.5 notch -> 1 HP
                return m < 0.5f ? lo + hi * (2.f * m) : lo * (2.f - 2.f * m) + hi;
            }
            }
        }
        case ftObLP12: case ftObLP24: case ftObBP: case ftObHP: {
            // CEM3320-style OTA cascade: saturation in every stage, softer and rounder than the ladder
            const float G = c.g / (1.f + c.g);
            const bool four = type == ftObLP24;
            const float k = res * (four ? 3.6f : 1.85f);
            float in = satTanh((x - k * obFb) * 0.8f) * 1.25f;
            float y[4];
            for (int i = 0; i < 4; ++i) {
                const float v = (satTanh(in) - s[i]) * G;
                const float o = v + s[i];
                s[i] = o + v;
                y[i] = o;
                in = o;
            }
            obFb = four ? y[3] : y[1];
            switch (type) {
            case ftObLP12: return y[1] * (1.f + 0.4f * res);
            case ftObLP24: return y[3] * (1.f + 0.6f * res);
            case ftObBP: return 2.2f * (y[0] - y[1]);
            default: return x - 2.f * y[0] + y[1];
            }
        }
        case ftXpLP4: case ftXpLP3: case ftXpLP2: case ftXpLP1: case ftXpHP3: case ftXpHP2: case ftXpHP1:
        case ftXpBP4: case ftXpBP2: case ftXpN2: case ftXpPH3: case ftXpHP2LP1: case ftXpHP3LP1: case ftXpN2LP1: case ftXpPH3LP1: {
            // Oberheim Xpander / Matrix-12: one 4-pole cascade, 15 responses by mixing the pole outputs
            float y[5];
            ladder(x * 0.85f, c.g, res * 3.7f, y);
            switch (type) {
            case ftXpLP4: return y[4] * 1.18f;
            case ftXpLP3: return y[3] * 1.18f;
            case ftXpLP2: return y[2] * 1.18f;
            case ftXpLP1: return y[1] * 1.18f;
            case ftXpHP3: return y[0] - 3.f * y[1] + 3.f * y[2] - y[3];
            case ftXpHP2: return y[0] - 2.f * y[1] + y[2];
            case ftXpHP1: return y[0] - y[1];
            case ftXpBP4: return 4.f * (y[2] - 2.f * y[3] + y[4]);
            case ftXpBP2: return 2.f * (y[1] - y[2]);
            case ftXpN2:  return y[0] - 2.f * y[1] + 2.f * y[2];
            case ftXpPH3: return y[0] - 6.f * y[1] + 12.f * y[2] - 8.f * y[3];
            case ftXpHP2LP1: return 3.f * (y[1] - 2.f * y[2] + y[3]);
            case ftXpHP3LP1: return 3.f * (y[1] - 3.f * y[2] + 3.f * y[3] - y[4]);
            case ftXpN2LP1: return y[1] - 2.f * y[2] + 2.f * y[3];
            default: return y[1] - 6.f * y[2] + 12.f * y[3] - 8.f * y[4];
            }
        }
        case ftDualBP: case ftDualNotch: {
            if (type == ftDualBP) {
                float o = 0.f;
                for (int i = 0; i < 2; ++i) { svf[i].set(c.gBand[i], c.kBand); svf[i].tick(x, lo, bp, hi); o += bp * c.kBand; }
                return o * 0.9f;
            }
            float y = x;
            for (int i = 0; i < 2; ++i) { svf[i].set(c.gBand[i], c.kBand); svf[i].tick(y, lo, bp, hi); y = lo + hi; }
            return y;
        }
        case ftCombPos: case ftCombNeg: {
            float rp = (float)combPos - c.combDelay;
            if (rp < 0.f) rp += (float)kCombSize;
            const int i0 = (int)rp; const float f = rp - (float)i0;
            const float d = comb[i0 & (kCombSize - 1)] * (1.f - f) + comb[(i0 + 1) & (kCombSize - 1)] * f;
            const float fb = type == ftCombPos ? c.combFb : -c.combFb;
            combLp += (d - combLp) * c.combDamp;
            const float w = satTanh(x + combLp * fb);
            comb[combPos] = w;
            combPos = (combPos + 1) & (kCombSize - 1);
            return (w + d) * 0.5f * (1.f - std::fabs(c.combFb) * 0.4f);
        }
        case ftPhaser4: case ftPhaser8: {
            const int n = type == ftPhaser4 ? 4 : 8;
            float y = x + phFb * res * 0.85f;
            for (int i = 0; i < n; ++i) {
                svf[i].set(c.gBand[i], 1.2f);
                svf[i].tick(y, lo, bp, hi);
                y = lo + hi - svf[i].k * bp;
            }
            phFb = satTanh(y);
            return (x + y) * 0.5f;
        }
        case ftVowel: {
            float o = 0.f;
            for (int i = 0; i < 4; ++i) { svf[i].set(c.gBand[i], c.kBand); svf[i].tick(x, lo, bp, hi); o += bp * c.kBand * c.gainBand[i]; }
            return o * 2.6f;
        }
        default:
            if (type >= ftFlgFirst && type <= ftFlgLast) return flanges(kFlangeDefs[type - ftFlgFirst], x, c);
            break;
        case ftFormant: break;
        }
        {   // formant
            float o = 0.f;
            for (int i = 0; i < 3; ++i) { svf[i].set(c.gBand[i], c.kBand); svf[i].tick(x, lo, bp, hi); o += bp * c.kBand * c.gainBand[i]; }
            return o * 2.2f;
        }
    }

    inline float loopFilter(const FlangeDef& d, float v, const FilterCoefs& c) {
        if (d.lp) { lpS += (v - lpS) * c.lpK; v = lpS; }
        if (d.hp) { hpS += (v - hpS) * c.hpK; v -= hpS; }
        return v;
    }
    inline float allpass(int i, float v, float a) {
        const float y = a * v + apX[i] - a * apY[i];
        apX[i] = v; apY[i] = y;
        return y;
    }
    inline float readDelay(float dl) const {
        float rp = (float)combPos - dl;
        if (rp < 0.f) rp += (float)kCombSize;
        const int i0 = (int)rp; const float f = rp - (float)i0;
        return comb[i0 & (kCombSize - 1)] * (1.f - f) + comb[(i0 + 1) & (kCombSize - 1)] * f;
    }
    inline float flanges(const FlangeDef& d, float x, const FilterCoefs& c) {
        const float fb = c.combFb * (float)d.sign;
        switch (d.kind) {
        case 0: {   // comb: resonant peaks at multiples of the cutoff
            const float dl = readDelay(c.combDelay);
            const float w = satTanh(x + loopFilter(d, dl, c) * fb);
            comb[combPos] = w; combPos = (combPos + 1) & (kCombSize - 1);
            const float wet = w * (1.f - 0.55f * c.combFb);
            return wet + (x - wet) * c.mix;
        }
        case 1: {   // flanger: dry + delayed copy (notches), feedback through the loop filter
            const float dl = readDelay(c.combDelay);
            const float w = satTanh(x + loopFilter(d, dl, c) * fb);
            comb[combPos] = w; combPos = (combPos + 1) & (kCombSize - 1);
            const float m = c.mix;
            return (x * (1.f - m) + dl * (float)d.sign * m) * (1.f - 0.35f * c.combFb);
        }
        case 2: {   // phaser: first-order allpass chain mixed with the input
            float y = x + loopFilter(d, phFb, c) * fb;
            for (int i = 0; i < d.stages; ++i) y = allpass(i, y, c.apA[i]);
            phFb = satTanh(y);
            return (x + y) * 0.5f * (1.f - 0.3f * c.combFb);
        }
        default: {  // flange-phaser: allpass pair inside a short delay loop (dispersive flanging)
            float y = readDelay(c.combDelay);
            for (int i = 0; i < d.stages; ++i) y = allpass(i, y, c.apA[i]);
            const float w = satTanh(x + loopFilter(d, y, c) * fb);
            comb[combPos] = w; combPos = (combPos + 1) & (kCombSize - 1);
            return (x + y * (float)d.sign) * 0.5f * (1.f - 0.3f * c.combFb);
        }
        }
    }
};

// Control-rate coefficients for multi-band / comb types. fc in Hz.
static inline void filterControlCoefs(int type, float fc, float res, float shape, float fs, FilterCoefs& c) {
    const float nyq = 0.45f * fs;
    auto gOf = [&](float f) { return std::tan(3.14159265f * fclamp(f, 16.f, nyq) / fs); };
    c.shape = shape;
    switch (type) {
    case ftDualBP: case ftDualNotch: {
        const float sp = std::exp2(0.25f + 1.75f * shape) * 0.5f;   // half the band spacing, in octaves
        c.gBand[0] = gOf(fc * std::exp2(-sp)); c.gBand[1] = gOf(fc * std::exp2(sp));
        c.kBand = type == ftDualBP ? 2.f - 1.9f * res : 1.6f - 1.4f * res;
        break;
    }
    case ftCombPos: case ftCombNeg:
        c.combDelay = fclamp(fs / fclamp(fc, 16.f, nyq), 2.f, (float)kCombSize - 3.f);
        c.combFb = res * 0.97f;
        c.combDamp = 1.f - 0.92f * shape;     // shape = damping in the feedback path
        break;
    case ftPhaser4: case ftPhaser8: {
        const float spread = 1.15f + shape * 0.8f;
        float m = 0.5f;
        for (int i = 0; i < kMaxPhaserStages; ++i) { c.gBand[i] = gOf(fc * m); m *= spread; }
        break;
    }
    case ftFormant: {
        static const float V[5][3] = { { 730, 1090, 2440 }, { 530, 1840, 2480 }, { 270, 2290, 3010 },
                                       { 570, 840, 2410 }, { 300, 870, 2240 } };
        float x = (std::log2(fclamp(fc, 80.f, 8000.f)) - std::log2(80.f)) / (std::log2(8000.f) - std::log2(80.f)) * 4.f
                + (shape - 0.5f) * 2.f;
        x = fclamp(x, 0.f, 3.999f);
        const int i = (int)x; const float fr = x - (float)i;
        for (int b = 0; b < 3; ++b) c.gBand[b] = gOf(V[i][b] + (V[i + 1][b] - V[i][b]) * fr);
        c.gainBand[0] = 1.f; c.gainBand[1] = 0.6f; c.gainBand[2] = 0.35f;
        c.kBand = std::max(0.03f, 0.25f - 0.2f * res);
        break;
    }
    case ftVowel: {
        // A E I O U (adult voice, 4 formants); shape picks the vowel, cutoff shifts the vocal tract size
        static const float F[5][4] = { { 800, 1150, 2900, 3900 }, { 400, 1600, 2700, 3300 }, { 350, 1700, 2700, 3700 },
                                       { 450, 800, 2830, 3800 }, { 325, 700, 2530, 3500 } };
        static const float G[5][4] = { { 1.f, 0.63f, 0.1f, 0.05f }, { 1.f, 0.25f, 0.18f, 0.1f }, { 1.f, 0.1f, 0.18f, 0.12f },
                                       { 1.f, 0.35f, 0.05f, 0.03f }, { 1.f, 0.25f, 0.03f, 0.02f } };
        const float x = fclamp(shape, 0.f, 1.f) * 3.999f;
        const int i = (int)x; const float fr = x - (float)i;
        const float scale = fclamp(fc / 1000.f, 0.5f, 2.f);
        for (int b = 0; b < 4; ++b) {
            c.gBand[b] = gOf((F[i][b] + (F[i + 1][b] - F[i][b]) * fr) * scale);
            c.gainBand[b] = G[i][b] + (G[i + 1][b] - G[i][b]) * fr;
        }
        c.kBand = std::max(0.02f, 0.2f - 0.17f * res);
        break;
    }
    default:
        if (type >= ftFlgFirst && type <= ftFlgLast) {
            const FlangeDef& d = kFlangeDefs[type - ftFlgFirst];
            const float f0 = fclamp(fc, 16.f, nyq);
            const float onePole = 6.2831853f / fs;
            c.combDelay = fclamp(fs / f0, 2.f, (float)kCombSize - 3.f);
            c.combFb = res * 0.96f;
            const bool filtered = d.lp || d.hp;
            // shape: dry/wet on the plain comb and flanger, loop-filter tone on the filtered ones,
            // stage spread on the plain phasers
            c.mix = d.kind == 0 ? (filtered ? 0.f : shape * 0.6f) : (d.kind == 1 ? (filtered ? 0.5f : 0.15f + 0.7f * shape) : 0.5f);
            const float lpF = fclamp(f0 * std::exp2(filtered ? 1.f + 5.f * shape : 6.f), 40.f, nyq);
            const float hpF = fclamp(f0 * std::exp2(-3.f + 4.f * (filtered ? shape : 0.f)), 8.f, nyq);
            c.lpK = 1.f - std::exp(-onePole * lpF);
            c.hpK = 1.f - std::exp(-onePole * hpF);
            const float spread = (d.kind == 2 && !filtered) ? std::exp2((shape - 0.5f) * 0.8f) : 1.f;
            const float centre = d.kind == 3 ? f0 * 2.f : f0;
            for (int i = 0; i < kMaxPhaserStages; ++i) {
                const float fi = fclamp(centre * std::pow(spread, (float)i - 0.5f * (float)(d.stages - 1)), 16.f, nyq);
                const float t = std::tan(3.14159265f * fi / fs);
                c.apA[i] = (t - 1.f) / (t + 1.f);
            }
        }
        break;
    }
}

// Magnitude response (dB) at n log-spaced points 20 Hz..20 kHz, measured from the impulse response.
static inline void filterResponse(int type, float fc, float res, float shape, float* outDb, int n, float fs = 48000.f) {
    static MultiFilter f;
    f.reset();
    FilterCoefs c;
    c.g = std::tan(3.14159265f * fclamp(fc, 16.f, 0.45f * fs) / fs);
    c.shape = shape;
    filterControlCoefs(type, fc, res, shape, fs, c);
    const int len = 4096;
    static float ir[4096];
    const float amp = 0.01f;
    for (int i = 0; i < len; ++i) ir[i] = f.process(type, i == 0 ? amp : 0.f, res, c) / amp;
    for (int p = 0; p < n; ++p) {
        const double freq = 20.0 * std::pow(1000.0, (double)p / (n - 1));
        const double w = 2.0 * 3.14159265358979 * freq / fs;
        double re = 0, im = 0;
        for (int i = 0; i < len; ++i) { re += ir[i] * std::cos(w * i); im -= ir[i] * std::sin(w * i); }
        outDb[p] = (float)(10.0 * std::log10(re * re + im * im + 1e-12));
    }
}

} // namespace DRIFT_DSP_NS
