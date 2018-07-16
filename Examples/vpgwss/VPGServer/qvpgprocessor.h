#ifndef QVPGPROCESSOR_H
#define QVPGPROCESSOR_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include <dlib/image_processing/shape_predictor.h>

#include "vpg.h"

class QVPGProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QVPGProcessor(QObject *parent=nullptr);
    ~QVPGProcessor();        

signals:
    void measurementsUpdated(const QJsonObject &json);

public slots:
    void init(double _fps);
    void deinit();
    void enrollFace(const cv::Mat &_mat, const dlib::full_object_detection &_faceshape);

private:
    void __checkChannelsCorrelation();
    void __releaseMemory();
    void __meas2json(const dlib::full_object_detection &_faceshape);

    vpg::FaceProcessor  *faceproc;
    vpg::PulseProcessor *pulseproc;
    vpg::PeakDetector   *peakdet;
    vpg::HRVProcessor   *hrvproc;

    std::vector<double> vhrhistory;
    size_t hrpos;

    double fps, hrPeriod, brPeriod, vR[4], vG[4], vB[4], red, green ,blue, reflectance, hr, br, time;
    bool initialized;
};

#endif // QVPGPROCESSOR_H
