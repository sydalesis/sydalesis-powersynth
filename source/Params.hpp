// Driftfield 4 — parameter registry shared by engine, plugin and UI.
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include "Tuning.hpp"

// ------------------------------------------------------------------------------------------------
// Per-oscillator block (repeated for A, B, C)
enum OscField : uint32_t {
    ofOn, ofType, ofLevel, ofPan, ofOct, ofSemi, ofFine, ofVoices, ofDetune, ofUniWidth, ofPhase, ofRetrig,
    ofFmSrc, ofFmMode, ofFmAmt, ofSyncSrc,
    ofTable, ofPos, ofWarp, ofWarpAmt, ofWtBlend,
    ofAnWave, ofAnPW, ofAnSync, ofAnSlop, ofAnDrift,
    ofAdSpec1, ofAdSpec2, ofAdMorph, ofAdPartials, ofAdTilt, ofAdOddEven, ofAdStretch,
    ofAdPeakPos, ofAdPeakAmt, ofAdPeakWidth, ofAdComb, ofAdCombShift, ofAdShimmer, ofAdShimRate, ofAdDetune,
    ofAdVowel, ofAdVowelAmt, ofAdBlur, ofAdFreeze,
    ofPmModel, ofPmExciter, ofPmDecay, ofPmDamp, ofPmHard, ofPmPos, ofPmStretch, ofPmSpread, ofPmCouple, ofPmBuzz,
    ofGrSource, ofGrPos, ofGrScan, ofGrSize, ofGrDensity, ofGrSpray, ofGrPitchRand, ofGrShape, ofGrSpread, ofGrReverse,
    kOscFields
};
static constexpr int kOscs = 3;
static inline constexpr uint32_t oscP(int osc, uint32_t field) { return (uint32_t)osc * kOscFields + field; }

enum OscType { otWavetable, otAnalog, otAdditive, otPhysical, otGranular, otSpectral, otFm, kOscTypes };   // 6.0: FM

// 4.2: second per-oscillator block (appended so earlier parameter ids stay valid); fields past kOsc2Used
// are hidden reserves for later engines
enum Osc2Field : uint32_t { o2Pressure, o2Noise, o2Nonlin,
    // 5.0 spectral oscillator (were hidden reserves in 4.2)
    o2SpPos, o2SpSpeed, o2SpFreeze, o2SpBlur, o2SpShift, o2SpStretch, o2SpTilt, o2SpFormant, o2SpSparse,
    o2SpCross, o2SpCrossMode, o2SpKeyed,
    o2PhaseRnd,                                      // 6.0: random start phase (with RTG on)
    kOsc2Used, kOsc2Fields = 16 };

static constexpr int kLfos = 6;
static constexpr int kLfoFields = 8;       // rate wave mode fade sync div phase smooth
static constexpr int kMsegs = 4;          // 4.2: MSEG 3 and 4
static constexpr int kMacros = 8;
static constexpr int kModSlots = 32;      // 4.2: slots 25-32 live in their own block (see slotP)
static constexpr int kModSlots1 = 24;
static constexpr int kSlotFields = 5;      // src via dst amt curve
static constexpr int kSeqSteps = 16;

// ------------------------------------------------------------------------------------------------
enum ParamId : uint32_t {
    pOscEnd = kOscs * kOscFields - 1,
    pSubLevel, pSubWave, pSubOct, pNoiseLevel, pNoiseColor, pRingLevel, pRingSource,
    pF1Type, pF1Cutoff, pF1Res, pF1Drive, pF1DriveType, pF1Key, pF1Env, pF1Shape, pF1Type2, pF1Morph, pF1FmSrc, pF1FmAmt,
    pF2Type, pF2Cutoff, pF2Res, pF2Drive, pF2DriveType, pF2Key, pF2Env, pF2Shape, pF2Type2, pF2Morph, pF2FmSrc, pF2FmAmt,
    pFRouting, pFBalance,
    pFEnvA, pFEnvD, pFEnvS, pFEnvR,
    pAEnvA, pAEnvD, pAEnvS, pAEnvR,
    pMEnvA, pMEnvD, pMEnvS, pMEnvR,
    pL1Rate,
    pLLast = pL1Rate + kLfos * kLfoFields - 1,
    pMs1Mode, pMs1Scale, pMs1Sync, pMs2Mode, pMs2Scale, pMs2Sync,
    pRndGRate, pRndVRate,
    pMacro1,
    pMacroLast = pMacro1 + kMacros - 1,
    pM1Src,
    pMLast = pM1Src + kModSlots1 * kSlotFields - 1,
    pBendRange, pPressCut, pPressAmp, pSlideCut, pSlideTimbre, pVelAmp, pMWCut,
    pPolyMode, pGlide, pDrift, pSpread, pMaster, pQuality, pPolyphony,
    pArpOn, pArpMode, pArpOct, pArpRate, pArpGate, pArpSwing, pArpHold,
    pSeqMode, pSeqRate, pSeqSteps, pSeqGate, pSeqSwing, pSeqGlide,
    pSeqV1,
    pSeqVLast = pSeqV1 + kSeqSteps - 1,
    pSeqG1,
    pSeqGLast = pSeqG1 + kSeqSteps - 1,
    // FX units (order is a state string; each unit has its own on switch)
    pDrvOn, pDrvAmt, pDrvTone,
    pPhOn, pPhMix, pPhRate, pPhDepth, pPhFb,
    pChOn, pChMode, pChMix, pChRate, pChDepth, pChFb,
    pDlOn, pDlMix, pDlTime, pDlFb, pDlMode, pDlDamp, pDlSync, pDlDiv,
    pFsOn, pFsShift, pFsMix, pFsFb, pFsStereo,
    pTpOn, pTpDrive, pTpWow, pTpFlutter, pTpTone, pTpMix,
    pCrOn, pCrBits, pCrDown, pCrMix,
    pWdOn, pWdWidth, pWdBassMono,
    pEqOn, pEqLowG, pEqLowF, pEqMidG, pEqMidF, pEqMidQ, pEqHighG, pEqHighF,
    // 3.1 (appended so parameter ids of 3.0 sessions stay valid)
    pFEnvCurve, pAEnvCurve, pMEnvCurve,
    // 4.0: microtuning and note expression
    pTnSource, pTnScale, pTnRoot, pTnRef, pTnAmount, pTnSnap, pTnGlide, pNeVibDepth, pNeVibRate,
    // 4.2: physical modelling 2.0 (per osc), MSEG 3/4, mod slots 25-32, Resonator + Space FX, scenes, motion
    pO2First,
    pO2Last = pO2First + kOscs * kOsc2Fields - 1,
    pMs3Mode, pMs3Scale, pMs3Sync, pMs4Mode, pMs4Scale, pMs4Sync,
    pM25Src,
    pM2Last = pM25Src + (kModSlots - kModSlots1) * kSlotFields - 1,
    pRsOn, pRsMix, pRsDecay, pRsBright, pRsMode, pRsRoot, pRsOct, pRsWidth,
    pSpOn, pSpMix, pSpSize, pSpDecay, pSpDamp, pSpPre, pSpMod, pSpWidth,
    pScOn, pScPos, pScGlide,
    pMoMode, pMoSpeed,
    pSxOn, pSxMix, pSxFreeze, pSxBlur, pSxShift, pSxTilt, pSxSpread, pSxGate,   // 5.0 Spectral FX (were reserves)
    pXy1Mode, pXy1Rate, pXy1Sync, pXy1Div, pXy1Size, pXy1Dir,                     // 5.1.1 XY auto-motion (were reserves)
    pXy2Mode, pXy2Rate, pXy2Sync, pXy2Div, pXy2Size, pXy2Dir,
    pFRelCurve, pARelCurve, pMRelCurve,                                           // 5.1.3 release curves (were reserves)
    pTpWowRate, pTpFlutRate, pPitch,                                              // 5.1.6 tape speeds, layer pitch (were reserves)
    pFDecCurve, pADecCurve, pMDecCurve,                                           // 6.0 decay curves (were reserves)
    pLRndPh1, pLRndPhLast = pLRndPh1 + kLfos - 1,                                 // 6.0 LFO random start phase
    pLevelComp,                                                                   // 6.0 Granular / Spectral level compensation
    pFmV6,                                                                        // 6.0 bits 0-2: osc A-C use PM / FM on Add / Phys / Spec
    pRsv1,                              // hidden reserves: later versions add layer parameters here
    pRsvLast = 699,
    kLayerParams,                       // everything above is one layer's sound (layer 1 = these indices)
    // 4.1: layer setup, shared by both layers (+ hidden reserves)
    pLyMode = kLayerParams, pLySplit, pLyWidth, pLyMorph,
    pGRsv1,
    pGRsvLast = kLayerParams + 15,
    kEngineParams,                      // an engine instance sees indices 0 .. kEngineParams-1
    kLayerBOffset = kEngineParams,      // layer 2 = kLayerBOffset + (layer-1 index), symbol "b_" + symbol
    kParamCountV5 = kEngineParams + kLayerParams       // every parameter up to 5.1.7
};

