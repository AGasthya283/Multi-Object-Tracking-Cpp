#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <opencv2/opencv.hpp>

class KalmanFilter {
public:
    KalmanFilter();
    
    void init(const cv::Rect& bbox);
    cv::Rect predict();
    void update(const cv::Rect& bbox);
    
private:
    cv::KalmanFilter kf;
    bool initialized;
    
    // Convert bbox to state vector [x, y, w, h, vx, vy, vw, vh]
    cv::Mat bboxToState(const cv::Rect& bbox) const;
    cv::Rect stateToBbox(const cv::Mat& state) const;
};

#endif // KALMAN_FILTER_H
