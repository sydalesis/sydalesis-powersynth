// ===================================================================================== LAYERS page (4.1)
// (included inside class DriftfieldUI)
// Layer setup (mode, split, crossfade width, morph), layer management (edit, copy, swap, load a preset
// into layer 2), RESAMPLE (capture the plug-in's output into a grain sample) and ANALYSE (a WAV into
// additive partials or a wavetable).

    Rect lyCardSetup, lyCardLayers, lyCardResample, lyCardAnalyse;
    Rect lyMap;                              // gain map display
    Rect lyEditBtn[2], lyCopy12, lyCopy21, lySwap, lyClear2, lyLoad2, lyLayerSwitch[2];
    Rect lyFader[2], lyMeterR[2];            // 4.2 layer mix: each layer's Master, both always on screen
    float lyMeter[2] = { 0.f, 0.f };
    Rect rsLenBtn[5], rsOscBtn[3], rsCapture, rsView, rsAsBtn[2];
    int rsAs = 0;   // 5.0: capture into the grain sample (0) or the spectral source (1)
    Rect anOscBtn[3], anPartials, anTable, anModes, anSrcBtn[5];
    int anSrc = 0;   // 5.1.1: 0 = a WAV file, 1..4 = the last 0.5 / 1 / 2 / 4 s of Driftfield's output
    static constexpr float kAnLens[5] = { 0.f, 0.5f, 1.f, 2.f, 4.f };
    bool anHaveModes = false; drift::ModalData anModesD;
    int rsLen = 2, rsOsc = 0, anOsc = 0;
    std::string rsLast, anLast; bool anLastOk = true;
    float anAmps[128] = {}; bool anHaveAmps = false;
    std::string layerNames[2];
    static constexpr float kRsLens[5] = { 2.f, 4.f, 8.f, 15.f, 30.f };

    void buildLayers() {
        lyCardSetup = { 16, 72, 760, 430 };
        lyCardLayers = { 788, 72, 796, 430 };
        lyCardResample = { 16, 514, 760, 470 };
        lyCardAnalyse = { 788, 514, 796, 470 };
        panel(lyCardSetup, "LAYER SETUP", pgLayer);
        panel(lyCardLayers, "LAYERS", pgLayer);
        panel(lyCardResample, "RESAMPLE  ·  OUTPUT INTO A GRAIN SAMPLE", pgLayer);
        panel(lyCardAnalyse, "ANALYSE  ·  A WAV INTO PARTIALS, A WAVETABLE OR MODES", pgLayer);
        choice(pLyMode, { 32, 108, 728, 30 }, 6, pgLayer);
        knob(pLySplit, 92, 210, 22, "SPLIT KEY", pgLayer);
        knob(pLyWidth, 200, 210, 22, "XFADE WIDTH", pgLayer);
        knob(pLyMorph, 308, 210, 22, "MORPH / CENTRE", pgLayer);
        lyMap = { 32, 300, 728, 186 };
        for (int l = 0; l < 2; ++l) lyEditBtn[l] = { 804.f, 140.f + l * 72.f, 180, 30 };
        lyLoad2  = { 804, 272, 372, 30 };
        lyCopy12 = { 804, 360, 180, 30 };
        lyCopy21 = { 996, 360, 180, 30 };
        lySwap   = { 1188, 360, 180, 30 };
        lyClear2 = { 1380, 360, 180, 30 };
        for (int l = 0; l < 2; ++l) { lyFader[l] = { 870, 428.f + l * 32.f, 380, 22 }; lyMeterR[l] = { 1334, 432.f + l * 32.f, 226, 14 }; }
        for (int i = 0; i < 5; ++i) rsLenBtn[i] = { 32.f + i * 84.f, 576, 78, 28 };
        for (int o = 0; o < 3; ++o) rsOscBtn[o] = { 480.f + o * 92.f, 576, 86, 28 };
        rsCapture = { 32, 628, 560, 40 };
        rsAsBtn[0] = { 600, 628, 78, 40 }; rsAsBtn[1] = { 682, 628, 78, 40 };
        rsView = { 32, 684, 728, 220 };
        for (int o = 0; o < 3; ++o) anOscBtn[o] = { 804.f + o * 92.f, 576, 86, 28 };
        anSrcBtn[0] = { 1130, 576, 96, 28 };
        for (int i = 1; i < 5; ++i) anSrcBtn[i] = { 1232.f + (i - 1) * 84.f, 576, 78, 28 };
        anPartials = { 804, 628, 248, 40 };
        anTable = { 1062, 628, 248, 40 };
        anModes = { 1320, 628, 248, 40 };
        lyLayerSwitch[0] = { 120, 37, 30, 15 };
        lyLayerSwitch[1] = { 154, 37, 30, 15 };
    }

    int lyMode() const { return std::max(0, std::min(5, (int)std::lround(values[pLyMode]))); }
    static const char* noteName(int k) { static char b[8]; static const char* const n[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }; std::snprintf(b, sizeof b, "%s%d", n[(k % 12 + 12) % 12], k / 12 - 1); return b; }
    // the engine's crossfade position, mirrored for the display (0 = layer 1, 1 = layer 2)
    float lyPos(float x) const {
        const float w = values[pLyWidth], m = values[pLyMorph];
        switch (lyMode()) {
        case 2: if (w < 0.01f) return x >= values[pLySplit] ? 1.f : 0.f; return std::max(0.f, std::min(1.f, (x - values[pLySplit] + 0.5f) / w + 0.5f));
        case 3: case 4: { const float ww = w / 48.f + 0.02f; return std::max(0.f, std::min(1.f, (x - m) / ww + 0.5f)); }
        case 5: return m;
        default: return 0.5f;
        }
    }

    // ---- header switch: which layer the pages edit --------------------------------------------
    void drawLayerSwitch() {
        label(20, 44.5f, "LAYER", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        const bool off = lyMode() == 0;
        for (int l = 0; l < 2; ++l) {
            const Rect& r = lyLayerSwitch[l];
            const bool sel = editLayer == l, hot = hovered(r);
            beginPath(); roundedRect(r.x, r.y, r.w, r.h, 3);
            fillColor(sel ? tint(l ? cViolet : cAccent, 0.3f) : (hot ? shade(36.f) : cInset)); fill();
            strokeColor(sel ? (l ? cViolet : cAccent) : cPanelEdge); strokeWidth(1); stroke();
            label(r.cx(), r.cy() + 0.5f, l ? "L2" : "L1", 8.f, sel ? cText : cDim, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        if (editLayer == 1) {
            // a violet edge marks that every page now edits layer 2
            beginPath(); rect(0, 0, kW, 3); fillColor(cViolet); fill();
            label(192, 44.5f, off ? "layer 2 is off" : "editing layer 2", 7.5f, off ? cWarn : cViolet, ALIGN_LEFT | ALIGN_MIDDLE);
        }
    }
    // ---- 4.2 layer mix ---------------------------------------------------------------------------
    static float masterNorm(float db) { return (db + 36.f) / 48.f; }
    void drawLayerMix() {
        letterSp(1.5f);
        label(804, 414, "LAYER MIX", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
        const int mode = lyMode();
        for (int l = 0; l < 2; ++l) {
            const Rect& f = lyFader[l];
            const Color col = l ? cViolet : cAccent;
            const float db = hostVals[pMaster + (l ? kLayerBOffset : 0)];
            label(f.x - 58, f.cy(), l ? "LAYER 2" : "LAYER 1", 9.f, l == 1 && mode == 0 ? cFaint : col, ALIGN_LEFT | ALIGN_MIDDLE);
            beginPath(); roundedRect(f.x, f.cy() - 3, f.w, 6, 3); fillColor(cInset); fill();
            const float x = f.x + f.w * masterNorm(db);
            beginPath(); roundedRect(f.x, f.cy() - 3, x - f.x, 6, 3); fillColor(withA(col, l == 1 && mode == 0 ? 0.25f : 0.7f)); fill();
            const float x0 = f.x + f.w * masterNorm(0.f);
            beginPath(); moveTo(x0, f.y + 2); lineTo(x0, f.y + f.h - 2); strokeColor(cFaint); strokeWidth(1); stroke();
            beginPath(); roundedRect(x - 6, f.y + 1, 12, f.h - 2, 3); fillColor((sd == sdLyFader && sdIndex == l) || hovered(f) ? cText : cKnobHi); fill(); strokeColor(col); stroke();
            char b[32]; std::snprintf(b, sizeof b, "%+.1f dB", db);
            label(f.x + f.w + 8, f.cy(), b, 9.f, cText, ALIGN_LEFT | ALIGN_MIDDLE);
            // meter: the layer's own output peak (after its Master and FX)
            float pk = 0.f;
            if (shared) { const drift::Telemetry& t = l ? shared->tel2 : shared->tel; pk = std::max(t.peakL, t.peakR); }
            lyMeter[l] = std::max(pk, lyMeter[l] * 0.88f);
            const Rect& m = lyMeterR[l];
            beginPath(); roundedRect(m.x, m.y, m.w, m.h, 3); fillColor(cInset); fill();
            const float dbm = 20.f * std::log10(std::max(1e-5f, lyMeter[l]));
            const float u = std::max(0.f, std::min(1.f, (dbm + 60.f) / 66.f));
            beginPath(); roundedRect(m.x, m.y, m.w * u, m.h, 3); fillColor(dbm > -0.5f ? cWarn : withA(col, 0.8f)); fill();
            const float z = m.x + m.w * (60.f / 66.f);
            beginPath(); moveTo(z, m.y - 2); lineTo(z, m.y + m.h + 2); strokeColor(cFaint); strokeWidth(1); stroke();
        }
        if (mode == 0) label(870, 486, "layer 2 is off: choose a mode in LAYER SETUP to hear it", 8.5f, cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
        else label(870, 486, "the faders are each layer's Master (also on its MAIN page) · double-click resets · meters 0 dB at the tick", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void lyFaderSet(int l, float mx) {
        const Rect& f = lyFader[l];
        const uint32_t i = pMaster + (l ? kLayerBOffset : 0);
        float db = -36.f + 48.f * std::max(0.f, std::min(1.f, (mx - f.x) / f.w));
        db = std::round(db * 10.f) / 10.f;
        hostVals[i] = db;
        const int vi = viewOfParam(i); if (vi >= 0) values[vi] = db;
        UI::setParameterValue(i, db);
        repaint();
    }
    bool lyFaderPress(float mx, float my, uint32_t t) {
        for (int l = 0; l < 2; ++l) {
            const Rect hit = { lyFader[l].x - 8, lyFader[l].y, lyFader[l].w + 16, lyFader[l].h };
            if (!hit.contains(mx, my)) continue;
            const uint32_t i = pMaster + (l ? kLayerBOffset : 0);
            pushUndo(-1);
            if (doubleClick(3000 + l, t)) { hostSetParam(i, P(pMaster).def); return true; }
            UI::editParameter(i, true);
            sd = sdLyFader; sdIndex = l;
            lyFaderSet(l, mx);
            return true;
        }
        return false;
    }
    void lyFaderRelease() { if (sdIndex >= 0 && sdIndex < 2) UI::editParameter(pMaster + (sdIndex ? kLayerBOffset : 0), false); sd = sdNone; sdIndex = -1; }
    bool layerSwitchClick(float mx, float my) {
        for (int l = 0; l < 2; ++l) if (lyLayerSwitch[l].contains(mx, my)) { if (l != editLayer) setEditLayer(l); return true; }
        return false;
    }

    // ---- layer management ----------------------------------------------------------------------
    void copyLayer(int from, int to) {
        pushUndo(-1);
        forEachLayerParam([&](uint32_t p) {
            const uint32_t a = from ? layer2Of(p) : p, b = to ? layer2Of(p) : p;
            if (hostVals[b] != hostVals[a]) hostSetParam(b, hostVals[a]);
        });
        for (int k = 0; k < kLayerStates; ++k) { const int a = k + from * kLayerStates, b = k + to * kLayerStates; if (hostStates[b] != hostStates[a]) hostSetState(b, std::string(hostStates[a])); }
        layerNames[to] = layerNames[from];
        publishLayerNames();
    }
    void swapLayers() {
        pushUndo(-1);
        forEachLayerParam([&](uint32_t p) { const uint32_t q = layer2Of(p); const float a = hostVals[p], b = hostVals[q]; if (a != b) { hostSetParam(p, b); hostSetParam(q, a); } });
        for (int k = 0; k < kLayerStates; ++k) { const std::string a = hostStates[k], b = hostStates[k + kLayerStates]; if (a != b) { hostSetState(k, b); hostSetState(k + kLayerStates, a); } }
        std::swap(layerNames[0], layerNames[1]);
        publishLayerNames();
    }
    void clearLayer2() {
        pushUndo(-1);
        forEachLayerParam([&](uint32_t p) { const uint32_t q = layer2Of(p); if (hostVals[q] != P(p).def) hostSetParam(q, P(p).def); });
        for (int k = 0; k < kLayerStates; ++k) if (!hostStates[k + kLayerStates].empty()) hostSetState(k + kLayerStates, "");
        layerNames[1].clear();
        publishLayerNames();
    }
    // a preset's sound (its layer 1) into layer `to`; switches layering on when it was off
    void loadPresetIntoLayer(int idx, int to) {
        float pv[kParamCount];
        std::string st[kStateCount];
        if (idx < (int)kFactoryPresetCount) { presetValues((uint32_t)idx, pv); presetStates((uint32_t)idx, st); }
        else {
            const UserPreset& up = userPresets[idx - kFactoryPresetCount];
            std::ifstream in(drift::lib::fromU8(up.path), std::ios::binary);
            std::stringstream ss; ss << in.rdbuf();
            std::string name, cat, tags;
            if (!presetFromText(ss.str(), pv, st, &name, &cat, &tags)) return;
        }
        pushUndo(-1);
        forEachLayerParam([&](uint32_t p) { const uint32_t q = to ? layer2Of(p) : p; if (hostVals[q] != pv[p]) hostSetParam(q, pv[p]); });
        for (int k = 0; k < kLayerStates; ++k) if (hostStates[k + to * kLayerStates] != st[k]) hostSetState(k + to * kLayerStates, st[k]);
        if (to == 1 && lyMode() == 0) hostSetParam(pLyMode, 1.f);
        layerNames[to] = entryName(idx);
        publishLayerNames();
        markEdited();
        flash((std::string("Loaded ") + entryName(idx) + (to ? " into layer 2" : " into layer 1")).c_str());
    }
    void publishLayerNames() { setState("layerNames", (layerNames[0] + "|" + layerNames[1]).c_str()); }
    void applyLayerNames(const char* v) {
        const std::string s = v ? v : "";
        const size_t bar = s.find('|');
        layerNames[0] = bar == std::string::npos ? s : s.substr(0, bar);
        layerNames[1] = bar == std::string::npos ? "" : s.substr(bar + 1);
    }

    // ---- resample -------------------------------------------------------------------------------
    static std::string capturesDir() { return drift::lib::u8(drift::lib::fromU8(drift::lib::samplesDir()) / "Captured"); }
    // 5.1.1: the last `seconds` of output as a normalised WAV (Samples\\Captured); "" on failure (flashes why)
    std::string captureToWav(float seconds, const char* prefix) {
        drift::CaptureRing* r = shared ? shared->capture.load() : nullptr;
        if (!r || r->size <= 0) { flash("Capture needs the editor in the plug-in process"); return ""; }
        const int filled = r->filled.load(std::memory_order_acquire);
        const int want = std::min(filled, (int)(seconds * r->sr));
        if (want < (int)(0.25f * r->sr)) { flash("Nothing to capture yet: play something first"); return ""; }
        const int w = r->write.load(std::memory_order_acquire);
        std::vector<float> out((size_t)want * 2);
        float peak = 0.f;
        for (int i = 0; i < want; ++i) {
            int k = w - want + i; if (k < 0) k += r->size;
            out[(size_t)2 * i] = r->data[(size_t)2 * k]; out[(size_t)2 * i + 1] = r->data[(size_t)2 * k + 1];
            peak = std::max(peak, std::max(std::fabs(out[(size_t)2 * i]), std::fabs(out[(size_t)2 * i + 1])));
        }
        if (peak < 1e-4f) { flash("The last seconds are silent"); return ""; }
        const float g = 0.9f / peak;
        for (float& v : out) v *= g;
        const std::time_t t = std::time(nullptr);
        char nm[96]; std::strftime(nm, sizeof nm, " %Y-%m-%d %H%M%S.wav", std::localtime(&t));
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(capturesDir()), ec);
        const std::string path = drift::lib::u8(drift::lib::fromU8(capturesDir()) / (std::string(prefix) + nm));
        if (!drift::ana::writeWav(path, out, 2, (int)r->sr)) { flash("Could not write the capture file"); return ""; }
        samplesScanned = false;
        return path;
    }
    void analyseCapture(int kind) {
        const std::string path = captureToWav(kAnLens[anSrc], "Analysed capture");
        if (path.empty()) return;
        anKind = kind;
        analyseSelected(path.c_str());
    }
    void captureToGrain() {
        drift::CaptureRing* r = shared ? shared->capture.load() : nullptr;
        if (!r || r->size <= 0) { flash("Capture needs the editor in the plug-in process"); return; }
        const int filled = r->filled.load(std::memory_order_acquire);
        const int want = std::min(filled, (int)(kRsLens[rsLen] * r->sr));
        if (want < (int)(0.25f * r->sr)) { flash("Nothing to capture yet: play something first"); return; }
        const int w = r->write.load(std::memory_order_acquire);
        std::vector<float> out((size_t)want * 2);
        float peak = 0.f;
        for (int i = 0; i < want; ++i) {
            int k = w - want + i; if (k < 0) k += r->size;
            out[(size_t)2 * i] = r->data[(size_t)2 * k]; out[(size_t)2 * i + 1] = r->data[(size_t)2 * k + 1];
            peak = std::max(peak, std::max(std::fabs(out[(size_t)2 * i]), std::fabs(out[(size_t)2 * i + 1])));
        }
        if (peak < 1e-4f) { flash("The last seconds are silent"); return; }
        const float g = 0.9f / peak;
        for (float& v : out) v *= g;
        // short fades so the grain buffer does not click at its ends
        const int fade = std::min(want / 4, (int)(0.01f * r->sr));
        for (int i = 0; i < fade; ++i) { const float a = (float)i / fade; out[(size_t)2 * i] *= a; out[(size_t)2 * i + 1] *= a; out[(size_t)2 * (want - 1 - i)] *= a; out[(size_t)2 * (want - 1 - i) + 1] *= a; }
        const std::time_t t = std::time(nullptr);
        char nm[64]; std::strftime(nm, sizeof nm, "Capture %Y-%m-%d %H%M%S.wav", std::localtime(&t));
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(capturesDir()), ec);
        const std::string path = drift::lib::u8(drift::lib::fromU8(capturesDir()) / nm);
        if (!drift::ana::writeWav(path, out, 2, (int)r->sr)) { flash("Could not write the capture file"); return; }
        pushUndo(-1);
        if (rsAs == 1) {   // spectral: resynthesise the capture (freeze it, stretch it, cross it)
            if (oscType(rsOsc) != otSpectral) setParamUser(oscP(rsOsc, ofType), (float)otSpectral);
            if (values[oscP(rsOsc, ofOn)] < 0.5f) setParamUser(oscP(rsOsc, ofOn), 1.f);
            pushState(skSrcA + rsOsc, drift::lib::encode(path), false);
        } else {
            if (oscType(rsOsc) != otGranular) setParamUser(oscP(rsOsc, ofType), (float)otGranular);
            if (values[oscP(rsOsc, ofOn)] < 0.5f) setParamUser(oscP(rsOsc, ofOn), 1.f);
            setParamUser(oscP(rsOsc, ofGrSource), 1.f);            // keyed: the capture follows the keyboard
            pushState(skSmA + rsOsc, drift::lib::encode(path), false);
        }
        samplesScanned = false;                                // the browser picks up the new file
        rsLast = nm;
        char b[160]; std::snprintf(b, sizeof b, "Captured %.1f s into osc %c (layer %d)", want / r->sr, 'A' + rsOsc, editLayer + 1);
        flash(b);
    }

    // ---- analyse --------------------------------------------------------------------------------
    int anKind = 0;   // 1 partials, 2 wavetable
    void openAnalyse(int kind) {
        anKind = kind;
        browserKind = bkAnalyse;
        FileBrowserOptions opts;
        opts.title = kind == 1 ? "Analyse a WAV into additive partials" : (kind == 2 ? "Analyse a WAV into a wavetable" : "Analyse a struck or bowed WAV into resonant modes");
        const std::string dir = drift::lib::samplesDir();
        opts.startDir = dir.c_str();
        openFileBrowser(opts);
    }
    void analyseSelected(const char* path) {
        drift::ana::Info info;
        const std::string stem = drift::lib::u8(drift::lib::fromU8(path).stem());
        if (anKind == 1) {
            float amps[128];
            if (!drift::ana::toSpectrum(path, amps, info)) { anLastOk = false; anLast = stem + ": " + info.err; flash(("Analysis failed: " + info.err).c_str()); return; }
            pushUndo(-1);
            if (oscType(anOsc) != otAdditive) setParamUser(oscP(anOsc, ofType), (float)otAdditive);
            if (values[oscP(anOsc, ofOn)] < 0.5f) setParamUser(oscP(anOsc, ofOn), 1.f);
            setParamUser(oscP(anOsc, ofAdSpec1), 15.f);         // Custom
            setParamUser(oscP(anOsc, ofAdMorph), 0.f);
            pushState(skSpA + anOsc, drift::spectrumToString(amps), false);
            std::copy(amps, amps + 128, anAmps); anHaveAmps = true; anHaveModes = false;
            char b[200]; std::snprintf(b, sizeof b, "%s: %.1f Hz, 128 partials into osc %c", stem.c_str(), info.f0, 'A' + anOsc);
            anLast = b; anLastOk = true; flash(b);
        } else if (anKind == 3) {
            drift::ModalData md;
            if (!drift::ana::toModes(path, md, info)) { anLastOk = false; anLast = stem + ": " + info.err; flash(("Analysis failed: " + info.err).c_str()); return; }
            pushUndo(-1);
            if (oscType(anOsc) != otPhysical) setParamUser(oscP(anOsc, ofType), (float)otPhysical);
            if (values[oscP(anOsc, ofOn)] < 0.5f) setParamUser(oscP(anOsc, ofOn), 1.f);
            setParamUser(oscP(anOsc, ofPmModel), (float)pmAnalysed);
            setParamUser(oscP(anOsc, ofPmDecay), 5.f);           // 5 s = the measured decays
            pushState(skModesA + anOsc, drift::modalToString(md), false);
            anModesD = md; anHaveModes = true; anHaveAmps = false;
            char b[220]; std::snprintf(b, sizeof b, "%s: %d modes from %.1f Hz into osc %c (Physical · Analysed)", stem.c_str(), md.count, info.f0, 'A' + anOsc);
            anLast = b; anLastOk = true; flash(b);
        } else {
            const std::string out = drift::lib::u8(drift::lib::fromU8(drift::lib::libraryDir()) / "Analysed" / (stem + ".wav"));
            if (!drift::ana::toWavetable(path, out, info)) { anLastOk = false; anLast = stem + ": " + info.err; flash(("Analysis failed: " + info.err).c_str()); return; }
            pushUndo(-1);
            if (oscType(anOsc) != otWavetable) setParamUser(oscP(anOsc, ofType), (float)otWavetable);
            if (values[oscP(anOsc, ofOn)] < 0.5f) setParamUser(oscP(anOsc, ofOn), 1.f);
            pushState(skWtA + anOsc, drift::lib::encode(out), false);
            libraryScanned = false;
            char b[220]; std::snprintf(b, sizeof b, "%s: %.1f Hz, 64-frame wavetable into osc %c (Wavetables/Analysed)", stem.c_str(), info.f0, 'A' + anOsc);
            anLast = b; anLastOk = true; flash(b);
        }
    }

    // ---- drawing --------------------------------------------------------------------------------
    void drawLayers() {
        static const char* const modeHelp[6] = {
            "Off: only layer 1 plays. Layer 2 keeps its settings.",
            "Layer: both layers play every note.",
            "Split: keys below the split key play layer 1, from it up layer 2; XFADE WIDTH blends across the split.",
            "Velocity XF: soft notes play layer 1, hard notes layer 2. MORPH / CENTRE sets the crossover, XFADE WIDTH how gradual it is.",
            "Pressure XF: MPE pressure (or note-expression Expression) moves each note from layer 1 to layer 2 while it plays.",
            "Morph: MORPH blends the two layers; automate it or map it to a controller." };
        const int mode = lyMode();
        labelS(32, 160, modeHelp[mode], 9.f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        char b[96];
        std::snprintf(b, sizeof b, "%s", noteName((int)std::lround(values[pLySplit])));
        label(92, 252, b, 9.5f, mode == 2 ? cAccent : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        // gain map
        const Rect& r = lyMap;
        inset(r);
        const bool keys = mode == 2 || mode <= 1;
        const float x0 = r.x + 40, x1 = r.x + r.w - 16, y0 = r.y + 24, y1 = r.y + r.h - 34;
        label(r.x + 12, y0, "1", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(r.x + 12, y1, "0", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        for (int l = 0; l < 2; ++l) {
            beginPath();
            for (int i = 0; i <= 200; ++i) {
                const float u = i / 200.f;
                const float x = keys ? 24.f + u * 84.f : u;     // keys 24..108, or the 0..1 range of velocity / pressure / morph
                float g;
                if (mode == 0) g = l == 0 ? 1.f : 0.f;
                else if (mode == 1) g = 1.f;
                else { const float s = mode == 5 ? values[pLyMorph] : lyPos(x); g = l == 0 ? std::cos(s * 1.5707963f) : std::sin(s * 1.5707963f); }
                const float px = x0 + u * (x1 - x0), py = y1 - g * (y1 - y0) + (l ? 0.f : 1.5f);
                if (i == 0) moveTo(px, py); else lineTo(px, py);
            }
            strokeColor(l ? cViolet : cAccent); strokeWidth(2.f); stroke();
        }
        const char* xl = keys ? "keys C1 to C8" : (mode == 3 ? "velocity" : (mode == 4 ? "pressure" : "morph"));
        label((x0 + x1) * 0.5f, r.y + r.h - 14, xl, 8.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        label(x1, r.y + 12, "layer 1", 8.5f, cAccent, ALIGN_RIGHT | ALIGN_MIDDLE);
        label(x1 - 60, r.y + 12, "layer 2", 8.5f, cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
        if (mode == 5) { const float px = x0 + values[pLyMorph] * (x1 - x0); beginPath(); moveTo(px, y0); lineTo(px, y1); strokeColor(withA(cWarm, 0.7f)); strokeWidth(1.5f); stroke(); }

        // layers card
        {
            const Rect& c = lyCardLayers;
            for (int l = 0; l < 2; ++l) {
                const Rect& e = lyEditBtn[l];
                button(e, l ? "EDIT LAYER 2" : "EDIT LAYER 1", editLayer == l, hovered(e), 10.f, l ? &cViolet : nullptr);
                const std::string nm = layerNames[l].empty() ? std::string(l == 0 ? (currentPreset >= 0 ? entryName(currentPreset).c_str() : "current sound") : "(not named)") : layerNames[l];
                labelS(e.x + e.w + 16, e.cy() - 6, nm, 11.f, cText, ALIGN_LEFT | ALIGN_MIDDLE);
                const float mdb = hostVals[pMaster + (l ? kLayerBOffset : 0)];
                std::snprintf(b, sizeof b, "level %+.1f dB (its Master knob)%s", mdb, l == 1 && mode == 0 ? "  ·  off" : "");
                label(e.x + e.w + 16, e.cy() + 10, b, 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            }
            button(lyLoad2, "LOAD A PRESET INTO LAYER 2…", false, hovered(lyLoad2), 9.5f, &cViolet);
            label(c.x + 16, c.y + 258, "Each layer is a complete sound: oscillators, filters, envelopes, modulation, FX chain, tuning.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 16, c.y + 274, "The pages edit the layer chosen with L1 / L2 at the top left.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            button(lyCopy12, "COPY 1 → 2", false, hovered(lyCopy12), 9.5f);
            button(lyCopy21, "COPY 2 → 1", false, hovered(lyCopy21), 9.5f);
            button(lySwap, "SWAP", false, hovered(lySwap), 9.5f);
            button(lyClear2, "CLEAR LAYER 2", false, hovered(lyClear2), 9.5f);
            drawLayerMix();
        }
        // resample card
        {
            label(32, 560, "LENGTH", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            static const char* const ln[5] = { "2 s", "4 s", "8 s", "15 s", "30 s" };
            for (int i = 0; i < 5; ++i) button(rsLenBtn[i], ln[i], rsLen == i, hovered(rsLenBtn[i]), 9.5f);
            label(480, 560, "INTO OSCILLATOR (edited layer)", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            static const char* const on[3] = { "A", "B", "C" };
            for (int o = 0; o < 3; ++o) button(rsOscBtn[o], on[o], rsOsc == o, hovered(rsOscBtn[o]), 10.f);
            std::snprintf(b, sizeof b, "CAPTURE THE LAST %s INTO OSC %c", ln[rsLen], 'A' + rsOsc);
            button(rsCapture, b, false, hovered(rsCapture), 11.f, &cWarm);
            button(rsAsBtn[0], "AS GRAIN", rsAs == 0, hovered(rsAsBtn[0]), 8.5f);
            button(rsAsBtn[1], "AS SPECTRAL", rsAs == 1, hovered(rsAsBtn[1]), 8.5f);
            const Rect& v = rsView;
            inset(v);
            drift::CaptureRing* ring = shared ? shared->capture.load() : nullptr;
            if (!ring || ring->size <= 0) label(v.cx(), v.cy(), "not available in this host (separate editor process)", 9.f, cWarn, ALIGN_CENTER | ALIGN_MIDDLE);
            else {
                // the last 30 s as a peak envelope; the capture window is tinted
                const int cols = 360, w = ring->write.load(), n = ring->size, per = std::max(1, n / cols);
                const float mid = v.cy(), hh = v.h * 0.42f;
                const float sel = kRsLens[rsLen] / 30.f;
                beginPath(); rect(v.x + v.w * (1.f - sel), v.y + 2, v.w * sel - 2, v.h - 4); fillColor(tint(cWarm, 0.12f)); fill();
                beginPath();
                for (int c2 = 0; c2 < cols; ++c2) {
                    float pk = 0.f;
                    for (int j = 0; j < per; j += 8) { int k = w - n + c2 * per + j; while (k < 0) k += n; k %= n; pk = std::max(pk, std::fabs(ring->data[(size_t)2 * k])); }
                    pk = std::min(1.f, pk * 1.5f);
                    const float x = v.x + 4 + (v.w - 8) * c2 / (cols - 1.f);
                    moveTo(x, mid - pk * hh); lineTo(x, mid + pk * hh);
                }
                strokeColor(withA(cAccent, 0.8f)); strokeWidth(1.2f); stroke();
                label(v.x + 8, v.y + 12, "last 30 s of output", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
                label(v.x + v.w - 8, v.y + 12, "now", 8.f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
            }
            labelS(32, 924, rsLast.empty() ? std::string("Captures are saved in Documents\\Driftfield\\Samples\\Captured and travel with the session.") : "Last capture: " + rsLast, 9.f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            label(32, 944, rsAs == 1 ? "The oscillator switches to Spectral: freeze, stretch or cross the capture on the MAIN page." : "The oscillator switches to Granular, keyed source; scan and size it on the MAIN page.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        // analyse card
        {
            label(804, 560, "INTO OSCILLATOR (edited layer)", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            static const char* const on[3] = { "A", "B", "C" };
            for (int o = 0; o < 3; ++o) button(anOscBtn[o], on[o], anOsc == o, hovered(anOscBtn[o]), 10.f);
            label(1124, 560, "SOURCE", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            static const char* const sl[5] = { "WAV FILE…", "LAST 0.5 s", "LAST 1 s", "LAST 2 s", "LAST 4 s" };
            for (int i = 0; i < 5; ++i) button(anSrcBtn[i], sl[i], anSrc == i, hovered(anSrcBtn[i]), 8.5f);
            const bool cap = anSrc > 0;
            button(anPartials, cap ? "CAPTURE → 128 PARTIALS" : "WAV → 128 PARTIALS…", false, hovered(anPartials), 10.f);
            button(anTable, cap ? "CAPTURE → 64-FRAME TABLE" : "WAV → 64-FRAME TABLE…", false, hovered(anTable), 10.f);
            button(anModes, cap ? "CAPTURE → MODES (physical)" : "WAV → MODES (physical)…", false, hovered(anModes), 10.f, &cWarm);
            const Rect v = { 804, 684, 764, 220 };
            inset(v);
            if (anHaveAmps) {
                const float bw = (v.w - 16) / 128.f;
                for (int h = 0; h < 128; ++h) {
                    const float a = std::sqrt(std::max(0.f, anAmps[h]));
                    beginPath(); rect(v.x + 8 + h * bw, v.y + v.h - 10 - a * (v.h - 28), std::max(1.f, bw - 1), a * (v.h - 28)); fillColor(withA(cAccent, 0.85f)); fill();
                }
                label(v.x + 8, v.y + 12, "last analysis: partial amplitudes (square-root scale)", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            } else if (anHaveModes) {
                // modes on a log-frequency axis (ratio 0.5 .. 64), height = amplitude, brightness = decay
                for (int i = 0; i < anModesD.count; ++i) {
                    const float x = v.x + 8 + (v.w - 16) * std::log2(std::max(0.5f, anModesD.ratio[i]) * 2.f) / 7.f;
                    if (x > v.x + v.w - 6) continue;
                    const float a = std::sqrt(anModesD.amp[i]), d = std::min(1.f, anModesD.decay[i] / 10.f);
                    beginPath(); moveTo(x, v.y + v.h - 10); lineTo(x, v.y + v.h - 10 - a * (v.h - 30)); strokeColor(withA(cWarm, 0.3f + 0.7f * d)); strokeWidth(2.f); stroke();
                }
                label(v.x + 8, v.y + 12, "last analysis: modes (log frequency · height = level · brightness = decay)", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            } else label(v.cx(), v.cy(), "Pick a pitched recording: a sung note, a bowed string, a held synth note.", 9.f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            labelS(804, 924, anLast.empty() ? std::string("Partials go into the osc's Custom spectrum; wavetables are written to Wavetables\\Analysed.") : anLast, 9.f, anLastOk ? cDim : cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
            label(804, 944, anSrc ? "Capture: play the sound, then press a button: the last seconds of Driftfield's output are analysed (kept in Samples\\Captured)."
                                  : "Partials / table: pitched sounds. MODES: bells, bowls, gongs, glass, plates (any inharmonic decay, 0.5 s or longer).", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        }
    }

    bool layersClick(float mx, float my) {
        for (int l = 0; l < 2; ++l) if (lyEditBtn[l].contains(mx, my)) { setEditLayer(l); return true; }
        if (lyLoad2.contains(mx, my)) { pbInto = 2; openPresetBrowser(); return true; }
        if (lyCopy12.contains(mx, my)) { copyLayer(0, 1); flash("Layer 1 copied into layer 2"); return true; }
        if (lyCopy21.contains(mx, my)) { copyLayer(1, 0); flash("Layer 2 copied into layer 1"); return true; }
        if (lySwap.contains(mx, my)) { swapLayers(); flash("Layers swapped"); return true; }
        if (lyClear2.contains(mx, my)) { clearLayer2(); flash("Layer 2 reset to Init"); return true; }
        for (int i = 0; i < 5; ++i) if (rsLenBtn[i].contains(mx, my)) { rsLen = i; repaint(); return true; }
        for (int o = 0; o < 3; ++o) if (rsOscBtn[o].contains(mx, my)) { rsOsc = o; repaint(); return true; }
        if (rsCapture.contains(mx, my)) { captureToGrain(); return true; }
        for (int i = 0; i < 2; ++i) if (rsAsBtn[i].contains(mx, my)) { rsAs = i; repaint(); return true; }
        for (int o = 0; o < 3; ++o) if (anOscBtn[o].contains(mx, my)) { anOsc = o; repaint(); return true; }
        for (int i = 0; i < 5; ++i) if (anSrcBtn[i].contains(mx, my)) { anSrc = i; repaint(); return true; }
        if (anPartials.contains(mx, my)) { if (anSrc) analyseCapture(1); else openAnalyse(1); return true; }
        if (anTable.contains(mx, my)) { if (anSrc) analyseCapture(2); else openAnalyse(2); return true; }
        if (anModes.contains(mx, my)) { if (anSrc) analyseCapture(3); else openAnalyse(3); return true; }
        return false;
    }
