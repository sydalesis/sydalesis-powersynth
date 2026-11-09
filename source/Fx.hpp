// Driftfield 4 — global stereo effects. Eleven units, any order:
// drive, phaser, chorus/ensemble/solina/flanger, delay, frequency shifter, tape, crusher, width, EQ,
// 4.2: resonator (8 sympathetic strings tuned to the scale or the held notes), space (FDN reverb).
// 5.0: spectral (STFT freeze, blur / smear, pitch shift, tilt, stereo spread, spectral gate).
// Included inside the engine namespace (compiled once per ISA build). Buffers use new[]/delete[].
#pragma once

struct Biquad {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1L = 0, z2L = 0, z1R = 0, z2R = 0;
    void reset() { z1L = z2L = z1R = z2R = 0.f; }
    inline void tick(float& l, float& r) {
        const float yl = b0 * l + z1L; z1L = b1 * l - a1 * yl + z2L; z2L = b2 * l - a2 * yl; l = yl;
        const float yr = b0 * r + z1R; z1R = b1 * r - a1 * yr + z2R; z2R = b2 * r - a2 * yr; r = yr;
    }
    // RBJ cookbook: type 0 low shelf, 1 peak, 2 high shelf
    void set(int type, float f, float gainDb, float q, float fs) {
        const float A = std::pow(10.f, gainDb / 40.f);
        const float w = 6.2831853f * fclamp(f, 10.f, 0.45f * fs) / fs;
        const float cw = std::cos(w), sw = std::sin(w);
        float B0, B1, B2, A0, A1, A2;
        if (type == 1) {
            const float al = sw / (2.f * q);
            B0 = 1 + al * A; B1 = -2 * cw; B2 = 1 - al * A; A0 = 1 + al / A; A1 = -2 * cw; A2 = 1 - al / A;
        } else {
            const float al = sw / 2.f * std::sqrt(2.f);   // S = 1
            const float sA = 2.f * std::sqrt(A) * al;
            if (type == 0) {
                B0 = A * ((A + 1) - (A - 1) * cw + sA); B1 = 2 * A * ((A - 1) - (A + 1) * cw); B2 = A * ((A + 1) - (A - 1) * cw - sA);
                A0 = (A + 1) + (A - 1) * cw + sA; A1 = -2 * ((A - 1) + (A + 1) * cw); A2 = (A + 1) + (A - 1) * cw - sA;
            } else {
                B0 = A * ((A + 1) + (A - 1) * cw + sA); B1 = -2 * A * ((A - 1) + (A + 1) * cw); B2 = A * ((A + 1) + (A - 1) * cw - sA);
                A0 = (A + 1) - (A - 1) * cw + sA; A1 = 2 * ((A - 1) - (A + 1) * cw); A2 = (A + 1) - (A - 1) * cw - sA;
            }
        }
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0;
    }
};

// 90-degree phase-difference network (two 4-stage allpass chains) for the frequency shifter
struct Hilbert {
    static constexpr float ca[4] = { 0.6923878f, 0.9360654322959f, 0.9882295226860f, 0.9987488452737f };
    static constexpr float cb[4] = { 0.4021921162426f, 0.8561710882420f, 0.9722909545651f, 0.9952884791278f };
    float xa[4][2] = {}, ya[4][2] = {}, xb[4][2] = {}, yb[4][2] = {};
    float bDelay = 0.f;
    void reset() { std::memset(xa, 0, sizeof(xa)); std::memset(ya, 0, sizeof(ya)); std::memset(xb, 0, sizeof(xb)); std::memset(yb, 0, sizeof(yb)); bDelay = 0.f; }
    inline void tick(float x, float& re, float& im) {
        float a = x, b = x;
        for (int i = 0; i < 4; ++i) {
            const float y = ca[i] * (a + ya[i][1]) - xa[i][1];
            xa[i][1] = xa[i][0]; xa[i][0] = a; ya[i][1] = ya[i][0]; ya[i][0] = y; a = y;
        }
        for (int i = 0; i < 4; ++i) {
            const float y = cb[i] * (b + yb[i][1]) - xb[i][1];
            xb[i][1] = xb[i][0]; xb[i][0] = b; yb[i][1] = yb[i][0]; yb[i][0] = y; b = y;
        }
        re = a; im = bDelay; bDelay = b;
    }
};

struct FxChain {
    float fs = 48000.f;
    std::atomic<uint64_t> orderPacked { 0 };
    double bpm = 120.0;

