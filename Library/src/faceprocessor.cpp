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

#define FACE_PROCESSOR_LENGTH 33

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
    m_minFaceSize = cv::Size(110,110);
}

FaceProcessor::~FaceProcessor()
{
    delete[] v_rects;
}

void FaceProcessor::enrollImage(const cv::Mat &rgbImage, float &resV, float &resT)
{
    cv::Mat img;
    float scaleX = 1.0f, scaleY = 1.0f;
    if(rgbImage.cols > 640 || rgbImage.rows > 480) {
        if( ((float)rgbImage.cols/rgbImage.rows) > 14.0/9.0 ) {
            cv::resize(rgbImage, img, cv::Size(640, 360), 0.0, 0.0, cv::INTER_AREA);
            scaleX = (float)rgbImage.cols / 640.0f;
            scaleY = (float)rgbImage.rows / 360.0f;
        } else if ( ((float)rgbImage.cols/rgbImage.rows) > 1.0) {
            cv::resize(rgbImage, img, cv::Size(640, 480), 0.0, 0.0, cv::INTER_AREA);
            scaleX = (float)rgbImage.cols / 640.0f;
            scaleY = (float)rgbImage.rows / 480.0f;
        } else if ( ((float)rgbImage.rows/rgbImage.cols) > 14.0/9.0) {
            cv::resize(rgbImage, img, cv::Size(360, 640), 0.0, 0.0, cv::INTER_AREA);
            scaleX = (float)rgbImage.cols / 360.0f;
            scaleY = (float)rgbImage.rows / 640.0f;
        } else {
            cv::resize(rgbImage, img, cv::Size(480, 640), 0.0, 0.0, cv::INTER_AREA);
            scaleX = (float)rgbImage.cols / 480.0f;
            scaleY = (float)rgbImage.rows / 640.0f;
        }
    } else {
        img = rgbImage;
    }

    std::vector<cv::Rect> faces;
    m_classifier.detectMultiScale(img, faces, 1.3, 5, cv::CASCADE_FIND_BIGGEST_OBJECT, m_minFaceSize, m_minFaceSize*4);

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
        cv::Mat region = cv::Mat(rgbImage, m_faceRect);
        int dX = W / 16;
        int dY = H / 30;
        // It will be rect inside m_faceRect
        m_ellRect = cv::Rect(dX, -6 * dY, W - 2 * dX, H + 6 * dY);
        int X = m_ellRect.x;
        W = m_ellRect.width;
        unsigned char *ptr;
		if (region.channels() == 1)
		{
			unsigned char tG = 0;
#pragma omp parallel for private(ptr,tG) reduction(+:area,green)
			for (int j = 0; j < H; j++)
			{
				ptr = region.ptr(j);
				for (int i = X; i < X + W; i++) {
					tG = ptr[3 * i + 1];
					if (__insideEllipse(i, j)) {
						area++;
						green += tG;
					}
				}
			}
		}
		else
		{
			unsigned char tR = 0, tG = 0, tB = 0;
#pragma omp parallel for private(ptr,tB,tG,tR) reduction(+:area,green)
			for (int j = 0; j < H; j++)
			{
				ptr = region.ptr(j);
				for (int i = X; i < X + W; i++)
				{
					tB = ptr[3*i];
					tG = ptr[3 * i + 1];
					tR = ptr[3*i+2];
					if (__skinColor(tR, tG, tB) && __insideEllipse(i, j)) {
						area++;
						green += tG;
					}
				}
			}
		}
    }

    resT = static_cast<float>(1000.0*(cv::getTickCount() -  m_markTime) / cv::getTickFrequency());
    m_markTime = cv::getTickCount();
    if(area > static_cast<unsigned long>(m_minFaceSize.area()/2))
        resV = static_cast<float>(green) / area;
    else
        resV = 0.0;    
}

void FaceProcessor::enrollImagePart(const cv::Mat &rgbImage, float &resRed, float &resGreen, float &resBlue, float &resT, cv::Rect roirect)
{
    if(roirect == cv::Rect()) {
        roirect = cv::Rect(0,0,rgbImage.cols,rgbImage.rows);
    } else {
        roirect = roirect & cv::Rect(0,0,rgbImage.cols,rgbImage.rows);
    }
    unsigned long red = 0;
    unsigned long green = 0;
    unsigned long blue = 0;
    unsigned long area = 0;
    if(roirect.area() > 0) {
        cv::Mat region = cv::Mat(rgbImage,roirect);
        unsigned char *ptr;
        unsigned char tR = 0, tG = 0, tB = 0;
        #pragma omp parallel for private(ptr,tB,tG,tR) reduction(+:area,green)
        for(int j = 0; j < roirect.height; j++) {
            ptr = region.ptr(j);
            for(int i = 0; i < roirect.width; i++) {
                tB = ptr[3*i];
                tG = ptr[3*i+1];
                tR = ptr[3*i+2];
                if( /*__skinColor(tR, tG, tB)*/ true) {
                    area++;
                    red   += tR;
                    green += tG;
                    blue  += tB;
                }
            }
        }
    }

    resT = static_cast<float>(1000.0*(cv::getTickCount() -  m_markTime) / cv::getTickFrequency());
    m_markTime = cv::getTickCount();
    if(area > 16) {
        resRed   = static_cast<float>(red)   / area;
        resGreen = static_cast<float>(green) / area;
        resBlue  = static_cast<float>(blue)  / area;
    } else {
        resRed   = 0.0f;
        resGreen = 0.0f;
        resBlue  = 0.0f;
    }
}