// ------------------------------------------------------------------------------------------------
// 6.0 extension block: parameters added after the 5.x list so no existing parameter id moves (sessions and
// automation keep working). Layer 1's block starts at pX0, layer 2's copy follows it ("b_" symbols). The engine
// sees layer-local indices: 0 .. kEngineParams-1 and pX0 .. pX0+kExtParams-1.
// FM oscillator (per osc): 8 oscillator-wide fields, then 6 operators x 13 fields.
enum FmField { fmAlgo, fmFeedback, fmIndex, fmBright, fmSpread, fmEnvTime, fmVelIdx, fmKeyIdx, kFmGlobal };
enum FmOpField { foCoarse, foFine, foDetune, foFixed, foHz, foLevel, foWave, foAttack, foDecay, foSustain, foRelease, foVel, foKey, kFmOpFields };
static constexpr int kFmOps = 6;
static constexpr int kFmFields = kFmGlobal + kFmOps * kFmOpFields;    // 86
static constexpr int kFmAlgos = 32;
static constexpr int kFmWaves = 8;
static constexpr uint32_t pX0 = kParamCountV5;
static constexpr uint32_t kExtUsed = 3 * kFmFields;
static constexpr uint32_t kExtParams = kExtUsed + 30;                 // + hidden reserves for later
static constexpr uint32_t kParamCount = kParamCountV5 + 2 * kExtParams;
static constexpr uint32_t kEngineArr = pX0 + kExtParams;               // size of an engine's parameter array
static inline uint32_t fmP(int osc, int f) { return pX0 + (uint32_t)osc * kFmFields + (uint32_t)f; }
static inline uint32_t fmOpP(int osc, int op, int f) { return fmP(osc, kFmGlobal + op * kFmOpFields + f); }
static inline bool isExtParam(uint32_t i) { return i >= pX0 && i < pX0 + kExtParams; }
// a layer parameter (one value per layer): the 5.x layer block or the extension block (layer-1 indices)
static inline bool isLayerParam(uint32_t i) { return i < kLayerParams || isExtParam(i); }
// layer-1 index -> layer-2 index
static inline uint32_t layer2Of(uint32_t i) { return i < kLayerParams ? i + kLayerBOffset : i + kExtParams; }
// host index -> (layer, engine index); layer -1 = shared by both layers
static inline int hostToEngine(uint32_t i, uint32_t& e) {
    if (i < kLayerParams) { e = i; return 0; }
    if (i < kEngineParams) { e = i; return -1; }
    if (i < kParamCountV5) { e = i - kLayerBOffset; return 1; }
    if (i < pX0 + kExtParams) { e = i; return 0; }
    e = i - kExtParams; return 1;
}
// every layer-1 parameter index, in order (5.x block then the extension block)
template <typename F> static inline void forEachLayerParam(F f) {
    for (uint32_t i = 0; i < kLayerParams; ++i) f(i);
    for (uint32_t i = pX0; i < pX0 + kExtParams; ++i) f(i);
}
static_assert(pRsv1 <= pRsvLast, "layer parameters overflow the reserve");
static inline uint32_t osc2P(int osc, uint32_t field) { return pO2First + (uint32_t)osc * kOsc2Fields + field; }
static inline uint32_t lfoP(int i, int f) { return pL1Rate + (uint32_t)kLfoFields * (uint32_t)i + (uint32_t)f; }
enum LfoField { lfRate, lfWave, lfMode, lfFade, lfSync, lfDiv, lfPhase, lfSmooth };
static inline uint32_t msegP(int i, int f) { return (i < 2 ? pMs1Mode + 3u * (uint32_t)i : pMs3Mode + 3u * (uint32_t)(i - 2)) + (uint32_t)f; }   // 0 mode 1 scale 2 sync
static inline uint32_t slotP(int s, int f) { return (s < kModSlots1 ? pM1Src + (uint32_t)kSlotFields * (uint32_t)s : pM25Src + (uint32_t)kSlotFields * (uint32_t)(s - kModSlots1)) + (uint32_t)f; }
enum SlotField { sfSrc, sfVia, sfDst, sfAmt, sfCurve };
static inline uint32_t modSrcParam(int s) { return slotP(s, sfSrc); }
static inline uint32_t modViaParam(int s) { return slotP(s, sfVia); }
static inline uint32_t modDstParam(int s) { return slotP(s, sfDst); }
static inline uint32_t modAmtParam(int s) { return slotP(s, sfAmt); }
static inline uint32_t modCurveParam(int s) { return slotP(s, sfCurve); }
static inline bool inMatrix(uint32_t p) { return (p >= pM1Src && p <= pMLast) || (p >= pM25Src && p <= pM2Last); }
static inline uint32_t slotFieldOf(uint32_t p) { return (p <= pMLast ? p - pM1Src : p - pM25Src) % kSlotFields; }
static inline int slotOfParam(uint32_t p) { return p <= pMLast ? (int)((p - pM1Src) / kSlotFields) : kModSlots1 + (int)((p - pM25Src) / kSlotFields); }
static inline bool isModSrc(uint32_t p) { return inMatrix(p) && (slotFieldOf(p) == sfSrc || slotFieldOf(p) == sfVia); }
static inline bool isModDst(uint32_t p) { return inMatrix(p) && slotFieldOf(p) == sfDst; }
static inline uint32_t filtP(int f, int k) { return (f == 0 ? pF1Type : pF2Type) + (uint32_t)k; }   // k: 0 type .. 11 fmAmt
enum FiltField { ffType, ffCutoff, ffRes, ffDrive, ffDriveType, ffKey, ffEnv, ffShape, ffType2, ffMorph, ffFmSrc, ffFmAmt };

// ------------------------------------------------------------------------------------------------
enum ParamFlags : uint32_t { fNone = 0, fInt = 1, fLog = 2, fBool = 4, fHidden = 8 };

struct ParamDef {
    const char* name;
    const char* symbol;
    const char* unit;
    float min, max, def;
    uint32_t flags;
    const char* const* enumNames;   // for enumerated integer params
};

// ------------------------------------------------------------------------------------------------
// Enumeration names
static constexpr int kXyShapes = 8;
static const char* const kXyShapeNames[kXyShapes] = { "Off", "Circle", "Ellipse", "Figure 8", "Lissajous", "Spiral", "Pendulum", "Wander" };
static const char* const kLyModeNames[6] = { "Off", "Layer", "Split", "Velocity XF", "Pressure XF", "Morph" };
static const char* const kTnSourceNames[4] = { "Equal", "Library", "Scala", "MTS-ESP" };
static const char* const kTnRootNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static const char* const kOscTypeNames[kOscTypes] = { "Wavetable", "Analog", "Additive", "Physical", "Granular", "Spectral", "FM" };
static const char* const kFmWaveNames[kFmWaves] = { "Sine", "Half Sine", "Abs Sine", "Quarter", "Alt Sine", "Camel", "Soft Square", "Soft Saw" };
static const char* const kFmAlgoNames[kFmAlgos] = { "Alg 1", "Alg 2", "Alg 3", "Alg 4", "Alg 5", "Alg 6", "Alg 7", "Alg 8", "Alg 9", "Alg 10", "Alg 11", "Alg 12", "Alg 13", "Alg 14", "Alg 15", "Alg 16", "Alg 17", "Alg 18", "Alg 19", "Alg 20", "Alg 21", "Alg 22", "Alg 23", "Alg 24", "Alg 25", "Alg 26", "Alg 27", "Alg 28", "Alg 29", "Alg 30", "Alg 31", "Alg 32" };
static const char* const kSpCrossNames[3] = { "Off", "Morph", "Vocode" };
static const char* const kTableNames[8] = { "Analog Morph", "Spectral Tilt", "Formant Choir", "Glass Partials",
                                            "Hollow Pulse", "Drawbar Organ", "Odd Shimmer", "Dark Reed" };
static constexpr int kWarpModes = 10;
static const char* const kWarpNames[kWarpModes] = { "Off", "Bend +", "Bend -", "Bend ±", "Pulse", "Sync", "Mirror", "Asym", "Quantize", "Flip" };
static const char* const kBlendNames[3] = { "Smooth", "Spectral", "Stepped" };
static constexpr int kAnWaves = 6;
static const char* const kAnWaveNames[kAnWaves] = { "Saw", "Pulse", "Saw + Pulse", "Triangle", "Sine", "Supersaw" };
static constexpr int kSpectra = 17;
static const char* const kSpectrumNames[kSpectra] = { "Saw", "Square", "Triangle", "Organ", "Choir Ah", "Choir Oo",
                                                      "Bell", "Glass", "Hollow", "Strings", "Reed",
                                                      "Throat", "Octaves", "Fifths", "Primes", "Custom", "Custom 2" };
static constexpr int kSpecCustom = 15;
static constexpr int kSpecCustom2 = 16;   // 6.0: a second drawable spectrum, so two drawings can morph
static constexpr int kPmModels = 14;
static const char* const kPmModelNames[kPmModels] = { "String", "Bar", "Bell", "Membrane", "Plate", "Glass Bowl", "Tube",
                                                      "Waveguide", "Tanpura", "Sympathetic", "Reed", "Flute", "Gong", "Analysed" };
enum PmModel { pmString, pmBar, pmBell, pmMembrane, pmPlate, pmBowl, pmTube, pmWaveguide, pmTanpura, pmSympathetic,
               pmReed, pmFlute, pmGong, pmAnalysed };
