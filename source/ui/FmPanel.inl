// ===================================================================================== 6.0 FM oscillator panel
// (included inside class DriftfieldUI)
// Six operators, 32 algorithms; the diagram selects the operator the knob rows edit. DX7 .syx banks load into
// any FM oscillator (one voice at a time, the voice list steps through the bank).

    Rect fmSyxBtn[3], fmVoicePrev[3], fmVoiceBox[3], fmVoiceNext[3], fmOpTab[3][6], fmOpBox[3][6], fmDiag[3], fmEnvView[3];
    struct DxOp { int r[4], l[4], ld, rd, lc, rc, kvs, ol, mode, fc, ff, det; };
    struct DxVoice { DxOp op[6]; int alg = 0, fb = 0, trnsp = 24; std::string name; };   // op[0] = OP1
    std::vector<DxVoice> dxBank;
    std::string dxBankName;
    int dxVoice[3] = { -1, -1, -1 };

    int fmSelOp(int o) const { return std::max(0, std::min(5, uiSel[ukFmOpA + o])); }

    // ---- DX7 system exclusive -----------------------------------------------------------------------
    static std::string dxName(const uint8_t* p) {
        std::string s;
        for (int i = 0; i < 10; ++i) { const char c = (char)(p[i] & 0x7f); s += (c >= 32 && c < 127) ? c : ' '; }
        while (!s.empty() && s.back() == ' ') s.pop_back();
        while (!s.empty() && s.front() == ' ') s.erase(s.begin());
        return s.empty() ? std::string("(unnamed)") : s;
    }
    static DxVoice dxUnpacked(const uint8_t* d) {          // 155-byte single voice
        DxVoice v;
        for (int i = 0; i < 6; ++i) {
            const uint8_t* q = d + (5 - i) * 21;            // stored OP6 first
            DxOp& o = v.op[i];
            for (int k = 0; k < 4; ++k) { o.r[k] = q[k] & 127; o.l[k] = q[4 + k] & 127; }
            o.ld = q[9]; o.rd = q[10]; o.lc = q[11] & 3; o.rc = q[12] & 3; o.kvs = q[15] & 7; o.ol = q[16] & 127;
            o.mode = q[17] & 1; o.fc = q[18] & 31; o.ff = q[19] & 127; o.det = q[20] & 15;
        }
        v.alg = d[134] & 31; v.fb = d[135] & 7; v.trnsp = d[144] % 49; v.name = dxName(d + 145);
        return v;
    }
    static DxVoice dxPacked(const uint8_t* d) {            // 128-byte bulk entry
        DxVoice v;
        for (int i = 0; i < 6; ++i) {
            const uint8_t* q = d + (5 - i) * 17;
            DxOp& o = v.op[i];
            for (int k = 0; k < 4; ++k) { o.r[k] = q[k] & 127; o.l[k] = q[4 + k] & 127; }
            o.ld = q[9] & 127; o.rd = q[10] & 127; o.lc = q[11] & 3; o.rc = (q[11] >> 2) & 3;
            o.det = (q[12] >> 3) & 15; o.kvs = (q[13] >> 2) & 7; o.ol = q[14] & 127;
            o.mode = q[15] & 1; o.fc = (q[15] >> 1) & 31; o.ff = q[16] & 127;
        }
        v.alg = d[110] & 31; v.fb = d[111] & 7; v.trnsp = d[117] % 49; v.name = dxName(d + 118);
        return v;
    }
    // every voice in a file: bulk dumps (one or more), single-voice dumps, or a headerless 4096-byte bank
    static std::vector<DxVoice> parseSyx(const std::vector<uint8_t>& f) {
        std::vector<DxVoice> out;
        for (size_t i = 0; i + 6 <= f.size(); ++i) {
            if (f[i] != 0xF0 || f[i + 1] != 0x43) continue;
            const uint8_t fmt = f[i + 3];
            if (fmt == 9 && i + 6 + 4096 <= f.size()) { for (int k = 0; k < 32; ++k) out.push_back(dxPacked(&f[i + 6 + k * 128])); i += 4096; }
            else if (fmt == 0 && i + 6 + 155 <= f.size()) { out.push_back(dxUnpacked(&f[i + 6])); i += 155; }
        }
        if (out.empty() && f.size() == 4096) for (int k = 0; k < 32; ++k) out.push_back(dxPacked(&f[k * 128]));
        if (out.empty() && f.size() == 155) out.push_back(dxUnpacked(&f[0]));
        return out;
    }
    void openSyx(int o) {
        importTarget = o;
        browserKind = bkSyx;
        FileBrowserOptions opts;
        opts.title = "Load a DX7 bank (.syx) into the FM oscillator";
        const std::string dir = drift::lib::libraryDir();
        opts.startDir = dir.c_str();
        openFileBrowser(opts);
    }
    void syxSelected(int o, const char* path) {
        std::vector<uint8_t> f;
        if (!drift::readFileUtf8(path, f)) { flash("Could not read the file"); return; }
        auto v = parseSyx(f);
        if (v.empty()) { flash("No DX7 voices in that file (needs a bulk or single-voice dump)"); return; }
        dxBank = std::move(v);
        dxBankName = drift::lib::u8(drift::lib::fromU8(path).stem());
        for (int k = 0; k < 3; ++k) dxVoice[k] = -1;
        applyDxVoice(o, 0);
    }
    // a DX7 voice -> the FM oscillator's parameters (envelopes and scaling are approximated)
    void applyDxVoice(int o, int idx) {
        if (dxBank.empty()) return;
        idx = (idx % (int)dxBank.size() + (int)dxBank.size()) % (int)dxBank.size();
        dxVoice[o] = idx;
        const DxVoice& v = dxBank[(size_t)idx];
        pushUndo(-1);
        auto put = [&](uint32_t p, float x) { const ParamDef& d = P(p); x = std::max(d.min, std::min(d.max, x)); if (values[p] != x) { editParameter(p, true); setParam(p, x); editParameter(p, false); } };
        auto amp = [](int l) { return l <= 0 ? 0.f : std::exp2(((float)l - 99.f) / 8.f); };        // ~0.75 dB per step
        auto rateT = [](int r) { return 38.f * std::exp(-0.0937f * (float)r); };                   // rate 99 ~ 3.5 ms, 50 ~ 0.4 s
        put(oscP(o, ofType), (float)otFm);
        put(fmP(o, fmAlgo), (float)v.alg);
        put(fmP(o, fmFeedback), v.fb / 7.f);
        put(fmP(o, fmIndex), 1.f); put(fmP(o, fmBright), 0.f); put(fmP(o, fmSpread), 0.f);
        put(fmP(o, fmEnvTime), 1.f); put(fmP(o, fmVelIdx), 0.f); put(fmP(o, fmKeyIdx), 0.f);
        for (int k = 0; k < 6; ++k) {
            const DxOp& d = v.op[k];
            const float peak = std::max(amp(d.l[0]), std::max(amp(d.l[1]), 1e-4f));
            put(fmOpP(o, k, foLevel), std::sqrt(amp(d.ol) * std::min(1.f, peak)));
            put(fmOpP(o, k, foFixed), (float)d.mode);
            put(fmOpP(o, k, foCoarse), (float)d.fc);
            put(fmOpP(o, k, foFine), d.ff / 100.f);
            put(fmOpP(o, k, foHz), std::pow(10.f, (float)(d.fc & 3) + d.ff * 0.0099f));
            put(fmOpP(o, k, foDetune), (d.det - 7) * 0.9f);
            put(fmOpP(o, k, foWave), 0.f);
            put(fmOpP(o, k, foAttack), rateT(d.r[0]) * std::max(0.05f, amp(d.l[0])));
            put(fmOpP(o, k, foDecay), rateT(d.r[1]) * 0.5f + rateT(d.r[2]));
            put(fmOpP(o, k, foSustain), std::min(1.f, amp(d.l[2]) / peak));
            put(fmOpP(o, k, foRelease), rateT(d.r[3]));
            put(fmOpP(o, k, foVel), d.kvs / 7.f);
            const float sR = d.rc <= 1 ? 1.f : -1.f, sL = d.lc <= 1 ? 1.f : -1.f;
            put(fmOpP(o, k, foKey), 0.5f * (sR * d.rd - sL * d.ld) / 99.f);
        }
        const int sh = v.trnsp - 24;
        put(oscP(o, ofOct), (float)(sh / 12)); put(oscP(o, ofSemi), (float)(sh % 12)); put(oscP(o, ofFine), 0.f);
        markEdited();
        flash(("DX7 voice " + std::to_string(idx + 1) + ": " + v.name).c_str());
    }

    // ---- diagram layout: carriers on the bottom row, modulators stacked above what they modulate --------
    void fmLayout(int alg, float colOut[6], int rowOut[6], int& cols, int& rows) const {
        const drift::FmAlgo& A = drift::fmAlgoDef(alg);
        int maxD = 0;
        for (int k = 0; k < 6; ++k) { rowOut[k] = A.depth[k] > 8 ? 0 : A.depth[k]; maxD = std::max(maxD, rowOut[k]); }
        rows = maxD + 1;
        float lastInRow[8]; for (auto& x : lastInRow) x = -1.f;
        bool placed[6] = {};
        // carriers left to right, then each row in op order aligned with its first target
        for (int d = 0; d <= maxD; ++d)
            for (int k = 0; k < 6; ++k) {
                if (rowOut[k] != d || placed[k]) continue;
                float want = 0.f;
                if (d > 0) { want = 99.f; for (int c = 0; c < 6; ++c) if ((A.mods[c] >> k) & 1 && placed[c]) want = std::min(want, colOut[c]); if (want > 98.f) want = 0.f; }
                else want = lastInRow[0] + 1.f;
                colOut[k] = std::max(want, lastInRow[d] + 1.f);
                lastInRow[d] = colOut[k]; placed[k] = true;
            }
        cols = 1; for (int k = 0; k < 6; ++k) cols = std::max(cols, (int)colOut[k] + 1);
    }

    void drawFmPanel(int o) {
        const float px = oscX[o], py = oscY;
        const int alg = std::max(0, std::min(kFmAlgos - 1, ival(fmP(o, fmAlgo))));
        const drift::FmAlgo& A = drift::fmAlgoDef(alg);
        const int sel = fmSelOp(o);
        // DX7 bank
        button(fmSyxBtn[o], "SYX", false, hovered(fmSyxBtn[o]), 9.f);
        drawButtonArrow(fmVoicePrev[o], true, hovered(fmVoicePrev[o]));
        drawButtonArrow(fmVoiceNext[o], false, hovered(fmVoiceNext[o]));
        {
            const Rect& b = fmVoiceBox[o];
            beginPath(); roundedRect(b.x, b.y, b.w, b.h, 4); fillColor(hovered(b) ? shade(31.f) : cInset); fill(); strokeColor(cPanelEdge); stroke();
            save(); scissor(b.x + 4, b.y, b.w - 8, b.h);
            std::string nm = dxBank.empty() ? std::string("load a DX7 bank…") : (dxVoice[o] >= 0 ? std::to_string(dxVoice[o] + 1) + "  " + dxBank[(size_t)dxVoice[o]].name : dxBankName + " (" + std::to_string(dxBank.size()) + ")");
            labelS(b.cx(), b.cy() + 1, nm, 10.f, dxBank.empty() ? cFaint : cAccent, ALIGN_CENTER | ALIGN_MIDDLE);
            restore();
        }
        // diagram
        const Rect& r = fmDiag[o];
        inset(r);
        float col[6] = {}; int row[6] = {}; int cols = 1, rows = 1;
        fmLayout(alg, col, row, cols, rows);
        const float bw = 26.f, bh = 17.f;
        const float gx = std::min(40.f, (r.w - 24.f) / std::max(1, cols)), gy = std::min(26.f, (r.h - 22.f) / std::max(1, rows));
        const float x0 = r.x + 12.f + (r.w - 24.f - gx * cols) * 0.5f, yb = r.y + r.h - 14.f;
        auto boxAt = [&](int k) { return Rect{ x0 + col[k] * gx + (gx - bw) * 0.5f, yb - (row[k] + 1) * gy + (gy - bh) * 0.5f, bw, bh }; };
        for (int k = 0; k < 6; ++k) fmOpBox[o][k] = boxAt(k);
        // output bus
        {
            float xa = 1e9f, xb = -1e9f;
            for (int k = 0; k < 6; ++k) if ((A.carriers >> k) & 1) { xa = std::min(xa, fmOpBox[o][k].cx()); xb = std::max(xb, fmOpBox[o][k].cx()); }
            beginPath(); moveTo(xa, yb - 2); lineTo(xb, yb - 2); strokeColor(withA(cAccent, 0.7f)); strokeWidth(1.5f); stroke();
            for (int k = 0; k < 6; ++k) if ((A.carriers >> k) & 1) { const Rect& b = fmOpBox[o][k]; beginPath(); moveTo(b.cx(), b.y + b.h); lineTo(b.cx(), yb - 2); stroke(); }
        }
        // modulation lines
        strokeColor(withA(cViolet, 0.8f)); strokeWidth(1.2f);
        for (int c = 0; c < 6; ++c) for (int m = 0; m < 6; ++m) if ((A.mods[c] >> m) & 1) {
            const Rect& a = fmOpBox[o][m]; const Rect& b = fmOpBox[o][c];
            beginPath(); moveTo(a.cx(), a.y + a.h); lineTo(b.cx(), b.y); stroke();
        }
        // feedback loop
        {
            const Rect& s = fmOpBox[o][A.fbSrc]; const Rect& d = fmOpBox[o][A.fbDst];
            const float fb = values[fmP(o, fmFeedback)];
            beginPath();
            moveTo(s.x + s.w, s.cy()); lineTo(s.x + s.w + 6, s.cy());
            lineTo(std::max(s.x, d.x) + std::max(s.w, d.w) + 6, d.y - 5); lineTo(d.cx(), d.y - 5); lineTo(d.cx(), d.y);
            strokeColor(withA(cWarm, 0.25f + 0.7f * fb)); strokeWidth(1.f + fb); stroke();
        }
        for (int k = 0; k < 6; ++k) {
            const Rect& b = fmOpBox[o][k];
            const float lv = values[fmOpP(o, k, foLevel)];
            const bool car = (A.carriers >> k) & 1;
            const Color base = car ? cAccent : cViolet;
            beginPath(); roundedRect(b.x, b.y, b.w, b.h, 3);
            fillColor(mixc(cInset, base, 0.15f + 0.55f * lv)); fill();
            strokeColor(k == sel ? cText : withA(base, 0.8f)); strokeWidth(k == sel ? 2.f : 1.f); stroke();
            char t[4]; std::snprintf(t, sizeof t, "%d", k + 1);
            label(b.cx(), b.cy() + 0.5f, t, 9.5f, k == sel ? cText : cDim, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        char hd[48]; std::snprintf(hd, sizeof hd, "ALGORITHM %d · %d carrier%s", alg + 1, drift::fmCarrierCount(A), drift::fmCarrierCount(A) == 1 ? "" : "s");
        label(r.x + 8, r.y + 9, hd, 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        // selected operator: envelope picture + frequency
        const Rect& e = fmEnvView[o];
        inset(e);
        {
            const float a = values[fmOpP(o, sel, foAttack)], d = values[fmOpP(o, sel, foDecay)], s = values[fmOpP(o, sel, foSustain)], rl = values[fmOpP(o, sel, foRelease)];
            const float lv = values[fmOpP(o, sel, foLevel)];
            // time axis: log-ish segments so short and long envelopes both read
            auto seg = [](float t) { return 0.15f + std::log2(1.f + t * 4.f) * 0.25f; };
            const float wa = seg(a), wd = seg(d), wh = 0.35f, wr = seg(rl), tot = wa + wd + wh + wr;
            const float ex = e.x + 6, ew = e.w - 12, ey = e.y + 16, eh = e.h - 22;
            const float xa = ex + ew * wa / tot, xd = xa + ew * wd / tot, xs = xd + ew * wh / tot, xr = ex + ew;
            const float top = ey + eh * (1.f - lv * lv), sy = ey + eh * (1.f - lv * lv * s), bot = ey + eh;
            beginPath(); moveTo(ex, bot); lineTo(xa, top);
            for (int i = 1; i <= 12; ++i) { const float u = i / 12.f; lineTo(xa + (xd - xa) * u, top + (sy - top) * (1.f - std::exp(-u * 4.6f)) / (1.f - std::exp(-4.6f))); }
            lineTo(xs, sy);
            for (int i = 1; i <= 12; ++i) { const float u = i / 12.f; lineTo(xs + (xr - xs) * u, sy + (bot - sy) * (1.f - std::exp(-u * 6.9f)) / (1.f - std::exp(-6.9f))); }
            strokeColor((A.carriers >> sel) & 1 ? cAccent : cViolet); strokeWidth(1.6f); stroke();
            char t[64];
            if (values[fmOpP(o, sel, foFixed)] >= 0.5f) std::snprintf(t, sizeof t, "OP %d · %.1f Hz fixed", sel + 1, values[fmOpP(o, sel, foHz)]);
            else {
                const float c = std::round(values[fmOpP(o, sel, foCoarse)]);
                std::snprintf(t, sizeof t, "OP %d · ×%.2f", sel + 1, (c < 0.5f ? 0.5f : c) * (1.f + values[fmOpP(o, sel, foFine)]));
            }
            label(e.x + 6, e.y + 9, t, 8.f, cText, ALIGN_LEFT | ALIGN_MIDDLE);
            label(e.x + e.w - 6, e.y + 9, (A.carriers >> sel) & 1 ? "carrier" : "modulator", 7.5f, (A.carriers >> sel) & 1 ? cAccent : cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        // operator tabs
        for (int k = 0; k < 6; ++k) {
            char t[8]; std::snprintf(t, sizeof t, "OP %d", k + 1);
            button(fmOpTab[o][k], t, k == sel, hovered(fmOpTab[o][k]), 8.5f, (A.carriers >> k) & 1 ? &cAccent : &cViolet);
        }
        // the second row: two operator knobs, then the oscillator-wide ones
        beginPath(); moveTo(px + 90.5f, py + 266); lineTo(px + 90.5f, py + 296); strokeColor(withA(cPanelEdge, 0.8f)); strokeWidth(1); stroke();
    }

    bool fmClick(int o, float mx, float my) {
        if (fmSyxBtn[o].contains(mx, my)) { openSyx(o); return true; }
        if (fmVoicePrev[o].contains(mx, my)) { if (dxBank.empty()) openSyx(o); else applyDxVoice(o, dxVoice[o] - 1); return true; }
        if (fmVoiceNext[o].contains(mx, my)) { if (dxBank.empty()) openSyx(o); else applyDxVoice(o, dxVoice[o] + 1); return true; }
        if (fmVoiceBox[o].contains(mx, my)) { if (dxBank.empty()) openSyx(o); else applyDxVoice(o, dxVoice[o] + 1); return true; }
        for (int k = 0; k < 6; ++k)
            if (fmOpTab[o][k].contains(mx, my) || fmOpBox[o][k].contains(mx, my)) { uiSel[ukFmOpA + o] = k; repaint(); return true; }
        return false;
    }
