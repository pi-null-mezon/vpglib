#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <opencv2/opencv.hpp>
#include "vpg.h"

template<typename T>
std::string num2str(T num, unsigned char precision=0)
{
    if(precision > 0)
        return std::to_string(static_cast<long>(num)) + "." + std::to_string(static_cast<long>(std::abs(static_cast<double>(num) - static_cast<long>(num))*std::pow(10.0, precision)));
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
    char *outputHRfilename = 0;
    char *outputVPGfilename = 0;
    char *outputVideofilename = 0;
    char *inputVideofilename = 0;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        char option = *++argv[0];
        switch(option) {
            case 'v':
                deviceID = str2num<int>(++argv[0]);
                break;
            case 'i':
                inputVideofilename = ++argv[0];
                break;
            case 't':
                measInt_ms = str2num<float>(++argv[0]);
                break;
            case 'o':
                outputHRfilename = ++argv[0];
                break;
            case 's':
                outputVPGfilename = ++argv[0];
                break;
            case 'w':
                outputVideofilename = ++argv[0];
                break;
            case 'h':
                std::cout << APP_NAME << " v" << APP_VERSION << " help" << std::endl << std::endl
                          << " -v[int] - video device enumerator (default " << deviceID << ")" << std::endl
                          << " -t[real] - measurement interval (default " << measInt_ms << " ms)" << std::endl
                          << " -i[str] - input video file name (if used video file will be processed)" << std::endl
                          << " -o[str] - output file with the HR vs time" << std::endl
                          << " -s[str] - output file with the VPG counts vs frame number" << std::endl
                          << " -w[str] - output video file name" << std::endl
                          << " -h - help :)" << std::endl << std::endl
                          << APP_DESIGNER << std::endl;
                return 0;
        }
    }
    // Open stream for heart rate log if it is neeeded
    std::ofstream ohrfs;
    if(outputHRfilename != 0) {
        ohrfs.open(outputHRfilename);
        if(!ohrfs.is_open()) {
            std::cout << "Could not open file " << outputHRfilename
                      << " for writing. Abort...";
            return -1;
        } else {
            ohrfs << "File created by " <<  APP_NAME << " v" << APP_VERSION << std::endl;
        }
    }
    // Open stream for vpg signal log if it is neeeded
    std::ofstream ovpgfs;
    if(outputVPGfilename != 0) {
        ovpgfs.open(outputVPGfilename);
        if(!ovpgfs.is_open()) {
            std::cout << "Could not open file " << outputVPGfilename
                      << " for writing. Abort...";
            return -1;
        } else {
            ovpgfs << "File created by " <<  APP_NAME << " v" << APP_VERSION << std::endl;
        }
    }

    cv::VideoCapture capture;
    if(inputVideofilename) {
        if(capture.open(inputVideofilename) == false) {
            std::cout << "Could not open video file " << inputVideofilename << " Abort..." << std::endl;
            return -1;
        }
    } else {
        if(capture.open(deviceID) == false) {
            std::cout << "Could not open video device with id " << deviceID << " Abort..." << std::endl;
            return -1;
        }
    }

    #ifdef DESIGNBUILD
    vpg::FaceProcessor faceproc(std::string(OPENCV_DATA_DIR) + std::string("/haarcascades/haarcascade_frontalface_alt2.xml"));
    #else
    vpg::FaceProcessor faceproc(std::string("haarcascades/haarcascade_frontalface_alt2.xml"));
    #endif

    std::cout << "Measure frame period... " << std::endl;
    double framePeriod = faceproc.measureFramePeriod(&capture); // milliseconds
    std::cout << framePeriod << " ms" << std::endl;
    vpg::PulseProcessor pulseproc(framePeriod);

    cv::VideoWriter videowriter;
    if(outputVideofilename)
        if(videowriter.open(outputVideofilename, CV_FOURCC('M','P','4','2'), 1000.0/framePeriod, cv::Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT))) == false)
            std::cout << "Warning! Output videofile can not be opened!" << std::endl;

    cv::Mat frame;
    double s = 0.0, t = 0.0, timeout = measInt_ms;
    int length = pulseproc.getLength();
    const double *vS = pulseproc.getSignal();
    cv::Point p1(0,0), p2(0,0);
    cv::Rect faceRect;
    unsigned int frequency = static_cast<unsigned int>(pulseproc.getFrequency());
    double snr = 0.0;
    unsigned long framecounter = 0;

    std::time_t _timet = std::time(0);
    struct std::tm * now = localtime( &_timet );
    if(ohrfs.is_open()) {
       ohrfs << "Record was started at "
             << std::setw(2) << std::setfill('0') << now->tm_mday << '.'
             << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '.'
             << (now->tm_year + 1900) << ' '
             << std::setw(2) << std::setfill('0') << (now->tm_hour) << ":"
             << std::setw(2) << std::setfill('0') << (now->tm_min) << ":"
             << std::setw(2) << std::setfill('0') << now->tm_sec << std::endl
             << "Measurement interval " << measInt_ms << "[ms]" << std::endl
             << "HR[bpm];\tSNR[dB]" << std::endl;
    }
    if(ovpgfs.is_open()) {
       ovpgfs << "Record was started at "
              << std::setw(2) << std::setfill('0') << now->tm_mday << '.'
              << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '.'
              << (now->tm_year + 1900) << ' '
              << std::setw(2) << std::setfill('0') << (now->tm_hour) << ":"
              << std::setw(2) << std::setfill('0') << (now->tm_min) << ":"
              << std::setw(2) << std::setfill('0') << now->tm_sec << std::endl
              << "HR measurement interval " << measInt_ms << "[ms]" << std::endl
              << "Frame;\tVPG[c.n.];\tHR[bpm];\tSNR[db]" << std::endl;
    }

    faceproc.dropTimer();
    while(true) {
        if(capture.read(frame)) {

            if(videowriter.isOpened())
                videowriter.write(frame);

            faceproc.enrollImage(frame, s, t);
            if(inputVideofilename) {
                pulseproc.update(s,framePeriod); // if videofile is used as source then we should use knowing frame period
            } else {
                pulseproc.update(s,t);
            }
            faceRect = faceproc.getFaceRect();

            if(faceRect.area() > 0) {

                float shiftX = frame.cols * 0.1f;
                float stepX = static_cast<float>(frame.cols - 2*shiftX) / length;
                float stepY = 0.025f * frame.rows;
                float shiftY = 0.9f * frame.rows;

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
            cv::putText(frame, num2str(t,1) + " ms, press ESC to exit or 's' to get DirectShow settings", cv::Point(11, frame.rows - 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
            cv::putText(frame, num2str(t,1) + " ms, press ESC to exit or 's' to get DirectShow settings", cv::Point(10, frame.rows - 11), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);

            cv::imshow("vpglib test", frame);
        } else {
            break; // stop processing when frame can not be read
        }

        if(inputVideofilename) {
            timeout -= framePeriod; // if videofile is used as source then we should use knowing frame period
        } else {
            timeout -= t;
        }

        if(timeout < 0.0) {

            frequency = static_cast<unsigned int>(pulseproc.computeFrequency());
            snr = pulseproc.getSNR();

            if(ohrfs.is_open())
                ohrfs << frequency << ";\t" << std::setprecision(2) << snr << std::endl;

            timeout = measInt_ms;
        }

        if(ovpgfs.is_open()) {
            ovpgfs << framecounter
                   << std::setprecision(3) << ";\t" << pulseproc.getSignalSampleValue()
                   << ";\t" << frequency
                   << std::setprecision(2) << ";\t" << snr
                   << std::endl;
        }

        int c = cv::waitKey(1); // process frame as often as it is possible
        if( (char)c == 27 ) // escape key code
            break;
        else switch(c) {
            case 's':
                capture.set(CV_CAP_PROP_SETTINGS,0.0);
                break;
        }

        framecounter++;
    }

    capture.release();

    if(videowriter.isOpened())
        videowriter.release();

    if(ohrfs.is_open())
        ohrfs.close();

    if(ovpgfs.is_open())
        ovpgfs.close();

    return 0;
}