static inline bool pmIsWaveguide(int m) { return m >= pmWaveguide && m <= pmFlute; }    // delay-line models
static constexpr int kPmExciters = 9;
static const char* const kPmExciterNames[kPmExciters] = { "Strike", "Bow", "Breath", "Osc Input", "Scrape", "Rain", "Pluck", "Mallet", "Sample" };
static const char* const kRsModeNames[6] = { "Fifths", "Octaves", "Major", "Minor", "Tanpura", "Held Notes" };
static const char* const kMoModeNames[4] = { "Off", "Loop", "Note", "Host" };
static const char* const kRsOctNames[5] = { "Octave 1", "Octave 2", "Octave 3", "Octave 4", "Octave 5" };
static constexpr int kScenes = 8;
static const char* const kGrSourceNames[3] = { "Wavetable", "Sample Keyed", "Sample Fixed" };
static const char* const kFmSrcNames[6] = { "Off", "Osc A", "Osc B", "Osc C", "Sub", "Noise" };
static const char* const kFmModeNames[6] = { "PM", "FM", "AM", "Ring", "PM Hard", "FM Hard" };   // 6.0: hard modes appended
static const char* const kSyncSrcNames[4] = { "Off", "Osc A", "Osc B", "Osc C" };
static const char* const kSubWaveNames[4] = { "Sine", "Triangle", "Saw", "Square" };
static const char* const kRingNames[4] = { "A x B", "A x C", "B x C", "A x B x C" };
static constexpr int kFilterTypes = 87;
static const char* const kFilterNames[kFilterTypes] = {
    "Off", "Ladder LP24", "Ladder LP18", "Ladder LP12", "Ladder LP6", "Ladder HP24", "Ladder HP18",
    "Ladder HP12", "Ladder HP6", "Ladder BP24", "Ladder BP12", "Ladder Notch", "Diode LP24",
    "SVF LP12", "SVF HP12", "SVF BP12", "SVF Notch", "SVF Peak", "SVF LP24", "SVF HP24", "SVF BP24",
    "MS-20 LP12", "MS-20 HP12", "SEM LP12", "SEM BP12", "SEM HP12", "SEM Notch", "SEM Morph",
    "OB-Xa LP12", "OB-Xa LP24", "OB-Xa BP", "OB-Xa HP",
    "Xpander LP4", "Xpander LP3", "Xpander LP2", "Xpander LP1", "Xpander HP3", "Xpander HP2", "Xpander HP1",
    "Xpander BP4", "Xpander BP2", "Xpander N2", "Xpander PH3", "Xpander HP2+LP1", "Xpander HP3+LP1",
    "Xpander N2+LP1", "Xpander PH3+LP1",
    "Dual Bandpass", "Dual Notch", "Comb +", "Comb -", "Phaser 4", "Phaser 8", "Formant", "Vowel",
    "Cmb +", "Cmb -", "Cmb L6+", "Cmb L6-", "Cmb H6+", "Cmb H6-", "Cmb HL6+", "Cmb HL6-",
    "Flg +", "Flg -", "Flg L6+", "Flg L6-", "Flg H6+", "Flg H6-", "Flg HL6+", "Flg HL6-",
    "Phs 12+", "Phs 12-", "Phs 24+", "Phs 24-", "Phs 36+", "Phs 36-", "Phs 48+", "Phs 48-",
    "Phs 48L6+", "Phs 48L6-", "Phs 48H6+", "Phs 48H6-", "Phs 48HL6+", "Phs 48HL6-", "FPhs 12HL6+", "FPhs 12HL6-" };
static const char* const kDriveTypeNames[4] = { "Soft", "Tube", "Diode", "Fold" };
static const char* const kFiltFmNames[5] = { "Off", "Osc A", "Osc B", "Osc C", "Noise" };
static const char* const kRoutingNames[3] = { "Serial", "Parallel", "Split A | BC" };
static constexpr int kLfoWaves = 8;
static const char* const kLfoWaveNames[kLfoWaves] = { "Sine", "Triangle", "Saw Up", "Saw Down", "Square", "S&H", "Drift", "Custom" };
static const char* const kLfoModeNames[4] = { "Global", "Voice", "Retrig", "One-shot" };
static constexpr int kDivs = 16;
static const char* const kDivNames[kDivs] = { "8 bars", "4 bars", "2 bars", "1 bar", "1/2", "1/2T", "1/4.", "1/4", "1/4T",
                                              "1/8.", "1/8", "1/8T", "1/16.", "1/16", "1/16T", "1/32" };
static const float kDivBeats[kDivs] = { 32.f, 16.f, 8.f, 4.f, 2.f, 4.f / 3.f, 1.5f, 1.f, 2.f / 3.f,
                                        0.75f, 0.5f, 1.f / 3.f, 0.375f, 0.25f, 1.f / 6.f, 0.125f };
static const char* const kMsegModeNames[3] = { "Envelope", "Loop", "Free Loop" };
static const char* const kCurveNames[8] = { "Linear", "Exp", "Log", "S-Curve", "Unipolar", "Invert", "Stepped", "Bipolar" };   // 5.1.6: Bipolar (0..1 -> -1..1)
static const char* const kPolyNames[3] = { "Poly", "Mono", "Legato" };
static const char* const kQualityNames[3] = { "Eco", "Normal", "HQ" };
static const char* const kArpModeNames[7] = { "Up", "Down", "Up/Down", "Down/Up", "Random", "Order", "Chord" };
static const char* const kSeqModeNames[3] = { "Off", "CV", "Notes" };
static const char* const kChorusNames[4] = { "Chorus", "Ensemble", "Solina", "Flanger" };
static const char* const kDelayNames[2] = { "Stereo", "Ping-Pong" };

enum ModSource {
    msNone,
    msLfo1, msLfo2, msLfo3, msLfo4, msLfo5, msLfo6,
    msFiltEnv, msAmpEnv, msModEnv, msMseg1, msMseg2,
    msVelocity, msPressure, msSlide, msBend, msModWheel, msKey,
    msNoteRnd, msRndGlobal, msRndVoice, msAlternate,
    msMacro1, msMacro2, msMacro3, msMacro4, msMacro5, msMacro6, msMacro7, msMacro8,
    msSeq, msExpression, msBreath,
    msMseg3, msMseg4,                                   // 4.2
    kModSrcCount };
static const char* const kModSrcNames[kModSrcCount] = {
    "—", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "LFO 5", "LFO 6",
    "Filter Env", "Amp Env", "Mod Env", "MSEG 1", "MSEG 2",
    "Velocity", "Pressure", "Slide", "Bend", "Mod Wheel", "Key",
    "Note Random", "Drift Global", "Drift Voice", "Alternate",
    "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Macro 5", "Macro 6", "Macro 7", "Macro 8",
    "Sequencer", "Expression", "Breath", "MSEG 3", "MSEG 4" };
// sources whose output is already -1..1 (their polarity switch is Linear / Unipolar; the rest use Bipolar / Linear)
static inline bool modSrcBipolar(int s) { return (s >= msLfo1 && s <= msLfo6) || s == msBend || s == msKey || s == msNoteRnd || s == msRndGlobal || s == msRndVoice || s == msAlternate || s == msSeq; }

enum ModDest {
    mdNone,
    mdF1Cutoff, mdF1Res, mdF1Drive, mdF1Shape, mdF1Morph, mdF1Fm,
    mdF2Cutoff, mdF2Res, mdF2Drive, mdF2Shape, mdF2Morph, mdF2Fm,
    mdCutoffBoth, mdBalance, mdPitchAll, mdTimbreAll, mdDetuneAll, mdPanAll, mdRing, mdSub, mdNoise, mdAmp,
    mdAPitch, mdALevel, mdAPan, mdAT1, mdAT2, mdAT3, mdAT4, mdAFm, mdADetune,
    mdBPitch, mdBLevel, mdBPan, mdBT1, mdBT2, mdBT3, mdBT4, mdBFm, mdBDetune,
    mdCPitch, mdCLevel, mdCPan, mdCT1, mdCT2, mdCT3, mdCT4, mdCFm, mdCDetune,
    mdLfo1Rate, mdLfo2Rate, mdLfo3Rate, mdLfo4Rate, mdLfo5Rate, mdLfo6Rate, mdMseg1Rate, mdMseg2Rate,
    mdFxPhaser, mdFxChorus, mdFxDelay, mdFxShift, mdFxShiftMix, mdFxWow, mdFxCrush, mdFxWidth, mdFxDrive,
    mdMseg3Rate, mdMseg4Rate, mdAPress, mdBPress, mdCPress, mdFxReso, mdFxSpace,      // 4.2
    mdFxSpecMix, mdFxSpecBlur,                                                        // 5.0
    mdAOct, mdASemi, mdAFine, mdBOct, mdBSemi, mdBFine, mdCOct, mdCSemi, mdCFine,      // 5.1.6: OCT / SEMI / FINE knobs
    mdAT5, mdAT6, mdAT7, mdAT8, mdAT9, mdAT10,                                        // 5.1.7: engine parameters
    mdBT5, mdBT6, mdBT7, mdBT8, mdBT9, mdBT10,                                        //        Timbre 1-4 do not reach
    mdCT5, mdCT6, mdCT7, mdCT8, mdCT9, mdCT10,                                        //        (see kTimbreXNames)
    // 6.0: envelope stages (per voice; times: amount 1 = x16 / /16), oscillator start phase (read at note-on),
    // and FX parameters beyond the mixes (global)
    mdFEnvA, mdFEnvD, mdFEnvS, mdFEnvR, mdAEnvA, mdAEnvD, mdAEnvS, mdAEnvR, mdMEnvA, mdMEnvD, mdMEnvS, mdMEnvR,
    mdAPhase, mdBPhase, mdCPhase,
    mdFxPhRate, mdFxPhFb, mdFxChRate, mdFxChDepth, mdFxDlTime, mdFxDlFb, mdFxDlDamp, mdFxFsFb,
    mdFxTpFlutter, mdFxTpTone, mdFxRsDecay, mdFxRsBright, mdFxSpSize, mdFxSpDecay, mdFxSpDamp, mdFxSxShift, mdFxSxTilt,
    kModDstCount };
