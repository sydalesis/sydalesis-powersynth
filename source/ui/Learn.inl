// ===================================================================================== MIDI learn
// (included inside class DriftfieldUI)
// Right-click a knob, fader or slider (without dragging) for the context menu. The plug-in does the
// learning on the audio thread: the UI arms a parameter in the shared block, the next learnable CC
// claims it. The map lives in the shared block and is saved with the session (state "midiMap").

    bool ctxOpen = false;
    int  ctxParam = -1;
    Rect ctxRect {};
    std::vector<int> ctxItems;          // action ids, see ctxLabel
    enum { caLearn, caCancelLearn, caForget, caClearAll, caSaveDefault, caLoadDefault, caDefaultValue };
    bool learnWaiting = false;
    uint32_t lastCcSeq = 0;
    bool rightMoved = false;
    int  rightCtx = -1;                 // parameter whose context menu opens on release if the drag never moved

    std::string flashText; double flashUntil = 0.0;
    void flash(const char* t) { flashText = t; flashUntil = secNow() + 2.5; repaint(); }
    void drawFlash() {
        if (flashText.empty()) return;
        const double now = secNow();
        if (now > flashUntil) { flashText.clear(); return; }
        const float a = (float)std::min(1.0, (flashUntil - now) / 0.4);
        uiFontSize(10.5f);
        Rectangle<float> bounds; textBounds(0, 0, flashText.c_str(), nullptr, bounds);
        const float w = bounds.getWidth() + 28.f, x = kW * 0.5f - w * 0.5f, y = kH - 40.f;
        beginPath(); roundedRect(x, y, w, 24, 6); fillColor(withA(shade(34.f), 0.96f * a)); fill(); strokeColor(withA(cWarm, 0.7f * a)); strokeWidth(1); stroke();
        labelS(kW * 0.5f, y + 12.5f, flashText, 10.5f, withA(cText, a), ALIGN_CENTER | ALIGN_MIDDLE);
    }
    std::vector<int> ccForParam = std::vector<int>(kParamCount, -1);
    void rebuildCcIndex() {
        std::fill(ccForParam.begin(), ccForParam.end(), -1);
        if (!shared) return;
        for (int c = 0; c < drift::kMidiMapSize; ++c) { const int p = shared->midiMap[c].load(std::memory_order_relaxed); if (p >= 0 && p < (int)kParamCount) ccForParam[(size_t)p] = c; }
    }

    static bool learnableKind(int kind) { return kind == ckKnob || kind == ckSlider || kind == ckVFader; }
    std::string midiLearnFile() const { return drift::lib::u8(drift::lib::fromU8(driftDir()) / "midi-learn-v3.txt"); }

    // p = view index of a control on the page; the map stores host indices (layer 2 = + kLayerBOffset)
    int ccOfParam(int p) const { if (p < 0 || p >= (int)kParamCount) return -1; const uint32_t h = hp((uint32_t)p); return ccForParam[(size_t)h]; }
    int mappedCount() const {
        if (!shared) return 0;
        int n = 0;
        for (int c = 0; c < drift::kMidiMapSize; ++c) n += shared->midiMap[c].load(std::memory_order_relaxed) >= 0 ? 1 : 0;
        return n;
    }
    void pushMidiMap() { if (shared) setState("midiMap", shared->mapString().c_str()); }

    std::string ctxLabel(int a) const {
        char b[64];
        switch (a) {
        case caLearn: return "MIDI Learn";
        case caCancelLearn: return "Cancel MIDI Learn";
        case caForget: { std::snprintf(b, sizeof(b), "Forget CC %d", ccOfParam(ctxParam)); return b; }
        case caClearAll: { std::snprintf(b, sizeof(b), "Clear all mappings (%d)", mappedCount()); return b; }
        case caSaveDefault: return "Save mappings as default";
        case caLoadDefault: return "Load default mappings";
        case caDefaultValue: return "Reset to default value";
        default: return "";
        }
    }

    void openCtx(int p, float x, float y) {
        ctxParam = p;
        ctxItems.clear();
        if (shared) {
            const bool armed = shared->learnArm.load() == (int)hp((uint32_t)p);
            ctxItems.push_back(armed ? caCancelLearn : caLearn);
            if (ccOfParam(p) >= 0) ctxItems.push_back(caForget);
            ctxItems.push_back(caDefaultValue);
            if (mappedCount() > 0) ctxItems.push_back(caClearAll);
            if (mappedCount() > 0) ctxItems.push_back(caSaveDefault);
            ctxItems.push_back(caLoadDefault);
        } else ctxItems.push_back(caDefaultValue);
        const float w = 196.f, h = 30.f + 21.f * (float)ctxItems.size() + (shared ? 0.f : 16.f);
        if (x + w > kW - 6) x = kW - 6 - w;
        if (y + h > kH - 6) y = kH - 6 - h;
        ctxRect = { std::max(6.f, x), std::max(60.f, y), w, h };
        ctxOpen = true;
        repaint();
    }
    Rect ctxItemRect(size_t k) const { return { ctxRect.x + 5, ctxRect.y + 25 + 21.f * (float)k, ctxRect.w - 10, 20 }; }

    void drawCtx() {
        const Rect& m = ctxRect;
        beginPath(); roundedRect(m.x + 3, m.y + 4, m.w, m.h, 7); fillColor(Color(0, 0, 0, 0.35f)); fill();
        beginPath(); roundedRect(m.x, m.y, m.w, m.h, 7); fillColor(shade(27.f)); fill(); strokeColor(cPanelEdge); strokeWidth(1); stroke();
        letterSp(1.0f);
        std::string title = ctxParam >= 0 ? std::string(P((uint32_t)ctxParam).name) : std::string();
        for (auto& ch : title) ch = (char)std::toupper((unsigned char)ch);
        labelS(m.x + 11, m.y + 14, title, 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
        for (size_t k = 0; k < ctxItems.size(); ++k) {
            const Rect r = ctxItemRect(k);
            const bool hot = r.contains(hoverX, hoverY);
            if (hot) { beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4); fillColor(shade(44.f)); fill(); }
            const int a = ctxItems[k];
            labelS(r.x + 7, r.cy() + 0.5f, ctxLabel(a), 10.5f, a == caLearn || a == caCancelLearn ? cAccent : cText, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        if (!shared) labelS(m.x + 11, m.y + m.h - 12, "MIDI learn needs the editor in the plug-in process", 8.5f, cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
    }

    void ctxClick(float x, float y) {
        ctxOpen = false;
        int a = -1;
        for (size_t k = 0; k < ctxItems.size(); ++k) if (ctxItemRect(k).contains(x, y)) { a = ctxItems[k]; break; }
        if (a < 0 || ctxParam < 0) return;
        const uint32_t p = (uint32_t)ctxParam;
        switch (a) {
        case caLearn: if (shared) { shared->learnArm.store((int)hp((uint32_t)ctxParam)); learnWaiting = true; } break;
        case caCancelLearn: if (shared) { shared->learnArm.store(-1); learnWaiting = false; } break;
        case caForget: if (shared) { const int c = ccOfParam(ctxParam); if (c >= 0) { shared->midiMap[c].store(-1); shared->ccSeq.fetch_add(1); pushMidiMap(); } } break;
        case caClearAll: if (shared) { int m[drift::kMidiMapSize]; for (int& v : m) v = -1; shared->setMap(m); pushMidiMap(); } break;
        case caSaveDefault: if (shared) {
            std::error_code ec;
            std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
            std::ofstream out(drift::lib::fromU8(midiLearnFile()));
            out << shared->mapString() << "\n";
            flash("MIDI mappings saved as default");
        } break;
        case caLoadDefault: if (shared) {
            std::ifstream in(drift::lib::fromU8(midiLearnFile()));
            std::string line; int m[drift::kMidiMapSize];
            if (std::getline(in, line) && drift::midiMapFromString(line.c_str(), m)) { shared->setMap(m); pushMidiMap(); flash("Default MIDI mappings loaded"); }
            else flash("No default MIDI mappings saved yet");
        } break;
        case caDefaultValue: setParamUser(p, P(p).def); break;
        default: break;
        }
        repaint();
    }

    // called every frame: mirror CC-driven values, notice a finished learn
    void tickMidiLearn() {
        if (!shared) return;
        const uint32_t seq = shared->ccSeq.load(std::memory_order_acquire);
        if (learnWaiting && shared->learnArm.load() < 0) {
            learnWaiting = false;
            pushMidiMap();      // the plug-in assigned it on the audio thread; store it with the session
            repaint();
        }
        if (shared->learnArm.load(std::memory_order_relaxed) >= 0 || !flashText.empty()) repaint();   // pulse / fade
        if (seq == lastCcSeq) return;
        lastCcSeq = seq;
        rebuildCcIndex();
        bool any = false;
        for (int c = 0; c < drift::kMidiMapSize; ++c) {
            const int p = shared->midiMap[c].load(std::memory_order_relaxed);
            if (p < 0 || p >= (int)kParamCount) continue;
            const float v = shared->ccValue[c].load(std::memory_order_relaxed);
            if (v != hostVals[p]) {
                hostVals[p] = v;
                const int vi = viewOfParam((uint32_t)p);
                if (vi >= 0 && dragParam != vi) { values[vi] = v; any = true; }
            }
        }
        (void)any;
        repaint();
    }

    // small CC badges next to mapped controls; armed control pulses
    void drawLearnBadges() {
        if (!shared) return;
        const int armed = shared->learnArm.load(std::memory_order_relaxed);
        if (armed < 0 && mappedCount() == 0) return;
        for (int i = 0; i < (int)ctls.size(); ++i) {
            const Ctl& c = ctls[i];
            if (!learnableKind(c.kind) || !visible(c)) continue;
            const int p = (int)c.p;
            const int cc = ccOfParam(p);
            const bool isArmed = armed >= 0 && (int)hp((uint32_t)p) == armed;
            if (cc < 0 && !isArmed) continue;
            const Rect r = ctlRect(c);
            float bx, by;
            if (c.kind == ckKnob) { bx = r.x + r.w * 0.9f; by = r.y - r.w - 7.f; }
            else { bx = r.x + r.w - 2.f; by = r.y - 5.f; }
            if (isArmed) {
                const float ph = 0.5f + 0.5f * std::sin((float)secNow() * 8.f);
                if (c.kind == ckKnob) { beginPath(); circle(r.x, r.y, r.w + 4.f); strokeColor(withA(cWarm, 0.35f + 0.6f * ph)); strokeWidth(2.f); stroke(); }
                else { beginPath(); roundedRect(r.x - 3, r.y - 3, r.w + 6, r.h + 6, 4); strokeColor(withA(cWarm, 0.35f + 0.6f * ph)); strokeWidth(2.f); stroke(); }
                labelS(bx, by, "LEARN", 7.5f, cWarm, ALIGN_CENTER | ALIGN_MIDDLE);
                continue;
            }
            char b[12]; std::snprintf(b, sizeof(b), "CC%d", cc);
            beginPath(); roundedRect(bx - 13, by - 5.5f, 26, 11, 3); fillColor(tint(cWarm, 0.22f, 0.95f)); fill();
            labelS(bx, by + 0.5f, b, 7.f, cWarm, ALIGN_CENTER | ALIGN_MIDDLE);
        }
    }
