// Driftfield 4.2 — worker pool for multicore voice rendering (generic code, used by the plug-in).
// run() hands out task indices through one atomic ticket (generation << 32 | next index), so a worker
// that wakes late can never take a task of a newer job. Workers spin briefly between jobs (audio
// blocks arrive every few milliseconds) and then sleep on a condition variable.
#pragma once
#include "EngineIface.hpp"
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <cstdlib>
#if defined(__SSE__) || defined(_M_X64) || defined(__x86_64__)
 #include <xmmintrin.h>
 #define DRIFT_POOL_X86 1
#endif
#ifdef __APPLE__
 #include <pthread.h>
 #include <sys/qos.h>
#endif
#ifdef _WIN32
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
#endif

namespace drift {

class WorkerPool final : public IWorkerPool {
public:
    explicit WorkerPool(int workerCount) {
        if (workerCount < 0) workerCount = 0;
        if (workerCount > kMaxPoolSlots - 1) workerCount = kMaxPoolSlots - 1;
        for (int i = 0; i < workerCount; ++i) threads.emplace_back([this, i] { loop(i + 1); });
    }
    ~WorkerPool() override {
        quit.store(true, std::memory_order_release);
        { std::lock_guard<std::mutex> g(m); }
        cv.notify_all();
        for (auto& t : threads) if (t.joinable()) t.join();
    }
    int workers() const override { return (int)threads.size(); }

    void run(int count, void (*fn)(void*, int, int), void* ctx) override {
        if (count <= 0) return;
        if (threads.empty()) { for (int t = 0; t < count; ++t) fn(ctx, t, 0); return; }
        jobFn = fn; jobCtx = ctx; jobCount.store(count, std::memory_order_relaxed);
        done.store(0, std::memory_order_relaxed);
        const uint64_t g = (uint64_t)(++gen) << 32;
        ticket.store(g, std::memory_order_release);
        if (sleepers.load(std::memory_order_acquire) > 0) { { std::lock_guard<std::mutex> lk(m); } cv.notify_all(); }
        work(g >> 32, 0);
        // wait for the tasks the workers took
        for (int spins = 0; done.load(std::memory_order_acquire) < count; ++spins) {
            if (spins < 20000) pause(); else std::this_thread::yield();
        }
    }

private:
    std::vector<std::thread> threads;
    std::mutex m;
    std::condition_variable cv;
    std::atomic<bool> quit { false };
    std::atomic<int> sleepers { 0 };
    std::atomic<uint64_t> ticket { 0 };
    std::atomic<int> done { 0 };
    uint32_t gen = 0;                          // written by the caller only
    void (*jobFn)(void*, int, int) = nullptr;
    void* jobCtx = nullptr;
    std::atomic<int> jobCount { 0 };

    static inline void pause() {
       #ifdef DRIFT_POOL_X86
        _mm_pause();
       #else
        std::this_thread::yield();
       #endif
    }
    // take tasks of job generation g until none are left
    void work(uint64_t g, int slot) {
        for (;;) {
            uint64_t t = ticket.load(std::memory_order_acquire);
            if ((t >> 32) != g) return;
            const int idx = (int)(t & 0xffffffffu);
            if (idx >= jobCount.load(std::memory_order_relaxed)) return;
            if (!ticket.compare_exchange_weak(t, t + 1, std::memory_order_acq_rel)) continue;
            jobFn(jobCtx, idx, slot);
            done.fetch_add(1, std::memory_order_release);
        }
    }
    void loop(int slot) {
       #ifdef DRIFT_POOL_X86
        _mm_setcsr(_mm_getcsr() | 0x8040);    // flush denormals like the audio thread
       #elif defined(__aarch64__)
        { uint64_t v; __asm__ __volatile__("mrs %0, fpcr" : "=r"(v)); v |= (1ull << 24); __asm__ __volatile__("msr fpcr, %0" : : "r"(v)); }   // 5.1.6: Apple Silicon
       #endif
       #ifdef _WIN32
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        if (HMODULE av = LoadLibraryA("avrt.dll")) {   // join the host's "Pro Audio" scheduling class when available
            typedef HANDLE (WINAPI *AvSet)(LPCSTR, LPDWORD);
            if (AvSet f = (AvSet)(void*)GetProcAddress(av, "AvSetMmThreadCharacteristicsA")) { DWORD idx = 0; f("Pro Audio", &idx); }
        }
       #endif
       #ifdef __APPLE__
        // macOS: highest QoS keeps voice workers on the performance cores (Apple Silicon would otherwise
        // be free to park them on efficiency cores and the audio thread would wait on them)
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
       #endif
        uint64_t seen = 0;
        for (;;) {
            // spin about 0.2 ms for the next job, then sleep
            uint64_t g = 0;
            for (int spins = 0; spins < 6000; ++spins) {
                if (quit.load(std::memory_order_acquire)) return;
                g = ticket.load(std::memory_order_acquire) >> 32;
                if (g != seen) break;
                pause();
            }
            if (g == seen) {
                std::unique_lock<std::mutex> lk(m);
                sleepers.fetch_add(1, std::memory_order_acq_rel);
                cv.wait_for(lk, std::chrono::milliseconds(50), [&] { return quit.load(std::memory_order_acquire) || (ticket.load(std::memory_order_acquire) >> 32) != seen; });
                sleepers.fetch_sub(1, std::memory_order_acq_rel);
                if (quit.load(std::memory_order_acquire)) return;
                g = ticket.load(std::memory_order_acquire) >> 32;
                if (g == seen) continue;
            }
            seen = g;
            work(g, slot);
        }
    }
};

// "0" / "" = off, "auto", or a thread count 2..8 (the audio thread counts as one)
static inline int poolThreadsFor(const char* v) {
    if (!v || !*v || (v[0] == '0' && !v[1])) return 1;
    if (v[0] == 'a') {
        const unsigned hc = std::thread::hardware_concurrency();
        int n = hc >= 8 ? 4 : (hc >= 4 ? 2 : 1);
        return n;
    }
    int n = std::atoi(v);
    return n < 1 ? 1 : (n > kMaxPoolSlots ? kMaxPoolSlots : n);
}

} // namespace drift
