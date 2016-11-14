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

#include "vpg.h"

#ifndef min
#define min(a,b) (((a) > (b))? (b) : (a))
#endif

#define FILTER_LENGTH 9

namespace vpg {

//---------------------------------PulseProcessor--------------------------------
PulseProcessor::PulseProcessor(double dT_ms, ProcessType type)
{
    switch(type){
        case HeartRate:
            __init(7777.0, 500.0, dT_ms, type);
            break;
    }
}

PulseProcessor::PulseProcessor(double Tov_ms, double Tcn_ms, double dT_ms, ProcessType type)
{
    __init(Tov_ms, Tcn_ms, dT_ms, type);
}

void PulseProcessor::__init(double Tov_ms, double Tcn_ms, double dT_ms, ProcessType type)
{
    m_length = static_cast<int>( Tov_ms / dT_ms );
    m_procType = type;
    curpos = 0;    

    switch(m_procType){
        case HeartRate:
            m_Frequency = 80.0;
            m_interval = static_cast<int>( Tcn_ms/ dT_ms );
            m_bottomFrequencyLimit = 0.8; // 48 bpm
            m_topFrequencyLimit = 3.0;    // 180 bpm
            break;        
    }

    v_raw = new double[m_length];
    v_Y = new double[m_length];
    v_time = new double[m_length];
    v_FA = new double[m_length/2 + 1];

    for(int i = 0; i < m_length; i++)  {
        v_raw[i] = (double)(i % 2);
        v_Y[i] = 0.0;
        v_time[i] = dT_ms;
    }
    v_X = new double[FILTER_LENGTH];
    for(int i = 0; i < FILTER_LENGTH; i ++)
		v_X[i] = (double)i;

    v_data = new cv::Mat(1, m_length, CV_64F);
    v_dft = new cv::Mat(1, m_length, CV_64F);     
}

PulseProcessor::~PulseProcessor()
{
    delete[] v_raw;
    delete[] v_Y;
    delete[] v_X;
    delete[] v_time;
    delete[] v_FA;
    delete v_dft;
    delete v_data;
}

void PulseProcessor::update(double value, double time)
{
    v_raw[curpos] = value;
    v_time[curpos] = time;

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
    for(int i = 0; i < FILTER_LENGTH; i++) {
        //integral += v_X[__seek(curpos - i)];
        // does the same as above if we add up along whole filter length
        integral += v_X[i];
    }

    v_Y[curpos] = ( integral + v_Y[__loop(curpos - 1)] )  / (FILTER_LENGTH + 1.0);

    curpos = (++curpos) % m_length;
}

double PulseProcessor::computeFrequency()
{
    double time = 0.0;
    for (int i = 0; i < m_length; i++) {
        time += v_time[i];
    }
    double *pt = v_data->ptr<double>(0);
    for(int i = 0; i < m_length; i++)
        pt[i] = v_Y[__loop(curpos - 1 - i)];
    cv::dft(*v_data, *v_dft);
    const double *v_fft = v_dft->ptr<const double>(0);

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
    for (int i = bottom; i <= top; i++)    {
        if ( (i >= i_maxpower - 2) && (i <= i_maxpower + 2) )       {
            signal_power += v_FA[i];
            signal_moment += i * v_FA[i];
        } else {
            noise_power += v_FA[i];
        }
    }

    m_snr = 0.0;
    if(signal_power > 0.01) {
        m_snr = 10.0 * std::log10( signal_power / noise_power );
        double bias = (double)i_maxpower - ( signal_moment / signal_power );
        m_snr *= (1.0 / (1.0 + bias*bias));
    }
    if(m_snr > 2.0)
        m_Frequency = (signal_moment / signal_power) * 60000.0 / time;

    return m_Frequency;
}

int PulseProcessor::getLength() const
{
    return m_length;
}

const double * PulseProcessor::getSignal() const
{
    return v_Y;
}

double PulseProcessor::getSNR() const
{
    return m_snr;
}

int PulseProcessor::__loop(int d) const
{
    return ((m_length + (d % m_length)) % m_length);
}

int PulseProcessor::__seek(int d) const
{
    return ((FILTER_LENGTH + (d % FILTER_LENGTH)) % FILTER_LENGTH);
}
//--------------------------------FaceProcessor--------------------------------

#define FACE_PROCESSOR_LENGTH 25

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
    if(f_firstface)
        m_markTime = cv::getTickCount();

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
    if( (vR > 95) && (vR > vG) && (vG > 40) && (vB > 20) && ((vR - min(vG,vB)) > 5) && ((vR - vG) > 5) )
        return true;
    else
        return false;
}

cv::Rect FaceProcessor::getFaceRect() const
{
    return m_faceRect;
}
//------------------------------End of FaceProcessor--------------------------------
} // end of namespace vpg
