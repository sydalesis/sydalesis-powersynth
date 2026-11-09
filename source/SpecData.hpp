// Driftfield 5 — spectral analysis of a recording for the Spectral oscillator.
// Short-time Fourier analysis (Hann, 4x overlap): per frame and bin the magnitude, the frequency
// deviation from the bin centre (instantaneous frequency) and the phase. The engine resynthesises from
// this with a phase-locked vocoder, so pitch, time and spectrum can be changed independently.
// Analysis runs on the file-loader thread (generic code); the engine only reads it.
#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>

namespace drift {

static constexpr int kSpecMaxSec = 30;        // longer files are analysed up to here
static constexpr int kSpecDevScale = 8000;    // dev: bins * kSpecDevScale (+-4 bins)

struct SpecData {
    int N = 2048, hop = 512, bins = 1025, frames = 0;
    float sr = 48000.f;
    float root = 261.6256f;                   // frequency played at the original pitch (detected, or C4)
    bool pitched = false;
    float peak = 1.f;                         // largest magnitude in the file
    std::vector<float> mag;                   // frames * bins
    std::vector<int16_t> dev, ph;             // frames * bins
    std::string name;
    const float* magOf(int f) const { return mag.data() + (size_t)f * bins; }
    const int16_t* devOf(int f) const { return dev.data() + (size_t)f * bins; }
    const int16_t* phOf(int f) const { return ph.data() + (size_t)f * bins; }
    float seconds() const { return frames > 0 ? (float)((frames - 1) * hop + N) / sr : 0.f; }
};

// Generic code (EngineFactory.cpp): decode + analyse a WAV; nullptr on failure.
SpecData* loadSpecFile(const char* path, std::string* err = nullptr);
// Analyse mono audio (any sample rate); returns false when it is empty / too short.
bool analyseSpec(const float* x, int n, float sr, SpecData& out);
void deleteSpec(SpecData* s);

} // namespace drift
