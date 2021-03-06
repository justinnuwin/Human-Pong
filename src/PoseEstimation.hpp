#ifndef POSEESTIMATION_H_
#define POSEESTIMATION_H_

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <cstring>
#include <iostream>
#include <map>

class PoseEstimation {
    public:
        enum BodyParts {e_Nose, e_Neck, e_RShoulder, e_RElbow, e_RWrist, e_LShoulder, e_LElbow, 
                        e_LWrist, e_RHip, e_RKnee, e_RAnkle, e_LHip, e_LKnee, e_LAnkle, e_REye,
                        e_LEye, e_REar, e_LEar, e_Background};
        static const std::vector<enum BodyParts> bodyparts_list;
        static const std::map<enum PoseEstimation::BodyParts, std::string> bodypart_names;
        static const std::map<enum BodyParts, std::vector<enum BodyParts>> bodypart_connections;
        static const std::map <enum PoseEstimation::BodyParts, int> bodypart_layer_map;

        struct PosePoint {
            enum BodyParts part;
            cv::Point2i location;
            float confidence;

            PosePoint(enum BodyParts p, cv::Point l, float c) :
                part(p), location(l), confidence(c) {}

            std::string toString() const;

        };

        typedef std::vector<struct PosePoint> PosePoints;

    public:
        PoseEstimation(std::string graph_path, float confidence_threshold=0.3, int network_input_width=368,
                       int network_input_height=368, float mean_offset=127.5);
        PosePoints estimate(const cv::Mat &image);
        void draw_pose(cv::Mat &image, PoseEstimation::PosePoints pose);

        int network_input_width;
        int network_input_height;
        float confidence_threshold;
        float mean_offset;

    private:
        cv::Mat run_network(const cv::Mat &image);

        cv::dnn::Net net;
};

std::ostream &operator<<(std::ostream &out, const struct PoseEstimation::PosePoint &p);

#endif  // POSEESTIMATION_H_
