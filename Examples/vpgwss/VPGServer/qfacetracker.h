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
    void faceUpdated(const cv::Mat &faceImg);
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

// Grabbed from http://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/
inline void draw_polyline(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed = false)
{
    std::vector <cv::Point> points;
    for(int i = start; i <= end; ++i) {
        //cv::putText(img,std::to_string(i),cv::Point(d.part(i).x(), d.part(i).y()),CV_FONT_HERSHEY_SIMPLEX,0.4,cv::Scalar(0,0,255),1,CV_AA);
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(0,255,0), 1, CV_AA);
}

inline void render_face_shape(cv::Mat &img, const dlib::full_object_detection& d)
{
    DLIB_CASSERT (
         d.num_parts() == 68 || d.num_parts() == 5,
         "\n\t Invalid inputs were given to this function. "
         << "\n\t d.num_parts():  " << d.num_parts()
     );

    if(d.num_parts() == 68) {
        draw_polyline(img, d, 0, 16);           // Jaw line
        draw_polyline(img, d, 17, 21);          // Left eyebrow
        draw_polyline(img, d, 22, 26);          // Right eyebrow
        draw_polyline(img, d, 27, 30);          // Nose bridge
        draw_polyline(img, d, 30, 35, true);    // Lower nose
        draw_polyline(img, d, 36, 41, true);    // Left eye
        draw_polyline(img, d, 42, 47, true);    // Right Eye
        draw_polyline(img, d, 48, 59, true);    // Outer lip
        draw_polyline(img, d, 60, 67, true);    // Inner lip
    } else if(d.num_parts() == 5) {
        draw_polyline(img, d, 0, 1);    // Left eye
        draw_polyline(img, d, 2, 3);    // Right Eye
        draw_polyline(img, d, 4, 4);    // Lower nose
    }

}

#endif // QFACETRACKER_H
