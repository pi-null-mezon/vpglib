/***************************************************************
 * FaceTracker is a class that searches biggest face on image.
 * FaceTracker based on cv::CascadeClassifier but works in a wider
 * range of angles.
 * Alex (pi-null-mezon) Taranov, 2016
 ***************************************************************/
#ifndef FACETRACKER_H
#define FACETRACKER_H

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv.h>

class FaceTracker
{
public:
    /**
     * @brief The AlignMethod enum
     */
    enum AlignMethod {Eyes, Skin, Otsu, EyesAndSkin, EyesThenSkin, FaceShape, NoAlign};
    /**
     * @brief The PrimaryFaceDetector enum
     */
    enum PrimaryFaceDetector {ViolaJones, HOG};
    /**
     * @brief FaceTracker
     * @param length - history length
     * @param method - method of alignment
     * @note if you will pass length equal to 1, then facetracker will work in single image face detection mode
     */
    FaceTracker(uchar length = 4, AlignMethod method = Eyes);
    ~FaceTracker();
    /**
     * @brief searchFace
     * @param img - input image
     * @return RotatedRect that embraces the face on img
     */
    cv::RotatedRect searchFace(const cv::Mat &img);
    /**
     * @brief getFaceImage
     * @param img - input image
     * @return aligned image of the face (could be empty, should be checked on the caller side)
     * @note could return an empty image, so check it after call
     */
    cv::Mat getFaceImage(const cv::Mat &img);
    /**
     * @brief getFaceImage
     * @param img - input image
     * @return aligned image of the face (could be empty, should be checked on the caller side) with only one channel
     * @note could return an empty image, so check it after call
     */
    cv::Mat getFaceGrayImage(const cv::Mat &img);
    /**
     * @brief loadFaceClassifier
     * @param pointer - pointer to an object instance
     * @return has classifier been loaded or not
     */
    bool setFaceClassifier(cv::CascadeClassifier *pointer);
    /**
     * @brief loadEyeClassifier
     * @param pointer - pointer to an object instance
     * @return has classifier been loaded or not
     */
    bool setEyeClassifier(cv::CascadeClassifier *pointer);
    /**
     * @brief setFaceShapePredictor
     * @param _dlibfaceshapepredictor - pointer to an object instance (use 68 point detector)
     */
    void setFaceShapePredictor(dlib::shape_predictor *_dlibfaceshapepredictor);
    /**
     * @brief threshSkin
     * @param inputArray - input image with BGR(CV_8UC3) format
     * @param outputArray - output CV_8UC1 format
     * @param minVal - output pixel value if not skin
     * @param maxVal - output pixel value if skin
     */
    static void threshSkin(const cv::Mat &inputArray, cv::Mat &outputArray, uchar minVal, uchar maxVal);
    /**
     * @brief isSkinColor probes RGB pixel to be a skin projection
     * @param vR
     * @param vG
     * @param vB
     * @return
     */
    static bool isSkinColor(const uchar vR, const uchar vG, const uchar vB);
    /**
     * @brief resetHistory resets history vector
     */
    void resetHistory();
    /**
     * @brief getFaceRotatedRect
     * @return rotated rect that embraces the face
     */
    cv::RotatedRect getFaceRotatedRect() const;
    /**
     * @brief getResizedFaceImage
     * @param img - input image
     * @param size - desired face size
     * @return resized image
     * @note image will be cropped before resize, so the possible distortion should be minimal
     * @note could return an empty image, so it is your responsibility to check it
     */
    cv::Mat getResizedFaceImage(const cv::Mat &img, const cv::Size &size);
    /**
     * @brief getResizedFaceImageGray
     * @param img - input image
     * @param size - desired face size
     * @return resized image with one channel
     * @note image will be cropped before resize, so the possible distortion should be minimal
     * @note could return an empty image, so it is your responsibility to check it
     */
    cv::Mat getResizedGrayFaceImage(const cv::Mat &img, const cv::Size &size);
    /**
     * @brief setMinFaceSize
     * @param size - minimal recognizable face size, maximum size will be 10 times bigger
     */
    void setMinFaceSize(const cv::Size size);
    /**
     * @brief setMinNeighbours for face detector
     * @param value - threshold value
     */
    void setMinNeighbours(int value);
    /**
     * @brief setFaceRectPortions
     * @param xPortion - portion of face rect along horizontal dimension, e.g. 0.9 means 90 % of face rect width
     * @param yPortion - portion of face rect along vertical dimension, e.g 1.1 means 110% of face rect height
     */
    void setFaceRectPortions(float xPortion, float yPortion);    
    /**
     * @brief setFaceRectShifts
     * @param _xShift - portion of face rect along horizontal dimension that will be added to center point of returned face image
     * @param _yShift - portion of face rect along vertical dimension that will be added to center point of returned face image
     */
    void setFaceRectShifts(float _xShift, float _yShift);
    /**
     * @brief setFaceAlignMethod - use to setup face align algorithm
     * @param _method - self explained
     */
    void setFaceAlignMethod(FaceTracker::AlignMethod _method);
    /**
     * @brief getFaceAlignMethod - use to chek which one of the face align methods is used
     * @return current face align method, it could be one of FaceTracker::AlignMethod enum's entity
     */
    FaceTracker::AlignMethod getFaceAlignMethod() const;

