#include "qvpgprocessor.h"

#include <QDateTime>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

#include <QJsonObject>
#include <QJsonDocument>

template <typename T>
std::string num2str(T value, unsigned char precision=1)
{
    std::string _fullstring = std::to_string(value);
    size_t _n = 0;
    for(size_t i = 0; i < _fullstring.size(); ++i) {
        if(_fullstring[i] == '.')
            break;
                _n++;
    }
    if(precision > 0) {
        _n += precision + 1;
    }
    return std::string(_fullstring.begin(), _fullstring.begin() + _n);
}

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const std::vector<const double *>_data, const int _datalength, double _ymax, double _ymin, const std::vector<cv::Scalar> &_colors)
{
    if((_datalength > 0) && (_windowsize.area() > 0) && (_data.size() > 0)  && (_data.size() == _colors.size())) {

        const cv::Scalar _backgroundcolor   = cv::Scalar(15,15,15);
        const cv::Scalar _coordcolor        = cv::Scalar(55,55,55);
        const cv::Scalar _fontcolor         = cv::Scalar(155,155,155);

        cv::Mat _colorplot = cv::Mat::zeros(_windowsize, CV_8UC3);
        cv::rectangle(_colorplot,cv::Rect(0,0,_colorplot.cols,_colorplot.rows),_backgroundcolor, -1);

        int _ticksX = 10;
        double _tickstepX = static_cast<double>(_windowsize.width)/ _ticksX ;
        for(int i = 1; i < _ticksX ; i++)
            cv::line(_colorplot, cv::Point2f(i*_tickstepX,0), cv::Point2f(i*_tickstepX,_colorplot.rows), _coordcolor, 1);

        int _ticksY = 8;
        double _tickstepY = static_cast<double>(_windowsize.height)/ _ticksY ;
        for(int i = 1; i < _ticksY ; i++) {
            cv::line(_colorplot, cv::Point2f(0,i*_tickstepY), cv::Point2f(_colorplot.cols,i*_tickstepY), _coordcolor, 1);
            cv::putText(_colorplot, num2str(_ymax - i * (_ymax-_ymin)/_ticksY), cv::Point(5, i*_tickstepY - 10), CV_FONT_HERSHEY_SIMPLEX, 0.4, _fontcolor, 1, CV_AA);
        }

        double invstepY = (_ymax - _ymin) / _windowsize.height;
        double stepX = static_cast<double>(_windowsize.width) / (_datalength - 1);

        for(size_t c = 0; c < _data.size(); ++c) {
            const double *_curve = _data[c];
            for(int i = 0; i < _datalength - 1; i++) {
                cv::line(_colorplot, cv::Point2f(i*stepX, _windowsize.height - (_curve[i] - _ymin)/invstepY),
                         cv::Point2f((i+1)*stepX, _windowsize.height - (_curve[i+1] - _ymin)/invstepY),
                        _colors[c], 1, CV_AA);
            }
        }
        cv::imshow(_title, _colorplot);
    }
}

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
    if(std::isinf(_fps) == false) {
        fps = _fps;
        faceproc = new vpg::FaceProcessor();
        pulseproc = new vpg::PulseProcessor(1000.0 / _fps);
        peakdet = new vpg::PeakDetector(pulseproc->getLength(), 27, 11, 1000.0 / _fps);
        pulseproc->setPeakDetector(peakdet);
        vhrhistory.resize(4);
        hrpos = 0;
        hr = 0;
        br = 0;
        for(int i = 0; i < 4; ++i) {
            vR[i] = 0;
            vG[i] = 0;
            vB[i] = 0;
        }
        initialized = true;
        qDebug("QVPGProcessor::Debug - initialized for fps %.2f, wait for measurements...", _fps);
        // Reset time
        hrPeriod = 1.25*pulseproc->getLength()*1000.0/fps; // huge initial delay saves from unstable measurements
        brPeriod = hrPeriod*1.75;
        faceproc->dropTimer();
    } else {
        qCritical("QVPGProcessor::Critiacal - can not initialize because of infinite fps!");
    }
}

void QVPGProcessor::deinit()
{
    qDebug("QVPGProcessor::Debug - deinitialized");
    initialized = false;
}

void QVPGProcessor::enrollFace(const cv::Mat &_mat, const dlib::full_object_detection &_faceshape)
{   
    if(initialized) {
        // Enroll face image
        faceproc->enrollFace(_mat,vR,vG,vB,time);
        double _dummytime;
        faceproc->enrollImagePart(_mat,red,green,blue,_dummytime);
        // Update vpg signal
        pulseproc->update(green,time);

        // For Debug purpose
        /*drawDataWindow("QVPGProcessor",cv::Size(640,240),
                       std::vector<const double*>(1,pulseproc->getSignal()),pulseproc->getLength(),
                       3.0,-3.0,std::vector<cv::Scalar>(1,cv::Scalar(0,255,0)));*/

        // Check if HR need to be updated
        hrPeriod -= time;
        if(hrPeriod < 0.0) {
            vhrhistory[hrpos] = pulseproc->computeFrequency();
            uint validvalues = 0;
            hr = 0.0;
            for(size_t i = 0; i < vhrhistory.size(); ++i) {
                if(vhrhistory[i] > 1.0) {
                    validvalues++;
                    hr += vhrhistory[i];
                }
            }            
            if(validvalues > 0) {
                hr /= validvalues;
            }

            hrPeriod = 1000.0;
            hrpos = (hrpos + 1) % vhrhistory.size();
        }

        // Check if BR need to be updated
        brPeriod -= time;
        if(brPeriod < 0.0) {
            br = -0.1 * peakdet->averageCardiointervalms(9) + 88.0;
            if(br < 6.0 )
                br = 6.0;
            brPeriod = 3000.0;
        }

        __meas2json(_faceshape);

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

void QVPGProcessor::__meas2json(const dlib::full_object_detection &_faceshape)
{
    QJsonArray _jshape;
    for(unsigned long i = 0; i < _faceshape.num_parts(); ++i) {
        QJsonObject _jpoint;
        _jpoint["x"] = static_cast<int>(_faceshape.part(i).x());
        _jpoint["y"] = static_cast<int>(_faceshape.part(i).y());
        _jshape.push_back(_jpoint);
    }
    QJsonArray _jcolors;
    for(int i = 0; i < 4; ++i) {
        QJsonObject _jcolor;
        _jcolor["r"] = vR[i];
        _jcolor["g"] = vG[i];
        _jcolor["b"] = vB[i];
        _jcolors.push_back(_jcolor);
    }
    QJsonObject _json;
    _json["faceshape"] = _jshape;
    _json["rawcolors"] = _jcolors;
    _json["hr"] = static_cast<int>(hr);
    _json["snr"] = pulseproc->getSNR();
    _json["br"] = static_cast<int>(br);
    _json["time"] = time;
    _json["vpg"] = pulseproc->getSignalSampleValue();
    emit measurementsUpdated(_json);
}

