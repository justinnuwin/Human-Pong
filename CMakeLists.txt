cmake_minimum_required(VERSION 3.4)
project(pong)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)

find_package( OpenCV 4.4.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(SOURCES "src/BackgroundMask.cpp" "src/BackgroundSub.cpp" "src/PoseEstimation.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable("pong" "src/main.cpp" ${SOURCES} ${HEADERS})
target_link_libraries("pong" ${OpenCV_LIBS})

add_executable("BackgroundBench" "src/BackgroundBench.cpp" ${SOURCES} ${HEADERS})
target_link_libraries("BackgroundBench" ${OpenCV_LIBS})

add_executable("PoseEstimationTest" "src/TestPoseEstimation.cpp" ${SOURCES} ${HEADERS})
target_link_libraries("PoseEstimationTest" ${OpenCV_LIBS})
