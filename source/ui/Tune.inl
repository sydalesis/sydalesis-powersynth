// ===================================================================================== TUNE page (4.0)
// (included inside class DriftfieldUI)
// Source (Equal / Library / Scala / MTS-ESP), the built-in scale list, root / reference / amount,
// note-expression settings, and three views of the active tuning: the period as a ring, the keyboard
// with each key's offset from 12-TET, and the degree table.

    Rect tnSrcCard, tnLibCard, tnSetCard, tnViewCard;
    Rect tnScaleRow[drift::kBuiltinScales];
    Rect tnLoadScl, tnLoadKbm, tnClearScl, tnExportBtn;
    Rect tnRing, tnKeys, tnTable;
    drift::ScaleDef tnUser; drift::KbmDef tnUserKbm; bool tnUserOk = false; std::string tnUserErr;
    float tnTab[128]; bool tnMember[128];
    uint32_t tnNeLast = 0; double tnNeTime = -10.0;

    void buildTune() {
        tnSrcCard  = { 16, 72, 380, 430 };
        tnLibCard  = { 408, 72, 620, 430 };
        tnSetCard  = { 1040, 72, 544, 430 };
        tnViewCard = { 16, 514, 1568, 470 };
        panel(tnSrcCard, "TUNING SOURCE", pgTune);
        panel(tnLibCard, "SCALE LIBRARY", pgTune);
        panel(tnSetCard, "ROOT · REFERENCE · NOTE EXPRESSION", pgTune);
        panel(tnViewCard, "ACTIVE TUNING", pgTune);
        choice(pTnSource, { 32, 108, 348, 30 }, 4, pgTune);
        tnLoadScl  = { 32, 330, 168, 28 };
        tnLoadKbm  = { 212, 330, 168, 28 };
        tnClearScl = { 32, 366, 168, 28 };
        tnExportBtn = { 212, 366, 168, 28 };
        const int half = (drift::kBuiltinScales + 1) / 2;
        for (int i = 0; i < drift::kBuiltinScales; ++i) {
            const int col = i / half, row = i % half;
            tnScaleRow[i] = { 424.f + col * 298.f, 106.f + row * 23.f, 290, 21 };
        }
        // root / reference / amount / glide / snap
        choice(pTnRoot, { 1056, 112, 512, 28 }, 12, pgTune);
        knob(pTnRef, 1100, 210, 22, "A4 REF", pgTune);
        knob(pTnAmount, 1200, 210, 22, "AMOUNT", pgTune);
        knob(pTnGlide, 1300, 210, 22, "RETUNE", pgTune);
        tog(pTnSnap, { 1380, 196, 180, 28 }, "SNAP TO SCALE", pgTune);
        knob(pNeVibDepth, 1100, 356, 20, "VIB DEPTH", pgTune);
        knob(pNeVibRate, 1200, 356, 20, "VIB RATE", pgTune);
        tnRing  = { 40, 552, 420, 410 };
        tnKeys  = { 490, 560, 1070, 210 };
        tnTable = { 490, 790, 1070, 180 };
    }

    // --- helpers -------------------------------------------------------------------------------
    void refreshUserScale() {
        const std::string scl = drift::tnUnescape(stateValue[skScl]), kbm = drift::tnUnescape(stateValue[skKbm]);
        tnUserErr.clear();
        tnUserOk = !scl.empty() && drift::parseScl(scl.c_str(), tnUser, &tnUserErr);
        tnUserKbm = drift::KbmDef();
        if (!kbm.empty()) { std::string e2; if (!drift::parseKbm(kbm.c_str(), tnUserKbm, &e2) && tnUserErr.empty()) tnUserErr = "keyboard map: " + e2; }
    }
    const drift::ScaleDef* activeScale() const {
        const int src = ival(pTnSource);
        if (src == 1) return &drift::builtinScales()[std::max(0, std::min(drift::kBuiltinScales - 1, ival(pTnScale)))];
        if (src == 2 && tnUserOk) return &tnUser;
        return nullptr;
    }
    // effective pitch per key (what the engine plays), into tnTab
    void computeTuneView() {
        const int src = ival(pTnSource);
        const float refOff = 12.f * std::log2(values[pTnRef] / 440.f), amt = values[pTnAmount];
        for (int k = 0; k < 128; ++k) { tnTab[k] = (float)k + refOff; tnMember[k] = true; }
        if (src == 3) {
            if (shared && shared->mtsConnected.load()) for (int k = 0; k < 128; ++k) tnTab[k] = (float)k + (shared->mtsPitch[k] - (float)k) * amt;
            else for (int k = 0; k < 128; ++k) tnTab[k] = (float)k;
            return;
        }
        const drift::ScaleDef* sc = activeScale();
        if (!sc) return;
        float t[128];
        drift::buildTuningTable(*sc, src == 2 && tnUserKbm.valid ? &tnUserKbm : nullptr, ival(pTnRoot), values[pTnRef], values[pTnSnap] >= 0.5f, t, tnMember);
        for (int k = 0; k < 128; ++k) { const float et = (float)k + refOff; tnTab[k] = et + (t[k] - et) * amt; }
    }
    // the pitch as written in the scale if it is a ratio, else a small-integer ratio within 0.1 cent
    static std::string degreeText(const drift::ScaleDef& sc, int i) {
        if (i == 0) return "1/1";
        if (std::strchr(sc.txt[i], '/')) return sc.txt[i];
        if (sc.txt[i][0] && !std::strchr(sc.txt[i], '.')) return std::string(sc.txt[i]) + "/1";
        return ratioFor(sc.c[i]);
    }
    static std::string ratioFor(double cents) {
        const double r = std::pow(2.0, cents / 1200.0);
        for (int d = 1; d <= 32; ++d) {
            const int n = (int)std::lround(r * d);
            if (n <= 0) continue;
            if (std::fabs(1200.0 * std::log2((double)n / d) - cents) < 0.1) {
                int a = n, b = d; while (b) { const int t = a % b; a = b; b = t; }
                char buf[32]; std::snprintf(buf, sizeof(buf), "%d/%d", n / a, d / a);
                return buf;
            }
        }
        return "";
    }
    static const char* pcName(int pc) { static const char* const n[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }; return n[(pc % 12 + 12) % 12]; }

    // --- drawing -------------------------------------------------------------------------------
    void drawTune() {
        computeTuneView();
        const int src = ival(pTnSource);
        // source card
        {
            const Rect& c = tnSrcCard;
            static const char* const what[4] = {
                "12-tone equal temperament. A4 reference still applies.",
                "One of the built-in scales on the right, on the chosen root.",
                "A Scala .scl file, optionally with a .kbm keyboard map.",
                "Follow an MTS-ESP master plug-in (e.g. MTS-ESP Mini) across every instance." };
            labelS(c.x + 16, c.y + 84, what[std::max(0, std::min(3, src))], 9.f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            float y = c.y + 116;
            auto line = [&](const char* k, const std::string& v, const Color& col) {
                label(c.x + 16, y, k, 9.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
                save(); scissor(c.x + 110, y - 8, c.w - 126, 16);
                labelS(c.x + 110, y, v, 9.5f, col, ALIGN_LEFT | ALIGN_MIDDLE);
                restore();
                y += 20;
            };
            const drift::ScaleDef* sc = activeScale();
            if (src == 3) {
                const bool on = shared && shared->mtsConnected.load();
                line("MTS-ESP", on ? "master connected" : (shared ? "no master running (playing equal)" : "status not available in this host"), on ? cAccent : cWarn);
                if (on) {
                    char nm[64]; uint32_t s0;
                    do { s0 = shared->mtsSeq.load(); std::memcpy(nm, shared->mtsScaleName, sizeof nm); } while ((s0 & 1u) || s0 != shared->mtsSeq.load());
                    nm[63] = 0;
                    line("Scale", nm[0] ? nm : "(unnamed)", cText);
                }
            } else if (sc) {
                line("Scale", sc->name[0] && src != 2 ? sc->name : (sc->desc[0] ? sc->desc : "Scala"), cText);
                char b[96];
                std::snprintf(b, sizeof b, "%d notes, period %.2f cents%s", sc->n, sc->c[sc->n], sc->twelve ? "  ·  12-key layout" : "  ·  one key per degree");
                line("Layout", b, cDim);
                if (src == 1) line("About", sc->desc, cDim);
            } else if (src == 2) {
                line("Scala", tnUserErr.empty() ? "no file loaded (playing equal)" : tnUserErr, tnUserErr.empty() ? cDim : cWarn);
            }
            if (src == 2) {
                std::string km = stateValue[skKbm].empty() ? "none (linear from the root)" : (tnUserKbm.valid ? "loaded" : "could not be read");
                if (tnUserKbm.valid) { char b[96]; std::snprintf(b, sizeof b, "%d-key map, middle key %d, ref key %d = %.2f Hz", tnUserKbm.size, tnUserKbm.middle, tnUserKbm.refNote, tnUserKbm.refFreq); km = b; }
                line("Keyboard", km, cDim);
            }
            // note expression activity
            if (shared) {
                const uint32_t ne = (uint32_t)shared->neSeen.load();
                if (ne != tnNeLast) { tnNeLast = ne; tnNeTime = secNow(); }
            }
            const bool neLive = secNow() - tnNeTime < 1.5;
            label(c.x + 16, c.y + 236, "Note expression", 9.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 110, c.y + 236, neLive ? "receiving (VST3)" : "idle  ·  Studio One: Tuning, Volume, Pan, Vibrato, Expression, Brightness", 9.f, neLive ? cAccent : cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            button(tnLoadScl, "LOAD .SCL", false, hovered(tnLoadScl), 9.5f);
            button(tnLoadKbm, "LOAD .KBM", false, hovered(tnLoadKbm), 9.5f);
            button(tnClearScl, "CLEAR SCALA", false, hovered(tnClearScl), 9.5f);
            button(tnExportBtn, "SAVE AS .SCL", false, hovered(tnExportBtn), 9.5f);
            if (src != 2) { dimRect(tnLoadKbm); dimRect(tnClearScl); }
            if (!sc) dimRect(tnExportBtn);
            label(c.x + 16, c.y + 402, "Tunings folder: Documents\\Sydalesis\\Tunings", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        // library
        {
            const int cur = ival(pTnScale);
            for (int i = 0; i < drift::kBuiltinScales; ++i) {
                const Rect& r = tnScaleRow[i];
                const bool sel = src == 1 && i == cur, hot = hovered(r);
                if (sel || hot) { beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4); fillColor(sel ? tint(cAccent, 0.2f) : shade(36.f)); fill(); }
                const drift::ScaleDef& s = drift::builtinScales()[i];
                label(r.x + 8, r.cy() + 0.5f, s.name, 10.f, sel ? cAccent : (src == 1 ? cText : cDim), ALIGN_LEFT | ALIGN_MIDDLE);
                char b[16]; std::snprintf(b, sizeof b, "%d", s.n);
                label(r.x + r.w - 8, r.cy() + 0.5f, b, 8.5f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
            }
            if (src != 1) label(tnLibCard.x + 16, tnLibCard.y + tnLibCard.h - 14, "Click a scale to switch the source to Library.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        // settings card captions
        {
            const Rect& c = tnSetCard;
            label(c.x + 16, c.y + 32, "ROOT (degree 0 of the scale; automatable for drone-root changes)", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 340, c.y + 166, "SNAP: keys outside a 12-key scale", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 340, c.y + 180, "play the nearest scale note", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 16, c.y + 198, "AMOUNT blends equal temperament (0) into the tuning (1)  ·  RETUNE glides held notes when the root, scale or master changes", 8.f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            beginPath(); moveTo(c.x + 16, c.y + 222); lineTo(c.x + c.w - 16, c.y + 222); strokeColor(cGrid); strokeWidth(1); stroke();
            label(c.x + 16, c.y + 240, "NOTE EXPRESSION VIBRATO (VST3 Vibrato lane; depth in semitones at full lane)", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 260, c.y + 274, "Tuning lane: ±120 semitones, absolute per note", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 260, c.y + 290, "Volume, Pan: per-note gain and position", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 260, c.y + 306, "Expression: drives the Pressure source", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 260, c.y + 322, "Brightness: drives the Slide source", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            label(c.x + 260, c.y + 338, "Works alongside MPE; both add up.", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        drawTuneRing();
        drawTuneKeys();
        drawTuneTable();
    }

    void drawTuneRing() {
        const Rect& r = tnRing;
        const float cx = r.cx(), cy = r.cy() + 4, rad = 150.f;
        const drift::ScaleDef* sc = activeScale();
        const int src = ival(pTnSource);
        const double period = sc ? sc->c[sc->n] : 1200.0;
        beginPath(); circle(cx, cy, rad); strokeColor(cGrid); strokeWidth(1.5f); stroke();
        // 12-TET reference ticks (per 100 cents of the period)
        const int ticks = (int)std::floor(period / 100.0 + 0.5);
        for (int i = 0; i < ticks; ++i) {
            const float a = (float)(i * 100.0 / period) * 2.f * kPi - kPi * 0.5f;
            beginPath(); moveTo(cx + std::cos(a) * (rad - 8), cy + std::sin(a) * (rad - 8)); lineTo(cx + std::cos(a) * (rad + 8), cy + std::sin(a) * (rad + 8));
            strokeColor(cFaint); strokeWidth(1); stroke();
        }
        // held notes (degree highlight)
        bool held[kTuneMaxDegreesUi] = {};
        const int n = sc ? sc->n : 12;
        const int root = 60 + ival(pTnRoot);
        forVoices([&](const drift::TelVoice& v, bool) {
            const int d = ((v.note - root) % n + n) % n;
            if (d < kTuneMaxDegreesUi) held[d] = true;
        });
        for (int i = 0; i < n && i < kTuneMaxDegreesUi; ++i) {
            const double c = sc ? sc->c[i] : i * 100.0;
            const float a = (float)(c / period) * 2.f * kPi - kPi * 0.5f;
            const float x = cx + std::cos(a) * rad, y = cy + std::sin(a) * rad;
            const bool mem = !sc || !sc->twelve || ((sc->mask >> i) & 1) || values[pTnSnap] < 0.5f;
            beginPath(); circle(x, y, held[i] ? 7.f : 5.f);
            fillColor(held[i] ? cWarm : (mem ? cAccent : withA(cAccent, 0.3f))); fill();
            if (n <= 24) {
                const float lx = cx + std::cos(a) * (rad + 26), ly = cy + std::sin(a) * (rad + 26);
                const std::string rt = sc ? degreeText(*sc, i) : std::string();
                char b[32]; std::snprintf(b, sizeof b, "%.0f", c);
                labelS(lx, ly - 5, rt.empty() ? std::string(b) : rt, 8.5f, mem ? cText : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
                if (!rt.empty()) label(lx, ly + 6, b, 7.f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            }
        }
        const char* title = src == 0 ? "Equal" : (src == 3 ? "MTS-ESP" : (sc ? (src == 1 ? sc->name : "Scala") : "Equal"));
        label(cx, cy - 8, title, 12.f, cText, ALIGN_CENTER | ALIGN_MIDDLE);
        char b[64];
        if (src == 3) std::snprintf(b, sizeof b, "per-key from the master");
        else std::snprintf(b, sizeof b, "root %s  ·  A4 %.1f Hz", pcName(ival(pTnRoot)), values[pTnRef]);
        label(cx, cy + 10, b, 8.5f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
        if (src != 0 && values[pTnAmount] < 0.999f) { std::snprintf(b, sizeof b, "amount %.0f%%", values[pTnAmount] * 100.f); label(cx, cy + 26, b, 8.5f, cWarm, ALIGN_CENTER | ALIGN_MIDDLE); }
    }
    static constexpr int kTuneMaxDegreesUi = 129;

    void drawTuneKeys() {
        const Rect& r = tnKeys;
        inset(r);
        // three octaves from the C below the root region: keys 48..83
        const int k0 = 48, nk = 36;
        const float kw = (r.w - 20) / nk, base = r.y + r.h - 46, top = r.y + 22, mid = (top + base) * 0.5f;
        bool on[128] = {};
        forVoices([&](const drift::TelVoice& v, bool) { if (v.note >= 0 && v.note < 128) on[v.note] = true; });
        beginPath(); moveTo(r.x + 10, mid); lineTo(r.x + r.w - 10, mid); strokeColor(cGrid); strokeWidth(1); stroke();
        label(r.x + 14, top - 8, "+50 c", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(r.x + 14, base + 6, "-50 c", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        for (int i = 0; i < nk; ++i) {
            const int k = k0 + i;
            const float x = r.x + 10 + i * kw;
            const float semis = tnTab[k];
            const int nearest = (int)std::lround(semis);
            const float dev = (semis - (float)nearest) * 100.f;
            const float h = std::max(-50.f, std::min(50.f, dev)) / 50.f * (mid - top);
            const bool member = tnMember[k];
            beginPath(); rect(x + 3, h >= 0 ? mid - h : mid, kw - 6, std::fabs(h) + 0.5f);
            fillColor(on[k] ? cWarm : (member ? (dev >= 0 ? cAccent : cViolet) : withA(cDim, 0.4f))); fill();
            // key
            const int pc = k % 12; const bool black = pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
            beginPath(); roundedRect(x + 1, base + 14, kw - 2, 28, 2); fillColor(on[k] ? cWarm : (black ? cKeyBlack : cKeyWhite)); fill();
            char b[16];
            std::snprintf(b, sizeof b, "%s%d", pcName(nearest), nearest / 12 - 1);
            label(x + kw * 0.5f, base + 4, b, 7.f, member ? cText : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            std::snprintf(b, sizeof b, "%+.0f", dev);
            label(x + kw * 0.5f, base + 32, b, 7.f, on[k] ? cBg : (black ? cKeyWhite : cKeyBlack), ALIGN_CENTER | ALIGN_MIDDLE);
        }
        label(r.x + r.w - 12, top - 8, "each key: the note it sounds nearest to, and its offset in cents (bar)", 7.5f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }

    void drawTuneTable() {
        const Rect& r = tnTable;
        inset(r);
        const drift::ScaleDef* sc = activeScale();
        if (!sc) {
            label(r.cx(), r.cy(), ival(pTnSource) == 3 ? "MTS-ESP supplies a pitch per key; see the keyboard above." : "Equal temperament: every step 100 cents.", 10.f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            return;
        }
        const int n = std::min(sc->n, 48);
        const int perRow = 16;
        const float cw = (r.w - 20) / perRow;
        for (int i = 0; i <= n; ++i) {
            const int row = i / perRow, col = i % perRow;
            if (row > 2) break;
            const float x = r.x + 10 + col * cw, y = r.y + 14 + row * 56;
            const double c = sc->c[i];
            char b[32];
            std::snprintf(b, sizeof b, "%d", i);
            label(x + 6, y + 6, b, 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            const std::string rt = degreeText(*sc, i);
            labelS(x + 6, y + 22, rt.empty() ? std::string("-") : rt, 10.f, cText, ALIGN_LEFT | ALIGN_MIDDLE);
            std::snprintf(b, sizeof b, "%.2f c", c);
            label(x + 6, y + 38, b, 8.f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        if (sc->n > 47) label(r.x + r.w - 12, r.y + r.h - 10, "(first 48 degrees shown)", 7.5f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }

    // --- input -----------------------------------------------------------------------------------
    int tnBrowse = 0;   // 1 scl, 2 kbm
    static std::string tuningsDir() { return drift::lib::u8(drift::lib::fromU8(drift::lib::dataDir()) / "Tunings"); }
    void openTuningFile(int which) {
        tnBrowse = which;
        browserKind = bkTuning;
        FileBrowserOptions opts;
        opts.title = which == 1 ? "Load a Scala scale (.scl)" : "Load a Scala keyboard map (.kbm)";
        const std::string dir = tuningsDir();
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(dir), ec);
        opts.startDir = dir.c_str();
        openFileBrowser(opts);
    }
    void tuningFileSelected(const char* path) {
        std::ifstream in(drift::lib::fromU8(path), std::ios::binary);
        std::stringstream ss; ss << in.rdbuf();
        std::string text = ss.str();
        if (text.size() > 64 * 1024) { flash("File too large for a tuning"); return; }
        std::string err;
        if (tnBrowse == 1) {
            drift::ScaleDef sc;
            if (!drift::parseScl(text.c_str(), sc, &err)) { flash(("Not a valid .scl: " + err).c_str()); return; }
            pushUndo(-1);
            stateValue[skScl] = drift::tnEscape(text); setState(kStateKeys[skScl], stateValue[skScl].c_str());
            if (ival(pTnSource) != 2) setParamUser(pTnSource, 2.f);
            flash((std::string("Loaded ") + std::filesystem::path(drift::lib::fromU8(path)).filename().u8string()).c_str());
        } else {
            drift::KbmDef km;
            if (!drift::parseKbm(text.c_str(), km, &err)) { flash(("Not a valid .kbm: " + err).c_str()); return; }
            pushUndo(-1);
            stateValue[skKbm] = drift::tnEscape(text); setState(kStateKeys[skKbm], stateValue[skKbm].c_str());
            flash("Keyboard map loaded");
        }
        refreshUserScale();
        repaint();
    }
    void exportScl() {
        const drift::ScaleDef* sc = activeScale();
        if (!sc) return;
        std::string nm = ival(pTnSource) == 1 ? sc->name : (sc->desc[0] ? sc->desc : "scala");
        std::string file;
        for (char ch : nm) file += (std::isalnum((unsigned char)ch) || ch == '-' ? ch : '_');
        const std::string dir = tuningsDir();
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(dir), ec);
        std::ofstream out(drift::lib::fromU8(dir) / (file + ".scl"));
        out << "! " << file << ".scl\n!\n" << (sc->desc[0] ? sc->desc : sc->name) << "\n " << sc->n << "\n!\n";
        for (int i = 1; i <= sc->n; ++i) {
            const std::string rt = std::strchr(sc->txt[i], '/') ? std::string(sc->txt[i]) : std::string();
            if (!rt.empty()) out << " " << rt << "\n";
            else { char b[32]; std::snprintf(b, sizeof b, " %.5f\n", sc->c[i]); for (char* q = b; *q; ++q) if (*q == ',') *q = '.'; out << b; }
        }
        flash(("Saved " + file + ".scl in the Tunings folder").c_str());
    }
    bool tuneClick(float mx, float my) {
        for (int i = 0; i < drift::kBuiltinScales; ++i) if (tnScaleRow[i].contains(mx, my)) {
            pushUndo(-1);
            editParameter(pTnScale, true); setParam(pTnScale, (float)i); editParameter(pTnScale, false);
            if (ival(pTnSource) != 1) { editParameter(pTnSource, true); setParam(pTnSource, 1.f); editParameter(pTnSource, false); }
            return true;
        }
        if (tnLoadScl.contains(mx, my)) { openTuningFile(1); return true; }
        if (tnLoadKbm.contains(mx, my) && ival(pTnSource) == 2) { openTuningFile(2); return true; }
        if (tnClearScl.contains(mx, my) && ival(pTnSource) == 2) {
            pushUndo(-1);
            stateValue[skScl].clear(); stateValue[skKbm].clear();
            setState(kStateKeys[skScl], ""); setState(kStateKeys[skKbm], "");
            refreshUserScale(); repaint(); return true;
        }
        if (tnExportBtn.contains(mx, my)) { exportScl(); return true; }
        return false;
    }
