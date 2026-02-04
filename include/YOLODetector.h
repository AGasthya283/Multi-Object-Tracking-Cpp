#ifndef YOLO_DETECTOR_H
#define YOLO_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include "Detection.h"

class YOLODetector {
public:
    YOLODetector(const std::string& modelPath, const std::string& configPath, 
                 const std::string& classesPath);
    
    std::vector<Detection> detect(const cv::Mat& frame, float confThreshold = 0.5f, 
                                   float nmsThreshold = 0.4f);
    
    bool isLoaded() const { return !net.empty(); }
    
private:
    cv::dnn::Net net;
    std::vector<std::string> classNames;
    cv::Size inputSize;
    
    void loadClassNames(const std::string& classesPath);
    std::vector<cv::String> getOutputNames();
};

#endif // YOLO_DETECTOR_H
