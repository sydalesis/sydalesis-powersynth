// ===================================================================================== MAIN page
// (included inside class DriftfieldUI)

    void buildMain() {
        static const char* const oscTitles[3] = { "OSC A", "OSC B", "OSC C" };
        for (int o = 0; o < 3; ++o) {
            const float px = 10.f + o * 396.f, py = oscY;
            oscX[o] = px;
            panel({ px, py, 390, 410 }, oscTitles[o], pgMain);
            for (int t = 0; t < kOscTypes; ++t) typeTab[o][t] = { px + 58 + t * 41.f, py + 8, 39, 21 };   // 6.0: seven engines
            onBtn[o] = { px + 346, py + 8, 36, 21 };
            engDisp[o] = { px + 12, py + 68, 366, 132 };
            tablePrev[o] = { px + 12, py + 38, 22, 24 }; tableBox[o] = { px + 36, py + 38, 318, 24 }; tableNext[o] = { px + 356, py + 38, 22, 24 };
            smpPrev[o] = { px + 120, py + 38, 22, 24 }; smpBox[o] = { px + 144, py + 38, 210, 24 }; smpNext[o] = { px + 356, py + 38, 22, 24 };
            spcPrev[o] = { px + 12, py + 38, 22, 24 }; spcBox[o] = { px + 36, py + 38, 190, 24 }; spcNext[o] = { px + 228, py + 38, 22, 24 };
            spcX[o] = { px + 256, py + 38, 122, 24 };
            specEdit[o] = { px + 280, py + 38, 30, 24 }; specImport[o] = { px + 312, py + 38, 30, 24 }; freezeBtn[o] = { px + 344, py + 38, 34, 24 };
            // wavetable
            { Ctl& c = selector(oscP(o, ofWarp), { px + 84, py + 214, 116, 20 }, true, pgMain); c.osc = o; c.vis = otWavetable; }
            { Ctl& c = selector(oscP(o, ofWtBlend), { px + 84, py + 246, 116, 20 }, true, pgMain); c.osc = o; c.vis = otWavetable; }
            { Ctl& c = knob(oscP(o, ofPos), px + 40, py + 250, 20, "POSITION", pgMain); c.osc = o; c.vis = otWavetable; }
            { Ctl& c = knob(oscP(o, ofWarpAmt), px + 236, py + 250, 15, "WARP", pgMain); c.osc = o; c.vis = otWavetable; }
            { Ctl& c = knob(oscP(o, ofPhase), px + 290, py + 250, 13, "PHASE", pgMain); c.osc = o; c.vis = 10; }
            { Ctl& c = tog(oscP(o, ofRetrig), { px + 316, py + 226, 46, 18 }, "RTG", pgMain); c.osc = o; c.vis = 10; }
            { Ctl& c = knob(osc2P(o, o2PhaseRnd), px + 340, py + 258, 9, "PH RND", pgMain); c.osc = o; c.vis = 10; }   // 6.0
            // analog
            { Ctl& c = choice(oscP(o, ofAnWave), { px + 12, py + 38, 366, 24 }, kAnWaves, pgMain); c.osc = o; c.vis = otAnalog; }
            { Ctl& c = knob(oscP(o, ofAnPW), px + 40, py + 250, 15, "PW / MIX", pgMain); c.osc = o; c.vis = otAnalog; }
            { Ctl& c = knob(oscP(o, ofAnSync), px + 104, py + 250, 15, "SYNC", pgMain); c.osc = o; c.vis = otAnalog; }
            { Ctl& c = knob(oscP(o, ofAnSlop), px + 168, py + 250, 15, "SLOP", pgMain); c.osc = o; c.vis = otAnalog; }
            { Ctl& c = knob(oscP(o, ofAnDrift), px + 232, py + 250, 15, "DRIFT", pgMain); c.osc = o; c.vis = otAnalog; }
            // additive
            { Ctl& c = selector(oscP(o, ofAdSpec1), { px + 12, py + 38, 130, 24 }, false, pgMain); c.osc = o; c.vis = otAdditive; }
            { Ctl& c = selector(oscP(o, ofAdSpec2), { px + 146, py + 38, 130, 24 }, false, pgMain); c.osc = o; c.vis = otAdditive; }
            {
                const uint32_t f[16] = { ofAdMorph, ofAdPartials, ofAdTilt, ofAdOddEven, ofAdStretch, ofAdPeakPos, ofAdPeakAmt, ofAdPeakWidth,
                                         ofAdComb, ofAdCombShift, ofAdShimmer, ofAdShimRate, ofAdDetune, ofAdVowel, ofAdVowelAmt, ofAdBlur };
                static const char* const l[16] = { "MORPH", "PARTIALS", "TILT", "ODD/EVN", "STRETCH", "PEAK", "PK AMT", "PK WIDTH",
                                                   "COMB", "SHIFT", "SHIMMER", "SHM RATE", "P.DETUNE", "VOWEL", "VOW AMT", "BLUR" };
                for (int i = 0; i < 16; ++i) { Ctl& c = knob(oscP(o, f[i]), px + 30 + (i % 8) * 47.f, py + 220 + (i / 8) * 46.f, 11, l[i], pgMain); c.osc = o; c.vis = otAdditive; }
            }
            // physical
            { Ctl& c = selector(oscP(o, ofPmModel), { px + 12, py + 38, 180, 24 }, false, pgMain); c.osc = o; c.vis = otPhysical; }
            { Ctl& c = selector(oscP(o, ofPmExciter), { px + 198, py + 38, 180, 24 }, false, pgMain); c.osc = o; c.vis = otPhysical; }
            {
                const uint32_t f[8] = { ofPmDecay, ofPmDamp, ofPmHard, ofPmPos, ofPmStretch, ofPmSpread, ofPmCouple, ofPmBuzz };
                static const char* const l[8] = { "DECAY", "DAMPING", "HARDNESS", "POSITION", "STRETCH", "SPREAD", "COUPLING", "BUZZ/SYMP" };
                for (int i = 0; i < 8; ++i) { Ctl& c = knob(oscP(o, f[i]), px + 30 + i * 47.f, py + 220, 11, l[i], pgMain); c.osc = o; c.vis = otPhysical; }
                // 4.2
                const uint32_t f2[3] = { o2Pressure, o2Noise, o2Nonlin };
                static const char* const l2[3] = { "PRESSURE", "NOISE", "NONLINEAR" };
                for (int i = 0; i < 3; ++i) { Ctl& c = knob(osc2P(o, f2[i]), px + 30 + i * 47.f, py + 266, 11, l2[i], pgMain); c.osc = o; c.vis = otPhysical; }
            }
            // 5.0 spectral
            {
                const uint32_t f[8] = { o2SpPos, o2SpSpeed, o2SpBlur, o2SpShift, o2SpStretch, o2SpTilt, o2SpFormant, o2SpSparse };
                static const char* const l[8] = { "POSITION", "SPEED", "BLUR", "SHIFT", "STRETCH", "TILT", "FORMANT", "SPARSE" };
                for (int i = 0; i < 8; ++i) { Ctl& c = knob(osc2P(o, f[i]), px + 30 + i * 47.f, py + 220, 11, l[i], pgMain); c.osc = o; c.vis = otSpectral; }
                { Ctl& c = knob(osc2P(o, o2SpCross), px + 30, py + 266, 11, "CROSS", pgMain); c.osc = o; c.vis = otSpectral; }
                { Ctl& c = selector(osc2P(o, o2SpCrossMode), { px + 60, py + 256, 92, 20 }, true, pgMain); c.osc = o; c.vis = otSpectral; }
                { Ctl& c = tog(osc2P(o, o2SpKeyed), { px + 160, py + 256, 70, 20 }, "KEYED", pgMain); c.osc = o; c.vis = otSpectral; }
                { Ctl& c = tog(osc2P(o, o2SpFreeze), { px + 236, py + 256, 70, 20 }, "FREEZE", pgMain); c.osc = o; c.vis = otSpectral; }
            }
            // granular
            { Ctl& c = selector(oscP(o, ofGrSource), { px + 12, py + 38, 104, 24 }, false, pgMain); c.osc = o; c.vis = otGranular; }
            {
                const uint32_t f[9] = { ofGrPos, ofGrScan, ofGrSize, ofGrDensity, ofGrSpray, ofGrPitchRand, ofGrShape, ofGrSpread, ofGrReverse };
                static const char* const l[9] = { "POSITION", "SCAN", "SIZE", "DENSITY", "SPRAY", "PITCH RND", "SHAPE", "SPREAD", "REVERSE" };
                for (int i = 0; i < 9; ++i) { Ctl& c = knob(oscP(o, f[i]), px + 28 + i * 41.5f, py + 250, 13, l[i], pgMain); c.osc = o; c.vis = otGranular; }
            }
            // 6.0 FM: algorithm + DX7 bank, diagram, operator tabs, operator rows, oscillator-wide row
            {
                { Ctl& c = selector(fmP(o, fmAlgo), { px + 12, py + 38, 96, 24 }, true, pgMain); c.osc = o; c.vis = otFm; }
                fmSyxBtn[o] = { px + 112, py + 38, 38, 24 };
                fmVoicePrev[o] = { px + 154, py + 38, 22, 24 }; fmVoiceBox[o] = { px + 178, py + 38, 176, 24 }; fmVoiceNext[o] = { px + 356, py + 38, 22, 24 };
                fmDiag[o] = { px + 12, py + 68, 214, 124 }; fmEnvView[o] = { px + 230, py + 68, 148, 124 };
                for (int k = 0; k < 6; ++k) fmOpTab[o][k] = { px + 12 + k * 40.f, py + 198, 38, 18 };
                for (int k = 0; k < 6; ++k) {
                    const uint32_t f1[9] = { foCoarse, foFine, foDetune, foHz, foLevel, foAttack, foDecay, foSustain, foRelease };
                    static const char* const l1[9] = { "RATIO", "FINE", "DETUNE", "FIX HZ", "LEVEL", "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
                    for (int i = 0; i < 9; ++i) { Ctl& c = knob(fmOpP(o, k, f1[i]), px + 28 + i * 41.5f, py + 234, 11, l1[i], pgMain); c.osc = o; c.vis = otFm; c.uiKey = ukFmOpA + o; c.uiVal = k; }
                    { Ctl& c = tog(fmOpP(o, k, foFixed), { px + 254, py + 198, 40, 18 }, "FIX", pgMain); c.osc = o; c.vis = otFm; c.uiKey = ukFmOpA + o; c.uiVal = k; }
                    { Ctl& c = selector(fmOpP(o, k, foWave), { px + 298, py + 198, 80, 18 }, true, pgMain); c.osc = o; c.vis = otFm; c.uiKey = ukFmOpA + o; c.uiVal = k; }
                    { Ctl& c = knob(fmOpP(o, k, foVel), px + 28, py + 278, 11, "VEL", pgMain); c.osc = o; c.vis = otFm; c.uiKey = ukFmOpA + o; c.uiVal = k; }
                    { Ctl& c = knob(fmOpP(o, k, foKey), px + 69.5f, py + 278, 11, "KEY", pgMain); c.osc = o; c.vis = otFm; c.uiKey = ukFmOpA + o; c.uiVal = k; }
                }
                const uint32_t g[7] = { fmIndex, fmFeedback, fmBright, fmSpread, fmEnvTime, fmVelIdx, fmKeyIdx };
                static const char* const gl[7] = { "INDEX", "FEEDBACK", "BRIGHT", "SPREAD", "ENV TIME", "VEL>IDX", "KEY>IDX" };
                for (int i = 0; i < 7; ++i) { Ctl& c = knob(fmP(o, g[i]), px + 28 + (i + 2) * 41.5f, py + 278, 11, gl[i], pgMain); c.osc = o; c.vis = otFm; }
            }
            // FM / sync row
            { Ctl& c = selector(oscP(o, ofFmSrc), { px + 36, py + 306, 72, 20 }, false, pgMain); c.osc = o; }
            { Ctl& c = selector(oscP(o, ofFmMode), { px + 112, py + 306, 58, 20 }, false, pgMain); c.osc = o; }
            { Ctl& c = slider(oscP(o, ofFmAmt), { px + 176, py + 306, 112, 20 }, nullptr, pgMain); c.osc = o; }
            { Ctl& c = selector(oscP(o, ofSyncSrc), { px + 320, py + 306, 58, 20 }, false, pgMain); c.osc = o; c.vis = 10; }
            // 6.0: Granular / Spectral level compensation (one switch per layer, shown on both engines)
            { Ctl& c = tog(pLevelComp, { px + 296, py + 306, 82, 20 }, "AUTO LEVEL", pgMain); c.osc = o; c.vis = otGranular; }
            { Ctl& c = tog(pLevelComp, { px + 296, py + 306, 82, 20 }, "AUTO LEVEL", pgMain); c.osc = o; c.vis = otSpectral; }
            // common row
            const uint32_t cf[8] = { ofLevel, ofPan, ofOct, ofSemi, ofFine, ofVoices, ofDetune, ofUniWidth };
            static const char* const cl[8] = { "LEVEL", "PAN", "OCT", "SEMI", "FINE", "VOICES", "DETUNE", "WIDTH" };
            for (int i = 0; i < 8; ++i) { Ctl& c = knob(oscP(o, cf[i]), px + 30 + i * 47.f, py + 366, 13, cl[i], pgMain); c.osc = o; c.vis = i >= 5 ? 11 : -1; }
        }
        // mixer / voice / MPE
        {
            const float px = 1198, py = oscY;
            panel({ px, py, 392, 410 }, "MIXER  ·  VOICE  ·  EXPRESSION", pgMain);
            buildMixer(px, py);
            selector(pPolyMode, { px + 14, py + 234, 92, 22 }, true, pgMain);
            // 5.1.6: PITCH (layer transpose, a modulation target: drop a source on it for pitch modulation)
            const uint32_t m2[6] = { pGlide, pDrift, pSpread, pPolyphony, pPitch, pMaster };
            static const char* const l2[6] = { "GLIDE", "DRIFT", "SPREAD", "POLY", "PITCH", "MASTER" };
            for (int i = 0; i < 6; ++i) knob(m2[i], px + 134 + i * 46.f, py + 246, i == 5 ? 15 : 13, l2[i], pgMain);
            const uint32_t m3[7] = { pBendRange, pPressCut, pPressAmp, pSlideCut, pSlideTimbre, pVelAmp, pMWCut };
            static const char* const l3[7] = { "BEND", "PRS>CUT", "PRS>AMP", "SLD>CUT", "SLD>TMB", "VEL>AMP", "MW>CUT" };
            for (int i = 0; i < 7; ++i) knob(m3[i], px + 32 + i * 54.f, py + 312, 12, l3[i], pgMain);
            voiceRect = { px + 12, py + 344, 368, 58 };
        }
        // filters
        for (int f = 0; f < 2; ++f) {
            const float px = f == 0 ? 10.f : 520.f, py = 478;
            panel({ px, py, 440, 252 }, f == 0 ? "FILTER 1" : "FILTER 2", pgMain);
            selector(filtP(f, ffType), { px + 80, py + 8, 236, 22 }, true, pgMain);
            selector(filtP(f, ffDriveType), { px + 322, py + 8, 106, 22 }, true, pgMain);
            knob(filtP(f, ffCutoff), px + 44, py + 70, 24, "CUTOFF", pgMain);
            const int ks[7] = { ffRes, ffDrive, ffKey, ffEnv, ffShape, ffMorph, ffFmAmt };
            static const char* const kl[7] = { "RESO", "DRIVE", "KEY", "ENV", "SHAPE", "MORPH", "FM" };
            for (int i = 0; i < 7; ++i) knob(filtP(f, ks[i]), px + 108 + i * 50.f, py + 60, 14, kl[i], pgMain);
            selector(filtP(f, ffType2), { px + 140, py + 100, 186, 20 }, true, pgMain);
            selector(filtP(f, ffFmSrc), { px + 356, py + 100, 72, 20 }, false, pgMain);
            filtDisp[f] = { px + 12, py + 128, 416, 114 };
        }
        {
            const float px = 456, py = 478;
            panel({ px, py, 58, 252 }, "", pgMain);
            for (int i = 0; i < 3; ++i) routeBtn[i] = { px + 5, py + 34 + i * 30.f, 48, 24 };
            knob(pFBalance, px + 29, py + 160, 15, "BAL", pgMain);
        }
        static const char* const envT[3] = { "FILTER ENV", "AMP ENV", "MOD ENV" };
        const uint32_t envB[3] = { pFEnvA, pAEnvA, pMEnvA };
        for (int e = 0; e < 3; ++e) {
            const float px = 966.f + e * 210.f, py = 478;
            panel({ px, py, 204, 252 }, envT[e], pgMain);
            envHandle[e] = { px + 178, py + 6, 18, 18 };
            envDisp[e] = { px + 10, py + 30, 184, 104 };
            static const char* const l[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
            for (int k = 0; k < 4; ++k) knob(envB[e] + k, px + 30 + k * 48.f, py + 162, 14, l[k], pgMain);
            slider(envCurveParam(e), { px + 12, py + 200, 140, 10 }, "A CURVE", pgMain);
            slider(envDecCurveParam(e), { px + 12, py + 217, 140, 10 }, "D CURVE", pgMain);   // 6.0
            slider(envRelCurveParam(e), { px + 12, py + 234, 140, 10 }, "R CURVE", pgMain);   // 5.1.3
            envClassicBtn[e] = { px + 160, py + 196, 34, 18 };
        }
        // 6.0: the 8 macros in two rows on the left; the LFO strip is gone (the MODULATOR panel edits every LFO)
        panel({ 10, 736, 260, 256 }, "MACROS", pgMain);
        for (int m = 0; m < kMacros; ++m) {
            const float x = 45.f + (m % 4) * 62.f, y = 736.f + 78.f + (m / 4) * 112.f;
            knob(pMacro1 + m, x, y, 18, nullptr, pgMain);
            macroHandle[m] = { x + 14, y - 34, 14, 14 };
            macroLabel[m] = { x - 30, y + 30, 60, 16 };
        }
        buildModPanel();
        panel({ 1062, 736, 528, 256 }, "OUTPUT", pgMain);
        scopeRect = { 1074, 764, 504, 176 };
        buildOutput();
    }

    // ------------------------------------------------------------------ main page drawing
    void drawMain() {
        for (int o = 0; o < 3; ++o) drawOscPanel(o);
        drawMixerExtras();
        for (int f = 0; f < 2; ++f) drawFilterPanel(f);
        drawRouting();
        drawEnvelope(0, envDisp[0], pFEnvA, cViolet);
        drawEnvelope(1, envDisp[1], pAEnvA, cAccent);
        drawEnvelope(2, envDisp[2], pMEnvA, cWarm);
        const int envSrc[3] = { msFiltEnv, msAmpEnv, msModEnv };
        const Color envCol[3] = { cViolet, cAccent, cWarm };
        for (int e = 0; e < 3; ++e) drawHandle(envHandle[e], envCol[e], envSrc[e]);
        for (int e = 0; e < 3; ++e) button(envClassicBtn[e], "3.0", envClassic(), hovered(envClassicBtn[e]), 8.5f, &cWarm);
        drawMacros(pgMain);
        drawModPanel();
        drawScope();
    }

    void drawOscPanel(int o) {
        static const char* const tabNames[kOscTypes] = { "WAVE", "ANALOG", "ADD", "PHYS", "GRAIN", "SPEC", "FM" };
        const int type = oscType(o);
        const float px = oscX[o], py = oscY;
        for (int t = 0; t < kOscTypes; ++t) button(typeTab[o][t], tabNames[t], t == type, hovered(typeTab[o][t]), 8.2f);
        {
            const bool on = values[oscP(o, ofOn)] >= 0.5f, hot = hovered(onBtn[o]);
            const Rect& ob = onBtn[o];
            beginPath(); roundedRect(ob.x, ob.y, ob.w, ob.h, ob.h * 0.5f);
            fillColor(on ? tint(cAccent, 0.23f) : (hot ? shade(38.f) : cInset)); fill();
            strokeColor(on ? tint(cAccent, 0.62f) : cPanelEdge); stroke();
            beginPath(); circle(ob.cx(), ob.cy(), 4.f); fillColor(on ? cAccent : cFaint); fill();
        }
        const Rect& d = engDisp[o];
        if (type == otWavetable) {
            drawTableBox(o, tablePrev[o], tableBox[o], tableNext[o]);
            save(); scissor(d.x, d.y, d.w, d.h); drawWavetable(o, d); restore();   // 5.1.1: nothing paints outside the display
        } else if (type == otAnalog) {
            save(); scissor(d.x, d.y, d.w, d.h); drawAnalogWave(o, d); restore();   // 5.1.1: nothing paints outside the display
        } else if (type == otAdditive) {
            button(specEdit[o], "✎", false, hovered(specEdit[o]), 11.f);
            button(specImport[o], "⇩", false, hovered(specImport[o]), 11.f);
            button(freezeBtn[o], "FRZ", values[oscP(o, ofAdFreeze)] >= 0.5f, hovered(freezeBtn[o]), 8.5f, &cBlue);
            save(); scissor(d.x, d.y, d.w, d.h); drawSpectrum(o, d); restore();   // 5.1.1: nothing paints outside the display
        } else if (type == otPhysical) {
            save(); scissor(d.x, d.y, d.w, d.h); drawModes(o, d); restore();   // 5.1.1: nothing paints outside the display
            static const char* const hint[kPmModels][2] = {
                { "modal string · POSITION = strike point", "NONLINEAR = energy blooms upward" },
                { "modal bar", "NONLINEAR = energy blooms upward" },
                { "modal bell", "NONLINEAR = energy blooms upward" },
                { "modal membrane", "NONLINEAR = energy blooms upward" },
                { "modal plate", "NONLINEAR = energy blooms upward" },
                { "modal glass bowl", "NONLINEAR = energy blooms upward" },
                { "modal tube (odd harmonics)", "NONLINEAR = energy blooms upward" },
                { "delay-line string · POSITION = pickup", "NONLINEAR = tension: louder is sharper" },
                { "BUZZ = jawari bridge", "NONLINEAR = tension: louder is sharper" },
                { "BUZZ = sympathetic strings", "NONLINEAR = tension: louder is sharper" },
                { "PRESSURE blows, chokes near the top", "POSITION opening · HARDNESS stiffness" },
                { "PRESSURE blows · POSITION embouchure", "DAMPING = bore loss · NOISE = breath" },
                { "dense inharmonic modes (48 in HQ)", "NONLINEAR = the swell into the highs" },
                { "modes measured from a WAV", "LAYERS → ANALYSE · DECAY scales them" } };
            const int mdl = std::max(0, std::min(kPmModels - 1, ival(oscP(o, ofPmModel))));
            label(px + 168, py + 260, hint[mdl][0], 7.6f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
            label(px + 168, py + 274, hint[mdl][1], 7.6f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            const int ex = ival(oscP(o, ofPmExciter));
            if (ex == 8) label(px + 168, py + 288, stateValue[skSmA + o].empty() ? "Sample: load one on the GRANULAR tab" : "Sample: the osc's grain sample, looped", 7.6f, stateValue[skSmA + o].empty() ? cWarn : cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        } else if (type == otSpectral) {
            drawSpecBoxes(o);
            save(); scissor(d.x, d.y, d.w, d.h); drawSpectral(o, d); restore();   // 5.1.1: nothing paints outside the display
        } else if (type == otFm) {
            drawFmPanel(o);
        } else {
            if (ival(oscP(o, ofGrSource)) == 0) drawTableBox(o, smpPrev[o], smpBox[o], smpNext[o]);
            else drawSampleBox(o);
            save(); scissor(d.x, d.y, d.w, d.h); drawGranular(o, d); restore();   // 5.1.1: nothing paints outside the display
        }
        label(px + 14, py + 316, "FM", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        if (type <= otAnalog) label(px + 296, py + 316, "SYNC", 8.5f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        // 6.0: what FM does on the engines without a single cycle (Additive, Physical, Spectral: on the output)
        if ((type == otAdditive || type == otPhysical || type == otSpectral) && ival(oscP(o, ofFmSrc)) > 0 && values[oscP(o, ofFmAmt)] > 0.f) {
            const int md = ival(oscP(o, ofFmMode));
            const bool pitch = md <= 1 || md >= 4;
            const bool on = !pitch || (ival(pFmV6) & (1 << o));
            char b[96];
            if (on) std::snprintf(b, sizeof b, "%s %.0f%% from %s%s", kFmModeNames[std::max(0, std::min(5, md))], values[oscP(o, ofFmAmt)] * 100.f,
                                  kFmSrcNames[std::max(0, std::min(5, ival(oscP(o, ofFmSrc))))], pitch ? "  (bends the whole output)" : "");
            else std::snprintf(b, sizeof b, "PM / FM silent in this 5.x preset: touch the FM controls to turn it on");
            label(d.x + 8, d.y + d.h - 9, b, 7.6f, on ? cViolet : cWarm, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        char hint[128];
        std::snprintf(hint, sizeof(hint), "TIMBRE  1 %s · 2 %s · 3 %s · 4 %s", kTimbreNames[type][0], kTimbreNames[type][1], kTimbreNames[type][2], kTimbreNames[type][3]);
        label(px + 14, py + 340, hint, 7.4f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        if (type == otWavetable) { label(px + 46 + 38, py + 208, "WARP", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE); label(px + 84, py + 240, "FRAMES", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE); }
    }

    void drawTableBox(int o, const Rect& prev, const Rect& b, const Rect& next) {
        drawButtonArrow(prev, true, hovered(prev));
        drawButtonArrow(next, false, hovered(next));
        beginPath(); roundedRect(b.x, b.y, b.w, b.h, 4); fillColor(hovered(b) ? shade(31.f) : cInset); fill();
        strokeColor(cPanelEdge); strokeWidth(1); stroke();
        save(); scissor(b.x + 4, b.y, b.w - 8, b.h);
        labelS(b.cx(), b.cy() + 1, tableName(o), 11, fileMissing[o] ? cWarn : (stateValue[skWtA + o].empty() ? cText : cAccent), ALIGN_CENTER | ALIGN_MIDDLE);
        restore();
    }
    void drawSampleBox(int o) {
        drawButtonArrow(smpPrev[o], true, hovered(smpPrev[o]));
        drawButtonArrow(smpNext[o], false, hovered(smpNext[o]));
        const Rect& b = smpBox[o];
        beginPath(); roundedRect(b.x, b.y, b.w, b.h, 4); fillColor(hovered(b) ? shade(31.f) : cInset); fill(); strokeColor(cPanelEdge); stroke();
        save(); scissor(b.x + 4, b.y, b.w - 8, b.h);
        const std::string nm = smpName[o].empty() ? std::string("choose a sample…") : smpName[o] + (smpMissing[o] ? "  (missing)" : "");
        labelS(b.cx(), b.cy() + 1, nm, 11, smpMissing[o] ? cWarn : (smpName[o].empty() ? cFaint : cAccent), ALIGN_CENTER | ALIGN_MIDDLE);
        restore();
    }

    // ---- 5.0 spectral display ----------------------------------------------------------------------
    void drawSpecBoxes(int o) {
        drawButtonArrow(spcPrev[o], true, hovered(spcPrev[o]));
        drawButtonArrow(spcNext[o], false, hovered(spcNext[o]));
        auto box = [&](const Rect& b, const SpecView& sv, const char* empty, const char* prefix) {
            beginPath(); roundedRect(b.x, b.y, b.w, b.h, 4); fillColor(hovered(b) ? shade(31.f) : cInset); fill(); strokeColor(cPanelEdge); stroke();
            save(); scissor(b.x + 4, b.y, b.w - 8, b.h);
            const std::string nm = sv.name.empty() ? std::string(empty) : std::string(prefix) + sv.name + (sv.missing ? "  (missing)" : "");
            labelS(b.cx(), b.cy() + 1, nm, 10.5f, sv.missing ? cWarn : (sv.name.empty() ? cFaint : cAccent), ALIGN_CENTER | ALIGN_MIDDLE);
            restore();
        };
        box(spcBox[o], specView[o], "choose a recording…", "");
        box(spcX[o], specXView[o], "cross partner…", "× ");
    }
    void drawSpectral(int o, const Rect& r) {
        inset(r);
        const SpecView& sv = specView[o];
        if (sv.cols <= 0) { label(r.cx(), r.cy(), sv.missing ? "the file is missing" : "Load any recording: voice, field recording, instrument, loop", 9, sv.missing ? cWarn : cFaint, ALIGN_CENTER | ALIGN_MIDDLE); return; }
        const float x0 = r.x + 6, y0 = r.y + 16, w = r.w - 12, h = r.h - 22;
        const float cw = w / sv.cols, rh = h / sv.rows;
        for (int c = 0; c < sv.cols; ++c)
            for (int k = 0; k < sv.rows; ++k) {
                const float v = sv.img[(size_t)c * sv.rows + k];
                if (v < 0.08f) continue;
                beginPath(); rect(x0 + c * cw, y0 + h - (k + 1) * rh, cw + 0.4f, rh + 0.4f);
                fillColor(withA(mixc(cBlue, cAccent, v), 0.12f + 0.8f * v * v)); fill();
            }
        // position knob and live playheads
        const float pos = values[osc2P(o, o2SpPos)];
        beginPath(); moveTo(x0 + w * pos, r.y + 4); lineTo(x0 + w * pos, r.y + r.h - 4); strokeColor(withA(cWarm, 0.7f)); strokeWidth(1.2f); stroke();
        if (shared && newestVoice() && values[oscP(o, ofOn)] >= 0.5f) {
            const float x = x0 + w * std::max(0.f, std::min(1.f, tel.specPos[o]));
            beginPath(); moveTo(x, r.y + 2); lineTo(x, r.y + r.h - 2); strokeColor(cText); strokeWidth(1.6f); stroke();
        }
        const bool frozen = values[osc2P(o, o2SpFreeze)] >= 0.5f;
        label(r.x + 8, r.y + 9, frozen ? "FROZEN" : "drag to set the start · white = playhead", 7.5f, frozen ? cBlue : cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        char b[96];
        std::snprintf(b, sizeof b, "%.1f s · %s %.1f Hz", sv.secs, sv.pitched ? "root" : "keyed from C4 =", sv.pitched ? sv.root : 261.6f);
        if (!specXView[o].name.empty() && ival(osc2P(o, o2SpCrossMode)) > 0) std::snprintf(b, sizeof b, "%s", ival(osc2P(o, o2SpCrossMode)) == 1 ? "morphing with the partner" : "vocoding with the partner");
        label(r.x + r.w - 8, r.y + 9, b, 7.5f, specXView[o].name.empty() || ival(osc2P(o, o2SpCrossMode)) == 0 ? cFaint : cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
    }

    // live helpers: modulated value for osc o, timbre k of the newest voice (falls back to the knob)
    float liveT(int o, int k, float fallback) const { const drift::TelVoice* v = newestVoice(); return v && values[oscP(o, ofOn)] >= 0.5f ? v->t[o][k] : fallback; }

    void drawWavetable(int o, const Rect& r) {
        const drift::WaveTable& t = tableFor(o);
        const bool hot = hovered(r) || (dragParam == (int)oscP(o, ofPos));
        inset(r, hot);
        if (t.frames <= 0) return;
        const float pos = values[oscP(o, ofPos)];
        const float lpos = std::max(0.f, std::min(1.f, liveT(o, 0, pos)));
        save(); scissor(r.x, r.y, r.w, r.h);
        const int layers = std::min(t.frames, 22);
        const float depthX = 56.f, depthY = 34.f;
        const float baseY = r.y + r.h * 0.62f, amp = r.h * 0.25f;
        const float x0 = r.x + 10.f, w = r.w - 20.f - depthX;
        const int pts = 110;
        const int warp = ival(oscP(o, ofWarp));
        const float wa = liveT(o, 1, values[oscP(o, ofWarpAmt)]);
        auto drawFrame = [&](float fidx, float dd, const Color& c, float width, bool warped) {
            int f0 = (int)fidx; if (f0 > t.frames - 2) f0 = std::max(0, t.frames - 2);
            const float fr = t.frames > 1 ? fidx - f0 : 0.f;
            const float* a = t.table(f0, 0);
            const float* bb = t.table(t.frames > 1 ? f0 + 1 : f0, 0);
            beginPath();
            for (int i = 0; i <= pts; ++i) {
                float ph = (float)i / pts;
                float sgn = 1.f;
                if (warped && warp) { if (warp == 9 && ph >= 1.f - 0.5f * wa) sgn = -1.f; ph = warpUi(warp, ph, wa); }
                int si = (int)(ph * drift::kWtSize); if (si >= drift::kWtSize) si = drift::kWtSize - 1; if (si < 0) si = 0;
                const float y = (a[si] + (bb[si] - a[si]) * fr) * sgn;
                const float xx = x0 + dd * depthX + w * (float)i / pts;
                const float yy = baseY - dd * depthY - y * amp;
                if (i == 0) moveTo(xx, yy); else lineTo(xx, yy);
            }
            strokeColor(c); strokeWidth(width); stroke();
        };
        for (int l = layers - 1; l >= 0; --l) {
            const float dd = layers > 1 ? (float)l / (layers - 1) : 0.f;
            drawFrame(dd * (t.frames - 1), dd, withA(mixc(cDim, cBlue, 0.3f), 0.10f + 0.22f * (1.f - dd)), 1.f, false);
        }
        // other voices: faint markers
        forVoices([&](const drift::TelVoice& v, bool newest) {
            if (newest) return;
            const float p2 = std::max(0.f, std::min(1.f, v.t[o][0]));
            drawFrame(p2 * (t.frames - 1), p2, withA(cViolet, 0.25f + 0.4f * v.amp), 1.f, true);
        });
        const float dpos = lpos * (t.frames - 1);
        drawFrame(dpos, lpos, withA(cAccent, 0.25f), 4.f, true);
        drawFrame(dpos, lpos, cAccent, 1.6f, true);
        drawModPreview(o, { x0 + lpos * depthX, r.y, w, r.h }, 1, baseY - lpos * depthY, amp);   // 6.0: on the front frame
        restore();
        const float bx = r.x + 10, bw = r.w - 20, by = r.y + r.h - 7;
        beginPath(); roundedRect(bx, by, bw, 3, 1.5f); fillColor(shade(36.f)); fill();
        beginPath(); roundedRect(bx, by, bw * pos, 3, 1.5f); fillColor(withA(cAccent, hot ? 0.9f : 0.55f)); fill();
        beginPath(); circle(bx + bw * pos, by + 1.5f, hot ? 4.5f : 3.5f); fillColor(cAccent); fill();
        forVoices([&](const drift::TelVoice& v, bool) { beginPath(); circle(bx + bw * std::max(0.f, std::min(1.f, v.t[o][0])), by + 1.5f, 2.5f); fillColor(cViolet); fill(); });
        char buf[48]; std::snprintf(buf, sizeof(buf), "%d / %d%s", (int)std::round(dpos) + 1, t.frames, ival(oscP(o, ofWtBlend)) == 1 ? "  spectral" : "");
        label(r.x + r.w - 8, r.y + 11, buf, 8.5f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }
    static float warpUi(int mode, float p, float a) {
        switch (mode) {
        case 1: { const float k = 8.f * a; return (1.f + k) * p / (1.f + k * p); }
        case 2: { const float k = -0.9f * a; return (1.f + k) * p / (1.f + k * p); }
        case 3: { const float k = 8.f * a; if (p < 0.5f) { const float q = 2.f * p; return 0.5f * (1.f + k) * q / (1.f + k * q); }
                  const float q = 2.f - 2.f * p; return 1.f - 0.5f * (1.f + k) * q / (1.f + k * q); }
        case 4: { const float w = 1.f - 0.95f * a; return p < w ? p / w : 0.f; }
        case 5: { const float q = p * (1.f + 7.f * a); return q - std::floor(q); }
        case 6: { const float m = p < 0.5f ? 2.f * p : 2.f - 2.f * p; return p + (m - p) * a; }
        case 7: { const float k = 0.5f - 0.49f * a; return p < k ? 0.5f * p / k : 0.5f + 0.5f * (p - k) / (1.f - k); }
        case 8: { const float steps = 2.f + (1.f - a) * (1.f - a) * 254.f; return std::floor(p * steps) / steps; }
        default: return p;
        }
    }

    // ---- 6.0 visual feedback ------------------------------------------------------------------------------
    // one cycle of an oscillator as the editor sees it (Analog: the naive shape; Wavetable: the frame at the
    // position, warped); other engines: a sine (their PM / FM preview shows the index, not their timbre)
    float oscCycleUi(int o, float q) {
        q -= std::floor(q);
        const int t = oscType(o);
        if (t == otAnalog) {
            const float pw = values[oscP(o, ofAnPW)];
            switch (ival(oscP(o, ofAnWave))) {
            case 0: case 5: return 2.f * q - 1.f;
            case 1: return q < pw ? 1.f : -1.f;
            case 2: return 0.5f * ((2.f * q - 1.f) + (q < pw ? 1.f : -1.f));
            case 3: return q < 0.5f ? 4.f * q - 1.f : 3.f - 4.f * q;
            default: return std::sin(2.f * kPi * q);
            }
        }
        if (t == otWavetable) {
            const drift::WaveTable& tb = tableFor(o);
            if (tb.frames <= 0) return 0.f;
            const int warp = ival(oscP(o, ofWarp)); const float wa = values[oscP(o, ofWarpAmt)];
            float sg = 1.f;
            if (warp) { if (warp == 9 && q >= 1.f - 0.5f * wa) sg = -1.f; q = warpUi(warp, q, wa); }
            const int f = std::max(0, std::min(tb.frames - 1, (int)std::round(values[oscP(o, ofPos)] * (tb.frames - 1))));
            int si = (int)(q * drift::kWtSize); si = std::max(0, std::min(drift::kWtSize - 1, si));
            return tb.table(f, 0)[si] * sg;
        }
        return std::sin(2.f * kPi * q);
    }
    float oscSemisUi(int o) const { return values[oscP(o, ofOct)] * 12.f + values[oscP(o, ofSemi)] + values[oscP(o, ofFine)] * 0.01f; }
    // FM / PM / AM / Ring drawn on the waveform display (violet: the carrier as the modulator bends it), and the
    // retrigger start phase. x spans r.x .. r.x + r.w for `cycles` cycles around the centre line cy.
    void drawModPreview(int o, const Rect& r, int cycles, float cy, float amp) {
        if (values[oscP(o, ofRetrig)] >= 0.5f) {
            const float ph = values[oscP(o, ofPhase)];
            for (int c = 0; c < cycles; ++c) {
                const float x = r.x + r.w * (c + ph) / cycles;
                beginPath(); moveTo(x, cy - amp * 1.15f); lineTo(x, cy + amp * 1.15f); strokeColor(withA(cWarm, 0.55f)); strokeWidth(1); stroke();
                beginPath(); moveTo(x - 4, cy + amp * 1.15f + 6); lineTo(x + 4, cy + amp * 1.15f + 6); lineTo(x, cy + amp * 1.15f); closePath(); fillColor(withA(cWarm, 0.8f)); fill();
            }
        }
        const int src = ival(oscP(o, ofFmSrc));
        const float a = values[oscP(o, ofFmAmt)];
        if (src <= 0 || a <= 0.f) return;
        const int mode = ival(oscP(o, ofFmMode));
        const float hard = mode >= 4 ? 4.f : 1.f;
        const float ratio = src <= 3 ? std::pow(2.f, (oscSemisUi(src - 1) - oscSemisUi(o)) / 12.f) : (src == 4 ? 0.5f : 1.f);
        auto M = [&](float q) {
            if (src <= 3) return oscCycleUi(src - 1, q);
            if (src == 4) return std::sin(2.f * kPi * q);
            const uint32_t h = (uint32_t)(q * 97.f) * 2654435761u; return (float)(h >> 8) / 8388608.f - 1.f;   // noise: steps
        };
        const int pts = 320;
        float phase = 0.f;
        beginPath();
        for (int i = 0; i <= pts; ++i) {
            const float x = (float)cycles * i / pts;
            const float m = M(x * ratio);
            float y;
            if (mode == 0 || mode == 4) y = oscCycleUi(o, x + a * a * 2.f * hard * m);
            else if (mode == 1 || mode == 5) { y = oscCycleUi(o, phase); phase += (float)cycles / pts * (1.f + a * a * 8.f * hard * m); }
            else if (mode == 2) y = oscCycleUi(o, x) * (1.f + a * m * 1.5f) * 0.6f;
            else y = oscCycleUi(o, x) * (1.f - a + a * m * 2.f);
            y = std::max(-1.4f, std::min(1.4f, y));
            const float xx = r.x + r.w * (float)i / pts, yy = cy - y * amp;
            if (i == 0) moveTo(xx, yy); else lineTo(xx, yy);
        }
        strokeColor(withA(cViolet, 0.85f)); strokeWidth(1.2f); stroke();
        char b[64]; std::snprintf(b, sizeof b, "%s %.0f%% from %s  x%.2f", kFmModeNames[std::max(0, std::min(5, mode))], a * 100.f,
                                  src <= 3 ? (src == 1 ? "A" : src == 2 ? "B" : "C") : (src == 4 ? "Sub" : "Noise"), ratio);
        label(r.x + 2, cy + amp * 1.15f + 16 > r.y + r.h - 6 ? r.y + r.h - 8 : cy + amp * 1.15f + 16, b, 7.6f, cViolet, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void drawAnalogWave(int o, const Rect& r) {
        inset(r);
        const int wave = ival(oscP(o, ofAnWave));
        const float pw = liveT(o, 0, values[oscP(o, ofAnPW)]), sync = liveT(o, 1, values[oscP(o, ofAnSync)]);
        const float ratio = std::pow(2.f, sync / 12.f);
        auto naive = [&](float q) {
            switch (wave) {
            case 0: case 5: return 2.f * q - 1.f;
            case 1: return q < pw ? 1.f : -1.f;
            case 2: return 0.5f * ((2.f * q - 1.f) + (q < pw ? 1.f : -1.f));
            case 3: return q < 0.5f ? 4.f * q - 1.f : 3.f - 4.f * q;
            default: return std::sin(2.f * kPi * q);
            }
        };
        save(); scissor(r.x, r.y, r.w, r.h);
        // unison / supersaw: draw the detuned copies drifting apart
        const int nu = wave == 5 ? 7 : std::max(1, ival(oscP(o, ofVoices)));
        const float det = values[oscP(o, ofDetune)] * (wave == 5 ? 1.2f : 1.f);
        const float tsec = (float)(nowMs() % 100000) * 0.001f;
        for (int j = nu - 1; j >= 0; --j) {
            const float posn = nu == 1 ? 0.f : 2.f * j / (nu - 1) - 1.f;
            const float drift = std::fmod(posn * det * 0.0004f * tsec * (liveVoices ? 20.f : 2.f) + 10.f, 1.f);
            const bool centre = j == nu / 2 || nu == 1;
            beginPath();
            const int pts = 280;
            for (int i = 0; i <= pts; ++i) {
                const float x = (float)i / pts * 2.f + (centre ? 0.f : drift);
                const float m = x - std::floor(x);
                const float q = std::fmod(m * ratio, 1.f);
                const float y = naive(q);
                const float xx = r.x + 12 + (r.w - 24) * (float)i / pts, yy = r.y + r.h * 0.5f - y * r.h * 0.33f;
                if (i == 0) moveTo(xx, yy); else lineTo(xx, yy);
            }
            if (centre) { strokeColor(withA(cAccent, 0.25f)); strokeWidth(4.f); stroke(); strokeColor(cAccent); strokeWidth(1.6f); stroke(); }
            else { strokeColor(withA(cViolet, 0.28f)); strokeWidth(1.f); stroke(); }
        }
        drawModPreview(o, { r.x + 12, r.y, r.w - 24, r.h }, 2, r.y + r.h * 0.5f, r.h * 0.33f);   // 6.0
        restore();
        char buf[64]; std::snprintf(buf, sizeof(buf), sync > 0.05f ? "PW %.0f%%   SYNC +%.1f st" : "PW %.0f%%", pw * 100.f, sync);
        label(r.x + r.w - 8, r.y + 11, buf, 8.5f, newestVoice() ? cViolet : cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }

    // additive: base (spec1 -> spec2 morph) and shaped result, computed like the engine
    void additiveSpectra(int o, float* base, float* shaped) const {
        const auto& T = drift::SpectrumTable::get();
        const int s1 = ival(oscP(o, ofAdSpec1)), s2 = ival(oscP(o, ofAdSpec2));
        auto specOf = [&](int s) -> const float* { return s == kSpecCustom ? customSpec[o] : s == kSpecCustom2 ? customSpec2[o] : T.a[std::min(s, kSpecCustom - 1)]; };
        const float* A = specOf(s1);
        const float* B = specOf(s2);
        const float morph = values[oscP(o, ofAdMorph)];
        const int n = ival(oscP(o, ofAdPartials));
        const float tilt = values[oscP(o, ofAdTilt)] * 1.5f, oe = values[oscP(o, ofAdOddEven)];
        const float oddW = oe > 0.f ? 1.f - oe : 1.f, evenW = oe < 0.f ? 1.f + oe : 1.f;
        const float peakPos = values[oscP(o, ofAdPeakPos)], peakAmt = values[oscP(o, ofAdPeakAmt)] * 8.f;
        const float peakW = (0.4f + 6.f * values[oscP(o, ofAdPeakWidth)]) * (1.f + peakPos * 0.08f);
        const float comb = values[oscP(o, ofAdComb)], cshift = values[oscP(o, ofAdCombShift)];
        float mx = 1e-9f;
        for (int i = 0; i < drift::kPartials; ++i) {
            const float h = (float)(i + 1);
            base[i] = A[i] + (B[i] - A[i]) * morph;
            float a = i < n ? base[i] : 0.f;
            if (tilt != 0.f) a *= std::pow(h, -tilt);
            if (i > 0) a *= (i & 1) ? evenW : oddW;
            if (peakAmt > 0.f) { const float d = (h - peakPos) / peakW; a *= 1.f + peakAmt * std::exp(-0.5f * d * d); }
            if (comb > 0.f) a *= 1.f - comb * (0.5f + 0.5f * std::cos(2.f * kPi * (h / 6.f + cshift)));
            shaped[i] = a; mx = std::max(mx, a);
        }
        for (int i = 0; i < drift::kPartials; ++i) shaped[i] /= mx;
    }
    void drawSpectrum(int o, const Rect& r) {
        const int tgt = addEditTarget(o);
        const bool editable = tgt > 0;
        inset(r, hovered(r) || (sd == sdAddDraw && sdIndex == o));
        float base[drift::kPartials], shaped[drift::kPartials];
        additiveSpectra(o, base, shaped);
        if (editable) { const float* c = tgt == 2 ? customSpec2[o] : customSpec[o]; for (int i = 0; i < drift::kPartials; ++i) base[i] = c[i]; }   // 6.0: bars = the drawing being edited
        const int nShow = std::max(16, std::min(drift::kPartials, ival(oscP(o, ofAdPartials)) <= 64 ? 64 : 128));
        const float bw = (r.w - 16) / (float)nShow;
        const float top = r.y + 18, bot = r.y + r.h - 6, hgt = bot - top;
        for (int i = 0; i < nShow; ++i) {
            const float x = r.x + 8 + i * bw;
            const float hb = std::max(0.f, std::min(1.f, base[i])) * hgt;
            beginPath(); rect(x + 0.3f, bot - hb, std::max(0.8f, bw - 1.f), hb);
            fillColor(withA(cViolet, editable ? 0.5f : 0.25f)); fill();
        }
        // live partials of the newest voice (dB scale), or the static shaped result
        const drift::TelVoice* nv = newestVoice();
        const bool live = nv && values[oscP(o, ofOn)] >= 0.5f;
        float mxl = 1e-9f;
        if (live) for (int i = 0; i < nShow; ++i) mxl = std::max(mxl, tel.partials[o][i]);
        beginPath();
        for (int i = 0; i < nShow; ++i) {
            const float x = r.x + 8 + (i + 0.5f) * bw;
            const float a = live ? tel.partials[o][i] / mxl : shaped[i];
            const float db = 20.f * std::log10(std::max(1e-4f, a));
            const float y = bot - std::max(0.f, 1.f + db / 60.f) * hgt;
            if (i == 0) moveTo(x, y); else lineTo(x, y);
        }
        strokeColor(live ? cAccent : withA(cAccent, 0.7f)); strokeWidth(live ? 1.6f : 1.2f); stroke();
        if (live) for (int i = 0; i < nShow; ++i) {
            const float a = tel.partials[o][i] / mxl;
            if (a < 0.02f) continue;
            const float x = r.x + 8 + (i + 0.5f) * bw, db = 20.f * std::log10(a), y = bot - std::max(0.f, 1.f + db / 60.f) * hgt;
            beginPath(); circle(x, y, 1.6f); fillColor(cAccent); fill();
        }
        label(r.x + 8, r.y + 10, editable ? (tgt == 2 ? "DRAW TO EDIT CUSTOM 2" : "DRAW TO EDIT CUSTOM") : "DRAG TO DRAW (copies to Custom)", 7.8f, editable && tgt == 2 ? cWarm : cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(r.x + r.w - 8, r.y + 10, live ? "live: newest voice" : "bars base · line result", 7.8f, live ? cViolet : cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }

    void drawModes(int o, const Rect& r) {
        inset(r);
        const int model = ival(oscP(o, ofPmModel));
        const drift::TelVoice* nv = newestVoice();
        const bool live = nv && tel.modeCount[o] > 0 && values[oscP(o, ofOn)] >= 0.5f;
        auto xOf = [&](float ratio) { return r.x + 8 + (r.w - 16) * std::log2(std::max(0.5f, ratio) * 2.f) / std::log2(64.f); };
        if (pmIsWaveguide(model)) {
            // waveguide family: show the string with a standing-wave shimmer driven by the newest voice
            const float tsec = (float)(nowMs() % 100000) * 0.001f;
            const float amp = nv ? std::min(1.f, nv->amp * 1.2f) : 0.f;
            save(); scissor(r.x, r.y, r.w, r.h);
            for (int k = 1; k <= 6; ++k) {
                beginPath();
                for (int i = 0; i <= 120; ++i) {
                    const float x = (float)i / 120.f;
                    const float y = std::sin(kPi * k * x) * std::sin(tsec * 6.f * k + k) * amp / k;
                    const float xx = r.x + 14 + (r.w - 28) * x, yy = r.cy() + y * r.h * 0.38f;
                    if (i == 0) moveTo(xx, yy); else lineTo(xx, yy);
                }
                strokeColor(withA(cWarm, 0.15f + 0.6f / k)); strokeWidth(k == 1 ? 1.6f : 1.f); stroke();
            }
            restore();
            const float pos = values[oscP(o, ofPmPos)];
            beginPath(); moveTo(r.x + 14 + (r.w - 28) * (0.25f + 0.5f * pos), r.y + 16); lineTo(r.x + 14 + (r.w - 28) * (0.25f + 0.5f * pos), r.y + r.h - 8);
            strokeColor(withA(cBlue, 0.5f)); strokeWidth(1); stroke();
            label(r.x + r.w - 8, r.y + 11, model == pmTanpura ? "TANPURA · jawari bridge" : (model == pmSympathetic ? "SYMPATHETIC · +4 resonant strings"
                  : (model == pmReed ? "REED · closed bore, odd harmonics" : (model == pmFlute ? "FLUTE · jet and open bore" : "WAVEGUIDE"))), 8, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
            return;
        }
        if (live) {
            float mx = 1e-9f;
            for (int i = 0; i < tel.modeCount[o]; ++i) mx = std::max(mx, tel.modes[o][i]);
            for (int i = 0; i < tel.modeCount[o]; ++i) {
                const float x = xOf(tel.modeRatio[o][i]);
                if (x > r.x + r.w - 6) continue;
                const float e = std::sqrt(tel.modes[o][i] / mx);
                const float hg = e * (r.h - 22);
                beginPath(); moveTo(x, r.y + r.h - 6); lineTo(x, r.y + r.h - 6 - hg);
                strokeColor(withA(cWarm, 0.35f + 0.65f * e)); strokeWidth(2.f); stroke();
                if (e > 0.25f) { beginPath(); circle(x, r.y + r.h - 6 - hg, 2.f + 2.f * e); fillColor(withA(cWarm, 0.4f * e)); fill(); }
            }
        } else {
            float ratios[48]; int n = 0;
            switch (model) {
            case pmGong: for (int i = 0; i < 48; ++i) ratios[i] = 1.f + 0.62f * std::pow((float)i, 1.12f) + (i ? 0.07f * std::sin((float)i * 2.39f) : 0.f); n = 48; break;
            case pmAnalysed: n = std::min(48, modalD[o].count); for (int i = 0; i < n; ++i) ratios[i] = modalD[o].ratio[i]; break;
            case 0: for (int i = 0; i < 24; ++i) ratios[i] = (float)(i + 1); n = 24; break;
            case 1: for (int i = 0; i < 16; ++i) { const float k = (2.f * (i + 1) + 1.f) / 3.f; ratios[i] = k * k; } n = 16; break;
            case 2: { const float b[16] = { 0.5f, 1.f, 1.183f, 1.506f, 2.f, 2.514f, 2.662f, 3.011f, 4.166f, 5.433f, 6.796f, 8.215f, 9.7f, 11.2f, 12.8f, 14.5f }; std::memcpy(ratios, b, sizeof(b)); n = 16; break; }
            case 3: { const float m[24] = { 1.f, 1.593f, 2.136f, 2.296f, 2.653f, 2.918f, 3.156f, 3.501f, 3.6f, 3.652f, 4.06f, 4.154f, 4.601f, 4.832f, 4.903f, 5.131f, 5.412f, 5.553f, 5.65f, 5.931f, 6.f, 6.209f, 6.536f, 6.749f }; std::memcpy(ratios, m, sizeof(m)); n = 24; break; }
            case 4: { const float pl[20] = { 1.f, 2.5f, 2.52f, 4.f, 5.f, 5.03f, 6.5f, 6.53f, 8.5f, 8.52f, 9.f, 10.f, 12.5f, 12.53f, 13.f, 14.5f, 14.53f, 16.f, 17.f, 18.f }; std::memcpy(ratios, pl, sizeof(pl)); n = 20; break; }
            case 5: { const float g[10] = { 1.f, 2.32f, 4.25f, 6.63f, 9.38f, 12.5f, 16.f, 19.9f, 24.2f, 28.9f }; std::memcpy(ratios, g, sizeof(g)); n = 10; break; }
            default: for (int i = 0; i < 16; ++i) ratios[i] = (float)(2 * i + 1); n = 16; break;
            }
            const float stretch = 1.f + 0.1f * values[oscP(o, ofPmStretch)];
            const float posn = values[oscP(o, ofPmPos)], damp = values[oscP(o, ofPmDamp)], cpl = values[oscP(o, ofPmCouple)];
            for (int i = 0; i < n; ++i) {
                const float rr = std::pow(ratios[i], stretch);
                float a = model == pmAnalysed ? modalD[o].amp[i]
                        : ((model == 0 || model == 6) ? std::fabs(std::sin(kPi * (i + 1) * posn)) / std::sqrt((float)(i + 1))
                                                      : (0.35f + 0.65f * std::fabs(std::sin(kPi * (i + 1) * posn * 1.37f + 0.3f))) / std::pow((float)(i + 1), 0.6f));
                const float decay = 1.f / (1.f + damp * 2.5f * std::pow(std::max(0.f, std::log2(rr)), 2.f));
                const float hg = std::min(1.f, a) * (r.h - 22);
                for (int c = 0; c < (cpl > 0.001f ? 2 : 1); ++c) {
                    const float x = xOf(rr * (cpl > 0.001f ? (c ? 1.f + cpl * 0.012f : 1.f - cpl * 0.012f) : 1.f)) + (cpl > 0.001f ? (c ? 1.5f : -1.5f) : 0.f);
                    if (x > r.x + r.w - 6) continue;
                    beginPath(); moveTo(x, r.y + r.h - 6); lineTo(x, r.y + r.h - 6 - hg);
                    strokeColor(withA(cWarm, 0.25f + 0.55f * decay)); strokeWidth(cpl > 0.001f ? 1.3f : 2.f); stroke();
                }
            }
        }
        label(r.x + r.w - 8, r.y + 11, live ? "live resonator energy" : "mode ratios · brightness = sustain", 8, live ? cViolet : cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }

    void drawGranular(int o, const Rect& r) {
        inset(r);
        const int src = ival(oscP(o, ofGrSource));
        const float top = r.y + 18, bot = r.y + r.h - 8, mid = (top + bot) * 0.5f, hh = (bot - top) * 0.5f;
        if (src != 0 && !smpOverview[o].empty()) {
            const int cols = (int)smpOverview[o].size() / 2;
            beginPath();
            for (int c = 0; c < cols; ++c) {
                const float x = r.x + 8 + (r.w - 16) * (float)c / cols;
                moveTo(x, mid - smpOverview[o][2 * c + 1] * hh); lineTo(x, mid - smpOverview[o][2 * c] * hh);
            }
            strokeColor(withA(mixc(cDim, cBlue, 0.3f), 0.45f)); strokeWidth(1); stroke();
        } else {
            // wavetable source: frames laid out left to right
            const drift::WaveTable& t = tableFor(o);
            const int nf = std::min(t.frames, 32);
            for (int f = 0; f < nf; ++f) {
                const int fr = nf > 1 ? f * (t.frames - 1) / (nf - 1) : 0;
                const float* a = t.table(fr, 0);
                const float x0 = r.x + 8 + (r.w - 16) * (float)f / nf, w = (r.w - 16) / nf;
                beginPath();
                for (int i = 0; i <= 16; ++i) { const float y = a[i * (drift::kWtSize / 16) % drift::kWtSize]; const float xx = x0 + w * i / 16.f, yy = mid - y * hh * 0.9f; if (i == 0) moveTo(xx, yy); else lineTo(xx, yy); }
                strokeColor(withA(mixc(cDim, cBlue, 0.3f), 0.4f)); strokeWidth(1); stroke();
            }
        }
        // position / spray window
        const float pos = liveT(o, 0, values[oscP(o, ofGrPos)]), spray = liveT(o, 3, values[oscP(o, ofGrSpray)]);
        const float xw = r.x + 8 + (r.w - 16) * pos;
        {   // 5.1.1: the spray window wraps around the ends of the sample (as the grains do) instead of spilling out
            const float W = r.w - 16, x0 = r.x + 8, sw = std::min(W, W * std::max(0.f, spray));
            float a = xw - sw * 0.5f - x0;            // window start relative to the sample
            a -= W * std::floor(a / W);
            const float first = std::min(sw, W - a);
            beginPath(); rect(x0 + a, top, first, bot - top); fillColor(withA(cAccent, 0.08f)); fill();
            if (sw > first) { beginPath(); rect(x0, top, sw - first, bot - top); fillColor(withA(cAccent, 0.08f)); fill(); }
        }
        beginPath(); moveTo(xw, top); lineTo(xw, bot); strokeColor(withA(cAccent, 0.7f)); strokeWidth(1.2f); stroke();
        // live grains
        if (newestVoice() && values[oscP(o, ofOn)] >= 0.5f)
            for (int g = 0; g < drift::kTelGrains; ++g) {
                const float a = tel.grainAmp[o][g];
                if (a <= 0.001f) continue;
                const float x = r.x + 8 + (r.w - 16) * std::max(0.f, std::min(1.f, tel.grainPos[o][g]));
                const float y = mid + std::sin((float)g * 2.39996f) * hh * 0.7f;
                beginPath(); circle(x, y, 2.f + 4.f * a); fillColor(withA(cViolet, 0.25f + 0.6f * a)); fill();
            }
        char buf[64]; std::snprintf(buf, sizeof(buf), "%.0f ms · %.0f /s", values[oscP(o, ofGrSize)] * 1000.f, values[oscP(o, ofGrDensity)]);
        label(r.x + r.w - 8, r.y + 11, buf, 8, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
        label(r.x + 8, r.y + 11, src == 0 ? "source: wavetable frames" : (smpName[o].empty() ? "no sample loaded: using wavetable" : "source: sample"), 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }

    // ------------------------------------------------------------------ mixer extras: voices
    void drawMixerExtras() {
        const float px = 1198, py = oscY;
        drawMixer(px, py);
        beginPath(); moveTo(px + 14, py + 214); lineTo(px + 378, py + 214); moveTo(px + 14, py + 280); lineTo(px + 378, py + 280);
        strokeColor(cGrid); strokeWidth(1); stroke();
        label(px + 14, py + 224, "VOICE", 8.2f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(px + 14, py + 290, "MPE  ·  EXPRESSION  (ch 2–16)", 8.2f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        // voice activity
        const Rect& r = voiceRect;
        inset(r);
        const int nv = drift::kTelVoices;
        const float cw = (r.w - 8) / nv;
        for (int i = 0; i < nv; ++i) {
            const drift::TelVoice& v = tel.v[i];
            const float x = r.x + 4 + i * cw;
            const bool on = shared && v.active;
            const float a = on ? std::min(1.f, v.amp) : 0.f;
            beginPath(); rect(x + 1, r.y + r.h - 18 - a * (r.h - 34), cw - 2, a * (r.h - 34)); fillColor(on && v.gate ? withA(cAccent, 0.35f + 0.6f * a) : withA(cViolet, 0.25f + 0.5f * a)); fill();
            if (on) {
                static const char* const nn[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                char b[8]; std::snprintf(b, sizeof(b), "%s%d", nn[((v.note % 12) + 12) % 12], v.note / 12 - 1);
                label(x + cw * 0.5f, r.y + r.h - 8, b, 6.5f, i == tel.newest ? cAccent : cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            }
        }
        label(r.x + 6, r.y + 9, "VOICES", 7.5f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }

    // ------------------------------------------------------------------ filters
    struct RespCache { int type = -1; float res = -1, shape = -1, fcKey = -1; float db[200]; };
    RespCache respCache[2][2];
    static constexpr int kRespN = 200;
    static float respFreq(int i) { return 5.f * std::pow(24000.f / 5.f, (float)i / (kRespN - 1)); }
    // magnitude response at fc = 1 kHz (or at the real fc for formant/vowel), 200 log points 5 Hz..24 kHz
    const float* responseFor(int f, int which, int type, float res, float shape, float fc) {
        RespCache& c = respCache[f][which];
        const bool fcDep = type == ftFormant || type == ftVowel;
        const float key = fcDep ? std::round(std::log2(fc) * 6.f) : 0.f;
        if (c.type == type && std::fabs(c.res - res) < 0.01f && std::fabs(c.shape - shape) < 0.01f && c.fcKey == key) return c.db;
        c.type = type; c.res = res; c.shape = shape; c.fcKey = key;
        const float fs = 48000.f, f0 = fcDep ? std::pow(2.f, key / 6.f) : 1000.f;
        static MultiFilter mf;
        mf.reset();
        FilterCoefs co;
        co.g = std::tan(kPi * f0 / fs);
        co.shape = shape;
        filterControlCoefs(type, f0, res, shape, fs, co);
        const int N = 8192;
        std::vector<drift::cpx> buf(N, drift::cpx(0.f, 0.f));
        const float amp = 0.01f;
        for (int i = 0; i < 4096; ++i) buf[i] = drift::cpx(mf.process(type, i == 0 ? amp : 0.f, res, co) / amp, 0.f);
        drift::fft::forward(buf);
        for (int i = 0; i < kRespN; ++i) {
            const float fr = respFreq(i) * (fcDep ? 1.f : 1.f);
            const float bin = fr / fs * N;
            const int b0 = std::min(N / 2 - 2, (int)bin);
            const float t = bin - (float)b0;
            const float m0 = std::abs(buf[b0]) * N, m1 = std::abs(buf[b0 + 1]) * N;
            c.db[i] = 20.f * std::log10(std::max(1e-6f, m0 + (m1 - m0) * t));
        }
        return c.db;
    }
    float respAt(const float* db, float freq, float fc, bool fcDep) const {
        const float f = fcDep ? freq : freq * 1000.f / std::max(10.f, fc);
        const float x = std::log(f / 5.f) / std::log(24000.f / 5.f) * (kRespN - 1);
        if (x <= 0.f) return db[0];
        if (x >= kRespN - 1) return db[kRespN - 1];
        const int i = (int)x; const float t = x - (float)i;
        return db[i] + (db[i + 1] - db[i]) * t;
    }
    void drawFilterPanel(int f) {
        const float px = f == 0 ? 10.f : 520.f, py = 478;
        label(px + 80, py + 110, "MORPH →", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        label(px + 338, py + 110, "FM", 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        const Rect& r = filtDisp[f];
        inset(r);
        const int ta = ival(filtP(f, ffType)), tb = ival(filtP(f, ffType2));
        const float fcSet = values[filtP(f, ffCutoff)], res = values[filtP(f, ffRes)], shape = values[filtP(f, ffShape)], morph = values[filtP(f, ffMorph)];
        save(); scissor(r.x, r.y, r.w, r.h);
        for (float fr : { 100.f, 1000.f, 10000.f }) {
            const float x = r.x + r.w * std::log10(fr / 20.f) / std::log10(1000.f);
            beginPath(); moveTo(x, r.y + 4); lineTo(x, r.y + r.h - 4); strokeColor(cGrid); strokeWidth(1); stroke();
        }
        beginPath(); moveTo(r.x, r.y + r.h * 0.4f); lineTo(r.x + r.w, r.y + r.h * 0.4f); strokeColor(cGrid); stroke();
        const Color col = f == 0 ? cAccent : cBlue;
        auto curve = [&](float fc, float rs, float sh, float mo, float* ys) {
            const float* da = responseFor(f, 0, ta, rs, sh, fc);
            const bool morphOn = mo > 0.001f && tb != ta;
            const float* dbb = morphOn ? responseFor(f, 1, tb, rs, sh, fc) : nullptr;
            for (int i = 0; i < 96; ++i) {
                const float freq = 20.f * std::pow(1000.f, (float)i / 95.f);
                float db = respAt(da, freq, fc, ta == ftFormant || ta == ftVowel);
                if (ta == ftOff) db = 0.f;
                if (morphOn) {
                    float dbB = tb == ftOff ? 0.f : respAt(dbb, freq, fc, tb == ftFormant || tb == ftVowel);
                    const float ma = std::pow(10.f, db / 20.f), mb = std::pow(10.f, dbB / 20.f);
                    db = 20.f * std::log10(std::max(1e-6f, ma + (mb - ma) * mo));
                }
                ys[i] = std::min(r.y + r.h + 2.f, std::max(r.y + 2.f, r.y + r.h * 0.4f - db * (r.h / 66.f)));
            }
        };
        float ys[96];
        const drift::TelVoice* nv = newestVoice();
        const bool live = nv && (ta != ftOff || morph > 0.f);
        // static curve (knob position)
        curve(fcSet, res, shape, morph, ys);
        beginPath(); moveTo(r.x, ys[0]);
        for (int i = 1; i < 96; ++i) lineTo(r.x + r.w * i / 95.f, ys[i]);
        strokeColor(live ? Color(col.red, col.green, col.blue, 0.3f) : col); strokeWidth(live ? 1.f : 1.6f); stroke();
        if (!live) {
            lineTo(r.x + r.w, r.y + r.h + 2); lineTo(r.x, r.y + r.h + 2); closePath();
            fillPaint(linearGradient(0, r.y, 0, r.y + r.h, Color(col.red, col.green, col.blue, 0.16f), Color(col.red, col.green, col.blue, 0.f))); fill();
        } else {
            // other voices: cutoff ticks
            forVoices([&](const drift::TelVoice& v, bool) {
                const float fc = f == 0 ? v.cut1 : v.cut2;
                const float x = r.x + r.w * std::log10(std::max(20.f, fc) / 20.f) / std::log10(1000.f);
                beginPath(); moveTo(x, r.y + r.h - 10); lineTo(x, r.y + r.h - 2); strokeColor(withA(cViolet, 0.4f + 0.5f * v.amp)); strokeWidth(1.5f); stroke();
            });
            const float fc = f == 0 ? nv->cut1 : nv->cut2;
            curve(fc, f == 0 ? nv->res1 : nv->res2, f == 0 ? nv->shape1 : nv->shape2, f == 0 ? nv->morph1 : nv->morph2, ys);
            beginPath(); moveTo(r.x, r.y + r.h + 2);
            for (int i = 0; i < 96; ++i) lineTo(r.x + r.w * i / 95.f, ys[i]);
            lineTo(r.x + r.w, r.y + r.h + 2); closePath();
            fillPaint(linearGradient(0, r.y, 0, r.y + r.h, Color(col.red, col.green, col.blue, 0.2f), Color(col.red, col.green, col.blue, 0.f))); fill();
            beginPath(); moveTo(r.x, ys[0]);
            for (int i = 1; i < 96; ++i) lineTo(r.x + r.w * i / 95.f, ys[i]);
            strokeColor(col); strokeWidth(1.7f); stroke();
            char b[32]; std::snprintf(b, sizeof(b), fc >= 1000.f ? "%.2f kHz" : "%.0f Hz", fc >= 1000.f ? fc / 1000.f : fc);
            label(r.x + r.w - 8, r.y + 11, b, 8.5f, cViolet, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        restore();
    }
    // 6.0: SHAPE only acts on some filter types (Morph, dual band, comb, phaser, formant, vowel, Flanges); on the
    // others it is dimmed. The morph target type counts while TYPE MORPH is above 0.
    void dimUnusedFilterShapes() {
        for (int f = 0; f < 2; ++f) {
            const int t = ival(filtP(f, ffType)), t2 = ival(filtP(f, ffType2));
            const bool used = drift::filterUsesShape(t) || (values[filtP(f, ffMorph)] > 0.f && drift::filterUsesShape(t2));
            if (used || t == 0) continue;
            for (const Ctl& c : ctls) if (c.page == pgMain && c.kind == ckKnob && c.p == filtP(f, ffShape)) {
                const Rect r = ctlRect(c);
                dimRect({ r.x - r.w - 7, r.y - r.w - 7, 2 * r.w + 14, 2 * r.w + 26 });
            }
        }
    }
    void drawRouting() {
        static const char* const l[3] = { "SER", "PAR", "SPLIT" };
        label(456 + 29, 478 + 18, "ROUTE", 8.2f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
        const int cur = ival(pFRouting);
        for (int i = 0; i < 3; ++i) button(routeBtn[i], l[i], i == cur, hovered(routeBtn[i]), 8.8f);
        const char* hint = cur == 0 ? "F1→F2" : (cur == 1 ? "F1‖F2" : "A│BC");
        label(456 + 29, 478 + 232, hint, 8, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
    }

    // ------------------------------------------------------------------ envelopes
    void envGeometry(uint32_t pa, const Rect& r, float* xs) const {
        auto seg = [](float t) { return 0.12f + std::log10(1.f + t * 20.f) / std::log10(601.f); };
        const float A = seg(values[pa]), D = seg(values[pa + 1]), S = 0.35f, R = seg(values[pa + 3]);
        const float total = A + D + S + R;
        const float x0 = r.x + 8, w = r.w - 16;
        xs[0] = x0; xs[1] = x0 + w * A / total; xs[2] = xs[1] + w * D / total; xs[3] = xs[2] + w * S / total; xs[4] = xs[3] + w * R / total;
    }
    static uint32_t envCurveParam(int e) { return e == 0 ? pFEnvCurve : (e == 1 ? pAEnvCurve : pMEnvCurve); }
    static uint32_t envRelCurveParam(int e) { return e == 0 ? pFRelCurve : (e == 1 ? pARelCurve : pMRelCurve); }
    static uint32_t envDecCurveParam(int e) { return e == 0 ? pFDecCurve : (e == 1 ? pADecCurve : pMDecCurve); }
    void drawEnvelope(int e, const Rect& r, uint32_t pa, const Color& col) {
        inset(r);
        float xs[5]; envGeometry(pa, r, xs);
        const float top = r.y + 16, bot = r.y + r.h - 8, h = bot - top;
        const float sus = values[pa + 2];
        beginPath();
        moveTo(xs[0], bot);
        const int n = 24;
        const float kc = values[envCurveParam(e)];
        const bool cl = envClassic();
        for (int i = 1; i <= n; ++i) {
            const float u = (float)i / n; const float s = u * u * (3.f - 2.f * u);
            const float sh = cl ? std::min(1.f, 1.25f * (1.f - std::exp(-1.609f * u))) : (kc >= 0.f ? s + (1.f - (1.f - u) * (1.f - u) * (1.f - u) - s) * kc : s + (u * u * u - s) * (-kc));
            lineTo(xs[0] + (xs[1] - xs[0]) * u, bot - h * sh);
        }
        const float dk = values[envDecCurveParam(e)];   // 6.0: decay curve, drawn like the engine (60 dB over the decay)
        for (int i = 1; i <= n; ++i) {
            const float u = (float)i / n;
            float g;
            if (dk == 0.f) g = std::exp(-4.f * u);
            else { const float hh = dk > 0.f ? 1.f - std::pow(1.f - u, 1.f + 3.f * dk) : std::pow(u, 1.f - 3.f * dk); g = (std::exp(-5.f * hh) - std::exp(-5.f)) / (1.f - std::exp(-5.f)); }
            lineTo(xs[1] + (xs[2] - xs[1]) * u, bot - h * (sus + (1.f - sus) * g));
        }
        lineTo(xs[3], bot - h * sus);
        const float rk = values[envRelCurveParam(e)];
        for (int i = 1; i <= n; ++i) {
            const float u = (float)i / n;
            const float hh = rk > 0.f ? 1.f - std::pow(1.f - u, 1.f + 3.f * rk) : std::pow(u, 1.f - 3.f * rk);   // engine's releaseShape
            const float ee = (std::exp(-5.f * hh) - std::exp(-5.f)) / (1.f - std::exp(-5.f));
            lineTo(xs[3] + (xs[4] - xs[3]) * u, bot - h * sus * ee);
        }
        strokeColor(col); strokeWidth(1.6f); stroke();
        lineTo(xs[0], bot); closePath();
        fillPaint(linearGradient(0, top, 0, bot, Color(col.red, col.green, col.blue, 0.2f), Color(col.red, col.green, col.blue, 0.f))); fill();
        // live levels: one dot per voice on the right edge, a level line for the newest
        forVoices([&](const drift::TelVoice& v, bool newest) {
            const float lv = e == 0 ? v.envF : (e == 1 ? v.amp : v.envM);
            const float y = bot - h * std::min(1.f, lv);
            if (newest) { beginPath(); moveTo(r.x + 6, y); lineTo(r.x + r.w - 6, y); strokeColor(Color(col.red, col.green, col.blue, 0.35f)); strokeWidth(1); stroke(); }
            beginPath(); circle(r.x + r.w - 8, y, newest ? 3.5f : 2.2f); fillColor(newest ? col : Color(col.red, col.green, col.blue, 0.5f)); fill();
        });
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.1fs %.1fs %.0f%% %.1fs", values[pa], values[pa + 1], sus * 100.f, values[pa + 3]);
        label(r.x + 6, r.y + 9, buf, 7.6f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }

    // ------------------------------------------------------------------ macros, lfo strip
    void drawMacros(int pg) {
        for (int m = 0; m < kMacros; ++m) {
            const Rect& h = pg == pgMain ? macroHandle[m] : modMacroHandle[m];
            drawHandle(h, cWarm, msMacro1 + m);
            if (pg == pgMod) labelS(h.x - 18, h.y + 58, macroName[m], 8.5f, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            if (pg == pgMain) {
                const Rect& lr = macroLabel[m];
                const bool editing = textTarget == ttMacro && textIndex == m;
                if (editing) { beginPath(); roundedRect(lr.x, lr.y, lr.w, lr.h, 3); fillColor(cInset); fill(); strokeColor(cAccent); stroke(); }
                save(); scissor(lr.x, lr.y, lr.w, lr.h);
                labelS(lr.cx(), lr.cy() + 0.5f, editing ? textBuf + "_" : macroName[m], 8.5f, editing ? cText : (hovered(lr) ? cAccent : cDim), ALIGN_CENTER | ALIGN_MIDDLE);
                restore();
            }
        }
    }
    float lfoShapeValue(int i, float ph) const {
        const int wave = ival(lfoP(i, lfWave));
        ph -= std::floor(ph);
        switch (wave) {
        case 0: return std::sin(2.f * kPi * ph);
        case 1: return ph < 0.5f ? 4.f * ph - 1.f : 3.f - 4.f * ph;
        case 2: return 2.f * ph - 1.f;
        case 3: return 1.f - 2.f * ph;
        case 4: return ph < 0.5f ? 1.f : -1.f;
        case 5: { static const float sh[8] = { 0.6f, -0.3f, 0.9f, -0.8f, 0.1f, -0.5f, 0.75f, -0.15f }; return sh[(int)(ph * 8.f) & 7]; }
        case 6: { static const float sh[9] = { 0.6f, -0.3f, 0.9f, -0.8f, 0.1f, -0.5f, 0.75f, -0.15f, 0.6f }; const float x = ph * 8.f; const int k = (int)x; const float t = x - k; const float s = t * t * (3.f - 2.f * t); return sh[k] + (sh[k + 1] - sh[k]) * s; }
        default: { const float x = ph * drift::kLfoShapePoints; const int k = (int)x; const float t = x - k; return lfoPts[i][k & 63] + (lfoPts[i][(k + 1) & 63] - lfoPts[i][k & 63]) * t; }
        }
    }
    void drawLfoWave(int i, const Rect& r, float lineW, bool dots) {
        const Color& c = cLfoCol[i];
        beginPath();
        const int n = (int)std::max(32.f, r.w);
        for (int k = 0; k <= n; ++k) {
            const float ph = (float)k / n;
            const float y = lfoShapeValue(i, ph);
            const float x = r.x + 4 + (r.w - 8) * ph, yy = r.cy() - y * (r.h * 0.4f);
            if (k == 0) moveTo(x, yy); else lineTo(x, yy);
        }
        strokeColor(c); strokeWidth(lineW); stroke();
        if (!dots || !shared) return;
        const bool global = ival(lfoP(i, lfMode)) == 0;
        if (global) {
            const float ph = tel.lfoGPh[i];
            beginPath(); circle(r.x + 4 + (r.w - 8) * ph, r.cy() - tel.lfoG[i] * (r.h * 0.4f), 3.5f); fillColor(cText); fill();
        } else {
            forVoices([&](const drift::TelVoice& v, bool newest) {
                beginPath(); circle(r.x + 4 + (r.w - 8) * v.lfoPh[i], r.cy() - v.lfo[i] * (r.h * 0.4f), newest ? 3.5f : 2.2f);
                fillColor(newest ? cText : Color(c.red, c.green, c.blue, 0.7f)); fill();
            });
        }
    }
    int lfoRouteCount(int i) const {
        int uses = 0;
        for (int s = 0; s < kModSlots; ++s) if ((ival(modSrcParam(s)) == msLfo1 + i || ival(modViaParam(s)) == msLfo1 + i) && ival(modDstParam(s)) > 0 && values[modAmtParam(s)] != 0.f) ++uses;
        return uses;
    }
    // ------------------------------------------------------------------ 5.1.6 modulator panel
    // Replaces the read-only "active modulation" list. Shows the modulator picked in its tabs (or the last
    // ◎ handle pressed on any page): its own controls, and every target it drives with a depth bar, a
    // polarity switch and a remove button. Tabs and the header ◎ drag onto any knob to add a target.
    static constexpr int kMainModSrc[kMainModTabs] = { msLfo1, msLfo2, msLfo3, msLfo4, msLfo5, msLfo6,
                                                       msMseg1, msMseg2, msMseg3, msMseg4, msRndGlobal, msRndVoice };
    static int msegOfSource(int src) { return src == msMseg1 ? 0 : src == msMseg2 ? 1 : src == msMseg3 ? 2 : src == msMseg4 ? 3 : -1; }
    int modSel() const { return uiSel[ukMainMod]; }
    void selectMod(int src) {
        if (src <= msNone || src >= kModSrcCount) return;
        uiSel[ukMainMod] = src;
        const int m = msegOfSource(src);
        if (m >= 0) uiSel[ukMsegA + (m & 1)] = m >> 1;     // the MOD page shows the same MSEG
        repaint();
    }
    Color modSrcColor(int src) const {
        if (src >= msLfo1 && src <= msLfo6) return cLfoCol[src - msLfo1];
        if (msegOfSource(src) >= 0) return cBlue;
        if (src == msRndGlobal || src == msRndVoice) return cRose;
        if (src >= msMacro1 && src <= msMacro8) return cWarm;
        if (src == msFiltEnv) return cViolet;
        if (src == msAmpEnv) return cAccent;
        if (src == msModEnv) return cWarm;
        return cText;
    }
    std::string modSrcName(int src) const {
        if (src >= msMacro1 && src <= msMacro8) return macroName[src - msMacro1];
        return kModSrcNames[std::max(0, std::min((int)kModSrcCount - 1, src))];
    }
    // 6.0: 780 wide (the LFO strip's space): a 470-wide editor column and the targets list
    void buildModPanel() {
        const float px = 276, py = 736;
        panel({ px, py, 780, 256 }, "MODULATOR", pgMain);
        mainModHandle = { px + 754, py + 6, 18, 18 };
        for (int t = 0; t < kMainModTabs; ++t) mainModTab[t] = { px + 10 + t * 40.f, py + 30, 38, 18 };
        const float ex = px + 10, ey = py + 54;
        mainLfoEdit = { ex, ey, 470, 104 };
        mainMsegEdit = { ex, ey, 470, 122 };
        mainRndEdit = { ex, ey, 470, 96 };
        mainModTargets = { px + 494, py + 54, 276, 196 };
        if (uiSel[ukMainMod] <= msNone) uiSel[ukMainMod] = msLfo1;
        for (int i = 0; i < kLfos; ++i) {
            auto g = [&](Ctl& c) -> Ctl& { c.uiKey = ukMainMod; c.uiVal = msLfo1 + i; return c; };
            const float ry = ey + 110;
            g(selector(lfoP(i, lfWave), { ex, ry, 120, 18 }, true, pgMain));
            g(selector(lfoP(i, lfMode), { ex + 124, ry, 120, 18 }, true, pgMain));
            g(tog(lfoP(i, lfSync), { ex + 250, ry, 52, 18 }, "SYNC", pgMain));
            { Ctl& c = g(selector(lfoP(i, lfDiv), { ex + 306, ry, 84, 18 }, false, pgMain)); c.visParam = (int)lfoP(i, lfSync); c.visVal = 1; }
            const float ky = ey + 160;
            { Ctl& c = g(knob(lfoP(i, lfRate), ex + 30, ky, 12, "RATE", pgMain)); c.visParam = (int)lfoP(i, lfSync); c.visVal = 0; }
            g(knob(lfoP(i, lfFade), ex + 100, ky, 12, "FADE", pgMain));
            g(knob(lfoP(i, lfPhase), ex + 170, ky, 12, "PHASE", pgMain));
            g(knob(pLRndPh1 + i, ex + 240, ky, 12, "RND PHASE", pgMain));
            g(knob(lfoP(i, lfSmooth), ex + 310, ky, 12, "SMOOTH", pgMain));
        }
        for (int m = 0; m < kMsegs; ++m) {
            auto g = [&](Ctl& c) -> Ctl& { c.uiKey = ukMainMod; c.uiVal = msegSource(m); return c; };
            const float ry = ey + 128;
            g(selector(msegP(m, 0), { ex, ry, 112, 18 }, true, pgMain));
            g(tog(msegP(m, 2), { ex + 116, ry, 104, 18 }, "SYNC (beats)", pgMain));
            g(knob(msegP(m, 1), ex + 446, ry + 14, 13, "TIME ×", pgMain));
        }
        // MSEG grid / snap and copy / paste (editor options, not sound parameters)
        msegGridX = { ex + 30, ey + 150, 76, 18 };
        msegGridY = { ex + 140, ey + 150, 64, 18 };
        msegCopyBtn = { ex + 224, ey + 128, 48, 18 };
        msegPasteBtn = { ex + 276, ey + 128, 52, 18 };
        lfoCopyBtn = { ex + 398, ey + 110, 34, 18 };
        lfoPasteBtn = { ex + 436, ey + 110, 34, 18 };
        for (int gi = 0; gi < 2; ++gi) {
            Ctl& c = knob(gi == 0 ? pRndGRate : pRndVRate, ex + 36, ey + 140, 15, "RATE", pgMain);
            c.uiKey = ukMainMod; c.uiVal = gi == 0 ? msRndGlobal : msRndVoice;
        }
    }
    // slots driven by the selected source, in slot order
    void modPanelSlots(std::vector<int>& out) const {
        out.clear();
        const int src = modSel();
        for (int s = 0; s < kModSlots; ++s) if (ival(modSrcParam(s)) == src && ival(modDstParam(s)) != mdNone) out.push_back(s);
    }
    static constexpr float kTgtRowH = 19.f;
    struct TgtRow { Rect bar, pol, del; };
    TgtRow tgtRow(int k) const {
        const Rect& r = mainModTargets;
        const float y = r.y + 18 + k * kTgtRowH;
        return { { r.x + 124, y + 4, 66, kTgtRowH - 8 }, { r.x + r.w - 46, y + 2, 30, kTgtRowH - 4 }, { r.x + r.w - 14, y + 2, 14, kTgtRowH - 4 } };
    }
    int tgtRowsMax() const { return (int)((mainModTargets.h - 18) / kTgtRowH); }
    // polarity of a route as the panel shows it: true = bipolar
    bool slotBipolar(int s) const {
        const int c = ival(modCurveParam(s));
        if (c == 7) return true;
        if (c == 4) return false;
        return modSrcBipolar(ival(modSrcParam(s)));
    }
    void toggleSlotPolarity(int s) {
        const bool natBi = modSrcBipolar(ival(modSrcParam(s)));
        const bool nowBi = slotBipolar(s);
        // bipolar sources: Linear <-> Unipolar; unipolar sources (envelopes, MSEGs, macros): Linear <-> Bipolar
        const float c = natBi ? (nowBi ? 4.f : 0.f) : (nowBi ? 0.f : 7.f);
        setParamUser(modCurveParam(s), c);
    }
    void clearSlot(int s) {
        pushUndo(-1);
        auto set = [&](uint32_t p, float v) { editParameter(p, true); setParam(p, v); editParameter(p, false); };
        set(modSrcParam(s), 0.f); set(modViaParam(s), 0.f); set(modDstParam(s), 0.f); set(modAmtParam(s), 0.f); set(modCurveParam(s), 0.f);
    }

    void drawModPanel() {
        const int src = modSel();
        const Color col = modSrcColor(src);
        // header: name + drag handle
        std::string hn = modSrcName(src);
        if (src >= msLfo1 && src <= msLfo6) hn += std::string("  ·  ") + kLfoWaveNames[std::max(0, std::min(kLfoWaves - 1, ival(lfoP(src - msLfo1, lfWave))))];
        labelS(mainModHandle.x - 8, mainModHandle.cy(), hn, 9, col, ALIGN_RIGHT | ALIGN_MIDDLE);
        drawHandle(mainModHandle, col, src);
        // tabs
        static const char* const tn[kMainModTabs] = { "LFO 1", "LFO 2", "LFO 3", "LFO 4", "LFO 5", "LFO 6", "MSEG 1", "MSEG 2", "MSEG 3", "MSEG 4", "DRIFT G", "DRIFT V" };
        std::vector<int> sl;
        for (int t = 0; t < kMainModTabs; ++t) {
            const int ts = kMainModSrc[t];
            bool used = false;
            for (int s = 0; s < kModSlots && !used; ++s) used = ival(modSrcParam(s)) == ts && ival(modDstParam(s)) != mdNone && values[modAmtParam(s)] != 0.f;
            const Rect& r = mainModTab[t];
            const bool on = ts == src, hot = hovered(r) || (sd == sdAssign && assignSrc == ts);
            const Color tc = modSrcColor(ts);
            beginPath(); roundedRect(r.x, r.y, r.w, r.h, 3);
            fillColor(on ? tint(tc, 0.24f) : (hot ? shade(22) : cInset)); fill();
            strokeColor(on ? tc : cPanelEdge); strokeWidth(1); stroke();
            labelFit(r.cx(), r.cy() + 0.5f, tn[t], 7.6f, on ? tc : (hot ? cText : cDim), ALIGN_CENTER | ALIGN_MIDDLE, r.w - 4);
            if (used && !on) { beginPath(); circle(r.x + r.w - 4, r.y + 4, 1.8f); fillColor(tc); fill(); }
        }
        // editor
        if (src >= msLfo1 && src <= msLfo6) {
            const int i = src - msLfo1;
            const Rect& r = mainLfoEdit;
            const bool custom = ival(lfoP(i, lfWave)) == 7;
            inset(r, hovered(r) || (sd == sdLfoDraw && sdIndex == i));
            save(); scissor(r.x, r.y, r.w, r.h);
            beginPath(); moveTo(r.x, r.cy()); lineTo(r.x + r.w, r.cy()); strokeColor(cGrid); strokeWidth(1); stroke();
            if (custom) for (int k = 0; k < drift::kLfoShapePoints; ++k) {
                const float x = r.x + 4 + (r.w - 8) * (float)k / drift::kLfoShapePoints;
                beginPath(); circle(x, r.cy() - lfoPts[i][k] * (r.h * 0.4f), 1.2f); fillColor(withA(col, 0.4f)); fill();
            }
            drawLfoWave(i, r, 1.5f, true);
            drawLfoSmoothing(i, r);
            restore();
            label(r.x + 5, r.y + 8, custom ? "draw to edit" : "drag in here to draw a custom shape", 7.2f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            button(lfoCopyBtn, "COPY", false, hovered(lfoCopyBtn), 7.6f);
            button(lfoPasteBtn, "PASTE", false, hovered(lfoPasteBtn) && clipLfoHave, 7.6f);
            if (!clipLfoHave) dimRect(lfoPasteBtn);
        } else if (msegOfSource(src) >= 0) {
            const int m = msegOfSource(src);
            drawMsegCard(m);
            const bool sync = values[msegP(m, 2)] >= 0.5f;
            char b[64]; std::snprintf(b, sizeof(b), "%.2f %s", msegTotal(m) * values[msegP(m, 1)], sync ? "beats" : "s");
            label(mainMsegEdit.x + mainMsegEdit.w - 6, mainMsegEdit.y + 9, b, 8, cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
            // 6.0 grid / snap, copy / paste
            label(msegGridX.x - 4, msegGridX.cy(), "GRID", 7.6f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
            miniSelect(msegGridX, kMsegGridXNames[optMsegGridX]);
            label(msegGridY.x - 4, msegGridY.cy(), "LEVEL", 7.6f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
            miniSelect(msegGridY, kMsegGridYNames[optMsegGridY]);
            label(msegGridY.x + msegGridY.w + 8, msegGridY.cy(), optMsegGridX || optMsegGridY ? "points snap to the grid (hold ALT to place freely)" : "no snapping", 7.2f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            button(msegCopyBtn, "COPY", false, hovered(msegCopyBtn), 7.6f);
            button(msegPasteBtn, "PASTE", false, hovered(msegPasteBtn) && clipMsegHave, 7.6f);
            if (!clipMsegHave) dimRect(msegPasteBtn);
            label(mainMsegEdit.x, mainMsegEdit.y + mainMsegEdit.h + 66, "click add · drag move · double-click delete · shift-drag curve · drag ▲ to move the sustain / loop points", 7.2f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
        } else if (src == msRndGlobal || src == msRndVoice) {
            const int g = src == msRndGlobal ? 0 : 1;
            const Rect& r = mainRndEdit;
            inset(r);
            beginPath();
            for (int k = 0; k < kRndHist; ++k) {
                const float v = rndHist[g][(rndHead[g] + k + 1) % kRndHist];
                const float x = r.x + 4 + (r.w - 8) * k / (kRndHist - 1), y = r.cy() - v * r.h * 0.42f;
                if (k == 0) moveTo(x, y); else lineTo(x, y);
            }
            strokeColor(cRose); strokeWidth(1.4f); stroke();
            label(r.x + 70, r.y + r.h + 44, g == 0 ? "one smooth random line shared by all voices" : "a separate random line for every voice (for a value held per note, use Note Random in the matrix)", 7.8f, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        } else {
            const Rect r = { mainLfoEdit.x, mainLfoEdit.y, 470, 140 };
            inset(r);
            const char* where = (src >= msMacro1 && src <= msMacro8) ? "Turn it on the MACROS panel." : (src == msFiltEnv || src == msAmpEnv || src == msModEnv) ? "Edit it on its envelope panel." : "A performance source (MIDI / MPE).";
            labelS(r.cx(), r.cy() - 8, modSrcName(src), 11, col, ALIGN_CENTER | ALIGN_MIDDLE);
            label(r.cx(), r.cy() + 10, where, 8.5f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        // targets
        const Rect& tr = mainModTargets;
        modPanelSlots(sl);
        char hb[48]; std::snprintf(hb, sizeof(hb), sl.empty() ? "TARGETS" : "TARGETS  %d", (int)sl.size());
        letterSp(1.f); label(tr.x, tr.y + 7, hb, 7.8f, cFaint, ALIGN_LEFT | ALIGN_MIDDLE); letterSp(0.f);
        label(tr.x + tr.w, tr.y + 7, "depth · polarity", 7.2f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
        if (sl.empty()) {
            const Rect e = { tr.x, tr.y + 18, tr.w, tr.h - 40 };
            inset(e);
            label(e.cx(), e.cy() - 9, "No targets yet.", 10, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            label(e.cx(), e.cy() + 8, "Drag ◎ or a tab onto any knob", 8.2f, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        const int maxRows = tgtRowsMax();
        for (int k = 0; k < (int)sl.size() && k < maxRows; ++k) {
            const int s = sl[k];
            const TgtRow row = tgtRow(k);
            const float y = row.bar.cy();
            const float amt = values[modAmtParam(s)];
            std::string dn = dstName(ival(modDstParam(s)));
            const int via = ival(modViaParam(s));
            if (via != msNone) dn += std::string(" × ") + modSrcName(via);
            labelFit(tr.x + 2, y, dn.c_str(), 8.6f, amt != 0.f ? cText : cDim, ALIGN_LEFT | ALIGN_MIDDLE, 120);
            // depth bar (drag sideways, double-click = 0)
            const Rect& b = row.bar;
            const bool hot = hovered(b) || (dragParam == (int)modAmtParam(s) && dragCtl < 0);
            beginPath(); roundedRect(b.x, b.y, b.w, b.h, 2); fillColor(hot ? shade(20) : cInset); fill();
            if (hot) { strokeColor(withA(col, 0.6f)); strokeWidth(1); stroke(); }
            const float cx = b.cx();
            beginPath(); rect(std::min(cx, cx + amt * b.w * 0.5f), b.y + 1, std::fabs(amt) * b.w * 0.5f, b.h - 2); fillColor(withA(col, 0.35f)); fill();
            if (shared && amt != 0.f && (newestVoice() || isGlobalDest(ival(modDstParam(s))) || src == msRndGlobal || (src >= msLfo1 && ival(lfoP(src - msLfo1, lfMode)) == 0))) {
                const float lv = std::max(-1.f, std::min(1.f, tel.slotOut[s]));
                beginPath(); rect(std::min(cx, cx + lv * b.w * 0.5f), b.y + 2, std::fabs(lv) * b.w * 0.5f, b.h - 4); fillColor(col); fill();
            }
            beginPath(); moveTo(cx, b.y); lineTo(cx, b.y + b.h); strokeColor(cFaint); strokeWidth(1); stroke();
            char vb[16]; std::snprintf(vb, sizeof(vb), "%+.0f%%", amt * 100.f);
            label(b.x + b.w + 32, y, vb, 8.2f, hot ? cText : cDim, ALIGN_RIGHT | ALIGN_MIDDLE);
            // polarity
            const bool bi = slotBipolar(s);
            const int crv = ival(modCurveParam(s));
            const char* pl = bi ? "BI" : "UNI";
            if (crv != 0 && crv != 4 && crv != 7) pl = crv == 1 ? "EXP" : crv == 2 ? "LOG" : crv == 3 ? "S" : crv == 5 ? "INV" : "STP";
            button(row.pol, pl, bi, hovered(row.pol), 7.6f, &col);
            // remove
            const bool dh = hovered(row.del);
            label(row.del.cx(), row.del.cy(), "✕", 8.5f, dh ? cWarn : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        if ((int)sl.size() > maxRows) { char b[40]; std::snprintf(b, sizeof(b), "+%d more on the MOD page", (int)sl.size() - maxRows); label(tr.x + tr.w, tr.y + tr.h + 2, b, 7.6f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE); }
    }

    // 6.0: a small dropdown-looking box that cycles on click (editor options)
    void miniSelect(const Rect& r, const char* txt) {
        const bool hot = hovered(r);
        beginPath(); roundedRect(r.x, r.y, r.w, r.h, 3); fillColor(hot ? shade(34.f) : cInset); fill(); strokeColor(cPanelEdge); strokeWidth(1); stroke();
        labelFit(r.cx(), r.cy() + 0.5f, txt, 8.4f, cText, ALIGN_CENTER | ALIGN_MIDDLE, r.w - 6);
    }
    // 6.0: LFO SMOOTH shown on the editor: the smoothed curve (one-pole on the shape at the current rate)
    void drawLfoSmoothing(int i, const Rect& r) {
        if (ival(lfoP(i, lfMode)) >= 2) {   // Retrig / One-shot: where each note starts (PHASE, + RND PHASE range)
            const float ph = values[lfoP(i, lfPhase)], rp = values[pLRndPh1 + i];
            const float x0 = r.x + 4 + (r.w - 8) * (ph - std::floor(ph));
            if (rp > 0.001f) { beginPath(); rect(x0, r.y + 2, (r.w - 8) * std::min(1.f, rp), r.h - 4); fillColor(withA(cWarm, 0.10f)); fill(); }
            beginPath(); moveTo(x0, r.y + 2); lineTo(x0, r.y + r.h - 2); strokeColor(withA(cWarm, 0.7f)); strokeWidth(1); stroke();
            label(x0 + 3, r.y + r.h - 8, "start", 7.f, cWarm, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        const float smt = values[lfoP(i, lfSmooth)];
        if (smt <= 0.001f) return;
        const float rate = values[lfoP(i, lfSync)] >= 0.5f ? 1.f : std::max(0.005f, values[lfoP(i, lfRate)]);
        const float tau = smt * smt * 0.5f + 1e-4f;
        const int n = (int)std::max(64.f, r.w);
        const float dt = 1.f / (rate * n);
        const float k = 1.f - std::exp(-dt / tau);
        float y = lfoShapeValue(i, 0.f);
        for (int pass = 0; pass < 3; ++pass) for (int q = 0; q <= n; ++q) y += (lfoShapeValue(i, (float)q / n) - y) * k;   // settle one cycle
        beginPath();
        for (int q = 0; q <= n; ++q) {
            y += (lfoShapeValue(i, (float)q / n) - y) * k;
            const float x = r.x + 4 + (r.w - 8) * q / n, yy = r.cy() - y * (r.h * 0.4f);
            if (q == 0) moveTo(x, yy); else lineTo(x, yy);
        }
        strokeColor(withA(cText, 0.55f)); strokeWidth(1.1f); stroke();
        label(r.x + r.w - 6, r.y + r.h - 8, "smoothed", 7.f, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
    }
    // clicks inside the panel that are not ordinary controls (those go through the generic path)
    bool modPanelClick(float mx, float my, const MouseEvent& ev) {
        const int src = modSel();
        if (src >= msLfo1 && src <= msLfo6) {
            const int i = src - msLfo1;
            if (lfoCopyBtn.contains(mx, my)) { copyLfo(i); return true; }
            if (lfoPasteBtn.contains(mx, my)) { pasteLfo(i); return true; }
        }
        if (msegOfSource(src) >= 0) {
            const int m = msegOfSource(src);
            if (msegGridX.contains(mx, my)) { optMsegGridX = (optMsegGridX + ((ev.button == 2 || (ev.mod & kModifierShift)) ? kMsegGridX - 1 : 1)) % kMsegGridX; saveOptions(); repaint(); return true; }
            if (msegGridY.contains(mx, my)) { optMsegGridY = (optMsegGridY + ((ev.button == 2 || (ev.mod & kModifierShift)) ? kMsegGridY - 1 : 1)) % kMsegGridY; saveOptions(); repaint(); return true; }
            if (msegCopyBtn.contains(mx, my)) { clipMseg = msegD[m]; clipMsegHave = true; flash("MSEG copied"); return true; }
            if (msegPasteBtn.contains(mx, my) && clipMsegHave) { pushUndo(-1); msegD[m] = clipMseg; pushState(msegStateKey(m), drift::msegToString(msegD[m]), false); flash("MSEG pasted"); repaint(); return true; }
        }
        if (src >= msLfo1 && src <= msLfo6 && mainLfoEdit.contains(mx, my)) { startLfoDraw(src - msLfo1, mx, my); return true; }
        if (msegOfSource(src) >= 0 && mainMsegEdit.contains(mx, my)) { msegPress(msegOfSource(src), mx, my, ev); return true; }
        std::vector<int> sl; modPanelSlots(sl);
        for (int k = 0; k < (int)sl.size() && k < tgtRowsMax(); ++k) {
            const int s = sl[k];
            const TgtRow row = tgtRow(k);
            if (row.del.contains(mx, my)) { clearSlot(s); return true; }
            if (row.pol.contains(mx, my)) { toggleSlotPolarity(s); return true; }
            const Rect hit = { row.bar.x - 2, row.bar.y - 4, row.bar.w + 32, row.bar.h + 8 };
            if (hit.contains(mx, my)) {
                if (doubleClick(3000 + s, ev.time)) { setParamUser(modAmtParam(s), 0.f); return true; }
                beginDrag(-1, modAmtParam(s), 3, mx, my);
                dragRect = row.bar;   // the bar spans the whole -1..+1 range
                return true;
            }
        }
        return false;
    }
