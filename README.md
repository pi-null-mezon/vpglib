# VPGLIB

Based on Opencv and was designed for one special purpose - to measure heart rate from video of the human face.
Pay attention that ordinary PC is not certified as measurement tool, so measurement
error could be very high. Use it at your own risk, no warranties are granted.
Measurement ranges from 55 bpm to 175 bpm, absolute error less than 5 bpm (compare to fingertip pulse oximeter).
Warning!!! Works fine with ordinary web-cams only if face of subject is well illuminated.

To work with vpg library you should provide the following dependencies: opencv

How to use:
```c++
#include "vpg.h"
...
cv::VideoCapture capture;
cv::Mat frame;

vpg::FaceProcessor faceproc(CASCADE_FILENAME); // CASCADE_FILENAME is a path to haarcascade or lbpcascade for the face detection
double framePeriod = 33.0; // it is discretization period of video in milliseconds, calculated as (1.0 / fps)
vpg::PulseProcessor pulseproc(framePeriod); // object that performs harmonic analysis of vpg-signal

capture.open(0); // open default video capture device
unsigned int k = 0; // it is counter of enrolled frames
double s = 0.0, t = 0.0; // this is variables for temporal results storing ('s' for vpg-signal count, 't' for actual frame time)
while(true) {
if(capture.read(frame)) {
        faceproc.enrollImage(frame, s, t); // perform face detection, then skin detection, then average skin pixels
        pulseproc.update(s,t); // update vpg-signal
        cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(255,255,255));
        cv::imshow("evpg test", frame);
}
std::printf("Frame %d, value: %.1f, time%.1f\n", k, s, t);
if(k % 64 == 0)
    std::printf("\nHR measurement: %d bpm\n\n", pulseproc.computeFrequency()); // compute and print heart rate estimation 
k++;
int c = cv::waitKey(framePeriod - 15.0); // time delay for event loop
if( (char)c == 'x' )
    break;
}
...
```
	
Also see test_Device and test_File

Designed by Alex A. Taranov, 2015

