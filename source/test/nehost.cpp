// Minimal VST3 host for the note-expression patch: loads Driftfield3.vst3 (Linux build), checks the
// INoteExpressionController answers, then plays a note with an id and sends Tuning / Volume
// expressions for that id, measuring the output pitch and level.
// build: g++ -std=gnu++17 -I../../dpf/distrho/src nehost.cpp -ldl -o nehost
#include "travesty/base.h"
#include "travesty/factory.h"
#include "travesty/component.h"
#include "travesty/audio_processor.h"
#include "travesty/events.h"
#include "travesty/edit_controller.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { ++bad; std::printf("FAIL: "); } else std::printf("ok:   "); std::printf(__VA_ARGS__); std::printf("\n"); } while (0)

// ---- event list -----------------------------------------------------------------------------
struct EvList {
    v3_event_list_cpp* vt;
    v3_event_list_cpp table;
    std::vector<v3_event> ev;
    EvList() {
        vt = &table;
        table.query_interface = [](void*, const v3_tuid, void** o) -> v3_result { *o = nullptr; return V3_NO_INTERFACE; };
        table.ref = [](void*) -> uint32_t { return 1; };
        table.unref = [](void*) -> uint32_t { return 1; };
        table.list.get_event_count = [](void* self) -> uint32_t { return (uint32_t)static_cast<EvList*>(self)->ev.size(); };
        table.list.get_event = [](void* self, int32_t i, v3_event* e) -> v3_result { auto* l = static_cast<EvList*>(self); if (i < 0 || i >= (int)l->ev.size()) return V3_INVALID_ARG; *e = l->ev[(size_t)i]; return V3_OK; };
        table.list.add_event = [](void*, v3_event*) -> v3_result { return V3_OK; };
    }
};
struct NoteExprTypeInfo {   // mirrors the patch's v3_note_expression_type_info
    uint32_t type_id; int16_t title[128], short_title[128], units[128]; int32_t unit_id;
    double def, mn, mx; int32_t steps; uint32_t assoc; int32_t flags;
};
struct NeController {
    int32_t (V3_API* count)(void*, int32_t, int16_t);
    v3_result (V3_API* info)(void*, int32_t, int16_t, int32_t, NoteExprTypeInfo*);
    v3_result (V3_API* str)(void*, int32_t, int16_t, uint32_t, double, int16_t*);
    v3_result (V3_API* val)(void*, int32_t, int16_t, uint32_t, const int16_t*, double*);
};
struct NeControllerCpp : v3_funknown { NeController ne; };

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : "../bin-linux/Driftfield3.vst3/Contents/x86_64-linux/Driftfield3.so";
#ifdef _WIN32
    HMODULE lib = LoadLibraryA(path);
    if (!lib) { std::printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    auto entry = (bool (*)())GetProcAddress(lib, "InitDll");
    if (entry) entry();
    auto getf = (v3_plugin_factory** (*)())GetProcAddress(lib, "GetPluginFactory");
#else
    void* lib = dlopen(path, RTLD_NOW);
    if (!lib) { std::printf("dlopen: %s\n", dlerror()); return 1; }
    auto entry = (bool (*)(void*))dlsym(lib, "ModuleEntry");
    if (entry) entry(lib);
    auto getf = (v3_plugin_factory** (*)())dlsym(lib, "GetPluginFactory");
#endif
    v3_plugin_factory** f = getf();
    v3_plugin_factory_cpp* fv = (v3_plugin_factory_cpp*)*f;
    v3_class_info ci; v3_tuid compId; bool found = false;
    for (int i = 0; i < fv->v1.num_classes(f); ++i) {
        fv->v1.get_class_info(f, i, &ci);
        if (std::strcmp(ci.category, "Audio Module Class") == 0) { std::memcpy(compId, ci.class_id, sizeof(v3_tuid)); found = true; }
    }
    if (!found) { std::printf("no component class\n"); return 1; }
    void* compRaw = nullptr;
    fv->v1.create_instance(f, compId, v3_component_iid, &compRaw);
    v3_component** comp = (v3_component**)compRaw;
    v3_component_cpp* cv = (v3_component_cpp*)*comp;
    cv->base.initialize(comp, nullptr);

    // note expression interface: on the component (single-object build) or its controller
    void* neRaw = nullptr;
    {
        static const v3_tuid neIid = V3_ID(0xB7F8F859, 0x41234872, 0x91169581, 0x4F3721A3);
        cv->query_interface(comp, neIid, &neRaw);
    }
    CHECK(neRaw != nullptr, "component answers INoteExpressionController");
    if (neRaw) {
        NeControllerCpp* nv = *(NeControllerCpp**)neRaw;
        const int n = nv->ne.count(neRaw, 0, 0);
        CHECK(n == 6, "6 note-expression types");
        char names[256] = {};
        for (int i = 0; i < n; ++i) {
            NoteExprTypeInfo inf; nv->ne.info(neRaw, 0, 0, i, &inf);
            char t[32]; int k = 0; for (; k < 31 && inf.title[k]; ++k) t[k] = (char)inf.title[k]; t[k] = 0;
            std::strcat(names, t); std::strcat(names, " ");
        }
        std::printf("      types: %s\n", names);
    }

    void* apRaw = nullptr;
    cv->query_interface(comp, v3_audio_processor_iid, &apRaw);
    v3_audio_processor** ap = (v3_audio_processor**)apRaw;
    v3_audio_processor_cpp* av = (v3_audio_processor_cpp*)*ap;
    v3_process_setup setup = { 0, 0, 512, 48000.0 };
    av->proc.setup_processing(ap, &setup);
    cv->comp.activate_bus(comp, 0, 1, 0, 1);   // audio output
    cv->comp.activate_bus(comp, 1, 0, 0, 1);   // event input
    cv->comp.set_active(comp, 1);
    av->proc.set_processing(ap, 1);

    std::vector<float> L(512), R(512);
    float* chans[2] = { L.data(), R.data() };
    v3_audio_bus_buffers out; out.num_channels = 2; out.channel_silence_bitset = 0; out.channel_buffers_32 = chans;
    EvList evl; EvList* evp = &evl;
    v3_process_data pd = {};
    pd.process_mode = 0; pd.symbolic_sample_size = 0; pd.nframes = 512; pd.num_output_buses = 1; pd.outputs = &out;
    pd.input_events = (v3_event_list**)&evl; (void)evp;

    auto run = [&](int blocks, std::vector<float>* rec) {
        for (int b = 0; b < blocks; ++b) {
            av->proc.process(ap, &pd);
            evl.ev.clear();
            if (rec) rec->insert(rec->end(), L.begin(), L.end());
        }
    };
    auto measure = [&](const std::vector<float>& x, double& rms) {
        const size_t s0 = x.size() / 3, N = 8192;
        double acc = 0; for (size_t i = s0; i < x.size(); ++i) acc += (double)x[i] * x[i];
        rms = std::sqrt(acc / (x.size() - s0));
        // normalised autocorrelation, lags for 50..2000 Hz; first peak above 0.9 of the best
        std::vector<double> rr(1000, 0.0);
        double best = 0;
        for (int lag = 24; lag < 960; ++lag) {
            double num = 0, e1 = 0, e2 = 0;
            for (size_t i = s0; i < s0 + N; ++i) { num += (double)x[i] * x[i + lag]; e1 += (double)x[i] * x[i]; e2 += (double)x[i + lag] * x[i + lag]; }
            rr[lag] = num / std::sqrt(e1 * e2 + 1e-30); if (rr[lag] > best) best = rr[lag];
        }
        for (int lag = 25; lag < 959; ++lag)
            if (rr[lag] > 0.9 * best && rr[lag] >= rr[lag - 1] && rr[lag] >= rr[lag + 1]) {
                const double a1 = rr[lag - 1], b1 = rr[lag], c1 = rr[lag + 1];
                const double d = 0.5 * (a1 - c1) / (a1 - 2 * b1 + c1);
                return 48000.0 / (lag + d);
            }
        return 0.0;
    };
    v3_event e = {};
    e.type = V3_EVENT_NOTE_ON; e.note_on.channel = 0; e.note_on.pitch = 57; e.note_on.velocity = 0.8f; e.note_on.note_id = 77; e.note_on.tuning = 0.f;
    evl.ev.push_back(e);
    run(300, nullptr);   // let the Init patch's attack settle
    std::vector<float> a; run(120, &a);
    double r0; const double f0 = measure(a, r0);
    // volume -6 dB, then tuning +12 semitones for note id 77
    v3_event x = {}; x.type = V3_EVENT_NOTE_EXP_VALUE; x.note_exp_value.note_id = 77;
    x.note_exp_value.type_id = 0; x.note_exp_value.value = 0.125; evl.ev.push_back(x);
    run(1, nullptr);
    std::vector<float> bv; run(120, &bv);
    double r1; measure(bv, r1);
    x.note_exp_value.type_id = 2; x.note_exp_value.value = 0.5 + 12.0 / 240.0; evl.ev.push_back(x);
    run(1, nullptr);
    std::vector<float> b; run(120, &b);
    double rb; const double f1 = measure(b, rb);
    std::printf("      before: %.1f Hz rms %.4f   after: %.1f Hz rms %.4f\n", f0, r0, f1, r1);
    CHECK(f0 > 0 && std::fabs(1200.0 * std::log2(f1 / f0) - 1200.0) < 30.0, "note-expression tuning raised the note an octave (%.0f cents)", 1200.0 * std::log2(f1 / f0));
    CHECK(r1 < r0 * 0.7, "note-expression volume lowered the note (%.1f dB)", 20 * std::log10(r1 / r0));
    // an expression for an unknown id is ignored
    x.note_exp_value.note_id = 999; x.note_exp_value.type_id = 2; x.note_exp_value.value = 0.0; evl.ev.push_back(x);
    run(1, nullptr);
    std::vector<float> c; run(60, &c);
    double r2; const double f2 = measure(c, r2);
    CHECK(std::fabs(f2 - f1) < 3.0, "unknown note id ignored (%.1f Hz)", f2);
    // note-on with tuning (+100 cents) on a new id
    e.note_on.note_id = 78; e.note_on.pitch = 45; e.note_on.tuning = 100.f;
    v3_event off = {}; off.type = V3_EVENT_NOTE_OFF; off.note_off.channel = 0; off.note_off.pitch = 57; off.note_off.note_id = 77;
    evl.ev.push_back(off); run(800, nullptr);
    evl.ev.push_back(e); run(1, nullptr);
    std::vector<float> d; run(120, &d);
    double r3; const double f3 = measure(d, r3);
    CHECK(std::fabs(1200.0 * std::log2(f3 / (f0 / 2.0)) - 100.0) < 25.0, "note-on tuning field: +%.0f cents", 1200.0 * std::log2(f3 / (f0 / 2.0)));

    av->proc.set_processing(ap, 0);
    cv->comp.set_active(comp, 0);
    cv->base.terminate(comp);
    cv->unref(comp);
    std::printf(bad ? "\n%d FAILURES\n" : "\nall good\n", bad);
    return bad ? 1 : 0;
}
