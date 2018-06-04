#include "qvpgserver.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QWebSocket>

#include <opencv2/imgcodecs.hpp>

QVPGServer::QVPGServer(quint16 port, QObject *parent) : QObject(parent),
    websocket(nullptr)
{
    qwssrv = new QWebSocketServer(APP_NAME,QWebSocketServer::NonSecureMode,this);
    if(qwssrv->listen(QHostAddress::LocalHost,port)) {
        setupVideosource();
        setupFaceTracker();
        setupVPGProcessor();
        connect(qwssrv,SIGNAL(newConnection()),SLOT(enrollConnection()));
        qInfo("%s listening incoming connections on ws://localhost:%d", APP_NAME, serverPort());
    }
}

QVPGServer::~QVPGServer()
{
    if(isListening()) {
        __decommutate();
        QTimer::singleShot(0,qvideosource,SLOT(close()));
        QTimer::singleShot(0,qfacetrackerthread,SLOT(quit()));
        QTimer::singleShot(0,qvideosourcethread,SLOT(quit()));

        qfacetrackerthread->wait();
        qvideosourcethread->wait();
    }
}

bool QVPGServer::isListening() const
{
    return qwssrv->isListening();
}

quint16 QVPGServer::serverPort() const
{
    return qwssrv->serverPort();
}

void QVPGServer::setupVideosource()
{
    qvideosource = new QVideoSource();
    qvideosourcethread = new QThread(this);
    qvideosource->moveToThread(qvideosourcethread);
    connect(qvideosourcethread,SIGNAL(started()),qvideosource,SLOT(init()));
    connect(qvideosourcethread,SIGNAL(finished()),qvideosource,SLOT(deleteLater()));
    connect(qvideosource,SIGNAL(error(QString)),this,SLOT(reportAboutError(QString)));
    qvideosourcethread->start();
}

void QVPGServer::setupFaceTracker()
{
    qfacetracker = new QFaceTracker(4,FaceTracker::AlignMethod::FaceShape);
    qfacetracker->setTargetSize(cv::Size(150,200));
    bool _isloaded = qfacetracker->loadFaceClassifier(qApp->applicationDirPath().append("/haarcascade_frontalface_alt2.xml"));
    assert(_isloaded);
    unsigned long _numparts = qfacetracker->loadFaceShapePredictor(qApp->applicationDirPath().append("/shape_predictor_5_face_landmarks.dat"));
    assert(_numparts);
    qfacetrackerthread = new QThread(this);
    qfacetracker->moveToThread(qfacetrackerthread);
    connect(qfacetrackerthread,SIGNAL(finished()),qfacetracker,SLOT(deleteLater()));
    qfacetrackerthread->start();
}

void QVPGServer::setupVPGProcessor()
{

}

void QVPGServer::enrollConnection()
{
    if(websocket != nullptr) {
        websocket->close();
        websocket->deleteLater();
        websocket = nullptr;
    }

    websocket = qwssrv->nextPendingConnection();
    websocket->setParent(this);

    connect(websocket,SIGNAL(textMessageReceived(QString)),this,SLOT(parseCommand(QString)));
    connect(websocket,SIGNAL(disconnected()),this,SLOT(closeVideodevice()));
}

void QVPGServer::parseCommand(const QString &_cmd)
{
    qDebug("QVPGServer::Debug - incoming command \"%s\"",_cmd.toUtf8().constData());

    if(_cmd.contains("openvideodev")) {
        openVideodevice(_cmd.section('(',1,1).section(')',0,0).toInt());
    } else if(_cmd.contains("closevideodev")) {
        closeVideodevice();
    } else if(_cmd.contains("setimgsize")) {
        int _w = _cmd.section('(',1,1).section(',',0,0).toInt();
        int _h = _cmd.section(',',1,1).section(')',0,0).toInt();
        qfacetracker->setTargetSize(cv::Size(_w,_h));
    }
}

void QVPGServer::closeVideodevice()
{
    __decommutate();
    QTimer::singleShot(0,qvideosource,SLOT(close()));
}

void QVPGServer::openVideodevice(int _id)
{
    qvideosource->setVideodevID(_id);
    QTimer::singleShot(0,qvideosource,SLOT(open()));
    __commutate();
}

void QVPGServer::__commutate()
{
    connect(qvideosource,SIGNAL(frameUpdated(cv::Mat)),qfacetracker,SLOT(updateImage(cv::Mat)),Qt::BlockingQueuedConnection);
    connect(qfacetracker,SIGNAL(faceUpdated(cv::Mat)),this,SLOT(sendFaceImage(cv::Mat)));
}

void QVPGServer::__decommutate()
{
    disconnect(qvideosource,SIGNAL(frameUpdated(cv::Mat)),qfacetracker,SLOT(updateImage(cv::Mat)));
    disconnect(qfacetracker,SIGNAL(faceUpdated(cv::Mat)),this,SLOT(sendFaceImage(cv::Mat)));
}

void QVPGServer::sendFaceImage(const cv::Mat &_faceimg)
{
    std::vector<uchar>  _encodedimg;
    cv::imencode("*.jpg",_faceimg,_encodedimg);
    websocket->sendBinaryMessage(QByteArray::fromRawData((const char *)&_encodedimg[0],_encodedimg.size()));
}

void QVPGServer::reportAboutError(const QString &_msg)
{
    qWarning("%s", _msg.toUtf8().constData());
}
