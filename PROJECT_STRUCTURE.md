# Multi-Object Tracking Project Structure

## File Organization

```
mot_project/
│
├── CMakeLists.txt                  # CMake build configuration
├── README.md                       # Main documentation
├── QUICKSTART.md                   # Quick setup guide
├── TECHNICAL_DETAILS.md            # Algorithm deep-dive
├── config.txt                      # Configuration parameters
│
├── include/                        # Header files
│   ├── Detection.h                 # Detection data structure
│   ├── Track.h                     # Track object definition
│   ├── KalmanFilter.h              # Motion prediction
│   ├── HungarianAlgorithm.h        # Assignment solver
│   ├── YOLODetector.h              # Object detector interface
│   └── Tracker.h                   # Multi-object tracker
│
├── src/                            # Implementation files
│   ├── main.cpp                    # Application entry point
│   ├── YOLODetector.cpp            # YOLO detector implementation
│   ├── Tracker.cpp                 # Tracking algorithm
│   ├── Track.cpp                   # Track management
│   ├── KalmanFilter.cpp            # Kalman filter math
│   └── HungarianAlgorithm.cpp      # Assignment algorithm
│
├── scripts/                        # Utility scripts
│   ├── download_models.sh          # Download YOLO models
│   ├── build.sh                    # Build the project
│   ├── run.sh                      # Run the tracker
│   └── test_installation.sh        # Verify setup
│
├── models/                         # Model files (after download)
│   ├── yolov4-tiny.weights         # YOLO weights (~23MB)
│   ├── yolov4-tiny.cfg             # YOLO architecture config
│   └── coco.names                  # Class labels (80 classes)
│
├── data/                           # Input videos (user provided)
│   └── input.mp4                   # Sample input video
│
└── build/                          # Build artifacts (generated)
    ├── mot_tracker                 # Executable binary
    └── ...                         # Object files
```

## Component Relationships

```
┌─────────────────────────────────────────────────────────────────┐
│                             main.cpp                              │
│  - Video I/O                                                      │
│  - Main processing loop                                           │
│  - Visualization                                                  │
└─────────┬───────────────────────────────────────┬────────────────┘
          │                                       │
          │                                       │
          ▼                                       ▼
┌────────────────────────┐           ┌───────────────────────────┐
│    YOLODetector.cpp    │           │      Tracker.cpp          │
│  - Load YOLO model     │           │  - Track management       │
│  - Forward inference   │           │  - Data association       │
│  - NMS filtering       │           │  - SORT algorithm         │
│  Returns: Detection[]  │           │  Returns: Track[]         │
└────────────────────────┘           └─────────┬─────────────────┘
                                               │
                         ┌─────────────────────┼─────────────────────┐
                         │                     │                     │
                         ▼                     ▼                     ▼
              ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
              │  Track.cpp       │  │ KalmanFilter.cpp │  │ Hungarian.cpp    │
              │ - Track state    │  │ - Predict motion │  │ - Optimal assign │
              │ - Trajectory     │  │ - Update state   │  │ - Cost minimize  │
              │ - Lifecycle      │  │ - Covariance     │  │ - Matching       │
              └──────────────────┘  └──────────────────┘  └──────────────────┘
```

## Data Flow

```
Input Video → Frame → YOLO Detection → Detections
                                           │
                                           ▼
                                    ┌──────────────┐
                                    │  IoU Matrix  │
                                    │ (Cost Comp.) │
                                    └──────┬───────┘
                                           │
                    ┌──────────────────────┼──────────────────────┐
                    │                      │                      │
                    ▼                      ▼                      ▼
              Matched Tracks        Unmatched Tracks      Unmatched Dets
                    │                      │                      │
                    ▼                      ▼                      ▼
            Kalman Update            Mark Missed            Create New Tracks
                    │                      │                      │
                    └──────────────────────┴──────────────────────┘
                                           │
                                           ▼
                                    Active Tracks
                                           │
                                           ▼
                                    Visualization
                                           │
                                           ▼
                                     Output Video
```

## Build Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                          System Level                             │
│  • Linux OS (Ubuntu 18.04+)                                      │
│  • GCC 7+ or Clang 5+                                            │
│  • CMake 3.10+                                                   │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                         OpenCV 4.0+                               │
│  • Core module                                                    │
│  • DNN module (for YOLO)                                         │
│  • Video module (for I/O)                                        │
│  • ImgProc module (for visualization)                            │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                       MOT Application                             │
│  Built from source files using CMake                              │
└─────────────────────────────────────────────────────────────────┘
```

## Class Hierarchy

```
Detection (struct)
  └── Simple data container for YOLO output

Track (class)
  ├── KalmanFilter (composition)
  ├── Trajectory storage (deque)
  └── State management (enum TrackState)

YOLODetector (class)
  ├── cv::dnn::Net (composition)
  └── Class names (vector)

Tracker (class)
  ├── vector<Track> (composition)
  └── HungarianAlgorithm (static utility)

KalmanFilter (class)
  └── cv::KalmanFilter (composition)

HungarianAlgorithm (class)
  └── Static methods only (no instance state)
```

## Processing Pipeline Timing

```
Frame N:
├── T0: Read frame (1-2ms)
├── T1: YOLO inference (10-50ms) ← Bottleneck on CPU
├── T2: Kalman predict all tracks (0.1ms)
├── T3: Compute IoU matrix (0.5ms)
├── T4: Hungarian assignment (1ms)
├── T5: Update tracks (0.5ms)
├── T6: Create/delete tracks (0.1ms)
├── T7: Visualization (2-5ms)
└── T8: Write output frame (1-2ms)

Total: ~15-60ms per frame (16-60 FPS)
```

## Memory Layout

```
┌──────────────────────────────────────┐
│     YOLO Model Weights (~23MB)       │  ← Loaded once at startup
├──────────────────────────────────────┤
│     Frame Buffers (2-4MB each)       │  ← Reused per frame
├──────────────────────────────────────┤
│     Track Objects (~1KB each)        │  ← Dynamic, grows/shrinks
├──────────────────────────────────────┤
│     Detection Buffers (~10KB/frame)  │  ← Temporary per frame
├──────────────────────────────────────┤
│     Visualization Buffers (2-4MB)    │  ← Reused per frame
└──────────────────────────────────────┘

Total: ~50-300MB depending on video resolution and track count
```

## Algorithm Complexity

| Component | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| YOLO Detect | O(1) fixed | O(1) fixed |
| Kalman Predict | O(T) | O(T) |
| IoU Matrix | O(T×D) | O(T×D) |
| Hungarian | O(max(T,D)³) | O(T×D) |
| Track Update | O(T+D) | O(T+D) |

Where:
- T = number of tracks
- D = number of detections per frame

## Configuration Parameters Impact

```
confidence_threshold ↑
    └→ Fewer detections
    └→ Fewer false positives
    └→ More missed objects

max_iou_distance ↑
    └→ More lenient matching
    └→ Fewer ID switches
    └→ More wrong associations

max_age ↑
    └→ Tracks survive longer
    └→ Better occlusion handling
    └→ More ghost tracks

min_hits ↑
    └→ Fewer tentative tracks
    └→ Fewer false tracks
    └→ Slower track initialization
```

## Coordinate Systems

```
Image Coordinates (OpenCV):
  (0,0) ────────> x (width)
    │
    │
    │
    ▼
    y (height)

Bounding Box Representation:
  bbox = (x, y, width, height)
  where (x,y) is top-left corner

Center-based (Internal):
  center = (cx, cy)
  cx = x + width/2
  cy = y + height/2
```
