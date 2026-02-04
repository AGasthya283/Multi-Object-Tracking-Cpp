#include "YOLODetector.h"
#include <fstream>
#include <iostream>

YOLODetector::YOLODetector(const std::string& modelPath, const std::string& configPath, 
                           const std::string& classesPath) 
    : inputSize(416, 416) {
    
    try {
        // Load YOLO network
        net = cv::dnn::readNetFromDarknet(configPath, modelPath);
        
        // Set backend and target
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        // Load class names
        loadClassNames(classesPath);
        
        std::cout << "YOLO model loaded successfully!" << std::endl;
        std::cout << "Loaded " << classNames.size() << " classes" << std::endl;
    }
    catch (const cv::Exception& e) {
        std::cerr << "Error loading YOLO model: " << e.what() << std::endl;
    }
}

void YOLODetector::loadClassNames(const std::string& classesPath) {
    std::ifstream ifs(classesPath);
    if (!ifs.is_open()) {
        std::cerr << "Could not open classes file: " << classesPath << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) {
            classNames.push_back(line);
        }
    }
}

std::vector<cv::String> YOLODetector::getOutputNames() {
    static std::vector<cv::String> names;
    if (names.empty()) {
        std::vector<int> outLayers = net.getUnconnectedOutLayers();
        std::vector<cv::String> layersNames = net.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i) {
            names[i] = layersNames[outLayers[i] - 1];
        }
    }
    return names;
}

std::vector<Detection> YOLODetector::detect(const cv::Mat& frame, float confThreshold, 
                                             float nmsThreshold) {
    std::vector<Detection> detections;
    
    if (net.empty()) {
        std::cerr << "Network not loaded!" << std::endl;
        return detections;
    }
    
    // Create blob from frame
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1/255.0, inputSize, cv::Scalar(), true, false);
    
    // Set input to network
    net.setInput(blob);
    
    // Forward pass
    std::vector<cv::Mat> outs;
    net.forward(outs, getOutputNames());
    
    // Process outputs
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    
    for (size_t i = 0; i < outs.size(); ++i) {
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            
            if (confidence > confThreshold) {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                
                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }
    
    // Apply Non-Maximum Suppression
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    
    // Create Detection objects
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        std::string className = classIds[idx] < classNames.size() ? 
                                classNames[classIds[idx]] : "unknown";
        detections.emplace_back(boxes[idx], confidences[idx], classIds[idx], className);
    }
    
    return detections;
}