static inline int envDst(int env, int stage) { return mdFEnvA + env * 4 + stage; }   // env 0 filter 1 amp 2 mod; stage 0 A .. 3 R
static constexpr int kTimbreX = 6;
static inline int modDstXBase(int osc) { return mdAT5 + osc * kTimbreX; }
static inline int modDstTuneBase(int osc) { return mdAOct + osc * 3; }   // + 0 octave, 1 semi, 2 fine
static constexpr int kOscDstStride = 9;   // pitch level pan t1 t2 t3 t4 fm detune
static inline int modDstOscBase(int osc) { return mdAPitch + osc * kOscDstStride; }
static inline bool isGlobalDest(int d) { return (d >= mdFxPhaser && d <= mdFxDrive) || (d >= mdFxReso && d <= mdFxSpecBlur) || (d >= mdFxPhRate && d <= mdFxSxTilt); }   // FX Resonator .. FX Spectral Blur
static const char* const kModDstNames[kModDstCount] = {
    "—",
    "F1 Cutoff", "F1 Reso", "F1 Drive", "F1 Shape", "F1 Morph", "F1 FM",
    "F2 Cutoff", "F2 Reso", "F2 Drive", "F2 Shape", "F2 Morph", "F2 FM",
    "Cutoff 1+2", "Filter Balance", "Pitch All", "Timbre 1 All", "Detune All", "Pan All", "Ring", "Sub", "Noise", "Amp",
    "A Pitch", "A Level", "A Pan", "A Timbre 1", "A Timbre 2", "A Timbre 3", "A Timbre 4", "A FM", "A Detune",
    "B Pitch", "B Level", "B Pan", "B Timbre 1", "B Timbre 2", "B Timbre 3", "B Timbre 4", "B FM", "B Detune",
    "C Pitch", "C Level", "C Pan", "C Timbre 1", "C Timbre 2", "C Timbre 3", "C Timbre 4", "C FM", "C Detune",
    "LFO 1 Rate", "LFO 2 Rate", "LFO 3 Rate", "LFO 4 Rate", "LFO 5 Rate", "LFO 6 Rate", "MSEG 1 Rate", "MSEG 2 Rate",
    "FX Phaser Mix", "FX Chorus Mix", "FX Delay Mix", "FX Shift Hz", "FX Shift Mix", "FX Tape Wow", "FX Crush Mix",
    "FX Width", "FX Drive",
    "MSEG 3 Rate", "MSEG 4 Rate", "A Pressure", "B Pressure", "C Pressure", "FX Resonator Mix", "FX Space Mix",
    "FX Spectral Mix", "FX Spectral Blur",
    "A Octave", "A Semi", "A Fine", "B Octave", "B Semi", "B Fine", "C Octave", "C Semi", "C Fine",
    "A Timbre 5", "A Timbre 6", "A Timbre 7", "A Timbre 8", "A Timbre 9", "A Timbre 10",
    "B Timbre 5", "B Timbre 6", "B Timbre 7", "B Timbre 8", "B Timbre 9", "B Timbre 10",
    "C Timbre 5", "C Timbre 6", "C Timbre 7", "C Timbre 8", "C Timbre 9", "C Timbre 10",
    "FEnv Attack", "FEnv Decay", "FEnv Sustain", "FEnv Release", "AEnv Attack", "AEnv Decay", "AEnv Sustain", "AEnv Release",
    "MEnv Attack", "MEnv Decay", "MEnv Sustain", "MEnv Release",
    "A Phase", "B Phase", "C Phase",
    "FX Phaser Rate", "FX Phaser Fb", "FX Chorus Rate", "FX Chorus Depth", "FX Delay Time", "FX Delay Fb", "FX Delay Damp", "FX Shift Fb",
    "FX Tape Flutter", "FX Tape Tone", "FX Reso Decay", "FX Reso Bright", "FX Space Size", "FX Space Decay", "FX Space Damp",
    "FX Spectral Shift", "FX Spectral Tilt" };

// What "Timbre 1..4" mean for each oscillator engine (shown in the UI)
static const char* const kTimbreNames[kOscTypes][4] = {
    { "Position", "Warp", "Uni Width", "Pos Fine" },
    { "Pulse Width", "Sync", "Slop", "Uni Width" },
    { "Morph", "Peak", "Tilt", "Vowel" },
    { "Position", "Hardness", "Decay", "Damping" },
    { "Position", "Size", "Density", "Spray" },
    { "Position", "Blur", "Shift", "Cross" },
    { "FM Index", "Feedback", "Brightness", "Spread" } };
// 5.1.7: Timbre 5..10 - the rest of each engine's continuous knobs (nullptr = no target on that engine)
static const char* const kTimbreXNames[kOscTypes][kTimbreX] = {
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
    { "Osc Drift", nullptr, nullptr, nullptr, nullptr, nullptr },
    { "Stretch", "Odd/Even", "Peak Amount", "Comb Shift", "Shimmer", "Partial Detune" },
    { "Stretch", "Mode Spread", "Coupling", "Buzz / Sympathy", "Exciter Noise", "Nonlinearity" },
    { "Scan", "Pitch Rnd", "Spread", "Reverse", nullptr, nullptr },
    { "Speed", "Stretch", "Tilt", "Formant", "Sparse", nullptr },
    { "Op 1 Level", "Op 2 Level", "Op 3 Level", "Op 4 Level", "Op 5 Level", "Op 6 Level" } };

// FX units, in the default chain order
enum FxUnit { fxDrive, fxPhaser, fxChorus, fxDelay, fxShift, fxTape, fxCrush, fxWidth, fxEq, fxReso, fxSpace, fxSpectral, kFxUnits };
static const char* const kFxUnitNames[kFxUnits] = { "Drive", "Phaser", "Chorus", "Delay", "Freq Shift", "Tape", "Crusher", "Width", "EQ", "Resonator", "Space", "Spectral" };
static const uint32_t kFxOnParam[kFxUnits] = { pDrvOn, pPhOn, pChOn, pDlOn, pFsOn, pTpOn, pCrOn, pWdOn, pEqOn, pRsOn, pSpOn, pSxOn };
static const int kFxDefaultOrder[kFxUnits] = { fxDrive, fxPhaser, fxChorus, fxSpectral, fxReso, fxDelay, fxShift, fxTape, fxCrush, fxWidth, fxEq, fxSpace };

// ------------------------------------------------------------------------------------------------
class ParamRegistry {
public:
    static const ParamRegistry& get() { static ParamRegistry r; return r; }
    const ParamDef& operator[](uint32_t i) const { return defs[i]; }
    uint32_t count() const { return (uint32_t)defs.size(); }
    int findSymbol(const std::string& s) const {
        for (uint32_t i = 0; i < defs.size(); ++i) if (s == defs[i].symbol) return (int)i;
        return -1;
    }

private:
    std::vector<ParamDef> defs;
    std::deque<std::string> strings;   // stable storage for generated names
    const char* keep(const std::string& s) { strings.push_back(s); return strings.back().c_str(); }

    void add(const char* name, const char* sym, const char* unit, float mn, float mx, float df, uint32_t fl,
             const char* const* en = nullptr) {
        defs.push_back({ name, sym, unit, mn, mx, df, fl, en });
    }

