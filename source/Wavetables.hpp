// Driftfield 3 — band-limited wavetables: built-in banks + .wav file loader (Serum/Vital-style).
// Layout per table: [frame][mip][kWtSize + 1 guard sample]; mip m keeps harmonics <= (1024 >> m).
#pragma once
#include <cmath>
#include <complex>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace drift {

static constexpr int kWtBanks     = 8;
static constexpr int kWtFrames    = 16;     // built-in frame count
static constexpr int kWtMaxFrames = 256;    // file tables
static constexpr int kWtMips      = 11;
static constexpr int kWtSize      = 2048;
static constexpr int kWtHarm      = 1024;

using cpx = std::complex<float>;

// NOTE: the DSP engine is compiled twice (generic + AVX2). To keep the two builds from sharing
// inline code, the engine only touches plain fields here (frames, mips, raw) and the
// static-inline helpers below; allocation and freeing happen in generic code only.
struct WaveTable {
    int frames = 0;           // 0 = sentinel meaning "no file table, use built-in"
    int mips = kWtMips;       // display-only tables keep just mip 0
    float* raw = nullptr;     // == data.data()
    WaveTable* spectral = nullptr;   // denser frame set made by spectral (magnitude/phase) interpolation
    std::string name;
    std::vector<float> data;
    ~WaveTable() { delete spectral; }

    inline const float* table(int frame, int mip) const {
        if (mip >= mips) mip = mips - 1;
        return raw + ((size_t)frame * mips + mip) * (kWtSize + 1);
    }
    inline float* tableW(int frame, int mip) {
        return raw + ((size_t)frame * mips + mip) * (kWtSize + 1);
    }
    static inline int mipFor(float inc) {
        const float hMax = 0.5f / (inc > 1e-7f ? inc : 1e-7f);
        int m = 0;
        while (m < kWtMips - 1 && (float)(kWtHarm >> m) > hMax) ++m;
        return m;
    }
};

static inline const float* wtTable(const WaveTable& t, int frame, int mip) {
    if (mip >= t.mips) mip = t.mips - 1;
    return t.raw + ((size_t)frame * t.mips + mip) * (kWtSize + 1);
}
static inline int wtMipFor(float inc) {
    const float hMax = 0.5f / (inc > 1e-7f ? inc : 1e-7f);
    int m = 0;
    while (m < kWtMips - 1 && (float)(kWtHarm >> m) > hMax) ++m;
    return m;
}

// ---------------------------------------------------------------------------------------------
namespace fft {
// twiddles for the largest size used (kWtSize); smaller powers of two index into it with a stride
struct Twiddles {
    std::vector<cpx> w;
    Twiddles() { w.resize(kWtSize / 2); for (int i = 0; i < kWtSize / 2; ++i) { const double a = 2.0 * 3.14159265358979323846 * i / kWtSize; w[i] = cpx((float)std::cos(a), (float)std::sin(a)); } }
    static const Twiddles& get() { static Twiddles t; return t; }
};
// In-place transform computing X[n] = sum a[k] * e^{+i 2 pi k n / N} (no scaling).
inline void inverse(std::vector<cpx>& a) {
    const int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    const bool table = n <= kWtSize && (kWtSize % n) == 0;
    const auto& tw = Twiddles::get().w;
    for (int len = 2; len <= n; len <<= 1) {
        const int half = len / 2;
        if (table) {
            const int stride = kWtSize / len;
            for (int i = 0; i < n; i += len)
                for (int j = 0; j < half; ++j) {
                    const cpx u = a[i + j], v = a[i + j + half] * tw[(size_t)j * stride];
                    a[i + j] = u + v; a[i + j + half] = u - v;
                }
        } else {
            const double ang = 2.0 * 3.14159265358979323846 / len;
            const cpx wl((float)std::cos(ang), (float)std::sin(ang));
            for (int i = 0; i < n; i += len) {
                cpx w(1.f, 0.f);
                for (int j = 0; j < half; ++j) {
                    const cpx u = a[i + j], v = a[i + j + half] * w;
                    a[i + j] = u + v; a[i + j + half] = u - v;
                    w *= wl;
                }
            }
        }
    }
}
// Forward transform with 1/N scaling: bins are harmonic coefficients of the cycle.
inline void forward(std::vector<cpx>& a) {
    for (auto& x : a) x = std::conj(x);
    inverse(a);
    const float s = 1.f / (float)a.size();
    for (auto& x : a) x = std::conj(x) * s;
}
} // namespace fft

