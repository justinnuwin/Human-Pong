#pragma once

#include <opencv2/core/mat.hpp>

#include "BackgroundMask.hpp"

class BackgroundSub
{
public:
   BackgroundSub(cv::Mat background);
   cv::Mat Sub(const cv::Mat& frame);

private:
   cv::Mat background;
   BackgroundMask bMask;
};
