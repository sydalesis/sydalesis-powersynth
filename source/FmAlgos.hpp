#pragma once
// 6.0 FM oscillator: the 32 six-operator algorithms (DX7 layout), shared by the engine and the UI diagram.
#include <cstdint>

namespace drift {

struct FmAlgo {
    uint8_t mods[6];     // bit m set in mods[k]: operator m+1 modulates operator k+1
    uint8_t carriers;    // bit k: operator k+1 goes to the output
    int8_t  fbSrc;       // feedback: output of operator fbSrc (0-based) ...
    int8_t  fbDst;       // ... is fed into the phase of operator fbDst (the same op for a self loop)
    uint8_t depth[6];    // modulation depth: 0 carrier, 1 modulates a carrier, 2 modulates a depth-1 op ...
};

namespace fmdetail {
// algorithms written as "m>c" edges (1-based); carriers are the operators that modulate nothing
struct Edge { int m, c; };
struct AlgoSrc { Edge e[8]; int ne; int fbSrc, fbDst; };
static const AlgoSrc kSrc[32] = {
    { { {2,1},{4,3},{5,4},{6,5} }, 4, 6, 6 },                 // 1
    { { {2,1},{4,3},{5,4},{6,5} }, 4, 2, 2 },                 // 2
    { { {2,1},{3,2},{5,4},{6,5} }, 4, 6, 6 },                 // 3
    { { {2,1},{3,2},{5,4},{6,5} }, 4, 4, 6 },                 // 4  (loop 4 -> 6)
    { { {2,1},{4,3},{6,5} }, 3, 6, 6 },                       // 5
    { { {2,1},{4,3},{6,5} }, 3, 5, 6 },                       // 6  (loop 5 -> 6)
    { { {2,1},{4,3},{5,3},{6,5} }, 4, 6, 6 },                 // 7
    { { {2,1},{4,3},{5,3},{6,5} }, 4, 4, 4 },                 // 8
    { { {2,1},{4,3},{5,3},{6,5} }, 4, 2, 2 },                 // 9
    { { {2,1},{3,2},{5,4},{6,4} }, 4, 3, 3 },                 // 10
    { { {2,1},{3,2},{5,4},{6,4} }, 4, 6, 6 },                 // 11
    { { {2,1},{4,3},{5,3},{6,3} }, 4, 2, 2 },                 // 12
    { { {2,1},{4,3},{5,3},{6,3} }, 4, 6, 6 },                 // 13
    { { {2,1},{4,3},{5,4},{6,4} }, 4, 6, 6 },                 // 14
    { { {2,1},{4,3},{5,4},{6,4} }, 4, 2, 2 },                 // 15
    { { {2,1},{3,1},{5,1},{4,3},{6,5} }, 5, 6, 6 },           // 16
    { { {2,1},{3,1},{5,1},{4,3},{6,5} }, 5, 2, 2 },           // 17
    { { {2,1},{3,1},{4,1},{5,4},{6,5} }, 5, 3, 3 },           // 18
    { { {2,1},{3,2},{6,4},{6,5} }, 4, 6, 6 },                 // 19
    { { {3,1},{3,2},{5,4},{6,4} }, 4, 3, 3 },                 // 20
    { { {3,1},{3,2},{6,4},{6,5} }, 4, 3, 3 },                 // 21
    { { {2,1},{6,3},{6,4},{6,5} }, 4, 6, 6 },                 // 22
    { { {3,2},{6,4},{6,5} }, 3, 6, 6 },                       // 23
    { { {6,3},{6,4},{6,5} }, 3, 6, 6 },                       // 24
    { { {6,4},{6,5} }, 2, 6, 6 },                             // 25
    { { {3,2},{5,4},{6,4} }, 3, 6, 6 },                       // 26
    { { {3,2},{5,4},{6,4} }, 3, 3, 3 },                       // 27
    { { {2,1},{4,3},{5,4} }, 3, 5, 5 },                       // 28
    { { {4,3},{6,5} }, 2, 6, 6 },                             // 29
    { { {4,3},{5,4} }, 2, 5, 5 },                             // 30
    { { {6,5} }, 1, 6, 6 },                                   // 31
    { { {0,0} }, 0, 6, 6 },                                   // 32
};
struct Table {
    FmAlgo a[32];
    Table() {
        for (int i = 0; i < 32; ++i) {
            FmAlgo& f = a[i];
            uint8_t modsSomething = 0;
            for (int k = 0; k < 6; ++k) f.mods[k] = 0;
            for (int e = 0; e < kSrc[i].ne; ++e) {
                const int m = kSrc[i].e[e].m - 1, c = kSrc[i].e[e].c - 1;
                f.mods[c] |= (uint8_t)(1u << m);
                modsSomething |= (uint8_t)(1u << m);
            }
            f.carriers = (uint8_t)(~modsSomething & 63u);
            f.fbSrc = (int8_t)(kSrc[i].fbSrc - 1);
            f.fbDst = (int8_t)(kSrc[i].fbDst - 1);
            // depth: carriers 0; a modulator is one deeper than the shallowest operator it modulates
            for (int k = 0; k < 6; ++k) f.depth[k] = (f.carriers >> k) & 1 ? 0 : 9;
            for (int pass = 0; pass < 6; ++pass)
                for (int c = 0; c < 6; ++c)
                    for (int m = 0; m < 6; ++m)
                        if ((f.mods[c] >> m) & 1 && f.depth[c] + 1 < f.depth[m]) f.depth[m] = (uint8_t)(f.depth[c] + 1);
        }
    }
};
} // namespace fmdetail

static inline const FmAlgo& fmAlgoDef(int i) {
    static const fmdetail::Table t;
    return t.a[i < 0 ? 0 : (i > 31 ? 31 : i)];
}
static inline int fmCarrierCount(const FmAlgo& a) { int n = 0; for (int k = 0; k < 6; ++k) n += (a.carriers >> k) & 1; return n; }

} // namespace drift
