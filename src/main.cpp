#include <iostream>
#include <chrono>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BackgroundSub.hpp"

int main(int argc, char *argv[]) {
   cv::VideoCapture cam;

   cam.open(2);

   if (!cam.isOpened()) {
        std::cerr << "Error: Unable to open camera\n";
        return -1;
   }

   cv::Mat frame;
   cam.read(frame);

   cv::Mat img = cv::imread("./bg.jpg");
   cv::resize(img, img, frame.size(), 0.0, 0.0, cv::INTER_CUBIC);

   BackgroundSub bSub = BackgroundSub(img);

   for (;;) {
      auto start = std::chrono::high_resolution_clock::now();
      cam.read(frame);

      if(frame.empty()) {
         std::cerr << "Error: Unable to get image from camera\n";
         return -1;
      }

      cv::Mat mask = bSub.Sub(frame);
      imshow("Background", mask);
      if(cv::waitKey(1) >= 0) {
         break;
      }

      auto end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      std::cout << "Game Loop: " << duration << "ms" << std::endl;
   }

   return 0;
}
