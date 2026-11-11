// Driftfield 3 — DPF wrapper (VST3 / CLAP / standalone)
#include "DistrhoPlugin.hpp"
#include "Params.hpp"
#include "EngineIface.hpp"
#include "Shared.hpp"
#include "Wavetables.hpp"
#include "Library.hpp"
#include "Spectra.hpp"
#include "Pool.hpp"
#include "SpecData.hpp"
#include <memory>
#include <cmath>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <vector>
#include "mts/libMTSClient.h"
#if defined(__SSE__) || defined(_M_X64) || defined(__x86_64__)
 #include <xmmintrin.h>
 #define DRIFT_FTZ 1
#elif defined(__aarch64__) || defined(_M_ARM64)
 // 5.1.6 (Apple Silicon): flush denormals through FPCR.FZ, as MXCSR FTZ/DAZ does on x86
 #define DRIFT_FTZ_ARM 1
 static inline uint64_t driftFpcrGet() { uint64_t v; __asm__ __volatile__("mrs %0, fpcr" : "=r"(v)); return v; }
 static inline void driftFpcrSet(uint64_t v) { __asm__ __volatile__("msr fpcr, %0" : : "r"(v)); }
#endif

START_NAMESPACE_DISTRHO

#include "Presets.hpp"

// MTS-ESP client: per-note tuning from a master plug-in (e.g. MTS-ESP Mini / Suite) when one is running
class MtsProvider : public drift::ITuningProvider
{
public:
    MtsProvider() : client(MTS_RegisterClient()) {}
    ~MtsProvider() override { if (client != nullptr) MTS_DeregisterClient(client); }
    bool connected() override { return client != nullptr && MTS_HasMaster(client); }
    double noteSemis(int note, int) override
    {
        const double f = MTS_NoteToFrequency(client, (char)(note & 127), (signed char)-1);
        return f > 0.0 ? 69.0 + 12.0 * std::log2(f / 440.0) : (double)note;
    }
    bool filtered(int note) { return client != nullptr && MTS_HasMaster(client) && MTS_ShouldFilterNote(client, (char)(note & 127), (signed char)-1); }
    const char* scaleName() { return client != nullptr ? MTS_GetScaleName(client) : ""; }
private:
    MTSClient* client;
};

// Extra plug-in states after the preset states
enum { kStPresetId = kStateCount, kStEnvModel, kStMidiMap, kStLayerNames, kStThreads, kStTotal };

static std::string driftDocFile(const char* name)
{
    return drift::lib::u8(drift::lib::fromU8(drift::lib::dataDir()) / name);
}

