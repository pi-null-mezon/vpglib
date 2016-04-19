# VPGLIB

Based on Opencv library and was designed for one special purpose - to measure heart rate from face video.
Pay attention that ordinary PC is not certified as measurement tool, so measurement
error could be very high. Use it at your own risk, no warranties are granted.
Measurement ranges from 55 bpm to 175 bpm, absolute error less than 5 bpm (compare to finger pulse oximeter).
Warning!!! Works fine with ordinary web-cams only if face of subject is well illuminated.

To work with vpg library you should provide the following dependencies: opencv

How to use:

    #include "vpg.h"
    ...
	cv::VideoCapture capture;
    cv::Mat frame;

    vpg::FaceProcessor faceproc(CASCADE_FILENAME);
    double framePeriod = 35.0; // ms
    vpg::PulseProcessor pulseproc(framePeriod);

    capture.open(0);
    unsigned int k = 0;
    double s = 0.0, t = 0.0;
    while(true) {
        if(capture.read(frame)) {
                faceproc.enrollImage(frame, s, t);
                pulseproc.update(s,t);
                cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(255,255,255));
                cv::imshow("evpg test", frame);
        }
        std::printf("Frame %d, value: %.1f, time%.1f\n", k, s, t);
        if(k % 64 == 0)
            std::printf("\nHR measurement: %d bpm\n\n", pulseproc.computeHR());
        k++;
        int c = cv::waitKey(framePeriod - 15.0); // cause image processing takes part of time
        if( (char)c == 'x' )
            break;
    }
	...
	
For more complex example see test_Device and test_File codes

Designed by Alex A. Taranov, 2015