// Build a table from per-frame half spectra (bins[f][h], h = 0..kWtSize/2-1; bin 0 ignored).
// normalizePerFrame: each frame peak-normalised; otherwise one gain for the whole table.
inline WaveTable* buildTable(const std::vector<std::vector<cpx>>& bins, bool normalizePerFrame,
                             bool displayOnly, const std::string& name) {
    WaveTable* t = new WaveTable();
    t->frames = (int)bins.size();
    t->mips = displayOnly ? 1 : kWtMips;
    t->name = name;
    t->data.assign((size_t)t->frames * t->mips * (kWtSize + 1), 0.f);
    t->raw = t->data.data();
    std::vector<cpx> buf(kWtSize);
    std::vector<float> frameGain(t->frames, 1.f);
    const cpx I(0.f, 1.f);
    for (int f = 0; f < t->frames; ++f) {
        // two mip levels per transform: both results are real, so one goes in the real part and one in the imaginary
        for (int m = 0; m < t->mips; m += 2) {
            const int m2 = m + 1 < t->mips ? m + 1 : -1;
            const int hA = std::min(kWtHarm >> m, kWtSize / 2 - 1);
            const int hB = m2 >= 0 ? std::min(kWtHarm >> m2, kWtSize / 2 - 1) : 0;
            std::fill(buf.begin(), buf.end(), cpx(0.f, 0.f));
            for (int h = 1; h <= hA; ++h) {
                const cpx a = bins[f][h];
                const cpx b = h <= hB ? bins[f][h] : cpx(0.f, 0.f);
                buf[h] = a + I * b;
                buf[kWtSize - h] = std::conj(a) + I * std::conj(b);
            }
            fft::inverse(buf);
            float* out = t->tableW(f, m);
            for (int i = 0; i < kWtSize; ++i) out[i] = buf[i].real();
            out[kWtSize] = out[0];
            if (m2 >= 0) {
                float* out2 = t->tableW(f, m2);
                for (int i = 0; i < kWtSize; ++i) out2[i] = buf[i].imag();
                out2[kWtSize] = out2[0];
            }
        }
        float pk = 0.f;
        const float* m0 = t->table(f, 0);
        for (int i = 0; i < kWtSize; ++i) pk = std::max(pk, std::fabs(m0[i]));
        frameGain[f] = pk;
    }
    float globalPk = 0.f;
    for (float g : frameGain) globalPk = std::max(globalPk, g);
    for (int f = 0; f < t->frames; ++f) {
        const float pk = normalizePerFrame ? frameGain[f] : globalPk;
        const float g = pk > 1e-9f ? 0.9f / pk : 1.f;
        for (int m = 0; m < t->mips; ++m) {
            float* out = t->tableW(f, m);
            for (int i = 0; i <= kWtSize; ++i) out[i] *= g;
        }
    }
    return t;
}

