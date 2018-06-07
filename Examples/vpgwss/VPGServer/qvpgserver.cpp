#include "qvpgserver.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QWebSocket>
#include <QJsonDocument>

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
    }
}

QVPGServer::~QVPGServer()
{
    if(isListening()) {
        __decommutate();

        QTimer::singleShot(0,qvideosource,SLOT(close()));

        QTimer::singleShot(0,qvpgprocthread,SLOT(quit()));
        QTimer::singleShot(0,qfacetrackerthread,SLOT(quit()));
        QTimer::singleShot(0,qvideosourcethread,SLOT(quit()));        

        qvpgprocthread->wait();
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
    qfacetracker->setTargetSize(cv::Size(256,340));
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
    qvpgproc = new QVPGProcessor();
    qvpgprocthread = new QThread(this);
    qvpgproc->moveToThread(qvpgprocthread);
    connect(qvpgprocthread,SIGNAL(finished()),qvpgproc,SLOT(deleteLater()));
    connect(qvideosource,SIGNAL(fpsMeasured(double)),qvpgproc,SLOT(init(double)));
    qvpgprocthread->start();
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
    }
}

void QVPGServer::closeVideodevice()
{
    __decommutate();
    QTimer::singleShot(0,qvideosource,SLOT(close()));
}

void QVPGServer::openVideodevice(int _id)
{   
    QTimer::singleShot(0,qvpgproc,SLOT(deinit()));
    qvideosource->setVideodevID(_id);
    QTimer::singleShot(0,qvideosource,SLOT(open()));
    //QTimer::singleShot(0,qvideosource,SLOT(measureActualFPS()));
    qvideosource->measureActualFPS();
    __commutate();
}

void QVPGServer::__commutate()
{
    connect(qvideosource,SIGNAL(frameUpdated(cv::Mat)),qfacetracker,SLOT(updateImage(cv::Mat)),Qt::BlockingQueuedConnection);
    connect(qfacetracker,SIGNAL(frameProcessed(cv::Mat)),this,SLOT(sendFrame(cv::Mat)));
    connect(qfacetracker,SIGNAL(faceUpdated(cv::Mat,dlib::full_object_detection)),qvpgproc,SLOT(enrollFace(cv::Mat,dlib::full_object_detection)));
    connect(qvpgproc,SIGNAL(measurementsUpdated(QJsonObject)),this,SLOT(sendMeasurements(QJsonObject)));
}

void QVPGServer::__decommutate()
{
    disconnect(qvideosource,SIGNAL(frameUpdated(cv::Mat)),qfacetracker,SLOT(updateImage(cv::Mat)));
    disconnect(qfacetracker,SIGNAL(frameProcessed(cv::Mat)),this,SLOT(sendFrame(cv::Mat)));
    disconnect(qfacetracker,SIGNAL(faceUpdated(cv::Mat,dlib::full_object_detection)),qvpgproc,SLOT(enrollFace(cv::Mat,dlib::full_object_detection)));
    disconnect(qvpgproc,SIGNAL(measurementsUpdated(QJsonObject)),this,SLOT(sendMeasurements(QJsonObject)));
}

void QVPGServer::sendFrame(const cv::Mat &_faceimg)
{
    std::vector<uchar>  _encodedimg;
    cv::imencode("*.jpg",_faceimg,_encodedimg);
    websocket->sendBinaryMessage(QByteArray::fromRawData((const char *)&_encodedimg[0],_encodedimg.size()));    
}

void QVPGServer::sendMeasurements(const QJsonObject &_json)
{
    websocket->sendTextMessage(QJsonDocument(_json).toJson(QJsonDocument::Compact));
}

void QVPGServer::reportAboutError(const QString &_msg)
{
    qWarning("%s", _msg.toUtf8().constData());
}
