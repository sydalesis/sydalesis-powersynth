// ===================================================================================== Widgets
// (included inside class DriftfieldUI)

    // 5.1.5 text visibility. Every string goes through label():
    //  * size: multiplied by the SETUP "Text size" choice (Normal / Large / Larger);
    //  * sharp: drawn with the transform reset, at a whole-pixel font size and a whole-pixel origin in
    //    window pixels, so glyphs come out of the font atlas 1:1 instead of being resampled by the
    //    UI scale (that resampling was the blur);
    //  * contrast: colours that sit too close to the panel colour are lifted toward white (dark skins)
    //    or black (light skins) until they reach a minimum contrast, keeping the text / dim / faint order.
    static float textMulFor(int i) { static const float m[3] = { 1.f, 1.2f, 1.36f }; return m[i < 0 ? 0 : (i > 2 ? 2 : i)]; }
    // small text gets the full boost, headline sizes (14 and up) stay as designed so the header fits
    float textPx(float size) const {
        const float m = textMulFor(optTextSize);
        if (optTextSize > 0 && size < 7.8f) size = 7.8f;   // the tiniest captions get a floor
        const float k = size <= 11.f ? 1.f : (size >= 14.f ? 0.f : (14.f - size) / 3.f);
        return size * (1.f + (m - 1.f) * k);
    }
    void uiFontSize(float size) { fontSize(textPx(size)); }   // for textBounds() measurements
    float curLetterSp = 0.f;
    void letterSp(float v) { curLetterSp = v; textLetterSpacing(v); }
    static float linC(float c) { return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f); }
    static float lumOf(const Color& c) { return 0.2126f * linC(c.red) + 0.7152f * linC(c.green) + 0.0722f * linC(c.blue); }
    struct InkCache { float r = -1, g, b, a; Color out; };
    InkCache inkCache[64];
    int inkSkin = -2;
    // contrast lift of a text colour against a surface luminance ls (dark = the surface is dark)
    static Color inkVs(const Color& c, float ls, bool dark) {
        const float lc = lumOf(c);
        // text darker than a dark surface (or lighter than a light one) sits on a fill of its own
        // (lit buttons, clip lamp): leave it alone
        if (dark ? lc <= ls : lc >= ls) return c;
        auto ratio = [&](float l) { return dark ? (l + 0.05f) / (ls + 0.05f) : (ls + 0.05f) / (l + 0.05f); };
        const float r0 = ratio(lc);
        // weak colours get the most help, strong ones keep their colour; order text > dim > faint is kept
        const float target = std::max(6.5f, std::min(r0 * 1.3f, dark ? 13.f : 11.f));
        Color out = c;
        if (r0 < target) {
            const Color ink = dark ? Color(1.f, 1.f, 1.f, c.alpha) : Color(0.f, 0.f, 0.f, c.alpha);
            float lo = 0.f, hi = 1.f;
            for (int it = 0; it < 10; ++it) { const float m = 0.5f * (lo + hi); if (ratio(lumOf(mixc(c, ink, m))) < target) lo = m; else hi = m; }
            out = mixc(c, ink, hi);
        }
        out.alpha = c.alpha < 1.f ? std::min(1.f, c.alpha * 1.25f + 0.1f) : 1.f;   // faded text: less faded
        return out;
    }
    Color textInk(const Color& c) {
        if (inkSkin != gSkin) { for (InkCache& e : inkCache) e.r = -1; inkSkin = gSkin; }
        const unsigned h = ((unsigned)(c.red * 255.f) * 73u + (unsigned)(c.green * 255.f) * 151u + (unsigned)(c.blue * 255.f) * 31u + (unsigned)(c.alpha * 255.f) * 7u) & 63u;
        InkCache& e = inkCache[h];
        if (e.r == c.red && e.g == c.green && e.b == c.blue && e.a == c.alpha) return e.out;
        // reference surface: background or panel top, whichever is closer to the text (the worse case)
        const float lb = lumOf(cBg), lp = lumOf(cPanelTop);
        const bool dark = lb < 0.25f;
        e.out = inkVs(c, dark ? std::max(lb, lp) : std::min(lb, lp), dark);
        e.r = c.red; e.g = c.green; e.b = c.blue; e.a = c.alpha;
        return e.out;
    }
    // text on a fill of its own (keyboard keys): contrast against that fill
    void labelOn(float x, float y, const char* s, float size, const Color& c, int align, const Color& surface) {
        const float l = lumOf(surface);
        labelInk(x, y, s, size, inkVs(c, l, l < 0.25f), align);
    }
    void label(float x, float y, const char* s, float size, const Color& c, int align) {
        if (s == nullptr || s[0] == '\0') return;
        labelInk(x, y, s, size, textInk(c), align);
    }
    void labelInk(float x, float y, const char* s, float size, const Color& ink, int align) {
        if (s == nullptr || s[0] == '\0') return;
        const float px = textPx(size);
        fillColor(ink); textAlign(align);
        float xf[6]; currentTransform(xf);
        const float sc = xf[3];
        if (sc <= 0.f || std::fabs(xf[1]) > 1e-4f || std::fabs(xf[2]) > 1e-4f || std::fabs(xf[0] - xf[3]) > 1e-4f) {
            fontSize(px); text(x, y, s, nullptr); return;   // rotated / skewed: plain path
        }
        const float dx = xf[0] * x + xf[4], dy = xf[3] * y + xf[5];
        save(); resetTransform();
        fontSize(std::max(7.f, std::round(px * sc)));
        if (curLetterSp != 0.f) textLetterSpacing(curLetterSp * sc);
        text(std::round(dx), std::round(dy), s, nullptr);
        restore();
    }
    // shrinks the size (never below the designed size) until the text fits maxW
    float fitSize(const char* s, float size, float maxW) {
        if (s == nullptr || s[0] == '\0' || maxW <= 0.f) return size;
        uiFontSize(size);
        Rectangle<float> bb; textBounds(0, 0, s, nullptr, bb);
        const float w = bb.getWidth();
        if (w <= maxW) return size;
        float px = textPx(size) * maxW / w;
        const float minPx = std::min(textPx(size), size * 0.92f);
        if (px < minPx) px = minPx;
        return px / (textPx(size) / size);   // back to a designed size that textPx() maps to px
    }
    void labelFit(float x, float y, const char* s, float size, const Color& c, int align, float maxW) {
        label(x, y, s, fitSize(s, size, maxW), c, align);
    }
    void labelS(float x, float y, const std::string& s, float size, const Color& c, int align) { label(x, y, s.c_str(), size, c, align); }
    bool hovered(const Rect& r) const { return !anyOverlay() && sd == sdNone && r.contains(hoverX, hoverY); }
    void inset(const Rect& r, bool hot = false) {
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 5); fillColor(cInset); fill();
        strokeColor(hot ? tint(cAccent, 0.31f) : shade(34.f)); strokeWidth(1); stroke();
    }
    void dimRect(const Rect& r) { beginPath(); roundedRect(r.x, r.y, r.w, r.h, 6); fillColor(withA(cBg, 0.55f)); fill(); }
    void drawPanel(const PanelDef& p) {
        beginPath(); roundedRect(p.r.x, p.r.y, p.r.w, p.r.h, 7);
        fillPaint(linearGradient(0, p.r.y, 0, p.r.y + p.r.h, cPanelTop, cPanelBot));
        fill();
        strokeColor(cPanelEdge); strokeWidth(1); stroke();
        letterSp(1.5f);
        label(p.r.x + 14, p.r.y + 19, p.title, 10.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
    }
    void button(const Rect& r, const char* t, bool sel, bool hot, float fs = 10.f, const Color* selCol = nullptr) {
        const Color sc = selCol ? *selCol : cAccent;
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4);
        fillColor(sel ? mixc(cInset, sc, 0.24f) : (hot ? shade(38.f) : cInset)); fill();
        strokeColor(sel ? mixc(cInset, sc, 0.6f) : cPanelEdge); strokeWidth(1); stroke();
        labelFit(r.x + r.w * 0.5f, r.y + r.h * 0.5f + 0.5f, t, fs, sel ? sc : (hot ? cText : cDim), ALIGN_CENTER | ALIGN_MIDDLE, r.w - 6.f);
    }
    void drawArrow(float cx, float cy, bool left, const Color& c) {
        beginPath();
        if (left) { moveTo(cx + 3, cy - 5); lineTo(cx - 3, cy); lineTo(cx + 3, cy + 5); }
        else      { moveTo(cx - 3, cy - 5); lineTo(cx + 3, cy); lineTo(cx - 3, cy + 5); }
        strokeColor(c); strokeWidth(1.6f); lineCap(ROUND); stroke(); lineCap(BUTT);
    }
    void drawButtonArrow(const Rect& r, bool left, bool hot) {
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4); fillColor(hot ? shade(41.f) : cInset); fill();
        strokeColor(cPanelEdge); strokeWidth(1); stroke();
        drawArrow(r.x + r.w * 0.5f, r.y + r.h * 0.5f, left, hot ? cAccent : cDim);
    }
    // drag handle for a modulation source (drag it onto a knob to create a route)
    void drawHandle(const Rect& r, const Color& c, int src) {
        const bool hot = hovered(r) || (sd == sdAssign && assignSrc == src);
        const float cx = r.cx(), cy = r.cy(), rad = r.h * 0.5f - 1.f;
        beginPath(); circle(cx, cy, rad); fillColor(hot ? Color(c.red, c.green, c.blue, 0.35f) : Color(c.red, c.green, c.blue, 0.12f)); fill();
        strokeColor(Color(c.red, c.green, c.blue, hot ? 1.f : 0.6f)); strokeWidth(1.2f); stroke();
        beginPath(); circle(cx, cy, rad * 0.35f); fillColor(c); fill();
    }

    // ------------------------------------------------------------------ knob with modulation ring
    void drawKnob(const Ctl& c, const Rect& r, bool hot) {
        const float kx = r.x, ky = r.y, kr = r.w;
        const float a0 = 0.75f * kPi, a1 = 2.25f * kPi;
        const float v = values[c.p];
        const float n = std::max(0.f, std::min(1.f, toNorm(c.p, v)));
        const float av = a0 + (a1 - a0) * n;
        const float aStart = isBipolar(c.p) ? a0 + (a1 - a0) * toNorm(c.p, 0.f) : a0;
        const bool assignTarget = sd == sdAssign && destOf(c.p) >= 0;
        beginPath(); circle(kx, ky, kr);
        fillPaint(radialGradient(kx - kr * 0.3f, ky - kr * 0.4f, 0, kr * 1.3f, cKnobHi, cKnobLo));
        fill();
        strokeColor(assignTarget ? (hot ? cViolet : tint(cViolet, 0.6f)) : cKnobEdge); strokeWidth(assignTarget ? 2.f : 1.f); stroke();
        const float tr = kr + 4.f;
        beginPath(); arc(kx, ky, tr, a0, a1, CW); strokeColor(cFaint); strokeWidth(2.5f); lineCap(ROUND); stroke();
        if (std::fabs(av - aStart) > 0.001f) {
            beginPath();
            if (av > aStart) arc(kx, ky, tr, aStart, av, CW); else arc(kx, ky, tr, av, aStart, CW);
            strokeColor(hot ? mixc(cAccent, cText, 0.35f) : cAccent); strokeWidth(2.5f); stroke();
        }
        // modulation: static range (violet arc outside) + live position dot
        const int d = destOf(c.p);
        if (d >= 0) {
            float lo = 0.f, hi = 0.f; bool any = false;
            for (int s = 0; s < kModSlots; ++s) {
                if (ival(modDstParam(s)) != d || ival(modSrcParam(s)) == msNone) continue;
                const float amt = values[modAmtParam(s)];
                if (amt == 0.f) continue;
                any = true;
                const float off = modNormOffset(c.p, d, std::fabs(amt));
                const int src = ival(modSrcParam(s));
                bool uni = src == msFiltEnv || src == msAmpEnv || src == msModEnv || src == msMseg1 || src == msMseg2 || src == msMseg3 || src == msMseg4 || src == msVelocity
                              || src == msPressure || src == msModWheel || (src >= msMacro1 && src <= msMacro8) || src == msExpression || src == msBreath;
                const int crv = ival(modCurveParam(s));
                if (crv == 7) uni = false;          // 5.1.6: Bipolar curve
                if (crv == 4) uni = true;
                if (uni) { if (amt > 0.f) hi += off; else lo += off; }
                else { hi += off; lo += off; }
            }
            if (any) {
                const float nl = std::max(0.f, n - lo), nh = std::min(1.f, n + hi);
                const float rr = kr + 8.f;
                beginPath(); arc(kx, ky, rr, a0 + (a1 - a0) * nl, a0 + (a1 - a0) * std::max(nl + 0.001f, nh), CW);
                strokeColor(withA(cViolet, 0.55f)); strokeWidth(2.f); lineCap(ROUND); stroke();
                if (newestVoice() || isGlobalDest(d)) {
                    const float live = std::max(0.f, std::min(1.f, n + modNormOffset(c.p, d, tel.md[d])));
                    const float al = a0 + (a1 - a0) * live;
                    beginPath(); circle(kx + std::cos(al) * rr, ky + std::sin(al) * rr, 2.8f); fillColor(cViolet); fill();
                    beginPath(); moveTo(kx + std::cos(al) * kr * 0.25f, ky + std::sin(al) * kr * 0.25f);
                    lineTo(kx + std::cos(al) * kr * 0.75f, ky + std::sin(al) * kr * 0.75f);
                    strokeColor(withA(cViolet, 0.7f)); strokeWidth(1.5f); stroke();
                }
            }
        }
        beginPath();
        moveTo(kx + std::cos(av) * kr * 0.28f, ky + std::sin(av) * kr * 0.28f);
        lineTo(kx + std::cos(av) * kr * 0.82f, ky + std::sin(av) * kr * 0.82f);
        strokeColor(cKnobPtr); strokeWidth(2); lineCap(ROUND); stroke();
        lineCap(BUTT);
        char buf[48];
        const bool enumShown = P(c.p).enumNames != nullptr;
        if (hot || enumShown) formatValue(c.p, v, buf, sizeof(buf));
        const float fs = kr < 13 ? 8.f : (kr < 16 ? 8.8f : 9.8f);
        if (c.label) labelFit(kx, ky + kr + 12, hot || enumShown ? buf : c.label, fs, hot ? cAccent : (enumShown ? cText : cDim), ALIGN_CENTER | ALIGN_MIDDLE, std::max(42.f, kr * 2.f + 16.f));
        if (enumShown && !hot && c.label) label(kx, ky + kr + 21, c.label, 7.2f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
    }

    void drawSelector(const Ctl& c, const Rect& r) {
        const bool hot = hovered(r);
        const float v = values[c.p];
        const bool isNone = isModSrc(c.p) || isModDst(c.p) ? v < 0.5f : false;
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4); fillColor(hot ? shade(34.f) : cInset); fill();
        strokeColor(menuOpen && menuParam == c.p ? cAccent : cPanelEdge); strokeWidth(1); stroke();
        const std::string nm = valueName(c.p, v);
        const float fs = r.h < 18 ? 9.f : (r.h < 22 ? 10.f : 11.f);
        if (c.arrows) {
            drawArrow(r.x + 10, r.y + r.h * 0.5f, true, hot && hoverX < r.x + 20 ? cAccent : cDim);
            drawArrow(r.x + r.w - 10, r.y + r.h * 0.5f, false, hot && hoverX > r.x + r.w - 20 ? cAccent : cDim);
            save(); scissor(r.x + 18, r.y, r.w - 36, r.h);
            label(r.x + r.w * 0.5f, r.y + r.h * 0.5f + 0.5f, nm.c_str(), fs, cText, ALIGN_CENTER | ALIGN_MIDDLE);
            restore();
        } else {
            save(); scissor(r.x + 2, r.y, r.w - 16, r.h);
            label(r.x + 7, r.y + r.h * 0.5f + 0.5f, nm.c_str(), fs, isNone ? cFaint : cText, ALIGN_LEFT | ALIGN_MIDDLE);
            restore();
            beginPath(); moveTo(r.x + r.w - 12, r.y + r.h * 0.5f - 2); lineTo(r.x + r.w - 8.5f, r.y + r.h * 0.5f + 2); lineTo(r.x + r.w - 5, r.y + r.h * 0.5f - 2);
            strokeColor(hot ? cAccent : cFaint); strokeWidth(1.3f); stroke();
        }
    }
    void drawChoice(const Ctl& c, const Rect& r) {
        const int cur = (int)std::round(values[c.p] - P(c.p).min);
        const float w = r.w / c.count;
        for (int i = 0; i < c.count; ++i) {
            const Rect b = { r.x + i * w + 1, r.y, w - 2, r.h };
            button(b, c.names ? c.names[i] : (P(c.p).enumNames ? P(c.p).enumNames[i] : ""), i == cur, hovered(b), r.h < 20 ? 8.5f : 9.2f);
        }
    }
    void drawSlider(const Ctl& c, const Rect& rr) {
        // label | bar | value
        const float lw = c.label ? 46.f : 0.f;
        const Rect r = { rr.x + lw, rr.y, rr.w - lw - 42, rr.h };
        const float v = values[c.p];
        const bool hot = hovered(rr) || dragParam == (int)c.p;
        if (c.label) labelFit(rr.x, rr.y + rr.h * 0.5f + 0.5f, c.label, 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE, 44.f);
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 3); fillColor(cInset); fill(); strokeColor(cPanelEdge); strokeWidth(1); stroke();
        const bool bip = isBipolar(c.p);
        const float x0 = bip ? r.x + r.w * toNorm(c.p, 0.f) : r.x, xv = r.x + r.w * toNorm(c.p, v);
        beginPath(); rect(std::min(x0, xv), r.y + 3, std::fabs(xv - x0), r.h - 6);
        fillColor(v >= 0 || !bip ? withA(cAccent, hot ? 0.8f : 0.55f) : withA(cWarm, hot ? 0.8f : 0.55f)); fill();
        if (bip) { beginPath(); moveTo(x0, r.y + 2); lineTo(x0, r.y + r.h - 2); strokeColor(cFaint); stroke(); }
        char buf[32]; formatValue(c.p, v, buf, sizeof(buf));
        label(rr.x + rr.w, r.y + r.h * 0.5f + 0.5f, buf, 8.8f, hot ? cText : cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
    }
    void drawToggle(const Ctl& c, const Rect& r) {
        const bool on = values[c.p] >= 0.5f, hot = hovered(r);
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, r.h * 0.5f);
        fillColor(on ? tint(cAccent, 0.23f) : (hot ? shade(38.f) : cInset)); fill();
        strokeColor(on ? tint(cAccent, 0.62f) : (hot ? cDim : cPanelEdge)); strokeWidth(1); stroke();
        const float ly = r.y + r.h * 0.5f;
        if (on) { beginPath(); circle(r.x + 10, ly, 6.f); fillPaint(radialGradient(r.x + 10, ly, 1, 7, withA(cAccent, 0.55f), withA(cAccent, 0.f))); fill(); }
        beginPath(); circle(r.x + 10, ly, 3.2f); fillColor(on ? cAccent : cFaint); fill();
        label(r.x + 18 + (r.w - 18) * 0.5f, ly + 0.5f, c.label ? c.label : (on ? "ON" : "OFF"), 8.6f, on ? cAccent : cDim, ALIGN_CENTER | ALIGN_MIDDLE);
    }
    void drawCtl(int i) {
        const Ctl& c = ctls[i];
        const Rect r = ctlRect(c);
        switch (c.kind) {
        case ckKnob: drawKnob(c, r, i == hoverCtl || i == dragCtl); break;
        case ckSelector: drawSelector(c, r); break;
        case ckChoice: drawChoice(c, r); break;
        case ckSlider: drawSlider(c, r); break;
        case ckToggle: drawToggle(c, r); break;
        case ckVFader: drawVFader(c, r); break;
        default: break;
        }
    }
    bool ctlHit(const Ctl& c, float x, float y) const {
        const Rect r = ctlRect(c);
        if (c.kind == ckKnob) { const float dx = x - r.x, dy = y - r.y; return dx * dx + dy * dy <= (r.w + 6) * (r.w + 6); }
        return r.contains(x, y);
    }
    int ctlAt(float x, float y) const {
        for (int i = (int)ctls.size() - 1; i >= 0; --i) if (visible(ctls[i]) && ctlHit(ctls[i], x, y)) return i;
        return -1;
    }

    // ------------------------------------------------------------------ header
    std::string presetLabel() const {
        if (currentPreset >= 0 && currentPreset < (int)kFactoryPresetCount)
            return std::string(kFactoryPresets[currentPreset].category) + "  /  " + kFactoryPresets[currentPreset].name;
        const int u = currentPreset - (int)kFactoryPresetCount;
        if (u >= 0 && u < (int)userPresets.size()) return "User  /  " + userPresets[u].name;
        if (!orphanPresetName.empty()) return orphanPresetName;
        return "—";
    }
    void drawHeader() {
        beginPath(); rect(0, 0, kW, 56); fillPaint(linearGradient(0, 0, 0, 56, cHeadTop, cBg)); fill();
        beginPath(); moveTo(0, 56.5f); lineTo(kW, 56.5f); strokeColor(cPanelEdge); strokeWidth(1); stroke();
        letterSp(5.5f);
        label(20, 25, "SYDALESIS", 20, cText, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
        label(198, 25, "5", 20, cAccent, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(1.6f);
        drawLayerSwitch();
        letterSp(0.f);
        for (int i = 0; i < kPages; ++i) button(tabRect[i], kPageNames[i], page == i, hovered(tabRect[i]), i == pgPerf ? 8.4f : 9.5f);
        drawButtonArrow(presetPrev, true, hovered(presetPrev));
        drawButtonArrow(presetNext, false, hovered(presetNext));
        beginPath(); roundedRect(presetBox.x, presetBox.y, presetBox.w, presetBox.h, 5);
        fillColor(hovered(presetBox) ? shade(31.f) : cInset); fill(); strokeColor(cPanelEdge); stroke();
        save(); scissor(presetBox.x + 4, presetBox.y, presetBox.w - 8, presetBox.h);
        labelS(presetBox.cx(), presetBox.cy() + 1, presetLabel(), 12, cText, ALIGN_CENTER | ALIGN_MIDDLE);
        if (presetEdited) label(presetBox.x + 12, presetBox.cy() + 1, "edited", 8, cWarm, ALIGN_LEFT | ALIGN_MIDDLE);
        restore();
        if (currentPreset >= 0 && isFavorite(presetKey(currentPreset))) label(presetBox.x + presetBox.w - 12, presetBox.cy() + 1, "★", 11, cWarm, ALIGN_CENTER | ALIGN_MIDDLE);
        button(saveBtn, "SAVE", false, hovered(saveBtn), 9.5f);
        button(abBtn[0], "A", abCur == 0, hovered(abBtn[0]), 10.f);
        button(abBtn[1], "B", abCur == 1, hovered(abBtn[1]), 10.f);
        button(undoBtn, "↶", false, hovered(undoBtn) && !undoStack.empty(), 13.f);
        button(redoBtn, "↷", false, hovered(redoBtn) && !redoStack.empty(), 13.f);
        if (undoStack.empty()) dimRect(undoBtn);
        if (redoStack.empty()) dimRect(redoBtn);
        button(randBtn, "RAND", randOpen, hovered(randBtn), 9.5f, &cWarm);
        // meters
        const float mx = 1334;
        const float cpu = shared ? std::min(1.f, tel.cpu) : 0.f;
        label(mx, 22, "CPU", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        beginPath(); roundedRect(mx + 26, 18, 70, 7, 3); fillColor(cInset); fill();
        beginPath(); roundedRect(mx + 26, 18, 70 * cpu, 7, 3); fillColor(cpu > 0.75f ? cWarn : (cpu > 0.45f ? cWarm : cAccent)); fill();
        char buf[64]; std::snprintf(buf, sizeof(buf), "%.0f%%", cpu * 100.f);
        label(mx + 100, 22, buf, 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        const int guard = shared ? shared->guardCap.load() : 0;
        if (guard > 0) std::snprintf(buf, sizeof(buf), "GUARD %d / %d", tel.activeCount, guard);
        else std::snprintf(buf, sizeof(buf), "%d / %d voices", shared ? tel.activeCount : 0, ival(pPolyphony));
        label(mx, 40, buf, 8.5f, guard > 0 ? cWarn : cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        if (shared && tel.bpm > 0.f) { std::snprintf(buf, sizeof(buf), "%.0f bpm%s", tel.bpm, tel.playing ? " ▶" : ""); label(mx + 100, 40, buf, 8.5f, tel.playing ? cAccent : cFaint, ALIGN_LEFT | ALIGN_MIDDLE); }
        label(kW - 14, 21, "MPE · v5.1.6", 8.5f, cAccent, ALIGN_RIGHT | ALIGN_MIDDLE);

        const float cur = currentRelativeScale();
        static const char* const lbl[4] = { "75", "90", "100", "125" };
        for (int i = 0; i < 4; ++i) button(sizeBtn[i], lbl[i], std::fabs(cur - kSizes[i]) < 0.02f, hovered(sizeBtn[i]), 8.f);
    }
    void drawGrip() {
        const bool hot = hoverX >= kW - 22 && hoverY >= kHT - 22;
        beginPath();
        for (int i = 0; i < 3; ++i) { const float d = 5.f + i * 4.5f; moveTo(kW - 4, kHT - 4 - d); lineTo(kW - 4 - d, kHT - 4); }
        strokeColor(hot || sd == sdResize ? cAccent : cDim); strokeWidth(1.4f); lineCap(ROUND); stroke(); lineCap(BUTT);
    }

    // ------------------------------------------------------------------ tooltip
    void drawTooltip() {
        if (!optTips || tipShown < 0 || tipShown >= (int)ctls.size() || tipShown != hoverStable || anyOverlay() || sd != sdNone) return;
        const Ctl& c = ctls[tipShown];
        if (!visible(c)) return;
        char val[64]; formatValue(c.p, values[c.p], val, sizeof(val));
        std::vector<std::string> lines;
        lines.push_back(std::string(P(c.p).name) + ":  " + val);
        const int d = destOf(c.p);
        if (d >= 0)
            for (int s = 0; s < kModSlots; ++s)
                if (ival(modDstParam(s)) == d && ival(modSrcParam(s)) != msNone && values[modAmtParam(s)] != 0.f) {
                    char b[96]; std::snprintf(b, sizeof(b), "mod %d: %s  %+.0f%%", s + 1, kModSrcNames[ival(modSrcParam(s))], values[modAmtParam(s)] * 100.f);
                    lines.push_back(b);
                }
        if (d >= 0 && lines.size() == 1) lines.push_back("drag a source here, or right-drag to set depth");
        if (c.p == pFEnvCurve || c.p == pAEnvCurve || c.p == pMEnvCurve) lines.push_back("left: slow swell · centre: soft S-curve · right: punchy analog rise");
        if (c.p == pFRelCurve || c.p == pARelCurve || c.p == pMRelCurve) lines.push_back("left: hold the level, fall away at the end · centre: natural exponential fade · right: drop fast, long soft tail");
        if (c.kind == ckKnob) lines.push_back("double-click: default · shift: fine");
        uiFontSize(10.5f);
        float w = 0.f;
        for (auto& l : lines) { Rectangle<float> bb; textBounds(0, 0, l.c_str(), nullptr, bb); w = std::max(w, bb.getWidth()); }
        const float h = 8.f + 15.f * lines.size();
        float x = hoverX + 14, y = hoverY + 18;
        if (x + w + 16 > kW) x = kW - w - 20;
        if (y + h > kHT) y = hoverY - h - 8;
        beginPath(); roundedRect(x, y, w + 16, h, 5); fillColor(shade(34.f, 0.97f)); fill(); strokeColor(cPanelEdge); stroke();
        for (size_t i = 0; i < lines.size(); ++i) labelS(x + 8, y + 12 + 15.f * i, lines[i], i == 0 ? 10.5f : 9.5f, i == 0 ? cText : (lines[i].rfind("mod", 0) == 0 ? cViolet : cDim), ALIGN_LEFT | ALIGN_MIDDLE);
    }

    // ------------------------------------------------------------------ dropdown menu
    std::vector<int> menuItems;   // enum indices shown; < 0 = group header (-1 - group)
    std::vector<Rect> menuRects;
    std::vector<std::string> menuHeaders;
    bool isFilterTypeParam(uint32_t p) const {
        return p == filtP(0, ffType) || p == filtP(1, ffType) || p == filtP(0, ffType2) || p == filtP(1, ffType2);
    }
    void openMenu(uint32_t p, const Rect& anchor) {
        menuOpen = true;
        menuParam = p;
        menuItems.clear(); menuRects.clear(); menuHeaders.clear();
        const float rowH = 21.f;
        if (isFilterTypeParam(p)) {
            // grouped in four columns (header, items...) - column breaks are explicit
            struct G { const char* name; int a, b; int col; };
            static const G groups[] = {
                { "CLASSIC", ftOff, ftDiodeLP24, 0 }, { "STATE VARIABLE", ftSvfLP12, ftSvfBP24, 0 },
                { "ANALOG MODELS", ftMs20LP, ftObHP, 1 }, { "XPANDER", ftXpLP4, ftXpPH3LP1, 1 },
                { "MULTI / VOCAL", ftDualBP, ftVowel, 2 }, { "FLANGES  ·  COMB", ftFlgFirst, ftFlgFirst + 7, 2 },
                { "FLANGES  ·  FLANGER", ftFlgFirst + 8, ftFlgFirst + 15, 3 }, { "FLANGES  ·  PHASER", ftFlgFirst + 16, ftFlgLast, 3 } };
            menuColW = 150.f; menuCols = 4;
            int rowsInCol[4] = { 0, 0, 0, 0 };
            for (const G& g : groups) {
                menuHeaders.push_back(g.name);
                menuItems.push_back(-(int)menuHeaders.size());
                menuRects.push_back({ 6 + g.col * menuColW, 6 + rowsInCol[g.col] * rowH, menuColW - 4, rowH - 1 });
                ++rowsInCol[g.col];
                for (int i = g.a; i <= g.b; ++i) {
                    menuItems.push_back(i);
                    menuRects.push_back({ 6 + g.col * menuColW, 6 + rowsInCol[g.col] * rowH, menuColW - 4, rowH - 1 });
                    ++rowsInCol[g.col];
                }
            }
            int rows = 0; for (int c = 0; c < 4; ++c) rows = std::max(rows, rowsInCol[c]);
            menuRect = { 0, 0, menuColW * 4 + 12, rows * rowH + 12 };
        } else {
            const int n = enumCount(p);
            for (int i = 0; i < n; ++i) menuItems.push_back(i);
            const int cnt = n;
            menuCols = cnt > 48 ? 4 : (cnt > 30 ? 3 : (cnt > 14 ? 2 : 1));
            const int rows = (cnt + menuCols - 1) / menuCols;
            menuColW = std::max(anchor.w, 150.f);
            for (int k = 0; k < cnt; ++k) menuRects.push_back({ 6 + (k / rows) * menuColW, 6 + (k % rows) * rowH, menuColW - 4, rowH - 1 });
            menuRect = { 0, 0, menuColW * menuCols + 12, rows * rowH + 12 };
        }
        const float w = menuRect.w, h = menuRect.h;
        float x = anchor.x, y = anchor.y + anchor.h + 3;
        if (y + h > kH - 6) y = anchor.y - h - 3;
        if (y < 60) y = std::max(60.f, kH - 6 - h);
        if (x + w > kW - 6) x = kW - 6 - w;
        if (x < 6) x = 6;
        menuRect.x = x; menuRect.y = y;
        repaint();
    }
    Rect menuItemRect(size_t k) const { const Rect& r = menuRects[k]; return { menuRect.x + r.x, menuRect.y + r.y, r.w, r.h }; }
    void drawMenu() {
        const Rect& m = menuRect;
        beginPath(); roundedRect(m.x + 3, m.y + 4, m.w, m.h, 7); fillColor(Color(0, 0, 0, 0.35f)); fill();
        beginPath(); roundedRect(m.x, m.y, m.w, m.h, 7); fillColor(shade(27.f)); fill(); strokeColor(cPanelEdge); stroke();
        const int cur = (int)std::round(values[menuParam] - P(menuParam).min);
        for (size_t k = 0; k < menuItems.size(); ++k) {
            const int i = menuItems[k];
            const Rect r = menuItemRect(k);
            if (i < 0) {
                letterSp(1.2f);
                labelS(r.x + 6, r.y + r.h * 0.5f + 2.f, menuHeaders[(size_t)(-i - 1)], 8.f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
                letterSp(0.f);
                beginPath(); moveTo(r.x + 4, r.y + r.h - 1); lineTo(r.x + r.w - 4, r.y + r.h - 1); strokeColor(cGrid); strokeWidth(1); stroke();
                continue;
            }
            const bool hot = r.contains(hoverX, hoverY);
            if (hot || i == cur) { beginPath(); roundedRect(r.x, r.y, r.w, r.h, 4); fillColor(hot ? shade(44.f) : tint(cAccent, 0.19f)); fill(); }
            labelS(r.x + 9, r.y + r.h * 0.5f + 0.5f, valueName(menuParam, P(menuParam).min + i), 10.5f, i == cur ? cAccent : cText, ALIGN_LEFT | ALIGN_MIDDLE);
        }
    }
    void menuClick(float x, float y) {
        menuOpen = false;
        if (!menuRect.contains(x, y)) return;
        int i = -1;
        for (size_t k = 0; k < menuItems.size(); ++k) if (menuItems[k] >= 0 && menuItemRect(k).contains(x, y)) { i = menuItems[k]; break; }
        if (i < 0) return;
        setParamUser(menuParam, P(menuParam).min + (float)i);
        if (isModSrc(menuParam) || isModDst(menuParam)) {
            const int slot = (int)(menuParam - pM1Src) / kSlotFields;
            const uint32_t amtP = modAmtParam(slot);
            if (values[amtP] == 0.f && i > 0 && (menuParam - pM1Src) % kSlotFields != sfVia) setParamUser(amtP, 0.3f);
        }
    }
