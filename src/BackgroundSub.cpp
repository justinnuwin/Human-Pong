#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "BackgroundSub.hpp"


BackgroundSub::BackgroundSub(const cv::Mat background) {
   this->background = background;
   this->bMask = BackgroundMask();
}

cv::Mat BackgroundSub::Sub(const cv::Mat& frame) {
   cv::Mat mask = bMask.GetBackground(frame);

   cv::resize(mask, mask, frame.size(), cv::INTER_CUBIC);
   cv::waitKey(20);

   cv::Mat out = cv::Mat::zeros(frame.size(), frame.type());
   cv::bitwise_or(out, frame, out, mask);
   cv::bitwise_not(mask, mask);
   cv::bitwise_or(out, background, out, mask);
   return out;
}
