// Driftfield 4 — engine interface. The engine is compiled twice (generic SSE2 and AVX2/FMA);
// createEngine() picks the best one for the running CPU.
#pragma once
#include <cstdint>

namespace drift {

struct WaveTable;
struct SampleBuf;
struct MsegData;
struct Telemetry;
struct ScaleDef;
struct KbmDef;
struct ITuningProvider;
struct SceneData;
struct MotionData;
struct ModalData;
struct SpecData;

// 4.2 multicore: runs tasks 0..count-1 on the calling thread and the pool's workers and returns when
// all are done. fn gets (ctx, task, slot) where slot 0 is the caller and 1..workers() the workers.
struct IWorkerPool {
    virtual ~IWorkerPool() {}
    virtual int workers() const = 0;
    virtual void run(int count, void (*fn)(void* ctx, int task, int slot), void* ctx) = 0;
};
static constexpr int kMaxPoolSlots = 8;   // caller + up to 7 workers

struct IEngine {
    virtual ~IEngine() {}
    virtual void setSampleRate(float sr) = 0;
    virtual void reset() = 0;
    virtual void setParam(uint32_t index, float value) = 0;
    virtual void midi(const uint8_t* data, uint32_t size) = 0;
    // host transport for tempo sync (call before process); beat = quarter notes since song start
    virtual void setTransport(double bpm, bool playing, double beat) = 0;
    virtual void process(float* outL, float* outR, uint32_t frames) = 0;
    // non-RT hand-overs (lock-free inside)
    virtual void postTable(int osc, WaveTable* table) = 0;           // nullptr = built-in table
    virtual void postSample(int osc, SampleBuf* sample) = 0;         // nullptr = none
    virtual void setCustomSpectrum(int osc, const float* amps128) = 0;
    virtual void setLfoShape(int lfo, const float* points64) = 0;
    virtual void setMseg(int idx, const MsegData& m) = 0;
    virtual void setFxOrder(const int* order) = 0;
    virtual void setTelemetry(Telemetry* t) = 0;
    virtual const char* isaName() const = 0;
    virtual int activeVoices() const = 0;
    virtual void setEnvClassic(bool on) = 0;      // 3.0 attack curve (old sessions)
    virtual void setGuardCap(int voices) = 0;     // CPU guard voice cap (kMaxVoices = off)
    // 4.0 tuning: Scala scale + optional keyboard map (copied; nullptr clears), external per-note tuning
    virtual void setUserScale(const ScaleDef* scale, const KbmDef* kbm) = 0;
    virtual void setTuningProvider(ITuningProvider* provider) = 0;
    // 4.1: which layer this engine plays (0 = layer 1, 1 = layer 2); gains come from the lyMode params
    virtual void setLayer(int index) = 0;
    // 4.2
    virtual void setScenes(const SceneData& d) = 0;
    virtual void setPerfAxes(const int* axes4) = 0;               // 5.1.1: XY pad -> macro (pad1 X, pad1 Y, pad2 X, pad2 Y)
    virtual void setMotion(const MotionData& d) = 0;          // copied (large; call off the audio thread)
    virtual void setModes(int osc, const ModalData& d) = 0;   // analysed modal bank of an oscillator
    virtual void setWorkerPool(IWorkerPool* pool) = 0;        // nullptr = render voices on one thread
    // 5.0: spectral analysis for an oscillator: which 0 = source, 1 = cross-synthesis partner (nullptr = none)
    virtual void postSpectral(int osc, int which, SpecData* data) = 0;
};

// Creates the engine and loads every parameter default.
IEngine* createEngine(bool forceGeneric = false);
// Frees memory in generic code (the engine never frees what it is handed).
void deleteWaveTable(WaveTable* t);
void deleteSample(SampleBuf* s);

} // namespace drift

#include <string>
namespace drift { SampleBuf* loadSampleFile(const char* path, std::string* err = nullptr); }
