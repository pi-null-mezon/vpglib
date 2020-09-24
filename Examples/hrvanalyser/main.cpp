#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio_c.h>

#include "vpg.h"

template <typename T>
std::string num2str(T value, unsigned char precision=1);

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const float *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color);

int main(int argc, char *argv[])
{          
    // Create FaceProcessor instance (it is needed to detect face and compute average skin reflection)
    cv::String facecascadefilename = "haarcascade_frontalface_alt2.xml";
    vpg::FaceProcessor faceproc(facecascadefilename);
    if(faceproc.empty()) {
        std::cout << "Could not load '" << facecascadefilename << "'! Abort...\n";
        return -1;
    }

    // Try to open default video capture device by Opencv's API
    cv::VideoCapture capture;
    if(argc > 1) {
        // Note that, video files that use compression codecs (like MPEG etc.) do not contain skin reflection changes
        if(capture.open(argv[1]) == false) {
            std::cerr << "Can not open video file " << argv[1] << " Abort...";
            return 1;
        }
    } else if(capture.open(0) == false) {
        std::cerr << "Can not open video capture device. Abort...";
        return 2;
    }

    // Now we should measure frame rate of the video
    std::cout << "Measuring actual frame period. Please wait... " << std::endl;
    float framePeriod = faceproc.measureFramePeriod(&capture); // ms
    std::cout << "  frame period: " << framePeriod << " ms" << std::endl;

    // Let's create instance of PulseProcessor (it analyzes counts of skin reflection and computes heart rate by means on FFT analysis)
    vpg::PulseProcessor pulseproc(framePeriod);

    // Add peak detector for the cardio intervals evaluation and analysis (it analyzes cardio intervals)
    int totalcardiointervals = 25;
    vpg::PeakDetector peakdetector(pulseproc.getLength(), totalcardiointervals, 11, framePeriod);
    pulseproc.setPeakDetector(&peakdetector);

    // Add HRVProcessor for HRV analysis
    vpg::HRVProcessor hrvproc;

    // Create local variables to store frame and processing values
    cv::Mat frame;
    unsigned int k = 0;
    float s = 0.0, t = 0.0, _snr = 0.0;
    int _hr = (int)pulseproc.getFrequency();

    const float *signal = pulseproc.getSignal();
    int length = pulseproc.getLength();

    const float *cardiointervals = peakdetector.getIntervalsVector();
    int cardiointervalslength = peakdetector.getIntervalsLength();
    const float *binarysignal = peakdetector.getBinarySignal();

    std::cout << "Press escape to exit..." << std::endl;

    // Start video processing cycle
    cv::Rect faceRect;
    while(true) {
        if(capture.read(frame)) {

            // Essential part for the PPG signal extraction, only 2 strings should be called for the each new frame
            faceproc.enrollImage(frame, s, t);
            if(argc > 1)
                pulseproc.update(s,framePeriod); // video file have fixed frame time
            else
                pulseproc.update(s,t);

            faceRect = faceproc.getFaceRect();
            if(faceRect.area() > 0) {
                cv::rectangle(frame, faceRect, cv::Scalar(0,0,0), 1, cv::LINE_AA);
                cv::rectangle(frame, faceRect - cv::Point(1,1), cv::Scalar(255,255,255), 1, cv::LINE_AA);
                // Draw hr
                cv::String _hrstr = "HR(FFT): " + std::to_string(_hr) + " bpm";
                cv::putText(frame, _hrstr, cv::Point(20,40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 1, cv::LINE_AA);
                cv::putText(frame, _hrstr, cv::Point(19,39), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255,255,255), 1, cv::LINE_AA);
                cv::String _snrstr = "SNR: " + num2str(_snr,2) + " dB";
                cv::putText(frame, _snrstr, cv::Point(20,60), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, cv::LINE_AA);
                cv::putText(frame, _snrstr, cv::Point(19,59), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, cv::LINE_AA);

                // Draw current cardio interval
                cv::String _avcardioint = "HR(HRV): " + num2str(60000.0 / peakdetector.averageCardiointervalms(9),0) + " bpm";
                cv::putText(frame, _avcardioint, cv::Point(20,120), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 1, cv::LINE_AA);
                cv::putText(frame, _avcardioint, cv::Point(19,119), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(127,0,127), 1, cv::LINE_AA);
                cv::String _cardiointerval = "Last interval: " + num2str(peakdetector.getCurrentInterval(),0) + " ms";
                cv::putText(frame, _cardiointerval, cv::Point(20,140), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, cv::LINE_AA);
                cv::putText(frame, _cardiointerval, cv::Point(19,139), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127,0,127), 1, cv::LINE_AA);

                // Draw cardio intervals history
                float xorigin = 10.0;
                float yorigin = (float)frame.rows;
                float xstep = (frame.cols - xorigin*2.0f) / (cardiointervalslength - 1);
                float ystep = frame.rows / 4000.0f;
                for(int i = 0; i < cardiointervalslength-1; i++) {
                    cv::line(frame, cv::Point2f(xorigin + i*xstep, yorigin - (float)cardiointervals[i]*ystep), cv::Point2f(xorigin + (i+1)*xstep, yorigin - (float)cardiointervals[(i+1)]*ystep),cv::Scalar(127,0,127),1,cv::LINE_AA);
                    cv::line(frame, cv::Point2f(xorigin + i*xstep, yorigin), cv::Point2f(xorigin + i*xstep, yorigin - (float)cardiointervals[i]*ystep),cv::Scalar(127,0,127),1,cv::LINE_AA);
                }
                cv::line(frame, cv::Point2f(xorigin + (cardiointervalslength-1)*xstep, yorigin), cv::Point2f(xorigin + (cardiointervalslength-1)*xstep, yorigin - (float)cardiointervals[cardiointervalslength-1]*ystep),cv::Scalar(127,0,127),1,cv::LINE_AA);

                // Draw ppg-signal
                xstep = (float)(frame.cols - xorigin*2.0f) / (length - 1);
                ystep *= 100.0;
                yorigin = (frame.rows) * 0.9f;
                for(int i = 0; i < length-1; i++)
                    cv::line(frame, cv::Point2f(xorigin + i*xstep, yorigin - (float)signal[i]*ystep), cv::Point2f(xorigin + (i+1)*xstep, yorigin - (float)signal[(i+1)]*ystep),cv::Scalar(0,200,0),1,cv::LINE_AA);

                // Draw binary-signal from PeakDetector
                for(int i = 0; i < length-1; i++)
                    cv::line(frame, cv::Point2f(xorigin + i*xstep, yorigin - (float)binarysignal[i]*ystep), cv::Point2f(xorigin + (i+1)*xstep, yorigin - (float)binarysignal[(i+1)]*ystep),cv::Scalar(0,0,255),1,cv::LINE_AA);

                // Draw frame time
                cv::String _periodstr = num2str(t) + " ms, press 's' to open capture device settings dialog";
                cv::putText(frame, _periodstr, cv::Point(20,frame.rows-10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, cv::LINE_AA);
                cv::putText(frame, _periodstr, cv::Point(19,frame.rows-11), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, cv::LINE_AA);
            }

            cv::imshow("Video probe", frame);
        } else {
            break;
        }
        // After frame processing We could want to evaluate heart rate, here at each 32-th frame
        if(k % 33 == 0) {
            _hr = (int)pulseproc.computeFrequency();
            _snr = pulseproc.getSNR();
        }

        // Evaluate and draw HRV signal
        if(k % 33 == 0) {
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
                capture.set(cv::CAP_PROP_SETTINGS,0.0);
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

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const float *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color)
{
    if(_datalength > 0 && _windowsize.area() > 0 && _data != NULL ) {

        cv::Mat _colorplot = cv::Mat::zeros(_windowsize, CV_8UC3);
        cv::rectangle(_colorplot,cv::Rect(0,0,_colorplot.cols,_colorplot.rows),cv::Scalar(15,15,15), -1);

        int _ticksX = 10;
        float _tickstepX = static_cast<float>(_windowsize.width)/ _ticksX ;
        for(int i = 1; i < _ticksX ; i++)
            cv::line(_colorplot, cv::Point2f(i*_tickstepX,0), cv::Point2f(i*_tickstepX,static_cast<float>(_colorplot.rows)), cv::Scalar(100,100,100), 1);

        int _ticksY = 8;
        float _tickstepY = static_cast<float>(_windowsize.height)/ _ticksY ;
        for(int i = 1; i < _ticksY ; i++) {
            cv::line(_colorplot, cv::Point2f(0,i*_tickstepY), cv::Point2f(static_cast<float>(_colorplot.cols),i*_tickstepY), cv::Scalar(100,100,100), 1);
            cv::putText(_colorplot, num2str(_ymax - i * (_ymax-_ymin)/_ticksY), cv::Point2f(5, i*_tickstepY - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(150,150,150), 1, cv::LINE_AA);
        }

        float invstepY = static_cast<float>(_ymax - _ymin) / _windowsize.height;
        float stepX = static_cast<float>(_windowsize.width) / (_datalength - 1);

        for(int i = 0; i < _datalength - 1; i++) {
            cv::line(_colorplot, cv::Point2f(i*stepX, _windowsize.height - static_cast<float>(_data[i] - _ymin)/invstepY),
                                 cv::Point2f((i+1)*stepX, _windowsize.height - static_cast<float>(_data[i+1] - _ymin)/invstepY),
                                 _color, 1, cv::LINE_AA);
        }
        cv::imshow(_title, _colorplot);
    }
}
