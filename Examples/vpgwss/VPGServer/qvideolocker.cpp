#include "qvideolocker.h"

QVideoLocker::QVideoLocker(QObject *parent) : QObject(parent),
    m_locked(false)
{    
}

void QVideoLocker::updateFrame(const cv::Mat &_framemat)
{
    if(m_locked == false) {
        __lock();
        Q_EMIT frameUpdated(_framemat);
    }
}

void QVideoLocker::unlock()
{
    __unlock();
}

void QVideoLocker::__lock()
{
    if(!m_locked) {
        m_locked = true;
    }
}

void QVideoLocker::__unlock()
{
    if(m_locked) {
        m_locked = false;
    }
}
