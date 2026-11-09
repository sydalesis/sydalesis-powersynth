// ===================================================================================== FX + SEQ pages
// (included inside class DriftfieldUI)

    void layoutFxCards() {
        for (int s = 0; s < kFxUnits; ++s) fxCardR[s] = { 10.f + (s % 4) * 395.f, 62.f + (s / 4) * 312.f, 387, 304 };
    }
    Ctl& fxKnob(int unit, uint32_t p, float x, float y, float r, const char* l) { Ctl& c = knob(p, x, y, r, l, pgFx); c.fxUnit = unit; return c; }
    void buildFx() {
        layoutFxCards();
        auto onTog = [&](int u) { Ctl& c = tog(kFxOnParam[u], { 450, 8, 58, 20 }, nullptr, pgFx); c.fxUnit = u; };
        for (int u = 0; u < kFxUnits; ++u) onTog(u);
        // drive
        fxKnob(fxDrive, pDrvAmt, 130, 130, 30, "DRIVE"); fxKnob(fxDrive, pDrvTone, 260, 130, 22, "TONE");
        // phaser
        { const uint32_t k[4] = { pPhMix, pPhRate, pPhDepth, pPhFb }; static const char* const l[4] = { "MIX", "RATE", "DEPTH", "FEEDBACK" };
          for (int i = 0; i < 4; ++i) fxKnob(fxPhaser, k[i], 80 + i * 120.f, 130, 22, l[i]); }
        // chorus
        { Ctl& c = choice(pChMode, { 14, 44, 492, 24 }, 4, pgFx); c.fxUnit = fxChorus; }
        { const uint32_t k[4] = { pChMix, pChRate, pChDepth, pChFb }; static const char* const l[4] = { "MIX", "RATE", "DEPTH", "FEEDBACK" };
          for (int i = 0; i < 4; ++i) fxKnob(fxChorus, k[i], 80 + i * 120.f, 150, 22, l[i]); }
        // delay
        { Ctl& c = selector(pDlMode, { 14, 44, 140, 22 }, true, pgFx); c.fxUnit = fxDelay; }
        { Ctl& c = tog(pDlSync, { 166, 45, 64, 20 }, "SYNC", pgFx); c.fxUnit = fxDelay; }
        { Ctl& c = selector(pDlDiv, { 236, 44, 90, 22 }, false, pgFx); c.fxUnit = fxDelay; c.visParam = (int)pDlSync; c.visVal = 1; }
        { const uint32_t k[4] = { pDlMix, pDlTime, pDlFb, pDlDamp }; static const char* const l[4] = { "MIX", "TIME", "FEEDBACK", "DAMPING" };
          for (int i = 0; i < 4; ++i) { Ctl& c = fxKnob(fxDelay, k[i], 80 + i * 120.f, 150, 22, l[i]); if (i == 1) { c.visParam = (int)pDlSync; c.visVal = 0; } } }
        // shifter
        fxKnob(fxShift, pFsShift, 90, 140, 30, "SHIFT");
        { const uint32_t k[3] = { pFsMix, pFsFb, pFsStereo }; static const char* const l[3] = { "MIX", "FEEDBACK", "STEREO" };
          for (int i = 0; i < 3; ++i) fxKnob(fxShift, k[i], 220 + i * 100.f, 140, 20, l[i]); }
        // tape
        { const uint32_t k[5] = { pTpDrive, pTpWow, pTpFlutter, pTpTone, pTpMix }; static const char* const l[5] = { "DRIVE", "WOW", "FLUTTER", "TONE", "MIX" };
          for (int i = 0; i < 5; ++i) fxKnob(fxTape, k[i], 62 + i * 98.f, 96, 19, l[i]); }
        fxKnob(fxTape, pTpWowRate, 160, 162, 14, "WOW SPEED");      // 5.1.6
        fxKnob(fxTape, pTpFlutRate, 258, 162, 14, "FLUT SPEED");
        // crusher
        { const uint32_t k[3] = { pCrBits, pCrDown, pCrMix }; static const char* const l[3] = { "BITS", "DOWNSAMPLE", "MIX" };
          for (int i = 0; i < 3; ++i) fxKnob(fxCrush, k[i], 100 + i * 160.f, 130, 24, l[i]); }
        // width
        fxKnob(fxWidth, pWdWidth, 140, 130, 28, "WIDTH"); fxKnob(fxWidth, pWdBassMono, 340, 130, 22, "BASS MONO");
        // eq
        { const uint32_t k[7] = { pEqLowG, pEqLowF, pEqMidG, pEqMidF, pEqMidQ, pEqHighG, pEqHighF }; static const char* const l[7] = { "LOW", "LOW HZ", "MID", "MID HZ", "Q", "HIGH", "HIGH HZ" };
          for (int i = 0; i < 7; ++i) fxKnob(fxEq, k[i], 46 + i * 71.f, 74, 17, l[i]); }
        // 4.2 resonator
        { Ctl& c = choice(pRsMode, { 14, 44, 492, 24 }, 6, pgFx); c.fxUnit = fxReso; }
        { const uint32_t k[4] = { pRsMix, pRsDecay, pRsBright, pRsWidth }; static const char* const l[4] = { "MIX", "DECAY", "BRIGHTNESS", "WIDTH" };
          for (int i = 0; i < 4; ++i) fxKnob(fxReso, k[i], 80 + i * 120.f, 126, 22, l[i]); }
        { Ctl& c = selector(pRsRoot, { 14, 178, 150, 22 }, true, pgFx); c.fxUnit = fxReso; }
        { Ctl& c = selector(pRsOct, { 176, 178, 130, 22 }, true, pgFx); c.fxUnit = fxReso; }
        // 5.0 spectral
        { Ctl& c = tog(pSxFreeze, { 14, 44, 160, 26 }, "FREEZE", pgFx); c.fxUnit = fxSpectral; }
        { const uint32_t k[4] = { pSxMix, pSxBlur, pSxShift, pSxTilt }; static const char* const l[4] = { "MIX", "BLUR", "SHIFT", "TILT" };
          for (int i = 0; i < 4; ++i) fxKnob(fxSpectral, k[i], 80 + i * 120.f, 130, 22, l[i]); }
        fxKnob(fxSpectral, pSxSpread, 80, 222, 18, "SPREAD"); fxKnob(fxSpectral, pSxGate, 200, 222, 18, "GATE");
        // 4.2 space
        { const uint32_t k[7] = { pSpMix, pSpSize, pSpDecay, pSpDamp, pSpPre, pSpMod, pSpWidth }; static const char* const l[7] = { "MIX", "SIZE", "DECAY", "DAMPING", "PRE", "MOD", "WIDTH" };
          for (int i = 0; i < 7; ++i) fxKnob(fxSpace, k[i], 46 + i * 71.f, 74, 17, l[i]); }
    }
    // resonator string pitches as the engine sets them (12-TET names; the engine applies the tuning)
    int resoNotes(int* out) const {
        const int mode = ival(pRsMode), base = 12 * (ival(pRsOct) + 1) + ival(pRsRoot);
        static const int iv[5][8] = { { 0, 7, 12, 19, 24, 31, 36, 43 }, { 0, 12, 24, 36, 48, 12, 24, 36 }, { 0, 4, 7, 11, 12, 14, 16, 19 },
                                      { 0, 3, 7, 10, 12, 14, 15, 19 }, { 0, 7, 12, 12, 19, 24, 0, 7 } };
        if (mode < 5) { for (int k = 0; k < 8; ++k) out[k] = base + iv[mode][k]; return 8; }
        int n = 0;
        forVoices([&](const drift::TelVoice& v, bool) { if (v.gate && n < 8) { bool d = false; for (int i = 0; i < n; ++i) if (out[i] == v.note) d = true; if (!d) out[n++] = v.note; } });
        return n;
    }

    void drawFx() {
        for (int s = 0; s < kFxUnits; ++s) {
            const int u = fxOrder[s];
            const Rect& c = fxCardR[s];
            const bool dragging = sd == sdFxCard && sdIndex == u;
            PanelDef pd{ c, "", pgFx };
            drawPanel(pd);
            if (sd == sdFxCard && fxDropSlot == s && !dragging) { beginPath(); roundedRect(c.x + 1, c.y + 1, c.w - 2, c.h - 2, 7); strokeColor(cAccent); strokeWidth(2); stroke(); }
            // grip + title
            const Rect grip = { c.x + 8, c.y + 7, 22, 22 };
            for (int k = 0; k < 3; ++k) for (int j = 0; j < 2; ++j) { beginPath(); circle(grip.x + 7 + j * 7, grip.y + 5 + k * 6, 1.6f); fillColor(hovered(grip) ? cAccent : cFaint); fill(); }
            char t[48]; std::snprintf(t, sizeof(t), "%d  %s", s + 1, kFxUnitNames[u]);
            letterSp(1.5f);
            label(c.x + 36, c.y + 18, t, 10.5f, values[kFxOnParam[u]] >= 0.5f ? cText : cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            letterSp(0.f);
            if (s < kFxUnits - 1) label(c.x + c.w - 8, c.y + c.h - 10, "→", 10, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
            drawFxVisual(u, c);
            if (values[kFxOnParam[u]] < 0.5f) dimRect({ c.x + 1, c.y + 34, c.w - 2, c.h - 35 });
            if (dragging) dimRect(c);
        }
        if (sd == sdFxCard) {
            const int u = sdIndex;
            beginPath(); roundedRect(hoverX - 90, hoverY - 16, 180, 32, 6); fillColor(tint(cAccent, 0.19f, 0.95f)); fill(); strokeColor(cAccent); stroke();
            label(hoverX, hoverY, kFxUnitNames[u], 11, cAccent, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        label(kW - 14, kH - 6, "chain runs 1 → 12 · drag ⠿ to reorder · Space (reverb) is optional and off in every factory preset", 7.5f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }
    void drawFxVisual(int u, const Rect& c) {
        const Rect v = { c.x + 14, c.y + 206, c.w - 28, 84 };
        switch (u) {
        case fxEq: {
            const Rect e = { c.x + 14, c.y + 122, c.w - 28, 168 };
            inset(e);
            beginPath(); moveTo(e.x, e.cy()); lineTo(e.x + e.w, e.cy()); strokeColor(cGrid); stroke();
            auto shelf = [](float f, float fc, float g, bool low) { const float x = std::log2(f / fc); const float s = 1.f / (1.f + std::exp((low ? 1.f : -1.f) * x * 3.f)); return g * s; };
            beginPath();
            for (int i = 0; i <= 120; ++i) {
                const float f = 20.f * std::pow(1000.f, i / 120.f);
                float db = shelf(f, values[pEqLowF], values[pEqLowG], true) + shelf(f, values[pEqHighF], values[pEqHighG], false);
                const float bw = 1.2f / std::max(0.2f, values[pEqMidQ]);
                const float d = std::log2(f / values[pEqMidF]) / bw;
                db += values[pEqMidG] * std::exp(-0.5f * d * d * 4.f);
                const float x = e.x + e.w * i / 120.f, y = e.cy() - db * (e.h / 36.f);
                if (i == 0) moveTo(x, y); else lineTo(x, y);
            }
            strokeColor(cAccent); strokeWidth(1.6f); stroke();
            break;
        }
        case fxShift: {
            inset(v);
            const float hz = values[pFsShift] + (shared ? tel.md[mdFxShift] * 200.f : 0.f);
            char b[32]; std::snprintf(b, sizeof(b), "%+.1f Hz", hz);
            label(v.cx(), v.cy() - 8, b, 18, cAccent, ALIGN_CENTER | ALIGN_MIDDLE);
            label(v.cx(), v.cy() + 18, "the same Hz shift for every partial: inharmonic, bell-like", 8, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            break;
        }
        case fxTape: {
            inset(v);
            const float tsec = (float)(nowMs() % 100000) * 0.001f;
            beginPath();
            for (int i = 0; i <= 200; ++i) {
                const float x = (float)i / 200.f;
                const float y = std::sin(2.f * kPi * (x * 3.f + tsec * values[pTpWowRate])) * values[pTpWow] * 0.6f + std::sin(2.f * kPi * (x * 40.f + tsec * values[pTpFlutRate])) * values[pTpFlutter] * 0.15f;
                const float xx = v.x + 6 + (v.w - 12) * x, yy = v.cy() - y * v.h * 0.4f;
                if (i == 0) moveTo(xx, yy); else lineTo(xx, yy);
            }
            strokeColor(cWarm); strokeWidth(1.4f); stroke();
            label(v.x + 8, v.y + 10, "pitch wander", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            break;
        }
        case fxCrush: {
            inset(v);
            const float bits = values[pCrBits], q = std::pow(2.f, bits - 1.f), down = values[pCrDown];
            beginPath();
            float hold = 0.f, cnt = 999.f;
            for (int i = 0; i <= 300; ++i) {
                const float x = (float)i / 300.f;
                cnt += 1.f;
                if (cnt >= down * 0.25f) { cnt = 0.f; hold = std::floor(std::sin(2.f * kPi * x * 2.f) * q + 0.5f) / q; }
                const float xx = v.x + 6 + (v.w - 12) * x, yy = v.cy() - hold * v.h * 0.4f;
                if (i == 0) moveTo(xx, yy); else lineTo(xx, yy);
            }
            strokeColor(cRose); strokeWidth(1.2f); stroke();
            break;
        }
        case fxWidth: {
            inset(v);
            const float w = values[pWdWidth];
            const float cx = v.cx(), cy = v.cy();
            beginPath(); ellipse(cx, cy, 30 + 90 * w, 30); fillColor(withA(cAccent, 0.12f)); fill(); strokeColor(cAccent); stroke();
            label(cx, cy, w < 0.05f ? "mono" : (w < 0.95f ? "narrower" : (w < 1.05f ? "unchanged" : "wider")), 9, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            break;
        }
        case fxDelay:
            label(c.x + 14, c.y + 230, "Off in every factory preset.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            break;
        case fxChorus: {
            static const char* const d[4] = { "stereo chorus, two quadrature taps", "three-tap ensemble with fast shimmer", "Solina string ensemble: slow + fast vibrato, three lines", "flanger with feedback" };
            label(c.x + 14, c.y + 230, d[std::max(0, std::min(3, ival(pChMode)))], 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            break;
        }
        case fxReso: {
            const Rect r = { c.x + 14, c.y + 212, c.w - 28, 78 };
            inset(r);
            static const char* const nm[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
            int notes[8]; const int n = resoNotes(notes);
            if (n == 0) { label(r.cx(), r.cy(), "Held Notes: play to tune the strings", 8.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE); break; }
            const float cw = (r.w - 8) / 8.f;
            for (int k = 0; k < 8; ++k) {
                const int nt = notes[k % n] + (n < 8 && ival(pRsMode) == 5 ? 12 * (k / n) : 0);
                const float hz = 440.f * std::pow(2.f, (nt - 69) / 12.f);
                const float h = (r.h - 26) * std::min(1.f, std::log2(hz / 30.f) / 7.f);
                const float x = r.x + 4 + k * cw;
                beginPath(); roundedRect(x + 4, r.y + r.h - 16 - h, cw - 8, h, 2); fillColor(withA(cWarm, 0.55f)); fill();
                char b[8]; std::snprintf(b, sizeof b, "%s%d", nm[((nt % 12) + 12) % 12], nt / 12 - 1);
                label(x + cw * 0.5f, r.y + r.h - 8, b, 7.5f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            }
            label(r.x + 6, r.y + 9, "strings follow the TUNE page", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            break;
        }
        case fxSpectral: {
            const Rect e = { c.x + 250 * c.w / 520.f, c.y + 200, c.w - 250 * c.w / 520.f - 14, 90 };
            inset(e);
            const bool fz = values[pSxFreeze] >= 0.5f;
            label(e.cx(), e.y + 22, fz ? "FROZEN" : (values[pSxBlur] > 0.01f ? "smearing" : "live"), 12, fz ? cBlue : cAccent, ALIGN_CENTER | ALIGN_MIDDLE);
            label(e.cx(), e.y + 46, "freeze · smear · shift the whole sound", 7.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            label(e.cx(), e.y + 62, "about 85 ms behind (one analysis frame)", 7.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            break;
        }
        case fxSpace: {
            const Rect e = { c.x + 14, c.y + 122, c.w - 28, 168 };
            inset(e);
            const float t60 = values[pSpDecay], pre = values[pSpPre], total = std::max(2.f, std::min(20.f, t60 * 1.2f + pre));
            beginPath();
            for (int i = 0; i <= 120; ++i) {
                const float t = total * i / 120.f;
                float db = t < pre ? -60.f : -60.f * (t - pre) / std::max(0.05f, t60);
                db = std::max(-60.f, db);
                const float x = e.x + 6 + (e.w - 12) * i / 120.f, y = e.y + 14 + (e.h - 28) * (-db / 60.f);
                if (i == 0) moveTo(x, y); else lineTo(x, y);
            }
            strokeColor(cBlue); strokeWidth(1.5f); stroke();
            char b[48]; std::snprintf(b, sizeof b, "-60 dB after %.1f s", t60 + pre);
            label(e.x + e.w - 8, e.y + 12, b, 8, cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
            label(e.x + 8, e.y + e.h - 10, "optional reverb · off in every factory preset", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            break;
        }
        default: break;
        }
    }

    // ------------------------------------------------------------------ SEQ page
    void buildSeq() {
        arpCard = { 10, 62, 520, 440 };
        panel(arpCard, "ARPEGGIATOR", pgSeq);
        tog(pArpOn, { 10 + 400, 62 + 8, 50, 20 }, "ON", pgSeq);
        tog(pArpHold, { 10 + 456, 62 + 8, 56, 20 }, "HOLD", pgSeq);
        choice(pArpMode, { 22, 62 + 40, 496, 24 }, 7, pgSeq);
        knob(pArpOct, 80, 62 + 130, 20, "OCTAVES", pgSeq);
        knob(pArpRate, 190, 62 + 130, 20, "RATE", pgSeq);
        knob(pArpGate, 300, 62 + 130, 20, "GATE", pgSeq);
        knob(pArpSwing, 410, 62 + 130, 20, "SWING", pgSeq);
        arpDisp = { 22, 62 + 190, 496, 236 };
        seqCard = { 536, 62, 1054, 440 };
        panel(seqCard, "STEP SEQUENCER", pgSeq);
        choice(pSeqMode, { 536 + 14, 62 + 36, 240, 24 }, 3, pgSeq);
        const uint32_t k[5] = { pSeqRate, pSeqSteps, pSeqGate, pSeqSwing, pSeqGlide };
        static const char* const l[5] = { "RATE", "STEPS", "GATE", "SWING", "CV GLIDE" };
        for (int i = 0; i < 5; ++i) knob(k[i], 536 + 330 + i * 96.f, 62 + 52, 17, l[i], pgSeq);
        seqSteps = { 536 + 16, 62 + 108, 1022, 250 };
        seqGates = { 536 + 16, 62 + 366, 1022, 24 };
        keysRect = { 22, 538, 1556, 112 };
        panel({ 10, 508, 1580, 484 }, "PLAYING NOTES", pgSeq);
        seqDisp = { 22, 660, 1556, 322 };
    }

    void drawSeq() {
        // arp display: active voices as a mini piano roll timeline
        const Rect& a = arpDisp;
        inset(a);
        label(a.x + 8, a.y + 10, "notes now playing (newest highlighted)", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        if (shared) {
            int lo = 127, hi = 0;
            forVoices([&](const drift::TelVoice& v, bool) { lo = std::min(lo, v.note); hi = std::max(hi, v.note); });
            if (hi >= lo) {
                lo = std::max(0, lo - 3); hi = std::min(127, hi + 3);
                const float rowH = (a.h - 30) / (float)(hi - lo + 1);
                forVoices([&](const drift::TelVoice& v, bool newest) {
                    const float y = a.y + a.h - 8 - (v.note - lo + 1) * rowH;
                    const float w = (a.w - 20) * std::min(1.f, v.amp);
                    beginPath(); roundedRect(a.x + 10, y, std::max(4.f, w), std::max(3.f, rowH - 2), 2);
                    fillColor(newest ? cAccent : withA(cViolet, 0.6f)); fill();
                });
            }
            char b[64]; std::snprintf(b, sizeof(b), "step %d   note %d", tel.arpStep, tel.arpNote);
            if (values[pArpOn] >= 0.5f) label(a.x + a.w - 8, a.y + 10, b, 8, cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        if (values[pArpOn] < 0.5f) label(a.cx(), a.cy(), "Arpeggiator off", 11, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        if (ival(pSeqMode) == 2 && values[pArpOn] >= 0.5f) label(a.cx(), a.cy() + 18, "(Sequencer in Notes mode takes over the keys)", 9, cWarn, ALIGN_CENTER | ALIGN_MIDDLE);

        // sequencer steps
        const Rect& r = seqSteps;
        inset(r, hovered(r));
        const int steps = ival(pSeqSteps);
        const int mode = ival(pSeqMode);
        const float cw = r.w / kSeqSteps;
        beginPath(); moveTo(r.x, r.cy()); lineTo(r.x + r.w, r.cy()); strokeColor(cGrid); strokeWidth(1); stroke();
        for (int s = 0; s < kSeqSteps; ++s) {
            const float x = r.x + s * cw;
            const bool used = s < steps;
            const bool cur = shared && tel.seqStep == s;
            const float v = values[pSeqV1 + s];
            if (cur) { beginPath(); rect(x + 1, r.y + 1, cw - 2, r.h - 2); fillColor(withA(cAccent, 0.08f)); fill(); }
            const float h = v * r.h * 0.46f;
            beginPath(); roundedRect(x + 6, std::min(r.cy(), r.cy() - h), cw - 12, std::max(2.f, std::fabs(h)), 3);
            fillColor(!used ? cFaint : (cur ? cAccent : withA(cAccent, 0.55f))); fill();
            char b[16];
            if (mode == 2) std::snprintf(b, sizeof(b), "%+d", (int)std::floor(v * 24.f + 0.5f));
            else std::snprintf(b, sizeof(b), "%+.0f", v * 100.f);
            label(x + cw * 0.5f, r.y + r.h - 10, b, 8.5f, used ? cDim : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            label(x + cw * 0.5f, r.y + 10, std::to_string(s + 1).c_str(), 8, cur ? cAccent : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            const Rect g = { x + 6, seqGates.y, cw - 12, seqGates.h };
            const bool on = values[pSeqG1 + s] >= 0.5f;
            beginPath(); roundedRect(g.x, g.y, g.w, g.h, 4); fillColor(on ? (cur ? cAccent : tint(cAccent, 0.23f)) : cInset); fill(); strokeColor(used ? cPanelEdge : cGrid); stroke();
            if (!used) dimRect({ x + 2, r.y + 2, cw - 4, r.h - 4 });
        }
        label(seqGates.x, seqGates.y + seqGates.h + 14, mode == 2 ? "Notes: hold a key, steps transpose it (±24 st), gates trigger · drag bars, click gates"
                                                               : (mode == 1 ? "CV: the step value is the 'Sequencer' source in the matrix · drag bars to set values"
                                                                            : "Off · choose CV (modulation source) or Notes (Berlin-school sequencing)"), 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);

        // keyboard with live notes
        const Rect& k = keysRect;
        const int lowNote = 24, octs = 8, nWhite = octs * 7;
        const float ww = k.w / nWhite;
        static const int whiteIdx[7] = { 0, 2, 4, 5, 7, 9, 11 };
        auto noteActive = [&](int n, float& amp, bool& newest) {
            amp = 0.f; newest = false; bool any = false;
            forVoices([&](const drift::TelVoice& v, bool nw) { if (v.note == n) { amp = std::max(amp, v.amp); newest = newest || nw; any = true; } });
            return any;
        };
        for (int w = 0; w < nWhite; ++w) {
            const int n = lowNote + (w / 7) * 12 + whiteIdx[w % 7];
            float amp; bool nw;
            const bool on = noteActive(n, amp, nw);
            beginPath(); rect(k.x + w * ww + 0.5f, k.y, ww - 1, k.h); fillColor(on ? mixc(cKeyWhite, cAccent, 0.45f + 0.55f * amp) : cKeyWhite); fill();
            if (w % 7 == 0) { char b[8]; std::snprintf(b, sizeof(b), "C%d", n / 12 - 1); labelOn(k.x + w * ww + ww * 0.5f, k.y + k.h - 8, b, 7.5f, cKeyLabel, ALIGN_CENTER | ALIGN_MIDDLE, cKeyWhite); }
        }
        for (int w = 0; w < nWhite; ++w) {
            const int wi = w % 7;
            if (wi == 2 || wi == 6) continue;
            const int n = lowNote + (w / 7) * 12 + whiteIdx[wi] + 1;
            float amp; bool nw;
            const bool on = noteActive(n, amp, nw);
            beginPath(); rect(k.x + (w + 1) * ww - ww * 0.32f, k.y, ww * 0.64f, k.h * 0.6f); fillColor(on ? cAccent : cKeyBlack); fill();
        }
        // per-voice lanes
        const Rect& d = seqDisp;
        inset(d);
        label(d.x + 8, d.y + 10, "VOICE LANES · amp · pitch · filter 1 cutoff · LFO 1", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        const float lh = (d.h - 24) / drift::kTelVoices;
        for (int i = 0; i < drift::kTelVoices; ++i) {
            const drift::TelVoice& v = tel.v[i];
            const float y = d.y + 20 + i * lh;
            if (!shared || !v.active) continue;
            const bool nw = i == tel.newest;
            char b[24]; std::snprintf(b, sizeof(b), "%2d  %3d", i + 1, v.note);
            label(d.x + 10, y + lh * 0.5f, b, 8, nw ? cAccent : cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            beginPath(); rect(d.x + 70, y + 2, 300 * std::min(1.f, v.amp), lh - 4); fillColor(v.gate ? withA(cAccent, 0.6f) : withA(cViolet, 0.5f)); fill();
            std::snprintf(b, sizeof(b), "%+.2f st", v.pitch - v.note);
            label(d.x + 390, y + lh * 0.5f, b, 8, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            const float cn = std::log2(std::max(20.f, v.cut1) / 20.f) / std::log2(1000.f);
            beginPath(); rect(d.x + 470, y + 2, 400 * std::min(1.f, cn), lh - 4); fillColor(withA(cBlue, 0.55f)); fill();
            beginPath(); rect(d.x + 1050 + std::min(0.f, v.lfo[0] * 150.f), y + 2, std::fabs(v.lfo[0] * 150.f), lh - 4); fillColor(withA(cAccent, 0.55f)); fill();
        }
    }