// Spectral in-betweens: magnitudes interpolate linearly, phases along the shortest arc. Gives
// smooth timbral morphs between frames instead of the "two waves cross-fading" sound.
inline WaveTable* buildSpectralCompanion(const std::vector<std::vector<cpx>>& bins, bool normalizePerFrame) {
    const int nf = (int)bins.size();
    if (nf < 2) return nullptr;
    const int factor = nf <= 24 ? 4 : (nf <= 128 ? 2 : 1);
    if (factor == 1) return nullptr;
    const int out = (nf - 1) * factor + 1;
    std::vector<std::vector<cpx>> ib(out, std::vector<cpx>(kWtSize / 2, cpx(0.f, 0.f)));
    for (int k = 0; k < out; ++k) {
        const int f0 = std::min(k / factor, nf - 2);
        const float t = (float)(k - f0 * factor) / (float)factor;
        for (int h = 1; h < kWtSize / 2; ++h) {
            const cpx a = bins[f0][h], b = bins[f0 + 1][h];
            const float ma = std::abs(a), mb = std::abs(b);
            const float m = ma + (mb - ma) * t;
            if (m < 1e-9f) continue;
            float pa = ma > 1e-9f ? std::arg(a) : std::arg(b), pb = mb > 1e-9f ? std::arg(b) : pa;
            float d = pb - pa;
            while (d > 3.14159265f) d -= 6.2831853f;
            while (d < -3.14159265f) d += 6.2831853f;
            ib[k][h] = std::polar(m, pa + d * t);
        }
    }
    return buildTable(ib, normalizePerFrame, false, "");
}
inline WaveTable* buildTableWithSpectral(const std::vector<std::vector<cpx>>& bins, bool normalizePerFrame,
                                         bool displayOnly, const std::string& name) {
    WaveTable* t = buildTable(bins, normalizePerFrame, displayOnly, name);
    if (!displayOnly) t->spectral = buildSpectralCompanion(bins, normalizePerFrame);
    return t;
}

// ---------------------------------------------------------------------------------------------
// Built-in banks (fallback when no file is loaded)
class BuiltinTables;
const BuiltinTables& builtinTables();   // defined once, in generic code (Platform.cpp)

class BuiltinTables {
public:
    static const BuiltinTables& get() { return builtinTables(); }
    const WaveTable& bank(int b) const { return *banks[std::max(0, std::min(kWtBanks - 1, b))]; }
    ~BuiltinTables() { for (auto* b : banks) delete b; }
    WaveTable* banks[kWtBanks] = {};

    BuiltinTables() {
        for (int b = 0; b < kWtBanks; ++b) {
            std::vector<std::vector<cpx>> bins(kWtFrames, std::vector<cpx>(kWtSize / 2, cpx(0.f, 0.f)));
            for (int f = 0; f < kWtFrames; ++f) {
                const float t = (float)f / (float)(kWtFrames - 1);
                for (int h = 1; h < kWtSize / 2; ++h) {
                    const float a = h <= kWtHarm ? spectrum(b, h, t) : 0.f;
                    bins[f][h] = cpx(0.f, -0.5f * a);   // sine-phase series
                }
            }
            banks[b] = buildTableWithSpectral(bins, true, false, "");
        }
    }

    static float lerp(float a, float b, float x) { return a + (b - a) * x; }