    // drive
    float dcxL = 0.f, dcyL = 0.f, dcxR = 0.f, dcyR = 0.f, toneL = 0.f, toneR = 0.f;
    // phaser
    float apL[6] = {}, apR[6] = {};
    float phFbL = 0.f, phFbR = 0.f, phPhase = 0.f;
    // chorus / ensemble / solina / flanger
    static constexpr int kModSize = 8192;
    float* modL = nullptr; float* modR = nullptr;
    int   modPos = 0;
    float chPhase = 0.f, chPhase2 = 0.f, flFbL = 0.f, flFbR = 0.f;
    // delay
    float* dlL = nullptr; float* dlR = nullptr;
    int   dlSize = 0, dlPos = 0;
    float dlTimeS = 0.38f * 48000.f, dlLpL = 0.f, dlLpR = 0.f;
    // shifter
    Hilbert hL, hR;
    double fsPhase = 0.0;
    float fsFbL = 0.f, fsFbR = 0.f;
    // tape
    static constexpr int kTapeSize = 8192;
    float* tpL = nullptr; float* tpR = nullptr;
    int   tpPos = 0;
    float wowPh = 0.f, flPh = 0.f, wowRnd = 0.f, wowRndT = 0.f, tpLpL = 0.f, tpLpR = 0.f;
    uint32_t tpSeed = 0x1234567u;
    // crusher
    float crHoldL = 0.f, crHoldR = 0.f, crCount = 0.f;
    // width
    float wdLp = 0.f;
    // eq
    Biquad eq[3];
    float eqKey[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

    // resonator
    static constexpr int kRsStrings = 8;
    int    rsSize = 0;
    float* rsBuf = nullptr;                  // kRsStrings lines of rsSize
    int    rsPos = 0;
    float  rsLP[kRsStrings] = {}, rsDly[kRsStrings] = {}, rsTgt[kRsStrings] = {};
    float  rsHz[kRsStrings] = { 65.4f, 98.f, 130.8f, 196.f, 261.6f, 392.f, 523.3f, 784.f };
    float  rsInLP = 0.f, rsDcX = 0.f, rsDcY = 0.f;
    bool   rsFresh = true;
    // space (8-line feedback delay network)
    static constexpr int kSpLines = 8;
    int    spSize = 0, spPreSize = 0;
    float* spBuf = nullptr;                  // kSpLines lines of spSize
    float* spPre = nullptr;
    int    spPos = 0, spPrePos = 0;
    float  spLP[kSpLines] = {}, spPh[4] = { 0.f, 0.25f, 0.5f, 0.75f };
    static constexpr int kSpDiff = 4, kSpDiffSize = 2048;
    float* spDf = nullptr;                   // input diffusers (allpass), kSpDiff lines
    int    spDfPos = 0;

    // spectral FX (5.0): STFT N = 4096 (8192 above 50 kHz), hop N/4
    SpecFFT sxFft;
    int    sxN = 0, sxH = 0, sxB = 0, sxPos = 0, sxToNext = 0;
    std::vector<float> sxInL, sxInR, sxOlaL, sxOlaR, sxWin, sxRe, sxIm;
    std::vector<float> sxPrevPh, sxPhOut, sxMagS, sxFrzMag, sxFrzW, sxRand, sxMag, sxW, sxOm, sxOw;   // [2][bins] where doubled
    bool   sxWasFrozen = false, sxFresh = true;
    int    sxFrzWait = 0;
    bool   sxFrzLatch = false;

    FxChain() { setOrder(kFxDefaultOrder); }
    ~FxChain() { delete[] dlL; delete[] dlR; delete[] modL; delete[] modR; delete[] tpL; delete[] tpR; delete[] rsBuf; delete[] spBuf; delete[] spPre; delete[] spDf; }

    void setSampleRate(float sr) {
        fs = sr;
        delete[] dlL; delete[] dlR;
        dlSize = (int)(sr * 2.1f) + 4;
        dlL = new float[dlSize]; dlR = new float[dlSize];
        if (!modL) { modL = new float[kModSize]; modR = new float[kModSize]; }
        if (!tpL) { tpL = new float[kTapeSize]; tpR = new float[kTapeSize]; }
        for (auto& k : eqKey) k = -1.f;
        delete[] rsBuf; rsSize = 1; while (rsSize < (int)(sr / 25.f) + 8) rsSize <<= 1;
        rsBuf = new float[(size_t)rsSize * kRsStrings];
        delete[] spBuf; spSize = 1; while (spSize < (int)(sr * 0.2f) + 64) spSize <<= 1;
        spBuf = new float[(size_t)spSize * kSpLines];
        delete[] spPre; spPreSize = (int)(sr * 0.26f) + 8; spPre = new float[(size_t)spPreSize * 2];
        sxN = sr > 50000.f ? 8192 : 4096; sxH = sxN / 4; sxB = sxN / 2 + 1;
        sxFft.init(sxN);
        for (auto* v : { &sxInL, &sxInR, &sxOlaL, &sxOlaR, &sxRe, &sxIm, &sxWin }) v->assign((size_t)sxN, 0.f);
        for (int i = 0; i < sxN; ++i) sxWin[(size_t)i] = 0.5f - 0.5f * (float)std::cos(6.283185307179586 * i / sxN);
        for (auto* v : { &sxPrevPh, &sxPhOut, &sxMagS, &sxFrzMag, &sxFrzW, &sxMag, &sxW }) v->assign((size_t)sxB * 2, 0.f);
        sxRand.assign((size_t)sxB, 0.f); sxOm.assign((size_t)sxB, 0.f); sxOw.assign((size_t)sxB, 0.f);
        { uint32_t r = 0x51ED27u; for (auto& x : sxRand) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; x = ((float)(r >> 8) / 16777216.f) * 6.2831853f; } }
        if (!spDf) spDf = new float[(size_t)kSpDiffSize * kSpDiff * 2];
        reset();
    }
    void reset() {
        dcxL = dcyL = dcxR = dcyR = toneL = toneR = 0.f;
        for (int i = 0; i < 6; ++i) apL[i] = apR[i] = 0.f;
        phFbL = phFbR = 0.f;
        if (modL) for (int i = 0; i < kModSize; ++i) modL[i] = modR[i] = 0.f;
        modPos = 0; flFbL = flFbR = 0.f;
        if (dlL) for (int i = 0; i < dlSize; ++i) dlL[i] = dlR[i] = 0.f;
        dlPos = 0; dlLpL = dlLpR = 0.f;
        hL.reset(); hR.reset(); fsFbL = fsFbR = 0.f;
        if (tpL) for (int i = 0; i < kTapeSize; ++i) tpL[i] = tpR[i] = 0.f;
        tpPos = 0; tpLpL = tpLpR = 0.f;
        crHoldL = crHoldR = crCount = 0.f;
        wdLp = 0.f;
        for (auto& b : eq) b.reset();
        if (rsBuf) std::memset(rsBuf, 0, sizeof(float) * (size_t)rsSize * kRsStrings);
        rsPos = 0; for (auto& v : rsLP) v = 0.f; rsInLP = rsDcX = rsDcY = 0.f; rsFresh = true;
        if (spBuf) std::memset(spBuf, 0, sizeof(float) * (size_t)spSize * kSpLines);
        if (spPre) std::memset(spPre, 0, sizeof(float) * (size_t)spPreSize * 2);
        if (spDf) std::memset(spDf, 0, sizeof(float) * (size_t)kSpDiffSize * kSpDiff * 2);
        spPos = spPrePos = spDfPos = 0; for (auto& v : spLP) v = 0.f;
        for (auto* v : { &sxInL, &sxInR, &sxOlaL, &sxOlaR, &sxPrevPh, &sxPhOut, &sxMagS, &sxFrzMag, &sxFrzW }) std::fill(v->begin(), v->end(), 0.f);
        sxPos = 0; sxToNext = sxH; sxWasFrozen = false; sxFresh = true; sxFrzWait = 0; sxFrzLatch = false;
    }
    // string pitches in semitones (MIDI note numbers, tuned), set once per chunk by the engine
    void setResonator(const float* semis) {
        for (int k = 0; k < kRsStrings; ++k) rsHz[k] = 440.f * fastExp2((fclamp(semis[k], 12.f, 110.f) - 69.f) * (1.f / 12.f));
    }
    void setOrder(const int* o) {
        uint64_t v = 0;
        for (int i = 0; i < kFxUnits; ++i) v |= (uint64_t)(o[i] & 15) << (4 * i);
        orderPacked.store(v, std::memory_order_release);
    }

