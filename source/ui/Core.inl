// ===================================================================================== Core
// (included inside class DriftfieldUI)

    // ------------------------------------------------------------------ state mirror
    // values / stateValue are the VIEW the pages draw: indices of layer 1 that show the edited layer
    // (layer 2 = host index + kLayerBOffset / + kLayerStates). hostVals / hostStates mirror the host.
    float values[kParamCount];
    std::string stateValue[kStateCount];
    float hostVals[kParamCount];
    std::string hostStates[kStateCount];
    int editLayer = 0;
    uint32_t hp(uint32_t p) const { return (editLayer == 1 && isLayerParam(p)) ? layer2Of(p) : p; }
    int hs(int k) const { return (editLayer == 1 && k >= 0 && k < kLayerStates) ? k + kLayerStates : k; }
    // host index -> view index (-1 when the edited layer does not show it)
    int viewOfParam(uint32_t i) const {
        uint32_t e; const int ly = hostToEngine(i, e);
        if (ly < 0) return (int)i;
        return ly == editLayer ? (int)e : -1;
    }
    int viewOfState(int k) const {
        if (k < kLayerStates) return editLayer == 0 ? k : -1;
        return editLayer == 1 ? k - kLayerStates : -1;
    }
    // every page talks in view indices; these route to the edited layer
    void setParameterValue(uint32_t p, float v) { hostVals[hp(p)] = v; UI::setParameterValue(hp(p), v); fmTouched(p); }
    // 6.0: touching an oscillator's FM controls gives it the 6.0 FM (PM / FM also on Additive, Physical, Spectral).
    // Presets and sessions from before 6.0 keep their dormant FM settings silent until then.
    void fmTouched(uint32_t p) {
        if (p >= kLayerParams) return;
        for (int o = 0; o < 3; ++o) {
            if (p != oscP(o, ofFmSrc) && p != oscP(o, ofFmMode) && p != oscP(o, ofFmAmt)) continue;
            const int bits = (int)(values[pFmV6] + 0.5f);
            if (bits & (1 << o)) return;
            const float nv = (float)(bits | (1 << o));
            values[pFmV6] = nv; hostVals[hp(pFmV6)] = nv;
            UI::editParameter(hp(pFmV6), true); UI::setParameterValue(hp(pFmV6), nv); UI::editParameter(hp(pFmV6), false);
            return;
        }
    }
    void editParameter(uint32_t p, bool started) { UI::editParameter(hp(p), started); }
    void setState(const char* key, const char* value) {
        int k = stateIndexOf(key);
        if (k >= 0 && k < kLayerStates) { k = hs(k); hostStates[k] = value; UI::setState(kStateKeys[k], value); return; }
        if (k >= 0) hostStates[k] = value;
        UI::setState(key, value);
    }
    // host-level writes (presets, undo, init): host indices, no view mapping
    void hostSetParam(uint32_t i, float v) {
        hostVals[i] = v;
        const int vi = viewOfParam(i); if (vi >= 0) values[vi] = v;
        if (i >= kEngineParams && !isExtParam(i)) values[i] = v;
        UI::editParameter(i, true); UI::setParameterValue(i, v); UI::editParameter(i, false);
    }
    void hostSetState(int k, const std::string& v) {
        hostStates[k] = v;
        UI::setState(kStateKeys[k], v.c_str());
        const int vk = viewOfState(k); if (vk >= 0) { stateValue[vk] = v; applyStateLocal(vk); }
    }
    void setEditLayer(int n) {
        editLayer = n ? 1 : 0;
        forEachLayerParam([&](uint32_t p) { values[p] = hostVals[editLayer ? layer2Of(p) : p]; });
        for (int k = 0; k < kLayerStates; ++k) { stateValue[k] = hostStates[k + (editLayer ? kLayerStates : 0)]; applyStateLocal(k); }
        lastTelSeq = 0; haveRaw = false; lastCcSeq = 0;
        repaint();
    }
    drift::Telemetry& telSrc() { return editLayer ? shared->tel2 : shared->tel; }
    drift::SharedBlock* shared = nullptr;
    drift::Telemetry tel;
    uint32_t lastTelSeq = 0;
    bool liveVoices = false;

    std::unique_ptr<drift::WaveTable> fileTable[3];
    bool fileMissing[3] = { false, false, false };
    std::vector<float> smpOverview[3];      // min/max pairs, 512 columns
    std::string smpName[3];
    bool smpMissing[3] = { false, false, false };
    float customSpec[3][drift::kPartials];
    float customSpec2[3][drift::kPartials];   // 6.0: Custom 2
    float lfoPts[kLfos][drift::kLfoShapePoints];
    drift::MsegData msegD[kMsegs];
    drift::ModalData modalD[3];
    drift::SceneData sceneD;
    std::unique_ptr<drift::MotionData> motionD { new drift::MotionData() };
    int fxOrder[kFxUnits];
    std::string macroName[kMacros];

    // ------------------------------------------------------------------ controls & layout
    std::vector<Ctl> ctls;
    int page = pgMain;
    struct PanelDef { Rect r; const char* title; int page; };
    std::vector<PanelDef> panels;
    static constexpr float kMinScale = 0.5f, kMaxScale = 2.0f;
    static constexpr float kSizes[4] = { 0.75f, 0.9f, 1.0f, 1.25f };

    // header
    Rect tabRect[kPages], presetPrev, presetNext, presetBox, saveBtn, initBtn, abBtn[2], undoBtn, redoBtn, randBtn, qualityBox, sizeBtn[4];
    // main page geometry
    float oscX[3] = {}, oscY = 62.f;
    Rect typeTab[3][kOscTypes], onBtn[3], engDisp[3], tableBox[3], tablePrev[3], tableNext[3];
    Rect specEdit[3], specImport[3], freezeBtn[3], smpBox[3], smpPrev[3], smpNext[3], retrigBtn[3];
    Rect spcPrev[3], spcBox[3], spcNext[3], spcX[3];
    // 5.0 spectral source pictures for the display (computed when the state changes)
    struct SpecView { std::vector<float> img; int cols = 0, rows = 0; std::string name; bool missing = false, pitched = false; float root = 261.6f, secs = 0.f; };
    SpecView specView[3], specXView[3];
    void refreshSpecView(SpecView& sv, const std::string& stored) {
        sv = SpecView();
        if (stored.empty()) return;
        sv.name = drift::lib::displayName(stored);
        std::vector<uint8_t> file; std::vector<float> x; float sr = 48000.f;
        if (!drift::readFileUtf8(drift::lib::resolve(stored).c_str(), file) || !drift::decodeWavMono(file, x, sr, nullptr) || x.size() < 2048) { sv.missing = true; return; }
        if (x.size() > (size_t)(sr * drift::kSpecMaxSec)) x.resize((size_t)(sr * drift::kSpecMaxSec));
        sv.secs = (float)x.size() / sr;
        const int N = 1024, cols = 180, rows = 64;
        sv.cols = cols; sv.rows = rows; sv.img.assign((size_t)cols * rows, 0.f);
        std::vector<double> re(N), im(N);
        float peak = 1e-9f;
        const float fLo = 40.f, fHi = std::min(16000.f, sr * 0.48f);
        for (int c = 0; c < cols; ++c) {
            const size_t at = (x.size() - N) * (size_t)c / (cols - 1);
            for (int i = 0; i < N; ++i) { re[(size_t)i] = x[at + i] * (0.5 - 0.5 * std::cos(drift::ana::kPi2 * i / N)); im[(size_t)i] = 0.0; }
            drift::ana::fft(re, im);
            for (int k = 0; k < rows; ++k) {
                const float a = fLo * std::pow(fHi / fLo, (float)k / rows), b = fLo * std::pow(fHi / fLo, (float)(k + 1) / rows);
                const int ka = std::max(1, (int)(a / sr * N)), kb = std::max(ka, std::min(N / 2 - 1, (int)(b / sr * N)));
                float m = 0.f;
                for (int q = ka; q <= kb; ++q) m = std::max(m, (float)std::sqrt(re[(size_t)q] * re[(size_t)q] + im[(size_t)q] * im[(size_t)q]));
                sv.img[(size_t)c * rows + k] = m; peak = std::max(peak, m);
            }
        }
        for (float& v : sv.img) v = std::max(0.f, std::min(1.f, (20.f * std::log10(v / peak + 1e-9f) + 66.f) / 66.f));
        const float f0 = drift::ana::detectF0(x, sr);
        sv.pitched = f0 > 30.f; sv.root = sv.pitched ? f0 : 261.6f;
    }
    Rect envClassicBtn[3];
    bool envClassicUi = false;
    bool envClassic() const { return shared ? shared->envClassic.load() != 0 : envClassicUi; }
    void setEnvClassic(bool on) { envClassicUi = on; setState("envModel", on ? "classic" : ""); repaint(); }
    Rect filtDisp[2], routeBtn[3], envDisp[3], envHandle[3], macroHandle[kMacros], macroLabel[kMacros];
    Rect scopeRect, routesRect, voiceRect;
    Rect msegGridX, msegGridY, msegCopyBtn, msegPasteBtn, lfoCopyBtn, lfoPasteBtn;   // 6.0 MODULATOR panel extras
    // 5.1.6 MAIN page modulator panel
    static constexpr int kMainModTabs = 12;
    Rect mainModTab[kMainModTabs], mainModHandle, mainLfoEdit, mainMsegEdit, mainRndEdit, mainModTargets;
    // mod page geometry
    Rect lfoCard[kLfos], lfoEdit[kLfos], lfoHandle[kLfos], lfoSyncBtn[kLfos];
    Rect msegCard[kMsegs], msegEdit[kMsegs], msegHandle[kMsegs], msegSyncBtn[kMsegs];
    Rect rndCard, rndHandle[2], rndDisp[2], modMacroHandle[kMacros], matrixRect;
    // fx page
    Rect fxCardR[kFxUnits];        // by slot
    // seq page
    Rect arpCard, arpOnBtn, arpHoldBtn, arpDisp, seqCard, seqSteps, seqGates, seqDisp, keysRect;

    // ------------------------------------------------------------------ interaction
    float hoverX = -1.f, hoverY = -1.f;
    int hoverCtl = -1, dragCtl = -1, dragParam = -1, dragKind = 0;   // 1 knob 2 x-drag 3 slider
    float dragStartX = 0.f, dragStartY = 0.f, dragStartNorm = 0.f;
    Rect dragRect = { 0, 0, 1, 1 };
    bool  rightDrag = false; int rightSlot = -1; float rightStartAmt = 0.f;
    uint32_t lastClickTime = 0; int lastClickId = -1;
    uint32_t hoverSince = 0; int hoverStable = -1;
    enum SpecialDrag { sdNone, sdAddDraw, sdLfoDraw, sdMsegPoint, sdMsegLoop, sdMsegCurve, sdSeqStep, sdSeqGate, sdFxCard, sdAssign, sdResize, sdXY, sdLyFader, sdPbBar };
    int sd = sdNone, sdIndex = -1, sdSub = -1;
    float sdLastX = -1.f, sdLastY = -1.f, sdAux = 0.f, msegDragTotal = 1.f;
    uint32_t lastStatePush = 0;
    int assignSrc = 0;
    float assignFromX = 0.f, assignFromY = 0.f;   // 6.0: where the assign drag started
    // 6.0 copy / paste between MSEGs and between LFOs (editor clipboard)
    drift::MsegData clipMseg; bool clipMsegHave = false;
    bool msegFree = false;   // 6.0: ALT held: place MSEG points off the grid
    float clipLfoPts[drift::kLfoShapePoints] = {}; float clipLfoVals[kLfoFields + 1] = {}; bool clipLfoHave = false;
    void copyLfo(int i) {
        for (int f = 0; f < kLfoFields; ++f) clipLfoVals[f] = values[lfoP(i, f)];
        clipLfoVals[kLfoFields] = values[pLRndPh1 + i];
        for (int k = 0; k < drift::kLfoShapePoints; ++k) clipLfoPts[k] = lfoPts[i][k];
        clipLfoHave = true; flash("LFO copied");
    }
    void pasteLfo(int i) {
        if (!clipLfoHave) return;
        pushUndo(-1);
        for (int f = 0; f < kLfoFields; ++f) setParamUser(lfoP(i, f), clipLfoVals[f]);
        setParamUser(pLRndPh1 + i, clipLfoVals[kLfoFields]);
        for (int k = 0; k < drift::kLfoShapePoints; ++k) lfoPts[i][k] = clipLfoPts[k];
        pushState(skLfo1 + i, drift::lfoShapeToString(lfoPts[i]), false);
        flash("LFO pasted"); repaint();
    }
    float resizeStartW = 0.f, resizeStartX = 0.f;
    int  fxDropSlot = -1;
    bool seqGatePaint = true;

    // menus / overlays
    bool menuOpen = false;
    uint32_t menuParam = 0;
    Rect menuRect = { 0, 0, 0, 0 };
    int menuCols = 1;
    float menuColW = 160.f;
    enum TextTarget { ttNone, ttPresetName, ttSearch, ttMacro };
    int textTarget = ttNone, textIndex = -1;
    std::string textBuf;

    // ------------------------------------------------------------------ helpers
    float uiScale() const { return (float)getWidth() / kW; }
    bool anyOverlay() const { return menuOpen || presetOpen || browserOpen || saveOpen || randOpen || ctxOpen || dxListOpen >= 0; }
    int dxListOpen = -1;
    Rect logoRect = { 14, 8, 200, 34 }, linkRect, itchRect;   // 6.0: the logo and the SETUP link open sydalesis.bandcamp.com   // 6.0: DX7 voice list over an FM oscillator panel
    int oscType(int o) const { return (int)std::round(values[oscP(o, ofType)]); }
    int ival(uint32_t p) const { return (int)std::round(values[p]); }
    bool visible(const Ctl& c) const {
        if (c.page >= 0 && c.page != page) return false;
        if (c.vis >= 0 && c.osc >= 0) {
            const int t = oscType(c.osc);
            if (c.vis == 10) { if (t > otAnalog) return false; }
            else if (c.vis == 11) { if (t > otAnalog && t != otFm) return false; }   // 6.0: unison on Wavetable, Analog, FM
            else if (t != c.vis) return false;
        }
        if (c.visParam >= 0 && ival((uint32_t)c.visParam) != c.visVal) return false;
        if (c.uiKey >= 0 && uiSel[c.uiKey] != c.uiVal) return false;
        return true;
    }
    // 4.2 editor-only selections (not saved): which MSEG each MOD card shows, which half of the matrix
    enum UiKey { ukMsegA, ukMsegB, ukMatrix, ukPerfXY, ukMainMod, ukFmOpA, ukFmOpB, ukFmOpC, kUiKeys };   // 6.0: the FM operator each osc panel edits   // 5.1.6: ukMainMod = source shown in the MAIN page's modulator panel
    int uiSel[kUiKeys] = {};
    static int msegStateKey(int m) { return m < 2 ? skMseg1 + m : skMseg3 + (m - 2); }
    static int msegSource(int m) { return m < 2 ? msMseg1 + m : msMseg3 + (m - 2); }
    bool msegShown(int m) const { return uiSel[ukMsegA + (m & 1)] == (m >> 1); }
    Rect ctlRect(const Ctl& c) const {
        Rect r = c.r;
        if (c.fxUnit >= 0) {   // FX controls are laid out for a 520-wide card; 4.2 cards are narrower
            const Rect& card = fxCardR[fxSlotOf(c.fxUnit)];
            const float sx = card.w / 520.f;
            r.x = card.x + r.x * sx; r.y += card.y;
            if (c.kind != ckKnob) r.w *= sx;
        }
        return r;
    }
    int fxSlotOf(int unit) const { for (int i = 0; i < kFxUnits; ++i) if (fxOrder[i] == unit) return i; return unit; }

    void setParam(uint32_t p, float v) { values[p] = v; setParameterValue(p, v); repaint(); }
    void setParamUser(uint32_t p, float v) { pushUndo(p); editParameter(p, true); setParam(p, v); editParameter(p, false); }
    void toggle(uint32_t p) { setParamUser(p, values[p] >= 0.5f ? 0.f : 1.f); }
    void stepEnum(uint32_t p, int dir) {
        const int n = enumCount(p);
        const int i = (int)std::round(values[p] - P(p).min);
        setParamUser(p, P(p).min + (float)((i + dir + n) % n));
    }
    void pushState(int k, const std::string& v, bool undoable = true) {
        if (undoable) pushUndo(-1);
        stateValue[k] = v;
        setState(kStateKeys[k], v.c_str());   // routed to the edited layer
        applyStateLocal(k);
        repaint();
    }

    // ------------------------------------------------------------------ host -> UI
    void parameterChanged(uint32_t index, float value) override {
        if (index >= kParamCount) return;
        hostVals[index] = value;
        const int vi = viewOfParam(index); if (vi >= 0) values[vi] = value;
        if (index >= kEngineParams && !isExtParam(index)) values[index] = value;
        repaint();
    }
    void programLoaded(uint32_t index) override {
        if (index >= kFactoryPresetCount) return;
        currentPreset = (int)index;
        presetEdited = false; orphanPresetName.clear();
        presetStates(index, hostStates);
        for (int k = 0; k < kLayerStates; ++k) { stateValue[k] = hostStates[k + (editLayer ? kLayerStates : 0)]; applyStateLocal(k); }
        repaint();
    }
    void stateChanged(const char* key, const char* value) override {
        if (key != nullptr && std::strcmp(key, "presetId") == 0) { applyPresetId(value != nullptr ? value : ""); repaint(); return; }
        if (key != nullptr && std::strcmp(key, "envModel") == 0) { envClassicUi = value != nullptr && std::strcmp(value, "classic") == 0; repaint(); return; }
        if (key != nullptr && std::strcmp(key, "midiMap") == 0) { lastCcSeq = 0; repaint(); return; }
        if (key != nullptr && std::strcmp(key, "layerNames") == 0) { applyLayerNames(value); repaint(); return; }
        if (key != nullptr && std::strcmp(key, "threads") == 0) { instThreads = value != nullptr ? value : ""; repaint(); return; }
        const int k = stateIndexOf(key);
        if (k < 0) return;
        hostStates[k] = value != nullptr ? value : "";
        const int vk = viewOfState(k);
        if (vk >= 0) { stateValue[vk] = hostStates[k]; applyStateLocal(vk); }
        repaint();
    }
    void uiIdle() override {
        // the window exists by now; our own timer gives a steady frame rate independent of the host idle
        if (!tickerOn && !tickerTried) { tickerTried = true; startTicker(); }
        frameTick(false);
    }
    bool tickerTried = false;
    int tipShown = -1;

    // ------------------------------------------------------------------ state -> local display copies
    void applyStateLocal(int k) {
        const std::string& v = stateValue[k];
        if (k >= skWtA && k <= skWtC) refreshDisplayTable(k - skWtA);
        else if (k >= skSpA && k <= skSpC) { if (!drift::spectrumFromString(v.c_str(), customSpec[k - skSpA])) drift::spectrumDefault(customSpec[k - skSpA]); }
        else if (k >= skSp2A && k <= skSp2C) { if (!drift::spectrumFromString(v.c_str(), customSpec2[k - skSp2A])) drift::spectrumDefault(customSpec2[k - skSp2A]); }
        else if (k >= skSmA && k <= skSmC) refreshSample(k - skSmA);
        else if (k == skScl || k == skKbm) refreshUserScale();
        else if (k >= skLfo1 && k <= skLfo6) { if (!drift::lfoShapeFromString(v.c_str(), lfoPts[k - skLfo1])) drift::lfoShapeDefault(lfoPts[k - skLfo1], k - skLfo1); }
        else if (k == skMseg1 || k == skMseg2 || k == skMseg3 || k == skMseg4) {
            const int m = k <= skMseg2 ? k - skMseg1 : 2 + k - skMseg3;
            if (!drift::msegFromString(v.c_str(), msegD[m])) drift::msegDefault(msegD[m], m);
        }
        else if (k >= skModesA && k <= skModesC) { if (!drift::modalFromString(v.c_str(), modalD[k - skModesA])) drift::modalDefault(modalD[k - skModesA]); }
        else if (k == skScenes) drift::scenesFromString(v.c_str(), sceneD);
        else if (k == skMotion) drift::motionFromString(v.c_str(), *motionD);
        else if (k == skPerf) perfFromString(v);
        else if (k >= skSrcA && k <= skSrcC) refreshSpecView(specView[k - skSrcA], v);
        else if (k >= skXsrcA && k <= skXsrcC) refreshSpecView(specXView[k - skXsrcA], v);
        else if (k == skFxOrder) { if (!drift::fxOrderFromString(v.c_str(), fxOrder)) drift::fxOrderDefault(fxOrder); }
        else if (k == skMacroNames) {
            for (int m = 0; m < kMacros; ++m) macroName[m] = "Macro " + std::to_string(m + 1);
            size_t pos = 0; int m = 0;
            while (m < kMacros && pos <= v.size() && !v.empty()) {
                size_t e = v.find('|', pos); if (e == std::string::npos) e = v.size();
                if (e > pos) macroName[m] = v.substr(pos, e - pos);
                ++m; pos = e + 1;
            }
        }
    }
    void refreshDisplayTable(int o) {
        fileTable[o].reset();
        fileMissing[o] = false;
        const std::string& s = stateValue[skWtA + o];
        if (s.empty()) return;
        drift::WaveTable* t = drift::loadWavetableFile(drift::lib::resolve(s).c_str(), true);
        if (t) fileTable[o].reset(t); else fileMissing[o] = true;
    }
    void refreshSample(int o) {
        smpOverview[o].clear(); smpName[o].clear(); smpMissing[o] = false;
        const std::string& s = stateValue[skSmA + o];
        if (s.empty()) return;
        smpName[o] = drift::lib::displayName(s);
        std::vector<uint8_t> file; std::vector<float> x; float sr = 48000.f;
        if (!drift::readFileUtf8(drift::lib::resolve(s).c_str(), file) || !drift::decodeWavMono(file, x, sr, nullptr)) { smpMissing[o] = true; return; }
        const int cols = 512;
        smpOverview[o].assign(cols * 2, 0.f);
        float pk = 1e-6f;
        for (float v : x) pk = std::max(pk, std::fabs(v));
        for (int c = 0; c < cols; ++c) {
            const size_t a = x.size() * c / cols, b = std::max(a + 1, x.size() * (c + 1) / cols);
            float mn = 0.f, mx = 0.f;
            for (size_t i = a; i < b && i < x.size(); ++i) { mn = std::min(mn, x[i]); mx = std::max(mx, x[i]); }
            smpOverview[o][2 * c] = mn / pk; smpOverview[o][2 * c + 1] = mx / pk;
        }
    }
    const drift::WaveTable& tableFor(int o) const {
        if (fileTable[o]) return *fileTable[o];
        return drift::BuiltinTables::get().bank(ival(oscP(o, ofTable)));
    }
    std::string tableName(int o) const {
        const std::string& s = stateValue[skWtA + o];
        if (!s.empty()) return drift::lib::displayName(s) + (fileMissing[o] ? "  (missing)" : "");
        return kTableNames[std::max(0, std::min(7, ival(oscP(o, ofTable))))];
    }

    // ------------------------------------------------------------------ telemetry helpers
    const drift::TelVoice* newestVoice() const {
        if (!shared || tel.newest < 0 || tel.newest >= drift::kTelVoices) return nullptr;
        return tel.v[tel.newest].active ? &tel.v[tel.newest] : nullptr;
    }
    template <typename F> void forVoices(F f) const {
        if (!shared) return;
        for (int i = 0; i < drift::kTelVoices; ++i) if (tel.v[i].active) f(tel.v[i], i == tel.newest);
    }

    // destination index for a knob parameter (-1 if not a matrix target)
    // 6.0.4: one table per oscillator-type combination (every knob asked this every frame, scanning every destination)
    mutable std::vector<int16_t> destOfTab;
    mutable int destOfTypes[3] = { -1, -1, -1 };
    int destOf(uint32_t p) const {
        const int types[3] = { oscType(0), oscType(1), oscType(2) };
        if (destOfTab.empty() || types[0] != destOfTypes[0] || types[1] != destOfTypes[1] || types[2] != destOfTypes[2]) {
            destOfTab.assign(kParamCount, (int16_t)-1);
            // first destination wins, as the old linear search did
            for (int d = kModDstCount - 1; d >= 1; --d) { const drift::DestMap m = drift::destMap(d, types); if (m.param >= 0 && m.param < (int)kParamCount) destOfTab[(size_t)m.param] = (int16_t)d; }
            for (int i = 0; i < 3; ++i) destOfTypes[i] = types[i];
        }
        return p < kParamCount ? destOfTab[p] : -1;
    }
    // normalised knob offset for a modulation amount on destination d
    float modNormOffset(uint32_t p, int d, float amt) const {
        const int types[3] = { oscType(0), oscType(1), oscType(2) };
        const drift::DestMap m = drift::destMap(d, types);
        if (m.param < 0) return 0.f;
        const ParamDef& pd = P(p);
        if (m.kind == 1) {
            if (pd.flags & fLog) return amt * m.scale / std::log2(pd.max / pd.min);
            const float v = values[p], nv = v * std::pow(2.f, amt * m.scale);
            return toNorm(p, nv) - toNorm(p, v);
        }
        return amt * m.scale / (pd.max - pd.min);
    }

    // ------------------------------------------------------------------ value text
    std::string dstName(int d) const {
        if (d >= mdAPitch && d < mdAPitch + 3 * kOscDstStride) {
            const int o = (d - mdAPitch) / kOscDstStride, k = (d - mdAPitch) % kOscDstStride;
            if (k >= 3 && k <= 6) return std::string(1, (char)('A' + o)) + ": " + kTimbreNames[oscType(o)][k - 3];
        }
        if (d >= mdAT5 && d < mdAT5 + 3 * kTimbreX) {                     // 5.1.7: Timbre 5..10
            const int o = (d - mdAT5) / kTimbreX, k = (d - mdAT5) % kTimbreX;
            const char* n = kTimbreXNames[oscType(o)][k];
            return std::string(1, (char)('A' + o)) + ": " + (n ? n : (std::string("Timbre ") + std::to_string(5 + k) + " (unused)").c_str());
        }
        return kModDstNames[std::max(0, std::min((int)kModDstCount - 1, d))];
    }
    std::string valueName(uint32_t p, float v) const {
        if (isModDst(p)) return dstName((int)std::round(v));
        if (const char* e = enumNameOf(p, v)) return e;
        return "";
    }
    void formatValue(uint32_t p, float v, char* buf, size_t n) const {
        const std::string e = valueName(p, v);
        if (!e.empty()) { std::snprintf(buf, n, "%s", e.c_str()); return; }
        const ParamDef& d = P(p);
        const std::string u = d.unit, sym = d.symbol;
        if (p == pGlide && v < 0.0015f) { std::snprintf(buf, n, "off"); return; }
        if (sym.find("Fade") != std::string::npos && v < 0.001f) { std::snprintf(buf, n, "off"); return; }
        if (sym.find("AdBlur") != std::string::npos && v < 0.001f) { std::snprintf(buf, n, "off"); return; }
        if (sym.find("AdPeakPos") != std::string::npos) { std::snprintf(buf, n, "H%.1f", v); return; }
        if (sym.find("Fm") != std::string::npos && sym.find("Coarse") != std::string::npos) { std::snprintf(buf, n, v < 0.5f ? "×0.5" : "×%.0f", v); return; }   // 6.0 FM ratio
        if (sym.find("Fm") != std::string::npos && sym.find("Fine") != std::string::npos) { std::snprintf(buf, n, "+%.2f", v); return; }
        if (p >= pSeqV1 && p <= pSeqVLast && ival(pSeqMode) == 2) { std::snprintf(buf, n, "%+d st", (int)std::floor(v * 24.f + 0.5f)); return; }
        if (p == pFDecCurve || p == pADecCurve || p == pMDecCurve) { if (v == 0.f) std::snprintf(buf, n, "exp"); else std::snprintf(buf, n, v > 0.f ? "fast %.0f%%" : "hold %.0f%%", std::fabs(v) * 100.f); return; }   // 6.0
        if (p == pFRelCurve || p == pARelCurve || p == pMRelCurve) { if (std::fabs(v) < 0.04f) std::snprintf(buf, n, "exp"); else std::snprintf(buf, n, v > 0.f ? "fast %.0f%%" : "hold %.0f%%", std::fabs(v) * 100.f); return; }
        if (p == pFEnvCurve || p == pAEnvCurve || p == pMEnvCurve) { if (std::fabs(v) < 0.04f) std::snprintf(buf, n, "soft"); else std::snprintf(buf, n, v > 0.f ? "punch %.0f%%" : "swell %.0f%%", std::fabs(v) * 100.f); return; }
        if (p == pCrBits) { std::snprintf(buf, n, "%.1f bit", v); return; }
        if (p == pScPos) { std::snprintf(buf, n, "scene %.2f", v + 1.f); return; }
        if (p == pPolyphony) { std::snprintf(buf, n, "%d voices", (int)v); return; }
        if ((d.flags & fBool)) { std::snprintf(buf, n, v >= 0.5f ? "on" : "off"); return; }
        if (u == "Hz")       { if (std::fabs(v) >= 1000.f) std::snprintf(buf, n, "%.2f kHz", v / 1000.f); else if (std::fabs(v) < 1.f) std::snprintf(buf, n, "%.3f Hz", v); else std::snprintf(buf, n, "%.1f Hz", v); }
        else if (u == "s")   { if (v < 1.f) std::snprintf(buf, n, "%.0f ms", v * 1000.f); else std::snprintf(buf, n, "%.2f s", v); }
        else if (u == "ct")  std::snprintf(buf, n, "%+.1f ct", v);
        else if (u == "st")  std::snprintf(buf, n, p == pBendRange ? "%.0f st" : "%+.1f st", v);
        else if (u == "oct") std::snprintf(buf, n, "%+.0f oct", v);
        else if (u == "dB")  std::snprintf(buf, n, "%+.1f dB", v);
        else if (u == "x")   std::snprintf(buf, n, "%.2fx", v);
        else if (d.flags & fInt) std::snprintf(buf, n, "%.0f", v);
        else if (sym.find("AnPW") != std::string::npos) std::snprintf(buf, n, "%.0f%%", v * 100.f);
        else if (isBipolar(p)) std::snprintf(buf, n, "%+.0f%%", v * 100.f);
        else std::snprintf(buf, n, "%.0f%%", v * 100.f);
    }

    // ------------------------------------------------------------------ scale persistence
    void applyScale(float sc) {
        sc = std::max(kMinScale, std::min(kMaxScale, sc));
        const double sf = getScaleFactor();
        setSize((uint)(kW * sc * sf), (uint)(kHT * sc * sf));
        repaint();
    }
    static std::string driftDir() { return drift::lib::dataDir(); }   // 5.1.2: Documents/Sydalesis
    static std::string scaleFile() { return driftDir() + "/ui-scale-v3.txt"; }
    float loadSavedScale() const {
        std::ifstream in(drift::lib::fromU8(scaleFile()));
        float sc = 0.9f;
        if (in >> sc && sc >= kMinScale && sc <= kMaxScale) return sc;
        return 0.9f;
    }
    float currentRelativeScale() const { return (float)getWidth() / (kW * (float)getScaleFactor()); }
    void saveScale(float sc) {
        if (!(sc >= kMinScale && sc <= kMaxScale)) return;
        std::error_code ec;
        std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
        std::ofstream out(drift::lib::fromU8(scaleFile()));
        out << sc;
    }

    // ------------------------------------------------------------------ layout helpers
    Ctl& addCtl(int kind, uint32_t p, Rect r, const char* label, int pg) { ctls.push_back(Ctl{ kind, p, r, label, pg }); return ctls.back(); }
    Ctl& knob(uint32_t p, float x, float y, float rad, const char* label, int pg) { return addCtl(ckKnob, p, { x, y, rad, rad }, label, pg); }
    Ctl& slider(uint32_t p, Rect r, const char* label, int pg) { return addCtl(ckSlider, p, r, label, pg); }
    Ctl& selector(uint32_t p, Rect r, bool arrows, int pg) { Ctl& c = addCtl(ckSelector, p, r, nullptr, pg); c.arrows = arrows; return c; }
    Ctl& tog(uint32_t p, Rect r, const char* label, int pg) { return addCtl(ckToggle, p, r, label, pg); }
    Ctl& choice(uint32_t p, Rect r, int count, int pg) { Ctl& c = addCtl(ckChoice, p, r, nullptr, pg); c.count = count; return c; }
    void panel(Rect r, const char* title, int pg) { panels.push_back({ r, title, pg }); }

    void buildLayout() {
        ctls.clear(); panels.clear();
        buildHeader();
        buildMain();
        buildMod();
        buildFx();
        buildSeq();
        buildTune();
        buildLayers();
        buildPerform();
        buildSetup();
    }
    void buildHeader() {
        for (int i = 0; i < kPages; ++i) tabRect[i] = { 230.f + i * 45.f, 15, 43, 26 };
        presetPrev = { 596, 14, 26, 28 };
        presetBox  = { 624, 14, 262, 28 };
        presetNext = { 888, 14, 26, 28 };
        saveBtn = { 920, 14, 50, 28 };
        initBtn = { 974, 14, 48, 28 };      // 6.0
        abBtn[0] = { 1028, 14, 26, 28 }; abBtn[1] = { 1056, 14, 26, 28 };
        undoBtn = { 1090, 14, 34, 28 }; redoBtn = { 1126, 14, 34, 28 };
        randBtn = { 1166, 14, 52, 28 };
        qualityBox = { 1226, 16, 96, 24 };
        selector(pQuality, qualityBox, true, -1);
        for (int i = 0; i < 4; ++i) sizeBtn[i] = { 1488.f + i * 27.f, 34, 25, 16 };
    }
