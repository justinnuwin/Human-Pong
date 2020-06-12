#ifndef GAME_H_
#define GAME_H_

#include <opencv2/core.hpp>
#include "PoseEstimation.hpp"

int lpf_paddle_position(int new_pos);
void draw_paddle(cv::Mat &image, enum PoseEstimation::BodyParts where, const PoseEstimation::PosePoints pose,
                cv::Scalar color=cv::Scalar(255, 255, 255), int paddle_radius=60, int stroke=8, int col=30);

#endif 
