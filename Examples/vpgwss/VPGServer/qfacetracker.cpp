#include "qfacetracker.h"

#include <opencv2/highgui.hpp>

QFaceTracker::QFaceTracker(uchar _length, FaceTracker::AlignMethod _method, QObject *parent) : QObject(parent),
    m_strobeValue(1),
    f_gray(false)
{
    pt_tracker = new FaceTracker(_length, _method);
    qRegisterMetaType<cv::RotatedRect>("cv::RotatedRect");
}

QFaceTracker::~QFaceTracker()
{
    delete pt_tracker;
}

bool QFaceTracker::setFaceClassifier(cv::CascadeClassifier *pt)
{
    return pt_tracker->setFaceClassifier(pt);
}

bool QFaceTracker::setEyeClassifier(cv::CascadeClassifier *pt)
{
    return pt_tracker->setEyeClassifier(pt);
}

void QFaceTracker::setFaceShapePredictor(dlib::shape_predictor *pt)
{
    pt_tracker->setFaceShapePredictor(pt);
}

bool QFaceTracker::loadFaceClassifier(const QString &_filename)
{
    facecascadeclassifier.load(_filename.toStdString());
    if(facecascadeclassifier.empty() == false)
        qInfo("Face classifier has been sucessfully loaded");
    return setFaceClassifier(&facecascadeclassifier);
}

unsigned long QFaceTracker::loadFaceShapePredictor(const QString &_filename)
{
    try {
        dlib::deserialize(_filename.toStdString()) >> faceshapepredictor;
        qInfo("Face shape predictor num of parts: %d", (int)faceshapepredictor.num_parts());
        setFaceShapePredictor(&faceshapepredictor);
    }
    catch(...) {
        qCritical("Something goes wrong when QFaceTracker::loadFaceShapePredictor(...) has been called!");
    }
    return faceshapepredictor.num_parts();
}

void QFaceTracker::updateImage(const cv::Mat &img)
{    
    cv::Mat faceImage;
    pt_tracker->searchFace(img);
    if(f_gray == true)
        faceImage = pt_tracker->getResizedGrayFaceImage(img,m_size);
    else
        faceImage = pt_tracker->getResizedFaceImage(img,m_size);

    if(!faceImage.empty()) {
        if( ++m_counter % m_strobeValue == 0) {

            if(pt_tracker->getFaceAlignMethod() == FaceTracker::FaceShape) {

               auto _faceshape = pt_tracker->getFaceShape();
               auto _faceRrect = pt_tracker->getFaceRotatedRect();
               for(size_t i = 0; i < _faceshape.num_parts(); ++i) {
                   dlib::point _p = _faceshape.part(i), _tp;
                   float _anglerad = - CV_PI * _faceRrect.angle / 180.0f;
                   _tp.x() = _p.x()*std::cos(_anglerad) + _p.y()*std::sin(_anglerad);
                   _tp.y() = - _p.x()*std::sin(_anglerad) + _p.y()*std::cos(_anglerad);
                   _tp += dlib::point(_faceRrect.center.x,_faceRrect.center.y);
                   _faceshape.part(i) = _tp;
               }
               faceImage = img;
               render_face_shape(faceImage,_faceshape);
            }

            emit faceUpdated(faceImage);
            emit foundFace(true);
            emit faceCoordsUpdated(pt_tracker->getFaceRotatedRect(), cv::Rect(0,0,img.cols,img.rows));
        }        
    } else {
        if( ++m_counter % m_strobeValue == 0) {
            emit faceUpdated(img);
            emit foundFace(false);
        }
    }
}

void QFaceTracker::setStrobe(uint value)
{
    m_strobeValue = value;
}

uint QFaceTracker::getStrobe() const
{
    return m_strobeValue;
}

void QFaceTracker::setGrayMode(bool value)
{
    f_gray = value;
}

void QFaceTracker::setTargetSize(const cv::Size &size)
{
    m_size = size;
}

void QFaceTracker::setFaceRectPortions(float _xPortion, float _yPortion)
{
    pt_tracker->setFaceRectPortions(_xPortion, _yPortion);
}

void QFaceTracker::setFaceRectShifts(float _xShift, float _yShift)
{
    pt_tracker->setFaceRectShifts(_xShift, _yShift);
}


