#include <cstdio>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "vpg.h"

template<typename T>
std::string num2str(T num, unsigned char precision=0)
{
    if(precision > 0) {
        int _wholepart = static_cast<int>(num);
        int _remainder = static_cast<int>( (num - static_cast<T>(_wholepart)) * std::pow(10.0, precision) );
        return num2str(_wholepart) + std::string(".") + num2str(_remainder);
    } else {
        std::stringstream stream;
        stream << num;
        return stream.str();
    }
}

int main(int argc, char *argv[])
{
    cv::VideoCapture capture;
    cv::Mat frame;

    vpg::FaceProcessor faceproc(std::string(OPENCV_DATA_DIR) +
                                std::string("/haarcascades/haarcascade_frontalface_alt2.xml"));
    double framePeriod = 33.0; // milliseconds
    vpg::PulseProcessor pulseproc(framePeriod);

    if(capture.open(0)) {
        unsigned int k = 0;
        double s = 0.0, t = 0.0;
        int length = pulseproc.getLength();
        const double *vS = pulseproc.getSignal();
        cv::Point p1(0,0), p2(0,0);
        cv::Rect faceRect;
        unsigned int frequency;
        while(true) {
            if(capture.read(frame)) {
                faceproc.enrollImage(frame, s, t);
                pulseproc.update(s,t);
                faceRect = faceproc.getFaceRect();
                if(faceRect.area() > 0) {                    
                    for(int i = 0; i < length - 1; i++) {
                        int stepX = frame.cols/length;
                        int shiftX = (frame.cols - stepX * length) / 2;
                        int stepY = 15;
                        int shiftY = frame.rows - stepY*5;
                        p1 = cv::Point(shiftX + stepX * i, shiftY + stepY * vS[i]);
                        p2 = cv::Point(shiftX + stepX * (i + 1), shiftY + stepY * vS[i + 1]);
                        cv::line(frame, p1, p2, cv::Scalar(0,255,0), 1, CV_AA);
                    }
                    cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(255,255,255), 1, CV_AA);
                    cv::putText(frame, "HR [bpm]:", cv::Point(11, 31), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                    cv::putText(frame, "HR [bpm]:", cv::Point(10, 30), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);
                    cv::putText(frame, num2str(frequency), cv::Point(101, 35), CV_FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0,0,0), 1, CV_AA);
                    cv::putText(frame, num2str(frequency), cv::Point(100, 34), CV_FONT_HERSHEY_SIMPLEX, 1.2, ( frequency > 65 && frequency < 85) ? cv::Scalar(0,230,0) : cv::Scalar(0,0,230), 1, CV_AA);
                    cv::putText(frame, num2str(t,1) + std::string(" ms") + std::string(", press escape to exit"), cv::Point(11, frame.rows - 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                    cv::putText(frame, num2str(t,1) + std::string(" ms") + std::string(", press escape to exit"), cv::Point(10, frame.rows - 11), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);
                }
                cv::imshow("vpglib test", frame);
            }
            if(k % (length/7) == 0)
                frequency = (unsigned int)pulseproc.computeFrequency();
            k++;
            int c = cv::waitKey(1); // process frame as often as it is possible
            if( (char)c == 27 ) // escape key code
                break;
        }
        capture.release();
        return 0;
    } else
        return -1;

}
