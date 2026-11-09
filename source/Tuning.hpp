// Driftfield 4 — microtuning: built-in scale library, Scala .scl / .kbm parsing, and the key -> pitch
// table the engine plays from. Pitches are absolute MIDI semitones (69 = A4 at 440 Hz), fractional.
// Everything here is allocation-free once the library exists, so the engine can rebuild a table on
// the audio thread when the root or scale changes.
#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

namespace drift {

static constexpr int kTuneMaxDegrees = 128;

struct ScaleDef {
    int n = 12;                              // degrees per period
    double c[kTuneMaxDegrees + 1] = {};      // cents; c[0] = 0, c[n] = period
    bool twelve = true;                      // laid out on the 12 keys of the octave (snap mask is meaningful)
    uint16_t mask = 0x0FFF;                  // scale members among the 12 keys (bit k = key root + k)
    char txt[kTuneMaxDegrees + 1][14] = {}; // the pitch as written ("5/4", "701.955"), for display
    char name[64] = "Equal";
    char desc[128] = "";
};

struct KbmDef {
    bool valid = false;
    int size = 0, first = 0, last = 127, middle = 60, refNote = 69, octaveDeg = 0;
    double refFreq = 440.0;
    int map[128] = {};                       // degree per map position, -1 = unmapped
};

// ------------------------------------------------------------------------------------------------
// locale-independent number parsing (hosts may switch the C locale)
static inline const char* tnSkipWs(const char* s) { while (*s == ' ' || *s == '\t' || *s == '\r') ++s; return s; }
static inline bool tnParseNumber(const char*& s, double& out, bool& hadDot) {
    s = tnSkipWs(s);
    bool neg = false;
    if (*s == '-') { neg = true; ++s; } else if (*s == '+') ++s;
    if (!((*s >= '0' && *s <= '9') || *s == '.')) return false;
    double v = 0.0; hadDot = false;
    while (*s >= '0' && *s <= '9') { v = v * 10.0 + (*s - '0'); ++s; }
    if (*s == '.') {
        hadDot = true; ++s;
        double f = 0.1;
        while (*s >= '0' && *s <= '9') { v += (*s - '0') * f; f *= 0.1; ++s; }
    }
    out = neg ? -v : v;
    return true;
}
// one Scala pitch line: "701.955" (cents), "3/2" (ratio) or "2" (integer ratio)
static inline bool tnParsePitch(const char* s, double& cents) {
    double a; bool dot;
    if (!tnParseNumber(s, a, dot)) return false;
    if (dot) { cents = a; return true; }
    s = tnSkipWs(s);
    double b = 1.0;
    if (*s == '/') { ++s; bool d2; if (!tnParseNumber(s, b, d2)) return false; }
    if (a <= 0.0 || b <= 0.0) return false;
    cents = 1200.0 * std::log2(a / b);
    return true;
}

// reads lines of a text buffer, skipping '!' comments
struct TnLines {
    const char* p;
    explicit TnLines(const char* t) : p(t ? t : "") {}
    bool next(char* out, int cap, bool skipComments = true) {
        while (*p) {
            const char* e = p;
            while (*e && *e != '\n') ++e;
            int len = (int)(e - p);
            const char* line = p;
            p = *e ? e + 1 : e;
            if (skipComments && len > 0 && line[0] == '!') continue;
            if (len >= cap) len = cap - 1;
            std::memcpy(out, line, (size_t)len); out[len] = 0;
            while (len > 0 && (out[len - 1] == '\r' || out[len - 1] == ' ' || out[len - 1] == '\t')) out[--len] = 0;
            return true;
        }
        return false;
    }
};

// Scala .scl; returns false with a reason in err
static inline bool parseScl(const char* text, ScaleDef& sc, std::string* err = nullptr) {
    TnLines L(text);
    char line[512];
    if (!L.next(line, sizeof line)) { if (err) *err = "empty file"; return false; }
    ScaleDef r;
    std::strncpy(r.desc, line, sizeof(r.desc) - 1);
    // count line (may be preceded by blank description)
    int count = -1;
    while (L.next(line, sizeof line)) {
        const char* s = line; double v; bool dot;
        if (tnParseNumber(s, v, dot) && !dot) { count = (int)v; break; }
        if (*tnSkipWs(line)) { if (err) *err = "no note count"; return false; }
    }
    if (count < 1 || count > kTuneMaxDegrees) { if (err) *err = "note count must be 1..128"; return false; }
    int got = 0;
    while (got < count && L.next(line, sizeof line)) {
        if (!*tnSkipWs(line)) continue;
        double c;
        if (!tnParsePitch(line, c)) { if (err) *err = std::string("bad pitch line: ") + line; return false; }
        r.c[++got] = c;
        { const char* q = tnSkipWs(line); int t = 0; while (*q && *q != ' ' && *q != '\t' && t < 13) r.txt[got][t++] = *q++; r.txt[got][t] = 0; }
    }
    if (got != count) { if (err) *err = "fewer pitches than the count says"; return false; }
    r.n = count;
    if (r.c[r.n] <= 0.0) { if (err) *err = "the last pitch (period) must be above 1/1"; return false; }
    r.twelve = r.n == 12 && std::fabs(r.c[12] - 1200.0) < 0.5;
    r.mask = 0x0FFF;
    std::strncpy(r.name, "Scala", sizeof(r.name) - 1);
    sc = r;
    return true;
}

// Scala .kbm keyboard mapping
static inline bool parseKbm(const char* text, KbmDef& k, std::string* err = nullptr) {
    TnLines L(text);
    char line[256];
    double vals[7];
    for (int i = 0; i < 7; ++i) {
        if (!L.next(line, sizeof line)) { if (err) *err = "kbm header is incomplete"; return false; }
        const char* s = line; bool dot;
        if (!tnParseNumber(s, vals[i], dot)) { if (err) *err = std::string("bad kbm line: ") + line; return false; }
    }
    KbmDef r;
    r.size = (int)vals[0]; r.first = (int)vals[1]; r.last = (int)vals[2]; r.middle = (int)vals[3];
    r.refNote = (int)vals[4]; r.refFreq = vals[5]; r.octaveDeg = (int)vals[6];
    if (r.size < 0 || r.size > 128 || r.refFreq <= 0.0 || r.middle < 0 || r.middle > 127) { if (err) *err = "kbm values out of range"; return false; }
    for (int i = 0; i < r.size; ++i) {
        if (!L.next(line, sizeof line)) { r.map[i] = -1; continue; }
        const char* s = tnSkipWs(line); double v; bool dot;
        if (*s == 'x' || *s == 'X' || !tnParseNumber(s, v, dot)) r.map[i] = -1; else r.map[i] = (int)v;
    }
    r.valid = true;
    k = r;
    return true;
}

// ------------------------------------------------------------------------------------------------
// Built-in library
struct BuiltinScaleSrc { const char* name; const char* desc; bool twelve; uint16_t mask; const char* pitches; };
static const BuiltinScaleSrc kBuiltinScaleSrc[] = {
    { "JI 5-limit Major", "Ptolemaic just intonation; pure thirds and fifths on the root", true, 0x0AB5,
      "16/15 9/8 6/5 5/4 4/3 45/32 3/2 8/5 5/3 9/5 15/8 2/1" },
    { "JI 5-limit Minor", "just minor: 6/5, 8/5, 9/5 on the root", true, 0x05AD,
      "16/15 9/8 6/5 5/4 4/3 45/32 3/2 8/5 5/3 9/5 15/8 2/1" },
    { "JI 7-limit Septimal", "septimal minor third 7/6, tritone 7/5, harmonic seventh 7/4", true, 0x0FFF,
      "16/15 9/8 7/6 5/4 4/3 7/5 3/2 8/5 5/3 7/4 15/8 2/1" },
    { "Harmonic 16-32", "harmonics 16 to 31 folded into one octave", true, 0x0FFF,
      "17/16 9/8 19/16 5/4 21/16 11/8 3/2 13/8 27/16 7/4 15/8 2/1" },
    { "Pythagorean", "stacked pure fifths; bright thirds, pure open drones", true, 0x0FFF,
      "256/243 9/8 32/27 81/64 4/3 729/512 3/2 128/81 27/16 16/9 243/128 2/1" },
    { "Well-Tuned Piano", "La Monte Young's 7-limit tuning (root = E-flat in the original)", true, 0x0FFF,
      "567/512 9/8 147/128 21/16 1323/1024 189/128 3/2 49/32 7/4 441/256 63/32 2/1" },
    { "Meantone 1/4-comma", "pure major thirds; the Renaissance keyboard tuning", true, 0x0FFF,
      "76.049 193.157 310.265 386.314 503.422 579.471 696.578 772.627 889.735 1006.843 1082.892 1200.0" },
    { "Werckmeister III", "1691 well temperament; every key usable, each with its colour", true, 0x0FFF,
      "90.225 192.180 294.135 390.225 498.045 588.270 696.090 792.180 888.270 996.090 1092.180 1200.0" },
    { "Kirnberger III", "1779 well temperament with a pure C major third", true, 0x0FFF,
      "90.225 193.157 294.135 386.314 498.045 590.224 696.578 792.180 889.735 996.090 1088.269 1200.0" },
    { "Raga Yaman", "Kalyan thaat: S R G M# P D N", true, 0x0AD5,
      "16/15 9/8 6/5 5/4 4/3 45/32 3/2 8/5 5/3 9/5 15/8 2/1" },
    { "Raga Bhairav", "dawn raga: ati-komal re and dha, S r G m P d N", true, 0x09B3,
      "256/243 9/8 6/5 5/4 4/3 45/32 3/2 128/81 5/3 9/5 15/8 2/1" },
    { "Raga Todi", "S r g M# P d N with Pythagorean flats", true, 0x09CB,
      "256/243 9/8 32/27 5/4 4/3 729/512 3/2 128/81 5/3 16/9 15/8 2/1" },
    { "Raga Bhairavi", "all flat: S r g m P d n", true, 0x05AB,
      "16/15 9/8 6/5 5/4 4/3 45/32 3/2 8/5 5/3 9/5 15/8 2/1" },
    { "Raga Darbari", "Darbari Kanada: low g and d, S R g m P d n", true, 0x05AD,
      "16/15 9/8 32/27 5/4 4/3 45/32 3/2 128/81 5/3 16/9 15/8 2/1" },
    { "Raga Marwa", "sunset raga without Pa: S r G M# D N", true, 0x0A53,
      "16/15 9/8 6/5 5/4 4/3 45/32 3/2 8/5 5/3 9/5 15/8 2/1" },
    { "Raga Bhupali", "pentatonic: S R G P D", true, 0x0295,
      "16/15 9/8 6/5 5/4 4/3 45/32 3/2 8/5 5/3 9/5 15/8 2/1" },
    { "Maqam Rast", "neutral third and seventh (E and B half-flat)", true, 0x0AB5,
      "100.0 200.0 300.0 350.0 500.0 600.0 700.0 800.0 900.0 1000.0 1050.0 1200.0" },
    { "Maqam Bayati", "neutral second on the root (key 1 = 150 cents)", true, 0x05AB,
      "150.0 200.0 300.0 400.0 500.0 600.0 700.0 800.0 900.0 1000.0 1100.0 1200.0" },
    { "Slendro", "Javanese 5-tone gamelan (one key per tone)", false, 0,
      "231.0 474.0 717.0 955.0 1200.0" },
    { "Pelog", "Javanese 7-tone gamelan (one key per tone)", false, 0,
      "120.0 270.0 540.0 670.0 785.0 950.0 1200.0" },
    { "Harmonics 8-16", "the 8th to 16th harmonics, one key per harmonic", false, 0,
      "9/8 10/8 11/8 12/8 13/8 14/8 15/8 2/1" },
    { "22 Shruti", "the Indian 22-shruti octave, one key per shruti", false, 0,
      "256/243 16/15 10/9 9/8 32/27 6/5 5/4 81/64 4/3 27/20 45/32 729/512 3/2 128/81 8/5 5/3 27/16 16/9 9/5 15/8 243/128 2/1" },
    { "Partch 43", "Harry Partch's 43-tone 11-limit octave, one key per tone", false, 0,
      "81/80 33/32 21/20 16/15 12/11 11/10 10/9 9/8 8/7 7/6 32/27 6/5 11/9 5/4 14/11 9/7 21/16 4/3 27/20 11/8 7/5 10/7 16/11 40/27 3/2 32/21 14/9 11/7 8/5 18/11 5/3 27/16 12/7 7/4 16/9 9/5 20/11 11/6 15/8 40/21 64/33 160/81 2/1" },
    { "Bohlen-Pierce", "13 just steps to the tritave (3/1), no octaves", false, 0,
      "27/25 25/21 9/7 7/5 75/49 5/3 9/5 49/25 15/7 7/3 63/25 25/9 3/1" },
    { "24-EDO", "quarter tones, one key per 50 cents", false, 0, nullptr },
    { "19-EDO", "19 equal steps per octave", false, 0, nullptr },
    { "Carlos Alpha", "78-cent steps, no octave", false, 0, "77.965" },
};
static constexpr int kBuiltinScales = (int)(sizeof(kBuiltinScaleSrc) / sizeof(kBuiltinScaleSrc[0]));

static inline const ScaleDef* builtinScales() {
    static ScaleDef lib[kBuiltinScales];
    static bool done = [] {
        for (int i = 0; i < kBuiltinScales; ++i) {
            const BuiltinScaleSrc& s = kBuiltinScaleSrc[i];
            ScaleDef& d = lib[i];
            std::strncpy(d.name, s.name, sizeof(d.name) - 1);
            std::strncpy(d.desc, s.desc, sizeof(d.desc) - 1);
            d.twelve = s.twelve; d.mask = s.twelve ? s.mask : 0x0FFF;
            if (!s.pitches) {
                const int edo = std::strcmp(s.name, "24-EDO") == 0 ? 24 : 19;
                d.n = edo;
                for (int k = 1; k <= edo; ++k) d.c[k] = 1200.0 * k / edo;
                continue;
            }
            int n = 0;
            const char* p = s.pitches;
            while (*p && n < kTuneMaxDegrees) {
                while (*p == ' ') ++p;
                if (!*p) break;
                char tok[32]; int t = 0;
                while (*p && *p != ' ' && t < 31) tok[t++] = *p++;
                tok[t] = 0;
                double c; if (tnParsePitch(tok, c)) { d.c[++n] = c; std::strncpy(d.txt[n], tok, 13); }
            }
            d.n = n;
        }
        return true;
    }();
    (void)done;
    return lib;
}
static inline const char* const* builtinScaleNames() {
    static const char* names[kBuiltinScales];
    for (int i = 0; i < kBuiltinScales; ++i) names[i] = kBuiltinScaleSrc[i].name;
    return names;
}

// ------------------------------------------------------------------------------------------------
// key -> pitch table
static inline double scaleDegreeCents(const ScaleDef& sc, int deg) {
    const int n = sc.n > 0 ? sc.n : 1;
    int o = deg >= 0 ? deg / n : -((-deg + n - 1) / n);
    const int i = deg - o * n;
    return o * sc.c[n] + sc.c[i];
}
// rootPc: 0 = C .. 11 = B (the root key is that pitch class in the octave starting at middle C);
// refHz: A4 reference; snap: keys outside the scale's member set (12-key scales) play the nearest member.
// member[k] (optional) reports which keys are scale members.
static inline void buildTuningTable(const ScaleDef& sc, const KbmDef* kbm, int rootPc, double refHz, bool snap,
                                    float* semis, bool* member = nullptr) {
    double t[128]; bool mapped[128];
    if (kbm && kbm->valid) {
        const int m = kbm->size;
        auto centsOf = [&](int key, bool& ok) -> double {
            ok = true;
            const int d = key - kbm->middle;
            if (m == 0) return scaleDegreeCents(sc, d);
            const int o = d >= 0 ? d / m : -((-d + m - 1) / m);
            const int i = d - o * m;
            const int deg = kbm->map[i];
            if (deg < 0) { ok = false; return 0.0; }
            const double octC = kbm->octaveDeg > 0 ? scaleDegreeCents(sc, kbm->octaveDeg) : sc.c[sc.n];
            return o * octC + scaleDegreeCents(sc, deg);
        };
        bool okRef;
        double refC = centsOf(kbm->refNote, okRef);
        if (!okRef) refC = (kbm->refNote - kbm->middle) * 100.0;
        const double refS = 69.0 + 12.0 * std::log2(kbm->refFreq / 440.0);
        for (int k = 0; k < 128; ++k) {
            bool ok; const double c = centsOf(k, ok);
            mapped[k] = ok && k >= kbm->first && k <= kbm->last;
            t[k] = refS + (c - refC) / 100.0;
        }
    } else {
        const int R = 60 + (rootPc % 12 + 12) % 12;
        const double refOff = 12.0 * std::log2(refHz / 440.0);
        for (int k = 0; k < 128; ++k) {
            mapped[k] = true;
            if (sc.twelve) {
                const int d = k - R;
                const int o = d >= 0 ? d / 12 : -((-d + 11) / 12);
                const int i = d - o * 12;
                t[k] = R + refOff + (o * sc.c[12] + sc.c[i]) / 100.0;
                if (snap && !((sc.mask >> i) & 1)) mapped[k] = false;
            } else {
                t[k] = R + refOff + scaleDegreeCents(sc, k - R) / 100.0;
            }
        }
    }
    // unmapped / snapped keys take the nearest mapped key (lower one on a tie)
    for (int k = 0; k < 128; ++k) {
        if (member) member[k] = mapped[k];
        if (mapped[k]) { semis[k] = (float)t[k]; continue; }
        int best = -1;
        for (int d = 1; d < 128 && best < 0; ++d) {
            if (k - d >= 0 && mapped[k - d]) best = k - d;
            else if (k + d < 128 && mapped[k + d]) best = k + d;
        }
        semis[k] = (float)(best >= 0 ? t[best] : (double)k);
    }
    for (int k = 0; k < 128; ++k) { if (semis[k] < -24.f) semis[k] = -24.f; if (semis[k] > 160.f) semis[k] = 160.f; }
}

// Scala text is kept on one line in states and preset files: "\n" = newline, "\\" = backslash
static inline std::string tnEscape(const std::string& s) {
    std::string r; r.reserve(s.size() + 16);
    for (char c : s) { if (c == '\\') r += "\\\\"; else if (c == '\n') r += "\\n"; else if (c != '\r') r += c; }
    return r;
}
static inline std::string tnUnescape(const std::string& s) {
    std::string r; r.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) { const char n = s[++i]; r += n == 'n' ? '\n' : n; }
        else r += s[i];
    }
    return r;
}

// MTS-ESP (or any external source of per-note tuning)
struct ITuningProvider {
    virtual ~ITuningProvider() {}
    virtual bool connected() = 0;
    virtual double noteSemis(int note, int channel) = 0;   // absolute MIDI semitones
};

} // namespace drift
