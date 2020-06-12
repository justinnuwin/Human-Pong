#include <iostream>
#include <chrono>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BackgroundSub.hpp"
#include "PoseEstimation.hpp"

#define DFL_MODEL "../resources/models/bodypix_mobilenet_float_050_model-stride8.pb"

int lpf_paddle_position(int new_pos) {
    static float running_average = 100;
    const static float alpha = 0.3;
    running_average = (1. - alpha) * (float)new_pos + alpha * running_average;
    return (int)running_average;
}

void draw_paddle(cv::Mat &image, enum PoseEstimation::BodyParts where, const PoseEstimation::PosePoints pose,
                cv::Scalar color=cv::Scalar(255, 255, 255), int paddle_radius=60, int stroke=8, int col=30) {
    for (const struct PoseEstimation::PosePoint &point : pose) {
        if (point.part == where) {           
            int position = lpf_paddle_position(point.location.y);
            cv::Point from = cv::Point(col, position - paddle_radius);
            cv::Point to = cv::Point(col, position + paddle_radius);
            cv::line(image, from, to, color, stroke);
            cv::circle(image, point.location, 100, cv::Scalar(128, 128, 128), 2);
        }
    }
}

int main(int argc, char *argv[]) {
   cv::VideoCapture cam;

   cam.open(0);

   if (!cam.isOpened()) {
        std::cerr << "Error: Unable to open camera\n";
        return -1;
   }

   cv::Mat frame;
   cam.read(frame);

   cv::Mat bg = cv::imread("./bg.jpg");
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
