// ===================================================================================== Mixer strips + output analyser
// (included inside class DriftfieldUI)

    // ------------------------------------------------------------------ mixer
    static constexpr int kStrips = 6;
    float mixDisp[kStrips] = {}, mixHold[kStrips] = {};
    double mixHoldT[kStrips] = {};
    Rect stripRect[kStrips], faderRect[kStrips], meterRect[kStrips];

    void buildMixer(float px, float py) {
        static const uint32_t lvl[kStrips] = { 0, 0, 0, pRingLevel, pSubLevel, pNoiseLevel };
        for (int i = 0; i < kStrips; ++i) {
            const float sx = px + 10 + i * 62.f;
            stripRect[i] = { sx, py + 30, 58, 182 };
            faderRect[i] = { sx + 13, py + 72, 14, 92 };
            meterRect[i] = { sx + 33, py + 72, 6, 92 };
            const uint32_t p = i < 3 ? oscP(i, ofLevel) : lvl[i];
            Ctl& f = addCtl(ckVFader, p, faderRect[i], nullptr, pgMain);
            (void)f;
            if (i < 3) {
                tog(oscP(i, ofOn), { sx + 6, py + 50, 46, 16 }, "ON", pgMain);
                knob(oscP(i, ofPan), sx + 29, py + 190, 9, nullptr, pgMain);
            }
        }
        // ring: source; sub: wave + octave; noise: colour
        selector(pRingSource, { stripRect[3].x + 1, py + 182, 56, 18 }, true, pgMain).small = true;
        { Ctl& c = choice(pSubOct, { stripRect[4].x + 4, py + 50, 50, 16 }, 2, pgMain); c.names = kSubOctNames; }
        selector(pSubWave, { stripRect[4].x + 1, py + 182, 56, 18 }, true, pgMain).small = true;
        knob(pNoiseColor, stripRect[5].x + 29, py + 190, 9, nullptr, pgMain);
    }
    static constexpr const char* kSubOctNames[2] = { "-2", "-1" };

    void drawVFader(const Ctl& c, const Rect& r) {
        const bool hot = hovered(r) || dragParam == (int)c.p;
        const float n = std::max(0.f, std::min(1.f, toNorm(c.p, values[c.p])));
        beginPath(); roundedRect(r.x + r.w * 0.5f - 2, r.y, 4, r.h, 2); fillColor(cInset); fill(); strokeColor(cPanelEdge); strokeWidth(1); stroke();
        const float y = r.y + r.h * (1.f - n);
        beginPath(); roundedRect(r.x + r.w * 0.5f - 2, y, 4, r.y + r.h - y, 2); fillColor(withA(cAccent, hot ? 0.9f : 0.65f)); fill();
        // live modulation on the level (osc levels are matrix destinations)
        const int d = destOf(c.p);
        if (d >= 0 && (newestVoice() || isGlobalDest(d))) {
            const float ln = std::max(0.f, std::min(1.f, n + modNormOffset(c.p, d, tel.md[d])));
            if (std::fabs(ln - n) > 0.004f) { beginPath(); circle(r.x + r.w * 0.5f, r.y + r.h * (1.f - ln), 2.6f); fillColor(cViolet); fill(); }
        }
        beginPath(); roundedRect(r.x - 1, y - 4, r.w + 2, 8, 2);
        fillPaint(linearGradient(0, y - 4, 0, y + 4, cKnobHi, cKnobLo)); fill();
        strokeColor(hot ? cAccent : cKnobEdge); strokeWidth(1); stroke();
        beginPath(); moveTo(r.x + 2, y); lineTo(r.x + r.w - 2, y); strokeColor(cKnobPtr); strokeWidth(1.2f); stroke();
    }

    void drawMixer(float, float) {
        static const char* const nm[kStrips] = { "OSC A", "OSC B", "OSC C", "RING", "SUB", "NOISE" };
        static const uint32_t lvl[kStrips] = { 0, 0, 0, pRingLevel, pSubLevel, pNoiseLevel };
        const double now = secNow();
        for (int i = 0; i < kStrips; ++i) {
            const Rect& s = stripRect[i];
            const bool live = i < 3 ? values[oscP(i, ofOn)] >= 0.5f : values[lvl[i]] > 0.001f;
            beginPath(); roundedRect(s.x, s.y, s.w, s.h, 5); fillColor(withA(cInset, live ? 1.f : 0.5f)); fill(); strokeColor(cGrid); strokeWidth(1); stroke();
            label(s.cx(), s.y + 10, nm[i], 8.2f, live ? (i < 3 ? cText : cDim) : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            // meter with peak hold
            const Rect& m = meterRect[i];
            const float lv = mixDisp[i];
            const float db = 20.f * std::log10(std::max(1e-5f, lv));
            const float nn = std::max(0.f, std::min(1.f, (db + 54.f) / 54.f));
            beginPath(); roundedRect(m.x, m.y, m.w, m.h, 2); fillColor(shade(14.f)); fill();
            if (nn > 0.f) {
                beginPath(); roundedRect(m.x, m.y + m.h * (1.f - nn), m.w, m.h * nn, 2);
                fillPaint(linearGradient(0, m.y + m.h, 0, m.y, cAccent, db > -6.f ? cWarn : cWarm)); fill();
            }
            if (lv > mixHold[i] || now - mixHoldT[i] > 1.5) { mixHold[i] = lv; mixHoldT[i] = now; }
            const float hdb = 20.f * std::log10(std::max(1e-5f, mixHold[i]));
            const float hn = std::max(0.f, std::min(1.f, (hdb + 54.f) / 54.f));
            if (hn > 0.f) { beginPath(); rect(m.x, m.y + m.h * (1.f - hn), m.w, 1.5f); fillColor(cText); fill(); }
            // value
            const uint32_t p = i < 3 ? oscP(i, ofLevel) : lvl[i];
            char b[24]; formatValue(p, values[p], b, sizeof(b));
            label(s.cx() - 4, faderRect[i].y + faderRect[i].h + 10, b, 7.8f, hovered(faderRect[i]) ? cAccent : cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            if (i == 5) {
                const float col = values[pNoiseColor];
                label(s.cx(), s.y + 28, col < 0.33f ? "pink" : (col < 0.66f ? "mixed" : "white"), 7.6f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            }
            if (i == 3) label(s.cx(), s.y + 28, "ring mod", 7.6f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            if (i < 3) label(s.cx(), s.y + 176, "pan", 6.8f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            if (i == 5) label(s.cx(), s.y + 176, "colour", 6.8f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
    }
    void tickMixerMeters(float dt, bool fresh) {
        const float k = std::exp(-dt / 0.25f);
        for (int i = 0; i < kStrips; ++i) {
            const float in = fresh ? telRaw.mixPk[i] : 0.f;
            mixDisp[i] = std::max(in, mixDisp[i] * k);
        }
    }

    // ------------------------------------------------------------------ output analyser
    enum ScopeMode { smWave, smRoll, smXY, smSpectrum, kScopeModes };
    int scopeMode = smWave, scopeGain = 0, scopeTime = 1, scopeChan = 0, scopeTilt = 1, rollLen = 1;
    bool scopeFreeze = false, specHold = true;
    Rect scopeModeBtn[kScopeModes], scopeFreezeBtn, scopeOptBtn[2];
    float frozenL[drift::kScopeSize], frozenR[drift::kScopeSize];
    int frozenW = 0;
    // roll history
    static constexpr int kRollCols = 500;
    float rollMin[kRollCols] = {}, rollMax[kRollCols] = {};
    int rollHead = 0, lastScopeW = -1; double rollAcc = 0.0; float accMin = 0.f, accMax = 0.f;
    // spectrum
    static constexpr int kFftN = 4096, kSpecPts = 256;
    float specDb[kSpecPts], specPeak[kSpecPts]; double specPeakT[kSpecPts] = {};
    bool specInit = false;
    // meters
    float outPk[2] = {}, outHold[2] = {}; double outHoldT[2] = {}, clipT = -10.0;
    float outRms = 0.f, outCorr = 1.f;

    void buildOutput() {
        static const float bw[kScopeModes] = { 44, 40, 30, 64 };
        float x = 1150;
        for (int i = 0; i < kScopeModes; ++i) { scopeModeBtn[i] = { x, 741, bw[i], 18 }; x += bw[i] + 3; }
        scopeFreezeBtn = { x + 6, 741, 52, 18 };
        scopeOptBtn[0] = { 1408, 741, 82, 18 };
        scopeOptBtn[1] = { 1494, 741, 82, 18 };
    }
    const float* scopeL() const { return scopeFreeze ? frozenL : tel.scope; }
    const float* scopeRch() const { return scopeFreeze ? frozenR : tel.scopeR; }
    int scopeW() const { return scopeFreeze ? frozenW : tel.scopeWrite; }
    float scopeRate() const { return tel.scopeRate > 1000.f ? tel.scopeRate : 48000.f; }

    std::string optText(int k) const {
        static const char* const gains[5] = { "gain auto", "gain x1", "gain x2", "gain x4", "gain x8" };
        static const char* const times[5] = { "5 ms", "10 ms", "20 ms", "40 ms", "80 ms" };
        static const char* const chans[3] = { "mid", "L / R", "side" };
        static const char* const tilts[3] = { "tilt 0 dB", "tilt 3 dB", "tilt 4.5 dB" };
        static const char* const rolls[3] = { "2 s", "5 s", "10 s" };
        switch (scopeMode) {
        case smWave: return k == 0 ? times[scopeTime] : chans[scopeChan];
        case smRoll: return k == 0 ? rolls[rollLen] : gains[scopeGain];
        case smXY: return k == 0 ? gains[scopeGain] : "";
        default: return k == 0 ? tilts[scopeTilt] : (specHold ? "peak hold on" : "peak hold off");
        }
    }
    bool outputClick(float mx, float my) {
        for (int i = 0; i < kScopeModes; ++i) if (scopeModeBtn[i].contains(mx, my)) { scopeMode = i; saveOptions(); repaint(); return true; }
        if (scopeFreezeBtn.contains(mx, my)) {
            scopeFreeze = !scopeFreeze;
            if (scopeFreeze) { std::memcpy(frozenL, tel.scope, sizeof(frozenL)); std::memcpy(frozenR, tel.scopeR, sizeof(frozenR)); frozenW = tel.scopeWrite; }
            repaint(); return true;
        }
        for (int k = 0; k < 2; ++k) if (scopeOptBtn[k].contains(mx, my)) {
            if (scopeMode == smWave) { if (k == 0) scopeTime = (scopeTime + 1) % 5; else scopeChan = (scopeChan + 1) % 3; }
            else if (scopeMode == smRoll) { if (k == 0) { rollLen = (rollLen + 1) % 3; } else scopeGain = (scopeGain + 1) % 5; }
            else if (scopeMode == smXY) { if (k == 0) scopeGain = (scopeGain + 1) % 5; }
            else { if (k == 0) scopeTilt = (scopeTilt + 1) % 3; else specHold = !specHold; }
            saveOptions(); repaint(); return true;
        }
        if (scopeRect.contains(mx, my)) { scopeMode = (scopeMode + 1) % kScopeModes; saveOptions(); repaint(); return true; }
        return false;
    }
    float fixedGain() const { static const float g[5] = { 0.f, 1.f, 2.f, 4.f, 8.f }; return g[scopeGain]; }

    // called every frame after the display telemetry is updated
    void tickOutput(float dt, bool fresh) {
        if (!shared) return;
        // meters
        const float k = std::exp(-dt / 0.3f);
        const double now = secNow();
        for (int ch = 0; ch < 2; ++ch) {
            const float in = fresh ? (ch ? telRaw.peakR : telRaw.peakL) : 0.f;
            outPk[ch] = std::max(in, outPk[ch] * k);
            if (outPk[ch] > outHold[ch] || now - outHoldT[ch] > 2.0) { outHold[ch] = outPk[ch]; outHoldT[ch] = now; }
            if (in >= 0.999f) clipT = now;
        }
        if (scopeFreeze || !fresh) return;
        const int N = drift::kScopeSize, w = tel.scopeWrite;
        // rms + correlation over the last ~80 ms
        double sl = 0, sr = 0, slr = 0, sm = 0;
        for (int i = 0; i < N; ++i) { const float l = tel.scope[i], r = tel.scopeR[i]; sl += l * l; sr += r * r; slr += l * r; sm += 0.25 * (l + r) * (l + r); }
        const float rms = (float)std::sqrt(sm / N);
        outRms = outRms * 0.7f + rms * 0.3f;
        const double den = std::sqrt(sl * sr);
        outCorr = outCorr * 0.8f + 0.2f * (den > 1e-9 ? (float)(slr / den) : 1.f);
        // roll: consume new samples
        if (lastScopeW < 0) lastScopeW = w;
        int fresh_n = (w - lastScopeW + N) & (N - 1);
        if (fresh_n > N / 2) fresh_n = N / 2;
        static const float lens[3] = { 2.f, 5.f, 10.f };
        const double per = scopeRate() * lens[rollLen] / kRollCols;
        for (int i = 0; i < fresh_n; ++i) {
            const int idx = (lastScopeW + i) & (N - 1);
            const float m = 0.5f * (tel.scope[idx] + tel.scopeR[idx]);
            if (rollAcc == 0.0) { accMin = m; accMax = m; } else { accMin = std::min(accMin, m); accMax = std::max(accMax, m); }
            rollAcc += 1.0;
            if (rollAcc >= per) { rollHead = (rollHead + 1) % kRollCols; rollMin[rollHead] = accMin; rollMax[rollHead] = accMax; rollAcc = 0.0; }
        }
        lastScopeW = w;
    }

    static void fftInPlace(float* re, float* im, int n) {
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
        }
        for (int len = 2; len <= n; len <<= 1) {
            const float ang = -2.f * kPi / len;
            const float wr = std::cos(ang), wi = std::sin(ang);
            for (int i = 0; i < n; i += len) {
                float cr = 1.f, ci = 0.f;
                for (int k = 0; k < len / 2; ++k) {
                    const int a = i + k, b = a + len / 2;
                    const float tr = re[b] * cr - im[b] * ci, ti = re[b] * ci + im[b] * cr;
                    re[b] = re[a] - tr; im[b] = im[a] - ti; re[a] += tr; im[a] += ti;
                    const float nr = cr * wr - ci * wi; ci = cr * wi + ci * wr; cr = nr;
                }
            }
        }
    }

    void drawOutputHeader() {
        static const char* const nm[kScopeModes] = { "WAVE", "ROLL", "XY", "SPECTRUM" };
        for (int i = 0; i < kScopeModes; ++i) button(scopeModeBtn[i], nm[i], scopeMode == i, hovered(scopeModeBtn[i]), 8.2f);
        button(scopeFreezeBtn, "FREEZE", scopeFreeze, hovered(scopeFreezeBtn), 8.2f, &cWarm);
        for (int k = 0; k < 2; ++k) { const std::string t = optText(k); if (!t.empty()) button(scopeOptBtn[k], t.c_str(), false, hovered(scopeOptBtn[k]), 8.f); }
    }

    void drawScope() {
        const Rect& r = scopeRect;
        inset(r);
        drawOutputHeader();
        if (!shared) { label(r.cx(), r.cy(), "no telemetry in this host", 9, cFaint, ALIGN_CENTER | ALIGN_MIDDLE); drawOutMeters(); return; }
        save(); scissor(r.x + 1, r.y + 1, r.w - 2, r.h - 2);
        switch (scopeMode) {
        case smWave: drawWave(r); break;
        case smRoll: drawRoll(r); break;
        case smXY: drawXY(r); break;
        default: drawSpectrum(r); break;
        }
        restore();
        if (scopeFreeze) label(r.x + 8, r.y + 10, "FROZEN", 8, cWarm, ALIGN_LEFT | ALIGN_MIDDLE);
        drawOutMeters();
    }

    void wavePath(const Rect& r, const float* buf, int start, int show, float g, float off) {
        const int N = drift::kScopeSize;
        beginPath();
        for (int k = 0; k < show; ++k) {
            const float x = r.x + 4 + (r.w - 8) * k / (show - 1);
            const float y = r.cy() + off - buf[(start + k) & (N - 1)] * g * r.h * 0.45f;
            if (k == 0) moveTo(x, y); else lineTo(x, y);
        }
    }
    void drawWave(const Rect& r) {
        const int N = drift::kScopeSize;
        static const float ms[5] = { 5, 10, 20, 40, 80 };
        const int show = std::min(N - 600, std::max(32, (int)(scopeRate() * ms[scopeTime] / 1000.f)));
        const float* L = scopeL(); const float* R = scopeRch();
        static float mono[drift::kScopeSize];
        for (int i = 0; i < N; ++i) mono[i] = scopeChan == 2 ? 0.5f * (L[i] - R[i]) : 0.5f * (L[i] + R[i]);
        const int w = scopeW();
        int start = (w - show - 512 + N * 4) & (N - 1);
        for (int k = 0; k < 512; ++k) { const int i0 = (start + k) & (N - 1), i1 = (start + k + 1) & (N - 1); if (mono[i0] <= 0.f && mono[i1] > 0.f) { start = i1; break; } }
        float pk = 0.02f;
        for (int k = 0; k < show; ++k) pk = std::max(pk, std::fabs(mono[(start + k) & (N - 1)]));
        const float g = scopeGain == 0 ? std::min(8.f, 0.9f / pk) : fixedGain();
        // grid
        for (int q = 1; q < 4; ++q) { const float x = r.x + r.w * q / 4.f; beginPath(); moveTo(x, r.y + 4); lineTo(x, r.y + r.h - 4); strokeColor(cGrid); strokeWidth(1); stroke(); }
        beginPath(); moveTo(r.x, r.cy()); lineTo(r.x + r.w, r.cy()); strokeColor(cGrid); stroke();
        if (scopeChan == 1) {
            wavePath(r, L, start, show, g * 0.55f, -r.h * 0.22f); strokeColor(cAccent); strokeWidth(1.1f); stroke();
            wavePath(r, R, start, show, g * 0.55f, r.h * 0.22f); strokeColor(cViolet); strokeWidth(1.1f); stroke();
            label(r.x + r.w - 8, r.y + 12, "L", 8, cAccent, ALIGN_RIGHT | ALIGN_MIDDLE);
            label(r.x + r.w - 8, r.y + r.h - 10, "R", 8, cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
        } else {
            wavePath(r, mono, start, show, g, 0.f);
            strokeColor(withA(cAccent, 0.22f)); strokeWidth(3.f); stroke();
            strokeColor(scopeChan == 2 ? cViolet : cAccent); strokeWidth(1.2f); stroke();
        }
        char b[48]; std::snprintf(b, sizeof(b), "%s  ×%.1f", scopeGain == 0 ? "auto" : "fixed", g);
        label(r.x + r.w - 8, r.y + r.h - 10, scopeChan == 1 ? "" : b, 8, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
        label(r.x + 8, r.y + r.h - 10, (std::to_string((int)ms[scopeTime]) + " ms").c_str(), 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void drawRoll(const Rect& r) {
        float pk = 0.02f;
        for (int i = 0; i < kRollCols; ++i) pk = std::max(pk, std::max(std::fabs(rollMin[i]), std::fabs(rollMax[i])));
        const float g = scopeGain == 0 ? std::min(8.f, 0.95f / pk) : fixedGain();
        beginPath(); moveTo(r.x, r.cy()); lineTo(r.x + r.w, r.cy()); strokeColor(cGrid); strokeWidth(1); stroke();
        auto xAt = [&](int i) { return r.x + 2 + (r.w - 4) * i / (kRollCols - 1); };
        beginPath();
        for (int i = 0; i < kRollCols; ++i) { const int idx = (rollHead + 1 + i) % kRollCols; const float y = r.cy() - rollMax[idx] * g * r.h * 0.46f - 0.5f; if (i == 0) moveTo(xAt(i), y); else lineTo(xAt(i), y); }
        for (int i = kRollCols - 1; i >= 0; --i) { const int idx = (rollHead + 1 + i) % kRollCols; lineTo(xAt(i), r.cy() - rollMin[idx] * g * r.h * 0.46f + 0.5f); }
        closePath();
        fillColor(withA(cAccent, 0.55f)); fill();
        strokeColor(cAccent); strokeWidth(0.8f); stroke();
        static const char* const rolls[3] = { "2 s", "5 s", "10 s" };
        label(r.x + 8, r.y + r.h - 10, rolls[rollLen], 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void drawXY(const Rect& r) {
        const int N = drift::kScopeSize, n = 1536, w = scopeW();
        const float* L = scopeL(); const float* R = scopeRch();
        float pk = 0.02f;
        for (int k = 0; k < n; ++k) { const int i = (w - n + k + N) & (N - 1); pk = std::max(pk, std::max(std::fabs(L[i]), std::fabs(R[i]))); }
        const float g = scopeGain == 0 ? std::min(8.f, 0.9f / pk) : fixedGain();
        const float cx = r.cx(), cy = r.cy(), rad = r.h * 0.46f;
        beginPath(); moveTo(cx - rad, cy); lineTo(cx + rad, cy); moveTo(cx, cy - rad); lineTo(cx, cy + rad);
        moveTo(cx - rad * 0.7f, cy - rad * 0.7f); lineTo(cx + rad * 0.7f, cy + rad * 0.7f); moveTo(cx + rad * 0.7f, cy - rad * 0.7f); lineTo(cx - rad * 0.7f, cy + rad * 0.7f);
        strokeColor(cGrid); strokeWidth(1); stroke();
        beginPath();
        for (int k = 0; k < n; ++k) {
            const int i = (w - n + k + N) & (N - 1);
            const float side = (L[i] - R[i]) * 0.7071f * g, mid = (L[i] + R[i]) * 0.7071f * g;
            const float x = cx + std::max(-1.2f, std::min(1.2f, side)) * rad, y = cy - std::max(-1.2f, std::min(1.2f, mid)) * rad;
            if (k == 0) moveTo(x, y); else lineTo(x, y);
        }
        strokeColor(withA(cAccent, 0.5f)); strokeWidth(1.f); stroke();
        label(cx, r.y + 9, "M", 7.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        label(cx + rad + 8, cy, "S", 7.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        // correlation bar
        const Rect cb = { r.x + 14, r.y + r.h - 16, 110, 6 };
        beginPath(); roundedRect(cb.x, cb.y, cb.w, cb.h, 3); fillColor(shade(14.f)); fill();
        const float cpos = cb.x + cb.w * (0.5f + 0.5f * std::max(-1.f, std::min(1.f, outCorr)));
        beginPath(); circle(cpos, cb.y + 3, 4); fillColor(outCorr < 0.f ? cWarn : cAccent); fill();
        char b[32]; std::snprintf(b, sizeof(b), "corr %+.2f", outCorr);
        label(cb.x + cb.w + 8, cb.y + 3, b, 8, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void drawSpectrum(const Rect& r) {
        const int N = drift::kScopeSize;
        static float re[kFftN], im[kFftN];
        const float* L = scopeL(); const float* R = scopeRch();
        const int w = scopeW();
        for (int k = 0; k < kFftN; ++k) {
            const int i = (w - kFftN + k + N * 2) & (N - 1);
            const float win = 0.5f - 0.5f * std::cos(2.f * kPi * k / (kFftN - 1));
            re[k] = 0.5f * (L[i] + R[i]) * win; im[k] = 0.f;
        }
        fftInPlace(re, im, kFftN);
        const float sr = scopeRate(), binHz = sr / kFftN;
        static const float tilts[3] = { 0.f, 3.f, 4.5f };
        const double now = secNow();
        const float fall = 40.f;   // dB per second
        static double lastT = now; const float dt = (float)std::max(0.0, std::min(0.1, now - lastT)); lastT = now;
        if (!specInit) { for (int p = 0; p < kSpecPts; ++p) { specDb[p] = -120.f; specPeak[p] = -120.f; } specInit = true; }
        for (int p = 0; p < kSpecPts; ++p) {
            const float f0 = 20.f * std::pow(1000.f, (float)p / kSpecPts), f1 = 20.f * std::pow(1000.f, (float)(p + 1) / kSpecPts);
            int b0 = (int)(f0 / binHz), b1 = std::max(b0 + 1, (int)(f1 / binHz));
            b0 = std::max(1, b0); b1 = std::min(kFftN / 2, b1);
            float m = 0.f;
            for (int b = b0; b < b1; ++b) m = std::max(m, re[b] * re[b] + im[b] * im[b]);
            float db = 10.f * std::log10(m + 1e-14f) - 20.f * std::log10((float)kFftN * 0.25f);
            db += tilts[scopeTilt] * std::log2(std::sqrt(f0 * f1) / 1000.f);
            if (!scopeFreeze) {
                specDb[p] = std::max(db, specDb[p] - fall * dt);
                if (specDb[p] > specPeak[p] || now - specPeakT[p] > 1.5) { specPeak[p] = specDb[p]; specPeakT[p] = now; }
            }
        }
        // grid
        static const float fl[9] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
        static const char* const fn[9] = { "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };
        auto xOf = [&](float f) { return r.x + 2 + (r.w - 4) * std::log(f / 20.f) / std::log(1000.f); };
        auto yOf = [&](float db) { return r.y + 4 + (r.h - 18) * std::max(0.f, std::min(1.f, -db / 96.f)); };
        for (int i = 0; i < 9; ++i) { const float x = xOf(fl[i]); beginPath(); moveTo(x, r.y + 2); lineTo(x, r.y + r.h - 14); strokeColor(cGrid); strokeWidth(1); stroke(); label(x, r.y + r.h - 6, fn[i], 7, cFaint, ALIGN_CENTER | ALIGN_MIDDLE); }
        for (int d = 24; d < 96; d += 24) { const float y = yOf(-(float)d); beginPath(); moveTo(r.x, y); lineTo(r.x + r.w, y); strokeColor(cGrid); stroke(); char b[8]; std::snprintf(b, sizeof(b), "-%d", d); label(r.x + 6, y - 5, b, 7, cFaint, ALIGN_LEFT | ALIGN_MIDDLE); }
        beginPath();
        moveTo(xOf(20.f), r.y + r.h - 14);
        for (int p = 0; p < kSpecPts; ++p) lineTo(xOf(20.f * std::pow(1000.f, (p + 0.5f) / kSpecPts)), yOf(specDb[p]));
        lineTo(xOf(20000.f), r.y + r.h - 14); closePath();
        fillPaint(linearGradient(0, r.y, 0, r.y + r.h, withA(cAccent, 0.35f), withA(cAccent, 0.03f))); fill();
        beginPath();
        for (int p = 0; p < kSpecPts; ++p) { const float x = xOf(20.f * std::pow(1000.f, (p + 0.5f) / kSpecPts)), y = yOf(specDb[p]); if (p == 0) moveTo(x, y); else lineTo(x, y); }
        strokeColor(cAccent); strokeWidth(1.2f); stroke();
        if (specHold) {
            beginPath();
            for (int p = 0; p < kSpecPts; ++p) { const float x = xOf(20.f * std::pow(1000.f, (p + 0.5f) / kSpecPts)), y = yOf(specPeak[p]); if (p == 0) moveTo(x, y); else lineTo(x, y); }
            strokeColor(withA(cWarm, 0.6f)); strokeWidth(1.f); stroke();
        }
    }
    void drawOutMeters() {
        const Rect& r = scopeRect;
        const float my = r.y + r.h + 10;
        const double now = secNow();
        for (int ch = 0; ch < 2; ++ch) {
            const float pk = outPk[ch];
            const float db = 20.f * std::log10(std::max(1e-5f, pk));
            const float n = std::max(0.f, std::min(1.f, (db + 60.f) / 63.f));
            const Rect m = { r.x + 22, my + ch * 12.f, r.w - 190, 7 };
            beginPath(); roundedRect(m.x, m.y, m.w, m.h, 3); fillColor(cInset); fill();
            beginPath(); roundedRect(m.x, m.y, m.w * n, m.h, 3); fillColor(db > -3.f ? cWarn : (db > -12.f ? cWarm : cAccent)); fill();
            const float hdb = 20.f * std::log10(std::max(1e-5f, outHold[ch]));
            const float hn = std::max(0.f, std::min(1.f, (hdb + 60.f) / 63.f));
            if (hn > 0.f) { beginPath(); rect(m.x + m.w * hn - 1, m.y - 1, 2, m.h + 2); fillColor(cText); fill(); }
            label(r.x + 6, m.y + 3.5f, ch ? "R" : "L", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            char b[24]; std::snprintf(b, sizeof(b), outHold[ch] > 1e-5f ? "%.1f" : "-inf", hdb);
            label(m.x + m.w + 30, m.y + 3.5f, b, 8, cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        char b[64];
        const float rdb = 20.f * std::log10(std::max(1e-5f, outRms));
        std::snprintf(b, sizeof(b), "RMS %.1f dB", rdb);
        label(r.x + r.w - 4, my + 3.5f, b, 8, cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
        std::snprintf(b, sizeof(b), "corr %+.2f", outCorr);
        label(r.x + r.w - 4, my + 15.5f, b, 8, outCorr < 0.f ? cWarn : cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
        const bool clip = now - clipT < 2.0;
        beginPath(); roundedRect(r.x + r.w - 160, my, 34, 19, 3); fillColor(clip ? cWarn : cInset); fill(); strokeColor(cPanelEdge); stroke();
        label(r.x + r.w - 143, my + 10, "CLIP", 7.5f, clip ? cBg : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
    }