void FaceProcessor::enrollFace(const cv::Mat &rgbImage, float *v_resRed, float *v_resGreen, float *v_resBlue, float &resT)
{
    cv::Rect faceRect = cv::Rect(0,0,rgbImage.cols,rgbImage.rows);

    int X = faceRect.x;
    int Y = faceRect.y;
    int W = faceRect.width;
    int H = faceRect.height;
    int dY = H/30;
    // Clockwise, starts from left part of forehead
    unsigned long b[] = {0, 0, 0, 0};
    unsigned long g[] = {0, 0, 0, 0};
    unsigned long r[] = {0, 0, 0, 0};
    unsigned long a[] = {0, 0, 0, 0};

    if(faceRect.area() > 0) {
        cv::Mat region = cv::Mat(rgbImage,faceRect);
        m_ellRect = cv::Rect(X, Y - 6 * dY, W, H + 6 * dY);
        X = m_ellRect.x;
        W = m_ellRect.width;
        unsigned char *p;
        unsigned char tG = 0, tR = 0, tB = 0;
        for(int j = 0; j < Y + H; j++) {
            p = region.ptr(j); //takes pointer to beginning of data on row
            for(int i = X; i < X + W; i++) {
                tB = p[3*i];
                tG = p[3*i+1];
                tR = p[3*i+2];
                if(/*__skinColor(tR, tG, tB) &&*/ __insideEllipse(i, j)) {
                    if( j < Y + 2* H / 7 ) {
                        if(i < X + W / 2) {
                            b[3] += tB;
                            g[3] += tG;
                            r[3] += tR;
                            a[3]++;
                            //p[3*i] %= 32;
                            //p[3*i+2] %= 32;
                        } else if (i > X + W / 2) {
                            b[0] += tB;
                            g[0] += tG;
                            r[0] += tR;;
                            a[0]++;
                            //p[3*i] %= 32;
                        }
                    } else if( (j > Y + 3*H / 7) && (j < Y + 5*H / 7)) {
                        if( i < X + W / 2) {
                            b[2] += tB;
                            g[2] += tG;
                            r[2] += tR;
                            a[2]++;
                            //p[3*i+1] %= 32;
                        } else if (i > X + W / 2) {
                            b[1] += tB;
                            g[1] += tG;
                            r[1] += tR;
                            a[1]++;
                            //p[3*i+2] %= 32;
                        }
                    }
                }
            }
        }
    }

    resT = static_cast<float>(1000.0*(cv::getTickCount() -  m_markTime) / cv::getTickFrequency());
    m_markTime = cv::getTickCount();
    for(int i = 0; i < 4; i++) {
        if(a[i] > static_cast<unsigned long>(m_minFaceSize.area()/8)) {
            v_resBlue[i]  = static_cast<float>(b[i]) / a[i];
            v_resGreen[i] = static_cast<float>(g[i]) / a[i];
            v_resRed[i]   = static_cast<float>(r[i]) / a[i];
        } else {
            v_resBlue[i]  = 0.0f;
            v_resGreen[i] = 0.0f;
            v_resRed[i]   = 0.0f;
        }
    }
}


cv::Rect FaceProcessor::__getMeanRect() const
{
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
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

float FaceProcessor::measureFramePeriod(cv::VideoCapture *_vcptr)
{
    //Check if video source is opened
    if(_vcptr->isOpened() == false)
        return -1.0f;
    // We need to chek if videosource represents videofile or videocapturing device
    if(_vcptr->get(cv::CAP_PROP_FRAME_COUNT) <= 0) { // video source is a video device
        const int _iterations = 35, _framestodrop = 5;
        float _timeaccum = 0.0, _time = 0.0, _value = 0.0;
        cv::Mat _frame;
        dropTimer();
        for(int i = 0; i < _iterations; i++) {
            if(_vcptr->read(_frame)) {
                enrollImage(_frame,_value,_time);
                if(i >= _framestodrop) // exclude several frames in the begining because they could be delayed
                    _timeaccum += _time;                
            }
        }
        return _timeaccum / (_iterations - _framestodrop);
    } else { // video source is a video file
        return 1000.0f / static_cast<float>(_vcptr->get(cv::CAP_PROP_FPS));
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
        m_pos = (m_pos + 1) % FACE_PROCESSOR_LENGTH;
    } else {
        for(int i = 0; i < FACE_PROCESSOR_LENGTH; i++)
            v_rects[i] = rect;
    }
}

bool FaceProcessor::__insideEllipse(int x, int y) const
{
    float cx = (m_ellRect.x + m_ellRect.width / 2.0f - x) / (m_ellRect.width / 2.0f);
    float cy = (m_ellRect.y + m_ellRect.height / 2.0f - y) / (m_ellRect.height / 2.0f);
    if( (cx*cx + cy*cy) < 1.0f )
        return true;
    else
        return false;
}

cv::Rect FaceProcessor::getFaceRect() const
{
    return m_faceRect;
}

} // end of namespace vpg
