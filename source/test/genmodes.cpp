// prints the analysed modal banks of library samples as JSON (tools/modal_v42.json) for the 4.2 bank
#include "../Analysis.hpp"
#include <cstdio>
int main(int argc, char** argv) {
    std::printf("{\n");
    for (int i = 1; i < argc; i += 2) {
        drift::ModalData md; drift::ana::Info info;
        const bool ok = drift::ana::toModes(argv[i + 1], md, info);
        std::fprintf(stderr, "%s: %s f0 %.1f, %d modes %s\n", argv[i], ok ? "ok" : "FAILED", info.f0, md.count, info.err.c_str());
        std::printf("  \"%s\": \"%s\"%s\n", argv[i], ok ? drift::modalToString(md).c_str() : "", i + 2 < argc ? "," : "");
    }
    std::printf("}\n");
}
