#pragma once

#include "BackgroundSub.hpp"
#include "PoseEstimation.hpp"

class ProcessImg {
public:
   ProcessImg();
   ~ProcessImg();
   std::vector<uchar> Process(std::vector<uchar> left, std::vector<uchar> right);

private:
   BackgroundSub *bSub;
   PoseEstimation pose_estimator;
};
