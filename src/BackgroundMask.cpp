#include <iostream>

#include "BackgroundMask.hpp"

#define MODEL_FILE "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb"
#define THRESHOLD 0.7

BackgroundMask::BackgroundMask() {
   bodyPix = cv::dnn::readNetFromTensorflow(MODEL_FILE);
}

cv::Mat BackgroundMask::GetBackground(const cv::Mat& frame) {
   cv::Mat frameBlob;
   cv::dnn::blobFromImage(frame, frameBlob, 1.0, cv::Size(), cv::Scalar(), true);

   // normalize image for mobilenet
   frameBlob = (frameBlob/127.5) - 1;

   bodyPix.setInput(frameBlob);
   cv::Mat out = bodyPix.forward("float_segments/conv");

   std::vector<cv::Mat> outputImgs;
   cv::dnn::imagesFromBlob(out, outputImgs);
   cv::Mat outFrame = outputImgs[0];

   // sigmoid function on output
   cv::exp(-outFrame, outFrame);
   outFrame = 1.0 / (1.0 + outFrame);
   for(int x=0;x<outFrame.cols;x++) {
      for(int y=0;y<outFrame.rows;y++) {
         if(outFrame.at<float>(y,x) > THRESHOLD) {
            outFrame.at<float>(y,x) = 255;
         } else {
            outFrame.at<float>(y,x) = 0;
         }
      }
   }

   outFrame.convertTo(outFrame, CV_8U);
   return outFrame;
}
