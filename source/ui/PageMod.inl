// ===================================================================================== MOD page
// (included inside class DriftfieldUI)

    void buildMod() {
        for (int i = 0; i < kLfos; ++i) {
            const int col = i % 3, row = i / 3;
            const float x = 10.f + col * 398.f, y = 62.f + row * 218.f;
            lfoCard[i] = { x, y, 392, 212 };
            panel(lfoCard[i], "", pgMod);
            lfoHandle[i] = { x + 368, y + 6, 18, 18 };
            lfoEdit[i] = { x + 12, y + 30, 236, 112 };
            selector(lfoP(i, lfWave), { x + 258, y + 32, 122, 20 }, true, pgMod);
            selector(lfoP(i, lfMode), { x + 258, y + 56, 122, 20 }, true, pgMod);
            tog(lfoP(i, lfSync), { x + 258, y + 80, 58, 20 }, "SYNC", pgMod);
            { Ctl& c = selector(lfoP(i, lfDiv), { x + 320, y + 80, 60, 20 }, false, pgMod); c.visParam = (int)lfoP(i, lfSync); c.visVal = 1; }
            { Ctl& c = knob(lfoP(i, lfRate), x + 290, y + 134, 17, "RATE", pgMod); c.visParam = (int)lfoP(i, lfSync); c.visVal = 0; }
            knob(lfoP(i, lfFade), x + 350, y + 134, 13, "FADE", pgMod);
            knob(lfoP(i, lfPhase), x + 36, y + 176, 12, "PHASE", pgMod);
            knob(lfoP(i, lfSmooth), x + 92, y + 176, 12, "SMOOTH", pgMod);
        }
        // random / drift generators
        rndCard = { 1204, 62, 386, 212 };
        panel(rndCard, "DRIFT GENERATORS", pgMod);
        for (int g = 0; g < 2; ++g) {
            const float y = 62.f + 34.f + g * 86.f;
            rndHandle[g] = { 1204 + 356, y - 2, 18, 18 };
            rndDisp[g] = { 1216, y + 18, 270, 56 };
            knob(g == 0 ? pRndGRate : pRndVRate, 1204 + 340, y + 44, 15, "RATE", pgMod);
        }
        panel({ 1204, 280, 386, 212 }, "MACROS", pgMod);
        for (int m = 0; m < kMacros; ++m) {
            const float x = 1204 + 48.f + (m % 4) * 92.f, y = 280.f + 70.f + (m / 4) * 92.f;
            knob(pMacro1 + m, x, y, 20, nullptr, pgMod);
            modMacroHandle[m] = { x + 18, y - 30, 14, 14 };
        }
        // MSEGs: two cards, each shows one of two MSEGs (card 1: MSEG 1 / 3, card 2: MSEG 2 / 4)
        for (int m = 0; m < kMsegs; ++m) {
            const float x = 10, y = 498.f + (m & 1) * 248.f;
            msegCard[m] = { x, y, 700, 244 };
            if (m < 2) panel(msegCard[m], "", pgMod);
            msegTab[m] = { x + 12.f + (m >> 1) * 74.f, y + 7, 70, 20 };
            msegHandle[m] = { x + 672, y + 6, 18, 18 };
            msegEdit[m] = { x + 12, y + 30, 566, 176 };
            auto g = [&](Ctl& c) { c.uiKey = ukMsegA + (m & 1); c.uiVal = m >> 1; };
            g(selector(msegP(m, 0), { x + 588, y + 34, 102, 20 }, true, pgMod));
            g(knob(msegP(m, 1), x + 616, y + 104, 18, "TIME ×", pgMod));
            g(tog(msegP(m, 2), { x + 588, y + 146, 102, 20 }, "SYNC (beats)", pgMod));
        }
        // matrix: 32 slots, 16 per view
        matrixRect = { 716, 498, 874, 494 };
        panel(matrixRect, "MOD MATRIX", pgMod);
        for (int h = 0; h < 2; ++h) matTab[h] = { 716 + 130.f + h * 66.f, 498 + 7, 62, 20 };
        for (int s = 0; s < kModSlots; ++s) {
            const float y = 498.f + 44.f + (s % 16) * kMatRow;
            auto g = [&](Ctl& c) { c.uiKey = ukMatrix; c.uiVal = s / 16; };
            g(selector(modSrcParam(s), { 716 + 30, y, 140, 19 }, false, pgMod));
            g(selector(modViaParam(s), { 716 + 176, y, 116, 19 }, false, pgMod));
            g(selector(modCurveParam(s), { 716 + 298, y, 78, 19 }, false, pgMod));
            g(selector(modDstParam(s), { 716 + 382, y, 190, 19 }, false, pgMod));
            g(slider(modAmtParam(s), { 716 + 578, y + 2, 200, 15 }, nullptr, pgMod));
        }
    }
    static constexpr float kMatRow = 27.5f;
    Rect msegTab[kMsegs], matTab[2];
    bool slotActive(int s) const { return ival(modSrcParam(s)) > 0 && ival(modDstParam(s)) > 0 && values[modAmtParam(s)] != 0.f; }
    void drawTabBtn(const Rect& r, const char* t, bool on, bool dot, const Color& col) {
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4);
        fillColor(on ? tint(col, 0.22f) : (hovered(r) ? shade(22) : cInset)); fill();
        strokeColor(on ? col : cPanelEdge); strokeWidth(1); stroke();
        letterSp(1.2f);
        label(r.cx(), r.cy(), t, 9, on ? col : cDim, ALIGN_CENTER | ALIGN_MIDDLE);
        letterSp(0.f);
        if (dot) { beginPath(); circle(r.x + r.w - 6, r.y + 5, 2.2f); fillColor(col); fill(); }
    }
    // a route uses MSEG m (any slot)
    bool msegUsed(int m) const { const int src = msegSource(m); for (int s = 0; s < kModSlots; ++s) if ((ival(modSrcParam(s)) == src || ival(modViaParam(s)) == src) && slotActive(s)) return true; return false; }

    void drawMod() {
        for (int i = 0; i < kLfos; ++i) drawLfoCard(i);
        drawRandom();
        drawMacros(pgMod);
        for (int m = 0; m < kMsegs; ++m) {
            if (m < 2) for (int k = m; k < kMsegs; k += 2) { char t[16]; std::snprintf(t, sizeof t, "MSEG %d", k + 1); drawTabBtn(msegTab[k], t, msegShown(k), !msegShown(k) && msegUsed(k), cBlue); }
            if (msegShown(m)) drawMsegCard(m);
        }
        drawMatrix();
    }

    void drawLfoCard(int i) {
        const Rect& c = lfoCard[i];
        char t[16]; std::snprintf(t, sizeof(t), "LFO %d", i + 1);
        letterSp(1.5f);
        label(c.x + 14, c.y + 18, t, 10.5f, cLfoCol[i], ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
        drawHandle(lfoHandle[i], cLfoCol[i], msLfo1 + i);
        const Rect& r = lfoEdit[i];
        const bool custom = ival(lfoP(i, lfWave)) == 7;
        inset(r, hovered(r) || (sd == sdLfoDraw && sdIndex == i));
        save(); scissor(r.x, r.y, r.w, r.h);
        beginPath(); moveTo(r.x, r.cy()); lineTo(r.x + r.w, r.cy()); strokeColor(cGrid); strokeWidth(1); stroke();
        if (custom) for (int k = 0; k < drift::kLfoShapePoints; ++k) {
            const float x = r.x + 4 + (r.w - 8) * (float)k / drift::kLfoShapePoints;
            const float y = r.cy() - lfoPts[i][k] * (r.h * 0.4f);
            beginPath(); circle(x, y, 1.3f); fillColor(Color(cLfoCol[i].red, cLfoCol[i].green, cLfoCol[i].blue, 0.4f)); fill();
        }
        drawLfoWave(i, r, 1.6f, true);
        restore();
        label(r.x + 6, r.y + 9, custom ? "draw to edit" : "drag in here to draw a custom shape", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        // live value bar
        float live = 0.f;
        if (const drift::TelVoice* v = newestVoice()) live = v->lfo[i]; else live = tel.lfoG[i];
        const Rect b = { r.x + r.w + 6, r.y, 4, r.h };
        beginPath(); rect(b.x, b.y, b.w, b.h); fillColor(cInset); fill();
        beginPath(); rect(b.x, std::min(b.cy(), b.cy() - live * b.h * 0.5f), b.w, std::fabs(live) * b.h * 0.5f); fillColor(cLfoCol[i]); fill();
        const int u = lfoRouteCount(i);
        char rt[48]; std::snprintf(rt, sizeof(rt), u ? "→ %d route%s" : "unassigned — drag ◎", u, u == 1 ? "" : "s");
        label(c.x + 130, c.y + 176, rt, 8.5f, u ? cDim : cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }

    void drawRandom() {
        static const char* const nm[2] = { "GLOBAL (all voices)", "PER VOICE" };
        for (int g = 0; g < 2; ++g) {
            const Rect& r = rndDisp[g];
            label(r.x, r.y - 10, nm[g], 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            drawHandle(rndHandle[g], cRose, g == 0 ? msRndGlobal : msRndVoice);
            inset(r);
            // history trace
            float& head = rndHist[g][rndHead[g]];
            (void)head;
            beginPath();
            for (int k = 0; k < kRndHist; ++k) {
                const float v = rndHist[g][(rndHead[g] + k + 1) % kRndHist];
                const float x = r.x + 4 + (r.w - 8) * k / (kRndHist - 1), y = r.cy() - v * r.h * 0.42f;
                if (k == 0) moveTo(x, y); else lineTo(x, y);
            }
            strokeColor(cRose); strokeWidth(1.4f); stroke();
        }
    }
    static constexpr int kRndHist = 160;
    float rndHist[2][kRndHist] = {};
    int rndHead[2] = { 0, 0 };
    uint32_t rndLast = 0;
    void tickRandomHistory() {
        const uint32_t t = nowMs();
        if (t - rndLast < 40) return;
        rndLast = t;
        const drift::TelVoice* v = newestVoice();
        for (int g = 0; g < 2; ++g) {
            rndHead[g] = (rndHead[g] + 1) % kRndHist;
            float val = 0.f;
            if (g == 0) val = tel.rndG;
            else if (v) val = v->rndV;
            rndHist[g][rndHead[g]] = val;
        }
    }

    // ------------------------------------------------------------------ MSEG editor
    float msegTotal(int m) const { float t = 0.f; for (int i = 1; i < msegD[m].count; ++i) t += msegD[m].t[i]; return std::max(0.05f, t); }
    // 5.1.6: the MAIN page's modulator panel edits the selected MSEG in its own, smaller box
    Rect msegR(int m) const { return page == pgMain ? mainMsegEdit : msegEdit[m]; }
    Rect lfoR(int i) const { return page == pgMain ? mainLfoEdit : lfoEdit[i]; }
    float msegPointX(int m, int i, float total) const {
        const Rect r = msegR(m);
        float t = 0.f; for (int k = 1; k <= i; ++k) t += msegD[m].t[k];
        return r.x + 8 + (r.w - 16) * t / total;
    }
    float msegPointY(int m, float l) const { const Rect r = msegR(m); const float pb = page == pgMain ? 10.f : 14.f, pt = page == pgMain ? 18.f : 26.f; return r.y + r.h - pb - (r.h - pt) * l; }
    void drawMsegCard(int m) {
        const bool compact = page == pgMain;
        const Rect r = msegR(m);
        const drift::MsegData& d = msegD[m];
        if (!compact) drawHandle(msegHandle[m], cBlue, msegSource(m));
        inset(r, hovered(r));
        const float total = sd == sdMsegPoint && sdIndex == m ? msegDragTotal : msegTotal(m);
        // grid: seconds (or beats)
        const bool sync = values[msegP(m, 2)] >= 0.5f;
        const float scale = values[msegP(m, 1)];
        const float step = total > 20.f ? 5.f : (total > 8.f ? 2.f : (total > 3.f ? 1.f : 0.25f));
        for (float t = step; t < total; t += step) {
            const float x = r.x + 8 + (r.w - 16) * t / total;
            beginPath(); moveTo(x, r.y + 4); lineTo(x, r.y + r.h - 4); strokeColor(cGrid); strokeWidth(1); stroke();
        }
        // loop region
        const float lx0 = msegPointX(m, d.loopStart, total), lx1 = msegPointX(m, d.loopEnd, total);
        const int mode = ival(msegP(m, 0));
        if (d.loopEnd > d.loopStart || mode == 0) {
            beginPath(); rect(lx0, r.y + 2, std::max(2.f, lx1 - lx0), r.h - 4); fillColor(withA(cBlue, mode == 0 ? 0.0f : 0.08f)); fill();
        }
        beginPath(); moveTo(lx1, r.y + 2); lineTo(lx1, r.y + r.h - 2); strokeColor(withA(cBlue, 0.6f)); strokeWidth(1); stroke();
        if (mode != 0) { beginPath(); moveTo(lx0, r.y + 2); lineTo(lx0, r.y + r.h - 2); strokeColor(withA(cBlue, 0.6f)); stroke(); }
        // loop handles at the bottom
        const float hy = r.y + r.h - 6;
        if (mode != 0) { beginPath(); moveTo(lx0, hy - 6); lineTo(lx0 + 6, hy); lineTo(lx0, hy); closePath(); fillColor(cBlue); fill(); }
        beginPath(); moveTo(lx1, hy - 6); lineTo(lx1 - 6, hy); lineTo(lx1, hy); closePath(); fillColor(cBlue); fill();
        if (!compact) label(lx1 + 3, r.y + 10, mode == 0 ? "sustain" : "loop", 7.5f, cBlue, ALIGN_LEFT | ALIGN_MIDDLE);
        // curve
        beginPath();
        for (int i = 0; i < d.count; ++i) {
            const float x0 = msegPointX(m, i, total), y0 = msegPointY(m, d.l[i]);
            if (i == 0) { moveTo(x0, y0); continue; }
            const float xp = msegPointX(m, i - 1, total), lp = d.l[i - 1];
            for (int k = 1; k <= 24; ++k) {
                const float u = (float)k / 24.f;
                lineTo(xp + (x0 - xp) * u, msegPointY(m, lp + (d.l[i] - lp) * drift::msegCurve(u, d.c[i])));
            }
        }
        strokeColor(cBlue); strokeWidth(1.7f); stroke();
        // points
        for (int i = 0; i < d.count; ++i) {
            const float x = msegPointX(m, i, total), y = msegPointY(m, d.l[i]);
            const bool hot = std::fabs(hoverX - x) < 7 && std::fabs(hoverY - y) < 7;
            beginPath(); circle(x, y, hot ? 5.5f : 4.f); fillColor(hot ? cText : cBlue); fill();
            beginPath(); circle(x, y, hot ? 5.5f : 4.f); strokeColor(cInset); strokeWidth(1.5f); stroke();
        }
        // curve handles (segment midpoints)
        for (int i = 1; i < d.count; ++i) {
            const float xa = msegPointX(m, i - 1, total), xb = msegPointX(m, i, total);
            const float lm = d.l[i - 1] + (d.l[i] - d.l[i - 1]) * drift::msegCurve(0.5f, d.c[i]);
            beginPath(); circle((xa + xb) * 0.5f, msegPointY(m, lm), 2.5f); strokeColor(withA(cBlue, 0.7f)); strokeWidth(1); stroke();
        }
        // playheads. 5.1.6: x comes from the engine's position inside the segment (it used to be solved from
        // the value as if the segment were a straight line, so the dot left the curve on bent segments)
        forVoices([&](const drift::TelVoice& v, bool newest) {
            const int seg = v.msegSeg[m];
            if (seg < 1 || seg >= d.count) return;
            const float u = std::max(0.f, std::min(1.f, v.msegU[m]));
            const float xa = msegPointX(m, seg - 1, total), xb = msegPointX(m, seg, total);
            const float x = xa + (xb - xa) * u;
            const float y = msegPointY(m, v.mseg[m]);
            beginPath(); circle(x, y, newest ? (compact ? 4.f : 5.f) : 3.f); fillColor(newest ? withA(cText, 0.9f) : withA(cViolet, 0.7f)); fill();
            if (newest) { beginPath(); moveTo(x, r.y + 3); lineTo(x, r.y + r.h - 3); strokeColor(withA(cText, 0.18f)); strokeWidth(1); stroke(); }
        });
        if (compact) return;
        char b[96];
        std::snprintf(b, sizeof(b), "%.2f %s total · click: add · drag: move · double-click: delete · shift-drag segment: curve", total * scale, sync ? "beats" : "s");
        label(r.x, r.y + r.h + 14, b, 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }

    // ------------------------------------------------------------------ matrix
    void drawMatrix() {
        const float x = matrixRect.x, y = matrixRect.y;
        label(x + 30, y + 34, "SOURCE", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(x + 176, y + 34, "× VIA", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(x + 298, y + 34, "CURVE", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(x + 382, y + 34, "DESTINATION", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(x + 578, y + 34, "AMOUNT", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(x + 790, y + 34, "LIVE", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        for (int h = 0; h < 2; ++h) {
            bool other = false; for (int s = h * 16; s < h * 16 + 16; ++s) other = other || slotActive(s);
            drawTabBtn(matTab[h], h ? "17–32" : "1–16", uiSel[ukMatrix] == h, uiSel[ukMatrix] != h && other, cAccent);
        }
        for (int s = uiSel[ukMatrix] * 16; s < uiSel[ukMatrix] * 16 + 16; ++s) {
            const float ry = y + 44.f + (s % 16) * kMatRow + 1.f;
            const bool act = slotActive(s);
            char b[8]; std::snprintf(b, sizeof(b), "%d", s + 1);
            label(x + 16, ry + 8.5f, b, 8.5f, act ? cAccent : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            const Rect m = { x + 790, ry + 3, 72, 11 };
            beginPath(); roundedRect(m.x, m.y, m.w, m.h, 2); fillColor(cInset); fill();
            if (act && shared) {
                const float lv = std::max(-1.f, std::min(1.f, tel.slotOut[s]));
                beginPath(); rect(std::min(m.cx(), m.cx() + lv * m.w * 0.5f), m.y + 1, std::fabs(lv) * m.w * 0.5f, m.h - 2); fillColor(cViolet); fill();
            }
            beginPath(); moveTo(m.cx(), m.y); lineTo(m.cx(), m.y + m.h); strokeColor(cFaint); strokeWidth(1); stroke();
        }
    }
