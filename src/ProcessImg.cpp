#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "ProcessImg.h"
#include "game.hpp"

ProcessImg::ProcessImg() :
   pose_estimator(PoseEstimation("../resources/models/openpose-mobilenet.pb", 0.25, 128, 128))
{
   cv::Mat bg = cv::imread("../resources/bg.jpg");
   bSub = new BackgroundSub(bg, "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb");
}

ProcessImg::~ProcessImg() {
    delete bSub;
}

std::vector<uchar> ProcessImg::Process(std::vector<uchar> left, std::vector<uchar> right) {
   std::vector<uchar> out = std::vector<uchar>();

   // We don't have a frame from both clients yet
   if(left.size() == 0 || right.size() == 0) {
      return out;
   }

   cv::Mat leftMat = cv::imdecode(left, cv::IMREAD_COLOR);
   cv::Mat rightMat = cv::imdecode(right, cv::IMREAD_COLOR);

   CV_Assert(!leftMat.empty());
   CV_Assert(!rightMat.empty());

   if (leftMat.size() != rightMat.size()) {
      cv::resize(rightMat, rightMat, leftMat.size(), 0.0, 0.0, cv::INTER_CUBIC);
   }

   PoseEstimation::PosePoints left_pose = pose_estimator.estimate(leftMat);
    std::cout << "Here" << std::endl;
   PoseEstimation::PosePoints right_pose = pose_estimator.estimate(rightMat);
   leftMat = bSub->Sub(leftMat);
   rightMat = bSub->Sub(rightMat);
   draw_paddle(leftMat, PoseEstimation::e_Nose, left_pose);
   draw_paddle(rightMat, PoseEstimation::e_Nose, right_pose);

   cv::Mat finalMat;
   cv::hconcat(leftMat, rightMat, finalMat);
   cv::imencode(".jpg", finalMat, out);
   return out;
}

