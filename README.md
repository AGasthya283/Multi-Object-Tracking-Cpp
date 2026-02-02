# Multi-Object Tracking (MOT) System in C++

A complete end-to-end Multi-Object Tracking application using C++ with YOLO object detection and SORT-based tracking algorithm.

## Overview

This project implements a real-time multi-object tracking system that:
- Detects objects in video frames using YOLOv4-tiny
- Tracks detected objects across frames using Kalman filtering
- Assigns unique IDs to objects and maintains trajectories
- Handles object occlusions and re-identification
- Outputs annotated video with tracking visualizations

## Features

- **Object Detection**: YOLOv4-tiny model for fast and accurate detection
- **Kalman Filter**: Motion prediction for robust tracking
- **Hungarian Algorithm**: Optimal assignment between detections and tracks
- **Track Management**: Handles track creation, update, and deletion
- **Visual Output**: Bounding boxes, IDs, trajectories, and statistics
- **Performance Metrics**: Real-time FPS counter and track count

## Architecture

### Core Components

1. **YOLODetector**: Handles object detection using YOLO neural network
2. **Tracker**: Implements SORT (Simple Online and Realtime Tracking) algorithm
3. **KalmanFilter**: Predicts object motion using Kalman filtering
4. **Track**: Represents individual tracked objects with state management
5. **HungarianAlgorithm**: Solves the assignment problem for detection-track association

### System Flow

```
Video Input → Frame Extraction → YOLO Detection → 
Track Association → Kalman Prediction → Track Update → 
Visualization → Video Output
```

## Directory Structure

```
mot_project/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── Detection.h         # Detection data structure
│   ├── Track.h             # Track class definition
│   ├── KalmanFilter.h      # Kalman filter for motion prediction
│   ├── HungarianAlgorithm.h # Assignment algorithm
│   ├── YOLODetector.h      # YOLO detector interface
│   └── Tracker.h           # Multi-object tracker
├── src/                    # Source files
│   ├── main.cpp            # Main application
│   ├── YOLODetector.cpp    # YOLO implementation
│   ├── Tracker.cpp         # Tracking logic
│   ├── Track.cpp           # Track implementation
│   ├── KalmanFilter.cpp    # Kalman filter implementation
│   └── HungarianAlgorithm.cpp # Assignment algorithm
├── models/                 # Model files (created after download)
│   ├── yolov4-tiny.weights
│   ├── yolov4-tiny.cfg
│   └── coco.names
├── data/                   # Input videos
├── build/                  # Build output
└── scripts/                # Helper scripts
    ├── download_models.sh  # Download YOLO models
    ├── build.sh            # Build the project
    └── run.sh              # Run the tracker
```

## Prerequisites

### System Requirements
- Linux (Ubuntu 18.04+ recommended)
- C++17 compiler (GCC 7+ or Clang 5+)
- CMake 3.10+
- At least 2GB RAM
- GPU (optional, for faster inference)

### Dependencies
- OpenCV 4.0+ with DNN module
- wget (for downloading models)

## Installation

### 1. Install OpenCV

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake git
sudo apt-get install -y libopencv-dev

# Or build from source for latest version
# See: https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html
```

### 2. Clone/Download the Project

```bash
cd mot_project
```

### 3. Download YOLO Models

```bash
./scripts/download_models.sh
```

This downloads:
- YOLOv4-tiny weights (~23MB)
- YOLOv4-tiny config file
- COCO class names (80 classes)

### 4. Build the Project

```bash
./scripts/build.sh
```

This will:
- Create a `build` directory
- Run CMake configuration
- Compile all source files
- Generate the `mot_tracker` executable

## Usage

### Basic Usage

```bash
./scripts/run.sh --video input.mp4
```

### Advanced Usage

```bash
./scripts/run.sh \
    --video data/test_video.mp4 \
    --weights models/yolov4-tiny.weights \
    --config models/yolov4-tiny.cfg \
    --names models/coco.names \
    --output results/output.avi
```

### Command Line Options

- `-v, --video`: Input video file path
- `-w, --weights`: YOLO weights file
- `-c, --config`: YOLO config file
- `-n, --names`: Class names file
- `-o, --output`: Output video file path
- `-h, --help`: Show help message

### Direct Execution

```bash
./build/mot_tracker <video> [weights] [config] [classes] [output]
```

Example:
```bash
./build/mot_tracker input.mp4 models/yolov4-tiny.weights \
    models/yolov4-tiny.cfg models/coco.names output.avi
