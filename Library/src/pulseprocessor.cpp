/*
 * Copyright (c) 2015, Taranov Alex <pi-null-mezon@yandex.ru>.
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 */
#include "pulseprocessor.h"

namespace vpg {

PulseProcessor::PulseProcessor(double dT_ms, ProcessType type)
{
    switch(type){
        case HeartRate:
            __init(7500.0, 400.0, 350.0, dT_ms, type);
            break;
    }
}

PulseProcessor::PulseProcessor(double Tov_ms, double Tcn_ms, double Tlpf_ms, double dT_ms, ProcessType type)
{
    __init(Tov_ms, Tcn_ms, Tlpf_ms, dT_ms, type);
}

void PulseProcessor::__init(double Tov_ms, double Tcn_ms, double Tlpf_ms, double dT_ms, ProcessType type)
{
    m_dTms = dT_ms;
    m_length = static_cast<int>( Tov_ms / dT_ms );
    m_filterlength = static_cast<int>( Tlpf_ms / dT_ms );

    switch(type){
        case HeartRate:
            m_Frequency = 0.0;
            m_interval = static_cast<int>( Tcn_ms/ dT_ms );
            m_bottomFrequencyLimit = 0.8; // 48 bpm
            m_topFrequencyLimit = 2.5;    // 150 bpm
            break;        
    }

    v_raw = new double[m_length];
    v_Y = new double[m_length];
    v_time = new double[m_length];
    v_FA = new double[m_length/2 + 1];

    for(int i = 0; i < m_length; i++)  {
        v_raw[i] = 0.0;
        v_Y[i] = 0.0;
        v_time[i] = dT_ms;
    }
    v_X = new double[m_filterlength];
    for(int i = 0; i < m_filterlength; i ++)
		v_X[i] = (double)i;

    v_datamat = cv::Mat(1, m_length, CV_64F);
    v_dftmat = cv::Mat(1, m_length, CV_64F);

    curpos = 0;
}

PulseProcessor::~PulseProcessor()
{
    delete[] v_raw;
    delete[] v_Y;
    delete[] v_X;
    delete[] v_time;
    delete[] v_FA;
}

void PulseProcessor::update(double value, double time, bool filter)
{
    if(filter) {
        v_raw[curpos] = value;
        if(std::abs(time - m_dTms) < m_dTms) {
            v_time[curpos] = time;
        } else {
            v_time[curpos] = m_dTms;
        }

        double mean = 0.0;
        double sko = 0.0;

        for(int i = 0; i < m_interval; i++) {
            mean += v_raw[__loop(curpos - i)];
        }
        mean /= m_interval;
        int pos = 0;
        for(int i = 0; i < m_interval; i++) {
            pos = __loop(curpos - i);
            sko += (v_raw[pos] - mean)*(v_raw[pos] - mean);
        }
        sko = std::sqrt( sko/(m_interval - 1));
        if(sko < 0.01) {
            sko = 1.0;
        }
        v_X[__seek(curpos)] = (v_raw[curpos] - mean)/ sko;

        double integral = 0.0;
        for(int i = 0; i < m_filterlength; i++) {
            integral += v_X[i];
        }

        v_Y[curpos] = ( integral + v_Y[__loop(curpos - 1)] )  / (m_filterlength + 1.0);
    } else {
        v_Y[curpos] = value;
        v_time[curpos] = time;
    }
	
	if(pt_peakdetector != 0)
        pt_peakdetector->update(v_Y[curpos], v_time[curpos]);

    curpos = (curpos + 1) % m_length;
}

double PulseProcessor::computeFrequency()
{
    unsigned int _zeros = 0;
    double *pt = v_datamat.ptr<double>(0);
    for(int i = 0; i < m_length; i++) {
        pt[i] = v_Y[__loop(curpos - 1 - i)];
        if(std::abs(pt[i]) <= 0.01) {
            _zeros++;
        }
    }
    if(_zeros > 0.5 * m_length) {
        m_snr = -10.0;
        return m_Frequency;
    }
    //cv::blur(v_datamat,v_datamat,cv::Size(3,1));
    cv::dft(v_datamat, v_dftmat);
    const double *v_fft = v_dftmat.ptr<const double>(0);

    // complex-conjugate-symmetrical array
    v_FA[0] = v_fft[0]*v_fft[0];
    if((m_length % 2) == 0) { // Even number of counts
        for(int i = 1; i < m_length/2; i++)
            v_FA[i] = v_fft[2*i-1]*v_fft[2*i-1] + v_fft[2*i]*v_fft[2*i];
        v_FA[m_length/2] = v_fft[m_length-1]*v_fft[m_length-1];
    } else { // Odd number of counts
        for(int i = 1; i <= m_length/2; i++)
            v_FA[i] = v_fft[2*i-1]*v_fft[2*i-1] + v_fft[2*i]*v_fft[2*i];
    }

    // Count time
    double time = 0.0;
    for (int i = 0; i < m_length; i++)
        time += v_time[i];

    int bottom = (int)(m_bottomFrequencyLimit * time / 1000.0);
    int top = (int)(m_topFrequencyLimit * time / 1000.0);
    if(top > (m_length/2))
        top = m_length/2;
    int i_maxpower = 0;
    double maxpower = 0.0;
    for (int i = bottom + 2 ; i <= top - 2; i++)
        if ( maxpower < v_FA[i] ) {
            maxpower = v_FA[i];
            i_maxpower = i;
        }

    double noise_power = 0.0;
    double signal_power = 0.0;
    double signal_moment = 0.0;
    for (int i = bottom; i <= top; i++) {
        if ( (i >= i_maxpower - 2) && (i <= i_maxpower + 2) ) {
            signal_power += v_FA[i];
            signal_moment += i * v_FA[i];
        } else {
            noise_power += v_FA[i];
        }
    }

    m_snr = 0.0;
    if(signal_power > 0.01 && noise_power > 0.01) {
        m_snr = 10.0 * std::log10( signal_power / noise_power );
        double bias = (double)i_maxpower - ( signal_moment / signal_power );
        m_snr *= (1.0 / (1.0 + bias*bias));
    }
    if(m_snr > 2.5)
        m_Frequency = (signal_moment / signal_power) * 60000.0 / time;

    return m_Frequency;
}

int PulseProcessor::getLength() const
{
    return m_length;
}

int PulseProcessor::getLastPos() const
{
    return __loop(curpos - 1);
}

const double * PulseProcessor::getSignal() const
{
    //return v_datamat.ptr<const double>(0);
    return v_Y;
}

double PulseProcessor::getFrequency() const
{
    return m_Frequency;
}

double PulseProcessor::getSNR() const
{
    return m_snr;
}

double PulseProcessor::getSignalSampleValue() const
{
    return v_Y[__loop(curpos-1)];
}

void PulseProcessor::setPeakDetector(PeakDetector *pointer)
{
    pt_peakdetector = pointer;
}

} // end of namespace vpg
