cmake_minimum_required(VERSION 3.10)
project(MultiObjectTracking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable
add_executable(mot_tracker
    src/main.cpp
    src/YOLODetector.cpp
    src/Tracker.cpp
    src/Track.cpp
    src/KalmanFilter.cpp
    src/HungarianAlgorithm.cpp
)

# Link libraries
target_link_libraries(mot_tracker ${OpenCV_LIBS})

# Set output directory
set_target_properties(mot_tracker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)