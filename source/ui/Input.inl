// ===================================================================================== Display + input
// (included inside class DriftfieldUI)

    double drawAcc = 0.0; int drawN = 0;
    void onNanoDisplay() override {
        const double t0 = secNow();
        onNanoDisplayImpl();
        drawAcc += secNow() - t0; if (++drawN == 60) { if (std::getenv("DF_DRAWPROF")) std::fprintf(stderr, "draw %.2f ms page %d\n", drawAcc / 60 * 1000.0, page); drawAcc = 0; drawN = 0; }
    }
    void onNanoDisplayImpl() {
        const float s = uiScale();
        scale(s, s);
        if (uiFont >= 0) fontFaceId(uiFont); else fontFace(NANOVG_DEJAVU_SANS_TTF);
        beginPath(); rect(0, 0, kW, kHT); fillColor(cBg); fill();
        ++fpsFrames;
        if (page == pgMod || page == pgMain) tickRandomHistory();
        drawHeader();
        for (const PanelDef& p : panels) if (p.page == page || p.page < 0) drawPanel(p);
        switch (page) {
        case pgMain: drawMain(); break;
        case pgMod: drawMod(); break;
        case pgFx: drawFx(); break;
        case pgSetup: drawSetup(); break;
        case pgTune: drawTune(); break;
        case pgLayer: drawLayers(); break;
        case pgPerf: drawPerform(); break;
        default: drawSeq(); break;
        }
        for (int i = 0; i < (int)ctls.size(); ++i) if (visible(ctls[i])) drawCtl(i);
        if (page == pgMain) for (int o = 0; o < 3; ++o) if (values[oscP(o, ofOn)] < 0.5f) dimRect({ oscX[o] + 1, oscY + 32, 388, 410 - 33 });
        if (sd == sdAssign) {
            Rect from = sourceHandleRect(assignSrc);
            beginPath(); moveTo(from.cx(), from.cy()); lineTo(hoverX, hoverY); strokeColor(withA(cViolet, 0.8f)); strokeWidth(2); stroke();
            beginPath(); circle(hoverX, hoverY, 5); fillColor(cViolet); fill();
            label(hoverX + 10, hoverY - 10, kModSrcNames[assignSrc], 9.5f, cViolet, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        drawKeyboard();
        drawGrip();
        drawTooltip();
        if (page == pgMain && envClassic()) for (int e = 0; e < 3; ++e) dimRect({ envClassicBtn[e].x - 150, envClassicBtn[e].y + 2, 146, 15 });
        drawLearnBadges();
        if (menuOpen) drawMenu();
        if (ctxOpen) drawCtx();
        drawFlash();
        if (randOpen) drawRandomize();
        if (presetOpen) drawPresetBrowser();
        if (browserOpen) drawBrowser();
        if (saveOpen) drawSaveDialog();
    }

    // which drawn handle belongs to a source (for the assign line)
    Rect sourceHandleRect(int src) const {
        if (page == pgMain) {   // 5.1.6: panel tabs / header handle
            for (int t = 0; t < kMainModTabs; ++t) if (kMainModSrc[t] == src && (src < msLfo1 || src > msLfo6)) return mainModTab[t];
        }
        if (src >= msLfo1 && src <= msLfo6) return page == pgMod ? lfoHandle[src - msLfo1] : lfoMiniHandle[src - msLfo1];
        if (src == msFiltEnv) return envHandle[0];
        if (src == msAmpEnv) return envHandle[1];
        if (src == msModEnv) return envHandle[2];
        if (src == msMseg1) return msegHandle[0];
        if (src == msMseg2) return msegHandle[1];
        if (src == msMseg3) return msegHandle[2];
        if (src == msMseg4) return msegHandle[3];
        if (src == msRndGlobal) return rndHandle[0];
        if (src == msRndVoice) return rndHandle[1];
        if (src >= msMacro1 && src <= msMacro8) return page == pgMod ? modMacroHandle[src - msMacro1] : macroHandle[src - msMacro1];
        return { hoverX, hoverY, 1, 1 };
    }
    int handleAt(float x, float y) const {
        if (page == pgMain) {
            for (int t = 0; t < kMainModTabs; ++t) if (mainModTab[t].contains(x, y)) return kMainModSrc[t];   // 5.1.6 panel tabs
            if (mainModHandle.contains(x, y)) return modSel();
            for (int i = 0; i < 3; ++i) if (envHandle[i].contains(x, y)) return i == 0 ? msFiltEnv : (i == 1 ? msAmpEnv : msModEnv);
            for (int i = 0; i < kLfos; ++i) if (lfoMiniHandle[i].contains(x, y)) return msLfo1 + i;
            for (int m = 0; m < kMacros; ++m) if (macroHandle[m].contains(x, y)) return msMacro1 + m;
        } else if (page == pgMod) {
            for (int i = 0; i < kLfos; ++i) if (lfoHandle[i].contains(x, y)) return msLfo1 + i;
            for (int m = 0; m < kMsegs; ++m) if (msegShown(m) && msegHandle[m].contains(x, y)) return msegSource(m);
            for (int g = 0; g < 2; ++g) if (rndHandle[g].contains(x, y)) return g == 0 ? msRndGlobal : msRndVoice;
            for (int m = 0; m < kMacros; ++m) if (modMacroHandle[m].contains(x, y)) return msMacro1 + m;
        }
        return 0;
    }
    // create (or reuse) a matrix slot routing src -> the knob under the cursor
    void assignTo(int src, float x, float y) {
        const int ci = ctlAt(x, y);
        if (ci < 0 || ctls[ci].kind != ckKnob) return;
        const int d = destOf(ctls[ci].p);
        if (d < 0) return;
        int slot = -1;
        for (int s = 0; s < kModSlots; ++s) if (ival(modSrcParam(s)) == src && ival(modDstParam(s)) == d) { slot = s; break; }
        if (slot < 0) for (int s = 0; s < kModSlots; ++s) if (ival(modSrcParam(s)) == msNone && ival(modDstParam(s)) == mdNone) { slot = s; break; }
        if (slot < 0) return;
        pushUndo(-1);
        auto set = [&](uint32_t p, float v) { editParameter(p, true); setParam(p, v); editParameter(p, false); };
        set(modSrcParam(slot), (float)src);
        set(modDstParam(slot), (float)d);
        if (values[modAmtParam(slot)] == 0.f) set(modAmtParam(slot), 0.25f);
        lastSlotForDest[d] = slot;
    }
    int lastSlotForDest[kModDstCount] = {};
    int routeSlotFor(int d) const {
        const int ls = lastSlotForDest[d];
        if (ival(modDstParam(ls)) == d && ival(modSrcParam(ls)) != msNone) return ls;
        for (int s = kModSlots - 1; s >= 0; --s) if (ival(modDstParam(s)) == d && ival(modSrcParam(s)) != msNone) return s;
        return -1;
    }

    void beginDrag(int ctl, uint32_t p, int kind, float mx, float my) {
        pushUndo(p);
        dragCtl = ctl; dragParam = (int)p; dragKind = kind; dragStartY = my; dragStartX = mx;
        dragStartNorm = toNorm(p, values[p]);
        editParameter(p, true);
        repaint();
    }
    bool doubleClick(int id, uint32_t t) {
        const bool dbl = t - lastClickTime < 350 && lastClickId == id;
        lastClickTime = t; lastClickId = dbl ? -1 : id;
        return dbl;
    }

    bool onMouse(const MouseEvent& ev) override {
        const float s = uiScale();
        const float mx = (float)ev.pos.getX() / s, my = (float)ev.pos.getY() / s;
        hoverX = mx; hoverY = my;
        tipShown = -1; hoverStable = -1;
        if (!ev.press) return mouseUp(mx, my, ev);
        if (textTarget == ttMacro) commitText();

        if (saveOpen)    { saveClick(mx, my); repaint(); return true; }
        if (browserOpen) { browserClick(mx, my); repaint(); return true; }
        if (presetOpen)  { presetBrowserClick(mx, my, ev.time); repaint(); return true; }
        if (randOpen)    { randomizeClick(mx, my); repaint(); return true; }
        if (menuOpen)    { menuClick(mx, my); repaint(); return true; }
        if (ctxOpen)     { ctxClick(mx, my); repaint(); return true; }

        if (ev.button == kMouseButtonRight) {
            const int ci = ctlAt(mx, my);
            if (ci >= 0 && ctls[ci].kind == ckKnob) {
                const int d = destOf(ctls[ci].p);
                const int slot = d >= 0 ? routeSlotFor(d) : -1;
                if (slot >= 0) {
                    if (ev.mod & kModifierShift) { pushUndo(-1); setParamUser(modSrcParam(slot), 0); setParamUser(modDstParam(slot), 0); setParamUser(modAmtParam(slot), 0); return true; }
                    // drag = modulation depth; release without moving = context menu (MIDI learn)
                    rightDrag = true; rightSlot = slot; rightStartAmt = values[modAmtParam(slot)]; dragStartY = my; dragStartX = mx;
                    rightMoved = false; rightCtx = (int)ctls[ci].p;
                    return true;
                }
            }
            if (ci >= 0 && learnableKind(ctls[ci].kind)) { openCtx((int)ctls[ci].p, mx + 4, my + 4); return true; }
            if (page == pgMod) for (int m = 0; m < kMsegs; ++m) if (msegShown(m) && msegEdit[m].contains(mx, my)) { msegDelete(m, mx, my); return true; }
            if (page == pgMain && msegOfSource(modSel()) >= 0 && mainMsegEdit.contains(mx, my)) { msegDelete(msegOfSource(modSel()), mx, my); return true; }
            return false;
        }
        if (ev.button != kMouseButtonLeft) return false;

        if (mx >= kW - 22 && my >= kHT - 22) { sd = sdResize; resizeStartW = (float)getWidth(); resizeStartX = (float)ev.pos.getX(); return true; }
        if (kbMouseDown(mx, my)) return true;    // 5.1.1 keyboard strip
        // header
        for (int i = 0; i < kPages; ++i) if (tabRect[i].contains(mx, my)) { page = i; repaint(); return true; }
        if (layerSwitchClick(mx, my)) return true;
        if (presetPrev.contains(mx, my)) { stepPreset(-1); return true; }
        if (presetNext.contains(mx, my)) { stepPreset(+1); return true; }
        if (presetBox.contains(mx, my))  { pbInto = 0; openPresetBrowser(); return true; }
        if (saveBtn.contains(mx, my))    { openSaveDialog(); return true; }
        for (int i = 0; i < 2; ++i) if (abBtn[i].contains(mx, my)) { switchAB(i); return true; }
        if (undoBtn.contains(mx, my)) { undo(); return true; }
        if (redoBtn.contains(mx, my)) { redo(); return true; }
        if (randBtn.contains(mx, my)) { randOpen = !randOpen; repaint(); return true; }
        for (int i = 0; i < 4; ++i) if (sizeBtn[i].contains(mx, my)) { applyScale(kSizes[i]); saveScale(kSizes[i]); return true; }

        // modulation source handles
        if (const int src = handleAt(mx, my)) { sd = sdAssign; assignSrc = src; selectMod(src); repaint(); return true; }   // 5.1.6: also picks it for the MAIN panel

        if (page == pgMain && outputClick(mx, my)) return true;
        if (page == pgMain && mainClick(mx, my, ev)) return true;
        if (page == pgMod && modClick(mx, my, ev)) return true;
        if (page == pgFx && fxClick(mx, my)) return true;
        if (page == pgSeq && seqClick(mx, my, ev)) return true;
        if (page == pgSetup && setupClick(mx, my)) return true;
        if (page == pgTune && tuneClick(mx, my)) return true;
        if (page == pgLayer && lyFaderPress(mx, my, ev.time)) return true;
        if (page == pgLayer && layersClick(mx, my)) return true;
        if (page == pgPerf && perfClick(mx, my, ev)) return true;

        // generic controls
        const int ci = ctlAt(mx, my);
        if (ci < 0) return false;
        const Ctl& c = ctls[ci];
        const Rect r = ctlRect(c);
        switch (c.kind) {
        case ckToggle: toggle(c.p); return true;
        case ckChoice: {
            const int i = (int)((mx - r.x) / (r.w / c.count));
            setParamUser(c.p, P(c.p).min + (float)std::max(0, std::min(c.count - 1, i)));
            return true;
        }
        case ckSelector:
            if (c.arrows) {
                if (mx < r.x + 20) { stepEnum(c.p, -1); return true; }
                if (mx > r.x + r.w - 20) { stepEnum(c.p, +1); return true; }
            }
            openMenu(c.p, r);
            return true;
        case ckSlider:
            if (doubleClick(ci, ev.time)) { setParamUser(c.p, P(c.p).def); return true; }
            beginDrag(ci, c.p, 3, mx, my);
            dragRect = { r.x + (c.label ? 46.f : 0.f), r.y, r.w - (c.label ? 46.f : 0.f) - 42.f, r.h };
            setParam(c.p, fromNorm(c.p, (mx - dragRect.x) / dragRect.w));
            return true;
        case ckKnob:
            if (doubleClick(ci, ev.time)) { setParamUser(c.p, P(c.p).def); return true; }
            beginDrag(ci, c.p, 1, mx, my);
            return true;
        case ckVFader:
            if (doubleClick(ci, ev.time)) { setParamUser(c.p, P(c.p).def); return true; }
            beginDrag(ci, c.p, 4, mx, my);
            dragRect = r;
            return true;
        default: return false;
        }
    }

    bool mouseUp(float mx, float my, const MouseEvent& ev) {
        (void)ev;
        if (kbMouseUp()) return true;             // 5.1.1 keyboard strip
        if (sd == sdResize) { sd = sdNone; saveScale(currentRelativeScale()); return true; }
        if (sd == sdXY) { xyRelease(); return true; }
        if (sd == sdLyFader) { lyFaderRelease(); return true; }
        if (sd == sdPbBar) { sd = sdNone; repaint(); return true; }
        if (sd == sdAssign) { sd = sdNone; assignTo(assignSrc, mx, my); repaint(); return true; }
        if (sd == sdAddDraw) { pushState(skSpA + sdIndex, drift::spectrumToString(customSpec[sdIndex]), false); sd = sdNone; return true; }
        if (sd == sdLfoDraw) { pushState(skLfo1 + sdIndex, drift::lfoShapeToString(lfoPts[sdIndex]), false); sd = sdNone; return true; }
        if (sd == sdMsegPoint || sd == sdMsegLoop || sd == sdMsegCurve) { pushState(msegStateKey(sdIndex), drift::msegToString(msegD[sdIndex]), false); sd = sdNone; repaint(); return true; }
        if (sd == sdSeqStep || sd == sdSeqGate) { sd = sdNone; return true; }
        if (sd == sdFxCard) {
            if (fxDropSlot >= 0) {
                const int from = fxSlotOf(sdIndex);
                if (from != fxDropSlot) {
                    int o[kFxUnits]; std::copy(fxOrder, fxOrder + kFxUnits, o);
                    const int u = o[from];
                    if (from < fxDropSlot) for (int i = from; i < fxDropSlot; ++i) o[i] = o[i + 1];
                    else for (int i = from; i > fxDropSlot; --i) o[i] = o[i - 1];
                    o[fxDropSlot] = u;
                    pushState(skFxOrder, drift::fxOrderToString(o));
                }
            }
            sd = sdNone; fxDropSlot = -1; repaint(); return true;
        }
        if (rightDrag) {
            if (rightMoved) editParameter(modAmtParam(rightSlot), false);
            else if (rightCtx >= 0) openCtx(rightCtx, mx + 4, my + 4);
            rightDrag = false; rightSlot = -1; rightCtx = -1; rightMoved = false;
            return true;
        }
        if (dragParam >= 0) { editParameter((uint32_t)dragParam, false); dragParam = -1; dragCtl = -1; dragKind = 0; repaint(); return true; }
        return false;
    }

    bool onMotion(const MotionEvent& ev) override {
        const float s = uiScale();
        const float mx = (float)ev.pos.getX() / s, my = (float)ev.pos.getY() / s;
        hoverX = mx; hoverY = my;
        pumpFrame();
        if (sd == sdResize) {
            const double sf = getScaleFactor();
            float w = resizeStartW + ((float)ev.pos.getX() - resizeStartX);
            w = std::max(kW * kMinScale * (float)sf, std::min(kW * kMaxScale * (float)sf, w));
            if (std::fabs(w - (float)getWidth()) >= 2.f) setSize((uint)w, (uint)(w * kHT / kW));
            return true;
        }
        if (sd == sdAssign) { repaint(); return true; }
        if (kbMotion(mx, my)) return true;        // 5.1.1 keyboard strip
        if (sd == sdXY) { xyDrag(mx, my); return true; }
        if (sd == sdLyFader) { lyFaderSet(sdIndex, mx); return true; }
        if (sd == sdPbBar) { pbBarDrag(my); return true; }
        if (sd == sdAddDraw) { addDrawAt(sdIndex, mx, my); throttledState(skSpA + sdIndex, [&] { return drift::spectrumToString(customSpec[sdIndex]); }); repaint(); return true; }
        if (sd == sdLfoDraw) { lfoDrawAt(sdIndex, mx, my); throttledState(skLfo1 + sdIndex, [&] { return drift::lfoShapeToString(lfoPts[sdIndex]); }); repaint(); return true; }
        if (sd == sdMsegPoint || sd == sdMsegLoop || sd == sdMsegCurve) { msegDrag(sdIndex, mx, my, (ev.mod & kModifierShift) != 0); throttledState(msegStateKey(sdIndex), [&] { return drift::msegToString(msegD[sdIndex]); }); repaint(); return true; }
        if (sd == sdSeqStep || sd == sdSeqGate) { seqDrag(mx, my); return true; }
        if (sd == sdFxCard) {
            fxDropSlot = -1;
            for (int sl = 0; sl < kFxUnits; ++sl) if (fxCardR[sl].contains(mx, my)) fxDropSlot = sl;
            repaint(); return true;
        }
        if (rightDrag) {
            if (!rightMoved) {
                if (std::fabs(my - dragStartY) < 3.f && std::fabs(mx - dragStartX) < 3.f) return true;
                rightMoved = true;
                pushUndo(modAmtParam(rightSlot));
                editParameter(modAmtParam(rightSlot), true);
            }
            const float sens = (ev.mod & kModifierShift) ? 900.f : 200.f;
            const float a = std::max(-1.f, std::min(1.f, rightStartAmt + (dragStartY - my) / sens));
            setParam(modAmtParam(rightSlot), std::round(a * 1000.f) / 1000.f);
            return true;
        }
        if (dragParam >= 0) {
            const uint32_t p = (uint32_t)dragParam;
            float v = values[p];
            if (dragKind == 1) {
                const float sens = (ev.mod & (kModifierShift | kModifierControl)) ? 900.f : 180.f;
                float n = dragStartNorm + (dragStartY - my) / sens;
                if (P(p).flags & fInt) {
                    const float steps = P(p).max - P(p).min;
                    const float pps = std::max(5.f, 120.f / steps);
                    n = dragStartNorm + std::round((dragStartY - my) / pps) / steps;
                }
                v = fromNorm(p, n);
            } else if (dragKind == 2) {
                v = std::max(0.f, std::min(1.f, (mx - dragRect.x - 10.f) / (dragRect.w - 20.f)));
            } else if (dragKind == 3) {
                const float fine = (ev.mod & (kModifierShift | kModifierControl)) ? 0.2f : 1.f;
                v = fromNorm(p, dragStartNorm + (mx - dragStartX) / dragRect.w * fine);
            } else if (dragKind == 4) {
                const float fine = (ev.mod & (kModifierShift | kModifierControl)) ? 0.2f : 1.f;
                v = fromNorm(p, dragStartNorm + (dragStartY - my) / dragRect.h * fine);
            }
            if (v != values[p]) setParam(p, v);
            return true;
        }
        const int h = anyOverlay() ? -1 : ctlAt(mx, my);
        if (h != hoverCtl) { hoverCtl = h; hoverStable = h; hoverSince = nowMs(); tipShown = -1; }
        repaint();
        return false;
    }

    template <typename F> void throttledState(int k, F make) {
        const uint32_t t = nowMs();
        if (t - lastStatePush < 40) return;
        lastStatePush = t;
        const std::string v = make();
        stateValue[k] = v;
        setState(kStateKeys[k], v.c_str());
    }

    bool onScroll(const ScrollEvent& ev) override {
        pumpFrame();
        const float s = uiScale();
        const float mx = (float)ev.pos.getX() / s, my = (float)ev.pos.getY() / s;
        const float dy = (float)ev.delta.getY();
        if (browserOpen) {
            if (browserListRect.contains(mx, my)) browserScroll = std::max(0.f, browserScroll - dy * 48.f);
            else if (browserCatRect.contains(mx, my)) catScroll = std::max(0.f, catScroll - dy * 48.f);
            repaint(); return true;
        }
        if (presetOpen) {
            if (pbListRect.contains(mx, my)) pbScroll = std::max(0.f, pbScroll - dy * 90.f);          // 3 rows per notch
            else if (pbCatRect.contains(mx, my)) pbCatScroll = std::max(0.f, pbCatScroll - dy * 56.f);
            repaint(); return true;
        }
        if (menuOpen || saveOpen || randOpen) return true;
        int ci = ctlAt(mx, my);
        int p = -1;
        if (ci >= 0) {
            const Ctl& c = ctls[ci];
            if (c.kind == ckSelector || c.kind == ckChoice) { stepEnum(c.p, dy > 0 ? -1 : 1); return true; }
            if (c.kind == ckKnob || c.kind == ckSlider) p = (int)c.p;
        }
        if (page == pgMain) for (int o = 0; o < 3; ++o) if (oscType(o) == otWavetable && engDisp[o].contains(mx, my)) p = (int)oscP(o, ofPos);
        if (p < 0) return false;
        float v;
        if (P(p).flags & fInt) v = values[p] + (dy > 0 ? 1.f : -1.f);
        else v = fromNorm(p, toNorm(p, values[p]) + dy * ((ev.mod & kModifierShift) ? 0.004f : 0.02f));
        v = std::max(P(p).min, std::min(P(p).max, v));
        pushUndoCoalesced(p);
        editParameter(p, true); setParam(p, v); editParameter(p, false);
        return true;
    }

    bool onKeyboard(const KeyboardEvent& ev) override {
        if (!ev.press) return false;
        if (textTarget != ttNone) {
            if (ev.key == kKeyEscape) { textTarget = ttNone; saveOpen = false; repaint(); return true; }
            if (ev.key == '\r' || ev.key == '\n') { if (textTarget == ttPresetName) doSave(); else commitText(); repaint(); return true; }
            if (ev.key == '\b' || ev.key == 127) {
                std::string& b = textTarget == ttPresetName ? saveName : (textTarget == ttSearch ? searchText : textBuf);
                if (!b.empty()) { do b.pop_back(); while (!b.empty() && ((unsigned char)b.back() & 0xC0) == 0x80); }
                if (textTarget == ttSearch) pbScroll = 0.f;
                repaint(); return true;
            }
            return true;
        }
        if (ev.key == kKeyEscape && ctxOpen) { ctxOpen = false; repaint(); return true; }
        if (ev.key == kKeyEscape && shared && shared->learnArm.load() >= 0) { shared->learnArm.store(-1); learnWaiting = false; repaint(); return true; }
        if (ev.key == kKeyEscape && anyOverlay()) { browserOpen = presetOpen = menuOpen = randOpen = false; repaint(); return true; }
        if ((ev.mod & kModifierControl) && (ev.key == 'z' || ev.key == 'Z')) { if (ev.mod & kModifierShift) redo(); else undo(); return true; }
        if ((ev.mod & kModifierControl) && (ev.key == 'y' || ev.key == 'Y')) { redo(); return true; }
        return false;
    }
    bool onCharacterInput(const CharacterInputEvent& ev) override {
        if (textTarget == ttNone) return false;
        std::string& b = textTarget == ttPresetName ? saveName : (textTarget == ttSearch ? searchText : textBuf);
        if (ev.character >= 32 && ev.character != 127 && b.size() < 48) {
            // 5.1.4: hosts that pass keys through VST3 onKeyDown (Studio One) arrive with the character code but an
            // empty string, so typing did nothing; build the UTF-8 text from the code point in that case
            char enc[8] = { 0 };
            const char* c = ev.string;
            if (c[0] == '\0') {
                const uint32_t u = ev.character;
                if (u < 0x80) enc[0] = (char)u;
                else if (u < 0x800) { enc[0] = (char)(0xC0 | (u >> 6)); enc[1] = (char)(0x80 | (u & 0x3F)); }
                else if (u < 0x10000) { enc[0] = (char)(0xE0 | (u >> 12)); enc[1] = (char)(0x80 | ((u >> 6) & 0x3F)); enc[2] = (char)(0x80 | (u & 0x3F)); }
                else { enc[0] = (char)(0xF0 | (u >> 18)); enc[1] = (char)(0x80 | ((u >> 12) & 0x3F)); enc[2] = (char)(0x80 | ((u >> 6) & 0x3F)); enc[3] = (char)(0x80 | (u & 0x3F)); }
                c = enc;
            }
            const bool bad = textTarget != ttSearch && std::strpbrk(c, "/\\:*?\"<>|") != nullptr;
            if (!bad) b += c;
            if (textTarget == ttSearch) pbScroll = 0.f;
        }
        repaint();
        return true;
    }
    void commitText() {
        if (textTarget == ttMacro && textIndex >= 0) {
            std::string nm = textBuf;
            for (auto& ch : nm) if (ch == '|') ch = '/';
            if (nm.empty()) nm = "Macro " + std::to_string(textIndex + 1);
            macroName[textIndex] = nm;
            std::string all;
            for (int m = 0; m < kMacros; ++m) all += (m ? "|" : "") + macroName[m];
            pushState(skMacroNames, all);
        }
        textTarget = ttNone; textIndex = -1;
    }

    // ------------------------------------------------------------------ page-specific clicks
    bool mainClick(float mx, float my, const MouseEvent& ev) {
        if (modPanelClick(mx, my, ev)) return true;   // 5.1.6
        for (int e = 0; e < 3; ++e) if (envClassicBtn[e].contains(mx, my)) { pushUndo(-1); setEnvClassic(!envClassic()); flash(envClassic() ? "3.0 attack: one-pole rise, as in Driftfield 3.0 sessions" : "3.1 timed attack with CURVE"); return true; }
        for (int o = 0; o < 3; ++o) {
            if (onBtn[o].contains(mx, my)) { toggle(oscP(o, ofOn)); return true; }
            for (int t = 0; t < kOscTypes; ++t) if (typeTab[o][t].contains(mx, my)) { setParamUser(oscP(o, ofType), (float)t); return true; }
            const int type = oscType(o);
            if (type == otWavetable || (type == otGranular && ival(oscP(o, ofGrSource)) == 0)) {
                const Rect& pv = type == otWavetable ? tablePrev[o] : smpPrev[o];
                const Rect& bx = type == otWavetable ? tableBox[o] : smpBox[o];
                const Rect& nx = type == otWavetable ? tableNext[o] : smpNext[o];
                if (pv.contains(mx, my)) { stepTable(o, -1); return true; }
                if (nx.contains(mx, my)) { stepTable(o, +1); return true; }
                if (bx.contains(mx, my)) { openBrowser(o, false); return true; }
            }
            if (type == otGranular && ival(oscP(o, ofGrSource)) != 0) {
                if (smpPrev[o].contains(mx, my)) { stepSample(o, -1); return true; }
                if (smpNext[o].contains(mx, my)) { stepSample(o, +1); return true; }
                if (smpBox[o].contains(mx, my)) { openBrowser(o, true); return true; }
            }
            if (type == otWavetable && engDisp[o].contains(mx, my)) {
                beginDrag(-1, oscP(o, ofPos), 2, mx, my);
                dragRect = engDisp[o];
                setParam(oscP(o, ofPos), std::max(0.f, std::min(1.f, (mx - dragRect.x - 10.f) / (dragRect.w - 20.f))));
                return true;
            }
            if (type == otGranular && engDisp[o].contains(mx, my)) {
                beginDrag(-1, oscP(o, ofGrPos), 2, mx, my);
                dragRect = engDisp[o];
                setParam(oscP(o, ofGrPos), std::max(0.f, std::min(1.f, (mx - dragRect.x - 10.f) / (dragRect.w - 20.f))));
                return true;
            }
            if (type == otSpectral) {
                if (spcPrev[o].contains(mx, my)) { stepSpec(o, -1, bkSpec); return true; }
                if (spcNext[o].contains(mx, my)) { stepSpec(o, +1, bkSpec); return true; }
                if (spcBox[o].contains(mx, my)) { openBrowserKind(o, bkSpec); return true; }
                if (spcX[o].contains(mx, my)) { openBrowserKind(o, bkSpecX); return true; }
                if (engDisp[o].contains(mx, my)) {
                    beginDrag(-1, osc2P(o, o2SpPos), 2, mx, my);
                    dragRect = engDisp[o];
                    setParam(osc2P(o, o2SpPos), std::max(0.f, std::min(1.f, (mx - dragRect.x - 6.f) / (dragRect.w - 12.f))));
                    return true;
                }
            }
            if (type == otAdditive) {
                if (specEdit[o].contains(mx, my)) { initCustomFromCurrent(o); return true; }
                if (specImport[o].contains(mx, my)) { openImport(o); return true; }
                if (freezeBtn[o].contains(mx, my)) { toggle(oscP(o, ofAdFreeze)); return true; }
                if (engDisp[o].contains(mx, my)) { startAddDraw(o, mx, my); return true; }
            }
        }
        for (int i = 0; i < 3; ++i) if (routeBtn[i].contains(mx, my)) { setParamUser(pFRouting, (float)i); return true; }
        for (int m = 0; m < kMacros; ++m)
            if (macroLabel[m].contains(mx, my)) {
                if (doubleClick(1000 + m, ev.time)) { textTarget = ttMacro; textIndex = m; textBuf = macroName[m]; repaint(); }
                return true;
            }
        // 5.1.6: click = show it in the modulator panel, double-click = open the MOD page
        for (int i = 0; i < kLfos; ++i) if (lfoMini[i].contains(mx, my)) { if (doubleClick(4000 + i, ev.time)) page = pgMod; else selectMod(msLfo1 + i); repaint(); return true; }
        return false;
    }
    bool modClick(float mx, float my, const MouseEvent& ev) {
        for (int i = 0; i < kLfos; ++i) if (lfoEdit[i].contains(mx, my)) { selectMod(msLfo1 + i); startLfoDraw(i, mx, my); return true; }
        for (int m = 0; m < kMsegs; ++m) if (msegTab[m].contains(mx, my)) { uiSel[ukMsegA + (m & 1)] = m >> 1; selectMod(msegSource(m)); repaint(); return true; }
        for (int g = 0; g < 2; ++g) if (rndDisp[g].contains(mx, my)) { selectMod(g == 0 ? msRndGlobal : msRndVoice); return true; }
        for (int h = 0; h < 2; ++h) if (matTab[h].contains(mx, my)) { uiSel[ukMatrix] = h; repaint(); return true; }
        for (int m = 0; m < kMsegs; ++m) if (msegShown(m) && msegEdit[m].contains(mx, my)) { selectMod(msegSource(m)); msegPress(m, mx, my, ev); return true; }
        return false;
    }
    bool fxClick(float mx, float my) {
        for (int s = 0; s < kFxUnits; ++s) {
            const Rect& c = fxCardR[s];
            const Rect grip = { c.x + 4, c.y + 4, 200, 26 };
            if (grip.contains(mx, my)) { sd = sdFxCard; sdIndex = fxOrder[s]; fxDropSlot = s; repaint(); return true; }
        }
        return false;
    }
    bool seqClick(float mx, float my, const MouseEvent& ev) {
        (void)ev;
        if (seqSteps.contains(mx, my)) { pushUndo(-1); sd = sdSeqStep; seqDrag(mx, my); return true; }
        if (seqGates.contains(mx, my)) {
            const int s = (int)((mx - seqGates.x) / (seqGates.w / kSeqSteps));
            if (s >= 0 && s < kSeqSteps) { pushUndo(-1); seqGatePaint = values[pSeqG1 + s] < 0.5f; sd = sdSeqGate; seqDrag(mx, my); }
            return true;
        }
        return false;
    }
    void seqDrag(float mx, float my) {
        if (sd == sdSeqStep) {
            const int s = (int)((mx - seqSteps.x) / (seqSteps.w / kSeqSteps));
            if (s < 0 || s >= kSeqSteps) return;
            float v = (seqSteps.cy() - my) / (seqSteps.h * 0.46f);
            v = std::max(-1.f, std::min(1.f, v));
            if (ival(pSeqMode) == 2) v = std::round(v * 24.f) / 24.f;
            editParameter(pSeqV1 + s, true); setParam(pSeqV1 + s, v); editParameter(pSeqV1 + s, false);
        } else {
            const int s = (int)((mx - seqGates.x) / (seqGates.w / kSeqSteps));
            if (s < 0 || s >= kSeqSteps) return;
            const float v = seqGatePaint ? 1.f : 0.f;
            if (values[pSeqG1 + s] != v) { editParameter(pSeqG1 + s, true); setParam(pSeqG1 + s, v); editParameter(pSeqG1 + s, false); }
        }
    }

    // ------------------------------------------------------------------ additive drawing / import
    void initCustomFromCurrent(int o) {
        float base[drift::kPartials], shaped[drift::kPartials];
        additiveSpectra(o, base, shaped);
        for (int i = 0; i < drift::kPartials; ++i) customSpec[o][i] = std::max(0.f, std::min(1.f, base[i]));
        pushState(skSpA + o, drift::spectrumToString(customSpec[o]));
        setParamUser(oscP(o, ofAdSpec1), (float)kSpecCustom);
        setParamUser(oscP(o, ofAdMorph), 0.f);
    }
    void startAddDraw(int o, float mx, float my) {
        const bool editable = ival(oscP(o, ofAdSpec1)) == kSpecCustom || ival(oscP(o, ofAdSpec2)) == kSpecCustom;
        if (!editable) initCustomFromCurrent(o); else pushUndo(-1);
        sd = sdAddDraw; sdIndex = o; sdLastX = -1.f;
        addDrawAt(o, mx, my);
    }
    void addDrawAt(int o, float mx, float my) {
        const Rect& r = engDisp[o];
        const int nShow = std::max(16, std::min(drift::kPartials, ival(oscP(o, ofAdPartials)) <= 64 ? 64 : 128));
        const float bw = (r.w - 16) / (float)nShow;
        const float top = r.y + 18, bot = r.y + r.h - 6;
        const float v = std::max(0.f, std::min(1.f, (bot - my) / (bot - top)));
        int i1 = std::max(0, std::min(nShow - 1, (int)((mx - r.x - 8) / bw)));
        int i0 = sdLastX < 0 ? i1 : std::max(0, std::min(nShow - 1, (int)((sdLastX - r.x - 8) / bw)));
        const float v0 = sdLastX < 0 ? v : std::max(0.f, std::min(1.f, (bot - sdLastY) / (bot - top)));
        const int a = std::min(i0, i1), b = std::max(i0, i1);
        for (int i = a; i <= b; ++i) {
            const float t = b == a ? 1.f : (float)(i - i0) / (float)(i1 - i0);
            customSpec[o][i] = v0 + (v - v0) * std::max(0.f, std::min(1.f, t));
        }
        sdLastX = mx; sdLastY = my;
    }
    int importTarget = -1;
    void openImport(int o) {
        importTarget = o;
        browserKind = bkImport;
        FileBrowserOptions opts;
        opts.title = "Import a WAV as additive spectrum";
        const std::string dir = drift::lib::libraryDir();
        opts.startDir = dir.c_str();
        openFileBrowser(opts);
    }
    // turns one cycle (the frame at the osc's wavetable position) into 128 harmonic amplitudes
    void importSpectrum(int o, const char* path) {
        std::unique_ptr<drift::WaveTable> t(drift::loadWavetableFile(path, true));
        if (!t || t->frames <= 0) return;
        const int fr = (int)std::round(values[oscP(o, ofPos)] * (t->frames - 1));
        const float* cyc = t->table(fr, 0);
        std::vector<drift::cpx> buf(drift::kWtSize);
        for (int i = 0; i < drift::kWtSize; ++i) buf[i] = drift::cpx(cyc[i], 0.f);
        drift::fft::forward(buf);
        float mx = 1e-9f;
        for (int h = 1; h <= drift::kPartials; ++h) { customSpec[o][h - 1] = std::abs(buf[h]); mx = std::max(mx, customSpec[o][h - 1]); }
        for (int h = 0; h < drift::kPartials; ++h) customSpec[o][h] /= mx;
        pushState(skSpA + o, drift::spectrumToString(customSpec[o]));
        setParamUser(oscP(o, ofAdSpec1), (float)kSpecCustom);
        setParamUser(oscP(o, ofAdMorph), 0.f);
        if (ival(oscP(o, ofAdPartials)) < 64) setParamUser(oscP(o, ofAdPartials), 96.f);
    }

    // ------------------------------------------------------------------ LFO shape drawing
    void startLfoDraw(int i, float mx, float my) {
        pushUndo(-1);
        if (ival(lfoP(i, lfWave)) != 7) {
            for (int k = 0; k < drift::kLfoShapePoints; ++k) lfoPts[i][k] = lfoShapeValue(i, (float)k / drift::kLfoShapePoints);
            editParameter(lfoP(i, lfWave), true); setParam(lfoP(i, lfWave), 7.f); editParameter(lfoP(i, lfWave), false);
        }
        sd = sdLfoDraw; sdIndex = i; sdLastX = -1.f;
        lfoDrawAt(i, mx, my);
    }
    void lfoDrawAt(int i, float mx, float my) {
        const Rect r = lfoR(i);
        const float v = std::max(-1.f, std::min(1.f, (r.cy() - my) / (r.h * 0.4f)));
        auto idx = [&](float x) { return std::max(0, std::min(drift::kLfoShapePoints - 1, (int)((x - r.x - 4) / (r.w - 8) * drift::kLfoShapePoints))); };
        const int i1 = idx(mx), i0 = sdLastX < 0 ? i1 : idx(sdLastX);
        const float v0 = sdLastX < 0 ? v : std::max(-1.f, std::min(1.f, (r.cy() - sdLastY) / (r.h * 0.4f)));
        const int a = std::min(i0, i1), b = std::max(i0, i1);
        for (int k = a; k <= b; ++k) { const float t = b == a ? 1.f : (float)(k - i0) / (float)(i1 - i0); lfoPts[i][k] = v0 + (v - v0) * std::max(0.f, std::min(1.f, t)); }
        sdLastX = mx; sdLastY = my;
    }

    // ------------------------------------------------------------------ MSEG editing
    float msegLevelAt(int m, float my) const {   // inverse of msegPointY
        const Rect r = msegR(m); const float pb = page == pgMain ? 10.f : 14.f, pt = page == pgMain ? 18.f : 26.f;
        return std::max(0.f, std::min(1.f, (r.y + r.h - pb - my) / (r.h - pt)));
    }
    int msegHit(int m, float mx, float my) const {
        const float total = msegTotal(m);
        for (int i = 0; i < msegD[m].count; ++i)
            if (std::fabs(mx - msegPointX(m, i, total)) < 8 && std::fabs(my - msegPointY(m, msegD[m].l[i])) < 8) return i;
        return -1;
    }
    void msegPress(int m, float mx, float my, const MouseEvent& ev) {
        drift::MsegData& d = msegD[m];
        const float total = msegTotal(m);
        msegDragTotal = total;
        pushUndo(-1);
        const Rect r = msegR(m);
        // loop handles
        const float hy = r.y + r.h - 6;
        if (std::fabs(my - hy) < 8) {
            if (std::fabs(mx - msegPointX(m, d.loopEnd, total)) < 8) { sd = sdMsegLoop; sdIndex = m; sdSub = 1; return; }
            if (std::fabs(mx - msegPointX(m, d.loopStart, total)) < 8) { sd = sdMsegLoop; sdIndex = m; sdSub = 0; return; }
        }
        const int hit = msegHit(m, mx, my);
        if (hit >= 0) {
            if (doubleClick(2000 + m * 32 + hit, ev.time)) { msegDelete(m, mx, my); return; }
            sd = sdMsegPoint; sdIndex = m; sdSub = hit; return;
        }
        // segment under the cursor
        int seg = -1;
        for (int i = 1; i < d.count; ++i) if (mx >= msegPointX(m, i - 1, total) && mx <= msegPointX(m, i, total)) { seg = i; break; }
        if ((ev.mod & kModifierShift) && seg > 0) { sd = sdMsegCurve; sdIndex = m; sdSub = seg; sdAux = d.c[seg]; sdLastY = my; return; }
        // add a point
        if (d.count >= drift::kMsegMaxPoints) return;
        const float t = std::max(0.f, (mx - r.x - 8) / (r.w - 16) * total);
        const float l = msegLevelAt(m, my);
        float acc = 0.f; int ins = d.count;
        for (int i = 1; i < d.count; ++i) { if (acc + d.t[i] > t) { ins = i; break; } acc += d.t[i]; }
        if (ins == d.count) {   // after the last point
            d.t[d.count] = std::max(0.01f, t - acc); d.l[d.count] = l; d.c[d.count] = 0.f; ++d.count;
        } else {
            for (int i = d.count; i > ins; --i) { d.t[i] = d.t[i - 1]; d.l[i] = d.l[i - 1]; d.c[i] = d.c[i - 1]; }
            const float before = std::max(0.005f, t - acc), after = std::max(0.005f, d.t[ins + 1] - before);
            d.t[ins] = before; d.l[ins] = l; d.c[ins] = 0.f; d.t[ins + 1] = after;
            ++d.count;
            if (d.loopStart >= ins) ++d.loopStart;
            if (d.loopEnd >= ins) ++d.loopEnd;
        }
        sd = sdMsegPoint; sdIndex = m; sdSub = ins;
    }
    void msegDrag(int m, float mx, float my, bool shift) {
        drift::MsegData& d = msegD[m];
        const Rect r = msegR(m);
        const float total = msegDragTotal;
        if (sd == sdMsegCurve) {
            d.c[sdSub] = std::max(-1.f, std::min(1.f, sdAux + (sdLastY - my) / 80.f * (d.l[sdSub] >= d.l[sdSub - 1] ? 1.f : -1.f)));
            return;
        }
        if (sd == sdMsegLoop) {
            int best = 0; float bd = 1e9f;
            for (int i = 0; i < d.count; ++i) { const float dx = std::fabs(mx - msegPointX(m, i, total)); if (dx < bd) { bd = dx; best = i; } }
            if (sdSub == 0) d.loopStart = std::min(best, d.loopEnd); else d.loopEnd = std::max(best, std::max(1, d.loopStart));
            return;
        }
        const int i = sdSub;
        if (i < 0 || i >= d.count) return;
        d.l[i] = msegLevelAt(m, my);
        if (shift) d.l[i] = std::round(d.l[i] * 8.f) / 8.f;
        if (i == 0) return;
        // horizontal: move the point between its neighbours, keep later points where they are
        float tStart = 0.f; for (int k = 1; k < i; ++k) tStart += d.t[k];
        float t = (mx - r.x - 8) / (r.w - 16) * total - tStart;
        if (i == d.count - 1) d.t[i] = std::max(0.005f, std::min(60.f, t));
        else {
            const float span = d.t[i] + d.t[i + 1];
            t = std::max(0.005f, std::min(span - 0.005f, t));
            d.t[i] = t; d.t[i + 1] = span - t;
        }
    }
    void msegDelete(int m, float mx, float my) {
        drift::MsegData& d = msegD[m];
        const int hit = msegHit(m, mx, my);
        if (hit <= 0 || d.count <= 2) return;
        pushUndo(-1);
        if (hit < d.count - 1) d.t[hit + 1] += d.t[hit];
        for (int i = hit; i < d.count - 1; ++i) { d.t[i] = d.t[i + 1]; d.l[i] = d.l[i + 1]; d.c[i] = d.c[i + 1]; }
        --d.count;
        if (d.loopStart >= hit && d.loopStart > 0) --d.loopStart;
        if (d.loopEnd >= hit && d.loopEnd > 0) --d.loopEnd;
        d.loopEnd = std::min(d.loopEnd, d.count - 1);
        d.loopStart = std::min(d.loopStart, d.loopEnd);
        pushState(msegStateKey(m), drift::msegToString(d), false);
        sd = sdNone;
    }
