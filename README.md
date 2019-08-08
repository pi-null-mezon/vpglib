# VPGLIB

VPGLIB is the library for the blood pulse extraction from the video of the human face. HR measurement ranges from 55 bpm to 175 bpm and absolute error in most cases should be less than 5 bpm. Pay attention that ordinary PC is not certified as measurement tool, so measurement error could be high. Use library only at your own risk, no warranties are granted.

**How to use:**
```c++
#include <iostream>

using namespace std;

#include "vpg.h"

int main()
{
    cv::VideoCapture capture;
    // open default video capture device
    if(capture.open(0)) {
        // CASCADE_FILENAME is a path to haarcascade or lbpcascade file for the face detection
        vpg::FaceProcessor faceproc(CASCADE_FILENAME);
        // measure discretization period of the video
        double framePeriod = faceproc.measureFramePeriod(&capture);
        printf("measured frame period: %.2f ms",framePeriod);
        // create object that performs harmonic analysis of vpg-signal
        vpg::PulseProcessor pulseproc(framePeriod);
        cv::Mat frame;
        unsigned int k = 0; // frame counter
        double s = 0.0, t = 0.0; // 's' for vpg-signal count, 't' for actual frame time
        while(true) {
            if(capture.read(frame)) {
                // perform frame enrollment
                faceproc.enrollImage(frame, s, t);
                // update vpg-signal if face tracked
                if(faceproc.getFaceRect().area() > 0) {
                    pulseproc.update(s,t);
                    // draw rect for tracked face
                    cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(127,255,127),1,CV_AA);
                    // periodically compute heart rate estimation
                    if(k % 64 == 0)
                        printf("\nHR measurement: %.0f bpm", pulseproc.computeFrequency());
                }
                cv::imshow("Video probe", frame);
            }
            // increment counter
            k++;
            // exit if user pressed 'escape'
            if( cv::waitKey(1) == 27 )
                break;
        }
    }
    return 0;
}
```	
**Dependencies:**
1. [OpenCV](https://github.com/opencv/opencv)

Designed by Alex A. Taranov, 2015, list of the project [sources](https://github.com/pi-null-mezon/QPULSECAPTURE/wiki)

