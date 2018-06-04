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

double QVideoSource::measureActualFPS(unsigned int _howlong_to_measure_ms)
{
    // Backend independent function
    QElapsedTimer _elapsedtimer;
    double _timeelapsed_ms = 0;
    unsigned int  _frames = 0;
    unsigned int  _startshift = 1;
    auto _moconn = connect(this, &QVideoSource::frameUpdated, [&]() {
        if(_frames == _startshift) {
            _elapsedtimer.start();
        } else if(_frames > _startshift) {
            _timeelapsed_ms = _elapsedtimer.elapsed();
            qInfo("%d) %f ms", _frames, _timeelapsed_ms);
        }
        _frames++;        
    });
    QEventLoop _el;
    QTimer::singleShot(_howlong_to_measure_ms,&_el,SLOT(quit()));
    _el.exec();
    disconnect(_moconn);
    double _actualfps = 1000.0*(_frames-_startshift-1)/_timeelapsed_ms;
    qInfo("QVideoSource: actual FPS measured %f", _actualfps);
    return _actualfps;
}

void QVideoSource::init()
{
    timer = new QTimer();
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(30.0);
    connect(timer, SIGNAL(timeout()), this, SLOT(_grabFrame()));
}
