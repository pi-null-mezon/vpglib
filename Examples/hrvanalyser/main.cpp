#include <iostream>
#include <opencv2/opencv.hpp>

#include "vpg.h"

template <typename T>
std::string num2str(T value, unsigned char precision=1);

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const double *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color);

int main(int argc, char *argv[])
{      
    // Try to open video capture device by Opencv's API
    cv::VideoCapture capture;
    if(argc > 1) {
        if(capture.open(argv[1]) == false) {
            std::cerr << "Can not open video file " << argv[1] << " Abort...";
            return -1;
        }
    } else if(capture.open(0) == false) {
        std::cerr << "Can not open video capture device. Abort...";
        return -2;
    }

    // Create FaceProcessor instance
    cv::String facecascadefilename;
    #ifdef CASCADE_FILENAME
        facecascadefilename = cv::String(CASCADE_FILENAME) + "haarcascade_frontalface_alt2.xml";
    #else
        facecascadefilename = "haarcascade_frontalface_alt2.xml";
    #endif
    vpg::FaceProcessor faceproc(facecascadefilename);
    if(faceproc.empty()) {
        std::cout << "Could not load cascade classifier! Abort...\n";
        return -1;
    }

    std::cout << "Measure frame period... " << std::endl;
    double framePeriod = faceproc.measureFramePeriod(&capture); // ms
    std::cout << framePeriod << " ms" << std::endl;
    vpg::PulseProcessor pulseproc(framePeriod); // note it is convinirnt to use default constructor only if frame period is near to 33 ms

    // Add peak detector for the cardio intervals evaluation and analysis
    int totalcardiointervals = 25;
    vpg::PeakDetector peakdetector(pulseproc.getLength(), totalcardiointervals, 11, framePeriod);
    pulseproc.setPeakDetector(&peakdetector);

    // Add HRVProcessor for HRV analysis
    vpg::HRVProcessor hrvproc;

    // Create local variables to store frame and processing values
    cv::Mat frame;
    unsigned int k = 0;
    double s = 0.0, t = 0.0, _snr = 0.0;
    int _hr = pulseproc.getFrequency();

    const double *signal = pulseproc.getSignal();
    int length = pulseproc.getLength();

    const double *cardiointervals = peakdetector.getIntervalsVector();
    int cardiointervalslength = peakdetector.getIntervalsLength();
    const double *binarysignal = peakdetector.getBinarySignal();

    std::cout << "Press escape to exit..." << std::endl;

    // Start video processing cycle
    cv::Rect faceRect;
    while(true) {
        if(capture.read(frame)) {

            // Essential part for the PPG signal extraction, only 2 strings should be called for the each new frame
            faceproc.enrollImage(frame, s, t);
            if(argc > 1) {
                pulseproc.update(s,framePeriod);
            } else {
                pulseproc.update(s,t);
            }

            faceRect = faceproc.getFaceRect();
            if(faceRect.area() > 0) {
                cv::rectangle(frame, faceRect, cv::Scalar(0,0,0), 1, CV_AA);
                cv::rectangle(frame, faceRect - cv::Point(1,1), cv::Scalar(255,255,255), 1, CV_AA);
                // Draw hr
                cv::String _hrstr = "HR(FFT): " + std::to_string(_hr) + " bpm";
                cv::putText(frame, _hrstr, cv::Point(20,40), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _hrstr, cv::Point(19,39), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255,255,255), 1, CV_AA);
                cv::String _snrstr = "SNR: " + num2str(_snr,2) + " dB";
                cv::putText(frame, _snrstr, cv::Point(20,60), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _snrstr, cv::Point(19,59), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);

                // Draw current cardio interval
                cv::String _avcardioint = "HR(HRV): " + num2str(60000.0 / peakdetector.averageCardiointervalms(9),0) + " bpm";
                cv::putText(frame, _avcardioint, cv::Point(20,120), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _avcardioint, cv::Point(19,119), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(127,0,127), 1, CV_AA);
                cv::String _cardiointerval = "Last interval: " + num2str(peakdetector.getCurrentInterval(),0) + " ms";
                cv::putText(frame, _cardiointerval, cv::Point(20,140), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _cardiointerval, cv::Point(19,139), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127,0,127), 1, CV_AA);


                // Draw cardio intervals history
                float xorigin = 10.0;
                float yorigin = frame.rows;
                float xstep = (float)(frame.cols - xorigin*2.0f) / (cardiointervalslength - 1);
                float ystep = (float)(frame.rows) / 4000.0f;
                for(int i = 0; i < cardiointervalslength-1; i++) {
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin - cardiointervals[i]*ystep), cv::Point(xorigin + (i+1)*xstep, yorigin - cardiointervals[(i+1)]*ystep),cv::Scalar(127,0,127),1,CV_AA);
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin), cv::Point(xorigin + i*xstep, yorigin - cardiointervals[i]*ystep),cv::Scalar(127,0,127),1,CV_AA);
                }
                cv::line(frame, cv::Point(xorigin + (cardiointervalslength-1)*xstep, yorigin), cv::Point(xorigin + (cardiointervalslength-1)*xstep, yorigin - cardiointervals[cardiointervalslength-1]*ystep),cv::Scalar(127,0,127),1,CV_AA);

                // Draw ppg-signal
                xstep = (float)(frame.cols - xorigin*2.0f) / (length - 1);
                ystep *= 100.0;
                yorigin = (frame.rows) * 0.9f;
                for(int i = 0; i < length-1; i++)
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin - signal[i]*ystep), cv::Point(xorigin + (i+1)*xstep, yorigin - signal[(i+1)]*ystep),cv::Scalar(0,200,0),1,CV_AA);

                // Draw binary-signal from PeakDetector
                for(int i = 0; i < length-1; i++)
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin - binarysignal[i]*ystep), cv::Point(xorigin + (i+1)*xstep, yorigin - binarysignal[(i+1)]*ystep),cv::Scalar(0,0,255),1,CV_AA);

                // Draw frame time
                cv::String _periodstr = num2str(t) + " ms";
                cv::putText(frame, _periodstr, cv::Point(20,frame.rows-10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _periodstr, cv::Point(19,frame.rows-11), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);
            }

            cv::imshow("Video probe", frame);
        }
        // After frame processing We could want to evaluate heart rate, here at each 32-th frame
        if(k % 33 == 0) {
            _hr = (int)pulseproc.computeFrequency();
            _snr = pulseproc.getSNR();
        }

        // Evaluate and draw HRV signal
        if(k % 5 == 0) {
            hrvproc.enrollIntervals(peakdetector.getIntervalsVector(), peakdetector.getIntervalsLength());
            drawDataWindow("HRV Signal, [milliseconds]",cv::Size(640,480),hrvproc.getHRVSignal(),hrvproc.getHRVSignalLength(), 1500.0, 0.0, cv::Scalar(0,127,255));
            //drawDataWindow("Amplitude Fourier spectrum of HRV Signal",cv::Size(640,480),hrvproc.getHRVAmplitudeSpectrum(),hrvproc.getHRVAmplitudeSpectrumLength(), 2.0e3, 0.0, cv::Scalar(0,0,255));
        }

        k++;
        // Process keyboard events
        int c = cv::waitKey(1);
        if( (char)c == 27 ) // 27 is escape ASCII code
            break;
        else switch(c) {
            case 's':
                capture.set(CV_CAP_PROP_SETTINGS,0.0);
                break;
        }
    }
    capture.release();
    return 0;
}