```

## Configuration

### Tracker Parameters

Edit `src/main.cpp` to adjust tracker parameters:

```cpp
// In main() function
Tracker tracker(
    0.7f,  // maxIoUDistance: IoU threshold for matching (0.0-1.0)
    30,    // maxAge: frames to keep unmatched tracks
    3      // minHits: hits before track is confirmed
);
```

### Detection Parameters

Adjust detection thresholds in `src/main.cpp`:

```cpp
std::vector<Detection> detections = detector.detect(
    frame, 
    0.5f,  // confidence threshold
    0.4f   // NMS threshold
);
```

### YOLO Input Size

Modify in `YOLODetector.cpp`:

```cpp
inputSize(416, 416)  // Change to 320, 608, etc.
```

Smaller size = faster, but less accurate
Larger size = slower, but more accurate

## Algorithm Details

### SORT Algorithm

The tracker implements the SORT (Simple Online and Realtime Tracking) algorithm:

1. **Detection**: YOLO detects objects in each frame
2. **Prediction**: Kalman filter predicts new positions for existing tracks
3. **Association**: Hungarian algorithm matches detections to tracks
4. **Update**: Matched tracks are updated with new detections
5. **Management**: Create new tracks for unmatched detections, delete old tracks

### Kalman Filter

State vector: [x, y, w, h, vx, vy, vw, vh]
- x, y: center coordinates
- w, h: width and height
- vx, vy, vw, vh: velocities

The filter predicts object position in the next frame based on constant velocity model.

### Track Lifecycle

```
New Detection → Tentative → Confirmed → Active → Lost → Deleted
                  (3 hits)              (>30 frames)
```

## Performance Optimization

### CPU Optimization
- Use smaller YOLO input size (320x320)
- Reduce video resolution
- Process every Nth frame
- Compile with `-O3` optimization

### GPU Acceleration
Enable CUDA in OpenCV build:
```bash
cmake -D WITH_CUDA=ON -D OPENCV_DNN_CUDA=ON ..
```

Then in code:
```cpp
net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
```

## Output Format

The output video includes:
- Colored bounding boxes (unique color per track ID)
- Track ID and object class labels
- Object trajectories (path history)
- Real-time statistics (frame count, FPS, active tracks)

## Testing

### Test Videos
Download sample videos:
```bash
# Example: pedestrian tracking
wget https://www.sample-videos.com/video321/mp4/720/big_buck_bunny_720p_1mb.mp4 -O data/test.mp4
```

### Expected Performance
- **YOLOv4-tiny**: ~50-100 FPS (CPU), ~200-500 FPS (GPU)
- **Full pipeline**: ~20-40 FPS (CPU), ~100-200 FPS (GPU)
- Varies based on:
  - Hardware specifications
  - Video resolution
  - Number of objects
  - Detection threshold

## Troubleshooting

### Common Issues

1. **OpenCV not found**
   ```
   Error: Could not find OpenCV
   ```
   Solution: Install OpenCV dev packages or set OpenCV_DIR

2. **Model files not found**
   ```
   Error: Could not load YOLO model
   ```
   Solution: Run `./scripts/download_models.sh`

3. **Video codec issues**
   ```
   Error: Could not open video file
   ```
   Solution: Install ffmpeg and opencv-contrib packages

4. **Low FPS**
   - Use smaller YOLO input size
   - Reduce video resolution
   - Enable GPU acceleration
   - Use YOLOv4-tiny instead of YOLOv4

## Extending the Project

### Add New Detection Models

Replace YOLODetector with other detectors:
- Faster R-CNN
- SSD
- EfficientDet
- Custom trained models

### Advanced Tracking

Enhance tracking with:
- Deep SORT (appearance features)
- Re-identification networks
- Multi-camera tracking
- 3D tracking with depth cameras

### Additional Features

- Track statistics export (CSV)
- Heatmap visualization
- Zone-based counting
- Speed estimation
- Action recognition

## Performance Metrics

Evaluate tracking quality using:
- MOTA (Multiple Object Tracking Accuracy)
- MOTP (Multiple Object Tracking Precision)
- IDF1 (ID F1 Score)
- FP, FN (False Positives/Negatives)

## References

- YOLO: https://pjreddie.com/darknet/yolo/
- SORT: https://arxiv.org/abs/1602.00763
- Kalman Filter: https://en.wikipedia.org/wiki/Kalman_filter
- Hungarian Algorithm: https://en.wikipedia.org/wiki/Hungarian_algorithm

## License

This project is provided as-is for educational purposes.

## Contributing

Contributions are welcome! Areas for improvement:
- Performance optimization
- Additional detection models
- Advanced tracking algorithms
- Better visualization
- Unit tests
- Documentation

## Author

Multi-Object Tracking System
Built with C++ and OpenCV

## Version History

- v1.0.0 (2026-02-02): Initial release
  - YOLOv4-tiny detection
  - SORT tracking algorithm
  - Kalman filter motion prediction
  - Video processing pipeline
