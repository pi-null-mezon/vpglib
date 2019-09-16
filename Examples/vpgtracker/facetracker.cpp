#include "facetracker.h"
#include <opencv2/imgproc.hpp>
//---------------------------------------------------------------------------------
#define PI_VALUE CV_PI
//---------------------------------------------------------------------------------
FaceTracker::FaceTracker(uchar length, AlignMethod method) :
    m_historyLength(length),
    m_emptyFrames(0),
    m_framesFaceFound(0),
    m_pos(0),
    m_angle(0.0),    
    m_method(method),    
    m_minNeighbours(5),
    m_xPortion(1.0f),
    m_yPortion(1.0f),
    m_xShift(0.0f),
    m_yShift(0.0f),
    m_primaryfacedetectortupe(FaceTracker::ViolaJones)
{
    v_rectHistory = new cv::Rect[m_historyLength];
    setMinFaceSize(cv::Size(110,110));
    clearMetaData();
    resetHistory();
    v_metaID.resize(2);
    dlibfacedet = dlib::get_frontal_face_detector();
}
//---------------------------------------------------------------------------------
FaceTracker::~FaceTracker()
{
    delete[] v_rectHistory;
}
//---------------------------------------------------------------------------------
bool FaceTracker::setFaceClassifier(cv::CascadeClassifier *pointer)
{
    pt_faceClassifier = pointer;
    return !pt_faceClassifier->empty();
}
//---------------------------------------------------------------------------------
bool FaceTracker::setEyeClassifier(cv::CascadeClassifier *pointer)
{
    pt_eyeClassifier = pointer;
    return !pt_eyeClassifier->empty();
}

