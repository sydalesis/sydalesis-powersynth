#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include <cstdio>
#include <vector>
int main(){ drift::Telemetry tel{}; auto* e=drift::createEngine(); e->setSampleRate(48000); e->setTelemetry(&tel);
 e->setParam(pSeqMode,2); std::vector<float> L(480),R(480);
 for(int pass=0;pass<2;++pass){ e->setTransport(120,pass==1,0);
 uint8_t on[3]={0x90,48,100}; e->midi(on,3);
 for(int i=0;i<100;++i){ e->process(L.data(),R.data(),480); if(i%10==0) printf("pass %d t=%.2f step %d active %d\n",pass,i*0.01,tel.seqStep,tel.activeCount);}
 uint8_t of[3]={0x80,48,0}; e->midi(of,3); for(int i=0;i<20;++i) e->process(L.data(),R.data(),480);}
}
