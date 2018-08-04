#include "qfacetracker.h"

QFaceTracker::QFaceTracker(uchar _length, FaceTracker::AlignMethod _method, QObject *parent) : QObject(parent)
{
    pt_tracker = new FaceTracker(_length, _method);
    qRegisterMetaType<cv::RotatedRect>("cv::RotatedRect");
    qRegisterMetaType<dlib::full_object_detection>("dlib::full_object_detection");
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
        qDebug("Face classifier has been sucessfully loaded");
    return setFaceClassifier(&facecascadeclassifier);
}

unsigned long QFaceTracker::loadFaceShapePredictor(const QString &_filename)
{
    try {
        dlib::deserialize(_filename.toStdString()) >> faceshapepredictor;
        qDebug("Face shape predictor num of parts: %d", (int)faceshapepredictor.num_parts());
        setFaceShapePredictor(&faceshapepredictor);
    }
    catch(...) {
        qDebug("Something goes wrong when QFaceTracker::loadFaceShapePredictor(...) has been called!");
    }
    return faceshapepredictor.num_parts();
}

void QFaceTracker::updateImage(const cv::Mat &img)
{    
    cv::Mat frame = img.clone();
    cv::Mat faceImage = pt_tracker->getResizedFaceImage(img,m_size);

    if(!faceImage.empty()) {       
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
           //render_face_shape(frame,_faceshape);
           emit faceUpdated(img,_faceshape);
        } else {
            qDebug("QFaceTracker::Warning - selected face align method does not support face shape option!");
        }
    }
    emit frameProcessed(frame);
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


