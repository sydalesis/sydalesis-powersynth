// Driftfield 4 — plugin interface (DPF / NanoVG).
// Pages: MAIN (oscillators, mixer, filters, envelopes, macros, live output), MOD (LFOs, MSEGs, random,
// 24-slot matrix), FX (nine reorderable units), SEQ (arpeggiator, step sequencer).
// Live modulation is read from the engine through DPF direct access (Shared.hpp telemetry).
#include "DistrhoUI.hpp"
#include "Params.hpp"
#include "Wavetables.hpp"
#include "Filters.hpp"
#include "Spectra.hpp"
#include "Library.hpp"
#include "Shared.hpp"
#include "Analysis.hpp"
#include "SpecData.hpp"
#include "FmAlgos.hpp"
// 6.0: open a web page in the default browser (Platform.cpp)
void driftPlatformOpenUrl(const char* url);
void driftPlatformTextFocus(uintptr_t nativeView, bool take);
struct DriftPacer;   // 6.0.4: Windows editor frame pacer (Platform.cpp)
DriftPacer* driftPlatformPacerStart(uintptr_t nativeView, int intervalMs, void (*cb)(void*), void* ctx);
void driftPlatformPacerInterval(DriftPacer* p, int intervalMs);
void driftPlatformPacerStop(DriftPacer* p);
static void openUrl(const char* url) { driftPlatformOpenUrl(url); }
static const char* const kBandcampUrl = "https://sydalesis.bandcamp.com";
static const char* const kItchUrl = "https://sydalesis.itch.io/";
#include "FontInter.hpp"
#include <ctime>
#include <array>
#include <memory>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <set>
#include <functional>
#include <cctype>
#include <cstring>
#include <unordered_map>

extern "C" int nvgAddFallbackFontId(NVGcontext* ctx, int baseFont, int fallbackFont);

START_NAMESPACE_DISTRHO

#include "Presets.hpp"

