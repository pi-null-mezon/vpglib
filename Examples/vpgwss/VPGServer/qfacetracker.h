#ifndef QFACETRACKER_H
#define QFACETRACKER_H

#include <QObject>

#include <dlib/image_processing/shape_predictor.h>

#include "facetracker.h"
#include <opencv2/imgproc.hpp>

class QFaceTracker : public QObject
{
    Q_OBJECT
public:
    explicit QFaceTracker(uchar _length=4, FaceTracker::AlignMethod _method=FaceTracker::AlignMethod::Eyes, QObject *parent = 0);
    ~QFaceTracker();

signals:
    void faceUpdated(const cv::Mat &faceImg,const dlib::full_object_detection &faceshape);
    void frameProcessed(const cv::Mat &frame);

public slots:
    void updateImage(const cv::Mat &img);
    bool setFaceClassifier(cv::CascadeClassifier *pt);
    bool setEyeClassifier(cv::CascadeClassifier *pt);
    void setFaceShapePredictor(dlib::shape_predictor *pt);
    bool loadFaceClassifier(const QString &_filename);
    unsigned long loadFaceShapePredictor(const QString &_filename);
    void setTargetSize(const cv::Size &size);
    void setFaceRectPortions(float _xPortion, float _yPortion);
    void setFaceRectShifts(float _xShift, float _yShift);

private:
    cv::CascadeClassifier facecascadeclassifier;
    dlib::shape_predictor faceshapepredictor;
    FaceTracker *pt_tracker;
    cv::Size m_size;
};

inline void draw_eyes(cv::Mat &img, const dlib::full_object_detection& d)
{
    cv::Point _vl1(d.part(0).x(), d.part(0).y());
    cv::Point _vl2(d.part(1).x(), d.part(1).y());
    double _r = std::sqrt((_vl1.x-_vl2.x)*(_vl1.x-_vl2.x) + (_vl1.y-_vl2.y)*(_vl1.y-_vl2.y))/2.5;
    cv::Point _cp = (_vl1+_vl2)/2;
    cv::circle(img,_cp,_r,cv::Scalar(255,255,255),-1,CV_AA);
    cv::circle(img,_cp,_r,cv::Scalar(0,0,0),1,CV_AA);
    cv::circle(img,_cp,1,cv::Scalar(0,0,0),-1,CV_AA);

    cv::Point _vr1(d.part(2).x(), d.part(2).y());
    cv::Point _vr2(d.part(3).x(), d.part(3).y());
    _r = std::sqrt((_vr1.x-_vr2.x)*(_vr1.x-_vr2.x) + (_vr1.y-_vr2.y)*(_vr1.y-_vr2.y))/2.5;
    _cp = (_vr1+_vr2)/2;
    cv::circle(img,_cp,_r,cv::Scalar(255,255,255),-1,CV_AA);
    cv::circle(img,_cp,_r,cv::Scalar(0,0,0),1,CV_AA);
    cv::circle(img,_cp,1,cv::Scalar(0,0,0),-1,CV_AA);
}

// Grabbed from http://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/
inline void draw_polyline(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed = false)
{
    std::vector <cv::Point> points;
    for(int i = start; i <= end; ++i) {
        //cv::putText(img,std::to_string(i),cv::Point(d.part(i).x(), d.part(i).y()),CV_FONT_HERSHEY_SIMPLEX,0.4,cv::Scalar(0,0,255),1,CV_AA);
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(255,127,127), 1, CV_AA);
}

inline void render_face_shape(cv::Mat &img, const dlib::full_object_detection& d)
{
    DLIB_CASSERT (
         d.num_parts() == 68 || d.num_parts() == 5,
         "\n\t Invalid inputs were given to this function. "
         << "\n\t d.num_parts():  " << d.num_parts()
     );

    if(d.num_parts() == 68) {
        //draw_polyline(img, d, 0, 16);           // Jaw line
        draw_polyline(img, d, 17, 21);          // Left eyebrow
        draw_polyline(img, d, 22, 26);          // Right eyebrow
        draw_polyline(img, d, 27, 30);          // Nose bridge
        draw_polyline(img, d, 30, 35, true);    // Lower nose
        draw_polyline(img, d, 36, 41, true);    // Left eye
        draw_polyline(img, d, 42, 47, true);    // Right Eye
        draw_polyline(img, d, 48, 59, true);    // Outer lip
        draw_polyline(img, d, 60, 67, true);    // Inner lip
    } else if(d.num_parts() == 5) {
        draw_eyes(img, d);
        /*draw_polyline(img, d, 0, 1);    // Left eye
        draw_polyline(img, d, 2, 3);    // Right Eye
        draw_polyline(img, d, 4, 4);    // Lower nose*/
    }

}

#endif // QFACETRACKER_H