void FaceTracker::setFaceShapeDetector(dlib::shape_predictor *_dlibfaceshapepredictor)
{
    pt_dlibfaceshapepredictor = _dlibfaceshapepredictor;
}
//---------------------------------------------------------------------------------
cv::RotatedRect FaceTracker::searchFace(const cv::Mat &img)
{
    if(m_historyLength == 1)
        beginFlag = true;

    cv::Mat image;
    float xScale = 1.0, yScale = 1.0;
    if(img.cols > 640 && img.rows > 480) {
        float relation = (float)img.cols / (float)img.rows;
        if(relation > (14.0 / 9.0)) {
            xScale = (float)img.cols / 640.0f;
            yScale = (float)img.rows / 360.0f;
            cv::resize(img, image, cv::Size(640, 360), 0.0, 0.0, CV_INTER_AREA);
        } else {
            xScale = (float)img.cols / 640.0f;
            yScale = (float)img.rows / 480.0f;
            cv::resize(img, image, cv::Size(640, 480), 0.0, 0.0, CV_INTER_AREA);
        }
    } else {
        image = img.clone();
    }
    if(std::abs(m_angle) > 1.0) {
        cv::Mat transform = cv::getRotationMatrix2D(m_centerPoint, m_angle, 1.0);
        cv::warpAffine(image,image, transform, cv::Size(image.cols, image.rows));
    }

    std::vector<cv::Rect> rects;
    switch(m_primaryfacedetectortupe) {
        case FaceTracker::ViolaJones:
            // Performance note: it is strange, but with scale=1.09 this detection still works fast
            // (less than 30 ms per frame) on my work Toshiba laptop (with AMD discrete GPU),
            // but on my own Samsung notebook (more powerfull CPU and discrete NVidia GPU) it works
            // more than 30 ms per frame.
            pt_faceClassifier->detectMultiScale(image.getUMat(cv::ACCESS_FAST), rects, 1.3, m_minNeighbours, CV_HAAR_FIND_BIGGEST_OBJECT, m_minFaceSize, m_maxFaceSize );
            break;
        case FaceTracker::HOG:
            cv::Mat _tmpgraymat;
            if(image.channels() == 3) {
                cv::cvtColor(image, _tmpgraymat, CV_BGR2GRAY);
            } else {
                _tmpgraymat = image;
            }
            std::vector<dlib::rectangle> _vdlibfacerects = dlibfacedet(dlib::cv_image<unsigned char>(_tmpgraymat));
            for(size_t i = 0; i < _vdlibfacerects.size(); ++i) {
                rects.push_back(cv::Rect(_vdlibfacerects[i].left(),_vdlibfacerects[i].top(),_vdlibfacerects[i].width(),_vdlibfacerects[i].height()));
            }
            break;
    }

    if(rects.size() > 0) {
        if(beginFlag == false) {
            cv::Rect intersection = rects[0] & __getAverageFaceRect();
            if(intersection.area() > rects[0].area()/(m_historyLength) ) {
                __updateHistory(rects[0]);
                m_emptyFrames = 0;
            } else {
                m_emptyFrames++;
            }
        } else { // i.e. if beginFlag == true
            __updateHistory(rects[0]);
            m_emptyFrames = 0;
        }
    } else {
        m_emptyFrames++;
    }
    if(m_emptyFrames == m_historyLength) {
        clearMetaData();       
        resetHistory();
        m_centerPoint = cv::Point2f(image.cols / 2.0f, image.rows / 2.0f);
    } else if(m_emptyFrames > m_historyLength) {
        m_angle = 45.0 * std::sin((m_emptyFrames - m_historyLength) * 0.1 * PI_VALUE);
        m_rRect = cv::RotatedRect(cv::Point2f(m_centerPoint.x * xScale, m_centerPoint.y * yScale),cv::Size2f(0.0,0.0),0.0);
        return m_rRect;
    }

    cv::Rect faceRect = __getAverageFaceRect() & cv::Rect(0,0,image.cols,image.rows);

    if(faceRect.area() > 0) {
        cv::Mat faceImage(image, faceRect);
        m_centerPoint = cv::Point2f(faceRect.x + faceRect.width/2.0f, faceRect.y + faceRect.height/2.0f);
        switch(m_method) {

            case NoAlign: {
                m_angle = 0.0;
            }
            break;

            case FaceShape: {
                dlib::cv_image<dlib::rgb_pixel> _dlibfaceimg(faceImage);
                faceshape = (*pt_dlibfaceshapepredictor)(_dlibfaceimg, dlib::rectangle(_dlibfaceimg.nc(),_dlibfaceimg.nr()));
                DLIB_CASSERT (faceshape.num_parts() == 68 || faceshape.num_parts() == 5, "\n\t Invalid inputs were given to this function. " << "\n\t d.num_parts():  " << faceshape.num_parts());
                cv::Point2f _lefteyecenter;
                cv::Point2f _righteyecenter;
                if(faceshape.num_parts() == 68) {
                    for(size_t i = 0; i < 6; ++i) {
                        _lefteyecenter += cv::Point2f(faceshape.part(i+36).x(),faceshape.part(i+36).y());
                        _righteyecenter += cv::Point2f(faceshape.part(i+42).x(),faceshape.part(i+42).y());
                    }
                    _lefteyecenter /= 6;
                    _righteyecenter /= 6;
                } else if(faceshape.num_parts() == 5) {
                    for(size_t i = 0; i < 2; ++i) {
                        _righteyecenter += cv::Point2f(faceshape.part(i).x(),faceshape.part(i).y());
                        _lefteyecenter += cv::Point2f(faceshape.part(i+2).x(),faceshape.part(i+2).y());
                    }
                    _lefteyecenter /= 2;
                    _righteyecenter /= 2;
                }
                cv::Point2f peyes = _righteyecenter - _lefteyecenter;
                m_angle += 45.0 * std::atan(peyes.y / peyes.x) / PI_VALUE; // 180.0 produces angle jitter, and 90.0 looks more stable
                for(size_t i = 0; i < faceshape.num_parts(); ++i) {
                    faceshape.part(i) -= dlib::point(_dlibfaceimg.nc()/2.0f,_dlibfaceimg.nr()/2.0f);
                }
            }
            break;

            case Eyes: {
                std::vector<cv::Rect> v_eyes;
                cv::Point shift(faceImage.cols/20, faceImage.rows/6);
                cv::Rect leftEyeRect(0,0, faceImage.cols/2, faceImage.rows/2);
                cv::Mat leftTopPart(faceImage, leftEyeRect + shift);
                pt_eyeClassifier->detectMultiScale(leftTopPart.getUMat(cv::ACCESS_FAST), v_eyes, 1.1, 3,  CV_HAAR_FIND_BIGGEST_OBJECT, m_minEyeSize);
                if(v_eyes.size() != 0) {
                    leftEyeRect = v_eyes[0] + shift;
                    cv::Point2f lep(leftEyeRect.x + leftEyeRect.width/2.0f, leftEyeRect.y + leftEyeRect.height/2.0f);
                    shift = cv::Point(faceImage.cols*9/20, faceImage.rows/6);
                    cv::Rect rightEyeRect(0,0, faceImage.cols/2, faceImage.rows/2);
                    cv::Mat rightTopPart(faceImage, rightEyeRect + shift);
                   pt_eyeClassifier->detectMultiScale(rightTopPart.getUMat(cv::ACCESS_FAST), v_eyes, 1.1, 3,  CV_HAAR_FIND_BIGGEST_OBJECT, m_minEyeSize);
                    if(v_eyes.size() != 0) {
                        rightEyeRect = v_eyes[0] + shift;
                        cv::Point2f rep(rightEyeRect.x + rightEyeRect.width/2.0f, rightEyeRect.y + rightEyeRect.height/2.0f);
                        cv::Point2f peyes = rep - lep;
                        m_angle += 45.0 * std::atan(peyes.y / peyes.x) / PI_VALUE; // 180.0 produces angle jitter, and 90.0 looks more stable
                    }
                }
            }
            break;

            case Skin: {
                cv::Mat bw;
                threshSkin(faceImage, bw, 0, 255);
                int size = (int)(cv::sum(bw)[0]/255.0);
                if(size > 0) {
                    cv::Mat data_pts = cv::Mat(size, 2, CV_64FC1);
                    int i = 0;
                    uchar *pt;
                    for(int y = 0; y < bw.rows; y++) {
                        pt = bw.ptr(y);
                        for(int x = 0; x < bw.cols; x++)
                            if(pt[x] == 255) {
                                data_pts.at<double>(i,0) = x;
                                data_pts.at<double>(i,1) = y;
                                i++;
                            }
                    }
                    cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
                    cv::Point2d eigenvector(pca_analysis.eigenvectors.at<double>(1,0), pca_analysis.eigenvectors.at<double>(1,1));
                    m_angle += 90.0 * atan(eigenvector.y / eigenvector.x) / PI_VALUE; // 180.0 produces angle jitter, and 90.0 looks more stable
                }
            }
            break;

            case Otsu: {
                cv::Mat bw;
                cv::cvtColor(faceImage, faceImage, CV_BGR2GRAY);
                cv::threshold(faceImage, bw, 0.0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);
                int size = (int)(cv::sum(bw)[0]/255.0);
                if(size > 0) {
                    cv::Mat data_pts = cv::Mat(size, 2, CV_64FC1);
                    int i = 0;
                    uchar *pt;
                    for(int y = 0; y < faceImage.rows; y++) {
                        pt = faceImage.ptr(y);
                        for(int x = 0; x < faceImage.cols; x++)
                            if(pt[x] == 255) {
                                data_pts.at<double>(i,0) = x;
                                data_pts.at<double>(i,1) = y;
                                i++;
                            }
                    }
                    cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
                    cv::Point2d eigenvector(pca_analysis.eigenvectors.at<double>(0,0), pca_analysis.eigenvectors.at<double>(0,1));
                    m_angle += 90.0 * std::atan(eigenvector.x / eigenvector.y) / PI_VALUE; // 180.0 produces angle jitter, and 90.0 looks more stable
                }
            }
            break;

            case EyesAndSkin: {
                double angle = 0.0;
                std::vector<cv::Rect> v_eyes;
                cv::Point shift(faceImage.cols/20, faceImage.rows/6);
                cv::Rect leftEyeRect(0,0, faceImage.cols/2, faceImage.rows/2);
                cv::Mat leftTopPart(faceImage, leftEyeRect + shift);
               pt_eyeClassifier->detectMultiScale(leftTopPart.getUMat(cv::ACCESS_FAST), v_eyes, 1.1, 3,  CV_HAAR_FIND_BIGGEST_OBJECT, m_minEyeSize);
                if(v_eyes.size() != 0) {
                    leftEyeRect = v_eyes[0] + shift;
                    cv::Point2f lep(leftEyeRect.x + leftEyeRect.width/2.0f, leftEyeRect.y + leftEyeRect.height/2.0f);
                    shift = cv::Point(faceImage.cols*9/20, faceImage.rows/6);
                    cv::Rect rightEyeRect(0,0, faceImage.cols/2, faceImage.rows/2);
                    cv::Mat rightTopPart(faceImage, rightEyeRect + shift);
                   pt_eyeClassifier->detectMultiScale(rightTopPart.getUMat(cv::ACCESS_FAST), v_eyes, 1.1, 3,  CV_HAAR_FIND_BIGGEST_OBJECT, m_minEyeSize);
                    if(v_eyes.size() != 0) {
                        rightEyeRect = v_eyes[0] + shift;
                        cv::Point2f rep(rightEyeRect.x + rightEyeRect.width/2.0f, rightEyeRect.y + rightEyeRect.height/2.0f);
                        cv::Point2f peyes = rep - lep;
                        angle = 90.0 * std::atan(peyes.y / peyes.x) / PI_VALUE; // 180.0 produces angle jitter, and 90.0 looks more stable
                    }
                }
                cv::Mat bw;
                threshSkin(faceImage, bw, 0, 255);
                int size = (int)(cv::sum(bw)[0]/255.0);
                if(size > 0) {
                    cv::Mat data_pts = cv::Mat(size, 2, CV_64FC1);
                    int i = 0;
                    uchar *pt;
                    for(int y = 0; y < bw.rows; y++) {
                        pt = bw.ptr(y);
                        for(int x = 0; x < bw.cols; x++)
                            if(pt[x] == 255) {
                                data_pts.at<double>(i,0) = x;
                                data_pts.at<double>(i,1) = y;
                                i++;
                            }
                    }
                    cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
                    cv::Point2d eigenvector(pca_analysis.eigenvectors.at<double>(1,0), pca_analysis.eigenvectors.at<double>(1,1));
                    angle = (angle + (90.0 * std::atan(eigenvector.y / eigenvector.x) / PI_VALUE)) /2.0; // 180.0 produce angle jitter, and 90.0 looks more stable
                }
                m_angle += angle;
            }
            break;

            case EyesThenSkin: {
                std::vector<cv::Rect> v_eyes;
                cv::Point shift(faceImage.cols/20, faceImage.rows/6);
                cv::Rect leftEyeRect(0,0, faceImage.cols/2, faceImage.rows/2);
                cv::Mat leftTopPart(faceImage, leftEyeRect + shift);
               pt_eyeClassifier->detectMultiScale(leftTopPart.getUMat(cv::ACCESS_FAST), v_eyes, 1.1, 3,  CV_HAAR_FIND_BIGGEST_OBJECT, m_minEyeSize);
                if(v_eyes.size() != 0) {
                    leftEyeRect = v_eyes[0] + shift;
                    cv::Point2f lep(leftEyeRect.x + leftEyeRect.width/2.0f, leftEyeRect.y + leftEyeRect.height/2.0f);
                    shift = cv::Point(faceImage.cols*9/20, faceImage.rows/6);
                    cv::Rect rightEyeRect(0,0, faceImage.cols/2, faceImage.rows/2);
                    cv::Mat rightTopPart(faceImage, rightEyeRect + shift);
                   pt_eyeClassifier->detectMultiScale(rightTopPart.getUMat(cv::ACCESS_FAST), v_eyes, 1.1, 3,  CV_HAAR_FIND_BIGGEST_OBJECT, m_minEyeSize);
                    if(v_eyes.size() != 0) {
                        rightEyeRect = v_eyes[0] + shift;
                        cv::Point2f rep(rightEyeRect.x + rightEyeRect.width/2.0f, rightEyeRect.y + rightEyeRect.height/2.0f);
                        cv::Point2f peyes = rep - lep;
                        m_angle += 90.0 * std::atan(peyes.y / peyes.x) / PI_VALUE;
                    }
                } else {
                    cv::Mat bw;
                    threshSkin(faceImage, bw, 0, 255);
                    int size = (int)(cv::sum(bw)[0]/255.0);
                    if(size > 0) {
                        cv::Mat data_pts = cv::Mat(size, 2, CV_64FC1);
                        int i = 0;
                        uchar *pt;
                        for(int y = 0; y < bw.rows; y++) {
                            pt = bw.ptr(y);
                            for(int x = 0; x < bw.cols; x++)
                                if(pt[x] == 255) {
                                    data_pts.at<double>(i,0) = x;
                                    data_pts.at<double>(i,1) = y;
                                    i++;
                                }
                        }
                        cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
                        cv::Point2d eigenvector(pca_analysis.eigenvectors.at<double>(1,0), pca_analysis.eigenvectors.at<double>(1,1));
                        m_angle += 90.0 * std::atan(eigenvector.y / eigenvector.x) / PI_VALUE; // 180.0 produce angle jitter, and 90.0 looks more stable
                    }
                }
            }
            break;
        }
    }

    cv::Point2f cp_out((m_centerPoint.x - m_xShift*faceRect.width) * xScale, (m_centerPoint.y - m_yShift*faceRect.height) * yScale);
    cv::Size2f size_out(faceRect.width * xScale * m_xPortion, faceRect.height * m_yPortion * yScale);
    m_rRect = cv::RotatedRect(cp_out, size_out, (float)m_angle);
    return m_rRect;
}
//---------------------------------------------------------------------------------
cv::Mat FaceTracker::getFaceImage(const cv::Mat &img)
{
    const cv::RotatedRect rRect = searchFace(img);
    cv::Mat outputImg;
    cv::Rect boundingRect = rRect.boundingRect() & cv::Rect(0,0,img.cols,img.rows);
    if(boundingRect.area() > 1) {
        cv::Mat imgPart(img, boundingRect);
        double angle = rRect.angle;
        if(std::abs(angle) > 0.1) {
            cv::Point2f cp = cv::Point2f(boundingRect.width/2.0f, boundingRect.height/2.0f);
            cv::Mat transform = cv::getRotationMatrix2D(cp, angle, 1.0);
            cv::Point2f vP[4];
            rRect.points(vP);
            int width = (int)( std::sqrt( (vP[2].x-vP[1].x)*(vP[2].x-vP[1].x) + (vP[2].y-vP[1].y)*(vP[2].y-vP[1].y) ) );
            int height = (int)( std::sqrt( (vP[1].x-vP[0].x)*(vP[1].x-vP[0].x) + (vP[1].y-vP[0].y)*(vP[1].y-vP[0].y) ) );
            transform.at<double>(0,2) += -(boundingRect.width - width)/2.0 ;
            transform.at<double>(1,2) += -(boundingRect.height - height)/2.0;
            cv::warpAffine(imgPart, outputImg, transform, cv::Size(width, height));
        } else {
            outputImg = img(boundingRect);
        }
    }      
    return outputImg;
}
//---------------------------------------------------------------------------------
cv::Mat FaceTracker::getFaceGrayImage(const cv::Mat &img)
{
    cv::Mat grayImage;
    if(img.channels() == 3)
        cv::cvtColor(img, grayImage, CV_BGR2GRAY);
    else
        grayImage = img;
    return getFaceImage(grayImage);
}

