#include <iostream>
#include <chrono>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BackgroundSub.hpp"
#include "PoseEstimation.hpp"
#include "game.hpp"

#define DFL_MODEL "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb"


int main(int argc, char *argv[]) {
   cv::VideoCapture cam;

   cam.open(0);

   if (!cam.isOpened()) {
        std::cerr << "Error: Unable to open camera\n";
        return -1;
   }

   cv::Mat frame;
   cam.read(frame);

   cv::Mat bg = cv::imread("../resources/bg.jpg");
   cv::resize(bg, bg, frame.size(), 0.0, 0.0, cv::INTER_CUBIC);

   BackgroundSub bSub(bg, "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb");
   PoseEstimation pose_estimator("../resources/models/openpose-mobilenet.pb", 0.25, 128, 128);

   for (;;) {
      auto start = std::chrono::high_resolution_clock::now();
      cam.read(frame);

      if(frame.empty()) {
         std::cerr << "Error: Unable to get image from camera\n";
         return -1;
      }

      PoseEstimation::PosePoints pose = pose_estimator.estimate(frame);
      cv::Mat mask = bSub.Sub(frame);
      // pose_estimator.draw_pose(mask, pose);
      draw_paddle(mask, PoseEstimation::e_Nose, pose);
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

