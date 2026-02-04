#include "Tracker.h"
#include "HungarianAlgorithm.h"
#include <algorithm>

Tracker::Tracker(float maxIoUDistance, int maxAge, int minHits)
    : nextId(1), maxIoUDistance(maxIoUDistance), maxAge(maxAge), minHits(minHits) {
}

float Tracker::calculateIoU(const cv::Rect& box1, const cv::Rect& box2) const {
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);
    
    int intersectionArea = std::max(0, x2 - x1) * std::max(0, y2 - y1);
    int box1Area = box1.width * box1.height;
    int box2Area = box2.width * box2.height;
    int unionArea = box1Area + box2Area - intersectionArea;
    
    return unionArea > 0 ? static_cast<float>(intersectionArea) / unionArea : 0.0f;
}

std::vector<std::vector<float>> Tracker::createCostMatrix(
    const std::vector<std::shared_ptr<Track>>& tracks,
    const std::vector<Detection>& detections) const {
    
    std::vector<std::vector<float>> costMatrix(tracks.size(), 
                                                std::vector<float>(detections.size()));
    
    for (size_t i = 0; i < tracks.size(); ++i) {
        cv::Rect predictedBbox = tracks[i]->getPredictedBbox();
        
        for (size_t j = 0; j < detections.size(); ++j) {
            float iou = calculateIoU(predictedBbox, detections[j].bbox);
            
            // Convert IoU to cost (1 - IoU)
            // Also check if classes match
            if (tracks[i]->getClassId() != detections[j].classId) {
                costMatrix[i][j] = 1.0f; // Maximum cost for different classes
            } else {
                costMatrix[i][j] = 1.0f - iou;
            }
        }
    }
    
    return costMatrix;
}

void Tracker::associate(const std::vector<Detection>& detections,
                       std::vector<int>& matchedTracks,
                       std::vector<int>& matchedDetections,
                       std::vector<int>& unmatchedTracks,
                       std::vector<int>& unmatchedDetections) {
    
    matchedTracks.clear();
    matchedDetections.clear();
    unmatchedTracks.clear();
    unmatchedDetections.clear();
    
    if (tracks.empty()) {
        for (size_t i = 0; i < detections.size(); ++i) {
            unmatchedDetections.push_back(i);
        }
        return;
    }
    
    if (detections.empty()) {
        for (size_t i = 0; i < tracks.size(); ++i) {
            unmatchedTracks.push_back(i);
        }
        return;
    }
    
    // Create cost matrix
    std::vector<std::vector<float>> costMatrix = createCostMatrix(tracks, detections);
    
    // Solve assignment problem
    std::vector<int> assignment = HungarianAlgorithm::solve(costMatrix);
    
    // Process assignments
    std::vector<bool> detectionMatched(detections.size(), false);
    
    for (size_t i = 0; i < assignment.size(); ++i) {
        if (assignment[i] >= 0 && costMatrix[i][assignment[i]] < maxIoUDistance) {
            matchedTracks.push_back(i);
            matchedDetections.push_back(assignment[i]);
            detectionMatched[assignment[i]] = true;
        } else {
            unmatchedTracks.push_back(i);
        }
    }
    
    // Find unmatched detections
    for (size_t i = 0; i < detections.size(); ++i) {
        if (!detectionMatched[i]) {
            unmatchedDetections.push_back(i);
        }
    }
}

std::vector<std::shared_ptr<Track>> Tracker::update(const std::vector<Detection>& detections) {
    // Predict new locations for all tracks
    for (auto& track : tracks) {
        track->predict();
    }
    
    // Associate detections to tracks
    std::vector<int> matchedTracks, matchedDetections;
    std::vector<int> unmatchedTracks, unmatchedDetections;
    
    associate(detections, matchedTracks, matchedDetections, 
             unmatchedTracks, unmatchedDetections);
    
    // Update matched tracks
    for (size_t i = 0; i < matchedTracks.size(); ++i) {
        int trackIdx = matchedTracks[i];
        int detectionIdx = matchedDetections[i];
        tracks[trackIdx]->update(detections[detectionIdx].bbox);
    }
    
    // Mark unmatched tracks as missed
    for (int trackIdx : unmatchedTracks) {
        tracks[trackIdx]->markMissed();
    }
    
    // Create new tracks for unmatched detections
    for (int detectionIdx : unmatchedDetections) {
        const Detection& det = detections[detectionIdx];
        auto newTrack = std::make_shared<Track>(det.bbox, det.classId, 
                                                 det.className, nextId++);
        tracks.push_back(newTrack);
    }
    
    // Remove dead tracks
    tracks.erase(
        std::remove_if(tracks.begin(), tracks.end(),
            [this](const std::shared_ptr<Track>& track) {
                return track->getTimeSinceUpdate() > maxAge;
            }),
        tracks.end()
    );
    
    // Return only confirmed tracks
    std::vector<std::shared_ptr<Track>> confirmedTracks;
    for (const auto& track : tracks) {
        if (track->getState() == TrackState::Confirmed || 
            track->getHitStreak() >= minHits) {
            confirmedTracks.push_back(track);
        }
    }
    
    return confirmedTracks;
}
