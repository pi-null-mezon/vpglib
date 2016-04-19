#include <iostream>
#include "vpg.h"

#define PI 3.1415926

int main(int argc, char *argv[])
{   
    std::cout << "Run evpglib test:" << std::endl;

    double Tovms = 10000.0;
    double dTms = 33.0;
    vpg::PulseProcessor *proc = new vpg::PulseProcessor(6000.0, dTms, vpg::PulseProcessor::HeartRate);

    double sV = 0.0;
    double f0 = 0.8;
    int meas;
    for(uint i = 0; i < 25; i++) {
        double f = f0 + i*0.1;
        std::cout << "Actual freq. is " << f * 60.0 << " bpm\n";
        for(uint j = 0; j < Tovms/dTms; j++) {
            sV = std::sin( 2 * PI * f * j * dTms/1000.0 + 1.34) + 1.0;
            proc->update( sV, dTms );
            if(j % 200 == 0) {
                meas = proc->computeHR();
                std::cout << "Measurement:\t"
                          << meas << " bpm,\terr: "
                          << (int)std::abs(meas - f*60.0) << " bpm\n";
            }
        }
        std::cout << "\n";
    }
    return 0;
}
