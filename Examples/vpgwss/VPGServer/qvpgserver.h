#ifndef QVPGSERVER_H
#define QVPGSERVER_H

#include <QWebSocketServer>

#include <QThread>

#include "qvideosource.h"
#include "qfacetracker.h"
#include "qvpgprocessor.h"

class QVPGServer : public QObject
{
    Q_OBJECT
public:
    explicit QVPGServer(quint16 port, QObject *parent = nullptr);
    ~QVPGServer();

    bool isListening() const;
    quint16 serverPort() const;

signals:

private slots:
    void setupVideosource();
    void setupFaceTracker();
    void setupVPGProcessor();

    void enrollConnection();
    void parseCommand(const QString &_cmd);

    void closeVideodevice();
    void openVideodevice(int _id);

    void __commutate();
    void __decommutate();

    void sendFrame(const cv::Mat &_faceimg);
    void sendMeasurements(const QJsonObject &_json);

    void reportAboutError(const QString &_msg);

private:
    QWebSocketServer *qwssrv;
    QWebSocket *websocket;

    QVideoSource *qvideosource;
    QThread *qvideosourcethread;

    QFaceTracker *qfacetracker;
    QThread *qfacetrackerthread;

    QVPGProcessor *qvpgproc;
    QThread *qvpgprocthread;
};

#endif // QVPGSERVER_H