    void addSlot(int m) {
        const std::string n = "Mod " + std::to_string(m + 1) + " ", s = "mod" + std::to_string(m + 1);
        add(keep(n + "Source"), keep(s + "Src"), "", 0, kModSrcCount - 1, 0, fInt, kModSrcNames);
        add(keep(n + "Via"), keep(s + "Via"), "", 0, kModSrcCount - 1, 0, fInt, kModSrcNames);
        add(keep(n + "Destination"), keep(s + "Dst"), "", 0, kModDstCount - 1, 0, fInt, kModDstNames);
        add(keep(n + "Amount"), keep(s + "Amt"), "", -1, 1, 0, fNone);
        add(keep(n + "Curve"), keep(s + "Curve"), "", 0, 7, 0, fInt, kCurveNames);
    }
    ParamRegistry() {
        defs.reserve(kParamCount);
        static const char* const L[3] = { "A", "B", "C" };
        static const char* const l[3] = { "a", "b", "c" };
        for (int o = 0; o < kOscs; ++o) {
            auto N = [&](const char* n) { return keep(std::string("Osc ") + L[o] + " " + n); };
            auto S = [&](const char* s) { return keep(std::string("o") + l[o] + s); };
            add(N("On"), S("On"), "", 0, 1, o < 2 ? 1.f : 0.f, fInt | fBool);
            add(N("Engine"), S("Type"), "", 0, kOscTypes - 1, o == 2 ? (float)otWavetable : (float)otAnalog, fInt, kOscTypeNames);
            add(N("Level"), S("Level"), "", 0, 1, 0.7f, fNone);
            add(N("Pan"), S("Pan"), "", -1, 1, 0, fNone);
            add(N("Octave"), S("Oct"), "oct", -3, 3, 0, fInt);
            add(N("Semi"), S("Semi"), "st", -12, 12, 0, fInt);
            add(N("Fine"), S("Fine"), "ct", -50, 50, o == 1 ? 7.f : 0.f, fNone);
            add(N("Voices"), S("Voices"), "", 1, 7, 1, fInt);
            add(N("Detune"), S("Detune"), "ct", 0, 100, 12, fNone);
            add(N("Unison Width"), S("UniWidth"), "", 0, 1, 0.7f, fNone);
            add(N("Phase"), S("Phase"), "", 0, 1, 0, fNone);
            add(N("Retrigger"), S("Retrig"), "", 0, 1, 0, fInt | fBool);
            add(N("FM Source"), S("FmSrc"), "", 0, 5, 0, fInt, kFmSrcNames);
            add(N("FM Mode"), S("FmMode"), "", 0, 5, 0, fInt, kFmModeNames);
            add(N("FM Amount"), S("FmAmt"), "", 0, 1, 0, fNone);
            add(N("Sync Source"), S("SyncSrc"), "", 0, 3, 0, fInt, kSyncSrcNames);
            add(N("Table"), S("Table"), "", 0, 7, 0, fInt, kTableNames);
            add(N("WT Position"), S("Pos"), "", 0, 1, 0.5f, fNone);
            add(N("Warp Mode"), S("Warp"), "", 0, kWarpModes - 1, 0, fInt, kWarpNames);
            add(N("Warp Amount"), S("WarpAmt"), "", 0, 1, 0, fNone);
            add(N("Frame Blend"), S("WtBlend"), "", 0, 2, 0, fInt, kBlendNames);
            add(N("Analog Wave"), S("AnWave"), "", 0, kAnWaves - 1, o == 1 ? 1.f : 0.f, fInt, kAnWaveNames);
            add(N("Pulse Width"), S("AnPW"), "", 0.05f, 0.95f, 0.5f, fNone);
            add(N("Sync"), S("AnSync"), "st", 0, 36, 0, fNone);
            add(N("Slop"), S("AnSlop"), "", 0, 1, 0.25f, fNone);
            add(N("Osc Drift"), S("AnDrift"), "", 0, 1, 0.2f, fNone);
            add(N("Spectrum 1"), S("AdSpec1"), "", 0, kSpectra - 1, 0, fInt, kSpectrumNames);
            add(N("Spectrum 2"), S("AdSpec2"), "", 0, kSpectra - 1, 3, fInt, kSpectrumNames);
            add(N("Morph"), S("AdMorph"), "", 0, 1, 0, fNone);
            add(N("Partials"), S("AdPartials"), "", 1, 128, 48, fInt);
            add(N("Tilt"), S("AdTilt"), "", -1, 1, 0, fNone);
            add(N("Odd/Even"), S("AdOddEven"), "", -1, 1, 0, fNone);
            add(N("Stretch"), S("AdStretch"), "", -1, 1, 0, fNone);
            add(N("Peak"), S("AdPeakPos"), "", 1, 96, 8, fLog);
            add(N("Peak Amount"), S("AdPeakAmt"), "", 0, 1, 0, fNone);
            add(N("Peak Width"), S("AdPeakWidth"), "", 0, 1, 0.3f, fNone);
            add(N("Comb"), S("AdComb"), "", 0, 1, 0, fNone);
            add(N("Comb Shift"), S("AdCombShift"), "", 0, 1, 0, fNone);
            add(N("Shimmer"), S("AdShimmer"), "", 0, 1, 0.2f, fNone);
            add(N("Shimmer Rate"), S("AdShimRate"), "Hz", 0.02f, 5, 0.15f, fLog);
            add(N("Partial Detune"), S("AdDetune"), "ct", 0, 30, 3, fNone);
            add(N("Vowel"), S("AdVowel"), "", 0, 1, 0, fNone);
            add(N("Vowel Amount"), S("AdVowelAmt"), "", 0, 1, 0, fNone);
            add(N("Blur"), S("AdBlur"), "s", 0, 10, 0, fNone);
            add(N("Freeze"), S("AdFreeze"), "", 0, 1, 0, fInt | fBool);
            add(N("Model"), S("PmModel"), "", 0, kPmModels - 1, 0, fInt, kPmModelNames);
            add(N("Exciter"), S("PmExciter"), "", 0, kPmExciters - 1, 1, fInt, kPmExciterNames);
            add(N("Decay"), S("PmDecay"), "s", 0.05f, 30, 5, fLog);
            add(N("Damping"), S("PmDamp"), "", 0, 1, 0.35f, fNone);
            add(N("Hardness"), S("PmHard"), "", 0, 1, 0.5f, fNone);
            add(N("Position"), S("PmPos"), "", 0, 1, 0.3f, fNone);
            add(N("Stretch "), S("PmStretch"), "", -1, 1, 0, fNone);
            add(N("Mode Spread"), S("PmSpread"), "", 0, 1, 0.5f, fNone);
            add(N("Coupling"), S("PmCouple"), "", 0, 1, 0, fNone);
            add(N("Buzz / Sympathy"), S("PmBuzz"), "", 0, 1, 0.4f, fNone);
            add(N("Grain Source"), S("GrSource"), "", 0, 2, 0, fInt, kGrSourceNames);
            add(N("Grain Position"), S("GrPos"), "", 0, 1, 0.3f, fNone);
            add(N("Grain Scan"), S("GrScan"), "", -1, 1, 0, fNone);
            add(N("Grain Size"), S("GrSize"), "s", 0.01f, 1.5f, 0.18f, fLog);
            add(N("Grain Density"), S("GrDensity"), "Hz", 1, 200, 24, fLog);
            add(N("Grain Spray"), S("GrSpray"), "", 0, 1, 0.1f, fNone);
            add(N("Grain Pitch Rnd"), S("GrPitchRand"), "ct", 0, 1200, 0, fNone);
            add(N("Grain Shape"), S("GrShape"), "", 0, 1, 0.5f, fNone);
            add(N("Grain Spread"), S("GrSpread"), "", 0, 1, 0.6f, fNone);
            add(N("Grain Reverse"), S("GrReverse"), "", 0, 1, 0, fNone);
        }
        add("Sub Level", "subLevel", "", 0, 1, 0, fNone);
        add("Sub Wave", "subWave", "", 0, 3, 0, fInt, kSubWaveNames);
        add("Sub Octave", "subOct", "oct", -2, -1, -1, fInt);
        add("Noise Level", "noiseLevel", "", 0, 1, 0, fNone);
        add("Noise Color", "noiseColor", "", 0, 1, 0.3f, fNone);
        add("Ring Level", "ringLevel", "", 0, 1, 0, fNone);
        add("Ring Source", "ringSource", "", 0, 3, 0, fInt, kRingNames);
        for (int f = 0; f < 2; ++f) {
            const std::string pre = f == 0 ? "Filter 1 " : "Filter 2 ";
            const std::string sp = f == 0 ? "f1" : "f2";
            auto N = [&](const char* n) { return keep(pre + n); };
            auto S = [&](const char* s) { return keep(sp + s); };
            add(N("Type"), S("Type"), "", 0, kFilterTypes - 1, f == 0 ? 1.f : 0.f, fInt, kFilterNames);
            add(N("Cutoff"), S("Cutoff"), "Hz", 20, 20000, f == 0 ? 1200.f : 3000.f, fLog);
            add(N("Resonance"), S("Res"), "", 0, 1, 0.05f, fNone);     // 5.1.2: clean by default (was 0.15)
            add(N("Drive"), S("Drive"), "", 0, 1, 0.f, fNone);          // 5.1.2: no drive by default (filter 1 was 0.1)
            add(N("Drive Type"), S("DriveType"), "", 0, 3, 0, fInt, kDriveTypeNames);
            add(N("Key Track"), S("Key"), "", 0, 1, 0.3f, fNone);
            add(N("Env Amount"), S("Env"), "", -1, 1, f == 0 ? 0.3f : 0.f, fNone);
            add(N("Shape"), S("Shape"), "", 0, 1, 0.5f, fNone);
            add(N("Morph Type"), S("Type2"), "", 0, kFilterTypes - 1, 0, fInt, kFilterNames);
            add(N("Type Morph"), S("Morph"), "", 0, 1, 0, fNone);
            add(N("FM Source"), S("FmSrc"), "", 0, 4, 0, fInt, kFiltFmNames);
            add(N("FM Amount"), S("FmAmt"), "", 0, 1, 0, fNone);
        }
        add("Filter Routing", "fRouting", "", 0, 2, 0, fInt, kRoutingNames);
        add("Filter Balance", "fBalance", "", 0, 1, 0.5f, fNone);
        static const char* const envN[3] = { "Filter Env ", "Amp Env ", "Mod Env " };
        static const char* const envS[3] = { "fe", "ae", "me" };
        const float envDef[3][4] = { { 2.f, 4.f, 0.4f, 6.f }, { 1.5f, 3.f, 0.9f, 6.f }, { 3.f, 6.f, 0.5f, 8.f } };
        for (int e = 0; e < 3; ++e) {
            add(keep(std::string(envN[e]) + "Attack"), keep(std::string(envS[e]) + "A"), "s", 0.001f, 20, envDef[e][0], fLog);
            add(keep(std::string(envN[e]) + "Decay"), keep(std::string(envS[e]) + "D"), "s", 0.001f, 30, envDef[e][1], fLog);
            add(keep(std::string(envN[e]) + "Sustain"), keep(std::string(envS[e]) + "S"), "", 0, 1, envDef[e][2], fNone);
            add(keep(std::string(envN[e]) + "Release"), keep(std::string(envS[e]) + "R"), "s", 0.001f, 30, envDef[e][3], fLog);
        }
        const float lfoRate[kLfos] = { 0.07f, 0.13f, 0.21f, 0.033f, 0.5f, 1.2f };
        const float lfoWave[kLfos] = { 0, 0, 1, 6, 0, 1 };
        const float lfoMode[kLfos] = { 0, 1, 1, 0, 0, 1 };
        for (int i = 0; i < kLfos; ++i) {
            const std::string n = "LFO " + std::to_string(i + 1) + " ", s = "lfo" + std::to_string(i + 1);
            add(keep(n + "Rate"), keep(s + "Rate"), "Hz", 0.005f, 30, lfoRate[i], fLog);
            add(keep(n + "Shape"), keep(s + "Wave"), "", 0, kLfoWaves - 1, lfoWave[i], fInt, kLfoWaveNames);
            add(keep(n + "Mode"), keep(s + "Mode"), "", 0, 3, lfoMode[i], fInt, kLfoModeNames);
            add(keep(n + "Fade In"), keep(s + "Fade"), "s", 0, 20, 0, fNone);
            add(keep(n + "Tempo Sync"), keep(s + "Sync"), "", 0, 1, 0, fInt | fBool);
            add(keep(n + "Division"), keep(s + "Div"), "", 0, kDivs - 1, 7, fInt, kDivNames);
            add(keep(n + "Phase"), keep(s + "Phase"), "", 0, 1, 0, fNone);
            add(keep(n + "Smooth"), keep(s + "Smooth"), "", 0, 1, 0, fNone);
        }
        for (int m = 0; m < 2; ++m) {
            const std::string n = "MSEG " + std::to_string(m + 1) + " ", s = "mseg" + std::to_string(m + 1);
            add(keep(n + "Mode"), keep(s + "Mode"), "", 0, 2, m == 0 ? 0.f : 2.f, fInt, kMsegModeNames);
            add(keep(n + "Time Scale"), keep(s + "Scale"), "x", 0.05f, 20, 1, fLog);
            add(keep(n + "Tempo Sync"), keep(s + "Sync"), "", 0, 1, 0, fInt | fBool);
        }
        add("Drift Global Rate", "rndGRate", "Hz", 0.01f, 10, 0.1f, fLog);
        add("Drift Voice Rate", "rndVRate", "Hz", 0.01f, 10, 0.2f, fLog);
        for (int m = 0; m < kMacros; ++m)
            add(keep("Macro " + std::to_string(m + 1)), keep("macro" + std::to_string(m + 1)), "", 0, 1, 0, fNone);
        for (int m = 0; m < kModSlots1; ++m) addSlot(m);
        add("MPE Bend Range", "bendRange", "st", 1, 96, 48, fInt);
        add("Pressure > Cutoff", "pressCut", "", 0, 1, 0.4f, fNone);
        add("Pressure > Amp", "pressAmp", "", 0, 1, 0.3f, fNone);
        add("Slide > Cutoff", "slideCut", "", 0, 1, 0.3f, fNone);
        add("Slide > Timbre", "slideTimbre", "", 0, 1, 0.4f, fNone);
        add("Velocity > Amp", "velAmp", "", 0, 1, 0.3f, fNone);
        add("ModWheel > Cutoff", "mwCut", "", 0, 1, 0.5f, fNone);
        add("Voice Mode", "polyMode", "", 0, 2, 0, fInt, kPolyNames);
        add("Glide", "glide", "s", 0.001f, 5, 0.001f, fLog);
        add("Analog Drift", "drift", "", 0, 1, 0.15f, fNone);
        add("Stereo Spread", "spread", "", 0, 1, 0.7f, fNone);
        add("Master", "master", "dB", -36, 24, -6, fNone);   // 5.1.2: +24 (clean presets have no drive gain)
        add("Quality", "quality", "", 0, 2, 2, fInt, kQualityNames);     // 5.1.1: HQ by default
        add("Polyphony", "polyphony", "", 1, 24, 24, fInt);              // 5.1.1: 24 voices by default
        add("Arp On", "arpOn", "", 0, 1, 0, fInt | fBool);
        add("Arp Mode", "arpMode", "", 0, 6, 0, fInt, kArpModeNames);
        add("Arp Octaves", "arpOct", "", 1, 4, 1, fInt);
        add("Arp Rate", "arpRate", "", 0, kDivs - 1, 13, fInt, kDivNames);
        add("Arp Gate", "arpGate", "", 0.05f, 1, 0.6f, fNone);
        add("Arp Swing", "arpSwing", "", 0, 0.75f, 0, fNone);
        add("Arp Hold", "arpHold", "", 0, 1, 0, fInt | fBool);
        add("Seq Mode", "seqMode", "", 0, 2, 0, fInt, kSeqModeNames);
        add("Seq Rate", "seqRate", "", 0, kDivs - 1, 13, fInt, kDivNames);
        add("Seq Steps", "seqSteps", "", 1, kSeqSteps, 16, fInt);
        add("Seq Gate", "seqGate", "", 0.05f, 1, 0.5f, fNone);
        add("Seq Swing", "seqSwing", "", 0, 0.75f, 0, fNone);
        add("Seq Glide", "seqGlide", "", 0, 1, 0, fNone);
        for (int s = 0; s < kSeqSteps; ++s)
            add(keep("Seq Step " + std::to_string(s + 1)), keep("seqV" + std::to_string(s + 1)), "", -1, 1, 0, fNone);
        for (int s = 0; s < kSeqSteps; ++s)
            add(keep("Seq Gate " + std::to_string(s + 1)), keep("seqG" + std::to_string(s + 1)), "", 0, 1, 1, fInt | fBool);
        // FX
        add("Drive On", "drvOn", "", 0, 1, 0, fInt | fBool);
        add("Drive", "drvAmt", "", 0, 1, 0.2f, fNone);
        add("Drive Tone", "drvTone", "", 0, 1, 0.5f, fNone);
        add("Phaser On", "phOn", "", 0, 1, 0, fInt | fBool);
        add("Phaser Mix", "phMix", "", 0, 1, 0.5f, fNone);
        add("Phaser Rate", "phRate", "Hz", 0.02f, 5, 0.15f, fLog);
        add("Phaser Depth", "phDepth", "", 0, 1, 0.7f, fNone);
        add("Phaser Feedback", "phFb", "", 0, 0.95f, 0.5f, fNone);
        add("Chorus On", "chOn", "", 0, 1, 1, fInt | fBool);
        add("Chorus Mode", "chMode", "", 0, 3, 0, fInt, kChorusNames);
        add("Chorus Mix", "chMix", "", 0, 1, 0.3f, fNone);
        add("Chorus Rate", "chRate", "Hz", 0.03f, 8, 0.3f, fLog);
        add("Chorus Depth", "chDepth", "", 0, 1, 0.5f, fNone);
        add("Chorus Feedback", "chFb", "", -0.9f, 0.9f, 0, fNone);
        add("Delay On", "dlOn", "", 0, 1, 0, fInt | fBool);
        add("Delay Mix", "dlMix", "", 0, 1, 0.25f, fNone);
        add("Delay Time", "dlTime", "s", 0.01f, 2, 0.38f, fLog);
        add("Delay Feedback", "dlFb", "", 0, 0.95f, 0.4f, fNone);
        add("Delay Mode", "dlMode", "", 0, 1, 1, fInt, kDelayNames);
        add("Delay Damping", "dlDamp", "", 0, 1, 0.4f, fNone);
        add("Delay Sync", "dlSync", "", 0, 1, 0, fInt | fBool);
        add("Delay Division", "dlDiv", "", 0, kDivs - 1, 9, fInt, kDivNames);
        add("Shifter On", "fsOn", "", 0, 1, 0, fInt | fBool);
        add("Shift", "fsShift", "Hz", -500, 500, 2, fNone);
        add("Shifter Mix", "fsMix", "", 0, 1, 0.5f, fNone);
        add("Shifter Feedback", "fsFb", "", 0, 0.95f, 0.3f, fNone);
        add("Shifter Stereo", "fsStereo", "", 0, 1, 0.5f, fNone);
        add("Tape On", "tpOn", "", 0, 1, 0, fInt | fBool);
        add("Tape Drive", "tpDrive", "", 0, 1, 0.f, fNone);          // 5.1.2: no saturation by default (was 0.3)
        add("Tape Wow", "tpWow", "", 0, 1, 0.3f, fNone);
        add("Tape Flutter", "tpFlutter", "", 0, 1, 0.2f, fNone);
        add("Tape Tone", "tpTone", "", 0, 1, 0.6f, fNone);
        add("Tape Mix", "tpMix", "", 0, 1, 1, fNone);
        add("Crusher On", "crOn", "", 0, 1, 0, fInt | fBool);
        add("Crusher Bits", "crBits", "", 1, 16, 10, fNone);
        add("Crusher Downsample", "crDown", "x", 1, 50, 4, fLog);
        add("Crusher Mix", "crMix", "", 0, 1, 0.5f, fNone);
        add("Width On", "wdOn", "", 0, 1, 0, fInt | fBool);
        add("Width", "wdWidth", "", 0, 2, 1.3f, fNone);
        add("Bass Mono", "wdBassMono", "Hz", 20, 500, 120, fLog);
        add("EQ On", "eqOn", "", 0, 1, 0, fInt | fBool);
        add("EQ Low Gain", "eqLowG", "dB", -15, 15, 0, fNone);
        add("EQ Low Freq", "eqLowF", "Hz", 30, 600, 120, fLog);
        add("EQ Mid Gain", "eqMidG", "dB", -15, 15, 0, fNone);
        add("EQ Mid Freq", "eqMidF", "Hz", 150, 8000, 1000, fLog);
        add("EQ Mid Q", "eqMidQ", "", 0.2f, 8, 0.8f, fLog);
        add("EQ High Gain", "eqHighG", "dB", -15, 15, 0, fNone);
        add("EQ High Freq", "eqHighF", "Hz", 1500, 18000, 6000, fLog);
        add("Filter Env Attack Curve", "fEnvCurve", "", -1, 1, 0, fNone);
        add("Amp Env Attack Curve", "aEnvCurve", "", -1, 1, 0, fNone);
        add("Mod Env Attack Curve", "mEnvCurve", "", -1, 1, 0, fNone);
        add("Tuning Source", "tnSource", "", 0, 3, 0, fInt, kTnSourceNames);
        add("Tuning Scale", "tnScale", "", 0, drift::kBuiltinScales - 1, 0, fInt, drift::builtinScaleNames());
        add("Tuning Root", "tnRoot", "", 0, 11, 0, fInt, kTnRootNames);
        add("Tuning Reference A4", "tnRef", "Hz", 400, 480, 440, fNone);
        add("Tuning Amount", "tnAmount", "", 0, 1, 1, fNone);
        add("Tuning Snap to Scale", "tnSnap", "", 0, 1, 0, fInt | fBool);
        add("Tuning Retune Glide", "tnGlide", "s", 0, 4, 0.08f, fNone);
        add("Note Expr Vibrato Depth", "neVibDepth", "st", 0, 2, 0.5f, fNone);
        add("Note Expr Vibrato Rate", "neVibRate", "Hz", 0.5f, 12, 5.5f, fNone);
        // 4.2
        for (int o = 0; o < kOscs; ++o) {
            auto N = [&](const char* n) { return keep(std::string("Osc ") + L[o] + " " + n); };
            auto S = [&](const char* s) { return keep(std::string("o") + l[o] + s); };
            add(N("Pressure"), S("PmPress"), "", 0, 1, 0.5f, fNone);
            add(N("Exciter Noise"), S("PmNoise"), "", 0, 1, 0, fNone);
            add(N("Nonlinearity"), S("PmNonlin"), "", 0, 1, 0, fNone);
            add(N("Spectral Position"), S("SpPos"), "", 0, 1, 0, fNone);
            add(N("Spectral Speed"), S("SpSpeed"), "x", -2, 2, 1, fNone);
            add(N("Spectral Freeze"), S("SpFreeze"), "", 0, 1, 0, fInt | fBool);
            add(N("Spectral Blur"), S("SpBlur"), "s", 0, 20, 0, fNone);
            add(N("Spectral Shift"), S("SpShift"), "Hz", -500, 500, 0, fNone);
            add(N("Spectral Stretch"), S("SpStretch"), "", -1, 1, 0, fNone);
            add(N("Spectral Tilt"), S("SpTilt"), "", -1, 1, 0, fNone);
            add(N("Spectral Formant"), S("SpFormant"), "", 0, 1, 0, fNone);
            add(N("Spectral Sparse"), S("SpSparse"), "", 0, 1, 0, fNone);
            add(N("Spectral Cross"), S("SpCross"), "", 0, 1, 0.5f, fNone);
            add(N("Spectral Cross Mode"), S("SpCrossMode"), "", 0, 2, 0, fInt, kSpCrossNames);
            add(N("Spectral Keyed"), S("SpKeyed"), "", 0, 1, 1, fInt | fBool);
            add(N("Phase Random"), S("PhaseRnd"), "", 0, 1, 0, fNone);                  // 6.0 (was a reserve)
            for (int k = kOsc2Used; k < (int)kOsc2Fields; ++k)
                add(N(("Reserved " + std::to_string(k)).c_str()), S(("Rsv" + std::to_string(k)).c_str()), "", 0, 1, 0, fHidden);
        }
        for (int m = 2; m < kMsegs; ++m) {
            const std::string n = "MSEG " + std::to_string(m + 1) + " ", s = "mseg" + std::to_string(m + 1);
            add(keep(n + "Mode"), keep(s + "Mode"), "", 0, 2, m == 2 ? 1.f : 2.f, fInt, kMsegModeNames);
            add(keep(n + "Time Scale"), keep(s + "Scale"), "x", 0.05f, 20, 1, fLog);
            add(keep(n + "Tempo Sync"), keep(s + "Sync"), "", 0, 1, 0, fInt | fBool);
        }
        for (int m = kModSlots1; m < kModSlots; ++m) addSlot(m);
        add("Resonator On", "rsOn", "", 0, 1, 0, fInt | fBool);
        add("Resonator Mix", "rsMix", "", 0, 1, 0.35f, fNone);
        add("Resonator Decay", "rsDecay", "s", 0.2f, 30, 4, fLog);
        add("Resonator Brightness", "rsBright", "", 0, 1, 0.6f, fNone);
        add("Resonator Tuning", "rsMode", "", 0, 5, 0, fInt, kRsModeNames);
        add("Resonator Root", "rsRoot", "", 0, 11, 0, fInt, kTnRootNames);
        add("Resonator Octave", "rsOct", "", 1, 5, 2, fInt, kRsOctNames);
        add("Resonator Width", "rsWidth", "", 0, 1, 0.7f, fNone);
        add("Space On", "spOn", "", 0, 1, 0, fInt | fBool);
        add("Space Mix", "spMix", "", 0, 1, 0.3f, fNone);
        add("Space Size", "spSize", "", 0, 1, 0.6f, fNone);
        add("Space Decay", "spDecay", "s", 0.3f, 60, 6, fLog);
        add("Space Damping", "spDamp", "", 0, 1, 0.5f, fNone);
        add("Space Pre-delay", "spPre", "s", 0, 0.25f, 0.02f, fNone);
        add("Space Modulation", "spMod", "", 0, 1, 0.3f, fNone);
        add("Space Width", "spWidth", "", 0, 1, 1, fNone);
        add("Scene Morph On", "scOn", "", 0, 1, 0, fInt | fBool);
        add("Scene Morph", "scPos", "", 0, kScenes - 1, 0, fNone);
        add("Scene Glide", "scGlide", "s", 0, 10, 0.5f, fNone);
        add("Motion Playback", "moMode", "", 0, 3, 0, fInt, kMoModeNames);
        add("Motion Speed", "moSpeed", "x", 0.25f, 4, 1, fLog);
        add("Spectral FX On", "sxOn", "", 0, 1, 0, fInt | fBool);
        add("Spectral FX Mix", "sxMix", "", 0, 1, 0.5f, fNone);
        add("Spectral FX Freeze", "sxFreeze", "", 0, 1, 0, fInt | fBool);
        add("Spectral FX Blur", "sxBlur", "s", 0, 30, 2, fNone);
        add("Spectral FX Shift", "sxShift", "st", -24, 24, 0, fNone);
        add("Spectral FX Tilt", "sxTilt", "", -1, 1, 0, fNone);
        add("Spectral FX Spread", "sxSpread", "", 0, 1, 0.3f, fNone);
        add("Spectral FX Gate", "sxGate", "", 0, 1, 0, fNone);
        for (int pd = 0; pd < 2; ++pd) {   // 5.1.1 XY pad auto-motion
            const std::string n = "XY" + std::to_string(pd + 1), s = "xy" + std::to_string(pd + 1);
            add(keep(n + " Motion"), keep(s + "Mo"), "", 0, kXyShapes - 1, 0, fInt, kXyShapeNames);
            add(keep(n + " Rate"), keep(s + "Rate"), "Hz", 0.005f, 2.f, 0.05f, fLog);
            add(keep(n + " Sync"), keep(s + "Sync"), "", 0, 1, 0, fInt | fBool);
            add(keep(n + " Division"), keep(s + "Div"), "", 0, kDivs - 1, 3, fInt, kDivNames);   // 1 bar
            add(keep(n + " Size"), keep(s + "Size"), "", 0, 1, 0.5f, fNone);
            add(keep(n + " Direction"), keep(s + "Dir"), "", 0, 1, 0, fInt | fBool);
        }
        // 5.1.3: release curves. 0 = the exponential release of every earlier version (a straight line in dB);
        // > 0 drops fast then lingers, < 0 holds the level and falls away at the end. Same release time either way.
        add("Filter Env Release Curve", "fRelCurve", "", -1, 1, 0, fNone);
        add("Amp Env Release Curve", "aRelCurve", "", -1, 1, 0, fNone);
        add("Mod Env Release Curve", "mRelCurve", "", -1, 1, 0, fNone);
        // 5.1.6: tape wow / flutter speeds (defaults = the fixed speeds of every earlier version) and a
        // layer pitch offset (a modulatable PITCH knob; 0 = unchanged)
        add("Tape Wow Speed", "tpWowRate", "Hz", 0.05f, 4.f, 0.45f, fLog);
        add("Tape Flutter Speed", "tpFlutRate", "Hz", 2.f, 30.f, 7.3f, fLog);
        add("Pitch", "pitch", "st", -24, 24, 0, fNone);
        // 6.0 (were reserves). Decay curves: 0 = the exponential decay of every earlier version, < 0 holds the
        // level longer (slow pads), > 0 drops faster then lingers. Same decay time either way.
        add("Filter Env Decay Curve", "fDecCurve", "", -1, 1, 0, fNone);
        add("Amp Env Decay Curve", "aDecCurve", "", -1, 1, 0, fNone);
        add("Mod Env Decay Curve", "mDecCurve", "", -1, 1, 0, fNone);
        for (int i = 0; i < kLfos; ++i)
            add(keep("LFO " + std::to_string(i + 1) + " Random Phase"), keep("lfo" + std::to_string(i + 1) + "RndPh"), "", 0, 1, 0, fNone);
        // 0 for sessions and presets made before 6.0 (they keep their levels); INIT and the 6.0 bank turn it on
        add("Level Compensation", "levelComp", "", 0, 1, 0, fInt | fBool);
        // 0 for everything made before 6.0, where PM / FM did nothing on Additive, Physical and Spectral (their
        // dormant settings stay silent); the editor sets an oscillator's bit when its FM controls are touched
        add("FM 6.0 Oscillators", "fmV6", "", 0, 7, 0, fInt | fHidden);
        for (int k = pRsv1; k <= (int)pRsvLast; ++k)
            add(keep("Reserved " + std::to_string(k - pRsv1 + 1)), keep("rsv" + std::to_string(k - pRsv1 + 1)), "", 0, 1, 0, fHidden);
        // layer setup
        add("Layer Mode", "lyMode", "", 0, 5, 0, fInt, kLyModeNames);
        add("Layer Split Note", "lySplit", "", 0, 127, 60, fInt);
        add("Layer Crossfade Width", "lyWidth", "st", 0, 48, 6, fNone);
        add("Layer Morph", "lyMorph", "", 0, 1, 0.5f, fNone);
        for (int k = pGRsv1; k <= (int)pGRsvLast; ++k)
            add(keep("Global Reserved " + std::to_string(k - pGRsv1 + 1)), keep("grsv" + std::to_string(k - pGRsv1 + 1)), "", 0, 1, 0, fHidden);
        // layer 2: a copy of every layer parameter
        const size_t n1 = kLayerParams;
        for (size_t i = 0; i < n1; ++i) {
            const ParamDef d = defs[i];
            add(keep(std::string("L2 ") + d.name), keep(std::string("b_") + d.symbol), d.unit, d.min, d.max, d.def, d.flags, d.enumNames);
        }
        // 6.0 extension block (layer 1), then its layer-2 copy
        const size_t x0 = defs.size();
        static const char* const FL[3] = { "A", "B", "C" };
        static const char* const fl[3] = { "a", "b", "c" };
        for (int o = 0; o < kOscs; ++o) {
            auto N = [&](const std::string& n) { return keep(std::string("Osc ") + FL[o] + " FM " + n); };
            auto S = [&](const std::string& s) { return keep(std::string("o") + fl[o] + "Fm" + s); };
            add(N("Algorithm"), S("Algo"), "", 0, kFmAlgos - 1, 4, fInt, kFmAlgoNames);
            add(N("Feedback"), S("Feedback"), "", 0, 1, 0, fNone);
            add(N("Index"), S("Index"), "x", 0, 2, 1, fNone);
            add(N("Brightness"), S("Bright"), "", -1, 1, 0, fNone);
            add(N("Spread"), S("Spread"), "", 0, 1, 0, fNone);
            add(N("Env Time"), S("EnvTime"), "x", 0.1f, 10, 1, fLog);
            add(N("Velocity to Index"), S("VelIdx"), "", 0, 1, 0.3f, fNone);
            add(N("Key to Index"), S("KeyIdx"), "", -1, 1, 0, fNone);
            for (int k = 0; k < kFmOps; ++k) {
                const std::string on = "Op " + std::to_string(k + 1) + " ", os = std::to_string(k + 1);
                // init patch: algorithm 5 (three 2-op pairs); op 1 carrier at full level, op 2 its modulator
                const float lvl = k == 0 ? 1.f : (k == 1 ? 0.45f : 0.f);
                add(N(on + "Ratio"), S(os + "Coarse"), "", 0, 32, 1, fInt);
                add(N(on + "Fine"), S(os + "Fine"), "", 0, 0.99f, 0, fNone);
                add(N(on + "Detune"), S(os + "Detune"), "ct", -50, 50, 0, fNone);
                add(N(on + "Fixed"), S(os + "Fixed"), "", 0, 1, 0, fInt | fBool);
                add(N(on + "Fixed Freq"), S(os + "Hz"), "Hz", 1, 9000, 440, fLog);
                add(N(on + "Level"), S(os + "Level"), "", 0, 1, lvl, fNone);
                add(N(on + "Wave"), S(os + "Wave"), "", 0, kFmWaves - 1, 0, fInt, kFmWaveNames);
                add(N(on + "Attack"), S(os + "Atk"), "s", 0.001f, 20, 0.005f, fLog);
                add(N(on + "Decay"), S(os + "Dec"), "s", 0.005f, 30, 2.f, fLog);
                add(N(on + "Sustain"), S(os + "Sus"), "", 0, 1, 0.8f, fNone);
                add(N(on + "Release"), S(os + "Rel"), "s", 0.005f, 30, 1.f, fLog);
                add(N(on + "Velocity"), S(os + "Vel"), "", 0, 1, 0.f, fNone);
                add(N(on + "Key Scale"), S(os + "Key"), "", -1, 1, 0, fNone);
            }
        }
        for (uint32_t k = kExtUsed; k < kExtParams; ++k)
            add(keep("Ext Reserved " + std::to_string(k - kExtUsed + 1)), keep("xrsv" + std::to_string(k - kExtUsed + 1)), "", 0, 1, 0, fHidden);
        const size_t x1 = defs.size();
        for (size_t i = x0; i < x1; ++i) {
            const ParamDef d = defs[i];
            add(keep(std::string("L2 ") + d.name), keep(std::string("b_") + d.symbol), d.unit, d.min, d.max, d.def, d.flags, d.enumNames);
        }
    }
};

