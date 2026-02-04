#ifndef TRACK_H
#define TRACK_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <deque>
#include "KalmanFilter.h"

enum class TrackState {
    Tentative,
    Confirmed,
    Deleted
};

class Track {
public:
    Track(const cv::Rect& bbox, int classId, const std::string& className, int trackId);
    
    void predict();
    void update(const cv::Rect& bbox);
    
    cv::Rect getPredictedBbox() const;
    cv::Rect getCurrentBbox() const;
    int getId() const { return id; }
    int getClassId() const { return classId; }
    std::string getClassName() const { return className; }
    TrackState getState() const { return state; }
    int getTimeSinceUpdate() const { return timeSinceUpdate; }
    int getHitStreak() const { return hitStreak; }
    std::vector<cv::Point> getTrajectory() const;
    
    void markMissed();
    void markHit();
    void setState(TrackState newState) { state = newState; }
    
private:
    int id;
    int classId;
    std::string className;
    KalmanFilter kf;
    TrackState state;
    int timeSinceUpdate;
    int hitStreak;
    int age;
    std::deque<cv::Point> trajectory;
    static const int MAX_TRAJECTORY_LENGTH = 30;
};

#endif // TRACK_H