    static float spectrum(int b, int h, float t) {
        const float fh = (float)h;
        const bool odd = (h & 1) != 0;
        switch (b) {
        case 0: { // Analog Morph: sine -> triangle -> saw -> square
            const float sine = h == 1 ? 1.f : 0.f;
            const float tri  = odd ? (((h - 1) / 2) % 2 ? -1.f : 1.f) * 8.f / (9.8696f * fh * fh) : 0.f;
            const float saw  = 2.f / (3.14159265f * fh) * (h % 2 ? 1.f : -1.f);
            const float sq   = odd ? 4.f / (3.14159265f * fh) : 0.f;
            const float x = t * 3.f;
            if (x < 1.f) return lerp(sine, tri, x);
            if (x < 2.f) return lerp(tri, saw, x - 1.f);
            return lerp(saw, sq, x - 2.f);
        }
        case 1: { const float c = 1.2f + 90.f * t * t; return (1.f / fh) / std::sqrt(1.f + std::pow(fh / c, 4.f)); }
        case 2: {
            static const float V[5][3] = { {325, 700, 2530}, {450, 800, 2830}, {800, 1150, 2900},
                                           {400, 1600, 2700}, {270, 2300, 3000} };
            const float x = t * 4.f; int i = (int)x; if (i > 3) i = 3; const float fr = x - i;
            const float hz = fh * 110.f;
            float a = 0.f;
            for (int k = 0; k < 3; ++k) {
                const float fc = lerp(V[i][k], V[i + 1][k], fr);
                const float bw = 80.f + 60.f * k;
                const float g  = k == 0 ? 1.f : (k == 1 ? 0.6f : 0.25f);
                a += g * std::exp(-0.5f * (hz - fc) * (hz - fc) / (bw * bw));
            }
            return (a + 0.04f / fh) * (h > 40 ? std::exp(-(fh - 40.f) / 10.f) : 1.f);
        }
        case 3: {
            static const int fib[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
            bool on = false; for (int k : fib) if (k == h) on = true;
            return (on ? 1.f / std::sqrt(fh) : 0.015f / fh) * std::exp(-fh / (3.f + 70.f * t));
        }
        case 4: { const float w = lerp(0.5f, 0.04f, t); return 2.f / (3.14159265f * fh) * std::sin(3.14159265f * fh * w); }
        case 5: {
            static const int bars[8] = {1, 2, 3, 4, 5, 6, 8, 10};
            static const float reg[4][8] = { {1, 0.8f, 0, 0, 0, 0, 0, 0}, {1, 0.6f, 0.7f, 0.3f, 0, 0, 0, 0},
                                             {0.8f, 1, 0.5f, 0.6f, 0.4f, 0.3f, 0.3f, 0}, {0.6f, 0.7f, 0.6f, 0.7f, 0.6f, 0.6f, 0.8f, 0.6f} };
            const float x = t * 3.f; int i = (int)x; if (i > 2) i = 2; const float fr = x - i;
            for (int k = 0; k < 8; ++k) if (bars[k] == h) return lerp(reg[i][k], reg[i + 1][k], fr);
            return 0.f;
        }
        case 6: {
            if (!odd) return 0.f;
            const float centre = lerp(5.f, 45.f, t);
            const float bump = 1.f + 3.f * std::exp(-0.5f * std::pow((fh - centre) / (2.f + centre * 0.25f), 2.f));
            return std::pow(fh, -lerp(2.f, 0.7f, t)) * bump * (h > 120 ? std::exp(-(fh - 120.f) / 30.f) : 1.f);
        }
        default: {
            uint32_t s = (uint32_t)h * 2654435761u; s ^= s >> 13; s *= 0x5bd1e995u; s ^= s >> 15;
            const float r = 0.3f + 0.7f * (float)(s & 0xFFFF) / 65535.f;
            return r / fh * std::exp(-fh / (3.f + 45.f * t));
        }
        }
    }
};

// ---------------------------------------------------------------------------------------------
// WAV wavetable loader. Accepts PCM 8/16/24/32 and float 32/64, any channel count (uses ch 1).
// Frame size: Serum "clm " chunk if present, else 2048; a file shorter than one frame is a single cycle.
// UTF-8 path -> bytes (std::filesystem handles wide paths on Windows)
inline bool readFileUtf8(const char* path, std::vector<uint8_t>& out) {
    std::error_code ec;
    const std::filesystem::path p = std::filesystem::u8path(path);
    const auto sz = std::filesystem::file_size(p, ec);
    if (ec || sz < 44 || sz > 256ull * 1024 * 1024) return false;
    std::ifstream in(p, std::ios::binary);
    if (!in) return false;
    out.resize((size_t)sz);
    in.read(reinterpret_cast<char*>(out.data()), (std::streamsize)sz);
    return (size_t)in.gcount() == (size_t)sz;
}

inline std::string fileStem(const std::string& path) {
    size_t s = path.find_last_of("/\\");
    std::string n = s == std::string::npos ? path : path.substr(s + 1);
    const size_t d = n.find_last_of('.');
    return d == std::string::npos ? n : n.substr(0, d);
}

inline WaveTable* loadWavetableFile(const char* path, bool displayOnly, std::string* err = nullptr) {
    auto fail = [&](const char* m) -> WaveTable* { if (err) *err = m; return nullptr; };
    std::vector<uint8_t> file;
    if (!readFileUtf8(path, file)) return fail("cannot read file");
    auto u16 = [&](size_t o) { return (uint32_t)file[o] | ((uint32_t)file[o + 1] << 8); };
    auto u32 = [&](size_t o) { return u16(o) | (u16(o + 2) << 16); };
    if (std::memcmp(file.data(), "RIFF", 4) || std::memcmp(file.data() + 8, "WAVE", 4)) return fail("not a WAV file");

    uint32_t fmtTag = 0, channels = 0, bits = 0, clmSize = 0;
    size_t dataOff = 0, dataLen = 0;
    size_t o = 12;
    while (o + 8 <= file.size()) {
        const uint32_t len = u32(o + 4);
        const size_t body = o + 8;
        if (body + len > file.size() && std::memcmp(file.data() + o, "data", 4)) break;
        if (!std::memcmp(file.data() + o, "fmt ", 4) && len >= 16) {
            fmtTag = u16(body); channels = u16(body + 2); bits = u16(body + 14);
            if (fmtTag == 0xFFFE && len >= 26) fmtTag = u16(body + 24);
        } else if (!std::memcmp(file.data() + o, "data", 4)) {
            dataOff = body; dataLen = std::min<size_t>(len, file.size() - body);
        } else if (!std::memcmp(file.data() + o, "clm ", 4)) {
            std::string s((const char*)file.data() + body, std::min<size_t>(len, 64));
            const size_t p = s.find("<!>");
            if (p != std::string::npos) clmSize = (uint32_t)std::atoi(s.c_str() + p + 3);
        }
        o = body + len + (len & 1);
    }
    if (!dataOff || !channels || !bits) return fail("missing fmt/data chunk");
    const uint32_t bps = bits / 8, stride = bps * channels;
    if (bps == 0 || (fmtTag != 1 && fmtTag != 3)) return fail("unsupported WAV encoding");
    const size_t nSamp = dataLen / stride;
    if (nSamp < 8) return fail("file too short");

    std::vector<float> x(nSamp);
    for (size_t i = 0; i < nSamp; ++i) {
        const uint8_t* q = file.data() + dataOff + i * stride;
        float v = 0.f;
        if (fmtTag == 3 && bits == 32) { std::memcpy(&v, q, 4); }
        else if (fmtTag == 3 && bits == 64) { double d; std::memcpy(&d, q, 8); v = (float)d; }
        else if (bits == 8)  v = ((int)q[0] - 128) / 128.f;
        else if (bits == 16) v = (int16_t)(q[0] | (q[1] << 8)) / 32768.f;
        else if (bits == 24) v = (int32_t)(((uint32_t)q[0] << 8) | ((uint32_t)q[1] << 16) | ((uint32_t)q[2] << 24)) / 2147483648.f;
        else if (bits == 32) v = (int32_t)u32(dataOff + i * stride) / 2147483648.f;
        else return fail("unsupported bit depth");
        if (!std::isfinite(v)) v = 0.f;
        x[i] = v;
    }

    size_t frameLen = clmSize >= 16 && clmSize <= 65536 ? clmSize : kWtSize;
    if (nSamp < frameLen) frameLen = nSamp;                    // single cycle of arbitrary length
    size_t nFrames = nSamp / frameLen;
    size_t step = 1;
    if (nFrames > (size_t)kWtMaxFrames) { step = (nFrames + kWtMaxFrames - 1) / kWtMaxFrames; }
    const size_t outFrames = (nFrames + step - 1) / step;

    std::vector<std::vector<cpx>> bins(outFrames, std::vector<cpx>(kWtSize / 2, cpx(0.f, 0.f)));
    std::vector<cpx> buf;
    for (size_t fo = 0; fo < outFrames; ++fo) {
        const float* src = x.data() + fo * step * frameLen;
        // Resample one cycle to kWtSize with linear interpolation, then let the FFT band-limit it.
        buf.assign(kWtSize, cpx(0.f, 0.f));
        if (frameLen == (size_t)kWtSize) {
            for (int i = 0; i < kWtSize; ++i) buf[i] = cpx(src[i], 0.f);
        } else {
            for (int i = 0; i < kWtSize; ++i) {
                const double pos = (double)i * frameLen / kWtSize;
                const size_t i0 = (size_t)pos; const float fr = (float)(pos - i0);
                const float a = src[i0 % frameLen], b = src[(i0 + 1) % frameLen];
                buf[i] = cpx(a + (b - a) * fr, 0.f);
            }
        }
        fft::forward(buf);
        for (int h = 1; h < kWtSize / 2; ++h) bins[fo][h] = buf[h];
    }
    WaveTable* t = buildTableWithSpectral(bins, false, displayOnly, fileStem(path));
    if (err) err->clear();
    return t;
}

// ---------------------------------------------------------------------------------------------
// Generic WAV decode (all channels mixed to mono). Used for granular source samples.
inline bool decodeWavMono(const std::vector<uint8_t>& file, std::vector<float>& out, float& sr, std::string* err) {
    auto fail = [&](const char* m) { if (err) *err = m; return false; };
    if (file.size() < 44 || std::memcmp(file.data(), "RIFF", 4) || std::memcmp(file.data() + 8, "WAVE", 4)) return fail("not a WAV file");
    auto u16 = [&](size_t o) { return (uint32_t)file[o] | ((uint32_t)file[o + 1] << 8); };
    auto u32 = [&](size_t o) { return u16(o) | (u16(o + 2) << 16); };
    uint32_t fmtTag = 0, channels = 0, bits = 0, rate = 0;
    size_t dataOff = 0, dataLen = 0, o = 12;
    while (o + 8 <= file.size()) {
        const uint32_t len = u32(o + 4);
        const size_t body = o + 8;
        if (body + len > file.size() && std::memcmp(file.data() + o, "data", 4)) break;
        if (!std::memcmp(file.data() + o, "fmt ", 4) && len >= 16) {
            fmtTag = u16(body); channels = u16(body + 2); rate = u32(body + 4); bits = u16(body + 14);
            if (fmtTag == 0xFFFE && len >= 26) fmtTag = u16(body + 24);
        } else if (!std::memcmp(file.data() + o, "data", 4)) {
            dataOff = body; dataLen = std::min<size_t>(len, file.size() - body);
        }
        o = body + len + (len & 1);
    }
    if (!dataOff || !channels || !bits || !rate) return fail("missing fmt/data chunk");
    const uint32_t bps = bits / 8, stride = bps * channels;
    if (bps == 0 || (fmtTag != 1 && fmtTag != 3)) return fail("unsupported WAV encoding");
    size_t nSamp = dataLen / stride;
    const size_t maxSamp = (size_t)rate * 120;   // two minutes is plenty for a grain source
    if (nSamp > maxSamp) nSamp = maxSamp;
    if (nSamp < 64) return fail("file too short");
    out.assign(nSamp, 0.f);
    for (size_t i = 0; i < nSamp; ++i) {
        float acc = 0.f;
        for (uint32_t c = 0; c < channels; ++c) {
            const size_t at = dataOff + i * stride + c * bps;
            const uint8_t* q = file.data() + at;
            float v = 0.f;
            if (fmtTag == 3 && bits == 32) { std::memcpy(&v, q, 4); }
            else if (fmtTag == 3 && bits == 64) { double d; std::memcpy(&d, q, 8); v = (float)d; }
            else if (bits == 8)  v = ((int)q[0] - 128) / 128.f;
            else if (bits == 16) v = (int16_t)(q[0] | (q[1] << 8)) / 32768.f;
            else if (bits == 24) v = (int32_t)(((uint32_t)q[0] << 8) | ((uint32_t)q[1] << 16) | ((uint32_t)q[2] << 24)) / 2147483648.f;
            else if (bits == 32) v = (int32_t)u32(at) / 2147483648.f;
            else return fail("unsupported bit depth");
            if (!std::isfinite(v)) v = 0.f;
            acc += v;
        }
        out[i] = acc / (float)channels;
    }
    sr = (float)rate;
    return true;
}

} // namespace drift
