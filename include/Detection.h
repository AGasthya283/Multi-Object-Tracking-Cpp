#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include <vector>

struct Detection {
    cv::Rect bbox;
    float confidence;
    int classId;
    std::string className;
    
    Detection() : confidence(0.0f), classId(-1) {}
    
    Detection(const cv::Rect& box, float conf, int cls, const std::string& name = "")
        : bbox(box), confidence(conf), classId(cls), className(name) {}
};

#endif // DETECTION_H
