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
#include "hrvprocessor.h"

namespace vpg {

HRVProcessor::HRVProcessor(float _timestepms, bool _blur)
{
    setTimestepms(_timestepms);
    setF_smooth(_blur);
}

HRVProcessor::~HRVProcessor()
{
}

void HRVProcessor::enrollIntervals(const float *_vIntervals, int _intervalsLength, bool _computespectrum)
{
    // Let's determine how many of counts do we really need
    float _totalduration = 0.0f;
    for(int i = 0; i < _intervalsLength - 1; i++) // -1 guaranties this (1)
        _totalduration += _vIntervals[i];

    const int _counts = static_cast<int>(_totalduration / timestepms());

    m_intervalsmat = cv::Mat(1, _counts, CV_32F); // this cv::Mat object will store interpolated intervals
    float *_pointer = m_intervalsmat.ptr<float>(0);

    // Make linear interpolation of the input data for uniform timestep
    int j = 0; // will count position in input data
    float _duration = _vIntervals[0];

    for(int i = 0; i < _counts; i++) {

        if(i*timestepms() > _duration) {
           j++; // (1) do not worry about 'j' it will not exceed (_intervalsLength - 1)
           _duration += _vIntervals[j];
        }

        if(_vIntervals[j+1] > _vIntervals[j]) {
            _pointer[i] = _vIntervals[j] + (_vIntervals[j] - _duration + i*timestepms()) * (_vIntervals[j+1] - _vIntervals[j]) / _vIntervals[j];
        } else {
            _pointer[i] = _vIntervals[j+1] + (_duration - i*timestepms()) * (_vIntervals[j] - _vIntervals[j+1]) / _vIntervals[j];
        }
    }

    // Let's slightly blur interpolated HRV signal with uniform kernel
    if(f_smooth)
        cv::blur(m_intervalsmat, m_intervalsmat, cv::Size(3,1));

    if(_computespectrum) {
    // Evaluate dft of the HRV signal
        cv::dft(m_intervalsmat, m_dftmat);
        const float *_pdft = m_dftmat.ptr<const float>(0);

        m_amplitudespectrum = cv::Mat(1, m_dftmat.cols/2 + 1, CV_32F);
        float *_amp = m_amplitudespectrum.ptr<float>(0);

        // complex conjugated symmetry, read on http://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#dft
        _amp[0] = 0.0; // exclude zero count which is equal to the mean value of the signal
        if(m_dftmat.cols % 2 == 0) {// Even number of counts
            for(int i = 1; i < m_dftmat.cols/2; i++)
                _amp[i] = std::sqrt(_pdft[2*i-1]*_pdft[2*i-1] + _pdft[2*i]*_pdft[2*i]);
            _amp[m_dftmat.cols/2] = std::sqrt(_pdft[m_dftmat.cols-1]*_pdft[m_dftmat.cols-1]);
        } else {
            for(int i = 1; i <= m_dftmat.cols/2; i++)
                _amp[i] = std::sqrt(_pdft[2*i-1]*_pdft[2*i-1] + _pdft[2*i]*_pdft[2*i]);
        }
    }
}

float HRVProcessor::timestepms() const
{
    return m_timestepms;
}

void HRVProcessor::setTimestepms(float timestepms)
{
    m_timestepms = timestepms;
}

const float *HRVProcessor::getHRVSignal() const
{
    return m_intervalsmat.ptr<const float>(0);
}

int HRVProcessor::getHRVSignalLength() const
{
    return m_intervalsmat.cols;
}

const float *HRVProcessor::getHRVAmplitudeSpectrum() const
{
    return m_amplitudespectrum.ptr<const float>(0);
}

int HRVProcessor::getHRVAmplitudeSpectrumLength() const
{
    return m_amplitudespectrum.cols;
}

bool HRVProcessor::getF_smooth() const
{
    return f_smooth;
}

void HRVProcessor::setF_smooth(bool value)
{
    f_smooth = value;
}

float HRVProcessor::computeLF2HF()
{
    if(m_intervalsmat.total() > 0) {
        float _totaldurationms = 0.0;
        float *_intervalms = m_intervalsmat.ptr<float>(0);
        for(size_t i = 0; i < m_intervalsmat.total(); i++)
            _totaldurationms += _intervalms[i];

        float _freqstep = 1000.0f / _totaldurationms, _hf = 0.0, _lf = 0.0, _freq;
        const float *_amp = m_amplitudespectrum.ptr<const float>(0);
        for(int i = 0; i <= m_dftmat.cols/2; ++i) {
            _freq = i*_freqstep;
            if((_freq > 0.04f) && (_freq <= 0.15f))
                _lf += _amp[i];
            else if((_freq > 0.15f) && (_freq <= 0.4f))
                _hf += _amp[i];
        }
        return _lf / _hf;
    }
    return -1.0;
}

} // end of namespace vpg
