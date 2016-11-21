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

/**
 * @file vpg.h
 * @author Taranov Alex <pi-null-mezon@yandex.ru>
 * @version 1.0.1.0
 *
 * Library was designed for one special purpose - to measure heart rate from face video.
 * Pay attention that ordinary PC is not certified as measurement tool, so measurement
 * error could be high. Use library at your own risk, no warranties are granted.
 *
 * @section DESCRIPTION
 */

#ifndef VPG_H
#define VPG_H

#include "opencv2/core.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"

#ifdef DLL_BUILD_SETUP
    #define DLLSPEC __declspec(dllexport)
#else
    #define DLLSPEC __declspec(dllimport)
#endif

/**
 * vpg namespace represents classes for ppg signal from video processing
 */
namespace vpg {
//-------------------------------------------------------
/**
 * The PulseProcessor class process ppg signal to measure pulse rate
 */
class DLLSPEC PulseProcessor
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
     * @brief get pointer to signal counts
     * @return pointer to data
     */
    const double *getSignal() const;
    /**
     * @brief get snr value
     * @return relation between pulse and noise harmonics energies
     */
    double getSNR() const;

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

    cv::Mat v_datamat;
    cv::Mat v_dftmat;
};
//-------------------------------------------------------
/**
 * The FaceProcessor class process face image into ppg signal
 */
class DLLSPEC FaceProcessor
{
public:     
    /**
     * Default class constructor
     */
    FaceProcessor();
    /**
     * Overloaded class constructor
     * @param filename - name of file for cv::CascadeClassifier class
     */
    FaceProcessor(const std::string &filename);

    /**
     * Class destructor
     */
    virtual ~FaceProcessor();
    /**
     * Enroll image to produce PPG-signal count
     * @param rgb - input image, BGR format only
     * @param resV - where result count should be written
     * @param resT - where processing time should be written
     */
    void enrollImage(const cv::Mat &rgbImage, double &resV, double &resT);
    /**
     * Get cv::Rect that bounds face on image
     * @return coordinates of face on image in cv::Rect form
     */
    cv::Rect getFaceRect() const;
    /**
     * Load cv::CascadeClassifier face pattern from a file
     * @param filename - name of file for cv::CascadeClassifier class
     * @return was file loaded or not
     */
    bool loadClassifier(const std::string &filename);

private:
    cv::CascadeClassifier m_classifier;
    cv::Rect *v_rects;
    cv::Rect m_ellRect;
    int64 m_markTime;
    unsigned int m_pos;
    uchar m_nofaceframes;
    bool f_firstface;
    cv::Rect m_faceRect;
    cv::Size m_minFaceSize;
    cv::Size m_blurSize;

    cv::Rect __getMeanRect() const;
    void __updateRects(const cv::Rect &rect);
    bool __insideEllipse(int x, int y) const;
    bool __skinColor(unsigned char vR, unsigned char vG, unsigned char vB) const;
    void __init();
};
//-------------------------------------------------------
} // end of namespace vpg*/

#endif