template <typename T>
std::string num2str(T value, unsigned char precision)
{
    std::string _fullstring = std::to_string(value);
    size_t _n = 0;
    for(size_t i = 0; i < _fullstring.size(); ++i) {
        _n++;
        if(_fullstring[i] == '.')
            break;
    }
    if(precision > 0) {
        _n += precision;
    } else {
        _n -= 1;
    }
    return std::string(_fullstring.begin(), _fullstring.begin() + _n);
}

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const double *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color)
{
    if(_datalength > 0 && _windowsize.area() > 0 && _data != NULL ) {

        cv::Mat _colorplot = cv::Mat::zeros(_windowsize, CV_8UC3);
        cv::rectangle(_colorplot,cv::Rect(0,0,_colorplot.cols,_colorplot.rows),cv::Scalar(15,15,15), -1);

        int _ticksX = 10;
        double _tickstepX = static_cast<double>(_windowsize.width)/ _ticksX ;
        for(int i = 1; i < _ticksX ; i++)
            cv::line(_colorplot, cv::Point2f(i*_tickstepX,0), cv::Point2f(i*_tickstepX,_colorplot.rows), cv::Scalar(100,100,100), 1);

        int _ticksY = 8;
        double _tickstepY = static_cast<double>(_windowsize.height)/ _ticksY ;
        for(int i = 1; i < _ticksY ; i++) {
            cv::line(_colorplot, cv::Point2f(0,i*_tickstepY), cv::Point2f(_colorplot.cols,i*_tickstepY), cv::Scalar(100,100,100), 1);
            cv::putText(_colorplot, num2str(_ymax - i * (_ymax-_ymin)/_ticksY), cv::Point(5, i*_tickstepY - 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(150,150,150), 1, CV_AA);
        }

        double invstepY = (_ymax - _ymin) / _windowsize.height;
        double stepX = static_cast<double>(_windowsize.width) / (_datalength - 1);

        for(int i = 0; i < _datalength - 1; i++) {
            cv::line(_colorplot, cv::Point2f(i*stepX, _windowsize.height - (_data[i] - _ymin)/invstepY),
                                 cv::Point2f((i+1)*stepX, _windowsize.height - (_data[i+1] - _ymin)/invstepY),
                                 _color, 1, CV_AA);
        }
        cv::imshow(_title, _colorplot);
    }
}
