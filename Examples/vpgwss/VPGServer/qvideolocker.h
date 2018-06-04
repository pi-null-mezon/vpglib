#ifndef QVIDEOLOCKER_H
#define QVIDEOLOCKER_H

#include <QObject>

#include <opencv2/core.hpp>

class QVideoLocker : public QObject
{
    Q_OBJECT
public:
    explicit QVideoLocker(QObject *parent = 0);

signals:
    void frameUpdated(const cv::Mat &_framemat);

public slots:
    void updateFrame(const cv::Mat &_framemat);
    void unlock();

private:
    void __lock();
    void __unlock();

    bool m_locked;
};

#endif // QVIDEOLOCKER_H
