#include "../Params.hpp"
#include "../Wavetables.hpp"
#include "../EngineIface.hpp"
#include "../Shared.hpp"
#include "../Library.hpp"
#include <cstdio>
#include <vector>
#include <cmath>
int main(int argc,char**argv){ auto* e=drift::createEngine(); e->setSampleRate(48000);
 e->setParam(oscP(1,ofOn),0); e->setParam(oscP(0,ofType),4); e->setParam(oscP(0,ofGrSource),1);
 e->setParam(oscP(0,ofGrPos),atof(argv[2])); e->setParam(oscP(0,ofGrSize),0.12f); e->setParam(oscP(0,ofGrDensity),60);
 e->setParam(pAEnvA,0.002f); e->setParam(pAEnvD,0.8f); e->setParam(pAEnvS,0.f);
 e->postSample(0, drift::loadSampleFile(drift::lib::resolve(argv[1]).c_str()));
 std::vector<float> L(48000),R(48000); uint8_t on[3]={0x90,60,100}; e->midi(on,3);
 for(int p=0;p<48000;p+=256) e->process(&L[p],&R[p],std::min(256,48000-p));
 double r=0; for(float v:L) r+=v*v; printf("rms %.5f\n", std::sqrt(r/48000)); }
