#!/bin/bash

# Script to download YOLOv4-tiny model and COCO class names

echo "Downloading YOLOv4-tiny model files..."

# Create models directory
mkdir -p models
cd models

# Download YOLOv4-tiny weights
if [ ! -f "yolov4-tiny.weights" ]; then
    echo "Downloading YOLOv4-tiny weights..."
    wget https://github.com/AlexeyAB/darknet/releases/download/yolov4/yolov4-tiny.weights
else
    echo "yolov4-tiny.weights already exists"
fi

# Download YOLOv4-tiny config
if [ ! -f "yolov4-tiny.cfg" ]; then
    echo "Downloading YOLOv4-tiny config..."
    wget https://raw.githubusercontent.com/AlexeyAB/darknet/master/cfg/yolov4-tiny.cfg
else
    echo "yolov4-tiny.cfg already exists"
fi

# Download COCO class names
if [ ! -f "coco.names" ]; then
    echo "Downloading COCO class names..."
    wget https://raw.githubusercontent.com/AlexeyAB/darknet/master/data/coco.names
else
    echo "coco.names already exists"
fi

cd ..

echo "Download complete!"
echo "Models saved in ./models/ directory"