//---------------------------------------------------------------------------------
cv::Mat FaceTracker::getResizedFaceImage(const cv::Mat &img, const cv::Size &size)
{    
    cv::Mat output;
    cv::Mat faceImg = getFaceImage(img);
    if(faceImg.cols > m_minFaceSize.width && faceImg.rows > m_minFaceSize.height)  {
        cv::Rect2f roiRect(0,0,0,0);
        if( (float)faceImg.cols/faceImg.rows > (float)size.width/size.height) {
            roiRect.height = (float)faceImg.rows;
            roiRect.width = faceImg.rows * (float)size.width/size.height;
            roiRect.x = (faceImg.cols - roiRect.width)/2.0f;
        } else {
            roiRect.width = (float)faceImg.cols;
            roiRect.height = faceImg.cols * (float)size.height/size.width;
            roiRect.y = (faceImg.rows - roiRect.height)/2.0f;
        }
        roiRect &= cv::Rect2f(0, 0, (float)faceImg.cols, (float)faceImg.rows);
        if(roiRect.area() > 0)  {
            cv::Mat croppedImg(faceImg, roiRect);
            int interpolationMethod = 0;
            if(size.area() > roiRect.area())
                interpolationMethod = CV_INTER_CUBIC;
            else
                interpolationMethod = CV_INTER_AREA;
            cv::resize(croppedImg, output, size, 0, 0, interpolationMethod);
        }
    }
    return output;
}
//---------------------------------------------------------------------------------
cv::Mat FaceTracker::getResizedGrayFaceImage(const cv::Mat &img, const cv::Size &size)
{
    cv::Mat grayImage;
    if(img.channels() == 3)
        cv::cvtColor(img, grayImage, CV_BGR2GRAY);
    else
        grayImage = img;
    return getResizedFaceImage(grayImage,size);
}
//---------------------------------------------------------------------------------
void FaceTracker::__updateHistory(const cv::Rect &rect)
{                  
    if(beginFlag) {
        for(uchar i = 0; i < m_historyLength; i++) {
            v_rectHistory[i] = rect;
        }
        beginFlag = false;
    } else {                                       
        v_rectHistory[m_pos] = rect;
        m_pos = (m_pos+1) % m_historyLength;
    }
    m_framesFaceFound++;
}