// Decodes wavetable and sample files on a worker thread so a session or preset load never blocks
// the host's thread on disk reads. Only the worker hands buffers to the engine (single producer).
class FileLoader
{
public:
    FileLoader(drift::IEngine* e0, drift::IEngine* e1, std::atomic<int>* pend) : pending(pend) { engines[0] = e0; engines[1] = e1; th = std::thread([this] { loop(); }); }
    ~FileLoader() { { std::lock_guard<std::mutex> g(m); quit = true; } cv.notify_all(); if (th.joinable()) th.join(); }
    // slot (layer * 12) + 0..2 = wavetable osc A..C, + 3..5 = grain sample, + 6..8 = spectral source,
    // + 9..11 = spectral cross partner (5.0: analysed here, off the audio thread)
    void request(int slot, const std::string& stored)
    {
        { std::lock_guard<std::mutex> g(m); req[slot] = stored; if (!has[slot]) { has[slot] = true; pending->fetch_add(1); } }
        cv.notify_all();
    }
    // offline rendering: wait for outstanding loads (bounded)
    void waitIdle(double maxSec)
    {
        const auto t0 = std::chrono::steady_clock::now();
        while (pending->load() > 0 && std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() < maxSec)
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
private:
    void loop()
    {
        std::unique_lock<std::mutex> lk(m);
        for (;;)
        {
            int slot = -1;
            for (int i = 0; i < kSlots; ++i) if (has[i]) { slot = i; break; }
            if (slot < 0) { if (quit) return; cv.wait(lk); continue; }
            const std::string v = req[slot];
            has[slot] = false;
            lk.unlock();
            drift::IEngine* const engine = engines[slot / 12];
            const int s12 = slot % 12;
            if (s12 < 3) engine->postTable(s12, v.empty() ? nullptr : drift::loadWavetableFile(drift::lib::resolve(v).c_str(), false));
            else if (s12 < 6) engine->postSample(s12 - 3, v.empty() ? nullptr : drift::loadSampleFile(drift::lib::resolve(v).c_str()));
            else engine->postSpectral((s12 - 6) % 3, s12 < 9 ? 0 : 1, v.empty() ? nullptr : drift::loadSpecFile(drift::lib::resolve(v).c_str()));
            pending->fetch_sub(1);
            lk.lock();
        }
    }
    static constexpr int kSlots = 24;
    drift::IEngine* engines[2];
    std::atomic<int>* pending;
    std::thread th;
    std::mutex m;
    std::condition_variable cv;
    bool quit = false;
    std::string req[kSlots];
    bool has[kSlots] = {};
};

class DriftfieldPlugin : public Plugin
{
public:
    DriftfieldPlugin()
        : Plugin(kParamCount, kFactoryPresetCount, kStTotal)
    {
        shared.reset(new drift::SharedBlock());
        std::memset(&shared->tel, 0, sizeof(shared->tel));
        std::memset(&shared->tel2, 0, sizeof(shared->tel2));
        shared->tel.newest = -1; shared->tel2.newest = -1;
        mts.reset(new MtsProvider());
        for (int l = 0; l < 2; ++l)
        {
            eng[l].reset(drift::createEngine());
            eng[l]->setLayer(l);
            eng[l]->setSampleRate((float)getSampleRate());
            eng[l]->setTelemetry(l == 0 ? &shared->tel : &shared->tel2);
            eng[l]->setTuningProvider(mts.get());
        }
        layerBuf.assign(2 * kLayerChunk, 0.f);
        allocCapture(getSampleRate());
        loader.reset(new FileLoader(eng[0].get(), eng[1].get(), &shared->loadsPending));
        readUserDefaults();
        setThreads(threadsSetting.c_str());
        if (const char* g = std::getenv("DF_GUARD_TEST")) { shared->guardRealtime.store(1); guardTestScale = (float)std::atof(g); }   // test hook
        loadProgramInternal(0, false);
        loader->waitIdle(5.0);
    }
    ~DriftfieldPlugin() override
    {
        loader.reset();
        for (auto& e : eng) { e->setTuningProvider(nullptr); e->setWorkerPool(nullptr); }
        pool.reset();
        shared->capture.store(nullptr);
    }

    drift::SharedBlock* sharedBlock() { return shared.get(); }

protected:
    const char* getLabel() const override       { return "Sydalesis"; }
    const char* getDescription() const override { return "Sydalesis 6 - MPE synthesizer: wavetable, analog, additive, physical, granular, spectral and FM oscillators, dual morphing filters, MSEG/LFO matrix, arp and sequencer"; }
    const char* getMaker() const override       { return "Sydalesis"; }
    const char* getHomePage() const override    { return "https://sydalesis.bandcamp.com"; }
    const char* getLicense() const override     { return "Copyright Sydalesis"; }
    uint32_t    getVersion() const override     { return d_version(6, 0, 0); }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void initParameter(uint32_t index, Parameter& param) override
    {
        const ParamDef& d = P(index);
        param.hints = kParameterIsAutomatable;
        if (d.flags & fInt) param.hints |= kParameterIsInteger;
        if (d.flags & fLog) param.hints |= kParameterIsLogarithmic;
        if (d.flags & fBool) param.hints |= kParameterIsBoolean;
        if (d.flags & fHidden) param.hints |= kParameterIsHidden;
        param.name = d.name;
        param.symbol = d.symbol;
        param.unit = d.unit;
        param.ranges.min = d.min;
        param.ranges.max = d.max;
        param.ranges.def = d.def;
        if (d.enumNames)
        {
            const int n = (int)(d.max - d.min) + 1;
            ParameterEnumerationValue* const ev = new ParameterEnumerationValue[n];
            for (int i = 0; i < n; ++i) { ev[i].value = d.min + (float)i; ev[i].label = d.enumNames[i]; }
            param.enumValues.count = (uint8_t)n;
            param.enumValues.restrictedMode = true;
            param.enumValues.values = ev;
        }
    }

    void initProgramName(uint32_t index, String& name) override
    {
        name = String(kFactoryPresets[index].category) + String(" / ") + kFactoryPresets[index].name;
    }

