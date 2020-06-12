#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "ProcessImg.h"

ProcessImg::ProcessImg() {
   cv::Mat bg = cv::imread("./bg.jpg");
   bSub = new BackgroundSub(bg, "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb");
}

std::vector<uchar> ProcessImg::Process(std::vector<uchar> left, std::vector<uchar> right) {
   std::vector<uchar> out = std::vector<uchar>();

   // We don't have a frame from both clients yet
   if(left.size() == 0 || right.size() == 0) {
      return out;
   }

   cv::Mat leftMat = cv::imdecode(left, cv::IMREAD_COLOR);
   cv::Mat rightMat = cv::imdecode(right, cv::IMREAD_COLOR);


   if (leftMat.size() != rightMat.size()) {
      cv::resize(rightMat, rightMat, leftMat.size(), 0.0, 0.0, cv::INTER_CUBIC);
   }

   //cv::Mat newImg = bSub->Sub(imgMat);

   cv::Mat finalMat;
   cv::hconcat(leftMat, rightMat, finalMat);
   cv::imencode(".jpg", finalMat, out);
   return out;
}
