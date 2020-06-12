#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/dnn.hpp>

class BackgroundMask
{
public:
   BackgroundMask(cv::String model);
   cv::Mat GetBackground(const cv::Mat& frame);

private:
   cv::dnn::Net bodyPix;
};