void FaceTracker::clearMetaData()
{
    m_metaID = -1;
    m_metaInfo = cv::String();
    m_metaConfidence = DBL_MAX;
}

uchar FaceTracker::getHistoryLength() const
{
    return m_historyLength;
}

unsigned int FaceTracker::getFaceTrackedFrames() const
{
    return m_framesFaceFound;
}

dlib::full_object_detection FaceTracker::getFaceShape() const
{
    return faceshape;
}

void FaceTracker::setMetaData(int _id, double _confidence, const cv::String &_info)
{
    m_metaID = _id;
    m_metaConfidence = _confidence;
    m_metaInfo = _info;
}

bool FaceTracker::updateMetaData(int _id, double _confidence, const cv::String &_info)
{
    // Update if two sucessive recognition events have same label (it should decrease FAR1 = FAR0*FAR0 and decrease TAR1 = TAR0*TAR0)
    static int p = 0;
    v_metaID[p] = _id;
    bool _hasbeenupdated = false;
    if(v_metaID[__loop(p,(int)v_metaID.size())] == v_metaID[__loop(p - 1,(int)v_metaID.size())]) {
        setMetaData(_id,_confidence,_info);
        _hasbeenupdated = true;
    }
    p = (p+1) % v_metaID.size();
    return _hasbeenupdated;
}


