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
#ifndef FACEPROCESSOR_H
#define FACEPROCESSOR_H
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
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui.hpp>
//-------------------------------------------------------
namespace vpg {
	
/**
 * @brief The FaceProcessor class should be used for PPG signal counts mining from the face video
 */
#ifndef VPG_BUILD_FROM_SOURCE
class DLLSPEC FaceProcessor
#else
class FaceProcessor
#endif
{
public:
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
     * @param rgbImage - input image, BGR format only
     * @param resV - where result count should be written
     * @param resT - where processing time should be written
     * @note  face detection will be performed inside the function
     */
    void enrollImage(const cv::Mat &rgbImage, float &resV, float &resT);
    /**
     * Enroll image's roi rect to produce PPG-signal count
     * @param rgbImage - input image, BGR format only
     * @param resRed - where result count should be written (red channel)
     * @param resGreen - where result count should be written (green channel)
     * @param resBlue - where result count should be written (blue channel)
     * @param resT - where processing time should be written
     * @note  no face detection will be performed!
     */
    void enrollImagePart(const cv::Mat &rgbImage, float &resRed, float &resGreen, float &resBlue, float &resT, cv::Rect roirect=cv::Rect());
    /**
     * Extract average colors from 4-part face partition
     * Pratition is clockwise, starts from left part of the forehead: |3|0|
     *                                                                |2|1|
     * @param rgbImage - input image of the face, BGR format only
     * @param v_resRed - pointer to 4 element vector for average red values
     * @param v_resGreen - pointer to 4 element vector for average red values
     * @param v_resBlue - pointer to 4 element vector for average red values
     * @param resT - where processing time should be written
     * @note  no face detection will be performed! It is your responsibility to provide face image
     */
    void enrollFace(const cv::Mat &rgbImage, float *v_resRed, float *v_resGreen, float *v_resBlue, float &resT);
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
    /**
     * @brief measureFramePeriod should be used to measure actual frame period (i.e. real fps) of video
     * @param _vcptr - pointe rto the target video capture (that will be used to VPG extraction)
     * @return average frame time in ms (use this value to instantiate PulseProcessor instance then)
     * @note VideoCapture object should be opened else -1.0 will be returned
     */
    float measureFramePeriod(cv::VideoCapture *_vcptr);
    /**
     * @brief dropTimer - call to drop the internal timer
     */
    void dropTimer();
    /**
     * @brief check if cascade classifier has been loaded
     * @return self explained
     */
    bool empty();

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

    cv::Rect __getMeanRect() const;
    void __updateRects(const cv::Rect &rect);
    bool __insideEllipse(int x, int y) const;
    bool __skinColor(unsigned char vR, unsigned char vG, unsigned char vB) const;
    void __init();
};

inline bool FaceProcessor::__skinColor(unsigned char vR, unsigned char vG, unsigned char vB) const
{
    if( (vR > 95) && (vR > vG) && (vG > 40) && (vB > 20) && ((vR - std::min(vG,vB)) > 5) && ((vR - vG) > 5) )
        return true;
    else
        return false;
}
}
//-------------------------------------------------------
#endif // FACEPROCESSOR_H
