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

HRVProcessor::HRVProcessor(double _timestepms, bool _blur)
{
    setTimestepms(_timestepms);
    setF_smooth(_blur);
}

HRVProcessor::~HRVProcessor()
{
}

void HRVProcessor::enrollIntervals(const double *_vIntervals, int _intervalsLength)
{
    // Let's determine how many of counts do we really need
    double _totalduration = 0.0;
    for(int i = 0; i < _intervalsLength - 1; i++) // -1 guaranties this (1)
        _totalduration += _vIntervals[i];

    const int _counts = static_cast<int>(_totalduration / timestepms());

    m_intervalsmat = cv::Mat(1, _counts, CV_64F); // this cv::Mat object will store interpolated intervals
    double *_pointer = m_intervalsmat.ptr<double>(0);

    // Make linear interpolation of the input data for uniform timestep
    int j = 0; // will count position in input data
    double _duration = _vIntervals[0];

    for(int i = 0; i < _counts; i++) {

        if(i*timestepms() > _duration) {
           j++; // (1) do not warry about j it will not exceed (_intervalsLength - 1)
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
        cv::blur(m_intervalsmat, m_intervalsmat, cv::Size(5,1));

    // Evaluate dft of the HRV signal
    cv::dft(m_intervalsmat, m_dftmat);
    const double *_pdft = m_dftmat.ptr<const double>(0);

    m_amplitudespectrum = cv::Mat(1, m_dftmat.cols/2 + 1, CV_64F);
    double *_amp = m_amplitudespectrum.ptr<double>(0);

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

double HRVProcessor::timestepms() const
{
    return m_timestepms;
}

void HRVProcessor::setTimestepms(double timestepms)
{
    m_timestepms = timestepms;
}

const double *HRVProcessor::getHRVSignal() const
{
    if(m_intervalsmat.empty())
        return NULL;
    else
        return m_intervalsmat.ptr<const double>(0);
}

int HRVProcessor::getHRVSignalLength() const
{
    return m_intervalsmat.cols;
}

const double *HRVProcessor::getHRVAmplitudeSpectrum() const
{
    if(m_amplitudespectrum.empty())
        return NULL;
    else
        return m_amplitudespectrum.ptr<const double>(0);
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

} // end of namespace vpg
