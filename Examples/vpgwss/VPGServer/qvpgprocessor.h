#ifndef QVPGPROCESSOR_H
#define QVPGPROCESSOR_H

#include <QObject>

#include "vpg.h"

class QVPGProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QVPGProcessor(QObject *parent=nullptr);
    ~QVPGProcessor();        

signals:
    void measurementsUpdated();

public slots:
    void init(double _fps);
    void deinit();
    void enrollFace(const cv::Mat &_mat);

private:
    void __checkChannelsCorrelation();
    void __releaseMemory();

    vpg::FaceProcessor  *faceproc;
    vpg::PulseProcessor *pulseproc;
    vpg::PeakDetector   *peakdet;

    std::vector<double> vhrhistory;
    size_t hrpos;

    double fps, hrPeriod, brPeriod, vR[4], vG[4], vB[4], time;
    bool initialized;
};

#endif // QVPGPROCESSOR_H
