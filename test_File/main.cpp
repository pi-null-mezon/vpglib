#include <cstdio>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "vpg.h"

int main(int argc, char *argv[])
{
    std::string fileName;
    std::string outputFileName;

    while( (--argc > 0) && ((*++argv)[0] == '-') ) {
        char option = *++argv[0];
        switch (option) {
            case 'i':
                fileName = ++(*argv);
                break;
            case 'o':
                outputFileName = ++(*argv);
                break;
            case 'h':
                std::printf("test_File\n"
                            "Options:\n"
                            " -i[filename] - input filename\n"
                            " -o[filename] - output filename\n"
                            " -h - this help ;)\n");
                return 0;
        }
    }

    cv::VideoCapture capture;
    if(!capture.open(fileName)) {
        std::printf("Can not open input file %s\n", fileName.data());
        return -1;
    }

    std::ofstream ofstream;
    if(outputFileName.size() > 0) {
        ofstream.open(outputFileName);
        if(!ofstream.is_open())
            std::printf("Can not open output file %s to write\n", outputFileName.data());
        else
            ofstream << "evpg lib test\n"
                     << "Heart rate log for " << fileName.data()
                     << "\n\nFrame;HR[bpm]\n";
    }

    vpg::FaceProcessor faceproc(std::string(OPENCV_DATA_DIR) +
                                std::string("/haarcascades/haarcascade_frontalface_alt.xml"));

    unsigned long totalFrames = (unsigned long)capture.get(CV_CAP_PROP_FRAME_COUNT);
    double framePeriod = 1000.0 / capture.get(CV_CAP_PROP_FPS); // milliseconds
    vpg::PulseProcessor pulseproc(7000.0, framePeriod, vpg::PulseProcessor::HeartRate);

    uint k = 1;
    double s = 0.0, t = 0.0;
    int heartRate = 0;
    cv::Mat frame;
    while(true) {
        if(capture.read(frame)) {
            //frame.convertTo(frame, -1, 1.0, 35.0); // brightness adjust for skin detector
            faceproc.enrollImage(frame, s, t);
            pulseproc.update(s,framePeriod);
            cv::rectangle(frame,faceproc.getFaceRect(),cv::Scalar(255,255,255));
            cv::imshow("evpg lib test", frame);
        } else {
            std::printf("End of file");
            ofstream << "End of file";
            break;
        }
        std::printf("Frames last %d, signal value: %.1f, proc.time: %.1f\n", totalFrames - k, s, t);
        if(k % (uint)(1000.0/framePeriod) == 0) {
            heartRate = pulseproc.computeFrequency();
            if(heartRate > 0)
                std::printf("\nHR measurement: %d bpm\n\n", heartRate);
            else
                std::printf("\nHR measurement: collect data\n\n", heartRate);
        }
        if(ofstream.is_open()) {
            ofstream << k << ";";
            if(heartRate > 0)
                ofstream << heartRate << "\n";
            else
                ofstream << "Collect data\n";
        }
        k++;
        int c = cv::waitKey(1);
        if( (char)c == 'x' )
            break;
    }
    ofstream.close();
    capture.release();
    return 0;
}
