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
#ifndef PEAKDETECTOR_H
#define PEAKDETECTOR_H
//-------------------------------------------------------
#ifdef DLL_BUILD_SETUP
    #ifdef TARGET_OS_LINUX
        #define DLLSPEC __attribute__((visibility("default")))
    #else
        #define DLLSPEC __declspec(dllexport)
    #endif
#else
    #ifdef TARGET_OS_LINUX
        #define DLLSPEC
    #else
        #define DLLSPEC __declspec(dllimport)
    #endif
#endif
//-------------------------------------------------------
#include "opencv2/core.hpp"
//-------------------------------------------------------
namespace vpg {
#ifndef VPG_BUILD_FROM_SOURCE
class DLLSPEC PeakDetector
#else
class PeakDetector
#endif
{
public:
    PeakDetector(int _signallength, int _intervalslength, int _intervalssubsetvolume = 11, float _dT_ms = 33.0);
    ~PeakDetector();

    void update(float value, float time);

    const float *getBinarySignal() const;
    int getSignalLength() const;

    const float *getIntervalsVector() const;
    int getIntervalsLength() const;

    float getCurrentInterval() const;
    int getIntervalsPosition() const;

    /**
     * @brief returns average of the last _n cardiointervals
     * @param _n - how many intervals should be counted (if n < 0 than full length of the v_Intervals should be counted)
     * @return average value of the cardiointerval
     */
    float averageCardiointervalms(int _n=9) const;

    /**
     * @brief compute Bayevsky's Stress Index
     * @param _cidata - pointer to the vector that stores cardiointervals
     * @param _length - vector's length
     * @return index value
     */
    float computeBSI();

private:
    void __init(int _signallength, int _intervalslength, int _intervalssubsetvolume, float _dT_ms);
    void __updateInterval(float _duration);
    // For the signal loop array
    int __loop(int d) const;
    // For the intervals loop array
    int __seek(int d) const;
    float __getDuration(int start, int stop);

    int curposforsignal;
    int curposforinterval;
    int m_intervalssubsetvolume;
    int lastfrontposition;
    float *v_S;
    float *v_BS;
    float *v_T;
    float *v_DS;
    float *v_Intervals;
    int m_signallength;
    int m_intervalslength;
};

inline int PeakDetector::__loop(int d) const
{
    return ((m_signallength + (d % m_signallength)) % m_signallength);
}

inline int PeakDetector::__seek(int d) const
{
    return ((m_intervalslength + (d % m_intervalslength)) % m_intervalslength);
}
}
//-------------------------------------------------------
#endif // PEAKDETECTOR_H
