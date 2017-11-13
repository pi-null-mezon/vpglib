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
#ifndef HRVPROCESSOR_H
#define HRVPROCESSOR_H
//-------------------------------------------------------
#ifdef DLL_BUILD_SETUP
    #ifdef Q_OS_LINUX
        #define DLLSPEC __attribute__((visibility("default")))
    #else
        #define DLLSPEC __declspec(dllexport)
    #endif
#else
    #ifdef Q_OS_LINUX
        #define DLLSPEC
    #else
        #define DLLSPEC __declspec(dllimport)
    #endif
#endif
//-------------------------------------------------------
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
//-------------------------------------------------------
namespace vpg {

#ifndef VPG_BUILD_FROM_SOURCE
class DLLSPEC HRVProcessor
#else
class HRVProcessor
#endif
{
public:
    HRVProcessor(double _timestepms = 250.0, bool _blur = true); // 4 Hz, seems it is convinient value

    ~HRVProcessor();

    void enrollIntervals(const double *_vIntervals, int _intervalsLength, bool _computespectrum=true);

    const double *getHRVSignal() const;
    int getHRVSignalLength() const;

    const double *getHRVAmplitudeSpectrum() const;
    int getHRVAmplitudeSpectrumLength() const;

    double timestepms() const;
    void setTimestepms(double timestepms);

    bool getF_smooth() const;
    void setF_smooth(bool value);

private:
    cv::Mat m_intervalsmat;
    cv::Mat m_dftmat;

    cv::Mat m_amplitudespectrum;

    bool f_smooth;
    double m_timestepms;
};
}
//-------------------------------------------------------
#endif // HRVPROCESSOR_H