    // DPF re-applies its own program index on every session load and whenever the host writes the
    // program parameter, even when the value is unchanged. Presets picked in our own browser never
    // reach that index, so a repeated load of the same index is ignored; the session's parameters
    // and states (restored right after) are what count.
    void loadProgram(uint32_t index) override
    {
        if (index == dpfProgram) return;
        dpfProgram = index;
        loadProgramInternal(index, true);
    }
    uint32_t dpfProgram = 0;
    std::string presetId;   // "F:<category>/<name>" or "U:<name>", "*" prefix = edited; shown in the editor header
    std::string layerNames; // "<layer 1 source>|<layer 2 source>", shown on the LAYERS page

    void initState(uint32_t index, State& state) override
    {
        state.hints = 0;
        state.defaultValue = "";
        if (index == kStPresetId) { state.key = "presetId"; state.label = "Preset"; return; }
        if (index == kStEnvModel) { state.key = "envModel"; state.label = "Envelope model"; return; }
        if (index == kStMidiMap)  { state.key = "midiMap";  state.label = "MIDI learn map"; return; }
        if (index == kStLayerNames) { state.key = "layerNames"; state.label = "Layer names"; return; }
        if (index == kStThreads) { state.key = "threads"; state.label = "Render threads"; return; }
        state.key = kStateKeys[index];
        state.label = kStateLabels[index];
        state.defaultValue = "";
        state.hints = 0;
    }

    void setState(const char* key, const char* value) override
    {
        if (key == nullptr) return;
        if (value == nullptr) value = "";
        if (key[0] == '_' && key[1] == '_')
        {
            if (std::strcmp(key, "__dfRestoreBegin") == 0) { restoring = true; sawNewFormat = false; }
            else if (std::strcmp(key, "__dfRestoreEnd") == 0)
            {
                // a session written before 3.1 carries neither the envelope-curve parameters nor the
                // preset id: keep its 3.0 attack so the project sounds the way it was mixed
                if (restoring && !sawNewFormat) setEnvModel(true);
                restoring = false;
            }
            else if (std::strcmp(key, "__dfProcessMode") == 0) { shared->guardRealtime.store(value[0] == '0' ? 1 : 0); processOffline.store(value[0] == '2'); }
            return;
        }
        if (std::strcmp(key, "presetId") == 0) { presetId = value; sawNewFormat = true; return; }
        if (std::strcmp(key, "envModel") == 0) { sawNewFormat = true; setEnvModel(std::strcmp(value, "classic") == 0); return; }
        if (std::strcmp(key, "midiMap") == 0) { int mm[drift::kMidiMapSize]; drift::midiMapFromString(value, mm); shared->setMap(mm); return; }
        if (std::strcmp(key, "layerNames") == 0) { layerNames = value; return; }
        if (std::strcmp(key, "threads") == 0) { setThreads(value); return; }
        const int k = stateIndexOf(key);
        if (k < 0) return;
        stateStr[k] = value != nullptr ? value : "";
        applyState(k);
    }

    String getState(const char* key) const override
    {
        if (key != nullptr && std::strcmp(key, "presetId") == 0) return String(presetId.c_str());
        if (key != nullptr && std::strcmp(key, "envModel") == 0) return String(shared->envClassic.load() ? "classic" : "");
        if (key != nullptr && std::strcmp(key, "midiMap") == 0) return String(shared->mapString().c_str());
        if (key != nullptr && std::strcmp(key, "layerNames") == 0) return String(layerNames.c_str());
        if (key != nullptr && std::strcmp(key, "threads") == 0) return String(threadsSetting.c_str());
        const int k = stateIndexOf(key);
        return String(k >= 0 ? stateStr[k].c_str() : "");
    }

    float getParameterValue(uint32_t index) const override { return index < kParamCount ? values[index] : 0.f; }
    void setParameterValue(uint32_t index, float value) override
    {
        if (index >= kParamCount) return;
        if (restoring && (index == pFEnvCurve || index == pAEnvCurve || index == pMEnvCurve)) sawNewFormat = true;
        setValue(index, value);
    }

    void sampleRateChanged(double sr) override { for (auto& e : eng) e->setSampleRate((float)sr); allocCapture(sr); }
    void activate() override { for (auto& e : eng) e->reset(); }