static inline const ParamDef& P(uint32_t i) { return ParamRegistry::get()[i]; }
static inline float defaultValue(uint32_t i) { return P(i).def; }

static inline const char* enumNameOf(uint32_t p, float v) {
    const ParamDef& d = P(p);
    if (!d.enumNames) return nullptr;
    int i = (int)(v - d.min + 0.5f);
    const int n = (int)(d.max - d.min) + 1;
    if (i < 0) i = 0;
    if (i >= n) i = n - 1;
    return d.enumNames[i];
}

// ------------------------------------------------------------------------------------------------
// Non-parameter state (strings). Kept in one list so plugin, presets and UI agree.
enum StateKey {
    skWtA, skWtB, skWtC,            // "lib:..." or absolute path of a wavetable file
    skSpA, skSpB, skSpC,            // custom additive spectrum (up to 128 values 0..1000)
    skSmA, skSmB, skSmC,            // granular sample file
    skLfo1, skLfo2, skLfo3, skLfo4, skLfo5, skLfo6,   // custom LFO shapes (64 values -1000..1000)
    skMseg1, skMseg2,               // MSEG points
    skFxOrder,                      // FX chain order, e.g. "0,1,2,3,4,5,6,7,8"
    skMacroNames,                   // macro names separated by '|'
    skScl, skKbm,                   // 4.0: Scala scale / keyboard mapping text (tuning source "Scala")
    skMseg3, skMseg4,               // 4.2: MSEG 3 / 4 points
    skModesA, skModesB, skModesC,   // 4.2: analysed modal bank per osc ("ratio,amp,decay;...")
    skScenes,                       // 4.2: 8 macro snapshots
    skMotion,                       // 4.2: recorded macro motion lanes
    skPerf,                         // 4.2: PERFORM page layout (XY pad assignments)
    skSrcA, skSrcB, skSrcC,         // 5.0: spectral source recording per osc
    skXsrcA, skXsrcB, skXsrcC,      // 5.0: spectral cross-synthesis recording per osc
    skSp2A, skSp2B, skSp2C,         // 6.0: second custom additive spectrum per osc ("Custom 2")
    kLayerStates,                   // 4.1: layer 2 states follow, keys "b_" + key
    kStateCount = kLayerStates * 2 };
