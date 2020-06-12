#include <iostream>
#include <chrono>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "BackgroundSub.hpp"
#include "PoseEstimation.hpp"

int main(int argc, char *argv[]) {
    cv::Mat image = cv::imread("../pose_test.jpg");
    PoseEstimation pose_estimator("../resources/models/openpose-mobilenet.pb", 0.2, 368, 368);
    PoseEstimation::PosePoints pose = pose_estimator.estimate(image);
    pose_estimator.draw_pose(image, pose);
    imshow("Image", image);
    cv::waitKey(0);

    return 0;
}