    static inline float sinTurns(float x) { return fastSinTurns(x); }
    inline float readMod(const float* b, float d) const {
        float rp = (float)modPos - d;
        if (rp < 0.f) rp += (float)kModSize;
        const int i0 = (int)rp; const float f = rp - (float)i0;
        return b[i0 & (kModSize - 1)] * (1.f - f) + b[(i0 + 1) & (kModSize - 1)] * f;
    }
    inline float readDl(const float* b, float d) const {
        float rp = (float)dlPos - d;
        while (rp < 0.f) rp += (float)dlSize;
        const int i0 = (int)rp; const float f = rp - (float)i0;
        int i1 = i0 + 1; if (i1 >= dlSize) i1 -= dlSize;
        return b[i0] * (1.f - f) + b[i1] * f;
    }
    inline float readTape(const float* b, float d) const {
        float rp = (float)tpPos - d;
        if (rp < 0.f) rp += (float)kTapeSize;
        const int i0 = (int)rp; const float f = rp - (float)i0;
        // cubic (Hermite) for a smoother wow
        const float y0 = b[(i0 - 1) & (kTapeSize - 1)], y1 = b[i0 & (kTapeSize - 1)], y2 = b[(i0 + 1) & (kTapeSize - 1)], y3 = b[(i0 + 2) & (kTapeSize - 1)];
        const float c1 = 0.5f * (y2 - y0), c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3, c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + y1;
    }

    // gm: global modulation offsets per destination (FX destinations only are read)
    void process(float* L, float* R, int n, const float* p, const float* gm) {
        const uint64_t ord = orderPacked.load(std::memory_order_acquire);
        bool delaySeen = false;
        for (int slot = 0; slot < kFxUnits; ++slot) {
            const int u = (int)((ord >> (4 * slot)) & 15);
            switch (u) {
            case fxDrive:  if (p[pDrvOn] >= 0.5f) drive(L, R, n, p, gm); break;
            case fxPhaser: if (p[pPhOn] >= 0.5f) phaser(L, R, n, p, gm); break;
            case fxChorus: if (p[pChOn] >= 0.5f) chorus(L, R, n, p, gm); break;
            case fxDelay:  delay(L, R, n, p, gm, p[pDlOn] >= 0.5f); delaySeen = true; break;
            case fxShift:  if (p[pFsOn] >= 0.5f) shifter(L, R, n, p, gm); break;
            case fxTape:   if (p[pTpOn] >= 0.5f) tape(L, R, n, p, gm); break;
            case fxCrush:  if (p[pCrOn] >= 0.5f) crusher(L, R, n, p, gm); break;
            case fxWidth:  if (p[pWdOn] >= 0.5f) width(L, R, n, p, gm); break;
            case fxEq:     if (p[pEqOn] >= 0.5f) eqProc(L, R, n, p); break;
            case fxReso:   if (p[pRsOn] >= 0.5f) resonator(L, R, n, p, gm); else rsFresh = true; break;
            case fxSpace:  if (p[pSpOn] >= 0.5f) space(L, R, n, p, gm); break;
            case fxSpectral: if (p[pSxOn] >= 0.5f) spectral(L, R, n, p, gm); else sxFresh = true; break;
            default: break;
            }
        }
        (void)delaySeen;
    }

    void drive(float* L, float* R, int n, const float* p, const float* gm) {
        const float drv = fclamp(p[pDrvAmt] + gm[mdFxDrive], 0.f, 1.f);
        const float g = 1.f + drv * 10.f, comp = 1.f / (1.f + drv * 2.6f);
        const float dc = 1.f - 30.f / fs;
        const float tone = p[pDrvTone];
        const float lpc = 1.f - std::exp(-6.2831853f * (800.f * fastExp2(tone * 4.5f)) / fs);
        for (int i = 0; i < n; ++i) {
            const float xl = satTanh(L[i] * g + 0.08f * drv) * comp * 1.6f;
            const float xr = satTanh(R[i] * g + 0.08f * drv) * comp * 1.6f;
            dcyL = xl - dcxL + dc * dcyL; dcxL = xl;
            dcyR = xr - dcxR + dc * dcyR; dcxR = xr;
            toneL += (dcyL - toneL) * lpc; toneR += (dcyR - toneR) * lpc;
            L[i] = toneL; R[i] = toneR;
        }
    }

