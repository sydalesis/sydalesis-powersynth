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
 #include <commctrl.h>
 #include <atomic>
 #include <thread>
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

// 6.0.5: editor frame pacing on Windows. pugl drives the editor's frame timer with SetTimer, which Windows clamps
// to 10 ms and rounds to the system tick (15.6 ms unless the host raised the timer resolution), and WM_TIMER only
// arrives when the message queue is otherwise empty: the 120 Hz setting ran at 64 Hz or less, 60 Hz at ~32 Hz.
// The pacer is a thread on a high-resolution waitable timer that posts one private message per frame to the editor
// window (subclassed to receive it). At most one message is in flight: a slow frame skips ticks instead of
// queueing them. Other platforms: nullptr (their native timers are precise).
struct DriftPacer;
#if defined(_WIN32)
struct DriftPacer {
    HWND hwnd = nullptr;
    void (*cb)(void*) = nullptr;
    void* ctx = nullptr;
    std::atomic<int> ms { 16 };
    std::atomic<bool> quit { false }, pending { false };
    std::thread th;
};
static constexpr UINT kPacerMsg = WM_APP + 0x5D1;
static constexpr UINT_PTR kPacerSubclassId = 0x5D1A;
static LRESULT CALLBACK driftPacerProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data)
{
    if (msg == kPacerMsg && id == kPacerSubclassId) {
        DriftPacer* p = reinterpret_cast<DriftPacer*>(data);
        if (p != nullptr && p->cb != nullptr) p->cb(p->ctx);
        if (p != nullptr) p->pending.store(false, std::memory_order_release);
        return 0;
    }
    if (msg == WM_NCDESTROY) RemoveWindowSubclass(h, driftPacerProc, kPacerSubclassId);
    return DefSubclassProc(h, msg, wp, lp);
}
#endif

DriftPacer* driftPlatformPacerStart(uintptr_t nativeView, int intervalMs, void (*cb)(void*), void* ctx)
{
#if defined(_WIN32)
    HWND h = reinterpret_cast<HWND>(nativeView);
    if (h == nullptr || cb == nullptr) return nullptr;
    DriftPacer* p = new DriftPacer();
    p->hwnd = h; p->cb = cb; p->ctx = ctx; p->ms.store(intervalMs < 1 ? 1 : intervalMs);
    if (!SetWindowSubclass(h, driftPacerProc, kPacerSubclassId, reinterpret_cast<DWORD_PTR>(p))) { delete p; return nullptr; }
    p->th = std::thread([p] {
        // high-resolution waitable timer (Windows 10 1803+), else a normal one
        HANDLE t = CreateWaitableTimerExW(nullptr, nullptr, 0x00000002 /* CREATE_WAITABLE_TIMER_HIGH_RESOLUTION */, TIMER_ALL_ACCESS);
        if (t == nullptr) t = CreateWaitableTimerW(nullptr, FALSE, nullptr);
        if (t == nullptr) return;
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
        LARGE_INTEGER due;
        while (!p->quit.load(std::memory_order_acquire)) {
            due.QuadPart = -(LONGLONG)p->ms.load(std::memory_order_relaxed) * 10000LL;    // relative, 100 ns units
            if (!SetWaitableTimer(t, &due, 0, nullptr, nullptr, FALSE)) { Sleep((DWORD)p->ms.load()); }
            else WaitForSingleObject(t, 1000);
            if (p->quit.load(std::memory_order_acquire)) break;
            if (!p->pending.exchange(true, std::memory_order_acq_rel))
                if (!PostMessageW(p->hwnd, kPacerMsg, 0, 0)) p->pending.store(false);
        }
        CloseHandle(t);
    });
    return p;
#else
    (void)nativeView; (void)intervalMs; (void)cb; (void)ctx;
    return nullptr;
#endif
}

void driftPlatformPacerInterval(DriftPacer* p, int intervalMs)
{
#if defined(_WIN32)
    if (p != nullptr) p->ms.store(intervalMs < 1 ? 1 : intervalMs);
#else
    (void)p; (void)intervalMs;
#endif
}

// call on the editor (window) thread, before the window goes away
void driftPlatformPacerStop(DriftPacer* p)
{
#if defined(_WIN32)
    if (p == nullptr) return;
    p->quit.store(true, std::memory_order_release);
    if (p->th.joinable()) p->th.join();
    RemoveWindowSubclass(p->hwnd, driftPacerProc, kPacerSubclassId);   // a message still queued now falls through to pugl (ignored)
    delete p;
#else
    (void)p;
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
