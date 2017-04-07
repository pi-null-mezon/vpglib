# VPGLIB

An Opencv extension library that uses digital image processing for the blood pulse extraction from the video of the human face. HR measurement ranges from 55 bpm to 175 bpm and absolute error in most cases should be less than 5 bpm. Pay attention that ordinary PC is not certified as measurement tool, so measurement error could be high. Use library only at your own risk, no warranties are granted.


Sources
====

You can find it [here](https://github.com/pi-null-mezon/QPULSECAPTURE/wiki) 


Dependencies:
====

* [Opencv](https://github.com/opencv/opencv)



How to use:
====

```c++
#include "vpg.h"
...
cv::VideoCapture capture;

if(capture.open(0)) { // open default video capture device)
	
	vpg::FaceProcessor faceproc(CASCADE_FILENAME); // CASCADE_FILENAME is a path to haarcascade or lbpcascade for the face detection
	double framePeriod = faceproc.measureFramePeriod(&capture); // measure discretization period of the video in milliseconds
	vpg::PulseProcessor pulseproc(framePeriod); // object that performs harmonic analysis of vpg-signal

	cv::Mat frame;
	unsigned int k = 0; // it is counter of enrolled frames
	double s = 0.0, t = 0.0; // this is variables for temporal results storing ('s' for vpg-signal count, 't' for actual frame time)
	while(true) {
		if(capture.read(frame)) {
			faceproc.enrollImage(frame, s, t); // perform face detection, then skin detection, then average skin pixels
			pulseproc.update(s,t); // update vpg-signal
			cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(255,255,255)); // draw target rect
			cv::imshow("Video probe", frame);
	}
	if(k % 64 == 0)
		std::printf("\nHR measurement: %d bpm\n\n", pulseproc.computeFrequency()); // compute and print heart rate estimation 
	k++;
	int c = cv::waitKey(1.0); // enroll user input
	if( (char)c == 'x' )
		break;
	}
}
...
```	
You can play with this [example](https://github.com/pi-null-mezon/vpglib/tree/master/test_Device)

Designed by Alex A. Taranov, 2015