    void phaser(float* L, float* R, int n, const float* p, const float* gm) {
        const float phMix = fclamp(p[pPhMix] + gm[mdFxPhaser], 0.f, 1.f);
        if (phMix < 0.001f) return;
        const float inc = p[pPhRate] / fs, depth = p[pPhDepth], fb = p[pPhFb];
        for (int i = 0; i < n; ++i) {
            const float m1 = 0.5f + 0.5f * sinTurns(phPhase), m2 = 0.5f + 0.5f * sinTurns(phPhase + 0.25f);
            phPhase += inc; if (phPhase >= 1.f) phPhase -= 1.f;
            for (int ch = 0; ch < 2; ++ch) {
                const float m = ch ? m2 : m1;
                const float f = 160.f * fastExp2(depth * 5.5f * m);
                const float t = fastTan(3.14159265f * fclamp(f, 20.f, 0.45f * fs) / fs);
                const float a = (1.f - t) / (1.f + t);
                float* ap = ch ? apR : apL;
                float& fbs = ch ? phFbR : phFbL;
                const float x = ch ? R[i] : L[i];
                float y = x + fbs * fb;
                for (int s = 0; s < 6; ++s) { const float out = a * y + ap[s]; ap[s] = y - a * out; y = out; }
                fbs = satTanh(y);
                const float o = x * (1.f - 0.5f * phMix) + y * 0.5f * phMix * 1.4f;
                if (ch) R[i] = o; else L[i] = o;
            }
        }
    }

    void chorus(float* L, float* R, int n, const float* p, const float* gm) {
        const float chMix = fclamp(p[pChMix] + gm[mdFxChorus], 0.f, 1.f);
        const int mode = (int)(p[pChMode] + 0.5f);
        const float rate = p[pChRate], depth = p[pChDepth], fb = p[pChFb];
        const float inc = rate / fs;
        for (int i = 0; i < n; ++i) {
            const float inL = L[i] + (mode == 3 ? flFbL * fb : 0.f);
            const float inR = R[i] + (mode == 3 ? flFbR * fb : 0.f);
            modL[modPos] = inL; modR[modPos] = inR;
            float wl = 0.f, wr = 0.f;
            if (chMix > 0.001f) {
                if (mode == 0) {
                    const float base = 0.007f * fs, dep = (0.0005f + 0.0045f * depth) * fs;
                    wl = readMod(modL, base + dep * 0.5f * (1.f + sinTurns(chPhase)));
                    wr = readMod(modR, base + dep * 0.5f * (1.f + sinTurns(chPhase + 0.25f)));
                } else if (mode == 1) {
                    const float base = 0.009f * fs, dep = (0.0008f + 0.003f * depth) * fs, fast = 0.00025f * fs;
                    for (int t = 0; t < 3; ++t) {
                        const float ph = chPhase + t / 3.f, ph2 = chPhase2 + t / 3.f;
                        wl += readMod(modL, base + dep * 0.5f * (1.f + sinTurns(ph)) + fast * sinTurns(ph2));
                        wr += readMod(modR, base + dep * 0.5f * (1.f + sinTurns(ph + 0.1667f)) + fast * sinTurns(ph2 + 0.5f));
                    }
                    wl *= 0.45f; wr *= 0.45f;
                } else if (mode == 2) {
                    // Solina-style string ensemble: three lines, each swept by a slow chorus LFO plus a fast vibrato
                    // LFO, the two offset by 120 degrees between lines. Mono-summed source spread to stereo.
                    const float base = 0.006f * fs, slow = (0.0015f + 0.004f * depth) * fs, fast = (0.0002f + 0.0005f * depth) * fs;
                    const float tapL[3] = { 1.f, 0.35f, 0.7f }, tapR[3] = { 0.35f, 1.f, 0.7f };
                    for (int t = 0; t < 3; ++t) {
                        const float d = base + slow * 0.5f * (1.f + sinTurns(chPhase + t / 3.f)) + fast * 0.5f * (1.f + sinTurns(chPhase2 + t / 3.f));
                        const float y = readMod(modL, d) + readMod(modR, d);
                        wl += y * tapL[t]; wr += y * tapR[t];
                    }
                    wl *= 0.42f; wr *= 0.42f;
                } else {
                    const float base = 0.0006f * fs, dep = (0.0003f + 0.0045f * depth) * fs;
                    wl = readMod(modL, base + dep * 0.5f * (1.f + sinTurns(chPhase)));
                    wr = readMod(modR, base + dep * 0.5f * (1.f + sinTurns(chPhase + 0.25f)));
                    flFbL = satTanh(wl); flFbR = satTanh(wr);
                }
                const float dryG = mode == 3 ? 1.f - 0.5f * chMix : (mode == 2 ? 1.f - 0.6f * chMix : 1.f - 0.4f * chMix);
                const float wetG = mode == 3 ? 0.5f * chMix : 0.75f * chMix;
                L[i] = L[i] * dryG + wl * wetG;
                R[i] = R[i] * dryG + wr * wetG;
            }
            modPos = (modPos + 1) & (kModSize - 1);
            chPhase += inc; if (chPhase >= 1.f) chPhase -= 1.f;
            chPhase2 += inc * (mode == 2 ? 9.5f : 7.3f); if (chPhase2 >= 1.f) chPhase2 -= 1.f;
        }
    }

