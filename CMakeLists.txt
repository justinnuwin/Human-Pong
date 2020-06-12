cmake_minimum_required(VERSION 3.4)
project(pong)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)

find_package( OpenCV 4.3.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMERMM REQUIRED IMPORTED_TARGET gstreamermm-1.0)

set(SOURCES "src/BackgroundMask.cpp" "src/BackgroundSub.cpp" "src/PoseEstimation.cpp" "src/ProcessImg.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable("PongTest" "src/PongTest.cpp" ${SOURCES} ${HEADERS})
target_link_libraries("PongTest" ${OpenCV_LIBS})

add_executable("BackgroundBench" "src/BackgroundBench.cpp" ${SOURCES} ${HEADERS})
target_link_libraries("BackgroundBench" ${OpenCV_LIBS})

set(NET_SOURCES "src/gethostbyname.cpp" "src/networks.cpp" "src/pong_packets.cpp")

add_executable("client" "src/pong_client.cpp" ${NET_SOURCES} ${HEADERS})
target_link_libraries("client" PUBLIC PkgConfig::GSTREAMERMM)

add_executable("server" "src/pong_server.cpp" ${SOURCES} ${NET_SOURCES} ${HEADERS})
target_link_libraries("server" PUBLIC PkgConfig::GSTREAMERMM ${OpenCV_LIBS})

add_executable("PoseEstimationTest" "src/TestPoseEstimation.cpp" ${SOURCES} ${HEADERS})
target_link_libraries("PoseEstimationTest" ${OpenCV_LIBS})
