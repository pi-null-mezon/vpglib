#include <iostream>
#include <vpg.h>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>

#include "facetracker.h"

#define FACE_CASCADE_FILENAME "C:/Programming/3rdPArties/opencv330/build/etc/haarcascades/haarcascade_frontalface_alt2.xml"
#define EYE_CASCADE_FILENAME  "C:/Programming/3rdPArties/opencv330/build/etc/haarcascades/haarcascade_eye.xml"
#define DLIB_FACE_SHAPE_FILENAME "C:/Programming/3rdParties/Dlib/build/etc/data/shape_predictor_68_face_landmarks.dat"

template <typename T>
std::string num2str(T value, unsigned char precision=1);

void render_face_shape (cv::Mat &img, const dlib::full_object_detection& d);

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const double *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color);

const cv::String keys = "{help h      |     | print help}"
                        "{device d    |  0  | video capture device enumerator}"
                        "{facesize    | 256 | horizontal size of the face region}";

int main(int argc, char *argv[])
{
    cv::CommandLineParser cmdargsparser(argc, argv, keys);
    cmdargsparser.about(cv::String(APP_NAME) + " v." + cv::String(APP_VERSION));
    if(cmdargsparser.has("help"))   {
       cmdargsparser.printMessage();
       std::cout << "Designed by " << APP_DESIGNER << std::endl;
       return 0;
    }

    // Facetracker
    std::cout << "Load resources from HDD. Please wait..." << std::endl;
    cv::String facecascadefilename;
#ifdef FACE_CASCADE_FILENAME
    facecascadefilename = cv::String(FACE_CASCADE_FILENAME);
#else
    facecascadefilename = "haarcascade_frontalface_alt2.xml";
#endif
    cv::CascadeClassifier facedet(facecascadefilename);
    if(facedet.empty()) {
        std::cout << "Could not load face detector resources! Abort...\n";
        return -1;
    }
    cv::String eyecascadefilename;
#ifdef EYE_CASCADE_FILENAME
    eyecascadefilename = cv::String(EYE_CASCADE_FILENAME);
#else
    eyecascadefilename = "haarcascade_eye.xml";
#endif
    cv::CascadeClassifier eyedet(eyecascadefilename);
    if(eyedet.empty()) {
        std::cout << "Could not load eye detector resources! Abort...\n";
        return -1;
    }
    FaceTracker facetracker(64, FaceTracker::NoAlign);
    facetracker.setFaceClassifier(&facedet);
    facetracker.setEyeClassifier(&eyedet);

    // Video capture
    cv::VideoCapture videocapture;
    if(videocapture.open(cmdargsparser.get<int>("device")) == false) {       
        std::cerr << "Can not open video device # " << cmdargsparser.get<int>("device") << "! Abort...";
        return -1;
    } else {
        videocapture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        videocapture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        videocapture.set(CV_CAP_PROP_FPS, 30.0);
    }

    // Dlib's stuff
    dlib::frontal_face_detector dlibfacedet = dlib::get_frontal_face_detector();
    dlib::shape_predictor       dlibshapepredictor;
    try {
#ifdef DLIB_FACE_SHAPE_FILENAME
    dlib::deserialize(DLIB_FACE_SHAPE_FILENAME) >> dlibshapepredictor;
#else
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> dlibshapepredictor;
#endif
    }
    catch(...) {
        std::cerr << "Can not load dlib's resources!";
    }
    facetracker.setFaceShapeDetector(&dlibshapepredictor);

    // VPGLIB's stuff
    #ifdef FACE_CASCADE_FILENAME
        facecascadefilename = cv::String(FACE_CASCADE_FILENAME);
    #else
        facecascadefilename = "haarcascade_frontalface_alt2.xml";
    #endif
    vpg::FaceProcessor faceproc(facecascadefilename);
    if(faceproc.empty()) {
        std::cout << "Could not load face detector resources! Abort...\n";
        return -1;
    }
    std::cout << "Measure frame period. Please wait..." << std::endl;
    double framePeriod = faceproc.measureFramePeriod(&videocapture); // ms
    std::cout << framePeriod << " ms" << std::endl;
    vpg::PulseProcessor pulseproc(framePeriod); // note it is convinirnt to use default constructor only if frame period is near to 33 ms
    // Add peak detector for the cardio intervals evaluation and analysis
    int totalcardiointervals = 25;
    vpg::PeakDetector peakdetector(pulseproc.getLength(), totalcardiointervals, 11, framePeriod);
    pulseproc.setPeakDetector(&peakdetector);
    // Add HRVProcessor for HRV analysis
    vpg::HRVProcessor hrvproc;
    // Create local variables to store frame and processing values
    unsigned int k = 0;
    double s = 0.0, t = 0.0, _snr = 0.0;
    int _hr = pulseproc.getFrequency();
    const double *signal = pulseproc.getSignal();
    int length = pulseproc.getLength();
    const double *cardiointervals = peakdetector.getIntervalsVector();
    int cardiointervalslength = peakdetector.getIntervalsLength();
    const double *binarysignal = peakdetector.getBinarySignal();
    std::cout << "Press escape to quit from the app" << std::endl;

    cv::Mat frame, faceregion;
    cv::Size targetfacesize(cmdargsparser.get<int>("facesize"), cmdargsparser.get<int>("facesize") * 1.33);
    double _frametime = 0.0, _timemark = cv::getTickCount();

    while(videocapture.read(frame)) {

        faceregion = facetracker.getResizedFaceImage(frame,targetfacesize);
        if(!faceregion.empty()) {
           cv::imshow("Probe",faceregion);
           faceproc.enrollImagePart(faceregion,s,t);
           pulseproc.update(s,t);
           drawDataWindow("VPG", cv::Size(640,360), signal, length,3.0,-3.0,cv::Scalar(0,255,0));

           cv::RotatedRect _faceRrect = facetracker.getFaceRotatedRect();
           cv::Point2f _vert[4];
           _faceRrect.points(_vert);
           for(unsigned char i = 0; i < 4; ++i) {
               cv::line(frame,_vert[i], _vert[(i+1)%4],cv::Scalar(0,0,255),1,CV_AA);
           }
           if(facetracker.getFaceAlignMethod() == FaceTracker::FaceShape) {
               dlib::full_object_detection _faceshape = facetracker.getFaceShape();
               for(size_t i = 0; i < _faceshape.num_parts(); ++i) {
                   dlib::point _p = _faceshape.part(i), _tp;
                   float _anglerad = - CV_PI * _faceRrect.angle / 180.0;
                   _tp.x() = _p.x()*std::cos(_anglerad) + _p.y()*std::sin(_anglerad);
                   _tp.y() = - _p.x()*std::sin(_anglerad) + _p.y()*std::cos(_anglerad);
                   _tp += dlib::point(_faceRrect.center.x,_faceRrect.center.y);
                   _faceshape.part(i) = _tp;
               }
               render_face_shape(frame,_faceshape);
            }
        }


        _frametime = (cv::getTickCount() - _timemark) * 1000.0 / cv::getTickFrequency();
        _timemark = cv::getTickCount();
        cv::String _periodstr = num2str(frame.cols,0) + "x" + num2str(frame.rows,0) + " " + num2str(_frametime) + " ms, press 's' to open capture device settings";
        cv::putText(frame, _periodstr, cv::Point(20,frame.rows-10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
        cv::putText(frame, _periodstr, cv::Point(19,frame.rows-11), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);
        cv::imshow(APP_NAME, frame);
        int c = cv::waitKey(1);
        if( (char)c == 27 ) { // 27 is escape ASCII code
            break;
        } else switch(c) {
            case 's':
                videocapture.set(CV_CAP_PROP_SETTINGS,0.0);
                break;
        }
    }

    videocapture.release();
    return 0;
}

template <typename T>
std::string num2str(T value, unsigned char precision)
{
    std::string _fullstring = std::to_string(value);
    size_t _n = 0;
    for(size_t i = 0; i < _fullstring.size(); ++i) {        
        if(_fullstring[i] == '.')
            break;
                _n++;
    }
    if(precision > 0) {
        _n += precision + 1;
    }
    return std::string(_fullstring.begin(), _fullstring.begin() + _n);
}

// Grabbed from http://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/
void draw_polyline(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed = false)
{
    std::vector <cv::Point> points;
    for (int i = start; i <= end; ++i) {
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(0,255,255), 1, CV_AA);

}

void render_face_shape (cv::Mat &img, const dlib::full_object_detection& d)
{
    DLIB_CASSERT (
         d.num_parts() == 68,
         "\n\t Invalid inputs were given to this function. "
         << "\n\t d.num_parts():  " << d.num_parts()
     );

    draw_polyline(img, d, 0, 16);           // Jaw line
    draw_polyline(img, d, 17, 21);          // Left eyebrow
    draw_polyline(img, d, 22, 26);          // Right eyebrow
    draw_polyline(img, d, 27, 30);          // Nose bridge
    draw_polyline(img, d, 30, 35, true);    // Lower nose
    draw_polyline(img, d, 36, 41, true);    // Left eye
    draw_polyline(img, d, 42, 47, true);    // Right Eye
    draw_polyline(img, d, 48, 59, true);    // Outer lip
    draw_polyline(img, d, 60, 67, true);    // Inner lip

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
            cv::putText(_colorplot, num2str(_ymax - i * (_ymax-_ymin)/_ticksY), cv::Point(5, i*_tickstepY - 10), CV_FONT_HERSHEY_SIMPLEX, 0.33, cv::Scalar(150,150,150), 1, CV_AA);
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
