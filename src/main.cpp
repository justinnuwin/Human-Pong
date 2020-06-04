#include <iostream>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include "BackgroundMask.hpp"

int main(int argc, char *argv[]) {
   cv::VideoCapture cam;
   BackgroundMask bMask;

   cam.open(4);

   if (!cam.isOpened()) {
        std::cerr << "Error: Unable to open camera\n";
        return -1;
   }

   for (;;) {
      cv::Mat frame;
      cam.read(frame);

      if(frame.empty()) {
         std::cerr << "Error: Unable to get image from camera\n";
         return -1;
      }

      cv::Mat mask = bMask.GetBackground(frame);
      imshow("Mask", frame);
      if(cv::waitKey(1) >= 0) {
         break;
      }
   }

   return 0;
}
