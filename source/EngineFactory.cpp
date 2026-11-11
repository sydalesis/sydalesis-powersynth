// Driftfield 3 — picks the engine build for the running CPU; owns allocation/free of shared data.
#include "Params.hpp"
#include "Wavetables.hpp"
#include "EngineIface.hpp"
#include "Shared.hpp"
#include "SpecData.hpp"
#include "Analysis.hpp"

namespace drift_generic { drift::IEngine* create(); }
namespace drift_avx2 { drift::IEngine* create(); }

namespace drift {

static bool cpuHasAvx2Fma()
{
#if defined(__x86_64__) || defined(__i386__)
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
#else
    return false;
#endif
}

IEngine* createEngine(bool forceGeneric)
{
    IEngine* e = (!forceGeneric && cpuHasAvx2Fma()) ? drift_avx2::create() : drift_generic::create();
    for (uint32_t i = 0; i < kEngineParams; ++i) e->setParam(i, P(i).def);
    for (uint32_t i = pX0; i < pX0 + kExtParams; ++i) e->setParam(i, P(i).def);
    return e;
}

void deleteWaveTable(WaveTable* t) { delete t; }

void deleteSample(SampleBuf* s)
{
    if (!s) return;
    delete[] s->data;
    delete s;
}

// Granular source: mono, peak-normalised, kept at the file's sample rate.
SampleBuf* loadSampleFile(const char* path, std::string* err)
{
    std::vector<uint8_t> file;
    if (!readFileUtf8(path, file)) { if (err) *err = "cannot read file"; return nullptr; }
    std::vector<float> x;
    float sr = 48000.f;
    if (!decodeWavMono(file, x, sr, err)) return nullptr;
    float pk = 0.f;
    for (float v : x) pk = std::max(pk, std::fabs(v));
    const float g = pk > 1e-6f ? 0.9f / pk : 1.f;
    SampleBuf* s = new SampleBuf();
    s->len = (int)x.size();
    s->sr = sr;
    s->data = new float[x.size() + 4];
    for (size_t i = 0; i < x.size(); ++i) s->data[i] = x[i] * g;
    for (int i = 0; i < 4; ++i) s->data[x.size() + i] = s->data[i];
    { double e = 0.0; for (size_t i = 0; i < x.size(); ++i) e += (double)s->data[i] * s->data[i]; s->rms = x.empty() ? 0.13f : (float)std::sqrt(e / (double)x.size()); }
    s->name = fileStem(path);
    return s;
}


// ---- 5.0 spectral analysis ------------------------------------------------------------------------
bool analyseSpec(const float* x, int n, float sr, SpecData& d)
{
    if (!x || n < 1024 || sr < 8000.f) return false;
    const int maxN = (int)(sr * kSpecMaxSec);
    if (n > maxN) n = maxN;
    d = SpecData();
    d.sr = sr;
    { double e = 0.0; for (int i = 0; i < n; ++i) e += (double)x[i] * x[i]; d.rmsIn = (float)std::sqrt(e / (double)n); }   // 6.0
    d.N = sr > 50000.f ? 4096 : 2048;
    d.hop = d.N / 4;
    d.bins = d.N / 2 + 1;
    const int N = d.N, H = d.hop, B = d.bins;
    d.frames = std::max(1, (n - N) / H + 2);         // the last frame runs past the end (zero padded)
    d.mag.assign((size_t)d.frames * B, 0.f);
    d.dev.assign((size_t)d.frames * B, 0);
    d.ph.assign((size_t)d.frames * B, 0);
    std::vector<double> win((size_t)N), re((size_t)N), im((size_t)N), prev((size_t)B, 0.0);
    for (int i = 0; i < N; ++i) win[(size_t)i] = 0.5 - 0.5 * std::cos(ana::kPi2 * i / N);
    const double twoPi = ana::kPi2;
    float peak = 0.f;
    for (int f = 0; f < d.frames; ++f) {
        const int at = f * H - N / 2;                 // frame f is centred on sample f * hop
        for (int i = 0; i < N; ++i) {
            const int k = at + i;
            re[(size_t)i] = (k >= 0 && k < n) ? x[k] * win[(size_t)i] : 0.0;
            im[(size_t)i] = 0.0;
        }
        ana::fft(re, im);
        float* mg = d.mag.data() + (size_t)f * B;
        int16_t* dv = d.dev.data() + (size_t)f * B;
        int16_t* pp = d.ph.data() + (size_t)f * B;
        for (int k = 0; k < B; ++k) {
            const double a = std::sqrt(re[(size_t)k] * re[(size_t)k] + im[(size_t)k] * im[(size_t)k]);
            const double phs = std::atan2(im[(size_t)k], re[(size_t)k]);
            // instantaneous frequency from the phase advance over one hop
            double dp = phs - prev[(size_t)k] - twoPi * k * H / N;
            dp -= twoPi * std::floor(dp / twoPi + 0.5);
            prev[(size_t)k] = phs;
            double dev = f == 0 ? 0.0 : dp * N / (twoPi * H);
            if (dev > 3.99) dev = 3.99;
            if (dev < -3.99) dev = -3.99;
            mg[k] = (float)a;
            dv[k] = (int16_t)std::lround(dev * kSpecDevScale);
            pp[k] = (int16_t)std::lround(phs / 3.14159265358979 * 32767.0);
            if (mg[k] > peak) peak = mg[k];
        }
    }
    d.peak = peak > 0.f ? peak : 1.f;
    // root: the detected pitch of a pitched recording, else C4
    std::vector<float> xs(x, x + n);
    const float f0 = n > 4096 ? ana::detectF0(xs, sr) : 0.f;
    d.pitched = f0 > 30.f;
    d.root = d.pitched ? f0 : 261.6256f;
    return true;
}

SpecData* loadSpecFile(const char* path, std::string* err)
{
    std::vector<uint8_t> file;
    if (!readFileUtf8(path, file)) { if (err) *err = "cannot read file"; return nullptr; }
    std::vector<float> x;
    float sr = 48000.f;
    if (!decodeWavMono(file, x, sr, err)) return nullptr;
    float pk = 0.f;
    for (float v : x) pk = std::max(pk, std::fabs(v));
    if (pk < 1e-6f) { if (err) *err = "the file is silent"; return nullptr; }
    for (float& v : x) v *= 0.9f / pk;
    SpecData* d = new SpecData();
    if (!analyseSpec(x.data(), (int)x.size(), sr, *d)) { delete d; if (err) *err = "the file is too short"; return nullptr; }
    d->name = fileStem(path);
    return d;
}

void deleteSpec(SpecData* s) { delete s; }

} // namespace drift
