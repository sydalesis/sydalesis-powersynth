// ===================================================================================== PERFORM page (4.2)
// (included inside class DriftfieldUI)
// Big macros, two XY pads (each axis drives a macro), 8 scenes of macro snapshots with a morph, and a
// motion recorder that captures macro movements into looping lanes. Everything here belongs to the
// edited layer.

    Rect pfMacroCard, pfXYCard, pfSceneCard, pfMotionCard;
    Rect pfLane[kMacros], pfXY[2], pfAxisPrev[2][2], pfAxisBox[2][2], pfAxisNext[2][2];
    Rect pfScene[kScenes], pfStoreBtn, pfClearScenes, pfRecBtn, pfClearMotion, pfTimeline;
    int  pfAxis[2][2] = { { 0, 1 }, { 2, 3 } };       // [pad][0 = X, 1 = Y] -> macro
    bool pfStoreMode = false;
    // motion recording (editor side: samples the macro knobs, the engine pauses playback meanwhile)
    bool motRec = false;
    double motRecStart = 0.0;
    std::vector<float> motRecT;
    std::vector<float> motRecV;                     // kMacros values per sample
    float motRecV0[kMacros] = {};
    std::string motPrev;
    // 6.0 overdub: armed lanes record over the running loop while the other lanes keep playing;
    // motRecPh = loop position (engine time) of each sample. Lane drawing edits motDrawLane.
    bool motArm[kMacros] = {};
    bool motOverdub = false;
    int motMask = 0, motDrawLane = 0;
    std::vector<float> motRecPh;
    bool motDrawing = false; float motDrawLastX = -1.f, motDrawLastY = 0.f;

    void perfFromString(const std::string& v) {
        int a[4];
        if (std::sscanf(v.c_str(), "%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3]) == 4) {
            for (int k = 0; k < 4; ++k) pfAxis[k / 2][k % 2] = std::max(0, std::min(kMacros - 1, a[k]));
        } else { pfAxis[0][0] = 0; pfAxis[0][1] = 1; pfAxis[1][0] = 2; pfAxis[1][1] = 3; }
    }
    std::string perfToString() const {
        char b[32]; std::snprintf(b, sizeof b, "%d,%d,%d,%d", pfAxis[0][0], pfAxis[0][1], pfAxis[1][0], pfAxis[1][1]);
        return b;
    }

    void buildPerform() {
        pfMacroCard = { 10, 62, 780, 430 };
        panel(pfMacroCard, "MACROS", pgPerf);
        for (int m = 0; m < kMacros; ++m) {
            const float cx = 10 + 98 + (m % 4) * 195.f, cy = 62 + 136 + (m / 4) * 186.f;
            knob(pMacro1 + m, cx, cy, 34, nullptr, pgPerf);
            pfLane[m] = { cx - 80, cy + 64, 160, 18 };
        }
        pfXYCard = { 800, 62, 790, 430 };
        panel(pfXYCard, "XY PADS", pgPerf);
        for (int p = 0; p < 2; ++p) {
            pfXY[p] = { 812 + p * 388.f, 96, 376, 282 };
            // 5.1.1 auto-motion row: shape, size, rate, sync + division, direction
            {
                const float x = pfXY[p].x; const uint32_t o = p ? (uint32_t)(pXy2Mode - pXy1Mode) : 0u;
                selector(pXy1Mode + o, { x, 446, 118, 24 }, true, pgPerf);
                knob(pXy1Size + o, x + 146, 456, 13, "SIZE", pgPerf);
                knob(pXy1Rate + o, x + 190, 456, 13, "RATE", pgPerf);
                tog(pXy1Sync + o, { x + 218, 442, 66, 18 }, "SYNC", pgPerf);
                selector(pXy1Div + o, { x + 218, 464, 66, 18 }, false, pgPerf);
                tog(pXy1Dir + o, { x + 292, 442, 84, 18 }, "REVERSE", pgPerf);
            }
            for (int a = 0; a < 2; ++a) {
                const float x0 = pfXY[p].x + a * 190.f, y = 388;
                pfAxisPrev[p][a] = { x0 + 18, y, 20, 22 };
                pfAxisBox[p][a] = { x0 + 40, y, 122, 22 };
                pfAxisNext[p][a] = { x0 + 164, y, 20, 22 };
            }
        }
        pfMotionCard = { 10, 500, 780, 492 };
        panel(pfMotionCard, "MOTION RECORDER", pgPerf);
        pfRecBtn = { 26, 540, 118, 40 };
        choice(pMoMode, { 160, 548, 330, 24 }, 4, pgPerf);
        knob(pMoSpeed, 546, 560, 20, "SPEED", pgPerf);
        pfClearMotion = { 636, 546, 136, 28 };
        pfTimeline = { 26, 606, 748, 330 };
        pfSceneCard = { 800, 500, 790, 492 };
        panel(pfSceneCard, "SCENES", pgPerf);
        for (int i = 0; i < kScenes; ++i) pfScene[i] = { 816 + (i % 4) * 191.f, 540 + (i / 4) * 112.f, 181, 102 };
        tog(pScOn, { 816, 784, 150, 24 }, "SCENE MORPH", pgPerf);
        slider(pScPos, { 980, 785, 430, 22 }, nullptr, pgPerf);
        knob(pScGlide, 1494, 797, 20, "GLIDE", pgPerf);
        pfStoreBtn = { 816, 856, 190, 30 };
        pfClearScenes = { 1016, 856, 150, 30 };
    }

    // ---- drawing
    void drawPerform() {
        const float a0 = 0.75f * kPi, a1 = 2.25f * kPi;
        const bool lanes = motionD->frames > 1 && !motionD->recording;
        for (int m = 0; m < kMacros; ++m) {
            const float cx = 10 + 98 + (m % 4) * 195.f, cy = 62 + 136 + (m / 4) * 186.f;
            const float eff = shared ? tel.macro[m] : values[pMacro1 + m];
            const bool driven = shared && (tel.motionOn[m] || std::fabs(eff - values[pMacro1 + m]) > 0.004f);   // scenes, lanes, XY auto-motion
            if (driven) {
                const float av = a0 + (a1 - a0) * std::max(0.f, std::min(1.f, eff));
                beginPath(); arc(cx, cy, 34 + 9, a0, av, CW); strokeColor(cViolet); strokeWidth(3); stroke();
                beginPath(); circle(cx + std::cos(av) * 43.f, cy + std::sin(av) * 43.f, 3.5f); fillColor(cViolet); fill();
            }
            labelS(cx, cy + 50, macroName[m], 10.5f, cText, ALIGN_CENTER | ALIGN_MIDDLE);
            const Rect& r = pfLane[m];
            inset(r);
            if (lanes && motionD->has[m]) {
                beginPath();
                for (int i = 0; i <= 80; ++i) {
                    const int k = std::min(motionD->frames - 1, (int)((float)i / 80.f * (motionD->frames - 1)));
                    const float x = r.x + 2 + (r.w - 4) * i / 80.f, y = r.y + r.h - 3 - (r.h - 6) * motionD->v[m][k];
                    if (i == 0) moveTo(x, y); else lineTo(x, y);
                }
                strokeColor(cViolet); strokeWidth(1.2f); stroke();
                if (shared && tel.motionLen > 0.f && tel.motionOn[m]) {
                    const float x = r.x + 2 + (r.w - 4) * std::fmod(tel.motionT, tel.motionLen) / tel.motionLen;
                    beginPath(); moveTo(x, r.y + 1); lineTo(x, r.y + r.h - 1); strokeColor(cText); strokeWidth(1); stroke();
                }
            } else label(r.cx(), r.cy(), motRec && (!motOverdub || (motMask & (1 << m))) ? "recording…" : "no lane", 7.5f, motRec ? cWarn : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            // 6.0: armed for overdub (red edge) / lane being drawn (accent edge)
            if (motArm[m]) { beginPath(); roundedRect(r.x - 1, r.y - 1, r.w + 2, r.h + 2, 4); strokeColor(cWarn); strokeWidth(1.6f); stroke(); }
            if (m == motDrawLane) { beginPath(); circle(r.x - 7, r.cy(), 2.5f); fillColor(cAccent); fill(); }
        }
        label(pfMacroCard.x + 14, pfMacroCard.y + pfMacroCard.h - 30, "click a lane strip to ARM it for overdub (and to pick it for drawing)", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        if (layerSecond()) label(pfMacroCard.x + pfMacroCard.w - 14, pfMacroCard.y + 19, "layer 2's macros", 8.5f, cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
        label(pfMacroCard.x + 14, pfMacroCard.y + pfMacroCard.h - 14, "violet ring = the value a scene or motion lane is playing · route macros on the MOD page", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);

        // XY pads
        for (int p = 0; p < 2; ++p) {
            const Rect& r = pfXY[p];
            inset(r, hovered(r) || (sd == sdXY && sdIndex == p));
            for (int k = 1; k < 4; ++k) {
                beginPath(); moveTo(r.x + r.w * k / 4.f, r.y + 4); lineTo(r.x + r.w * k / 4.f, r.y + r.h - 4); strokeColor(cGrid); strokeWidth(1); stroke();
                beginPath(); moveTo(r.x + 4, r.y + r.h * k / 4.f); lineTo(r.x + r.w - 4, r.y + r.h * k / 4.f); stroke();
            }
            const int mx = pfAxis[p][0], my = pfAxis[p][1];
            const float vx = values[pMacro1 + mx], vy = values[pMacro1 + my];
            const float px = r.x + 10 + (r.w - 20) * vx, py = r.y + r.h - 10 - (r.h - 20) * vy;
            beginPath(); moveTo(px, r.y + 4); lineTo(px, r.y + r.h - 4); strokeColor(withA(cAccent, 0.25f)); stroke();
            beginPath(); moveTo(r.x + 4, py); lineTo(r.x + r.w - 4, py); stroke();
            beginPath(); circle(px, py, 9); fillColor(withA(cAccent, 0.25f)); fill(); strokeColor(cAccent); strokeWidth(2); stroke();
            if (shared) {
                const float ex = r.x + 10 + (r.w - 20) * tel.macro[mx], ey = r.y + r.h - 10 - (r.h - 20) * tel.macro[my];
                if (std::fabs(ex - px) > 1.5f || std::fabs(ey - py) > 1.5f) { beginPath(); circle(ex, ey, 6); strokeColor(cViolet); strokeWidth(2); stroke(); }
            }
            {   // 5.1.1: the auto-motion path around the pad position
                const uint32_t o = p ? (uint32_t)(pXy2Mode - pXy1Mode) : 0u;
                const int mode = ival(pXy1Mode + o);
                if (mode > 0) {
                    const float sz = 0.5f * values[pXy1Size + o]; const bool rev = values[pXy1Dir + o] >= 0.5f;
                    beginPath();
                    const int N = mode == 5 ? 800 : 160; const double span = mode == 5 ? 8.0 : (mode == 7 ? 6.0 : 1.0);
                    for (int i = 0; i <= N; ++i) {
                        float x, y; xyShapeUi(mode, span * i / N, x, y); if (rev) y = -y;
                        const float qx = std::max(0.f, std::min(1.f, vx + sz * x)), qy = std::max(0.f, std::min(1.f, vy + sz * y));
                        const float X = r.x + 10 + (r.w - 20) * qx, Y = r.y + r.h - 10 - (r.h - 20) * qy;
                        if (i == 0) moveTo(X, Y); else lineTo(X, Y);
                    }
                    strokeColor(withA(cViolet, 0.45f)); strokeWidth(1.2f); stroke();
                }
            }
            char t[48]; std::snprintf(t, sizeof t, "PAD %d", p + 1);
            label(r.x + 8, r.y + 12, t, 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            if (ival(p ? pXy2Mode : pXy1Mode) > 0) label(r.x + r.w - 8, r.y + 12, "auto-motion · drag to move its centre", 8.f, cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
            if (p == 1 && (pfAxis[1][0] == pfAxis[0][0] || pfAxis[1][0] == pfAxis[0][1] || pfAxis[1][1] == pfAxis[0][0] || pfAxis[1][1] == pfAxis[0][1]))
                label(r.x + 8, r.y + 26, "shares a macro with PAD 1 (pick another with the arrows)", 8.f, cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
            for (int a = 0; a < 2; ++a) {
                label(pfAxisPrev[p][a].x - 12, pfAxisPrev[p][a].cy(), a ? "Y" : "X", 9.5f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
                drawButtonArrow(pfAxisPrev[p][a], true, hovered(pfAxisPrev[p][a]));
                drawButtonArrow(pfAxisNext[p][a], false, hovered(pfAxisNext[p][a]));
                const Rect& b = pfAxisBox[p][a];
                beginPath(); roundedRect(b.x, b.y, b.w, b.h, 4); fillColor(cInset); fill(); strokeColor(cPanelEdge); strokeWidth(1); stroke();
                save(); scissor(b.x + 3, b.y, b.w - 6, b.h);
                labelS(b.cx(), b.cy() + 1, macroName[pfAxis[p][a]], 9.5f, cText, ALIGN_CENTER | ALIGN_MIDDLE);
                restore();
            }
        }

        // scenes
        const bool morph = values[pScOn] >= 0.5f;
        const float spos = shared ? tel.scenePos : values[pScPos];
        for (int i = 0; i < kScenes; ++i) {
            const Rect& r = pfScene[i];
            const bool used = sceneD.used[i];
            const float near = morph ? std::max(0.f, 1.f - std::fabs(spos - (float)i)) : 0.f;
            beginPath(); roundedRect(r.x, r.y, r.w, r.h, 6);
            fillColor(used ? tint(cAccent, 0.08f + 0.22f * near) : cInset); fill();
            strokeColor(pfStoreMode ? cWarm : (near > 0.5f ? cAccent : (hovered(r) ? cDim : cPanelEdge))); strokeWidth(pfStoreMode || near > 0.5f ? 2.f : 1.f); stroke();
            char t[16]; std::snprintf(t, sizeof t, "SCENE %d", i + 1);
            letterSp(1.2f);
            label(r.x + 10, r.y + 14, t, 9, used ? cText : cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            letterSp(0.f);
            if (used) {
                const float bw = (r.w - 20) / kMacros;
                for (int m = 0; m < kMacros; ++m) {
                    const float h = (r.h - 38) * sceneD.v[i][m];
                    beginPath(); rect(r.x + 10 + m * bw + 1, r.y + r.h - 8 - h, bw - 3, std::max(1.f, h)); fillColor(withA(cAccent, 0.35f + 0.5f * near)); fill();
                }
            } else label(r.cx(), r.cy() + 8, pfStoreMode ? "click to store" : "empty", 8, pfStoreMode ? cWarm : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        button(pfStoreBtn, pfStoreMode ? "CLICK A SCENE…" : "STORE MACROS", pfStoreMode, hovered(pfStoreBtn), 9.5f, &cWarm);
        button(pfClearScenes, "CLEAR SCENES", false, hovered(pfClearScenes), 9.5f);
        label(980, 772, "morph position (automatable: Scene Morph)", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(816, 912, "Click a scene to glide to it (turns Scene Morph on) · shift-click or STORE to save the current macros", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(816, 930, "Automate Scene Morph to move through the scenes: between two scenes every macro blends.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);

        // motion recorder
        {
            const Rect& b = pfRecBtn;
            const bool blink = motRec && ((nowMs() / 400) & 1);
            beginPath(); roundedRect(b.x, b.y, b.w, b.h, 8); fillColor(motRec ? tint(cWarn, 0.35f) : (hovered(b) ? shade(24) : cInset)); fill();
            strokeColor(motRec ? cWarn : cPanelEdge); strokeWidth(motRec ? 2.f : 1.f); stroke();
            beginPath(); if (motRec) rect(b.x + 18, b.cy() - 7, 14, 14); else circle(b.x + 25, b.cy(), 8); fillColor(blink || !motRec ? cWarn : withA(cWarn, 0.4f)); fill();
            label(b.x + 44, b.cy(), motRec ? "STOP" : "REC", 12, motRec ? cWarn : cText, ALIGN_LEFT | ALIGN_MIDDLE);   // REC arms; recording starts with the first move
            button(pfClearMotion, "CLEAR LANES", false, hovered(pfClearMotion), 9.5f);
            const Rect& r = pfTimeline;
            inset(r);
            char st[160];
            if (motRec) {
                const double t = secNow() - motRecStart;
                bool moved0 = false;
                for (size_t i = 0; i < motRecT.size() && !moved0; ++i) for (int m = 0; m < kMacros; ++m) if (std::fabs(motRecV[i * kMacros + m] - motRecV0[m]) > 0.004f) { moved0 = true; break; }
                if (!moved0) std::snprintf(st, sizeof st, "armed · the take starts with your first move of a macro, XY pad or mapped MIDI control");
                else std::snprintf(st, sizeof st, "recording %.1f s (max %d s) · press STOP: the loop runs from your first move to your last", t, drift::kMotionMaxSec);
                label(r.x + 10, r.y + 14, st, 8.5f, cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
                const size_t n = motRecT.size();
                if (n > 1) {
                    const float total = std::max(1.f, motRecT.back());
                    for (int m = 0; m < kMacros; ++m) {
                        bool moved = false;
                        for (size_t i = 0; i < n && !moved; ++i) moved = std::fabs(motRecV[i * kMacros + m] - motRecV0[m]) > 0.004f;
                        if (!moved) continue;
                        beginPath();
                        for (size_t i = 0; i < n; i += std::max<size_t>(1, n / 400)) {
                            const float x = r.x + 8 + (r.w - 16) * motRecT[i] / total, y = r.y + r.h - 12 - (r.h - 40) * motRecV[i * kMacros + m];
                            if (i == 0) moveTo(x, y); else lineTo(x, y);
                        }
                        strokeColor(cLfoCol[m % 6]); strokeWidth(1.4f); stroke();
                    }
                }
            } else if (lanes) {
                std::string which;
                for (int m = 0; m < kMacros; ++m) if (motionD->has[m]) { if (!which.empty()) which += ", "; which += macroName[m]; }
                std::snprintf(st, sizeof st, "%.1f s loop · lanes: ", motionD->seconds);
                labelS(r.x + 10, r.y + 14, std::string(st) + which, 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
                for (int m = 0; m < kMacros; ++m) {
                    if (!motionD->has[m]) continue;
                    beginPath();
                    for (int i = 0; i <= 300; ++i) {
                        const int k = std::min(motionD->frames - 1, (int)((float)i / 300.f * (motionD->frames - 1)));
                        const float x = r.x + 8 + (r.w - 16) * i / 300.f, y = r.y + r.h - 12 - (r.h - 40) * motionD->v[m][k];
                        if (i == 0) moveTo(x, y); else lineTo(x, y);
                    }
                    strokeColor(cLfoCol[m % 6]); strokeWidth(1.4f); stroke();
                }
                if (shared && tel.motionLen > 0.f && ival(pMoMode) != 0) {
                    const float x = r.x + 8 + (r.w - 16) * std::fmod(tel.motionT, tel.motionLen) / tel.motionLen;
                    beginPath(); moveTo(x, r.y + 24); lineTo(x, r.y + r.h - 4); strokeColor(cText); strokeWidth(1.2f); stroke();
                }
                if (ival(pMoMode) == 0) label(r.cx(), r.cy(), "playback is Off: choose Loop, Note or Host", 10, cWarm, ALIGN_CENTER | ALIGN_MIDDLE);
            } else {
                label(r.cx(), r.cy() - 10, "No motion recorded", 11, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
                label(r.cx(), r.cy() + 12, "REC captures every macro you move until STOP; playback loops it and the lane plays that macro", 8.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
                label(r.cx(), r.cy() + 30, "or click a macro's lane strip above and draw its lane in here", 8.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
            }
            label(r.x, r.y + r.h + 16, "Loop: free-running · Note: restarts when you play after silence · Host: follows the song position", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            label(r.x, r.y + r.h + 34, "A lane overrides its macro's knob. With lanes ARMED, REC overdubs only them while the others play; none armed: REC replaces all. Drag in here to draw the picked lane.", 8.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        }
    }
    bool layerSecond() const { return editLayer == 1; }
    // same shapes as the engine's xyShape (EngineCore.hpp)
    static void xyShapeUi(int mode, double ph, float& x, float& y) {
        const float th = (float)(ph * 6.283185307179586);
        switch (mode) {
        case 1: x = std::cos(th); y = std::sin(th); break;
        case 2: x = std::cos(th); y = 0.45f * std::sin(th); break;
        case 3: x = std::sin(th); y = 0.8f * std::sin(2.f * th); break;
        case 4: x = std::sin(3.f * th); y = std::sin(2.f * th + 0.5f); break;
        case 5: { const float rr = 0.15f + 0.85f * (0.5f - 0.5f * std::cos((float)(ph / 8.0 * 6.283185307179586))); x = rr * std::cos(th); y = rr * std::sin(th); break; }
        case 6: { const float a = 1.1f * std::sin(th); x = std::sin(a) / std::sin(1.1f); y = -1.f + 2.f * (1.f - std::cos(a)) / (1.f - std::cos(1.1f)); break; }
        case 7: x = 0.62f * std::sin(th) + 0.38f * std::sin(2.31f * th + 1.3f); y = 0.62f * std::sin(1.17f * th + 0.5f) + 0.38f * std::sin(2.73f * th + 2.1f); break;
        default: x = y = 0.f;
        }
    }

    // ---- input
    bool perfClick(float mx, float my, const MouseEvent& ev) {
        for (int p = 0; p < 2; ++p) {
            if (pfXY[p].contains(mx, my)) {
                pushUndo(-1);
                editParameter(pMacro1 + pfAxis[p][0], true); editParameter(pMacro1 + pfAxis[p][1], true);
                sd = sdXY; sdIndex = p; xyDrag(mx, my); return true;
            }
            for (int a = 0; a < 2; ++a) {
                int d = 0;
                if (pfAxisPrev[p][a].contains(mx, my)) d = -1;
                else if (pfAxisNext[p][a].contains(mx, my) || pfAxisBox[p][a].contains(mx, my)) d = 1;
                if (d) {
                    // 6.0: a macro belongs to one pad only (two pads on one macro moved each other and mixed their
                    // auto-motion paths), so stepping skips the other pad's macros
                    const int q = 1 - p;
                    int m = pfAxis[p][a];
                    for (int k = 0; k < kMacros; ++k) { m = (m + d + kMacros) % kMacros; if (m != pfAxis[q][0] && m != pfAxis[q][1]) break; }
                    pfAxis[p][a] = m; pushState(skPerf, perfToString(), false); return true;
                }
            }
        }
        for (int i = 0; i < kScenes; ++i) {
            if (!pfScene[i].contains(mx, my)) continue;
            if (pfStoreMode || (ev.mod & kModifierShift)) {
                drift::SceneData d = sceneD;
                d.used[i] = true;
                for (int m = 0; m < kMacros; ++m) d.v[i][m] = values[pMacro1 + m];
                pushState(skScenes, drift::scenesToString(d));
                pfStoreMode = false;
                char b[48]; std::snprintf(b, sizeof b, "Scene %d stored", i + 1); flash(b);
            } else if (sceneD.used[i]) {
                if (values[pScOn] < 0.5f) setParamUser(pScOn, 1.f);
                setParamUser(pScPos, (float)i);
            } else { char b[64]; std::snprintf(b, sizeof b, "Scene %d is empty: STORE the macros into it first", i + 1); flash(b); }
            return true;
        }
        if (pfStoreBtn.contains(mx, my)) { pfStoreMode = !pfStoreMode; repaint(); return true; }
        if (pfClearScenes.contains(mx, my)) { pushState(skScenes, drift::scenesToString(drift::SceneData())); pfStoreMode = false; flash("Scenes cleared"); return true; }
        if (pfRecBtn.contains(mx, my)) { if (motRec) stopMotionRec(); else startMotionRec(); return true; }
        for (int m = 0; m < kMacros; ++m) if (pfLane[m].contains(mx, my) && !motRec) { motArm[m] = !motArm[m]; motDrawLane = m; repaint(); return true; }
        if (pfTimeline.contains(mx, my) && !motRec) { pushUndo(-1); motDrawing = true; motDrawLastX = -1.f; motionDrawAt(mx, my); return true; }
        if (pfClearMotion.contains(mx, my)) { if (motRec) { motRec = false; } pushState(skMotion, ""); flash("Motion lanes cleared"); return true; }
        return false;
    }
    void xyDrag(float mx, float my) {
        const int p = sdIndex;
        if (p < 0 || p > 1) return;
        const Rect& r = pfXY[p];
        const float x = std::max(0.f, std::min(1.f, (mx - r.x - 10) / (r.w - 20)));
        const float y = std::max(0.f, std::min(1.f, (r.y + r.h - 10 - my) / (r.h - 20)));
        setParam(pMacro1 + pfAxis[p][0], x);
        if (pfAxis[p][1] != pfAxis[p][0]) setParam(pMacro1 + pfAxis[p][1], y);
    }
    void xyRelease() {
        if (sdIndex >= 0 && sdIndex < 2) { editParameter(pMacro1 + pfAxis[sdIndex][0], false); editParameter(pMacro1 + pfAxis[sdIndex][1], false); }
        sd = sdNone; sdIndex = -1; repaint();
    }

    // ---- motion recording
    void startMotionRec() {
        motPrev = stateValue[skMotion];
        motRec = true; motRecStart = secNow();
        motRecT.clear(); motRecV.clear(); motRecPh.clear();
        for (int m = 0; m < kMacros; ++m) motRecV0[m] = values[pMacro1 + m];
        motMask = 0; for (int m = 0; m < kMacros; ++m) if (motArm[m]) motMask |= 1 << m;
        motOverdub = motMask != 0 && motionD->frames > 1 && ival(pMoMode) != 0;
        if (motOverdub) {
            // 6.0: the armed macros pause (their knobs are heard), every other lane keeps playing
            drift::MotionData* d = new drift::MotionData(*motionD);
            d->recording = true; d->recMask = motMask;
            pushState(skMotion, drift::motionToString(*d), false);
            delete d;
        } else pushState(skMotion, "rec", false);     // the engine pauses playback: the knobs are heard while recording
        perfSample(0.f);
    }
    void perfSample(float t) {
        motRecT.push_back(t);
        motRecPh.push_back(shared && tel.motionLen > 0.f ? std::fmod(tel.motionT, tel.motionLen) : 0.f);
        for (int m = 0; m < kMacros; ++m) motRecV.push_back(values[pMacro1 + m]);
    }
    // 6.0: overdub: write the armed macros that moved into the existing loop at the positions they were played
    void stopOverdub() {
        const size_t n = motRecT.size();
        std::unique_ptr<drift::MotionData> d(new drift::MotionData(*motionD));
        d->recording = false; d->recMask = 0xFF;
        const int N = d->frames;
        bool any = false;
        for (int m = 0; m < kMacros; ++m) {
            if (!(motMask & (1 << m))) continue;
            size_t i0 = n;
            for (size_t i = 0; i < n; ++i) if (std::fabs(motRecV[i * kMacros + m] - motRecV0[m]) > 0.004f) { i0 = i > 0 ? i - 1 : 0; break; }
            if (i0 >= n) continue;
            any = true;
            if (!d->has[m]) { for (int f = 0; f < N; ++f) d->v[m][f] = motRecV0[m]; d->has[m] = true; }
            const float fpS = (float)N / std::max(0.01f, d->seconds);
            for (size_t i = i0; i + 1 < n; ++i) {
                float fa = motRecPh[i] * fpS, fb = motRecPh[i + 1] * fpS;
                if (fb < fa - 0.5f * N) fb += (float)N;          // wrapped round the loop end
                if (fb < fa) fb = fa;
                const float va = motRecV[i * kMacros + m], vb = motRecV[(i + 1) * kMacros + m];
                for (int f = (int)std::ceil(fa); f <= (int)fb; ++f) {
                    const float u = fb > fa ? ((float)f - fa) / (fb - fa) : 1.f;
                    d->v[m][((f % N) + N) % N] = va + (vb - va) * u;
                }
            }
        }
        if (!any) { pushState(skMotion, motPrev, false); flash("Nothing moved on the armed lanes: kept as they were"); return; }
        pushState(skMotion, drift::motionToString(*d));
        flash("Overdub recorded");
    }
    // 6.0: draw the picked lane in the timeline (creates an 8 s loop when there is none)
    void motionDrawAt(float mx, float my) {
        const Rect& r = pfTimeline;
        std::unique_ptr<drift::MotionData> d(new drift::MotionData(*motionD));
        d->recording = false; d->recMask = 0xFF;
        if (d->frames < 2) { d->seconds = 8.f; d->frames = 8 * drift::kMotionRate; for (bool& h : d->has) h = false; }
        const int m = motDrawLane, N = d->frames;
        if (!d->has[m]) { for (int f = 0; f < N; ++f) d->v[m][f] = values[pMacro1 + m]; d->has[m] = true; }
        auto fOf = [&](float x) { return std::max(0, std::min(N - 1, (int)std::round((x - r.x - 8) / (r.w - 16) * (N - 1)))); };
        auto vOf = [&](float y) { return std::max(0.f, std::min(1.f, (r.y + r.h - 12 - y) / (r.h - 40))); };
        const int f1 = fOf(mx); const float v1 = vOf(my);
        const int f0 = motDrawLastX < 0.f ? f1 : fOf(motDrawLastX); const float v0 = motDrawLastX < 0.f ? v1 : vOf(motDrawLastY);
        const int a = std::min(f0, f1), b = std::max(f0, f1);
        for (int f = a; f <= b; ++f) d->v[m][f] = b == a ? v1 : v0 + (v1 - v0) * (float)(f - f0) / (float)(f1 - f0);
        motDrawLastX = mx; motDrawLastY = my;
        const std::string sv = drift::motionToString(*d);
        drift::motionFromString(sv.c_str(), *motionD);
        throttledState(skMotion, [&] { return sv; });
        stateValue[skMotion] = sv;
        if (ival(pMoMode) == 0) setParamUser(pMoMode, 1.f);
        repaint();
    }
    void stopMotionRec() {
        if (!motRec) return;
        motRec = false;
        if (motOverdub) { perfSample(std::min((float)drift::kMotionMaxSec, (float)(secNow() - motRecStart))); stopOverdub(); return; }
        const float stopT = std::min((float)drift::kMotionMaxSec, (float)(secNow() - motRecStart));
        perfSample(stopT);
        const size_t n = motRecT.size();
        // 5.1.1: the take runs from the first move to the last one (REC only arms), not from the click
        size_t iFirst = n, iLast = 0;
        for (size_t i = 0; i < n; ++i) for (int m = 0; m < kMacros; ++m) {
            if (iFirst == n && std::fabs(motRecV[i * kMacros + m] - motRecV0[m]) > 0.004f) iFirst = i;
            if (i > 0 && std::fabs(motRecV[i * kMacros + m] - motRecV[(i - 1) * kMacros + m]) > 0.0005f) iLast = i;
        }
        const float t0 = iFirst < n ? motRecT[iFirst > 0 ? iFirst - 1 : 0] : 0.f;
        float t1 = iLast > iFirst && iFirst < n ? motRecT[iLast] : stopT;
        if (t1 - t0 < 0.2f) t1 = stopT;
        const float len = t1 - t0;
        std::unique_ptr<drift::MotionData> d(new drift::MotionData());
        bool any = false;
        if (iFirst < n && len >= 0.2f && n >= 2) {
            d->seconds = len;
            d->frames = std::max(2, std::min(drift::kMotionMax, (int)std::ceil(len * drift::kMotionRate)));
            for (int m = 0; m < kMacros; ++m) {
                bool moved = false;
                for (size_t i = 0; i < n && !moved; ++i) moved = std::fabs(motRecV[i * kMacros + m] - motRecV0[m]) > 0.004f;
                if (!moved) continue;
                any = true; d->has[m] = true;
                size_t j = 0;
                for (int f = 0; f < d->frames; ++f) {
                    const float t = t0 + (float)f / drift::kMotionRate;
                    while (j + 1 < n - 1 && motRecT[j + 1] <= t) ++j;
                    const float ta = motRecT[j], tb = motRecT[std::min(n - 1, j + 1)];
                    const float va = motRecV[j * kMacros + m], vb = motRecV[std::min(n - 1, j + 1) * kMacros + m];
                    const float u = tb > ta ? std::max(0.f, std::min(1.f, (t - ta) / (tb - ta))) : 0.f;
                    d->v[m][f] = va + (vb - va) * u;
                }
                // 5.1.1: crossfade the end into the start so the loop has no jump
                const int N = d->frames, w = std::max(1, std::min((int)(0.6f * drift::kMotionRate), N / 5));
                const float v0 = d->v[m][0];
                for (int f = N - w; f < N; ++f) {
                    float u = (float)(f - (N - w) + 1) / (float)w; u = u * u * (3.f - 2.f * u);
                    d->v[m][f] = d->v[m][f] * (1.f - u) + v0 * u;
                }
            }
        }
        if (!any) { pushState(skMotion, motPrev, false); flash("Nothing moved: the previous lanes are kept"); return; }
        pushState(skMotion, drift::motionToString(*d));
        if (ival(pMoMode) == 0) setParamUser(pMoMode, 1.f);
        flash("Motion recorded: playback on (Loop)");
    }
    // called every UI frame
    bool perfTick() {
        if (!motRec) return page == pgPerf && motionD->frames > 1;
        const float t = (float)(secNow() - motRecStart);
        if (t >= (float)drift::kMotionMaxSec) { stopMotionRec(); return true; }
        if (motRecT.empty() || t - motRecT.back() >= 0.01f) perfSample(t);
        return page == pgPerf;
    }
