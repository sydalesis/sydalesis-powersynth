// Driftfield 5.1 checks: the compact factory-preset format decodes to exactly what the generator wrote
// (every setting of every preset, every state), and keyframed motion lanes match the original frames.
// usage: v51 ref51.txt   (written from gen_presets3.py: V name sym value / S name key value)
#include "../Params.hpp"
#include "../Presets.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <cmath>
#include <map>
#include <fstream>
#include <sstream>

static int fails = 0;
#define CHECK(c, ...) do { if (c) { std::printf("ok:   " __VA_ARGS__); std::printf("\n"); } else { std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); ++fails; } } while (0)

static std::string unesc(const std::string& s) {
    std::string o; o.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) { const char c = s[++i]; o += c == 'n' ? '\n' : (c == 't' ? '\t' : c); }
        else o += s[i];
    }
    return o;
}

int main(int argc, char** argv) {
    if (argc < 2) { std::printf("usage: v51 ref51.txt\n"); return 2; }
    std::map<std::string, uint32_t> idx;
    for (uint32_t i = 0; i < kFactoryPresetCount; ++i) idx[kFactoryPresets[i].name] = i;
    CHECK(kFactoryPresetCount == 6050 && idx.size() == kFactoryPresetCount, "%u factory presets, names unique", kFactoryPresetCount);

    std::ifstream in(argv[1]);
    std::string ln;
    std::vector<float> pv(kParamCount);
    std::vector<std::string> st(kStateCount);
    long nv = 0, badv = 0, ns = 0, bads = 0, nmo = 0, badmo = 0, unknown = 0;
    double worst = 0;
    uint32_t cur = 0xffffffffu;
    static drift::MotionData ma, mb;
    while (std::getline(in, ln)) {
        std::istringstream ss(ln);
        std::string kind, name, key, val;
        std::getline(ss, kind, '\t'); std::getline(ss, name, '\t'); std::getline(ss, key, '\t'); std::getline(ss, val);
        auto it = idx.find(name);
        if (it == idx.end()) { if (unknown++ < 3) std::printf("  unknown preset %s\n", name.c_str()); continue; }
        if (it->second != cur) { cur = it->second; presetValues(cur, pv.data()); presetStates(cur, st.data()); }
        if (kind == "V") {
            const int id = ParamRegistry::get().findSymbol(key.c_str());
            if (id < 0) { if (unknown++ < 3) std::printf("  unknown param %s\n", key.c_str()); continue; }
            ++nv;
            if (pv[(size_t)id] != (float)std::atof(val.c_str())) { if (badv++ < 5) std::printf("  %s %s %g != %s\n", name.c_str(), key.c_str(), pv[(size_t)id], val.c_str()); }
        } else {
            if (key == "layerNames") {      // browser metadata, read straight from the table
                ++ns; const char* got = nullptr;
                for (const PresetState* q = kFactoryPresets[cur].st; q->key; ++q) if (key == q->key) got = q->value;
                if ((!got || unesc(val) != got) && bads++ < 5) std::printf("  layerNames differs: %s\n", name.c_str());
                continue;
            }
            const int k = stateIndexOf(key.c_str());
            if (k < 0) { if (unknown++ < 6) std::printf("  unknown state %s (%s)\n", key.c_str(), name.c_str()); continue; }
            const std::string ref = unesc(val);
            if (key == "motion" || key == "b_motion") {
                ++nmo;
                drift::motionFromString(ref.c_str(), ma); drift::motionFromString(st[(size_t)k].c_str(), mb);
                bool ok = ma.frames == mb.frames && ma.seconds == mb.seconds;
                for (int m = 0; ok && m < kMacros; ++m) {
                    if (ma.has[m] != mb.has[m]) { ok = false; break; }
                    if (!ma.has[m]) continue;
                    for (int i = 0; i < ma.frames; ++i) { const double e = std::fabs(ma.v[m][i] - mb.v[m][i]); worst = std::max(worst, e); if (e > 0.0045) ok = false; }
                }
                if (!ok && badmo++ < 5) std::printf("  motion differs: %s\n", name.c_str());
            } else {
                ++ns;
                if (st[(size_t)k] != ref && bads++ < 5) std::printf("  state %s/%s differs\n", name.c_str(), key.c_str());
            }
        }
    }
    CHECK(unknown == 0, "every reference row maps to a preset / parameter / state (%ld unknown)", unknown);
    CHECK(nv > 1000000 && badv == 0, "%ld settings decode exactly (%ld wrong)", nv, badv);
    CHECK(ns > 10000 && bads == 0, "%ld states identical (%ld wrong)", ns, bads);
    CHECK(nmo > 500 && badmo == 0 && worst <= 0.0045, "%ld motion lanes match frame by frame (worst %.4f)", nmo, worst);

    // keyframe parser edge cases
    drift::motionFromString("1.000;0:~10:0,1000,0,500,250,1000", ma);     // 50 frames, keys at 0,10,20,30,40, last at 49
    CHECK(ma.frames == 50 && std::fabs(ma.v[0][5] - 0.5f) < 1e-6f && std::fabs(ma.v[0][10] - 1.f) < 1e-6f && ma.v[0][49] == 1.f
          && std::fabs(ma.v[0][45] - (0.25f + 0.75f * 5.f / 9.f)) < 1e-5f, "keyframe lanes interpolate, last key on the last frame");
    const std::string back = drift::motionToString(ma);
    drift::motionFromString(back.c_str(), mb);
    bool same = true; for (int i = 0; i < 50; ++i) if (std::fabs(ma.v[0][i] - mb.v[0][i]) > 0.0006f) same = false;
    CHECK(back.find('~') == std::string::npos && same, "saving writes the frame-by-frame form (readable by 4.2 / 5.0)");

    std::printf(fails ? "%d FAILURES\n" : "all good\n", fails);
    return fails ? 1 : 0;
}
