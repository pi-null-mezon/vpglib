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
#include "faceprocessor.h"

namespace vpg {

FaceProcessor::FaceProcessor(const std::string &filename)
{
    __init();
    loadClassifier(filename);
}

FaceProcessor::FaceProcessor()
{
    __init();
}

void FaceProcessor::__init()
{
    v_rects = new cv::Rect[FACE_PROCESSOR_LENGTH];
    m_pos = 0;
    m_nofaceframes = 0;
    f_firstface = true;
    m_minFaceSize = cv::Size(100,120);
    m_blurSize = cv::Size(3,3);
}

FaceProcessor::~FaceProcessor()
{
    delete[] v_rects;
}

void FaceProcessor::enrollImage(const cv::Mat &rgbImage, double &resV, double &resT)
{
    cv::Mat img;
    double scaleX = 1.0, scaleY = 1.0;
    if(rgbImage.cols > 640 || rgbImage.rows > 480) {
        if( ((float)rgbImage.cols/rgbImage.rows) > 14.0/9.0 ) {
            cv::resize(rgbImage, img, cv::Size(640, 360), 0.0, 0.0, CV_INTER_AREA);
            scaleX = (double)rgbImage.cols / 640.0;
            scaleY = (double)rgbImage.rows / 360.0;
        } else {
            cv::resize(rgbImage, img, cv::Size(640, 480), 0.0, 0.0, CV_INTER_AREA);
            scaleX = (double)rgbImage.cols / 640.0;
            scaleY = (double)rgbImage.rows / 480.0;
        }
    } else
        img = rgbImage;

    std::vector<cv::Rect> faces;
    m_classifier.detectMultiScale(img, faces, 1.15, 5, cv::CASCADE_FIND_BIGGEST_OBJECT, m_minFaceSize);

    if(faces.size() > 0) {
        __updateRects(faces[0]);
        m_nofaceframes = 0;
        f_firstface = false;
    } else {
        m_nofaceframes++;
        if(m_nofaceframes == FACE_PROCESSOR_LENGTH) {
            f_firstface = true;
            __updateRects(cv::Rect(0,0,0,0));
        }
    }

    cv::Rect tempRect = __getMeanRect();
    m_faceRect = cv::Rect((int)(tempRect.x*scaleX), (int)(tempRect.y*scaleY), (int)(tempRect.width*scaleX), (int)(tempRect.height*scaleY))
                 & cv::Rect(0, 0, rgbImage.cols, rgbImage.rows);

    int W = m_faceRect.width;
    int H = m_faceRect.height;
    unsigned long green = 0;
    unsigned long area = 0;

    if(m_faceRect.area() > 0 && m_nofaceframes < FACE_PROCESSOR_LENGTH) {
        cv::Mat region = cv::Mat(rgbImage, m_faceRect).clone();
        cv::blur(region,region, m_blurSize);
        int dX = W / 16;
        int dY = H / 30;
        // It will be rect inside m_faceRect
        m_ellRect = cv::Rect(dX, -6 * dY, W - 2 * dX, H + 6 * dY);
        int X = m_ellRect.x;
        W = m_ellRect.width;
        unsigned char *ptr;
        unsigned char tR = 0, tG = 0, tB = 0;
        #pragma omp parallel for private(ptr,tB,tG,tR) reduction(+:area,green)
        for(int j = 0; j < H; j++) {
            ptr = region.ptr(j);
            for(int i = X; i < X + W; i++) {
                tB = ptr[3*i];
                tG = ptr[3*i+1];
                tR = ptr[3*i+2];
                if( __skinColor(tR, tG, tB) && __insideEllipse(i, j)) {
                    area++;
                    green += tG;
                }
            }
        }
    }

    resT = ((double)cv::getTickCount() -  (double)m_markTime)*1000.0 / cv::getTickFrequency();
    m_markTime = cv::getTickCount();
    if(area > static_cast<unsigned long>(m_minFaceSize.area()/2)) {
        resV = (double)green / area;
    } else {
        resV = 0.0;
    }
}

cv::Rect FaceProcessor::__getMeanRect() const
{
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    for(int i = 0; i < FACE_PROCESSOR_LENGTH; i++) {
        x += v_rects[i].x;
        y += v_rects[i].y;
        w += v_rects[i].width;
        h += v_rects[i].height;
    }
    x /= FACE_PROCESSOR_LENGTH;
    y /= FACE_PROCESSOR_LENGTH;
    w /= FACE_PROCESSOR_LENGTH;
    h /= FACE_PROCESSOR_LENGTH;
    return cv::Rect((int)x, (int)y, (int)w, (int)h);
}


bool FaceProcessor::loadClassifier(const std::string &filename)
{
    return m_classifier.load(filename);
}

double FaceProcessor::measureFramePeriod(cv::VideoCapture *_vcptr)
{
    //Check if video source is opened
    if(_vcptr->isOpened() == false)
        return -1.0;

    if(_vcptr->get(cv::CAP_PROP_POS_MSEC) == -1) { // if the video source is a video device

        int _iterations = 35;
        double _timeaccum = 0.0, _time = 0.0, _value = 0.0;
        cv::Mat _frame;
        dropTimer();
        for(int i = 0; i < _iterations; i++) {
            if(_vcptr->read(_frame)) {
                enrollImage(_frame,_value,_time);
                if(i > 4) { // exclude first counts that could be delayed
                    _timeaccum += _time;
                }
            }
        }
        return _timeaccum / (_iterations - 5);
    } else { // if the video source is a video file
        return 1000.0 / _vcptr->get(cv::CAP_PROP_FPS);
    }
}

void FaceProcessor::dropTimer()
{
    m_markTime = cv::getTickCount();
}

bool FaceProcessor::empty()
{
    return m_classifier.empty();
}

void FaceProcessor::__updateRects(const cv::Rect &rect)
{
    if(f_firstface == false){
        v_rects[m_pos] = rect;
        m_pos = (++m_pos) % FACE_PROCESSOR_LENGTH;
    } else {
        for(int i = 0; i < FACE_PROCESSOR_LENGTH; i++)
            v_rects[i] = rect;
    }
}

bool FaceProcessor::__insideEllipse(int x, int y) const
{
    double cx = (m_ellRect.x + m_ellRect.width / 2.0 - x) / (m_ellRect.width / 2.0);
    double cy = (m_ellRect.y + m_ellRect.height / 2.0 - y) / (m_ellRect.height / 2.0);
    if( (cx*cx + cy*cy) < 1.0 )
        return true;
    else
        return false;
}

bool FaceProcessor::__skinColor(unsigned char vR, unsigned char vG, unsigned char vB) const
{
    if( (vR > 95) && (vR > vG) && (vG > 40) && (vB > 20) && ((vR - std::min(vG,vB)) > 5) && ((vR - vG) > 5) )
        return true;
    else
        return false;
}

cv::Rect FaceProcessor::getFaceRect() const
{
    return m_faceRect;
}

} // end of namespace vpg