    // Face recognition assistance interface---------------------------
    /**
     * @brief getMetaInfo - self explained
     * @return string of meata information about tracked face
     * @note metainformation is cleared each time tracker lost a face, after clear function returns an empty string
     */    
    cv::String getMetaInfo() const;
    /**
     * @brief getMetaID - self explained
     * @return numerical identifier of the tracked face
     * @note metainformation is cleared each time tracker lost a face, after clear function returns -1
     */
    int getMetaID() const;
    /**
     * @brief getMetaConfidence -self explained
     * @return confidence of the _id
     * @note metainformation is cleared each time tracker lost a face, after clear function returns DBL_MAX
     */
    double getMetaConfidence() const;
    /**
     * @brief It is interface for the facerecognition tasks - set meta data to the tracker
     * @param _id - numerical identifier, same as label
     * @param _confidence - confidence for the _id
     * @param _info - information about _id
     * @note meta data is cleared each time tracker lost a face
     */
    void setMetaData(int _id, double _confidence, const cv::String &_info);
    /**
     * @brief In contrast with setMetaData this method will check if _confidence greater* than the previous one, and change _id only if the result is true
     * @param _id - numerical identifier, same as label
     * @param _confidence - confidence for the _id
     * @param _info - information about _id
     * @return true if id has been updated
     * @note As our facerecognizer use distance as confidence, than for this particular case greater confidence means lower distance
     */
    bool updateMetaData(int _id, double _confidence, const cv::String &_info);
    /**
     * @brief call to clear all metadata
     */
    void clearMetaData();
    /**
     * @brief getHistoryLength
     * @return actual history length
     */
    uchar getHistoryLength() const;
    /**
     * @brief getFaceTrackedFrames
     * @return how many frames this tracker tracks face
     */
    unsigned int getFaceTrackedFrames() const;
    //--------------------------------------------------------------------

    dlib::full_object_detection getFaceShape() const;

private:
    cv::Rect    __getAverageFaceRect() const;
    void        __updateHistory(const cv::Rect &rect);
    inline int  __loop(int _d, int _size) const;

    cv::CascadeClassifier *pt_faceClassifier;
    cv::CascadeClassifier *pt_eyeClassifier;
    dlib::shape_predictor *pt_dlibfaceshapepredictor;
    cv::Size m_minFaceSize;
    cv::Size m_maxFaceSize;
    cv::Size m_minEyeSize;

    cv::Rect *v_rectHistory;
    cv::RotatedRect m_rRect;

    uchar m_historyLength;
    uchar m_emptyFrames;
    unsigned int m_framesFaceFound;

    uchar m_pos;
    bool beginFlag;

    double m_angle;
    cv::Point2f m_centerPoint;
    AlignMethod m_method;

    int m_minNeighbours;
    float m_xPortion;
    float m_yPortion;
    float m_xShift;
    float m_yShift;

    cv::String m_metaInfo;
    int m_metaID;
    double m_metaConfidence;

    std::vector<int> v_metaID;

    dlib::frontal_face_detector dlibfacedet;
    dlib::full_object_detection faceshape;
    PrimaryFaceDetector m_primaryfacedetectortupe;
};

#endif // FACETRACKER_H
