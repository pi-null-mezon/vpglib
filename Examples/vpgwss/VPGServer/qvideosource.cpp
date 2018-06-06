#include "qvideosource.h"

#include <QElapsedTimer>
#include <QEventLoop>
#include <QCameraInfo>

#include <opencv2/imgproc.hpp>

QVideoSource::QVideoSource(QObject *parent) : QObject(parent),
    devid(-1)
{    
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<QImage::Format>("QImage::Format");    
}

QVideoSource::~QVideoSource()
{   
}

void QVideoSource::open()
{
    if(devid > -1) {
        if(cvvideocapture.open(devid)) {
            cvvideocapture.set(cv::CAP_PROP_FRAME_WIDTH, 640.0);
            cvvideocapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480.0);
            cvvideocapture.set(cv::CAP_PROP_FPS, 15.0);
            timer->start();
        } else {
            emit error(QString("QVideoSource::Error - Can not open video device with id %1!").arg(QString::number(devid)));
        }
    }
}

void QVideoSource::_grabFrame()
{
    if(cvvideocapture.isOpened()) {
        cv::Mat _frame;
        if(cvvideocapture.read(_frame)) {
            emit frameUpdated(_frame);
        }
    }
}

int QVideoSource::getVideodevID() const
{
    return devid;
}

void QVideoSource::setVideodevID(int _id)
{
    //qDebug("QVideoSource::Debug - videoodevice id has been set to %d",_id);
    devid = _id;
}

void QVideoSource::close()
{
    if(cvvideocapture.isOpened()) {
        timer->stop();
        cvvideocapture.release();
    }
}

void QVideoSource::pause()
{
    if(cvvideocapture.isOpened()) {
        timer->stop();
    }
}

void QVideoSource::resume()
{
    if(cvvideocapture.isOpened()) {
        timer->start();
    }
}

void QVideoSource::measureActualFPS(unsigned int _howlong_to_measure_ms)
{
    // Backend independent function
    QElapsedTimer _qeltimer;
    double _elapsedms = 0;
    uint  _frames = 0, _shift = 1;
    auto _moconn = connect(this, &QVideoSource::frameUpdated, [&]() {
        if(_frames == _shift) {
            _qeltimer.start();
        } else if(_frames > _shift){
            _elapsedms = _qeltimer.elapsed();
            //qDebug("%d) %f ms", _frames, _elapsedms);
        }
        _frames++;        
    });
    QEventLoop _el;
    QTimer::singleShot(_howlong_to_measure_ms,&_el,SLOT(quit()));
    _el.exec();
    disconnect(_moconn);
    double _actualfps = 1000.0*(_frames - _shift - 1)/_elapsedms;
    //qDebug("QVideoSource: actual FPS measured %.2f", _actualfps);
    if(!std::isinf(_actualfps))
        emit fpsMeasured(_actualfps);
}

void QVideoSource::init()
{
    timer = new QTimer();
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(30.0);
    connect(timer, SIGNAL(timeout()), this, SLOT(_grabFrame()));
}
