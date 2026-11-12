// ===================================================================================== Options, display pipeline, SETUP page
// (included inside class DriftfieldUI)
//
// Telemetry arrives once per audio block (43 Hz at 1024 samples / 44.1 kHz), and the host's idle timer
// adds its own jitter. The UI therefore keeps the last raw frame and builds a display frame every tick:
//   * LFO phases are extrapolated with the rate the engine reports, so dots move continuously;
//     periodic shapes are evaluated at the extrapolated phase (random shapes keep the measured value);
//   * other live values (mod rings, filters, envelopes, positions, partials) are eased between frames.
// The tick runs on the UI's own window timer at the rate chosen on the SETUP page.

    // ------------------------------------------------------------------ options
    int  optRefresh = 1;           // index into kRefreshMs
    int  optSmooth = 1;            // 0 off, 1 light, 2 smooth
    bool optPredict = true, optTips = true, optLive = true, optGuard = true;
    int  optSkin = 0;
    int  optTextSize = 1;          // 5.1.5: 0 Normal, 1 Large (default), 2 Larger
    // 6.0 MSEG grid: time step in seconds (or beats with SYNC), level divisions; 0 = off (no snapping)
    int  optMsegGridX = 0, optMsegGridY = 0;
    static constexpr int kMsegGridX = 8, kMsegGridY = 7;
    static constexpr const char* kMsegGridXNames[kMsegGridX] = { "off", "1/16", "1/8", "1/4", "1/3", "1/2", "1", "2" };
    static constexpr float kMsegGridXVal[kMsegGridX] = { 0.f, 0.0625f, 0.125f, 0.25f, 1.f / 3.f, 0.5f, 1.f, 2.f };
    static constexpr const char* kMsegGridYNames[kMsegGridY] = { "off", "2", "3", "4", "8", "12", "16" };
    static constexpr int kMsegGridYVal[kMsegGridY] = { 0, 2, 3, 4, 8, 12, 16 };
    static constexpr int kRefreshCount = 4;
    static constexpr int kRefreshMs[kRefreshCount] = { 33, 16, 8, 4 };

    static std::string optionsFile() { return driftDir() + "/options-v3.txt"; }
    void loadOptions() {
        std::ifstream in(drift::lib::fromU8(optionsFile()));
        std::string k; int v;
        while (in >> k >> v) {
            if (k == "refresh") optRefresh = std::max(0, std::min(kRefreshCount - 1, v));
            else if (k == "smooth") optSmooth = std::max(0, std::min(2, v));
            else if (k == "predict") optPredict = v != 0;
            else if (k == "tooltips") optTips = v != 0;
            else if (k == "live") optLive = v != 0;
            else if (k == "skin") optSkin = std::max(0, std::min(kSkinCount - 1, v));
            else if (k == "scopeMode") scopeMode = std::max(0, std::min(3, v));
            else if (k == "scopeGain") scopeGain = std::max(0, std::min(4, v));
            else if (k == "scopeTime") scopeTime = std::max(0, std::min(4, v));
            else if (k == "scopeChan") scopeChan = std::max(0, std::min(2, v));
            else if (k == "scopeTilt") scopeTilt = std::max(0, std::min(2, v));
            else if (k == "rollLen") rollLen = std::max(0, std::min(2, v));
            else if (k == "cpuGuard") optGuard = v != 0;
            else if (k == "threads") optThreads = v;
            else if (k == "textSize") optTextSize = std::max(0, std::min(2, v));
            else if (k == "msegGridX") optMsegGridX = std::max(0, std::min(kMsegGridX - 1, v));
            else if (k == "msegGridY") optMsegGridY = std::max(0, std::min(kMsegGridY - 1, v));
            else if (k == "presetBank") pbBank = v ? 1 : 0;
        }
    }
    void saveOptions() const {
        std::error_code ec;
        std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
        std::ofstream out(drift::lib::fromU8(optionsFile()));
        out << "refresh " << optRefresh << "\nsmooth " << optSmooth << "\npredict " << (optPredict ? 1 : 0)
            << "\ntooltips " << (optTips ? 1 : 0) << "\nlive " << (optLive ? 1 : 0) << "\nskin " << optSkin << "\nscopeMode " << scopeMode << "\nscopeGain " << scopeGain << "\nscopeTime " << scopeTime
            << "\nscopeChan " << scopeChan << "\nscopeTilt " << scopeTilt << "\nrollLen " << rollLen << "\ncpuGuard " << (optGuard ? 1 : 0) << "\nthreads " << optThreads << "\ntextSize " << optTextSize
            << "\nmsegGridX " << optMsegGridX << "\nmsegGridY " << optMsegGridY << "\npresetBank " << pbBank << "\n";
    }

    // ------------------------------------------------------------------ frame timer
    struct Ticker : public DGL_NAMESPACE::IdleCallback {
        DriftfieldUI* ui = nullptr;
        void idleCallback() override { if (ui) ui->frameTick(true); }
    } ticker;
    bool tickerOn = false;
    double lastTimerTick = -1.0;
    void startTicker() {
        if (tickerOn) { removeIdleCallback(&ticker); tickerOn = false; }
        ticker.ui = this;
        tickerOn = addIdleCallback(&ticker, (uint)kRefreshMs[optRefresh]);
    }
    void stopTicker() { if (tickerOn) { removeIdleCallback(&ticker); tickerOn = false; } }
    static double secNow() {
        using namespace std::chrono;
        return duration<double>(steady_clock::now().time_since_epoch()).count();
    }

    // measured rates for the SETUP page
    double fpsWindowStart = 0.0; int fpsFrames = 0, telFrames = 0; float fpsMeasured = 0.f, telMeasured = 0.f;

    // ------------------------------------------------------------------ display pipeline
    drift::Telemetry telRaw;
    double telRawTime = 0.0, lastDisplayTime = 0.0;
    bool haveRaw = false;

    bool lfoPredictable(int i) const {
        const int w = ival(lfoP(i, lfWave));
        return w != 5 && w != 6 && values[lfoP(i, lfSmooth)] <= 0.001f;
    }
    static float lerpf(float a, float b, float k) { return a + (b - a) * k; }

    // returns true when something visible changed
    bool updateDisplay(double now) {
        bool fresh = false;
        if (shared) {
            const uint32_t s = __atomic_load_n(&telSrc().seq, __ATOMIC_ACQUIRE);
            if (s != lastTelSeq && drift::telRead(telSrc(), telRaw)) { lastTelSeq = s; telRawTime = now; haveRaw = true; fresh = true; ++telFrames; }
            if (std::getenv("DF_TELDBG") && fresh && (telFrames % 20) == 0) std::fprintf(stderr, "tel seq %u active %d cpu %.3f pk %.3f newest %d sw %d\n", s, telRaw.activeCount, telRaw.cpu, telRaw.peakL, telRaw.newest, telRaw.scopeWrite);
        }
        if (!haveRaw) return false;
        const float dt = (float)std::max(0.0, std::min(0.25, now - lastDisplayTime));
        lastDisplayTime = now;
        const float tau = optSmooth == 0 ? 0.f : (optSmooth == 1 ? 0.014f : 0.032f);
        const float k = tau <= 0.f ? 1.f : 1.f - std::exp(-dt / tau);
        const drift::Telemetry prev = tel;
        tel = telRaw;   // ints, scope, counters: always the latest
        if (k < 1.f) {
            for (int d = 0; d < kModDstCount; ++d) tel.md[d] = lerpf(prev.md[d], telRaw.md[d], k);
            for (int s = 0; s < kModSlots; ++s) { tel.slotOut[s] = lerpf(prev.slotOut[s], telRaw.slotOut[s], k); tel.slotSrc[s] = lerpf(prev.slotSrc[s], telRaw.slotSrc[s], k); }
            for (int m = 0; m < kMacros; ++m) tel.macro[m] = lerpf(prev.macro[m], telRaw.macro[m], k);
            tel.rndG = lerpf(prev.rndG, telRaw.rndG, k);
            tel.cpu = lerpf(prev.cpu, telRaw.cpu, std::min(1.f, k * 0.3f));
            for (int o = 0; o < 3; ++o) {
                for (int i = 0; i < drift::kTelPartials; ++i) tel.partials[o][i] = lerpf(prev.partials[o][i], telRaw.partials[o][i], k);
                if (prev.modeCount[o] == telRaw.modeCount[o]) for (int i = 0; i < telRaw.modeCount[o]; ++i) tel.modes[o][i] = lerpf(prev.modes[o][i], telRaw.modes[o][i], k);
            }
            for (int i = 0; i < drift::kTelVoices; ++i) {
                const drift::TelVoice& a = prev.v[i]; const drift::TelVoice& b = telRaw.v[i]; drift::TelVoice& v = tel.v[i];
                if (!b.active || !a.active || a.note != b.note || b.age < a.age) continue;   // new or restarted voice: snap
                v.amp = lerpf(a.amp, b.amp, k); v.envF = lerpf(a.envF, b.envF, k); v.envM = lerpf(a.envM, b.envM, k);
                v.pitch = lerpf(a.pitch, b.pitch, k);
                for (int o = 0; o < 3; ++o) { for (int q = 0; q < 4; ++q) v.t[o][q] = lerpf(a.t[o][q], b.t[o][q], k); v.level[o] = lerpf(a.level[o], b.level[o], k); v.fm[o] = lerpf(a.fm[o], b.fm[o], k); }
                v.cut1 = a.cut1 * std::pow(b.cut1 / std::max(1.f, a.cut1), k); v.cut2 = a.cut2 * std::pow(b.cut2 / std::max(1.f, a.cut2), k);
                v.res1 = lerpf(a.res1, b.res1, k); v.res2 = lerpf(a.res2, b.res2, k);
                v.shape1 = lerpf(a.shape1, b.shape1, k); v.shape2 = lerpf(a.shape2, b.shape2, k);
                v.morph1 = lerpf(a.morph1, b.morph1, k); v.morph2 = lerpf(a.morph2, b.morph2, k);
                v.rndV = lerpf(a.rndV, b.rndV, k);
                for (int l = 0; l < kLfos; ++l) v.lfo[l] = lerpf(a.lfo[l], b.lfo[l], k);
            }
            for (int l = 0; l < kLfos; ++l) tel.lfoG[l] = lerpf(prev.lfoG[l], telRaw.lfoG[l], k);
        }
        // LFO extrapolation
        if (optPredict) {
            const float ahead = (float)std::max(0.0, std::min(0.2, now - telRawTime));
            for (int l = 0; l < kLfos; ++l) {
                const bool pr = lfoPredictable(l);
                float ph = telRaw.lfoGPh[l] + telRaw.lfoGRate[l] * ahead; ph -= std::floor(ph);
                tel.lfoGPh[l] = ph;
                if (pr) tel.lfoG[l] = lfoShapeValue(l, ph);
                for (int i = 0; i < drift::kTelVoices; ++i) {
                    const drift::TelVoice& b = telRaw.v[i];
                    if (!b.active) continue;
                    float vp = b.lfoPh[l] + b.lfoRate[l] * ahead; vp -= std::floor(vp);
                    tel.v[i].lfoPh[l] = vp;
                    if (pr) tel.v[i].lfo[l] = lfoShapeValue(l, vp) * b.lfoFade[l];
                }
            }
        }
        liveVoices = tel.activeCount > 0;
        tickMixerMeters(dt, fresh);
        tickOutput(dt, fresh);
        return fresh || optPredict || k < 1.f;
    }

    // Windows delivers timer messages only when the message queue is empty, so a stream of mouse
    // moves over the editor starves the frame timer and the live displays stop. Mouse events pump a
    // frame themselves once a refresh interval has passed.
    double lastFrameAt = 0.0;
    void pumpFrame() {
        const double now = secNow();
        if (now - lastFrameAt >= kRefreshMs[optRefresh] * 0.001 * 0.9) frameTick(true);
    }
    void frameTick(bool fromTimer) {
        const double now = secNow();
        if (fromTimer) {
            if (now - lastFrameAt < kRefreshMs[optRefresh] * 0.001 * 0.5) return;   // already pumped by input
            lastTimerTick = now;
        }
        else if (tickerOn && now - lastTimerTick < 0.25) return;      // the timer is driving
        lastFrameAt = now;
        bool dirty = optLive ? updateDisplay(now) : false;
        if (!optLive && shared) {   // still keep meters/voice count roughly current
            const uint32_t s = __atomic_load_n(&telSrc().seq, __ATOMIC_ACQUIRE);
            if (s != lastTelSeq && now - telRawTime > 0.25 && drift::telRead(telSrc(), telRaw)) { lastTelSeq = s; telRawTime = now; tel = telRaw; liveVoices = tel.activeCount > 0; dirty = true; }
        }
        if (optTips && hoverStable >= 0 && !anyOverlay() && nowMs() - hoverSince > 420 && tipShown != hoverStable) { tipShown = hoverStable; dirty = true; }
        if (now - fpsWindowStart >= 1.0) {
            fpsMeasured = (float)(fpsFrames / (now - fpsWindowStart)); telMeasured = (float)(telFrames / (now - fpsWindowStart));
            fpsFrames = 0; telFrames = 0; fpsWindowStart = now;
            if (page == pgSetup) dirty = true;
        }
        tickMidiLearn();
        if (!debugHookDone) {   // test hook (screenshots / QA): SYD_DEBUG_PRESET=<index> SYD_DEBUG_PAGE=<page>; nothing when unset
            debugHookDone = true;
            if (const char* e = std::getenv("SYD_DEBUG_PRESET")) loadAnyPreset(std::atoi(e));
            if (const char* e = std::getenv("SYD_DEBUG_FM")) { const int a = std::atoi(e); for (int o = 0; o < 3; ++o) setParamUser(oscP(o, ofType), (float)otFm); setParamUser(fmP(0, fmAlgo), (float)a); setParamUser(fmP(1, fmAlgo), (float)((a + 10) % 32)); setParamUser(fmP(2, fmAlgo), 31.f); uiSel[ukFmOpB] = 3; }
            if (const char* e = std::getenv("SYD_DEBUG_SYX")) { syxSelected(0, e); if (const char* v = std::getenv("SYD_DEBUG_SYXV")) applyDxVoice(0, std::atoi(v)); if (std::getenv("SYD_DEBUG_DXLIST")) dxListOpen = 0; }
            if (const char* e = std::getenv("SYD_DEBUG_PAGE")) { page = std::max(0, std::min((int)kPages - 1, std::atoi(e))); dirty = true; }
        }
        if (perfTick()) dirty = true;
        if (page == pgLayer && shared) dirty = true;   // layer meters
        if (shared && shared->guardOn.load() != (optGuard ? 1 : 0)) shared->guardOn.store(optGuard ? 1 : 0);
        if (shared && shared->guardCap.load() != lastGuardCap) { lastGuardCap = shared->guardCap.load(); dirty = true; }
        if (dirty || sd == sdAssign) repaint();
    }
    int lastGuardCap = 0;
    bool debugHookDone = false;

    // ------------------------------------------------------------------ SETUP page
    Rect skinTile[kSkinCount], optRefreshBtn[kRefreshCount], optSmoothBtn[3], optPredictBtn[2], optTipsBtn[2], optLiveBtn[2], optGuardBtn[2], optThreadsBtn[4], optTextBtn[3];
    // 4.2 render threads: per instance (saved with the session); the choice also becomes the default for new instances
    static constexpr int kThreadVals[4] = { 1, 2, 4, 99 };
    int optThreads = 1;              // default for new instances (options file)
    std::string instThreads;         // this instance's "threads" state
    int threadsIndex() const {
        const std::string& v = instThreads;
        if (v.empty() || v == "0" || v == "1") return 0;
        if (v[0] == 'a') return 3;
        return std::atoi(v.c_str()) >= 4 ? 2 : 1;
    }
    Rect setupSkinCard, setupDispCard, setupInfoCard;
    static constexpr float kOptRowH = 64.f;

    void buildSetup() {
        setupSkinCard = { 16, 72, 1000, 480 };
        setupDispCard = { 1032, 72, 552, 560 };
        setupInfoCard = { 16, 648, 1568, 336 };
        panel(setupSkinCard, "SKIN", pgSetup);
        panel(setupDispCard, "DISPLAY", pgSetup);
        panel(setupInfoCard, "ABOUT · FOLDERS · LIVE RATES", pgSetup);
        for (int i = 0; i < kSkinCount; ++i) {   // 5.1.1: 16 skins, 6 per row
            const int col = i % 6, row = i / 6;
            skinTile[i] = { 32.f + col * 161.f, 100.f + row * 148.f, 153, 140 };
        }
        const float x0 = 1180, y0 = 112;
        const float dy = kOptRowH;
        for (int i = 0; i < 3; ++i) optTextBtn[i] = { x0 + i * 131.f, y0, 125, 28 };
        for (int i = 0; i < kRefreshCount; ++i) optRefreshBtn[i] = { x0 + i * 98.f, y0 + dy, 92, 28 };
        for (int i = 0; i < 3; ++i) optSmoothBtn[i] = { x0 + i * 131.f, y0 + 2 * dy, 125, 28 };
        for (int i = 0; i < 2; ++i) optPredictBtn[i] = { x0 + i * 196.f, y0 + 3 * dy, 190, 28 };
        for (int i = 0; i < 2; ++i) optLiveBtn[i] = { x0 + i * 196.f, y0 + 4 * dy, 190, 28 };
        for (int i = 0; i < 2; ++i) optTipsBtn[i] = { x0 + i * 196.f, y0 + 5 * dy, 190, 28 };
        for (int i = 0; i < 2; ++i) optGuardBtn[i] = { x0 + i * 196.f, y0 + 6 * dy, 190, 28 };
        for (int i = 0; i < 4; ++i) optThreadsBtn[i] = { x0 + i * 98.f, y0 + 7 * dy, 92, 28 };
    }

    void drawSkinPreview(int i, const Rect& rt) {
        // drawn at the original 182 x 200 and scaled to the tile
        const float sc = std::min(rt.w / 182.f, rt.h / 200.f);
        const bool hotT = hovered(rt);
        save(); translate(rt.x + (rt.w - 182.f * sc) * 0.5f, rt.y); scale(sc, sc);
        drawSkinPreview0(i, { 0, 0, 182, 200 }, hotT);
        restore();
    }
    void drawSkinPreview0(int i, const Rect& r, bool hotT) {
        const Skin& s = kSkins[i];
        const Color bg = rgb(s.bg), pt = rgb(s.panelTop), pb = rgb(s.panelBot), ed = rgb(s.edge), ac = rgb(s.accent), vi = rgb(s.violet);
        const Color wa = rgb(s.warm), tx = rgb(s.text), dm = rgb(s.dim), kh = rgb(s.knobHi), kl = rgb(s.knobLo), kp = rgb(s.knobPtr), ins = rgb(s.inset);
        const bool cur = optSkin == i, hot = hotT;
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 8); fillColor(bg); fill();
        strokeColor(cur ? cAccent : (hot ? cDim : cPanelEdge)); strokeWidth(cur ? 2.5f : 1.f); stroke();
        const Rect p = { r.x + 10, r.y + 10, r.w - 20, 132 };
        beginPath(); roundedRect(p.x, p.y, p.w, p.h, 6); fillPaint(linearGradient(0, p.y, 0, p.y + p.h, pt, pb)); fill(); strokeColor(ed); strokeWidth(1); stroke();
        // mini display with a wave and an LFO-like trace
        const Rect d = { p.x + 8, p.y + 10, p.w - 16, 56 };
        beginPath(); roundedRect(d.x, d.y, d.w, d.h, 4); fillColor(ins); fill();
        beginPath();
        for (int k = 0; k <= 60; ++k) { const float u = k / 60.f; const float y = d.cy() - std::sin(u * 2.f * kPi * 1.5f) * (1.f - 0.4f * u) * d.h * 0.35f; if (k == 0) moveTo(d.x + 4 + u * (d.w - 8), y); else lineTo(d.x + 4 + u * (d.w - 8), y); }
        strokeColor(ac); strokeWidth(1.6f); stroke();
        beginPath();
        for (int k = 0; k <= 60; ++k) { const float u = k / 60.f; const float y = d.cy() + 8 - (u < 0.5f ? 4.f * u - 1.f : 3.f - 4.f * u) * d.h * 0.2f; if (k == 0) moveTo(d.x + 4 + u * (d.w - 8), y); else lineTo(d.x + 4 + u * (d.w - 8), y); }
        strokeColor(withA(vi, 0.8f)); strokeWidth(1.2f); stroke();
        // three knobs
        for (int k = 0; k < 3; ++k) {
            const float kx = p.x + 28 + k * 54, ky = p.y + 98, kr = 13;
            beginPath(); circle(kx, ky, kr); fillPaint(radialGradient(kx - kr * 0.3f, ky - kr * 0.4f, 0, kr * 1.3f, kh, kl)); fill();
            const float a0 = 0.75f * kPi, a1 = 2.25f * kPi, av = a0 + (a1 - a0) * (0.3f + 0.25f * k);
            beginPath(); arc(kx, ky, kr + 4, a0, a1, CW); strokeColor(rgb(s.faint)); strokeWidth(2); stroke();
            beginPath(); arc(kx, ky, kr + 4, a0, av, CW); strokeColor(k == 2 ? wa : ac); strokeWidth(2); stroke();
            if (k == 1) { beginPath(); arc(kx, ky, kr + 7.5f, av - 0.5f, av + 0.6f, CW); strokeColor(withA(vi, 0.7f)); strokeWidth(1.6f); stroke(); }
            beginPath(); moveTo(kx + std::cos(av) * kr * 0.3f, ky + std::sin(av) * kr * 0.3f); lineTo(kx + std::cos(av) * kr * 0.8f, ky + std::sin(av) * kr * 0.8f);
            strokeColor(kp); strokeWidth(1.8f); lineCap(ROUND); stroke(); lineCap(BUTT);
        }
        label(r.x + 14, r.y + 160, s.name, 12, tx, ALIGN_LEFT | ALIGN_MIDDLE);
        label(r.x + 14, r.y + 180, cur ? "active" : "click to use", 8.5f, cur ? ac : dm, ALIGN_LEFT | ALIGN_MIDDLE);
        for (int k = 0; k < 5; ++k) {
            const Rgb* cc[5] = { &s.accent, &s.violet, &s.warm, &s.blue, &s.rose };
            beginPath(); circle(r.x + r.w - 62 + k * 11, r.y + 170, 4); fillColor(rgb(*cc[k])); fill();
        }
    }

    void optRow(float y, const char* title, const char* help) {
        label(1048, y + 14, title, 10.5f, cText, ALIGN_LEFT | ALIGN_MIDDLE);
        label(1048, y + 40, help, 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);   // help sits under the buttons
    }
    void drawSetup() {
        for (int i = 0; i < kSkinCount; ++i) drawSkinPreview(i, skinTile[i]);
        static const char* const rn[kRefreshCount] = { "30 Hz", "60 Hz", "120 Hz", "Max" };
        static const char* const sn[3] = { "Off", "Light", "Smooth" };
        const float y0 = 112, dy = kOptRowH;
        static const char* const tsn[3] = { "Normal", "Large", "Larger" };
        optRow(y0, "Text size", "Size of every label. Large is the default.");
        for (int i = 0; i < 3; ++i) button(optTextBtn[i], tsn[i], optTextSize == i, hovered(optTextBtn[i]));
        optRow(y0 + dy, "Refresh rate", "How often the live displays redraw. Higher costs more GPU.");
        for (int i = 0; i < kRefreshCount; ++i) button(optRefreshBtn[i], rn[i], optRefresh == i, hovered(optRefreshBtn[i]));
        optRow(y0 + 2 * dy, "Motion smoothing", "Eases values between audio blocks so rings and dots glide.");
        for (int i = 0; i < 3; ++i) button(optSmoothBtn[i], sn[i], optSmooth == i, hovered(optSmoothBtn[i]));
        optRow(y0 + 3 * dy, "LFO motion", "Predicted: dots run at full frame rate from the LFO's rate.");
        button(optPredictBtn[0], "Measured", !optPredict, hovered(optPredictBtn[0]));
        button(optPredictBtn[1], "Predicted", optPredict, hovered(optPredictBtn[1]));
        optRow(y0 + 4 * dy, "Live displays", "Off freezes the animated displays to save CPU / GPU.");
        button(optLiveBtn[0], "Off", !optLive, hovered(optLiveBtn[0]));
        button(optLiveBtn[1], "On", optLive, hovered(optLiveBtn[1]));
        optRow(y0 + 5 * dy, "Tooltips", "Hover help on every control.");
        button(optTipsBtn[0], "Off", !optTips, hovered(optTipsBtn[0]));
        button(optTipsBtn[1], "On", optTips, hovered(optTipsBtn[1]));
        optRow(y0 + 6 * dy, "CPU guard", "Near overload, fades the quietest voices instead of crackling. VST3 real-time only.");
        button(optGuardBtn[0], "Off", !optGuard, hovered(optGuardBtn[0]));
        button(optGuardBtn[1], "On", optGuard, hovered(optGuardBtn[1]));
        {
            const int n = shared ? shared->threadsActive.load() : 1;
            char hb[96]; std::snprintf(hb, sizeof hb, "Voices on several cores. This instance: %d thread%s. Saved with the session.", n, n == 1 ? "" : "s");
            optRow(y0 + 7 * dy, "Render threads", hb);
            static const char* const tn[4] = { "Off", "2", "4", "Auto" };
            for (int i = 0; i < 4; ++i) button(optThreadsBtn[i], tn[i], threadsIndex() == i, hovered(optThreadsBtn[i]));
        }

        // info card
        const Rect& c = setupInfoCard;
        char b[512];
        float y = c.y + 52;
        auto line = [&](const char* k, const std::string& v, const Color& col) {
            label(c.x + 24, y, k, 9.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            labelS(c.x + 200, y, v, 9.5f, col, ALIGN_LEFT | ALIGN_MIDDLE);
            y += 24;
        };
        {   // 6.0: version + links (the synth's page, the music)
            const float ly = y;
            line("Version", "Sydalesis 6  ·  6.0.2  (formerly Driftfield)", cText);
            itchRect = { c.x + 420, ly - 10, 150, 20 }; linkRect = { c.x + 600, ly - 10, 200, 20 };
            auto link = [&](const Rect& r, const char* t) {
                const bool hot = hovered(r);
                labelS(r.x, r.cy(), t, 9.5f, hot ? cText : cAccent, ALIGN_LEFT | ALIGN_MIDDLE);
                if (hot) { beginPath(); moveTo(r.x, r.y + r.h - 1); lineTo(r.x + r.w - 10, r.y + r.h - 1); strokeColor(withA(cAccent, 0.6f)); strokeWidth(1); stroke(); }
            };
            link(itchRect, "sydalesis.itch.io  ↗");
            link(linkRect, "music: sydalesis.bandcamp.com  ↗");
        }
        line("Options", "global for every instance, saved in " + driftDir() + "/options-v3.txt", cDim);
        std::snprintf(b, sizeof(b), "%.0f fps measured  (timer %s)", fpsMeasured, tickerOn ? "active" : "unavailable, using host idle");
        line("UI frame rate", b, cText);
        std::snprintf(b, sizeof(b), "%.0f frames/s from the engine (one per audio block)", telMeasured);
        line("Telemetry", shared ? std::string(b) : std::string("not available in this host (separate UI process)"), shared ? cText : cWarn);
        std::snprintf(b, sizeof(b), "%d in the 6.0 bank  ·  %d in Legacy 5.x  ·  %d user", (int)kFactoryBank6Count, (int)(kFactoryPresetCount - kFactoryBank6Count), (int)userPresets.size());
        line("Presets", b, cText);
        y += 10;
        line("Presets folder", driftDir() + "/Presets3", cAccent);
        line("Wavetables", driftDir() + "/Wavetables", cAccent);
        line("Grain samples", driftDir() + "/Samples", cAccent);
        line("Favourites", driftDir() + "/favorites-v3.txt", cAccent);
        std::snprintf(b, sizeof(b), "%d controls mapped  ·  right-click a knob, fader or slider  ·  default map: %s", mappedCount(), (driftDir() + "/midi-learn-v3.txt").c_str());
        line("MIDI learn", b, cText);
        y += 10;
        label(c.x + 24, y, "Drop .wav files into the Wavetables or Samples folders (sub-folders become categories). They show up in the browsers after reopening the editor.", 9, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        drawLibraryFolders();
    }
    // 6.0: extra library folders (wavetables / samples, e.g. on another drive)
    Rect folderAdd[2], folderDel[2][6];
    std::vector<std::string> folderCache[2]; double folderCacheAt = -10.0;   // re-read at most once a second
    const std::vector<std::string>& foldersCached(int k) {
        if (secNow() - folderCacheAt > 1.0) { folderCache[0] = drift::lib::extraFolders(false); folderCache[1] = drift::lib::extraFolders(true); folderCacheAt = secNow(); }
        return folderCache[k];
    }
    void drawLibraryFolders() {
        const Rect& c = setupInfoCard;
        const float x = c.x + 1000, w = c.w - 1016;
        float y = c.y + 52;
        for (int k = 0; k < 2; ++k) {
            letterSp(1.f); label(x, y, k ? "SAMPLE FOLDERS (grain / spectral)" : "WAVETABLE FOLDERS", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE); letterSp(0.f);
            folderAdd[k] = { x + w - 110, y - 11, 110, 22 };
            button(folderAdd[k], "+ ADD FOLDER", false, hovered(folderAdd[k]), 8.5f);
            y += 22;
            const auto& v = foldersCached(k);
            if (v.empty()) { label(x + 6, y, k ? "none (only " "Documents/Sydalesis/Samples)" : "none (only Documents/Sydalesis/Wavetables)", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE); y += 20; }
            for (int i = 0; i < (int)v.size() && i < 6; ++i) {
                std::error_code ec;
                const bool ok = std::filesystem::is_directory(drift::lib::fromU8(v[i]), ec);
                save(); scissor(x, y - 9, w - 30, 18);
                labelS(x + 6, y, v[i], 8.8f, ok ? cAccent : cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
                restore();
                folderDel[k][i] = { x + w - 22, y - 9, 18, 18 };
                label(folderDel[k][i].cx(), folderDel[k][i].cy(), "✕", 9, hovered(folderDel[k][i]) ? cWarn : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
                y += 20;
            }
            y += 16;
        }
        label(x, y, "ADD: pick any .wav inside the folder. Missing drives show in orange.", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    bool libraryFolderClick(float mx, float my) {
        for (int k = 0; k < 2; ++k) {
            if (folderAdd[k].contains(mx, my)) {
                browserKind = k ? bkFolderSmp : bkFolderWt;
                FileBrowserOptions opts;
                opts.title = k ? "Pick any .wav in the sample folder to add" : "Pick any .wav in the wavetable folder to add";
                openFileBrowser(opts);
                return true;
            }
            auto v = drift::lib::extraFolders(k == 1);
            for (int i = 0; i < (int)v.size() && i < 6; ++i) if (folderDel[k][i].contains(mx, my)) {
                v.erase(v.begin() + i); drift::lib::setExtraFolders(k == 1, v); folderCacheAt = -10.0; ensureLibrary(true); repaint(); return true;
            }
        }
        return false;
    }
    bool setupClick(float mx, float my) {
        if (libraryFolderClick(mx, my)) return true;
        for (int i = 0; i < kSkinCount; ++i) if (skinTile[i].contains(mx, my)) { optSkin = i; applySkin(i); saveOptions(); repaint(); return true; }
        for (int i = 0; i < 3; ++i) if (optTextBtn[i].contains(mx, my)) { optTextSize = i; saveOptions(); repaint(); return true; }
        for (int i = 0; i < kRefreshCount; ++i) if (optRefreshBtn[i].contains(mx, my)) { optRefresh = i; saveOptions(); startTicker(); repaint(); return true; }
        for (int i = 0; i < 3; ++i) if (optSmoothBtn[i].contains(mx, my)) { optSmooth = i; saveOptions(); repaint(); return true; }
        for (int i = 0; i < 2; ++i) if (optPredictBtn[i].contains(mx, my)) { optPredict = i == 1; saveOptions(); repaint(); return true; }
        for (int i = 0; i < 2; ++i) if (optLiveBtn[i].contains(mx, my)) { optLive = i == 1; saveOptions(); repaint(); return true; }
        for (int i = 0; i < 2; ++i) if (optTipsBtn[i].contains(mx, my)) { optTips = i == 1; tipShown = -1; saveOptions(); repaint(); return true; }
        for (int i = 0; i < 4; ++i) if (optThreadsBtn[i].contains(mx, my)) {
            optThreads = kThreadVals[i];
            instThreads = i == 0 ? "" : (i == 3 ? "auto" : std::to_string(kThreadVals[i]));
            UI::setState("threads", instThreads.c_str());
            saveOptions(); repaint(); return true;
        }
        for (int i = 0; i < 2; ++i) if (optGuardBtn[i].contains(mx, my)) { optGuard = i == 1; if (shared) shared->guardOn.store(optGuard ? 1 : 0); saveOptions(); repaint(); return true; }
        return false;
    }