    void run(const float**, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) override
    {
        std::lock_guard<std::mutex> runGuardLock(runLock);
        const auto t0 = std::chrono::steady_clock::now();
       #ifdef DRIFT_FTZ
        const unsigned int oldCsr = _mm_getcsr();
        _mm_setcsr(oldCsr | 0x8040);
       #endif
       #ifdef DRIFT_FTZ_ARM
        const uint64_t oldFpcr = driftFpcrGet();
        driftFpcrSet(oldFpcr | (1ull << 24));
       #endif
        const TimePosition& tp = getTimePosition();
        double bpm = 120.0, beat = 0.0;
        if (tp.bbt.valid)
        {
            bpm = tp.bbt.beatsPerMinute;
            // quarter notes since song start
            const double beatLen = tp.bbt.beatType > 0.f ? 4.0 / tp.bbt.beatType : 1.0;
            beat = ((double)(tp.bbt.bar - 1) * tp.bbt.beatsPerBar + (double)(tp.bbt.beat - 1)
                    + (tp.bbt.ticksPerBeat > 0.0 ? tp.bbt.tick / tp.bbt.ticksPerBeat : 0.0)) * beatLen;
        }
        else if (tp.frame > 0)
            beat = (double)tp.frame / getSampleRate() * bpm / 60.0;
        for (auto& e : eng) e->setTransport(bpm, tp.playing, beat);
        // 5.1.1: PANIC from the editor: silence both layers, held / latched / arp / seq notes, FX tails
        if (shared->panicReq.exchange(0, std::memory_order_acq_rel)) { eng[0]->reset(); eng[1]->reset(); limEnv = 0.f; }
        // 5.1.1: MIDI program change (with bank select) loads a factory preset
        if (pendingPc >= 0) { applyProgramChange(pendingPc); pendingPc = -1; }
        const bool layerOn = values[pLyMode] >= 0.5f;
        if (!layerOn && layerWasOn) eng[1]->reset();     // layer 2 switched off: drop its voices
        layerWasOn = layerOn;

        if (processOffline.load(std::memory_order_relaxed) && shared->loadsPending.load(std::memory_order_relaxed) > 0)
            loader->waitIdle(10.0);   // bounce/export: never render before the session's tables are in

        float* const L = outputs[0];
        float* const R = outputs[1];
        uint32_t pos = 0;
        // 5.1.1: notes / wheels from the on-screen keyboard, at the start of the block
        for (uint32_t r = shared->uiMidiR.load(std::memory_order_relaxed), w = shared->uiMidiW.load(std::memory_order_acquire); r != w; ++r)
        {
            const uint32_t m = shared->uiMidi[r % drift::SharedBlock::kUiMidiSize];
            const uint8_t d[3] = { (uint8_t)(m & 0xFF), (uint8_t)((m >> 8) & 0x7F), (uint8_t)((m >> 16) & 0x7F) };
            const uint32_t sz = (d[0] & 0xF0) == 0xD0 ? 2u : 3u;
            eng[0]->midi(d, sz);
            if (layerOn) eng[1]->midi(d, sz);
            shared->uiMidiR.store(r + 1, std::memory_order_release);
        }
        for (uint32_t i = 0; i < midiEventCount; ++i)
        {
            const MidiEvent& ev = midiEvents[i];
            const uint32_t at = ev.frame < frames ? ev.frame : frames;
            if (at > pos) { renderSegment(L, R, pos, at - pos, layerOn); pos = at; }
            const uint8_t* data = ev.size > MidiEvent::kDataSize ? ev.dataExt : ev.data;
            if (data == nullptr) continue;
            if (ev.size >= 3 && (data[0] & 0xF0) == 0xB0) midiLearnCC(data[1], data[2]);
            // 5.1.1: bank select (CC0 MSB / CC32 LSB) + program change: preset = (bank * 128 + program)
            if (ev.size >= 3 && (data[0] & 0xF0) == 0xB0 && data[1] == 0) bankMsb = data[2];
            if (ev.size >= 3 && (data[0] & 0xF0) == 0xB0 && data[1] == 32) bankLsb = data[2];
            if (ev.size >= 2 && (data[0] & 0xF0) == 0xC0) { pendingPc = ((bankMsb << 7 | bankLsb) << 7) | data[1]; continue; }
            if (data[0] == 0xF4 && ev.size == 12) shared->neSeen.fetch_add(1, std::memory_order_relaxed);
            if (ev.size >= 3 && (data[0] & 0xF0) == 0x90 && data[2] > 0 && values[pTnSource] >= 2.5f && mts->filtered(data[1])) continue;   // MTS-ESP: unmapped key
            eng[0]->midi(data, ev.size);
            if (layerOn) eng[1]->midi(data, ev.size);
        }
        if (pos < frames) renderSegment(L, R, pos, frames - pos, layerOn);
        safetyLimit(L, R, frames);
        recordCapture(L, R, frames);
       #ifdef DRIFT_FTZ
        _mm_setcsr(oldCsr);
       #endif
       #ifdef DRIFT_FTZ_ARM
        driftFpcrSet(oldFpcr);
       #endif
        const double used = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        const double budget = frames / getSampleRate();
        const float load = (budget > 0.0 ? (float)(used / budget) : 0.f) * (guardTestScale > 0.f ? guardTestScale : 1.f);
        cpuS += (load - cpuS) * (load > cpuS ? 0.3f : 0.03f);
        shared->tel.cpu = cpuS; shared->tel2.cpu = cpuS;
        runGuard(load, (float)budget);
        publishMts((float)budget);
    }

private:
    std::unique_ptr<drift::IEngine> eng[2];      // layer 1, layer 2
    static constexpr uint32_t kLayerChunk = 256;
    std::vector<float> layerBuf;                 // layer 2 render buffer (L then R)
    bool layerWasOn = false;
    // 5.1.1: output safety limiter. Transparent below -1 dBFS; above it the gain follows the peak with a
    // 1 ms attack and 150 ms release; a final soft knee keeps every sample under 0 dBFS.
    float limEnv = 0.f;
    void safetyLimit(float* L, float* R, uint32_t n)
    {
        const float thr = 0.8912509f;                      // -1 dBFS
        const float att = 1.f - std::exp(-1.f / (0.001f * (float)getSampleRate()));
        const float rel = 1.f - std::exp(-1.f / (0.150f * (float)getSampleRate()));
        float grMax = 0.f;
        for (uint32_t i = 0; i < n; ++i)
        {
            float pk = std::max(std::fabs(L[i]), std::fabs(R[i]));
            if (!(pk < 1e9f)) { L[i] = R[i] = 0.f; pk = 0.f; }   // NaN / inf guard
            limEnv += (pk - limEnv) * (pk > limEnv ? att : rel);
            const float g = limEnv > thr ? thr / limEnv : 1.f;
            float l = L[i] * g, r = R[i] * g;
            // soft knee above the threshold catches what the 1 ms attack lets through
            auto knee = [thr](float x) { const float a = std::fabs(x); if (a <= thr) return x; const float o = thr + (1.f - thr) * std::tanh((a - thr) / (1.f - thr)); return x < 0.f ? -o : o; };
            L[i] = knee(l); R[i] = knee(r);
            if (g < 1.f) grMax = std::max(grMax, -20.f * std::log10(g));
        }
        shared->limGr.store(grMax, std::memory_order_relaxed);
    }
    int bankMsb = 0, bankLsb = 0, pendingPc = -1;
    void applyProgramChange(int idx)
    {
        if (idx < 0 || idx >= (int)kFactoryPresetCount) return;
        float before[kParamCount];
        std::memcpy(before, values, sizeof(before));
        loadProgramInternal((uint32_t)idx, true);
        for (uint32_t i = 0; i < kParamCount; ++i) if (values[i] != before[i]) requestParameterValueChange(i, values[i]);
        shared->pcIndex.store(idx, std::memory_order_relaxed);
        shared->pcSerial.fetch_add(1, std::memory_order_release);
    }
    void renderSegment(float* L, float* R, uint32_t pos, uint32_t n, bool layerOn)
    {
        eng[0]->process(L + pos, R + pos, n);
        if (!layerOn) return;
        float* bl = layerBuf.data(); float* br = bl + kLayerChunk;
        for (uint32_t off = 0; off < n; off += kLayerChunk)
        {
            const uint32_t c = n - off < kLayerChunk ? n - off : kLayerChunk;
            eng[1]->process(bl, br, c);
            for (uint32_t i = 0; i < c; ++i) { L[pos + off + i] += bl[i]; R[pos + off + i] += br[i]; }
        }
    }
    // ---- resample (4.1): the last 30 s of output, read by the editor's CAPTURE --------------------
    std::unique_ptr<drift::CaptureRing> captureRing;
    void allocCapture(double sr)
    {
        shared->capture.store(nullptr);
        std::unique_ptr<drift::CaptureRing> r(new drift::CaptureRing());
        r->sr = (float)sr;
        r->size = (int)(sr * 30.0) + 1;
        r->data.assign((size_t)r->size * 2, 0.f);
        // the editor may still be copying from the old ring for a moment: keep it alive one swap longer
        oldRing = std::move(captureRing);
        captureRing = std::move(r);
        shared->capture.store(captureRing.get());
    }
    std::unique_ptr<drift::CaptureRing> oldRing;
    void recordCapture(const float* L, const float* R, uint32_t n)
    {
        drift::CaptureRing* r = captureRing.get();
        if (!r || r->size <= 0) return;
        int w = r->write.load(std::memory_order_relaxed);
        float* d = r->data.data();
        for (uint32_t i = 0; i < n; ++i) { d[2 * w] = L[i]; d[2 * w + 1] = R[i]; if (++w >= r->size) w = 0; }
        r->write.store(w, std::memory_order_release);
        r->filled.store(r->filled.load(std::memory_order_relaxed) + (int)n > r->size ? r->size : r->filled.load(std::memory_order_relaxed) + (int)n, std::memory_order_release);
    }
    std::unique_ptr<drift::SharedBlock> shared;
    std::unique_ptr<FileLoader> loader;
    // 4.2 multicore: one pool shared by both layer engines (they render one after the other)
    std::unique_ptr<drift::WorkerPool> pool;
    std::string threadsSetting;
    void setThreads(const char* v)
    {
        threadsSetting = v != nullptr ? v : "";
        const int n = drift::poolThreadsFor(threadsSetting.c_str());
        shared->threadsActive.store(n);
        if (pool && pool->workers() == n - 1) return;
        // the host calls setState off the audio thread but may be processing: detach first, then swap
        for (auto& e : eng) e->setWorkerPool(nullptr);
        { std::lock_guard<std::mutex> g(runLock); pool.reset(n > 1 ? new drift::WorkerPool(n - 1) : nullptr); }
        for (auto& e : eng) e->setWorkerPool(pool.get());
    }
    std::mutex runLock;                         // held by run() and by a pool swap
    std::unique_ptr<drift::MotionData> motionTmp { new drift::MotionData() };
    std::unique_ptr<MtsProvider> mts;
    float mtsTimer = 0.f;
    // editor status: MTS-ESP connection, scale name and per-key pitches, a few times a second
    void publishMts(float dt)
    {
        mtsTimer += dt;
        if (mtsTimer < 0.25f) return;
        mtsTimer = 0.f;
        const bool on = mts->connected();
        shared->mtsConnected.store(on ? 1 : 0, std::memory_order_relaxed);
        if (!on) return;
        shared->mtsSeq.fetch_add(1, std::memory_order_acq_rel);
        const char* nm = mts->scaleName();
        std::strncpy(shared->mtsScaleName, nm != nullptr ? nm : "", sizeof(shared->mtsScaleName) - 1);
        for (int k = 0; k < 128; ++k) shared->mtsPitch[k] = (float)mts->noteSemis(k, -1);
        shared->mtsSeq.fetch_add(1, std::memory_order_acq_rel);
    }
    void applyScala(int layer)
    {
        const int o = layer * kLayerStates;
        drift::IEngine* const engine = eng[layer].get();
        const std::string scl = drift::tnUnescape(stateStr[o + skScl]), kbm = drift::tnUnescape(stateStr[o + skKbm]);
        if (scl.empty()) { engine->setUserScale(nullptr, nullptr); shared->sclError.store(0); return; }
        drift::ScaleDef sc; drift::KbmDef km;
        if (!drift::parseScl(scl.c_str(), sc)) { engine->setUserScale(nullptr, nullptr); shared->sclError.store(1); return; }
        const bool hasK = !kbm.empty() && drift::parseKbm(kbm.c_str(), km);
        shared->sclError.store(0);
        engine->setUserScale(&sc, hasK ? &km : nullptr);
    }
    bool restoring = false, sawNewFormat = false;
    int guardCapCur = drift::kTelVoices, guardOver = 0, guardHold = 0;
    float guardCalm = 0.f, guardTestScale = 0.f;
    std::atomic<bool> processOffline { false };   // VST3 offline processing (bounce / export)

