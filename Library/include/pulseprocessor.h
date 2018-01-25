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
#ifndef PULSEPROCESSOR_H
#define PULSEPROCESSOR_H
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
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "peakdetector.h"
//-------------------------------------------------------
namespace vpg {

/**
 * @brief The PulseProcessor class should be used for pulse frequency evaluation
 */
#ifndef VPG_BUILD_FROM_SOURCE
class DLLSPEC PulseProcessor
#else
class PulseProcessor
#endif
{
public:
    enum ProcessType {HeartRate};
    /**
     * Default constructor
     * @param dT_ms - discretization period in milliseconds
     * @param type - type of desired pulse frequency source/range
     */
    PulseProcessor(double dT_ms = 33.0, ProcessType type = HeartRate);
    /**
     * Overloaded constructor
     * @param Tov_ms - length of signal record in time domain in milliseconds
     * @param Tcn_ms - time interval for signal centering and normalization
     * @param dT_ms - discretization period in milliseconds
     * @param type - type of desired pulse frequency source/range
     */
    PulseProcessor(double Tov_ms, double Tcn_ms, double Tlpf_ms,  double dT_ms, ProcessType type);
    /**
     * Class destructor
     */
    virtual ~PulseProcessor();
    /**
     * Update ppg signal by one count
     * @param value - count value
     * @param time - count measurement time in millisecond
     * @note function should be called at each video frame
     */
    void update(double value, double time);
    /**
     * Compute heart rate
     * @return heart rate in beats per minute
     */
    double computeFrequency();
    /**
     * Get signal length
     * @return signal length
     */
    int getLength() const;
    /**
     * @brief self explained
     * @return current position in the signal vector
     */
    int getLastPos() const;
    /**
     * @brief get pointer to signal counts
     * @return pointer to data
     */
    const double *getSignal() const;
    /**
     * @brief get last frequency estimation
     * @return frequency
     */
    double getFrequency() const;
    /**
     * @brief get last snr estimation
     * @return relation between pulse and noise harmonics energies
     */
    double getSNR() const;     
    /**
     * @brief use this function to get last one VPG signal sample value
     * @return value of the centered and normalized VPG signal
     */
    double getSignalSampleValue() const;
    /**
     * @brief setPeakDetector - set up particular peak detector that will be updated within processing pipeline
     * @param pointer - self explained
     */
    void setPeakDetector(PeakDetector *pointer);

private:

    int __loop(int d) const;
    int __seek(int d) const;
    void __init(double Tov_ms, double Tcn_ms, double Tlpf_ms, double dT_ms, ProcessType type);

    double *v_raw;
    double *v_time;
    double *v_Y;
    double *v_X;
    double *v_FA;
    int m_interval;
    int m_length;
    int m_filterlength;
    int curpos;
    double m_bottomFrequencyLimit;
    double m_topFrequencyLimit;    
    double m_snr;
    double m_Frequency;
    double m_dTms;

    cv::Mat v_datamat;
    cv::Mat v_dftmat;

    PeakDetector *pt_peakdetector = 0;
};

inline int PulseProcessor::__loop(int d) const
{
    return ((m_length + (d % m_length)) % m_length);
}

inline int PulseProcessor::__seek(int d) const
{
    return ((m_filterlength + (d % m_filterlength)) % m_filterlength);
}

}
//-------------------------------------------------------
#endif // PULSEPROCESSOR_H