static const char* const kStateKeys[kStateCount] = {
    "wtA", "wtB", "wtC", "spA", "spB", "spC", "smA", "smB", "smC", "lfo1", "lfo2", "lfo3", "lfo4", "lfo5", "lfo6", "mseg1", "mseg2", "fxOrder", "macroNames", "scl", "kbm", "mseg3", "mseg4", "modesA", "modesB", "modesC", "scenes", "motion", "perf", "srcA", "srcB", "srcC", "xsrcA", "xsrcB", "xsrcC", "sp2A", "sp2B", "sp2C",
    "b_wtA", "b_wtB", "b_wtC", "b_spA", "b_spB", "b_spC", "b_smA", "b_smB", "b_smC", "b_lfo1", "b_lfo2", "b_lfo3", "b_lfo4", "b_lfo5", "b_lfo6", "b_mseg1", "b_mseg2", "b_fxOrder", "b_macroNames", "b_scl", "b_kbm", "b_mseg3", "b_mseg4", "b_modesA", "b_modesB", "b_modesC", "b_scenes", "b_motion", "b_perf", "b_srcA", "b_srcB", "b_srcC", "b_xsrcA", "b_xsrcB", "b_xsrcC", "b_sp2A", "b_sp2B", "b_sp2C" };
static const char* const kStateLabels[kStateCount] = {
    "Osc A Wavetable", "Osc B Wavetable", "Osc C Wavetable", "Osc A Custom Spectrum", "Osc B Custom Spectrum", "Osc C Custom Spectrum", "Osc A Grain Sample", "Osc B Grain Sample", "Osc C Grain Sample", "LFO 1 Shape", "LFO 2 Shape", "LFO 3 Shape", "LFO 4 Shape", "LFO 5 Shape", "LFO 6 Shape", "MSEG 1", "MSEG 2", "FX Order", "Macro Names", "Scala Scale", "Scala Keyboard Map", "MSEG 3", "MSEG 4", "Osc A Analysed Modes", "Osc B Analysed Modes", "Osc C Analysed Modes", "Scenes", "Motion", "Perform Layout", "Osc A Spectral Source", "Osc B Spectral Source", "Osc C Spectral Source", "Osc A Spectral Cross", "Osc B Spectral Cross", "Osc C Spectral Cross", "Osc A Custom Spectrum 2", "Osc B Custom Spectrum 2", "Osc C Custom Spectrum 2",
    "L2 Osc A Wavetable", "L2 Osc B Wavetable", "L2 Osc C Wavetable", "L2 Osc A Custom Spectrum", "L2 Osc B Custom Spectrum", "L2 Osc C Custom Spectrum", "L2 Osc A Grain Sample", "L2 Osc B Grain Sample", "L2 Osc C Grain Sample", "L2 LFO 1 Shape", "L2 LFO 2 Shape", "L2 LFO 3 Shape", "L2 LFO 4 Shape", "L2 LFO 5 Shape", "L2 LFO 6 Shape", "L2 MSEG 1", "L2 MSEG 2", "L2 FX Order", "L2 Macro Names", "L2 Scala Scale", "L2 Scala Keyboard Map", "L2 MSEG 3", "L2 MSEG 4", "L2 Osc A Analysed Modes", "L2 Osc B Analysed Modes", "L2 Osc C Analysed Modes", "L2 Scenes", "L2 Motion", "L2 Perform Layout", "L2 Osc A Spectral Source", "L2 Osc B Spectral Source", "L2 Osc C Spectral Source", "L2 Osc A Spectral Cross", "L2 Osc B Spectral Cross", "L2 Osc C Spectral Cross", "L2 Osc A Custom Spectrum 2", "L2 Osc B Custom Spectrum 2", "L2 Osc C Custom Spectrum 2" };
static inline int stateIndexOf(const char* key) {
    if (!key) return -1;
    for (int k = 0; k < kStateCount; ++k) { const char* a = kStateKeys[k]; const char* b = key; while (*a && *a == *b) { ++a; ++b; } if (!*a && !*b) return k; }
    return -1;
}
