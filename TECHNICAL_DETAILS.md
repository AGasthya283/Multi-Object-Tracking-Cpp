# Technical Details - Multi-Object Tracking System

## Table of Contents
1. [Architecture Overview](#architecture-overview)
2. [Object Detection Pipeline](#object-detection-pipeline)
3. [Tracking Algorithm](#tracking-algorithm)
4. [Motion Prediction](#motion-prediction)
5. [Data Association](#data-association)
6. [Performance Analysis](#performance-analysis)

---

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────────┐
│                     Video Input Stream                        │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                   Frame Preprocessing                         │
│  - Resize, Normalize, Blob Creation                          │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  YOLO Object Detector                         │
│  - Forward Pass through Network                               │
│  - Non-Maximum Suppression                                   │
│  Output: List of Detection{bbox, confidence, class}          │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    Track Prediction                           │
│  - Kalman Filter Predict for Each Track                      │
│  Output: Predicted Bounding Boxes                            │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  Data Association                             │
│  - Compute IoU Cost Matrix                                   │
│  - Hungarian Algorithm Assignment                            │
│  Output: Matched & Unmatched Detections/Tracks              │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    Track Management                           │
│  - Update Matched Tracks (Kalman Update)                    │
│  - Create New Tracks (Unmatched Detections)                 │
│  - Delete Dead Tracks (Exceeded max_age)                    │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    Visualization                              │
│  - Draw Bounding Boxes, IDs, Trajectories                   │
│  - Display Statistics                                        │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    Output Video Stream                        │
└─────────────────────────────────────────────────────────────┘
```

---

## Object Detection Pipeline

### YOLO (You Only Look Once) v4-tiny

**Architecture**: Single-stage detector that treats object detection as a regression problem.

**Network Structure**:
- **Backbone**: CSPDarknet53-tiny (feature extractor)
- **Neck**: PANet for multi-scale feature fusion
- **Head**: Detection heads at multiple scales

**Input Processing**:
```
Original Image (H×W×3) → Resize (416×416×3) → 
Normalize (÷255) → Blob (1×3×416×416) → Network
```

**Output Tensor**: [1 × 85 × 13 × 13] for each detection scale
- 85 = 4 (bbox) + 1 (objectness) + 80 (classes for COCO)

**Post-Processing**:
1. **Confidence Filtering**: Keep boxes with `objectness > threshold`
2. **Class Prediction**: `argmax(class_scores)` for each box
3. **NMS (Non-Maximum Suppression)**:
   ```
   For each class:
       Sort boxes by confidence descending
       While boxes remain:
           Pick highest confidence box
           Remove boxes with IoU > nms_threshold
   ```

**Coordinate Transform**:
```cpp
// Network outputs normalized coordinates [0, 1]
centerX = data[0] * frameWidth;
centerY = data[1] * frameHeight;
width = data[2] * frameWidth;
height = data[3] * frameHeight;

// Convert to top-left corner format
bbox = Rect(centerX - width/2, centerY - height/2, width, height);
```

---

## Tracking Algorithm

### SORT (Simple Online and Realtime Tracking)

**Core Concept**: Frame-by-frame data association with Kalman filtering

**State Representation**:
Each track maintains:
- Position: [x, y, w, h] in image coordinates
- Velocity: [vx, vy, vw, vh] in pixels/frame
- Unique ID: Integer identifier
- Hit streak: Consecutive frames with detection
- Time since update: Frames without detection
- Track state: Tentative → Confirmed → Deleted

**Algorithm Flow**:

```python
SORT Algorithm (per frame):
    
    1. Prediction Phase:
       For each existing track:
           predicted_bbox = kalman_filter.predict()
    
    2. Detection Phase:
       detections = yolo_detector.detect(frame)
    
    3. Association Phase:
       cost_matrix = compute_iou_cost(tracks, detections)
       assignments = hungarian_algorithm(cost_matrix)
       
       matched_tracks = []
       matched_detections = []
       unmatched_tracks = []
       unmatched_detections = []
       
       For each assignment (track_idx, detection_idx):
           if cost < threshold:
               matched_tracks.append(track_idx)
               matched_detections.append(detection_idx)
           else:
               unmatched_tracks.append(track_idx)
    
    4. Update Phase:
       For each matched track:
           track.update(detection_bbox)
           track.hit_streak += 1
           track.time_since_update = 0
       
       For each unmatched track:
           track.time_since_update += 1
           track.hit_streak = 0
    
    5. Creation Phase:
       For each unmatched detection:
           new_track = Track(detection, new_id++)
           tracks.append(new_track)
    
    6. Deletion Phase:
       Remove tracks where:
           time_since_update > max_age
    
    7. Output Phase:
       Return tracks where:
           state == Confirmed OR hit_streak >= min_hits
```

**Track Lifecycle States**:

1. **Tentative** (Initial State):
   - Just created from unmatched detection
   - Not yet displayed to user
   - Transitions to Confirmed after `min_hits` consecutive detections

2. **Confirmed**:
   - Reliable track with consistent detections
   - Displayed in output
   - Can tolerate brief occlusions

3. **Deleted**:
   - No detection for `max_age` frames
   - Removed from tracker

---

## Motion Prediction

### Kalman Filter Implementation

**State Space Model**:

State Vector (8D):
```
x = [px, py, w, h, vx, vy, vw, vh]ᵀ

px, py: Center position (pixels)
w, h: Width and height (pixels)
vx, vy: Velocity in x, y (pixels/frame)
vw, vh: Rate of change in w, h (pixels/frame)
```

Measurement Vector (4D):
```
z = [px, py, w, h]ᵀ
```

**State Transition Matrix (F)**: Constant velocity model
```
F = [ 1  0  0  0  1  0  0  0 ]
    [ 0  1  0  0  0  1  0  0 ]
    [ 0  0  1  0  0  0  1  0 ]
    [ 0  0  0  1  0  0  0  1 ]
    [ 0  0  0  0  1  0  0  0 ]
    [ 0  0  0  0  0  1  0  0 ]
    [ 0  0  0  0  0  0  1  0 ]
    [ 0  0  0  0  0  0  0  1 ]

Interpretation: 
    px(t+1) = px(t) + vx(t)
    vx(t+1) = vx(t)  [constant velocity]
```

**Measurement Matrix (H)**:
```
H = [ 1  0  0  0  0  0  0  0 ]
    [ 0  1  0  0  0  0  0  0 ]
    [ 0  0  1  0  0  0  0  0 ]
    [ 0  0  0  1  0  0  0  0 ]

Only position and size are measured, not velocities
```

**Prediction Equations**:
```
x̂(t+1|t) = F × x̂(t|t)
P(t+1|t) = F × P(t|t) × Fᵀ + Q

x̂: State estimate
P: Error covariance
Q: Process noise covariance
```

**Update Equations** (when detection available):
```
y = z - H × x̂(t+1|t)           [Innovation]
S = H × P(t+1|t) × Hᵀ + R       [Innovation covariance]
K = P(t+1|t) × Hᵀ × S⁻¹        [Kalman gain]

x̂(t+1|t+1) = x̂(t+1|t) + K × y  [Updated state]
P(t+1|t+1) = (I - K×H) × P(t+1|t) [Updated covariance]

R: Measurement noise covariance
```

**Noise Parameters**:
```cpp
// Process noise: uncertainty in motion model
cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-2));

// Measurement noise: uncertainty in detections
cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-1));
```

---

## Data Association

### IoU (Intersection over Union)

**Definition**:
```
IoU(A, B) = Area(A ∩ B) / Area(A ∪ B)

Where:
    A ∩ B = Intersection area
    A ∪ B = Union area = Area(A) + Area(B) - Area(A ∩ B)
```

**Implementation**:
```cpp
float calculateIoU(const cv::Rect& box1, const cv::Rect& box2) {
    // Find intersection rectangle
    int x1 = max(box1.x, box2.x);
    int y1 = max(box1.y, box2.y);
    int x2 = min(box1.x + box1.width, box2.x + box2.width);
    int y2 = min(box1.y + box1.height, box2.y + box2.height);
    
    int intersectionArea = max(0, x2 - x1) * max(0, y2 - y1);
    int unionArea = box1.area() + box2.area() - intersectionArea;
    
    return float(intersectionArea) / unionArea;
}
```

**Cost Matrix Construction**:
```
Cost(track_i, detection_j) = 1 - IoU(track_i.predicted_bbox, detection_j.bbox)

Lower cost = better match
Cost ∈ [0, 1]
```

### Hungarian Algorithm

**Problem**: Assign detections to tracks to minimize total cost

**Formulation**:
```
Given:
    n tracks, m detections
    Cost matrix C[n×m]

Find:
    Assignment vector A[n] where A[i] = j means track i → detection j
    
Minimize:
    Total Cost = Σ C[i, A[i]] for all i
    
Constraints:
    Each track assigned to at most one detection
    Each detection assigned to at most one track
```

**Algorithm Steps** (simplified):

1. **Row Reduction**:
   ```
   For each row i:
       min_val = min(C[i, :])
       C[i, :] -= min_val
   ```

2. **Column Reduction**:
   ```
   For each column j:
       min_val = min(C[:, j])
       C[:, j] -= min_val
   ```

3. **Greedy Assignment**:
   ```
   For each row with single zero:
       Assign to that column
       Mark row and column as covered
   ```

4. **Assignment Filtering**:
   ```
   For each assignment (i, j):
       if original_cost[i, j] > threshold:
           Reject assignment
           Mark track i as unmatched
           Mark detection j as unmatched
   ```

**Complexity**: O(n³) for n×n matrix

---

## Performance Analysis

### Computational Complexity

**Per Frame**:
```
1. Detection: O(1) - fixed network forward pass
   Time: ~10-50ms (CPU), ~5-10ms (GPU)

2. Prediction: O(T) - T existing tracks
   Time: ~0.1ms per track

3. Cost Matrix: O(T × D) - T tracks, D detections
   Time: ~0.01ms per track-detection pair

4. Hungarian: O((T+D)³)
   Time: ~1ms for 100 tracks + 100 detections

5. Update: O(T + D)
   Time: ~0.1ms per track

Total per frame: ~10-50ms (CPU), dominated by detection
```

**Frame Rate Estimation**:
```
FPS = 1 / (detection_time + tracking_time + visualization_time)

Typical:
- CPU (i7): 20-40 FPS
- GPU (GTX 1080): 100-200 FPS
```

### Memory Usage

**Per Track**:
```
- State vector: 8 × 4 bytes = 32 bytes
- Covariance matrix: 8×8 × 4 bytes = 256 bytes
- Trajectory: 30 points × 8 bytes = 240 bytes
- Metadata: ~100 bytes
Total: ~630 bytes per track
```

**Total Memory**:
```
- YOLO model: ~23MB (tiny), ~250MB (full)
- Frame buffer: H × W × 3 bytes
- 1000 tracks: ~630KB
Total: ~50-300MB
```

### Accuracy Metrics

**MOTA (Multiple Object Tracking Accuracy)**:
```
MOTA = 1 - (FN + FP + IDSW) / GT

FN: False Negatives (missed detections)
FP: False Positives (false detections)
IDSW: ID Switches (track ID changes)
GT: Total ground truth objects
```

**IDF1 (ID F1 Score)**:
```
IDF1 = 2 × IDTP / (2 × IDTP + IDFP + IDFN)

IDTP: Correct ID predictions
IDFP: False positive IDs
IDFN: False negative IDs
```

---

## Advanced Optimizations

### 1. Multi-Threading
```cpp
// Process detection and prediction in parallel
std::thread detectionThread([&]() {
    detections = detector.detect(frame);
});

std::thread predictionThread([&]() {
    for (auto& track : tracks) {
        track->predict();
    }
});

detectionThread.join();
predictionThread.join();
```

### 2. Batch Processing
```cpp
// Process multiple frames in batch for GPU efficiency
std::vector<cv::Mat> frameBatch;
for (int i = 0; i < batchSize; ++i) {
    frameBatch.push_back(frames[i]);
}
std::vector<std::vector<Detection>> batchDetections = 
    detector.detectBatch(frameBatch);
```

### 3. Adaptive Thresholding
```cpp
// Adjust confidence threshold based on scene complexity
float adaptiveThreshold = baseThreshold;
if (detectionCount > maxDetections) {
    adaptiveThreshold += 0.1;  // Stricter
} else if (detectionCount < minDetections) {
    adaptiveThreshold -= 0.1;  // More lenient
}
```

---

## Mathematical Foundations

### Kalman Filter Derivation

**Bayesian Filtering Framework**:
```
Goal: Estimate p(x_t | z_1:t)

Prediction: p(x_t | z_1:t-1) = ∫ p(x_t | x_t-1) p(x_t-1 | z_1:t-1) dx_t-1
Update: p(x_t | z_1:t) ∝ p(z_t | x_t) p(x_t | z_1:t-1)
```

**Gaussian Assumption**:
```
p(x_t | z_1:t) = N(x̂_t, P_t)

Prediction and update reduce to matrix operations
```

### IoU Gradient

For optimization-based tracking:
```
∂IoU/∂x = [∂IoU/∂x₁, ∂IoU/∂y₁, ∂IoU/∂x₂, ∂IoU/∂y₂]

Can be used for gradient descent instead of Hungarian algorithm
```

---

## References

1. Bewley, A., et al. (2016). "Simple Online and Realtime Tracking"
2. Redmon, J., et al. (2018). "YOLOv3: An Incremental Improvement"
3. Kalman, R. E. (1960). "A New Approach to Linear Filtering"
4. Kuhn, H. W. (1955). "The Hungarian Method"

---

## Appendix: Parameter Tuning Guide

| Parameter | Effect | Recommended Range | Trade-off |
|-----------|--------|-------------------|-----------|
| confidence_threshold | Detection sensitivity | 0.3 - 0.7 | Lower = more detections but more false positives |
| nms_threshold | Detection overlap | 0.3 - 0.5 | Lower = fewer overlapping boxes |
| max_iou_distance | Track-detection matching | 0.5 - 0.9 | Higher = more lenient matching |
| max_age | Track persistence | 10 - 50 frames | Higher = tracks survive longer occlusions |
| min_hits | Track confirmation | 1 - 5 frames | Higher = fewer false tracks |
| process_noise | Motion uncertainty | 1e-4 - 1e-1 | Higher = more responsive to changes |
| measurement_noise | Detection uncertainty | 1e-2 - 1e0 | Higher = less trust in detections |

