#include <iostream>
#include "vpg.h"

int main(int argc, char *argv[])
{   
    std::cout << "Run evpglib test:" << std::endl;

    const float Tovms = 8000.0f, dTms = 33.0f;
    vpg::PulseProcessor *proc = new vpg::PulseProcessor(dTms, vpg::PulseProcessor::HeartRate);
    vpg::PeakDetector   *pdet = new vpg::PeakDetector(proc->getLength(),50,33,dTms);
    proc->setPeakDetector(pdet);

    float sV = 0.0f, f0 = 0.8f, meas;
    for(uint i = 0; i < 25; i++) {
        float f = f0 + i*0.1;
        std::cout << "Actual freq. is " << f * 60.0 << " bpm\n";
        float _phaseshift = 3.1415926f * std::rand() / RAND_MAX;
        for(uint j = 0; j < Tovms/dTms; j++) {
            sV = std::sin( 2 * 3.1415926 * f * j * dTms/1000.0 + _phaseshift) + 125.0;
            proc->update(sV, dTms);
            if(j % 100 == 0) {
                meas = proc->computeFrequency();
                std::cout << "Measurement (FFT):\t"
                          << meas << " bpm,\terr: "
                          << (int)std::abs(meas - f*60.0) << " bpm\n";
                meas = 60000.0f / pdet->averageCardiointervalms();
                std::cout << "Measurement (HRV):\t"
                          << meas << " bpm,\terr: "
                          << (int)std::abs(meas - f*60.0) << " bpm\n";
            }
        }
        std::cout << "\n";
    }
    delete proc;
    return 0;
}
