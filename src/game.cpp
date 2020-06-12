#include "game.hpp"

#include <opencv2/imgproc.hpp>

int lpf_paddle_position(int new_pos) {
    static float running_average = 100;
    const static float alpha = 0.3;
    running_average = (1. - alpha) * (float)new_pos + alpha * running_average;
    return (int)running_average;
}

void draw_paddle(cv::Mat &image, enum PoseEstimation::BodyParts where, const PoseEstimation::PosePoints pose,
                cv::Scalar color, int paddle_radius, int stroke, int col) {
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

