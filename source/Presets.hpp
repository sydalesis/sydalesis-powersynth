// Driftfield 3 — factory presets + user preset files.
#pragma once
#include "Params.hpp"
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct PresetState { const char* key; const char* value; };
// 5.1: settings are (symbol index, float bits) pairs in one flat array (kPresetData) with a shared symbol
// table (kPresetSyms): 8 bytes per setting and no relocations, so ~6000 presets stay small in the binary.
struct FactoryPreset {
    const char* name;
    const char* category;
    const char* tags;          // comma separated, lower case
    uint32_t    d0, dn;        // settings: kPresetData[2*d0 .. 2*(d0+dn))
    const PresetState* st;     // terminated by { nullptr, nullptr }
};

#include "FactoryPresets.inc"

// Full parameter set for a factory preset (defaults + the preset's settings).
inline void presetValues(uint32_t index, float* out) {
    // symbol table -> parameter ids, resolved once (thread-safe static init)
    static const std::vector<int> ids = [] {
        std::vector<int> v(kPresetSymCount);
        for (uint32_t i = 0; i < kPresetSymCount; ++i) v[i] = ParamRegistry::get().findSymbol(kPresetSyms[i]);
        return v;
    }();
    for (uint32_t i = 0; i < kParamCount; ++i) out[i] = P(i).def;
    if (index >= kFactoryPresetCount) return;
    const FactoryPreset& fp = kFactoryPresets[index];
    for (uint32_t k = 0; k < fp.dn; ++k) {
        const uint32_t* e = kPresetData + 2 * (size_t)(fp.d0 + k);
        const int id = e[0] < kPresetSymCount ? ids[e[0]] : -1;
        if (id < 0) continue;
        float v; std::memcpy(&v, &e[1], sizeof v);
        out[id] = v;
    }
}

inline void presetStates(uint32_t index, std::string* states /* [kStateCount] */) {
    for (int i = 0; i < kStateCount; ++i) states[i].clear();
    if (index >= kFactoryPresetCount) return;
    for (const PresetState* s = kFactoryPresets[index].st; s->key; ++s) {
        const int k = stateIndexOf(s->key);
        if (k >= 0) states[k] = s->value;
    }
}

// ---- user preset text format ------------------------------------------------------------------
//   driftfield3-preset 1
//   name <text> / category <text> / tags <text>
//   <symbol> <value>          (one per parameter)
//   @<stateKey> <value>       (empty values omitted)
inline std::string presetToText(const std::string& name, const std::string& category, const std::string& tags,
                                const float* values, const std::string* states) {
    std::string s = "driftfield3-preset 1\nname " + name + "\ncategory " + category + "\ntags " + tags + "\n";
    char buf[128];
    for (uint32_t i = 0; i < kParamCount; ++i) {
        std::snprintf(buf, sizeof(buf), "%s %.6g\n", P(i).symbol, values[i]);
        s += buf;
    }
    for (int k = 0; k < kStateCount; ++k) if (!states[k].empty()) s += std::string("@") + kStateKeys[k] + " " + states[k] + "\n";
    return s;
}

inline bool presetFromText(const std::string& text, float* values, std::string* states, std::string* name, std::string* category, std::string* tags) {
    if (text.rfind("driftfield3-preset", 0) != 0) return false;
    for (uint32_t i = 0; i < kParamCount; ++i) values[i] = P(i).def;
    for (int k = 0; k < kStateCount; ++k) states[k].clear();
    size_t pos = 0;
    while (pos < text.size()) {
        size_t e = text.find('\n', pos);
        if (e == std::string::npos) e = text.size();
        std::string line = text.substr(pos, e - pos);
        pos = e + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const size_t sp = line.find(' ');
        if (sp == std::string::npos) continue;
        const std::string key = line.substr(0, sp), val = line.substr(sp + 1);
        if (key == "name") { if (name) *name = val; continue; }
        if (key == "category") { if (category) *category = val; continue; }
        if (key == "tags") { if (tags) *tags = val; continue; }
        if (key[0] == '@') {
            const int k = stateIndexOf(key.c_str() + 1);
            if (k >= 0) states[k] = val;
            continue;
        }
        const int id = ParamRegistry::get().findSymbol(key);
        if (id >= 0) {
            const ParamDef& d = P((uint32_t)id);
            float v = (float)std::atof(val.c_str());
            values[id] = v < d.min ? d.min : (v > d.max ? d.max : v);
        }
    }
    return true;
}