    void delay(float* L, float* R, int n, const float* p, const float* gm, bool on) {
        if (!dlL) return;
        const float dlMix = fclamp(p[pDlMix] + gm[mdFxDelay], 0.f, 1.f);
        if (!on || dlMix < 0.001f) {
            for (int i = 0; i < n; ++i) { dlL[dlPos] = 0.f; dlR[dlPos] = 0.f; if (++dlPos >= dlSize) dlPos = 0; }
            return;
        }
        float secs = fclamp(p[pDlTime], 0.01f, 2.f);
        if (p[pDlSync] >= 0.5f) {
            const int d = (int)fclamp(p[pDlDiv], 0.f, (float)(kDivs - 1));
            secs = fclamp((float)(kDivBeats[d] * 60.0 / (bpm > 20.0 ? bpm : 120.0)), 0.01f, 2.f);
        }
        const float tgt = secs * fs;
        const float fb = p[pDlFb], damp = 0.05f + 0.9f * (1.f - p[pDlDamp]);
        const bool pp = p[pDlMode] >= 0.5f;
        const float sm = 1.f - std::exp(-1.f / (0.08f * fs));
        for (int i = 0; i < n; ++i) {
            dlTimeS += (tgt - dlTimeS) * sm;
            const float yl = readDl(dlL, dlTimeS), yr = readDl(dlR, dlTimeS);
            dlLpL += (yl - dlLpL) * damp; dlLpR += (yr - dlLpR) * damp;
            if (pp) {
                dlL[dlPos] = satTanh((L[i] + R[i]) * 0.5f + dlLpR * fb);
                dlR[dlPos] = satTanh(dlLpL * fb);
            } else {
                dlL[dlPos] = satTanh(L[i] + dlLpL * fb);
                dlR[dlPos] = satTanh(R[i] + dlLpR * fb);
            }
            if (++dlPos >= dlSize) dlPos = 0;
            L[i] += yl * dlMix; R[i] += yr * dlMix;
        }
    }

    void shifter(float* L, float* R, int n, const float* p, const float* gm) {
        const float mix = fclamp(p[pFsMix] + gm[mdFxShiftMix], 0.f, 1.f);
        if (mix < 0.001f) return;
        const float hz = fclamp(p[pFsShift] + gm[mdFxShift] * 200.f, -2000.f, 2000.f);
        const float fb = p[pFsFb], st = p[pFsStereo];
        const double inc = (double)hz / fs;
        for (int i = 0; i < n; ++i) {
            float re, im, c, s;
            const float phL = (float)fsPhase;
            hL.tick(L[i] + fsFbL * fb, re, im);
            c = fastSinTurns(phL + 0.25f); s = fastSinTurns(phL);
            const float yl = re * c - im * s;
            // right channel: shift direction blends toward the opposite sign with "stereo"
            hR.tick(R[i] + fsFbR * fb, re, im);
            const float phR = (float)(fsPhase * (1.0 - 2.0 * st));
            c = fastSinTurns(phR + 0.25f); s = fastSinTurns(phR);
            const float yr = re * c - im * s;
            fsFbL = satTanh(yl); fsFbR = satTanh(yr);
            L[i] = L[i] * (1.f - mix) + yl * mix * 1.15f;
            R[i] = R[i] * (1.f - mix) + yr * mix * 1.15f;
            fsPhase += inc; fsPhase -= std::floor(fsPhase);
        }
    }

    void tape(float* L, float* R, int n, const float* p, const float* gm) {
        const float mix = p[pTpMix];
        const float wow = fclamp(p[pTpWow] + gm[mdFxWow], 0.f, 1.f), flut = p[pTpFlutter], drv = p[pTpDrive];
        const float g = 1.f + drv * 4.f, comp = 1.f / (1.f + drv * 1.2f);
        const float lpc = 1.f - std::exp(-6.2831853f * (2500.f * fastExp2(p[pTpTone] * 3.f)) / fs);
        const float wowInc = fclamp(p[pTpWowRate], 0.05f, 4.f) / fs, flInc = fclamp(p[pTpFlutRate], 2.f, 30.f) / fs;   // 5.1.6: were fixed 0.45 / 7.3 Hz
        const float wowDepth = wow * 0.006f * fs, flDepth = flut * 0.0004f * fs, base = 0.008f * fs;
        for (int i = 0; i < n; ++i) {
            tpL[tpPos] = satTanh(L[i] * g) * comp; tpR[tpPos] = satTanh(R[i] * g) * comp;
            wowPh += wowInc; if (wowPh >= 1.f) { wowPh -= 1.f; }
            flPh += flInc; if (flPh >= 1.f) flPh -= 1.f;
            wowRndT -= 1.f;
            if (wowRndT <= 0.f) { tpSeed ^= tpSeed << 13; tpSeed ^= tpSeed >> 17; tpSeed ^= tpSeed << 5; wowRndT = fs * 0.7f; }
            const float rt = (float)(tpSeed >> 8) * (1.f / 16777216.f) * 2.f - 1.f;
            wowRnd += (rt - wowRnd) * (3.f / fs);
            const float d = base + wowDepth * (0.5f + 0.35f * sinTurns(wowPh) + 0.15f * wowRnd) + flDepth * (0.5f + 0.5f * sinTurns(flPh));
            float yl = readTape(tpL, d), yr = readTape(tpR, d + 0.3f);
            tpLpL += (yl - tpLpL) * lpc; tpLpR += (yr - tpLpR) * lpc;
            L[i] = L[i] * (1.f - mix) + tpLpL * mix;
            R[i] = R[i] * (1.f - mix) + tpLpR * mix;
            tpPos = (tpPos + 1) & (kTapeSize - 1);
        }
    }

    void crusher(float* L, float* R, int n, const float* p, const float* gm) {
        const float mix = fclamp(p[pCrMix] + gm[mdFxCrush], 0.f, 1.f);
        if (mix < 0.001f) return;
        const float bits = fclamp(p[pCrBits], 1.f, 16.f);
        const float q = fastExp2(bits - 1.f), iq = 1.f / q;
        const float down = fclamp(p[pCrDown], 1.f, 50.f);
        for (int i = 0; i < n; ++i) {
            crCount += 1.f;
            if (crCount >= down) {
                crCount -= down;
                crHoldL = std::floor(L[i] * q + 0.5f) * iq;
                crHoldR = std::floor(R[i] * q + 0.5f) * iq;
            }
            L[i] = L[i] * (1.f - mix) + crHoldL * mix;
            R[i] = R[i] * (1.f - mix) + crHoldR * mix;
        }
    }

