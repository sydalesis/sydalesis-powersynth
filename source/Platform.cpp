// Driftfield — platform helpers kept out of shared headers.
#include <string>
#include <filesystem>
#include <cstdlib>
#include <cstdint>
#ifdef _WIN32
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <shlobj.h>
 #include <shellapi.h>
#endif

// 6.0.1: text entry in hosts that keep the keyboard (Studio One): the editor window only receives WM_CHAR while
// it has the Win32 keyboard focus, and clicking a plug-in window does not give it that focus. Take it while a
// text field is active and hand it back to the host window afterwards (so the host's shortcuts work again).
void driftPlatformTextFocus(uintptr_t nativeView, bool take)
{
#if defined(_WIN32)
    HWND h = reinterpret_cast<HWND>(nativeView);
    if (h == nullptr) return;
    if (take) { SetFocus(h); return; }
    if (GetFocus() == h) { HWND p = GetParent(h); if (p != nullptr) SetFocus(p); }
#else
    (void)nativeView; (void)take;     // macOS / Linux: the view becomes first responder on click (Window::focus)
#endif
}

// 6.0: open a web page in the default browser (the synth's page / Bandcamp links)
void driftPlatformOpenUrl(const char* url)
{
#if defined(_WIN32)
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    const std::string c = std::string("open '") + url + "' &"; (void)std::system(c.c_str());
#else
    const std::string c = std::string("xdg-open '") + url + "' >/dev/null 2>&1 &"; (void)std::system(c.c_str());
#endif
}

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