    void setEnvModel(bool classic)
    {
        shared->envClassic.store(classic ? 1 : 0);
        for (auto& e : eng) e->setEnvClassic(classic);
    }

    void readUserDefaults()
    {
        {   // Setup page option shared with the editor
            std::ifstream in(drift::lib::fromU8(driftDocFile("options-v3.txt")));
            std::string k; int v;
            while (in >> k >> v)
            {
                if (k == "cpuGuard") shared->guardOn.store(v != 0 ? 1 : 0);
                if (k == "threads") threadsSetting = v <= 1 ? "" : (v >= 99 ? "auto" : std::to_string(v));   // default for new instances
            }
        }
        {   // default MIDI learn map for new instances
            std::ifstream in(drift::lib::fromU8(driftDocFile("midi-learn-v3.txt")));
            std::string line;
            if (std::getline(in, line)) { int mm[drift::kMidiMapSize]; if (drift::midiMapFromString(line.c_str(), mm)) shared->setMap(mm); }
        }
    }

    static float paramFromNorm(uint32_t p, float n)
    {
        const ParamDef& d = P(p);
        n = n < 0.f ? 0.f : (n > 1.f ? 1.f : n);
        float v = (d.flags & fLog) ? d.min * std::pow(d.max / d.min, n) : d.min + n * (d.max - d.min);
        if (d.flags & (fInt | fBool)) v = std::round(v);
        return v;
    }