    void width(float* L, float* R, int n, const float* p, const float* gm) {
        const float w = fclamp(p[pWdWidth] + gm[mdFxWidth], 0.f, 2.f);
        const float lpc = 1.f - std::exp(-6.2831853f * p[pWdBassMono] / fs);
        for (int i = 0; i < n; ++i) {
            const float m = 0.5f * (L[i] + R[i]);
            float s = 0.5f * (L[i] - R[i]);
            wdLp += (s - wdLp) * lpc;
            s = (s - wdLp) * w;               // side below the bass-mono frequency is removed
            L[i] = m + s; R[i] = m - s;
        }
    }

    void eqProc(float* L, float* R, int n, const float* p) {
        const float key[8] = { p[pEqLowG], p[pEqLowF], p[pEqMidG], p[pEqMidF], p[pEqMidQ], p[pEqHighG], p[pEqHighF], fs };
        bool changed = false;
        for (int k = 0; k < 8; ++k) if (key[k] != eqKey[k]) { changed = true; eqKey[k] = key[k]; }
        if (changed) {
            eq[0].set(0, p[pEqLowF], p[pEqLowG], 0.707f, fs);
            eq[1].set(1, p[pEqMidF], p[pEqMidG], p[pEqMidQ], fs);
            eq[2].set(2, p[pEqHighF], p[pEqHighG], 0.707f, fs);
        }
        for (int i = 0; i < n; ++i) { float l = L[i], r = R[i]; eq[0].tick(l, r); eq[1].tick(l, r); eq[2].tick(l, r); L[i] = l; R[i] = r; }
    }

    // ---- 4.2 resonator: eight damped string loops excited by the signal ----------------------------
    void resonator(float* L, float* R, int n, const float* p, const float* gm) {
        if (!rsBuf) return;
        const float mix = fclamp(p[pRsMix] + gm[mdFxReso], 0.f, 1.f);
        const float decay = fclamp(p[pRsDecay], 0.1f, 40.f);
        const float bright = fclamp(p[pRsBright], 0.f, 1.f);
        const float lpc = 0.08f + 0.92f * bright * bright;
        const float width = fclamp(p[pRsWidth], 0.f, 1.f);
        const float inC = 1.f - std::exp(-6.2831853f * (600.f + 9000.f * bright) / fs);
        float g[kRsStrings], panL[kRsStrings], panR[kRsStrings], inG[kRsStrings];
        for (int k = 0; k < kRsStrings; ++k) {
            const float hz = fclamp(rsHz[k], 25.f, fs * 0.2f);
            // the loop lowpass delays the loop by (1 - c) / c samples at low frequencies
            rsTgt[k] = fclamp(fs / hz - (1.f - lpc) / lpc, 2.f, (float)rsSize - 4.f);
            if (rsFresh) rsDly[k] = rsTgt[k];
            g[k] = std::exp(-6.9078f / (decay * hz));
            inG[k] = 0.35f * std::sqrt(1.f - g[k]);
            const float pn = (k & 1 ? 1.f : -1.f) * width * (0.35f + 0.65f * (float)(k >> 1) / 3.f);
            const float a = (pn + 1.f) * 0.25f * 3.14159265f;
            panL[k] = std::cos(a) * 1.4142f; panR[k] = std::sin(a) * 1.4142f;
        }
        rsFresh = false;
        const float dsm = 1.f - std::exp(-1.f / (0.03f * fs));   // retune glide
        const int mask = rsSize - 1;
        for (int i = 0; i < n; ++i) {
            const float xin = 0.5f * (L[i] + R[i]);
            rsInLP += (xin - rsInLP) * inC;
            rsDcY = rsInLP - rsDcX + 0.995f * rsDcY; rsDcX = rsInLP;
            const float x = rsDcY;
            float wl = 0.f, wr = 0.f;
            for (int k = 0; k < kRsStrings; ++k) {
                rsDly[k] += (rsTgt[k] - rsDly[k]) * dsm;
                float* line = rsBuf + (size_t)k * rsSize;
                float rp = (float)rsPos - rsDly[k]; if (rp < 0.f) rp += (float)rsSize;
                const int i0 = (int)rp; const float f = rp - (float)i0;
                const float y = line[i0 & mask] * (1.f - f) + line[(i0 + 1) & mask] * f;
                rsLP[k] += (y - rsLP[k]) * lpc;
                line[rsPos] = satTanh(x * inG[k] + rsLP[k] * g[k]);
                wl += y * panL[k]; wr += y * panR[k];
            }
            rsPos = (rsPos + 1) & mask;
            L[i] += wl * mix * 0.7f; R[i] += wr * mix * 0.7f;
        }
    }

