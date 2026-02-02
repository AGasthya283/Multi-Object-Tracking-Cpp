# Quick Start Guide

Get up and running with Multi-Object Tracking in 5 minutes!

## Prerequisites Check

```bash
# Check if you have required tools
g++ --version      # Should show GCC 7.0+
cmake --version    # Should show CMake 3.10+
pkg-config --modversion opencv4  # Should show OpenCV 4.0+
```

If any command fails, install the missing dependency first.

## 3-Step Setup

### Step 1: Install Dependencies (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget
sudo apt-get install -y libopencv-dev
```

### Step 2: Download Models

```bash
cd mot_project
./scripts/download_models.sh
```

Expected output:
```
Downloading YOLOv4-tiny model files...
Downloading YOLOv4-tiny weights...
Downloading YOLOv4-tiny config...
Downloading COCO class names...
Download complete!
```

### Step 3: Build the Project

```bash
./scripts/build.sh
```

Expected output:
```
Building Multi-Object Tracking System...
Running CMake...
Compiling...
Build successful!
Executable created: ./build/mot_tracker
```

## Run Your First Tracking

### Option 1: With a Test Video

```bash
# Download a sample video
wget https://www.pexels.com/download/video/3571264/ -O input.mp4

# Run tracking
./scripts/run.sh --video input.mp4 --output my_output.avi
```

### Option 2: With Webcam

Modify `src/main.cpp` line with VideoCapture:
```cpp
cv::VideoCapture cap(0);  // 0 for default webcam
```

Then rebuild and run:
```bash
./scripts/build.sh
./build/mot_tracker
```

## What You'll See

1. **Console Output**:
   ```
   === Multi-Object Tracking System ===
   Video: input.mp4
   Processing video...
   Processed 30 frames, Average FPS: 25.3
   ...
   === Processing Complete ===
   Total frames: 500
   Total unique tracks: 12
   Output saved to: output.avi
   ```

2. **Live Window**: Shows video with:
   - Colored bounding boxes around objects
   - Track IDs and class labels
   - Motion trajectories
   - Real-time FPS counter

3. **Output Video**: Saved as `output.avi` with all visualizations

## Common Use Cases

### Track Specific Objects Only

Edit detection filter in `src/main.cpp`:

```cpp
// After getting detections, filter for specific classes
std::vector<Detection> filteredDetections;
for (const auto& det : detections) {
    // Track only persons (class 0) and cars (class 2)
    if (det.classId == 0 || det.classId == 2) {
        filteredDetections.push_back(det);
    }
}
std::vector<std::shared_ptr<Track>> tracks = tracker.update(filteredDetections);
```

### Improve Performance

For faster processing:
```bash
# Use smaller input size
# Edit src/YOLODetector.cpp: inputSize(320, 320)
./scripts/build.sh
./scripts/run.sh --video input.mp4
```

### Higher Accuracy

For better detection:
```bash
# Download full YOLOv4 (larger, slower, more accurate)
cd models
wget https://github.com/AlexeyAB/darknet/releases/download/yolov4/yolov4.weights
wget https://raw.githubusercontent.com/AlexeyAB/darknet/master/cfg/yolov4.cfg
cd ..

# Run with full YOLO
./build/mot_tracker input.mp4 models/yolov4.weights models/yolov4.cfg models/coco.names output.avi
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `opencv not found` | `sudo apt-get install libopencv-dev` |
| `cannot open video` | Check file path, install `sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev` |
| Low FPS | Use smaller video resolution or YOLOv4-tiny |
| No detections | Lower confidence threshold in code (0.3 instead of 0.5) |
| Tracks lost quickly | Increase `max_age` parameter in Tracker constructor |

## Next Steps

- Read [README.md](README.md) for detailed documentation
- Experiment with different videos
- Adjust tracking parameters for your use case
- Try GPU acceleration for better performance
- Explore the code to understand the algorithms

## Support

For issues or questions:
1. Check the main README.md
2. Review the troubleshooting section
3. Examine the code comments in src/

Happy Tracking! 🎯
