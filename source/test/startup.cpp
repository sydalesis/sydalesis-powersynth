#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include <chrono>
#include <cstdio>
int main(){ auto t0=std::chrono::steady_clock::now(); auto* e=drift::createEngine(); auto t1=std::chrono::steady_clock::now();
 printf("engine create incl. built-in tables: %.0f ms\n", std::chrono::duration<double,std::milli>(t1-t0).count());
 auto t2=std::chrono::steady_clock::now(); auto* t=drift::loadWavetableFile((std::string(getenv("HOME"))+"/Documents/Driftfield/Wavetables/Drone/Abyssal.wav").c_str(), false); auto t3=std::chrono::steady_clock::now();
 printf("64-frame file table + spectral: %.0f ms (%d frames, spectral %d)\n", std::chrono::duration<double,std::milli>(t3-t2).count(), t?t->frames:0, t&&t->spectral?t->spectral->frames:0); delete e; }