    // ---- 4.2 space: pre-delay, four input diffusers, 8-line Householder FDN with modulated lines ------
    void space(float* L, float* R, int n, const float* p, const float* gm) {
        if (!spBuf) return;
        const float mix = fclamp(p[pSpMix] + gm[mdFxSpace], 0.f, 1.f);
        const float size = fclamp(p[pSpSize], 0.f, 1.f);
        const float t60 = fclamp(p[pSpDecay], 0.2f, 80.f);
        const float damp = fclamp(p[pSpDamp], 0.f, 1.f);
        const float modD = fclamp(p[pSpMod], 0.f, 1.f) * 0.0012f * fs;
        const float width = fclamp(p[pSpWidth], 0.f, 1.f);
        const int pre = (int)fclamp(p[pSpPre] * fs, 0.f, (float)spPreSize - 2.f);
        static const float base[kSpLines] = { 0.0297f, 0.0371f, 0.0411f, 0.0437f, 0.0479f, 0.0533f, 0.0593f, 0.0671f };
        static const float dfLen[kSpDiff] = { 0.0047f, 0.0036f, 0.0127f, 0.0093f };
        const float scale = 0.35f + 1.3f * size;
        float len[kSpLines], g[kSpLines];
        for (int k = 0; k < kSpLines; ++k) {
            len[k] = fclamp(base[k] * scale * fs, 16.f, (float)spSize - modD - 8.f);
            g[k] = std::pow(10.f, -3.f * len[k] / (fs * t60));
        }
        int dl[kSpDiff];
        for (int k = 0; k < kSpDiff; ++k) dl[k] = (int)fclamp(dfLen[k] * (0.5f + size) * fs, 4.f, (float)kSpDiffSize - 2.f);
        const float lpc = 1.f - std::exp(-6.2831853f * (1200.f + 16000.f * (1.f - damp) * (1.f - damp)) / fs);
        const float modInc = 0.37f / fs;
        const int mask = spSize - 1, dmask = kSpDiffSize - 1;
        const float wetG = 0.5f, wet1 = 0.5f + 0.5f * width, wet2 = 0.5f - 0.5f * width;
        for (int i = 0; i < n; ++i) {
            spPre[2 * spPrePos] = L[i]; spPre[2 * spPrePos + 1] = R[i];
            int rp = spPrePos - pre; if (rp < 0) rp += spPreSize;
            float xl = spPre[2 * rp], xr = spPre[2 * rp + 1];
            if (++spPrePos >= spPreSize) spPrePos = 0;
            // diffusion: Schroeder allpasses, g = 0.6
            for (int d = 0; d < kSpDiff; ++d) {
                float* bl = spDf + (size_t)(2 * d) * kSpDiffSize; float* br = bl + kSpDiffSize;
                const int r = (spDfPos - dl[d]) & dmask;
                const float yl = bl[r] - 0.6f * xl, yr = br[r] - 0.6f * xr;
                bl[spDfPos] = xl + 0.6f * yl; br[spDfPos] = xr + 0.6f * yr;
                xl = yl; xr = yr;
            }
            spDfPos = (spDfPos + 1) & dmask;
            float y[kSpLines], sum = 0.f;
            for (int k = 0; k < kSpLines; ++k) {
                float d = len[k];
                if (k < 4) d += modD * (0.5f + 0.5f * fastSinTurns(spPh[k]));
                float rpf = (float)spPos - d; if (rpf < 0.f) rpf += (float)spSize;
                const int i0 = (int)rpf; const float f = rpf - (float)i0;
                const float* line = spBuf + (size_t)k * spSize;
                float v = line[i0 & mask] * (1.f - f) + line[(i0 + 1) & mask] * f;
                spLP[k] += (v - spLP[k]) * lpc;
                y[k] = spLP[k] * g[k];
                sum += y[k];
            }
            for (int k = 0; k < 4; ++k) { spPh[k] += modInc * (1.f + 0.31f * k); if (spPh[k] >= 1.f) spPh[k] -= 1.f; }
            const float hh = sum * (2.f / kSpLines);
            float ol = 0.f, orr = 0.f;
            for (int k = 0; k < kSpLines; ++k) {
                const float in = (k & 1) ? xr : xl;
                spBuf[(size_t)k * spSize + spPos] = y[k] - hh + in * 0.35f;
                if (k & 1) orr += y[k]; else ol += y[k];
            }
            spPos = (spPos + 1) & mask;
            const float wl = (ol * wet1 + orr * wet2) * wetG, wr = (orr * wet1 + ol * wet2) * wetG;
            L[i] = L[i] * (1.f - 0.5f * mix) + wl * mix;
            R[i] = R[i] * (1.f - 0.5f * mix) + wr * mix;
        }
    }

