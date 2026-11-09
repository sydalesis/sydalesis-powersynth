// Driftfield 5 — real-time FFT for the spectral oscillator and the Spectral FX.
// Included inside the engine namespace (compiled once per ISA build), so it may use the build's flags.
// Tables are allocated by init() (never on the audio thread).
#pragma once

struct SpecFFT {
    int n = 0;
    std::vector<float> c, s;      // cos / sin of 2*pi*k/n, k < n/2
    std::vector<int> rev;
    void init(int N) {
        n = N;
        c.resize((size_t)N / 2); s.resize((size_t)N / 2); rev.resize((size_t)N);
        for (int k = 0; k < N / 2; ++k) { c[(size_t)k] = (float)std::cos(6.283185307179586 * k / N); s[(size_t)k] = (float)std::sin(6.283185307179586 * k / N); }
        int bits = 0; while ((1 << bits) < N) ++bits;
        for (int i = 0; i < N; ++i) { int r = 0; for (int b = 0; b < bits; ++b) if (i & (1 << b)) r |= 1 << (bits - 1 - b); rev[(size_t)i] = r; }
    }
    // in place; inverse is unscaled (divide by n yourself)
    void run(float* re, float* im, bool inverse) const {
        for (int i = 0; i < n; ++i) { const int j = rev[(size_t)i]; if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); } }
        const float sg = inverse ? 1.f : -1.f;
        for (int len = 2; len <= n; len <<= 1) {
            const int half = len >> 1, step = n / len;
            for (int i = 0; i < n; i += len) {
                for (int k = 0; k < half; ++k) {
                    const float wr = c[(size_t)(k * step)], wi = sg * s[(size_t)(k * step)];
                    const int u = i + k, v = u + half;
                    const float tr = re[v] * wr - im[v] * wi, ti = re[v] * wi + im[v] * wr;
                    re[v] = re[u] - tr; im[v] = im[u] - ti;
                    re[u] += tr; im[u] += ti;
                }
            }
        }
    }
};

static inline float wrapPi(float x) {
    x -= 6.2831853f * std::floor(x * 0.15915494f + 0.5f);
    return x;
}