    void midiLearnCC(int cc, int val)
    {
        if (!drift::ccLearnable(cc)) return;
        const int arm = shared->learnArm.load(std::memory_order_acquire);
        if (arm >= 0 && arm < (int)kParamCount)
        {
            for (int c = 0; c < drift::kMidiMapSize; ++c) if (shared->midiMap[c].load(std::memory_order_relaxed) == arm) shared->midiMap[c].store(-1);
            shared->midiMap[cc].store(arm, std::memory_order_release);
            shared->learnArm.store(-1, std::memory_order_release);
        }
        const int p = shared->midiMap[cc].load(std::memory_order_acquire);
        if (p < 0 || p >= (int)kParamCount) return;
        const float v = paramFromNorm((uint32_t)p, (float)val / 127.f);
        setValue((uint32_t)p, v);
        requestParameterValueChange((uint32_t)p, values[p]);
        shared->ccValue[cc].store(values[p], std::memory_order_relaxed);
        shared->ccSeq.fetch_add(1, std::memory_order_release);
    }

    // CPU guard (VST3 real-time processing only): when the block load stays above 90 %, cap the
    // voice count one below what is playing so the quietest voices fade out instead of the host
    // dropping out; the cap relaxes one voice per second once the load is back under 60 %.
    void runGuard(float load, float budgetSec)
    {
        const bool on = shared->guardOn.load(std::memory_order_relaxed) && shared->guardRealtime.load(std::memory_order_relaxed);
        if (!on)
        {
            if (guardCapCur < drift::kTelVoices) { guardCapCur = drift::kTelVoices; for (auto& e : eng) e->setGuardCap(guardCapCur); shared->guardCap.store(0); }
            guardOver = 0; guardCalm = 0.f;
            return;
        }
        if (guardHold > 0) --guardHold;
        if (load > 0.9f)
        {
            guardCalm = 0.f;
            if (++guardOver >= 3 && guardHold == 0)
            {
                guardOver = 0;
                const int active = eng[0]->activeVoices() > eng[1]->activeVoices() ? eng[0]->activeVoices() : eng[1]->activeVoices();
                int cap = active - 1 < 2 ? 2 : active - 1;
                if (cap < guardCapCur) { guardCapCur = cap; for (auto& e : eng) e->setGuardCap(cap); guardHold = 8; }   // let the fade land before judging again
            }
        }
        else
        {
            guardOver = 0;
            if (guardCapCur < drift::kTelVoices && cpuS < 0.6f)
            {
                guardCalm += budgetSec;
                if (guardCalm > 1.f)
                {
                    guardCalm = 0.f;
                    ++guardCapCur;
                    if (guardCapCur >= (int)values[pPolyphony]) guardCapCur = drift::kTelVoices;
                    for (auto& e : eng) e->setGuardCap(guardCapCur);
                }
            }
        }
        shared->guardCap.store(guardCapCur < drift::kTelVoices ? guardCapCur : 0, std::memory_order_relaxed);
    }
    float values[kParamCount] = {};
    std::string stateStr[kStateCount];
    float cpuS = 0.f;

