// ===================================================================================== 5.1.1 keyboard strip
// (included inside class DriftfieldUI)
// A playable keyboard under every page. It shows what every voice of both layers is doing (5.1.2, neutral): a held
// key looks pressed; a white dot rises with per-note pressure (MPE / poly AT / note expression); a circle around it
// grows with whole-keyboard aftertouch; slide (CC74) is a faint line across the key; pitch bend (incl. VST3
// note-expression tuning) is a marker sliding off the key toward the bent pitch.
// Mouse notes are MPE: each one gets its own channel, sideways = bend, up / down = slide, holding = pressure.
// Notes and wheels go to the audio thread through SharedBlock::pushUiMidi.

    static constexpr float kKbY = kH, kKbH2 = kKbH;   // strip origin / height
    static constexpr int kKbKeys = 61;                 // 5 octaves + 1
    int kbBase = 36;                                    // lowest key shown (C2)
    bool kbHold = false, kbSus = false, kbChordOn = false, kbVelPos = true;
    int kbChord[8] = { 0, 7, 12 }; int kbChordN = 3;
    struct KbNote { bool on = false; int note = 0, ch = 1; float x0 = 0, y0 = 0; double t0 = 0; int pressSent = -1, bendSent = 8192, slideSent = 64; bool latched = false; int chordN = 0; int chord[8] = {}; };
    KbNote kbN[16];                                     // index = channel (1..15 used)
    int kbNextCh = 1;
    int kbDragCh = -1;                                  // channel of the note under the mouse
    int kbWheel = -1;                                   // 0 pitch, 1 mod while dragging
    float kbPitch = 0.f, kbMod = 0.f;                   // -1..1, 0..1
    uint32_t kbPcSeen = 0;
    Rect kbPanic, kbPitchR, kbModR, kbOctDn, kbOctUp, kbHoldR, kbSusR, kbChordR, kbVelR, kbKeysR;
    drift::Telemetry kbTel2;

    void kbSend(uint8_t st, uint8_t d1, uint8_t d2) { if (shared) shared->pushUiMidi(st, d1, d2); }
    static bool kbBlack(int n) { const int pc = ((n % 12) + 12) % 12; return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10; }
    float kbWhiteW() const { return kbKeysR.w / 36.f; }
    // x range of a key in the strip
    void kbKeyX(int n, float& x0, float& x1) const {
        const float ww = kbWhiteW();
        int whites = 0;
        for (int k = kbBase; k < n; ++k) if (!kbBlack(k)) ++whites;
        if (!kbBlack(n)) { x0 = kbKeysR.x + whites * ww; x1 = x0 + ww; }
        else { const float c = kbKeysR.x + whites * ww; x0 = c - ww * 0.32f; x1 = c + ww * 0.32f; }
    }
    int kbNoteAt(float mx, float my) const {
        if (!kbKeysR.contains(mx, my)) return -1;
        const float blackH = kbKeysR.h * 0.6f;
        if (my < kbKeysR.y + blackH)
            for (int n = kbBase; n < kbBase + kKbKeys; ++n) if (kbBlack(n)) { float a, b; kbKeyX(n, a, b); if (mx >= a && mx < b) return n; }
        for (int n = kbBase; n < kbBase + kKbKeys; ++n) if (!kbBlack(n)) { float a, b; kbKeyX(n, a, b); if (mx >= a && mx < b) return n; }
        return -1;
    }

    void kbLayout() {
        const float y = kKbY + 6, h = kKbH2 - 12;
        kbPanic = { 8, y, 64, h };
        kbPitchR = { 80, y, 22, h }; kbModR = { 108, y, 22, h };
        kbOctDn = { 138, y, 24, 20 }; kbOctUp = { 204, y, 24, 20 };
        kbHoldR = { 138, y + 25, 43, 20 }; kbSusR = { 185, y + 25, 43, 20 };
        kbChordR = { 138, y + 50, 90, 20 }; kbVelR = { 138, y + 75, 90, 17 };
        kbKeysR = { 236, y, kW - 236 - 30, h };
    }

    // ------------------------------------------------------------------ drawing
    void drawKeyboard() {
        kbLayout();
        kbTick();
        beginPath(); rect(0, kKbY, kW, kKbH2); fillColor(shade(14.f)); fill();
        beginPath(); moveTo(0, kKbY + 0.5f); lineTo(kW, kKbY + 0.5f); strokeColor(cPanelEdge); strokeWidth(1); stroke();
        // panic
        {
            const bool hot = kbPanic.contains(hoverX, hoverY);
            beginPath(); roundedRect(kbPanic.x, kbPanic.y, kbPanic.w, kbPanic.h, 5);
            fillColor(hot ? withA(cWarn, 0.45f) : withA(cWarn, 0.22f)); fill(); strokeColor(cWarn); strokeWidth(1.2f); stroke();
            letterSp(1.2f); label(kbPanic.cx(), kbPanic.cy() - 6, "PANIC", 10.f, hot ? cText : cWarn, ALIGN_CENTER | ALIGN_MIDDLE); letterSp(0.f);
            label(kbPanic.cx(), kbPanic.cy() + 9, "all off", 7.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        // wheels
        auto wheel = [&](const Rect& r, float v01, const char* name, bool centred) {
            beginPath(); roundedRect(r.x, r.y, r.w, r.h - 12, 4); fillColor(cInset); fill(); strokeColor(cPanelEdge); stroke();
            const float yy = r.y + 4 + (1.f - v01) * (r.h - 20);
            if (centred) { beginPath(); moveTo(r.x + 3, r.y + (r.h - 12) * 0.5f); lineTo(r.x + r.w - 3, r.y + (r.h - 12) * 0.5f); strokeColor(cFaint); stroke(); }
            beginPath(); roundedRect(r.x + 2, yy - 4, r.w - 4, 8, 2); fillColor(cAccent); fill();
            label(r.cx(), r.y + r.h - 4, name, 7.f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        };
        wheel(kbPitchR, 0.5f + 0.5f * kbPitch, "PITCH", true);
        wheel(kbModR, kbMod, "MOD", false);
        // octave
        button(kbOctDn, "-", false, kbOctDn.contains(hoverX, hoverY), 11.f);
        button(kbOctUp, "+", false, kbOctUp.contains(hoverX, hoverY), 11.f);
        { char b[24]; const std::string lo = noteName(kbBase), hi = noteName(kbBase + kKbKeys - 1); std::snprintf(b, sizeof b, "%s-%s", lo.c_str(), hi.c_str());
          label((kbOctDn.x + kbOctUp.x + kbOctUp.w) * 0.5f, kbOctDn.cy(), b, 8.f, cDim, ALIGN_CENTER | ALIGN_MIDDLE); }
        button(kbHoldR, "HOLD", kbHold, kbHoldR.contains(hoverX, hoverY), 8.5f);
        button(kbSusR, "SUS", kbSus, kbSusR.contains(hoverX, hoverY), 8.5f);
        {
            std::string c = "CHORD";
            if (kbChordOn) { c += " "; for (int i = 0; i < kbChordN; ++i) { if (i) c += "·"; c += std::to_string(kbChord[i]); } }
            button(kbChordR, c.c_str(), kbChordOn, kbChordR.contains(hoverX, hoverY), 8.f);
        }
        button(kbVelR, kbVelPos ? "VEL: CLICK HEIGHT" : "VEL: FIXED 100", false, kbVelR.contains(hoverX, hoverY), 7.f);

        // collect what is sounding: layer 1 always, layer 2 when layering is on
        struct Lit { int note, ch, gate; float vel, press, at, slide, bend, amp; int layer; };
        Lit lit[2 * drift::kTelVoices]; int nl = 0;
        auto take = [&](const drift::Telemetry& t, int layer) {
            for (int i = 0; i < drift::kTelVoices; ++i) {
                const drift::TelVoice& v = t.v[i];
                if (!v.active || v.amp < 0.002f) continue;
                lit[nl++] = { v.note, v.ch, v.gate, v.vel, v.notePress, v.chanAT, v.slide, v.bend, v.amp, layer };
            }
        };
        if (shared) {
            take(shared->tel, 0);   // read live (small fields; a torn read only flickers one frame)
            if (ival(pLyMode) > 0 && drift::telRead(shared->tel2, kbTel2)) take(kbTel2, 1);
        }
        // auto-scroll: keep played notes in view (not while the mouse is playing)
        if (kbDragCh < 0) {
            int lo = 999, hi = -1;
            for (int i = 0; i < nl; ++i) if (lit[i].gate) { lo = std::min(lo, lit[i].note); hi = std::max(hi, lit[i].note); }
            if (hi >= 0) {
                while (lo < kbBase && kbBase > 0) kbBase -= 12;
                while (hi > kbBase + kKbKeys - 1 && kbBase + kKbKeys - 1 < 127) kbBase += 12;
            }
        }
        const float top = kbKeysR.y, H = kbKeysR.h, blackH = H * 0.6f, ww = kbWhiteW();
        const bool split = ival(pLyMode) == 2;
        const int root = ival(pTnSource) > 0 ? ival(pTnRoot) : -1;
        auto drawKey = [&](int n, bool black) {
            float x0, x1; kbKeyX(n, x0, x1);
            const float h = black ? blackH : H;
            beginPath(); rect(x0 + 0.5f, top, x1 - x0 - 1.f, h);
            fillColor(black ? cKeyBlack : mixc(cBg, cKeyWhite, 0.34f)); fill();   // skin key colours, white keys toned down
            strokeColor(shade(12.f)); strokeWidth(1); stroke();
            // 5.1.2: neutral display - a held key looks pressed; the white dot rises with per-note pressure; a circle
            // around it grows with whole-keyboard aftertouch; slide is a faint line across the key; bend a marker
            for (int i = 0; i < nl; ++i) {
                if (lit[i].note != n) continue;
                const float cx = (x0 + x1) * 0.5f;
                if (lit[i].gate) {
                    beginPath(); rect(x0 + 1.5f, top + 1, x1 - x0 - 3.f, h - 2);
                    fillColor(black ? mixc(cKeyBlack, cKeyWhite, 0.18f) : mixc(cBg, cKeyWhite, 0.20f)); fill();
                    strokeColor(withA(cText, 0.55f)); strokeWidth(1.f); stroke();
                } else {   // still ringing after release
                    beginPath(); rect(x0 + 1.5f, top + 1, x1 - x0 - 3.f, h - 2);
                    strokeColor(withA(cText, 0.25f * std::min(1.f, lit[i].amp * 4.f))); strokeWidth(1.f); stroke();
                    continue;
                }
                if (std::fabs(lit[i].slide) > 0.02f) {
                    const float sy = top + 6 + (1.f - (lit[i].slide * 0.5f + 0.5f)) * (h - 12);
                    beginPath(); moveTo(x0 + 3, sy); lineTo(x1 - 3, sy); strokeColor(withA(cText, 0.4f)); strokeWidth(1.5f); stroke();
                }
                const float pr = std::max(0.f, std::min(1.f, lit[i].press));
                const float dy = top + h - 7 - pr * (h - 14);
                if (lit[i].at > 0.02f) {
                    const float a = std::min(1.f, lit[i].at);
                    beginPath(); circle(cx, dy, 4.5f + 9.f * a); strokeColor(withA(cText, 0.25f + 0.6f * a)); strokeWidth(1.4f); stroke();
                }
                beginPath(); circle(cx, dy, 3.2f); fillColor(cText); fill();
                if (std::fabs(lit[i].bend) > 0.05f) {
                    const float tx = cx + lit[i].bend * ww * 7.f / 12.f, yy = top + h * 0.5f;
                    beginPath(); moveTo(cx, yy); lineTo(tx, yy); strokeColor(withA(cText, 0.7f)); strokeWidth(1.6f); stroke();
                    beginPath(); circle(tx, yy, 2.6f); fillColor(withA(cText, 0.9f)); fill();
                }
                if (lit[i].layer == 1) { beginPath(); rect(x0 + 1.5f, top + 1, x1 - x0 - 3.f, 2); fillColor(withA(cViolet, 0.8f)); fill(); }
            }
            if (!black && root >= 0 && ((n % 12) + 12) % 12 == root) { beginPath(); circle((x0 + x1) * 0.5f, top + h - 7, 2.2f); fillColor(cAccent); fill(); }
            if (!black && n % 12 == 0) labelOn((x0 + x1) * 0.5f, top + h - 16, noteName(n), 7.f, cKeyLabel, ALIGN_CENTER | ALIGN_MIDDLE, mixc(cBg, cKeyWhite, 0.34f));
        };
        save(); scissor(kbKeysR.x, kbKeysR.y, kbKeysR.w + 1, kbKeysR.h + 1);
        for (int n = kbBase; n < kbBase + kKbKeys; ++n) if (!kbBlack(n)) drawKey(n, false);
        for (int n = kbBase; n < kbBase + kKbKeys; ++n) if (kbBlack(n)) drawKey(n, true);
        if (split) {
            const int sn = ival(pLySplit);
            if (sn > kbBase && sn < kbBase + kKbKeys) {
                float x0, x1; kbKeyX(sn, x0, x1);
                const float xs = kbBlack(sn) ? (x0 + x1) * 0.5f : x0;
                beginPath(); moveTo(xs, top - 1); lineTo(xs, top + H + 1); strokeColor(cViolet); strokeWidth(2.f); stroke();
                label(xs - 4, top + 8, "L1", 7.5f, cAccent, ALIGN_RIGHT | ALIGN_MIDDLE);
                label(xs + 4, top + 8, "L2", 7.5f, cViolet, ALIGN_LEFT | ALIGN_MIDDLE);
            }
        } else if (ival(pLyMode) == 3 || ival(pLyMode) == 4) {
            label(kbKeysR.x + kbKeysR.w - 6, top + 8, ival(pLyMode) == 3 ? "velocity layers: soft = L1, hard = L2" : "pressure layers: light = L1, firm = L2", 7.5f, cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        restore();
    }

    // ------------------------------------------------------------------ playing
    int kbVelocity(float my, bool black) const {
        if (!kbVelPos) return 100;
        const float h = black ? kbKeysR.h * 0.6f : kbKeysR.h;
        const float f = std::max(0.f, std::min(1.f, (my - kbKeysR.y) / h));
        return std::max(20, std::min(127, (int)(30 + 97 * f)));    // top of the key soft, bottom hard
    }
    void kbNoteOnAt(int note, float mx, float my) {
        // reuse the channel of a latched copy of this key (toggle it off) when holding
        for (int c = 1; c < 16; ++c) if (kbN[c].on && kbN[c].latched && kbN[c].note == note) { kbNoteOff(c); return; }
        int ch = -1;
        for (int k = 0; k < 15 && ch < 0; ++k) { const int c = 1 + (kbNextCh - 1 + k) % 15; if (!kbN[c].on) ch = c; }
        if (ch < 0) return;
        kbNextCh = ch % 15 + 1;
        KbNote& kn = kbN[ch];
        kn = KbNote(); kn.on = true; kn.note = note; kn.ch = ch; kn.x0 = mx; kn.y0 = my; kn.t0 = secNow();
        const int vel = kbVelocity(my, kbBlack(note));
        kbSend((uint8_t)(0xE0 | ch), 0, 64); kbSend((uint8_t)(0xB0 | ch), 74, 64); kbSend((uint8_t)(0xD0 | ch), 0, 0);
        kn.chordN = kbChordOn ? kbChordN : 1;
        for (int i = 0; i < kn.chordN; ++i) {
            kn.chord[i] = std::max(0, std::min(127, note + (kbChordOn ? kbChord[i] : 0)));
            kbSend((uint8_t)(0x90 | ch), (uint8_t)kn.chord[i], (uint8_t)vel);
        }
        kbDragCh = ch;
    }
    void kbNoteOff(int ch) {
        KbNote& kn = kbN[ch];
        if (!kn.on) return;
        for (int i = 0; i < kn.chordN; ++i) kbSend((uint8_t)(0x80 | ch), (uint8_t)kn.chord[i], 0);
        kn.on = false; kn.latched = false;
    }
    void kbAllOff() { for (int c = 1; c < 16; ++c) kbNoteOff(c); }
    void kbDrag(float mx, float my) {
        if (kbDragCh < 0) return;
        KbNote& kn = kbN[kbDragCh];
        const float semi = kbWhiteW() * 7.f / 12.f;
        const float range = std::max(1.f, values[pBendRange]);
        const float st = (mx - kn.x0) / semi;
        const int b = std::max(0, std::min(16383, (int)(8192 + st / range * 8191.f)));
        if (b != kn.bendSent) { kbSend((uint8_t)(0xE0 | kn.ch), (uint8_t)(b & 127), (uint8_t)(b >> 7)); kn.bendSent = b; }
        const int sl = std::max(0, std::min(127, (int)(64 + (kn.y0 - my) / kbKeysR.h * 127.f)));
        if (sl != kn.slideSent) { kbSend((uint8_t)(0xB0 | kn.ch), 74, (uint8_t)sl); kn.slideSent = sl; }
        repaint();
    }
    void kbWheelAt(float my) {
        const Rect& r = kbWheel == 0 ? kbPitchR : kbModR;
        const float v = std::max(0.f, std::min(1.f, 1.f - (my - r.y - 4) / (r.h - 20)));
        if (kbWheel == 0) { kbPitch = v * 2.f - 1.f; const int b = std::max(0, std::min(16383, (int)(8192 + kbPitch * 8191.f))); kbSend(0xE0, (uint8_t)(b & 127), (uint8_t)(b >> 7)); }
        else { kbMod = v; kbSend(0xB0, 1, (uint8_t)(v * 127.f + 0.5f)); }
        repaint();
    }
    // per frame: pressure rises while a mouse note is held; program changes from MIDI refresh the editor
    void kbTick() {
        const double now = secNow();
        for (int c = 1; c < 16; ++c) {
            KbNote& kn = kbN[c];
            if (!kn.on || kn.latched) continue;
            const int pr = std::max(0, std::min(127, (int)((now - kn.t0) / 1.2 * 127.0)));
            if (pr != kn.pressSent) { kbSend((uint8_t)(0xD0 | c), (uint8_t)pr, 0); kn.pressSent = pr; }
        }
        if (shared) {
            const uint32_t s = shared->pcSerial.load(std::memory_order_acquire);
            if (s != kbPcSeen) { kbPcSeen = s; const int idx = shared->pcIndex.load(); if (idx >= 0) refreshFromPluginPreset(idx); }
            const int dp = shared->dbgPage.exchange(-1); if (dp >= 0 && dp < kPages) { page = dp; repaint(); }
        }
    }
    // the plug-in loaded factory preset idx on its own (MIDI program change): show it
    void refreshFromPluginPreset(int idx) {
        if (idx < 0 || idx >= (int)kFactoryPresetCount) return;
        static float pv[kParamCount];
        presetValues((uint32_t)idx, pv);
        for (uint32_t i = 0; i < kParamCount; ++i) values[i] = pv[i];
        std::string st[kStateCount];
        presetStates((uint32_t)idx, st);
        for (int k = 0; k < kStateCount; ++k) { stateValue[k] = st[k]; applyStateLocal(k); }
        currentPreset = idx; presetEdited = false;
        noteRecent(idx);
        repaint();
    }

    bool kbMouseDown(float mx, float my) {
        if (my < kKbY) return false;
        kbLayout();
        if (kbPanic.contains(mx, my)) {
            if (shared) shared->panicReq.store(1, std::memory_order_release);
            for (int c = 1; c < 16; ++c) { kbN[c].on = false; kbN[c].latched = false; }
            kbHold = false; if (kbSus) { kbSus = false; }
            flash("Panic: all notes and tails off");
            repaint(); return true;
        }
        if (kbPitchR.contains(mx, my)) { kbWheel = 0; kbWheelAt(my); return true; }
        if (kbModR.contains(mx, my)) { kbWheel = 1; kbWheelAt(my); return true; }
        if (kbOctDn.contains(mx, my)) { kbBase = std::max(0, kbBase - 12); repaint(); return true; }
        if (kbOctUp.contains(mx, my)) { kbBase = std::min(127 - kKbKeys + 1, kbBase + 12); repaint(); return true; }
        if (kbHoldR.contains(mx, my)) { kbHold = !kbHold; if (!kbHold) for (int c = 1; c < 16; ++c) if (kbN[c].latched) kbNoteOff(c); repaint(); return true; }
        if (kbSusR.contains(mx, my)) { kbSus = !kbSus; kbSend(0xB0, 64, kbSus ? 127 : 0); repaint(); return true; }
        if (kbChordR.contains(mx, my)) {
            if (!kbChordOn) {
                // learn from what is held right now (any keyboard), else a drone-friendly root / fifth / octave
                int notes[32], n = 0;
                if (shared) for (int i = 0; i < drift::kTelVoices && n < 32; ++i) { const drift::TelVoice& v = shared->tel.v[i]; if (v.active && v.gate) notes[n++] = v.note; }
                std::sort(notes, notes + n); n = (int)(std::unique(notes, notes + n) - notes);
                if (n >= 2) { kbChordN = std::min(8, n); for (int i = 0; i < kbChordN; ++i) kbChord[i] = notes[i] - notes[0]; }
                else { kbChordN = 3; kbChord[0] = 0; kbChord[1] = 7; kbChord[2] = 12; }
            }
            kbChordOn = !kbChordOn; repaint(); return true;
        }
        if (kbVelR.contains(mx, my)) { kbVelPos = !kbVelPos; repaint(); return true; }
        const int n = kbNoteAt(mx, my);
        if (n >= 0) { kbNoteOnAt(n, mx, my); repaint(); }
        return true;
    }
    bool kbMouseUp() {
        bool used = false;
        if (kbWheel == 0) { kbPitch = 0.f; kbSend(0xE0, 0, 64); used = true; }   // the pitch wheel springs back
        if (kbWheel >= 0) { kbWheel = -1; used = true; }
        if (kbDragCh >= 0) {
            if (kbHold) kbN[kbDragCh].latched = true;
            else kbNoteOff(kbDragCh);
            kbDragCh = -1; used = true;
        }
        if (used) repaint();
        return used;
    }
    bool kbMotion(float mx, float my) {
        if (kbWheel >= 0) { kbWheelAt(my); return true; }
        if (kbDragCh >= 0) { kbDrag(mx, my); return true; }
        return false;
    }
