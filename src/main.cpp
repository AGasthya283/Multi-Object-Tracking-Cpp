#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include "YOLODetector.h"
#include "Tracker.h"

// Color palette for visualization
std::vector<cv::Scalar> generateColors(int n) {
    std::vector<cv::Scalar> colors;
    for (int i = 0; i < n; ++i) {
        int hue = (i * 180 / n) % 180;
        cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(hue, 255, 255));
        cv::Mat bgr;
        cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
        colors.push_back(cv::Scalar(bgr.at<cv::Vec3b>(0, 0)[0],
                                     bgr.at<cv::Vec3b>(0, 0)[1],
                                     bgr.at<cv::Vec3b>(0, 0)[2]));
    }
    return colors;
}

void drawTracks(cv::Mat& frame, const std::vector<std::shared_ptr<Track>>& tracks,
                const std::vector<cv::Scalar>& colors) {
    for (const auto& track : tracks) {
        int id = track->getId();
        cv::Rect bbox = track->getCurrentBbox();
        cv::Scalar color = colors[id % colors.size()];
        
        // Draw bounding box
        cv::rectangle(frame, bbox, color, 2);
        
        // Draw track ID and class
        std::string label = "ID:" + std::to_string(id) + " " + track->getClassName();
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 
                                             0.5, 1, &baseLine);
        
        int top = std::max(bbox.y, labelSize.height);
        cv::rectangle(frame, 
                     cv::Point(bbox.x, top - labelSize.height - 5),
                     cv::Point(bbox.x + labelSize.width, top + baseLine),
                     color, cv::FILLED);
        cv::putText(frame, label, cv::Point(bbox.x, top - 2),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        
        // Draw trajectory
        std::vector<cv::Point> trajectory = track->getTrajectory();
        for (size_t i = 1; i < trajectory.size(); ++i) {
            cv::line(frame, trajectory[i-1], trajectory[i], color, 2);
        }
    }
}

void displayStats(cv::Mat& frame, int frameCount, double fps, int trackCount) {
    std::stringstream ss;
    ss << "Frame: " << frameCount << " | FPS: " << std::fixed << std::setprecision(1) 
       << fps << " | Tracks: " << trackCount;
    
    std::string text = ss.str();
    int baseLine;
    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine);
    
    cv::rectangle(frame, cv::Point(10, 10),
                 cv::Point(20 + textSize.width, 30 + textSize.height),
                 cv::Scalar(0, 0, 0), cv::FILLED);
    cv::putText(frame, text, cv::Point(15, 35),
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
}

int main(int argc, char** argv) {
    // Parse command line arguments
    std::string videoPath = "input.mp4";
    std::string modelPath = "models/yolov4-tiny.weights";
    std::string configPath = "models/yolov4-tiny.cfg";
    std::string classesPath = "models/coco.names";
    std::string outputPath = "output.avi";
    
    if (argc >= 2) videoPath = argv[1];
    if (argc >= 3) modelPath = argv[2];
    if (argc >= 4) configPath = argv[3];
    if (argc >= 5) classesPath = argv[4];
    if (argc >= 6) outputPath = argv[5];
    
    std::cout << "=== Multi-Object Tracking System ===" << std::endl;
    std::cout << "Video: " << videoPath << std::endl;
    std::cout << "Model: " << modelPath << std::endl;
    std::cout << "Config: " << configPath << std::endl;
    std::cout << "Classes: " << classesPath << std::endl;
    std::cout << "Output: " << outputPath << std::endl;
    std::cout << "====================================" << std::endl;
    
    // Initialize video capture
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file: " << videoPath << std::endl;
        return -1;
    }
    
    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double inputFps = cap.get(cv::CAP_PROP_FPS);
    
    std::cout << "Video resolution: " << frameWidth << "x" << frameHeight << std::endl;
    std::cout << "Input FPS: " << inputFps << std::endl;
    
    // Initialize video writer
    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('M','J','P','G'),
                          inputFps, cv::Size(frameWidth, frameHeight));
    
    // Initialize detector and tracker
    YOLODetector detector(modelPath, configPath, classesPath);
    if (!detector.isLoaded()) {
        std::cerr << "Error: Failed to load YOLO model!" << std::endl;
        return -1;
    }
    
    Tracker tracker(0.7f, 30, 3);
    
    // Generate color palette
    std::vector<cv::Scalar> colors = generateColors(100);
    
    // Processing loop
    cv::Mat frame;
    int frameCount = 0;
    double totalTime = 0.0;
    
    std::cout << "\nProcessing video..." << std::endl;
    
    while (cap.read(frame)) {
        auto startTime = cv::getTickCount();
        
        // Detect objects
        std::vector<Detection> detections = detector.detect(frame, 0.5f, 0.4f);
        
        // Update tracker
        std::vector<std::shared_ptr<Track>> tracks = tracker.update(detections);
        
        // Draw results
        drawTracks(frame, tracks, colors);
        
        // Calculate FPS
        auto endTime = cv::getTickCount();
        double frameTime = (endTime - startTime) / cv::getTickFrequency();
        totalTime += frameTime;
        double fps = 1.0 / frameTime;
        
        // Display stats
        displayStats(frame, frameCount + 1, fps, tracks.size());
        
        // Write frame
        writer.write(frame);
        
        // Display frame
        cv::imshow("Multi-Object Tracking", frame);
        
        frameCount++;
        
        // Print progress
        if (frameCount % 30 == 0) {
            std::cout << "Processed " << frameCount << " frames, "
                     << "Average FPS: " << frameCount / totalTime << std::endl;
        }
        
        // Exit on 'q' key
        if (cv::waitKey(1) == 'q') {
            std::cout << "User requested exit." << std::endl;
            break;
        }
    }
    
    // Cleanup
    cap.release();
    writer.release();
    cv::destroyAllWindows();
    
    // Print final statistics
    std::cout << "\n=== Processing Complete ===" << std::endl;
    std::cout << "Total frames: " << frameCount << std::endl;
    std::cout << "Total time: " << totalTime << " seconds" << std::endl;
    std::cout << "Average FPS: " << (frameCount / totalTime) << std::endl;
    std::cout << "Total unique tracks: " << tracker.getTotalTracks() - 1 << std::endl;
    std::cout << "Output saved to: " << outputPath << std::endl;
    
    return 0;
}
