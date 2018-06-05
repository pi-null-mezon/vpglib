#include "qvpgprocessor.h"

#include <QDateTime>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

QVPGProcessor::QVPGProcessor(QObject *parent) : QObject(parent),
    faceproc(NULL),
    pulseproc(NULL),
    peakdet(NULL),
    initialized(false)
{
    qRegisterMetaType<cv::Rect>("cv::Rect");
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

QVPGProcessor::~QVPGProcessor()
{
    __releaseMemory();
}

void QVPGProcessor::init(double _fps)
{
    __releaseMemory();
    fps = _fps;
    faceproc = new vpg::FaceProcessor();
    pulseproc = new vpg::PulseProcessor(1000.0 / _fps);
    peakdet = new vpg::PeakDetector(pulseproc->getLength(), 27, 11, 1000.0 / _fps);
    pulseproc->setPeakDetector(peakdet);   
    vhrhistory.resize(4);
    hrpos = 0;
    initialized = true;
    qDebug("QVPGProcessor::Debug - initialized for fps %.2f, wait for measurements...", _fps);
    // Reset time
    hrPeriod = 1.25*pulseproc->getLength()*1000.0/fps; // huge initial delay saves from unstable measurements
    brPeriod = hrPeriod*1.75;
    faceproc->dropTimer();

}

void QVPGProcessor::deinit()
{
    initialized = false;
}


void QVPGProcessor::enrollFace(const cv::Mat &_mat)
{   
    if(initialized) {
        // Enroll face image
        faceproc->enrollFace(_mat, vR,vG,vB, time);
        // Update vpg signal
        pulseproc->update(vG[0]+vG[1]+vG[2]+vG[3],time);

        // Check if HR need to be updated
        hrPeriod -= time;
        if(hrPeriod < 0.0) {
            vhrhistory[hrpos] = pulseproc->computeFrequency();

            uint validvalues = 0;
            double _hr = 0.0;
            for(size_t i = 0; i < vhrhistory.size(); ++i) {
                if(vhrhistory[i] > 1.0) {
                    validvalues++;
                    _hr += vhrhistory[i];
                }
            }
            if(validvalues > 0) {
                qDebug("HR: %.1f", _hr/validvalues);
            }

            hrPeriod = 1000.0;
            hrpos = (hrpos + 1) % vhrhistory.size();
        }

        // Check if BR need to be updated
        brPeriod -= time;
        if(brPeriod < 0.0) {
            qreal _br = -0.1 * peakdet->averageCardiointervalms(9) + 88.0;
            if(_br < 6.0)
                _br = 6.0;
            qDebug("BR: %.1f", _br);
            brPeriod = 3000.0;
        }
    } else {
        qDebug("QVPGProcessor::Warning - processor is not initialized");
    }
}

void QVPGProcessor::__releaseMemory()
{
    initialized = false;
    delete faceproc;
    delete pulseproc;
    delete peakdet;
}

