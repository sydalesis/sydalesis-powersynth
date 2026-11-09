// Driftfield 3 — engine build: AVX2
#define DRIFT_NS drift_avx2
#define DRIFT_DSP_NS drift_avx2
#define DRIFT_ISA_NAME "AVX2"
#include "Params.hpp"
#include "Wavetables.hpp"
#include "EngineIface.hpp"
#include "Shared.hpp"
#include "Filters.hpp"
#include "Spectra.hpp"
#include "EngineCore.hpp"

namespace drift_avx2 {
drift::IEngine* create() { return new Engine(); }
}
