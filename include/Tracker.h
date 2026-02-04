#ifndef TRACKER_H
#define TRACKER_H

#include <vector>
#include <memory>
#include "Track.h"
#include "Detection.h"

class Tracker {
public:
    Tracker(float maxIoUDistance = 0.7f, int maxAge = 30, int minHits = 3);
    
    std::vector<std::shared_ptr<Track>> update(const std::vector<Detection>& detections);
    
    int getTotalTracks() const { return nextId; }
    
private:
    std::vector<std::shared_ptr<Track>> tracks;
    int nextId;
    float maxIoUDistance;
    int maxAge;
    int minHits;
    
    // Calculate IoU (Intersection over Union) between two bounding boxes
    float calculateIoU(const cv::Rect& box1, const cv::Rect& box2) const;
    
    // Create cost matrix for Hungarian algorithm
    std::vector<std::vector<float>> createCostMatrix(
        const std::vector<std::shared_ptr<Track>>& tracks,
        const std::vector<Detection>& detections) const;
    
    // Associate detections to tracks
    void associate(const std::vector<Detection>& detections,
                  std::vector<int>& matchedTracks,
                  std::vector<int>& matchedDetections,
                  std::vector<int>& unmatchedTracks,
                  std::vector<int>& unmatchedDetections);
};

#endif // TRACKER_H
