// Driftfield 5 — real-time FFT for the spectral oscillator and the Spectral FX.
// Included inside the engine namespace (compiled once per ISA build), so it may use the build's flags.
// Tables are allocated by init() (never on the audio thread).
#pragma once

struct SpecFFT {
    int n = 0;
    std::vector<float> c, s;      // cos / sin of 2*pi*k/n, k < n/2
    std::vector<float> twR, twI;  // 6.0.4: the same values laid out per stage (contiguous, so the butterflies vectorise)
    std::vector<int> rev;
    void init(int N) {
        n = N;
        c.resize((size_t)N / 2); s.resize((size_t)N / 2); rev.resize((size_t)N);
        for (int k = 0; k < N / 2; ++k) { c[(size_t)k] = (float)std::cos(6.283185307179586 * k / N); s[(size_t)k] = (float)std::sin(6.283185307179586 * k / N); }
        int bits = 0; while ((1 << bits) < N) ++bits;
        for (int i = 0; i < N; ++i) { int r = 0; for (int b = 0; b < bits; ++b) if (i & (1 << b)) r |= 1 << (bits - 1 - b); rev[(size_t)i] = r; }
        twR.assign((size_t)N, 0.f); twI.assign((size_t)N, 0.f);
        for (int len = 2; len <= N; len <<= 1) {          // stage `len` starts at offset len/2
            const int half = len >> 1, step = N / len;
            for (int k = 0; k < half; ++k) { twR[(size_t)(half + k)] = c[(size_t)(k * step)]; twI[(size_t)(half + k)] = s[(size_t)(k * step)]; }
        }
    }
    // in place; inverse is unscaled (divide by n yourself)
    void run(float* __restrict re, float* __restrict im, bool inverse) const {
        for (int i = 0; i < n; ++i) { const int j = rev[(size_t)i]; if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); } }
        const float sg = inverse ? 1.f : -1.f;
        // len 2: twiddle 1
        for (int i = 0; i < n; i += 2) {
            const float tr = re[i + 1], ti = im[i + 1];
            re[i + 1] = re[i] - tr; im[i + 1] = im[i] - ti; re[i] += tr; im[i] += ti;
        }
        // len 4: twiddles 1 and (0, sg)
        for (int i = 0; i < n; i += 4) {
            { const float tr = re[i + 2], ti = im[i + 2];
              re[i + 2] = re[i] - tr; im[i + 2] = im[i] - ti; re[i] += tr; im[i] += ti; }
            { const float tr = -im[i + 3] * sg, ti = re[i + 3] * sg;
              re[i + 3] = re[i + 1] - tr; im[i + 3] = im[i + 1] - ti; re[i + 1] += tr; im[i + 1] += ti; }
        }
        for (int len = 8; len <= n; len <<= 1) {
            const int half = len >> 1;
            const float* __restrict wR = twR.data() + half;
            const float* __restrict wI = twI.data() + half;
            for (int i = 0; i < n; i += len) {
                float* __restrict ru = re + i; float* __restrict iu = im + i;
                float* __restrict rv = re + i + half; float* __restrict iv = im + i + half;
                for (int k = 0; k < half; ++k) {
                    const float wr = wR[k], wi = sg * wI[k];
                    const float tr = rv[k] * wr - iv[k] * wi, ti = rv[k] * wi + iv[k] * wr;
                    rv[k] = ru[k] - tr; iv[k] = iu[k] - ti;
                    ru[k] += tr; iu[k] += ti;
                }
            }
        }
    }
};

static inline float wrapPi(float x) {
    x -= 6.2831853f * std::floor(x * 0.15915494f + 0.5f);
    return x;
}
