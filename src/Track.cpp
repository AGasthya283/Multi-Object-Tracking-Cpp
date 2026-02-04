#include "Track.h"

Track::Track(const cv::Rect& bbox, int classId, const std::string& className, int trackId)
    : id(trackId), classId(classId), className(className), 
      state(TrackState::Tentative), timeSinceUpdate(0), hitStreak(0), age(0) {
    kf.init(bbox);
    
    // Initialize trajectory with center of bbox
    cv::Point center(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
    trajectory.push_back(center);
}

void Track::predict() {
    kf.predict();
    age++;
    timeSinceUpdate++;
}

void Track::update(const cv::Rect& bbox) {
    kf.update(bbox);
    timeSinceUpdate = 0;
    hitStreak++;
    
    // Update trajectory
    cv::Point center(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
    trajectory.push_back(center);
    
    // Limit trajectory length
    if (trajectory.size() > MAX_TRAJECTORY_LENGTH) {
        trajectory.pop_front();
    }
    
    // Transition to confirmed state after enough hits
    if (state == TrackState::Tentative && hitStreak >= 3) {
        state = TrackState::Confirmed;
    }
}

cv::Rect Track::getPredictedBbox() const {
    return const_cast<Track*>(this)->kf.predict();
}

cv::Rect Track::getCurrentBbox() const {
    // Return the last predicted bbox
    return const_cast<Track*>(this)->kf.predict();
}

std::vector<cv::Point> Track::getTrajectory() const {
    return std::vector<cv::Point>(trajectory.begin(), trajectory.end());
}

void Track::markMissed() {
    timeSinceUpdate++;
    hitStreak = 0;
}

void Track::markHit() {
    hitStreak++;
}
