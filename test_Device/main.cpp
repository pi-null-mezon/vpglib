#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <opencv2/highgui.hpp>
#include "vpg.h"

template<typename T>
std::string num2str(T num, unsigned char precision=0)
{
    if(precision > 0)
        return std::to_string(static_cast<long>(num)) + "." + std::to_string(static_cast<long>(std::abs(num - static_cast<long>(num))*std::pow(10.0, precision)));
    else
        return std::to_string(static_cast<long>(num));
}

template <typename T>
T str2num(const std::string &str)
{
     std::istringstream ss(str);
     T result;
     return (ss >> result ? result : 0);
}

int main(int argc, char *argv[])
{
    float measInt_ms = 1000.0f;
    int deviceID = 0;
    char *outputfilename = 0;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        char option = *++argv[0];
        switch(option) {
            case 'v':
                deviceID = str2num<int>(++argv[0]);
                break;
            case 't':
                measInt_ms = str2num<float>(++argv[0]);
                break;
            case 'o':
                outputfilename = ++argv[0];
                break;
            case 'h':
                std::cout << APP_NAME << " v" << APP_VERSION << " help" << std::endl
                          << " -v - video device enumerator (default " << deviceID << ")" << std::endl
                          << " -t - measurement interval (default " << measInt_ms << " ms)" << std::endl
                          << " -o - output file name" << std::endl
                          << " -h - this help" << std::endl
                          << APP_DESIGNER << std::endl;
                return 0;
        }
    }

    std::ofstream ofs;
    if(outputfilename != 0) {
        ofs.open(outputfilename);
        if(!ofs.is_open()) {
            std::cout << "Could not open file " << outputfilename
                      << " for writing. Abort...";
            return -1;
        } else {
            ofs << "File created by " <<  APP_NAME << " v" << APP_VERSION << std::endl;
        }
    }

    cv::VideoCapture capture;
    cv::Mat frame;

    #ifdef DESIGNBUILD
    vpg::FaceProcessor faceproc(std::string(OPENCV_DATA_DIR) +
                                std::string("/haarcascades/haarcascade_frontalface_alt2.xml"));
    #else
    vpg::FaceProcessor faceproc(std::string("haarcascades/haarcascade_frontalface_alt2.xml"));
    #endif

    double framePeriod = 33.0; // milliseconds
    vpg::PulseProcessor pulseproc(framePeriod);

    if(capture.open(deviceID)) {
        double s = 0.0, t = 0.0, timeout = measInt_ms;
        int length = pulseproc.getLength();
        const double *vS = pulseproc.getSignal();
        cv::Point p1(0,0), p2(0,0);
        cv::Rect faceRect;
        unsigned int frequency = 80;
        double snr = 0.0;

        if(ofs.is_open()) {
            std::time_t t = std::time(0);
            struct std::tm * now = localtime( &t );
            ofs  << "Record was started at "
                 << std::setw(2) << std::setfill('0') << now->tm_mday << '.'
                 << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '.'
                 << (now->tm_year + 1900) << ' '
                 << std::setw(2) << std::setfill('0') << (now->tm_hour) << ":"
                 << std::setw(2) << std::setfill('0') << (now->tm_min) << ":"
                 << std::setw(2) << std::setfill('0') << now->tm_sec << std::endl
                 << "Measurement interval " << measInt_ms << "[ms]" << std::endl
                 << "Pulse[bpm];\tSNR[dB]" << std::endl;
        }

        while(true) {
            if(capture.read(frame)) {

                faceproc.enrollImage(frame, s, t);
                pulseproc.update(s,t);
                faceRect = faceproc.getFaceRect();

                if(faceRect.area() > 0) {

                    float shiftX = 10.0f;
                    float stepX = static_cast<float>(frame.cols - 2*shiftX) / length;
                    float stepY = 0.025f * frame.rows;
                    float shiftY = 0.8f * frame.rows;

                    for(int i = 0; i < length - 1; i++) {                      
                        p1 = cv::Point2f(shiftX + stepX * i, shiftY + stepY * vS[i]);
                        p2 = cv::Point2f(shiftX + stepX * (i + 1), shiftY + stepY * vS[i + 1]);
                        cv::line(frame, p1, p2, cv::Scalar(0,255,0), 1, CV_AA);
                    }

                    cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(0,0,0), 1, CV_AA);
                    cv::rectangle(frame,faceproc.getFaceRect()-cv::Point(1,1),cv::Scalar(255,255,255), 1, CV_AA);

                    cv::putText(frame, "HR [bpm]:", cv::Point(11, 31), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                    cv::putText(frame, "HR [bpm]:", cv::Point(10, 30), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);        

                    std::string _freqstr = num2str(frequency);
                    cv::putText(frame, _freqstr, cv::Point(101, 35), CV_FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0,0,0), 1, CV_AA);
                    cv::putText(frame, _freqstr, cv::Point(100, 34), CV_FONT_HERSHEY_SIMPLEX, 1.2, ( frequency > 65 && frequency < 85) ? cv::Scalar(0,230,0) : cv::Scalar(0,0,230), 1, CV_AA);

                    std::string _snrstr = "snr: " + num2str(snr,2) + " dB";
                    cv::putText(frame, _snrstr, cv::Point(11, 61), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                    cv::putText(frame, _snrstr, cv::Point(10, 60), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);
                }
                cv::putText(frame, num2str(t,1) + " ms, press escape to exit", cv::Point(11, frame.rows - 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, num2str(t,1) + " ms, press escape to exit", cv::Point(10, frame.rows - 11), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);

                cv::imshow("vpglib test", frame);
            }

            timeout -= t;

            if(timeout < 0.0) {
                frequency = static_cast<int>(pulseproc.computeFrequency());
                snr = pulseproc.getSNR();

                if(ofs.is_open())
                    ofs << frequency << ";\t" << std::setprecision(2) << snr << std::endl;

                timeout = measInt_ms;
            }

            int c = cv::waitKey(1); // process frame as often as it is possible
            if( (char)c == 27 ) // escape key code
                break;
        }        
        capture.release();        

        if(ofs.is_open())
            ofs.close();

        return 0;
    } else {
        std::cout << "Can not open video capture device. Abort...";
        return -1;
    }
}
