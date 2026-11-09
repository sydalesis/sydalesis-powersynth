// Driftfield — platform helpers kept out of shared headers.
#include <string>
#include <filesystem>
#include <cstdlib>
#ifdef _WIN32
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <shlobj.h>
#endif

std::string driftPlatformDocumentsDir()
{
    namespace fs = std::filesystem;
    auto u8 = [](const fs::path& p) { const auto s = p.u8string(); return std::string(s.begin(), s.end()); };
#ifdef _WIN32
    wchar_t buf[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, buf))) return u8(fs::path(buf));   // follows OneDrive redirection
    if (const wchar_t* up = _wgetenv(L"USERPROFILE")) return u8(fs::path(up) / L"Documents");
    return "C:\\";
#else
    const char* home = std::getenv("HOME");
    return u8(fs::path(home ? home : "/tmp") / "Documents");
#endif
}

#include "Wavetables.hpp"
namespace drift {
const BuiltinTables& builtinTables()
{
    static BuiltinTables inst;
    return inst;
}
}
