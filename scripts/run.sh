#!/bin/bash

# Run script for Multi-Object Tracking

# Default parameters
VIDEO="input.mp4"
WEIGHTS="models/yolov4-tiny.weights"
CONFIG="models/yolov4-tiny.cfg"
CLASSES="models/coco.names"
OUTPUT="output.avi"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--video)
            VIDEO="$2"
            shift 2
            ;;
        -w|--weights)
            WEIGHTS="$2"
            shift 2
            ;;
        -c|--config)
            CONFIG="$2"
            shift 2
            ;;
        -n|--names)
            CLASSES="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -v, --video    Input video file (default: input.mp4)"
            echo "  -w, --weights  YOLO weights file (default: models/yolov4-tiny.weights)"
            echo "  -c, --config   YOLO config file (default: models/yolov4-tiny.cfg)"
            echo "  -n, --names    Class names file (default: models/coco.names)"
            echo "  -o, --output   Output video file (default: output.avi)"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check if executable exists
if [ ! -f "build/mot_tracker" ]; then
    echo "Error: mot_tracker executable not found!"
    echo "Please run ./scripts/build.sh first"
    exit 1
fi

# Check if video file exists
if [ ! -f "$VIDEO" ]; then
    echo "Error: Video file not found: $VIDEO"
    exit 1
fi

# Check if model files exist
if [ ! -f "$WEIGHTS" ] || [ ! -f "$CONFIG" ] || [ ! -f "$CLASSES" ]; then
    echo "Error: Model files not found!"
    echo "Please run ./scripts/download_models.sh first"
    exit 1
fi

# Run the tracker
echo "Running Multi-Object Tracker..."
./build/mot_tracker "$VIDEO" "$WEIGHTS" "$CONFIG" "$CLASSES" "$OUTPUT"