int FaceTracker::__loop(int _d, int _size) const
{
    return ((_d % _size) + _size) % _size;
}

cv::String FaceTracker::getMetaInfo() const
{
    return m_metaInfo;
}

int FaceTracker::getMetaID() const
{
    return m_metaID;
}

double FaceTracker::getMetaConfidence() const
{
    return m_metaConfidence;
}
//---------------------------------------------------------------------------------
cv::Rect FaceTracker::__getAverageFaceRect() const
{
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    for(uchar i = 0; i < m_historyLength; i++) {
        x += v_rectHistory[i].x;
        y += v_rectHistory[i].y;
        w += v_rectHistory[i].width;
        h += v_rectHistory[i].height;
    }
    x /= m_historyLength;
    y /= m_historyLength;
    w /= m_historyLength;
    h /= m_historyLength;
    return cv::Rect((int)x, (int)y, (int)w, (int)h);
}
//---------------------------------------------------------------------------------
bool FaceTracker::isSkinColor(const uchar vR, const uchar vG, const uchar vB)
{
    if( (vR > 95) && (vR > vG) && (vG > 40) && (vB > 20) &&
        ((vR - std::min(vG,vB)) > 5) && ((vR - vG) > 5 ) )
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------------
void FaceTracker::threshSkin(const cv::Mat &inputArray, cv::Mat &outputArray, uchar minVal, uchar maxVal)
{
    outputArray = cv::Mat(inputArray.rows, inputArray.cols, CV_8UC1);
    uchar *p_output;
    const uchar *p_input;
    #pragma omp parallel for private(p_output, p_input)
    for(int y = 0; y < inputArray.rows; y++) {
        p_input = inputArray.ptr<const uchar>(y);
        p_output = outputArray.ptr(y);
        for(int x = 0; x < inputArray.cols; x++) {
            if(isSkinColor(p_input[x*3+2], p_input[x*3+1], p_input[x*3]))
                p_output[x] = maxVal;
            else
                p_output[x] = minVal;
        }
    }
}
//---------------------------------------------------------------------------------
void FaceTracker::resetHistory()
{
    beginFlag = true;
    cv::Rect zeroRect(0,0,0,0);
    for(uchar i = 0; i < m_historyLength; ++i) {
        v_rectHistory[i] = zeroRect;
    }
    m_rRect = cv::RotatedRect();
    m_framesFaceFound = 0;
}
//---------------------------------------------------------------------------------
cv::RotatedRect FaceTracker::getFaceRotatedRect() const
{
    return m_rRect;
}
//---------------------------------------------------------------------------------
void FaceTracker::setMinFaceSize(const cv::Size size)
{
    m_minFaceSize = size;
    m_maxFaceSize = cv::Size(size.width*5, size.height*5);
    m_minEyeSize = cv::Size(0,0);
    //m_minEyeSize = cv::Size(size.width/8, size.height/10);
}
//---------------------------------------------------------------------------------
void FaceTracker::setMinNeighbours(int value)
{
    m_minNeighbours = value;
}
//---------------------------------------------------------------------------------
void FaceTracker::setFaceRectPortions(float xPortion, float yPortion)
{
    m_xPortion = xPortion;
    m_yPortion = yPortion;
}
//---------------------------------------------------------------------------------
void FaceTracker::setFaceRectShifts(float _xShift, float _yShift)
{
    m_xShift = _xShift;
    m_yShift = _yShift;
}

void FaceTracker::setFaceAlignMethod(FaceTracker::AlignMethod _method)
{
    m_method = _method;
}

FaceTracker::AlignMethod FaceTracker::getFaceAlignMethod() const
{
    return m_method;
}
//---------------------------------------------------------------------------------
