#include <iostream>
#include <chrono>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include "BackgroundMask.hpp"

int main(int argc, char *argv[]) {
   cv::VideoCapture cam;
   BackgroundMask bMask;

   cam.open(2);

   if (!cam.isOpened()) {
        std::cerr << "Error: Unable to open camera\n";
        return -1;
   }

   for (;;) {
      auto start = std::chrono::high_resolution_clock::now();
      cv::Mat frame;
      cam.read(frame);

      if(frame.empty()) {
         std::cerr << "Error: Unable to get image from camera\n";
         return -1;
      }

      cv::Mat mask = bMask.GetBackground(frame);
      imshow("Mask", mask);
      if(cv::waitKey(1) >= 0) {
         break;
      }

      auto end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      std::cout << "Game Loop: " << duration << "ms" << std::endl;
   }

   return 0;
}
