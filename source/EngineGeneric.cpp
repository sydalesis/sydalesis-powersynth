// Driftfield 3 — engine build: Generic
#define DRIFT_NS drift_generic
#define DRIFT_DSP_NS drift_generic
#define DRIFT_ISA_NAME "Generic"
#include "Params.hpp"
#include "Wavetables.hpp"
#include "EngineIface.hpp"
#include "Shared.hpp"
#include "Filters.hpp"
#include "Spectra.hpp"
#include "EngineCore.hpp"

namespace drift_generic {
drift::IEngine* create() { return new Engine(); }
}