namespace {

using drift::MultiFilter; using drift::FilterCoefs; using drift::filterControlCoefs;
using drift::ftOff; using drift::ftFormant; using drift::ftVowel; using drift::ftDiodeLP24; using drift::ftSvfLP12; using drift::ftSvfBP24;
using drift::ftMs20LP; using drift::ftObHP; using drift::ftXpLP4; using drift::ftXpPH3LP1; using drift::ftDualBP; using drift::ftFlgFirst; using drift::ftFlgLast;

constexpr float kW = 1600.f, kH = 1000.f;          // page area
constexpr float kKbH = 104.f, kHT = kH + kKbH;     // 5.1.1: keyboard strip under every page; kHT = whole window
constexpr float kPi = 3.14159265f;

// ------------------------------------------------------------------ skins
// Every colour the UI draws comes from these tokens. Surfaces that used to be fixed dark greys are
// expressed with shade(v): v is the old 0..255 grey level, mapped between the skin's background
// (v = 10) and panel edge (v = 36) so light skins invert correctly.
struct Rgb { int r, g, b; };
struct Skin {
    const char* name;
    Rgb bg, edge, inset, text, dim, faint, grid;
    Rgb accent, violet, warm, warn, blue, rose, lime;
    Rgb panelTop, panelBot, knobHi, knobLo, knobEdge, knobPtr, headTop, keyWhite, keyBlack, keyLabel;
};
static const Skin kSkins[] = {
    { "Sydalesis", {29,33,39},{76,86,97},{25,30,35},{233,237,240},{174,181,188},{106,122,139},{60,69,79},
      {142,226,208},{181,152,246},{241,191,133},{241,168,133},{139,184,243},{239,140,176},{205,223,144},
      {52,58,68},{41,46,55},{84,94,107},{45,51,59},{12,15,18},{233,237,240},{44,51,59},{211,215,219},{49,55,61},{93,108,124} },
    { "Desert Dusk", {42,33,31},{112,87,79},{36,28,27},{246,239,230},{205,191,180},{153,120,103},{80,63,59},
      {242,170,123},{201,160,213},{250,214,151},{246,144,118},{160,186,213},{231,142,142},{223,205,144},
      {72,57,52},{55,43,39},{123,99,89},{61,48,44},{16,11,10},{246,239,230},{63,49,46},{240,231,218},{63,50,46},{142,112,95} },
    { "Ember", {36,30,28},{99,81,73},{31,26,24},{242,237,232},{191,179,172},{137,112,100},{72,60,55},
      {250,164,103},{233,126,118},{255,210,137},{249,144,117},{215,180,145},{240,126,135},{231,205,135},
      {63,52,47},{49,40,36},{109,90,82},{55,45,42},{14,11,9},{242,237,232},{55,44,41},{228,220,212},{57,49,46},{124,103,90} },
    { "Glacier", {27,34,46},{79,98,120},{24,31,41},{237,242,247},{184,193,204},{103,128,157},{58,72,89},
      {147,207,255},{194,166,255},{246,217,159},{247,173,145},{138,238,248},{229,164,229},{176,235,216},
      {48,60,75},{38,48,61},{91,110,133},{45,56,69},{9,14,20},{237,242,247},{41,52,67},{228,235,241},{46,57,71},{102,123,145} },
    { "Moss", {29,36,29},{81,96,79},{25,31,25},{236,239,232},{180,189,174},{115,136,104},{60,72,58},
      {180,223,136},{231,188,135},{239,212,140},{240,161,126},{146,207,190},{221,158,167},{211,229,156},
      {54,63,51},{43,50,41},{93,106,87},{48,56,45},{11,15,11},{236,239,232},{46,54,44},{224,229,218},{52,60,49},{107,122,98} },
    { "Nebula", {31,27,44},{90,80,123},{28,24,39},{239,236,246},{193,187,207},{119,103,159},{66,58,89},
      {206,153,255},{133,222,240},{255,188,150},{247,145,145},{153,162,255},{255,137,210},{169,236,188},
      {57,49,78},{44,38,61},{105,92,138},{51,44,69},{11,8,19},{239,236,246},{49,43,68},{232,228,242},{55,46,71},{114,100,156} },
    { "Oberheim", {36,36,40},{124,124,131},{34,34,36},{244,244,245},{208,208,210},{134,134,142},{72,72,78},
      {243,99,91},{250,198,103},{248,211,138},{249,136,110},{167,185,212},{232,128,128},{213,213,160},
      {72,72,78},{51,51,56},{189,189,194},{115,115,125},{15,15,15},{24,24,26},{63,63,69},{235,235,235},{43,43,46},{117,117,126} },
    { "Virus", {38,26,28},{118,68,73},{34,23,24},{246,237,237},{206,184,185},{155,97,104},{81,51,55},
      {255,114,123},{140,208,255},{255,201,134},{255,153,121},{160,169,255},{255,134,184},{231,231,142},
      {72,44,47},{56,34,38},{125,77,82},{61,38,41},{15,7,9},{246,237,237},{64,39,43},{241,229,229},{61,37,40},{143,89,92} },
    { "Graphite", {33,33,33},{93,93,93},{30,30,30},{242,242,242},{192,192,192},{123,123,123},{66,66,66},
      {229,229,229},{160,196,255},{212,212,212},{239,167,140},{186,186,186},{206,206,206},{199,199,199},
      {60,60,60},{46,46,46},{110,110,110},{51,51,51},{10,10,10},{242,242,242},{51,51,51},{230,230,230},{52,52,52},{118,118,118} },
    { "Paper", {226,222,214},{163,155,143},{238,235,228},{28,28,31},{72,71,69},{118,114,108},{208,203,194},
      {0,119,102},{92,43,203},{177,102,27},{176,64,36},{28,84,168},{167,45,92},{95,123,30},
      {240,237,231},{230,226,218},{250,248,244},{205,200,192},{150,145,138},{40,40,44},{236,232,225},{250,250,248},{50,50,54},{104,101,96} },
    // 5.1.1: six more in the blue range
    { "Abyss", {21,28,42},{64,81,110},{20,25,37},{230,237,246},{172,184,197},{93,119,156},{48,63,85},
      {103,233,250},{162,153,255},{246,209,152},{248,156,138},{121,170,255},{230,149,212},{154,238,210},
      {38,47,67},{29,36,54},{74,93,123},{36,45,61},{7,10,17},{230,237,246},{34,43,62},{214,223,236},{37,46,63},{90,112,142} },
    { "Midnight Ink", {25,29,48},{73,82,118},{23,25,42},{242,244,250},{186,190,205},{106,115,158},{54,60,86},
      {205,222,255},{169,160,255},{237,209,162},{247,154,145},{140,174,255},{221,158,221},{179,227,207},
      {44,49,75},{34,38,60},{89,97,133},{42,47,66},{8,10,19},{242,244,250},{39,44,67},{228,231,242},{43,48,68},{101,109,150} },
    { "Cobalt", {30,42,76},{102,124,179},{28,40,70},{240,244,252},{203,211,228},{122,141,182},{64,83,128},
      {248,216,138},{176,166,255},{255,221,157},{255,157,131},{147,190,255},{238,154,201},{176,235,176},
      {52,71,122},{40,56,98},{129,146,188},{51,69,113},{8,13,32},{248,232,194},{47,64,109},{224,230,246},{40,55,95},{119,137,177} },
    { "Arctic Steel", {36,42,52},{109,124,144},{33,39,48},{240,244,247},{200,207,214},{129,143,161},{70,81,95},
      {173,216,255},{184,174,244},{236,212,169},{243,165,146},{152,199,246},{219,173,201},{186,226,206},
      {60,70,85},{47,55,68},{138,150,165},{60,70,83},{12,15,20},{240,244,247},{54,63,76},{229,234,240},{52,60,72},{125,138,156} },
    { "Deep Ocean", {19,37,49},{60,107,129},{19,34,45},{230,243,246},{170,192,201},{87,141,165},{42,75,92},
      {112,241,233},{147,182,255},{246,217,159},{248,156,138},{110,189,249},{221,158,203},{161,241,189},
      {33,60,76},{25,49,63},{69,117,139},{33,57,73},{5,13,20},{230,243,246},{29,54,70},{214,232,238},{28,52,67},{83,133,154} },
    { "Indigo Hour", {34,32,61},{103,101,160},{32,30,56},{242,241,251},{203,202,222},{127,123,173},{68,65,109},
      {166,176,255},{205,160,255},{255,206,157},{251,151,151},{147,173,255},{246,152,218},{176,235,196},
      {58,55,100},{45,43,78},{126,124,173},{55,54,90},{10,9,23},{242,241,251},{51,50,89},{226,224,245},{50,47,81},{123,119,168} },
};
static constexpr int kSkinCount = (int)(sizeof(kSkins) / sizeof(kSkins[0]));

static Color cBg, cPanelEdge, cInset, cText, cDim, cFaint, cGrid;
static Color cAccent, cViolet, cWarm, cWarn, cBlue, cRose, cLime;
static Color cPanelTop, cPanelBot, cKnobHi, cKnobLo, cKnobEdge, cKnobPtr, cHeadTop, cKeyWhite, cKeyBlack, cKeyLabel;
static Color cLfoCol[6];
static int gSkin = -1;

inline Color rgb(const Rgb& c) { return Color(c.r, c.g, c.b); }
inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
inline Color mixc(const Color& a, const Color& b, float t) {
    return Color(clamp01(a.red + (b.red - a.red) * t), clamp01(a.green + (b.green - a.green) * t),
                 clamp01(a.blue + (b.blue - a.blue) * t), clamp01(a.alpha + (b.alpha - a.alpha) * t));
}
inline Color withA(const Color& c, float a) { return Color(c.red, c.green, c.blue, clamp01(a)); }
inline Color tint(const Color& c, float k, float a = 1.f) { return withA(mixc(cBg, c, k), a); }
inline Color shade(float v, float a = 1.f) { return withA(mixc(cBg, cPanelEdge, (v - 10.f) / 26.f), a); }

inline void applySkin(int i) {
    if (i < 0 || i >= kSkinCount) i = 0;
    gSkin = i;
    const Skin& s = kSkins[i];
    cBg = rgb(s.bg); cPanelEdge = rgb(s.edge); cInset = rgb(s.inset); cText = rgb(s.text); cDim = rgb(s.dim); cFaint = rgb(s.faint); cGrid = rgb(s.grid);
    cAccent = rgb(s.accent); cViolet = rgb(s.violet); cWarm = rgb(s.warm); cWarn = rgb(s.warn); cBlue = rgb(s.blue); cRose = rgb(s.rose); cLime = rgb(s.lime);
    cPanelTop = rgb(s.panelTop); cPanelBot = rgb(s.panelBot); cKnobHi = rgb(s.knobHi); cKnobLo = rgb(s.knobLo); cKnobEdge = rgb(s.knobEdge);
    cKnobPtr = rgb(s.knobPtr); cHeadTop = rgb(s.headTop); cKeyWhite = rgb(s.keyWhite); cKeyBlack = rgb(s.keyBlack); cKeyLabel = rgb(s.keyLabel);
    cLfoCol[0] = cAccent; cLfoCol[1] = cViolet; cLfoCol[2] = cWarm; cLfoCol[3] = cBlue; cLfoCol[4] = cRose; cLfoCol[5] = cLime;
}

struct Rect {
    float x, y, w, h;
    bool contains(float px, float py) const { return px >= x && px < x + w && py >= y && py < y + h; }
    float cx() const { return x + w * 0.5f; }
    float cy() const { return y + h * 0.5f; }
};

enum CtlKind { ckKnob, ckSlider, ckSelector, ckToggle, ckChoice, ckButtonVal, ckVFader };
enum Page { pgMain, pgMod, pgFx, pgSeq, pgTune, pgLayer, pgPerf, pgSetup, kPages };
static const char* const kPageNames[kPages] = { "MAIN", "MOD", "FX", "SEQ", "TUNE", "LAYERS", "PERFORM", "SETUP" };

// A parameter-bound control. vis: -1 always, 0..4 = osc engine type, 10 = wavetable or analog.
// visParam: also requires values[visParam] (rounded) == visVal.
struct Ctl {
    int kind;
    uint32_t p;
    Rect r;              // knobs: x,y = centre, w = radius
    const char* label;
    int page;
    int osc = -1, vis = -1;
    int fxUnit = -1;     // FX page: coordinates are relative to the unit's card
    int visParam = -1, visVal = 1;
    int uiKey = -1, uiVal = 0;   // 4.2: also requires uiSel[uiKey] == uiVal (editor-only tabs: MSEG 3/4, slots 17-32)
    int count = 0;       // choice buttons
    bool arrows = false; // selector
    bool small = false;
    const char* const* names = nullptr;   // choice: custom labels
};

inline bool isBipolar(uint32_t p) { return P(p).min < 0.f && P(p).max > 0.f; }
inline float toNorm(uint32_t p, float v) {
    const ParamDef& d = P(p);
    if (d.flags & fLog) return std::log(v / d.min) / std::log(d.max / d.min);
    return (v - d.min) / (d.max - d.min);
}
inline float fromNorm(uint32_t p, float n) {
    const ParamDef& d = P(p);
    n = n < 0.f ? 0.f : (n > 1.f ? 1.f : n);
    float v = (d.flags & fLog) ? d.min * std::pow(d.max / d.min, n) : d.min + n * (d.max - d.min);
    if (d.flags & fInt) v = std::round(v);
    return v;
}
inline int enumCount(uint32_t p) { return (int)(P(p).max - P(p).min) + 1; }
inline uint32_t nowMs() {
    using namespace std::chrono;
    return (uint32_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
inline std::string lower(std::string s) { for (auto& c : s) c = (char)std::tolower((unsigned char)c); return s; }

} // namespace

class DriftfieldUI : public UI
{
public:
    DriftfieldUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT)
    {
        loadOptions();
        applySkin(optSkin);
        for (uint32_t i = 0; i < kParamCount; ++i) values[i] = hostVals[i] = P(i).def;
        presetValues(0, values);
        presetStates(0, stateValue);
        for (int k = 0; k < kStateCount; ++k) applyStateLocal(k);
        std::memset(&tel, 0, sizeof(tel));
        tel.newest = -1;
        std::memset(&telRaw, 0, sizeof(telRaw));
        telRaw.newest = -1;
       #if DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
        shared = driftSharedFromInstance(getPluginInstancePointer());
       #endif
        loadSharedResources();
        loadUiFont();
        const double sf = getScaleFactor();
        setGeometryConstraints((uint)(kW * kMinScale * sf), (uint)(kHT * kMinScale * sf), true);
        const float saved = loadSavedScale();
        if ((uint)(kW * saved * sf) != getWidth() || (uint)(kHT * saved * sf) != getHeight()) applyScale(saved);
        buildLayout();
        scanUserPresets();
        loadFavorites();
    }

    ~DriftfieldUI() override {
        stopTicker();
        // 5.1.1: notes played or latched on the on-screen keyboard must not outlive the editor
        kbAllOff();
        if (kbSus) kbSend(0xB0, 64, 0);
        if (kbPitch != 0.f) kbSend(0xE0, 0, 64);
    }

protected:
    // 5.1.5: Inter Medium for every label (tall x-height, clear at small sizes); DejaVu Sans stays
    // loaded as the fallback for symbols Inter does not carry (★ ✗ ◎ ⠿ ...).
    FontId uiFont = -1;
    void loadUiFont() {
        uiFont = findFont("SydalesisUI");
        if (uiFont < 0) uiFont = createFontFromMemory("SydalesisUI", kInterUi, kInterUiSize, false);
        const FontId dv = findFont(NANOVG_DEJAVU_SANS_TTF);
        if (uiFont >= 0 && dv >= 0) nvgAddFallbackFontId(getContext(), uiFont, dv);
    }
    #include "ui/Core.inl"
    #include "ui/Widgets.inl"
    #include "ui/PageMain.inl"
    #include "ui/FmPanel.inl"
    #include "ui/PageMod.inl"
    #include "ui/PageFxSeq.inl"
    #include "ui/Input.inl"
    #include "ui/Presets.inl"
    #include "ui/Options.inl"
    #include "ui/Output.inl"
    #include "ui/Learn.inl"
    #include "ui/Tune.inl"
    #include "ui/Layers.inl"
    #include "ui/Perform.inl"
    #include "ui/Keys.inl"

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DriftfieldUI)
};

UI* createUI()
{
    return new DriftfieldUI();
}

END_NAMESPACE_DISTRHO
