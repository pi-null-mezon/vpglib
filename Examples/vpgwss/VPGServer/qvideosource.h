#ifndef QVIDEOSOURCE_H
#define QVIDEOSOURCE_H

#include <QObject>
#include <QCamera>
#include <QTimer>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

class QVideoSource : public QObject
{
    Q_OBJECT
public:
    explicit QVideoSource(QObject *parent = 0);
    ~QVideoSource();

    int             getVideodevID() const;
    void            setVideodevID(int _id);

    double          measureActualFPS(unsigned int _howlong_to_measure_ms=3000);

signals:
    void error(const QString &_msg);
    void frameUpdated(const cv::Mat &_cvmat);
    void preparedForVPG();

public slots:    
    void open();
    void pause();
    void resume();
    void close();
    void init();

private slots:
    void _grabFrame();

private:
    QTimer *timer;
    cv::VideoCapture cvvideocapture;
    int devid;
};


#endif // QVIDEOSOURCE_H