    void applyState(int kAll)
    {
        const std::string& v = stateStr[kAll];
        const int layer = kAll / kLayerStates, k = kAll % kLayerStates;
        drift::IEngine* const engine = eng[layer].get();
        if (k >= skWtA && k <= skWtC)
        {
            loader->request(layer * 12 + k - skWtA, v);
        }
        else if (k >= skSpA && k <= skSpC)
        {
            float amps[drift::kPartials];
            if (!drift::spectrumFromString(v.c_str(), amps)) drift::spectrumDefault(amps);
            engine->setCustomSpectrum(k - skSpA, amps);
        }
        else if (k >= skSp2A && k <= skSp2C)   // 6.0: Custom 2
        {
            float amps[drift::kPartials];
            if (!drift::spectrumFromString(v.c_str(), amps)) drift::spectrumDefault(amps);
            engine->setCustomSpectrum(3 + k - skSp2A, amps);
        }
        else if (k >= skSmA && k <= skSmC)
        {
            loader->request(layer * 12 + 3 + k - skSmA, v);
        }
        else if (k >= skLfo1 && k <= skLfo6)
        {
            float pts[drift::kLfoShapePoints];
            if (!drift::lfoShapeFromString(v.c_str(), pts)) drift::lfoShapeDefault(pts, k - skLfo1);
            engine->setLfoShape(k - skLfo1, pts);
        }
        else if (k == skMseg1 || k == skMseg2 || k == skMseg3 || k == skMseg4)
        {
            const int mi = k <= skMseg2 ? k - skMseg1 : 2 + k - skMseg3;
            drift::MsegData m;
            if (!drift::msegFromString(v.c_str(), m)) drift::msegDefault(m, mi);
            engine->setMseg(mi, m);
        }
        else if (k >= skSrcA && k <= skSrcC) loader->request(layer * 12 + 6 + k - skSrcA, v);
        else if (k >= skXsrcA && k <= skXsrcC) loader->request(layer * 12 + 9 + k - skXsrcA, v);
        else if (k >= skModesA && k <= skModesC)
        {
            drift::ModalData md;
            if (!drift::modalFromString(v.c_str(), md)) drift::modalDefault(md);
            engine->setModes(k - skModesA, md);
        }
        else if (k == skScenes)
        {
            drift::SceneData sd;
            drift::scenesFromString(v.c_str(), sd);
            engine->setScenes(sd);
        }
        else if (k == skPerf)
        {   // 5.1.1: the XY pads' macro axes drive the auto-motion in the engine
            int a[4] = { 0, 1, 2, 3 };
            if (std::sscanf(v.c_str(), "%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3]) != 4) { a[0] = 0; a[1] = 1; a[2] = 2; a[3] = 3; }
            engine->setPerfAxes(a);
        }
        else if (k == skMotion)
        {
            drift::motionFromString(v.c_str(), *motionTmp);
            engine->setMotion(*motionTmp);
        }
        else if (k == skScl || k == skKbm) applyScala(layer);
        else if (k == skFxOrder)
        {
            int o[kFxUnits];
            if (!drift::fxOrderFromString(v.c_str(), o)) drift::fxOrderDefault(o);
            engine->setFxOrder(o);
        }
    }

    void loadProgramInternal(uint32_t index, bool notify)
    {
        if (index >= kFactoryPresetCount) return;
        float pv[kParamCount];
        presetValues(index, pv);
        for (uint32_t i = 0; i < kParamCount; ++i) setValue(i, pv[i]);
        std::string st[kStateCount];
        presetStates(index, st);
        for (int k = 0; k < kStateCount; ++k)
        {
            const bool changed = stateStr[k] != st[k] || !notify;
            stateStr[k] = st[k];
            if (changed) applyState(k);
            if (notify) updateStateValue(kStateKeys[k], st[k].c_str());
        }
        presetId = std::string("F:") + kFactoryPresets[index].category + "/" + kFactoryPresets[index].name;
        if (notify) updateStateValue("presetId", presetId.c_str());
        setEnvModel(false);
        if (notify) updateStateValue("envModel", "");
        layerNames.clear();
        for (const PresetState* q = kFactoryPresets[index].st; q && q->key; ++q) if (std::strcmp(q->key, "layerNames") == 0) layerNames = q->value;
        if (notify) updateStateValue("layerNames", layerNames.c_str());
    }

    void setValue(uint32_t i, float v)
    {
        const ParamDef& d = P(i);
        v = v < d.min ? d.min : (v > d.max ? d.max : v);
        values[i] = v;
        uint32_t e; const int ly = hostToEngine(i, e);
        if (ly < 0) { eng[0]->setParam(e, v); eng[1]->setParam(e, v); }
        else eng[ly]->setParam(e, v);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DriftfieldPlugin)
};

Plugin* createPlugin()
{
    return new DriftfieldPlugin();
}

END_NAMESPACE_DISTRHO

drift::SharedBlock* driftSharedFromInstance(void* inst)
{
    if (inst == nullptr) return nullptr;
    return static_cast<DISTRHO_NAMESPACE::DriftfieldPlugin*>(static_cast<DISTRHO_NAMESPACE::Plugin*>(inst))->sharedBlock();
}
