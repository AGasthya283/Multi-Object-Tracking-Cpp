#include "KalmanFilter.h"

KalmanFilter::KalmanFilter() : initialized(false) {
    // State: [x, y, w, h, vx, vy, vw, vh]
    // Measurement: [x, y, w, h]
    kf.init(8, 4, 0);
    
    // Transition matrix (F)
    cv::setIdentity(kf.transitionMatrix);
    for (int i = 0; i < 4; ++i) {
        kf.transitionMatrix.at<float>(i, i + 4) = 1.0f; // position += velocity
    }
    
    // Measurement matrix (H)
    kf.measurementMatrix = cv::Mat::zeros(4, 8, CV_32F);
    for (int i = 0; i < 4; ++i) {
        kf.measurementMatrix.at<float>(i, i) = 1.0f;
    }
    
    // Process noise covariance (Q)
    cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-2));
    
    // Measurement noise covariance (R)
    cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-1));
    
    // Error covariance (P)
    cv::setIdentity(kf.errorCovPost, cv::Scalar::all(1));
}

void KalmanFilter::init(const cv::Rect& bbox) {
    kf.statePost = bboxToState(bbox);
    initialized = true;
}

cv::Rect KalmanFilter::predict() {
    if (!initialized) {
        return cv::Rect();
    }
    
    cv::Mat prediction = kf.predict();
    return stateToBbox(prediction);
}

void KalmanFilter::update(const cv::Rect& bbox) {
    if (!initialized) {
        init(bbox);
        return;
    }
    
    cv::Mat measurement = cv::Mat::zeros(4, 1, CV_32F);
    measurement.at<float>(0) = bbox.x + bbox.width / 2.0f;
    measurement.at<float>(1) = bbox.y + bbox.height / 2.0f;
    measurement.at<float>(2) = bbox.width;
    measurement.at<float>(3) = bbox.height;
    
    kf.correct(measurement);
}

cv::Mat KalmanFilter::bboxToState(const cv::Rect& bbox) const {
    cv::Mat state = cv::Mat::zeros(8, 1, CV_32F);
    state.at<float>(0) = bbox.x + bbox.width / 2.0f;   // center x
    state.at<float>(1) = bbox.y + bbox.height / 2.0f;  // center y
    state.at<float>(2) = bbox.width;
    state.at<float>(3) = bbox.height;
    // velocities initialized to 0
    return state;
}

cv::Rect KalmanFilter::stateToBbox(const cv::Mat& state) const {
    float cx = state.at<float>(0);
    float cy = state.at<float>(1);
    float w = state.at<float>(2);
    float h = state.at<float>(3);
    
    return cv::Rect(
        static_cast<int>(cx - w / 2),
        static_cast<int>(cy - h / 2),
        static_cast<int>(w),
        static_cast<int>(h)
    );
}