    // ---- 5.0 spectral FX --------------------------------------------------------------------------
    void spectralHop(const float* p, const float* gm) {
        const int N = sxN, B = sxB, H = sxH;
        const bool freeze = p[pSxFreeze] >= 0.5f;
        const float blurT = fclamp(p[pSxBlur] + gm[mdFxSpecBlur] * 10.f, 0.f, 40.f);
        const float ratio = fastExp2(fclamp(p[pSxShift], -24.f, 24.f) / 12.f);
        const float tilt = fclamp(p[pSxTilt], -1.f, 1.f), spread = fclamp(p[pSxSpread], 0.f, 1.f), gate = fclamp(p[pSxGate], 0.f, 1.f);
        const bool transparent = !freeze && blurT < 0.01f && std::fabs(ratio - 1.f) < 1e-4f && tilt == 0.f && gate == 0.f && spread == 0.f;
        // the last N input samples, windowed, L + iR in one forward FFT
        for (int i = 0; i < N; ++i) {
            const int at = (sxPos + i) & (N - 1);   // sxPos = oldest
            sxRe[(size_t)i] = sxInL[(size_t)at] * sxWin[(size_t)i];
            sxIm[(size_t)i] = sxInR[(size_t)at] * sxWin[(size_t)i];
        }
        sxFft.run(sxRe.data(), sxIm.data(), false);
        const float binHz = fs / (float)N, dPh = 6.2831853f * (float)H / fs;
        const float bl = blurT > 0.01f ? std::exp(-((float)H / fs) / blurT) : 0.f;
        float frameMax = 0.f;
        for (int k = 0; k < B; ++k) {
            const int nk = (N - k) & (N - 1);
            const float zr = sxRe[(size_t)k], zi = sxIm[(size_t)k], cr = sxRe[(size_t)nk], ci = -sxIm[(size_t)nk];   // conj(Z[N-k])
            const float lr = 0.5f * (zr + cr), li = 0.5f * (zi + ci);            // X_L
            const float rr = 0.5f * (zi - ci), ri = -0.5f * (zr - cr);           // X_R = (Z - conj) / 2i
            for (int ch = 0; ch < 2; ++ch) {
                const float a = ch ? rr : lr, b = ch ? ri : li;
                const float m = std::sqrt(a * a + b * b), ph = std::atan2(b, a);
                float d = ph - sxPrevPh[(size_t)(ch * B + k)] - (float)k * 6.2831853f * (float)H / (float)N;
                d = wrapPi(d);
                sxPrevPh[(size_t)(ch * B + k)] = ph;
                sxMag[(size_t)(ch * B + k)] = m;
                sxW[(size_t)(ch * B + k)] = ((float)k + d * (float)N / (6.2831853f * (float)H)) * binHz;   // Hz
                frameMax = std::max(frameMax, m);
                if (transparent) sxPhOut[(size_t)(ch * B + k)] = ph;
            }
        }
        // capture on the rising edge. Armed on silence (session saved frozen, freeze before any sound): follow the
        // input until sound above -70 dBFS has been there for 0.3 s, then hold that frame.
        const float floorMag = (float)N * 0.25f * 3.2e-4f;
        if (freeze && !sxWasFrozen) sxFrzWait = frameMax < floorMag ? -1 : 0;
        if (freeze && sxFrzWait != 0) {
            if (sxFrzWait < 0 && frameMax >= floorMag) sxFrzWait = std::max(1, (int)(0.3f * fs / (float)H));
            else if (sxFrzWait > 0) --sxFrzWait;
        }
        if (freeze && (!sxWasFrozen || sxFrzWait != 0 || sxFrzLatch)) {
            for (int i = 0; i < 2 * B; ++i) { sxFrzMag[(size_t)i] = sxMag[(size_t)i]; sxFrzW[(size_t)i] = sxW[(size_t)i]; }
        }
        sxFrzLatch = freeze && sxFrzWait != 0;   // the frame in which the wait ends is captured too
        sxWasFrozen = freeze;
        const float gThr = gate > 0.f ? frameMax * std::pow(10.f, (-80.f + 70.f * gate) / 20.f) : 0.f;
        for (int k = 0; k < N; ++k) { sxRe[(size_t)k] = 0.f; sxIm[(size_t)k] = 0.f; }
        // per channel: freeze / gate / shift / tilt / blur, then propagate the phases
        for (int ch = 0; ch < 2; ++ch) {
            float* magS = sxMagS.data() + ch * B; float* phO = sxPhOut.data() + ch * B;
            const float* mi = freeze ? sxFrzMag.data() + ch * B : sxMag.data() + ch * B;
            const float* wi = freeze ? sxFrzW.data() + ch * B : sxW.data() + ch * B;
            // remap bins for the pitch shift (target bin keeps the louder source)
            float* tm = sxMag.data() + ch * B;   // reuse: target magnitudes (sxMag no longer needed for this ch)
            float* tw = sxW.data() + ch * B;
            if (!transparent) {
                std::vector<float>& om = sxOm; std::vector<float>& ow = sxOw;
                std::fill(om.begin(), om.end(), 0.f); std::fill(ow.begin(), ow.end(), 0.f);
                for (int k = 1; k < B - 1; ++k) {
                    float m = mi[k];
                    if (m < gThr) continue;
                    const float f = wi[k] * ratio;
                    const int j = (int)(f / binHz + 0.5f);
                    if (j < 1 || j >= B - 1) continue;
                    if (tilt != 0.f) m *= std::min(8.f, std::pow(std::max(20.f, f) / 1000.f, tilt * 1.5f));
                    if (m > om[(size_t)j]) { om[(size_t)j] = m; ow[(size_t)j] = f; }
                }
                for (int k = 0; k < B; ++k) { tm[k] = om[(size_t)k]; tw[k] = ow[(size_t)k] > 0.f ? ow[(size_t)k] : (float)k * binHz; }
            }
            for (int k = 1; k < B - 1; ++k) {
                float m, ph;
                if (transparent) { m = (ch ? sxMag[(size_t)(B + k)] : sxMag[(size_t)k]); ph = phO[k]; magS[k] = m; }
                else {
                    magS[k] = sxFresh ? tm[k] : magS[k] * bl + tm[k] * (1.f - bl);
                    m = magS[k];
                    phO[k] = wrapPi(phO[k] + tw[k] * dPh);
                    ph = phO[k] + (ch ? spread * sxRand[(size_t)k] : 0.f);
                }
                if (m <= 0.f) continue;
                float sn, cs; fastSinCos(ph, sn, cs);
                const float a = m * cs, b = m * sn;
                if (ch == 0) { sxRe[(size_t)k] += a; sxIm[(size_t)k] += b; sxRe[(size_t)(N - k)] += a; sxIm[(size_t)(N - k)] -= b; }
                else { sxRe[(size_t)k] -= b; sxIm[(size_t)k] += a; sxRe[(size_t)(N - k)] += b; sxIm[(size_t)(N - k)] += a; }
            }
        }
        sxFresh = false;
        sxFft.run(sxRe.data(), sxIm.data(), true);
        const float norm = 1.f / ((float)N * 1.5f);
        for (int i = 0; i < N; ++i) {
            const int at = (sxPos + i) & (N - 1);
            sxOlaL[(size_t)at] += sxRe[(size_t)i] * sxWin[(size_t)i] * norm;
            sxOlaR[(size_t)at] += sxIm[(size_t)i] * sxWin[(size_t)i] * norm;
        }
    }
    void spectral(float* L, float* R, int n, const float* p, const float* gm) {
        if (sxN <= 0) return;
        const float mix = fclamp(p[pSxMix] + gm[mdFxSpecMix], 0.f, 1.f);
        const int mask = sxN - 1;
        for (int i = 0; i < n; ++i) {
            // ring: sxPos is the oldest input sample and the OLA sample due now (the output lags by one frame)
            sxInL[(size_t)sxPos] = L[i]; sxInR[(size_t)sxPos] = R[i];
            const float wl = sxOlaL[(size_t)sxPos], wr = sxOlaR[(size_t)sxPos];
            sxOlaL[(size_t)sxPos] = 0.f; sxOlaR[(size_t)sxPos] = 0.f;
            sxPos = (sxPos + 1) & mask;
            if (--sxToNext <= 0) { spectralHop(p, gm); sxToNext = sxH; }
            L[i] = L[i] * (1.f - mix) + wl * mix;
            R[i] = R[i] * (1.f - mix) + wr * mix;
        }
    }
};
