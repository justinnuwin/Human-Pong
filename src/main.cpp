#include <iostream>
#include <chrono>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BackgroundSub.hpp"
#include "PoseEstimation.hpp"

#define DFL_MODEL "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb"

int main(int argc, char *argv[]) {
   cv::VideoCapture cam;

   cam.open(2);

   if (!cam.isOpened()) {
        std::cerr << "Error: Unable to open camera\n";
        return -1;
   }

   cv::Mat frame;
   cam.read(frame);

   cv::Mat bg = cv::imread("./bg.jpg");
   cv::resize(bg, bg, frame.size(), 0.0, 0.0, cv::INTER_CUBIC);

   BackgroundSub bSub(bg, "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb");
   PoseEstimation pose_estimator("../resources/models/openpose-mobilenet.pb", 0.2, 128, 128);

   for (;;) {
      auto start = std::chrono::high_resolution_clock::now();
      cam.read(frame);

      if(frame.empty()) {
         std::cerr << "Error: Unable to get image from camera\n";
         return -1;
      }

      PoseEstimation::PosePoints pose = pose_estimator.estimate(frame);
      for (int i = 0; i < pose.size(); ++i) {
          std::cout << pose[i] << std::endl;
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
