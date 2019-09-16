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

PulseProcessor::PulseProcessor(float dT_ms, ProcessType type)
{
    switch(type){
        case HeartRate:
            __init(7500.0f, 400.0f, 350.0f, dT_ms, type);
            break;
    }
}

PulseProcessor::PulseProcessor(float Tov_ms, float Tcn_ms, float Tlpf_ms, float dT_ms, ProcessType type)
{
    __init(Tov_ms, Tcn_ms, Tlpf_ms, dT_ms, type);
}

void PulseProcessor::__init(float Tov_ms, float Tcn_ms, float Tlpf_ms, float dT_ms, ProcessType type)
{
    m_dTms = dT_ms;
    m_length = static_cast<int>( Tov_ms / dT_ms );
    m_filterlength = static_cast<int>( Tlpf_ms / dT_ms );

    switch(type){
        case HeartRate:
            m_Frequency = 0.0f;
            m_interval = static_cast<int>( Tcn_ms/ dT_ms );
            m_bottomFrequencyLimit = 0.8f; // 48 bpm
            m_topFrequencyLimit = 2.5f;    // 150 bpm
            break;        
    }

    v_raw = new float[m_length];
    v_Y = new float[m_length];
    v_time = new float[m_length];
    v_FA = new float[m_length/2 + 1];

    for(int i = 0; i < m_length; i++)  {
        v_raw[i] = 0.0f;
        v_Y[i] = 0.0f;
        v_time[i] = dT_ms;
    }
    v_X = new float[m_filterlength];
    for(int i = 0; i < m_filterlength; i ++)
        v_X[i] = static_cast<float>(i);

    v_datamat = cv::Mat(1, m_length, CV_32F);
    v_dftmat = cv::Mat(1, m_length, CV_32F);

    curpos = 0;
    m_snr  = 0;
    m_stdev = 0;
}

PulseProcessor::~PulseProcessor()
{
    delete[] v_raw;
    delete[] v_Y;
    delete[] v_X;
    delete[] v_time;
    delete[] v_FA;
}

void PulseProcessor::update(float value, float time, bool filter)
{
    if(filter) {
        v_raw[curpos] = value;
        if(std::abs(time - m_dTms) < m_dTms)
            v_time[curpos] = time;
        else
            v_time[curpos] = m_dTms;       
        float mean = 0.0, sko = 0.0;
        for(int i = 0; i < m_interval; i++)
            mean += v_raw[__loop(curpos - i)];
        mean /= m_interval;
        int pos = 0;
        for(int i = 0; i < m_interval; i++) {
            pos = __loop(curpos - i);
            sko += (v_raw[pos] - mean)*(v_raw[pos] - mean);
        }
        sko = std::sqrt( sko/(m_interval - 1));
        m_stdev = sko;
        if(sko < 0.01f)
            sko = 1.0f;

        v_X[__seek(curpos)] = (v_raw[curpos] - mean)/ sko;

        float integral = 0.0f;
        for(int i = 0; i < m_filterlength; i++)
            integral += v_X[i];

        v_Y[curpos] = ( integral + v_Y[__loop(curpos - 1)] )  / (m_filterlength + 1.0);
    } else {
        v_Y[curpos] = value;
        v_time[curpos] = time;
    }
	
	if(pt_peakdetector != 0)
        pt_peakdetector->update(v_Y[curpos], v_time[curpos]);

    curpos = (curpos + 1) % m_length;
}

float PulseProcessor::computeFrequency()
{
    int _zeros = 0;
    float *pt = v_datamat.ptr<float>(0);
    for(int i = 0; i < m_length; i++) {
        pt[i] = v_Y[__loop(curpos - 1 - i)];
        if(std::abs(pt[i]) <= 0.01f) {
            _zeros++;
        }
    }
    if(_zeros > m_length/2) {
        m_snr = -10.0f;
        return m_Frequency;
    }
    //cv::blur(v_datamat,v_datamat,cv::Size(3,1));
    cv::dft(v_datamat, v_dftmat);
    const float *v_fft = v_dftmat.ptr<const float>(0);

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
    float time = 0.0f;
    for(int i = 0; i < m_length; i++)
        time += v_time[i];

    int bottom = m_bottomFrequencyLimit * time / 1000.0f;
    int top = m_topFrequencyLimit * time / 1000.0f;
    if(top > m_length/2)
        top = m_length/2;
    int i_maxpower = 0;
    float maxpower = 0.0;
    for(int i = bottom + 2 ; i <= top - 2; i++)
        if( maxpower < v_FA[i] ) {
            maxpower = v_FA[i];
            i_maxpower = i;
        }

    float noise_power = 0.0;
    float signal_power = 0.0;
    float signal_moment = 0.0;
    for (int i = bottom; i <= top; i++) {
        if ( (i >= i_maxpower - 2) && (i <= i_maxpower + 2) ) {
            signal_power += v_FA[i];
            signal_moment += i * v_FA[i];
        } else {
            noise_power += v_FA[i];
        }
    }

    m_snr = 0.0f;
    if(signal_power > 0.01f && noise_power > 0.01f) {
        m_snr = 10.0f * std::log10( signal_power / noise_power );
        float bias = i_maxpower - ( signal_moment / signal_power );
        m_snr *= (1.0f / (1.0f + bias*bias));
    }
    if(m_snr > 2.5f)
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

const float *PulseProcessor::getSignal() const
{
    return v_Y;
}

float PulseProcessor::getFrequency() const
{
    return m_Frequency;
}

float PulseProcessor::getSNR() const
{
    return m_snr;
}

float PulseProcessor::getSignalSampleValue() const
{
    return v_Y[__loop(curpos-1)];
}

float PulseProcessor::getSignalStdev() const
{
    return m_stdev;
}

void PulseProcessor::setPeakDetector(PeakDetector *pointer)
{
    pt_peakdetector = pointer;
}

} // end of namespace vpg
