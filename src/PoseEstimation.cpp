#include "PoseEstimation.hpp"

#include <opencv2/imgproc.hpp>      // Drawing

#include <cstring>      // sprintf

const std::map<enum PoseEstimation::BodyParts, int> PoseEstimation::bodypart_layer_map = { {e_Nose, 0},
    {e_Neck, 10}, {e_RShoulder, 21}, {e_RElbow, 3}, {e_RWrist, 22}, {e_LShoulder, 1}, {e_LElbow, 17},
    {e_LWrist, 2}, {e_RHip, 14}, {e_RKnee, 6}, {e_RAnkle, 15}, {e_LHip, 11}, {e_LKnee, 12}, {e_LAnkle, 5},
    {e_REye, 28}, {e_LEye, 7}, {e_REar, 27}, {e_LEar, 8}, {e_Background, 9}};

const std::map<enum PoseEstimation::BodyParts, std::string> PoseEstimation::bodypart_names = {{e_Nose, "Nose"},
    {e_Neck, "Neck"}, {e_RShoulder, "RShoulder"}, {e_RElbow, "RElbow"}, {e_RWrist, "RWrist"},
    {e_LShoulder,"LShoulder"}, {e_LElbow, "LElbow"}, {e_LWrist, "LWrist"}, {e_RHip, "RHip"},
    {e_RKnee,"RKnee"}, {e_RAnkle, "RAnkle"}, {e_LHip, "LHip"}, {e_LKnee, "LKnee"}, {e_LAnkle, "LAnkle"},
    {e_REye, "REye"}, {e_LEye, "LEye"}, {e_REar, "REar"}, {e_LEar, "LEar"}, {e_Background, "Background"}};

const std::vector<PoseEstimation::BodyParts> PoseEstimation::bodyparts_list = {e_Nose, e_Neck, e_RShoulder,
    e_RElbow, e_RWrist, e_LShoulder, e_LElbow, e_LWrist, e_RHip, e_RKnee, e_RAnkle, e_LHip, e_LKnee, e_LAnkle,
    e_REye, e_LEye, e_REar, e_LEar, e_Background};

const std::map<enum PoseEstimation::BodyParts, std::vector<enum PoseEstimation::BodyParts>> PoseEstimation::bodypart_connections = 
    {
        {e_Nose, {e_LEye, e_REye, e_Neck}}, 
        {e_LEye, {e_LEar}},
        {e_REye, {e_REar}},
        {e_Neck, {e_LShoulder, e_RShoulder, e_LHip, e_RHip}},
        {e_LShoulder, {e_LElbow}},
        {e_RShoulder, {e_RElbow}},
        {e_LElbow, {e_LWrist}},
        {e_RElbow, {e_RWrist}},
        {e_LHip, {e_LKnee}},
        {e_RHip, {e_RKnee}},
        {e_LKnee, {e_LAnkle}},
        {e_RKnee, {e_RAnkle}}
    };

std::ostream &operator<<(std::ostream &out, const struct PoseEstimation::PosePoint &p) {
    out << p.toString();
    return out;
}

std::string PoseEstimation::PosePoint::toString() const {
    static char buff[64] = {'\0'};
    sprintf(buff, "%s [%d, %d] %3.2f", PoseEstimation::bodypart_names.at(part).c_str(), location.x, location.y, confidence);
    return std::string(buff);
}

PoseEstimation::PoseEstimation(std::string graph_path, float confidence_threshold, int network_input_width, int network_input_height, float mean_offset) {
    net = cv::dnn::readNetFromTensorflow(graph_path);
    /*
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    */
    this->confidence_threshold = confidence_threshold;
    this->network_input_width = network_input_width;
    this->network_input_height = network_input_height;
    this->mean_offset = mean_offset;
}

cv::Mat PoseEstimation::run_network(const cv::Mat &image) {
    cv::Mat input_blob = cv::dnn::blobFromImage(image, 1., cv::Size(network_input_width, network_input_height), cv::Scalar(mean_offset, mean_offset, mean_offset), false, false);
    net.setInput(input_blob);
    cv::Mat network_output = net.forward();
    return network_output;
}

PoseEstimation::PosePoints PoseEstimation::estimate(const cv::Mat &image) {
    cv::Mat output = run_network(image);

    static int heatmap_height = output.size[2];
    static int heatmap_width = output.size[3];

    static float heatmap_width_scale = (float)image.cols / (float)heatmap_width;
    static float heatmap_height_scale = (float)image.rows / (float)heatmap_height;

    std::vector<struct PosePoint> pose;
    for (const enum BodyParts &i : bodyparts_list) {
    //for (int i = 0; i <= 35; ++i) {
        // https://stackoverflow.com/a/26938681
        int layer_number = bodypart_layer_map.at((enum BodyParts)i);
        // int layer_number = i;
        cv::Range heatmap_range[4] = {cv::Range(0, 1), cv::Range(layer_number, layer_number+1),
                                      cv::Range::all(), cv::Range::all()};
        cv::Mat heatmap_slice = output(heatmap_range);
        float *slice_data = (float *)heatmap_slice.data + layer_number * heatmap_width * heatmap_height;
        cv::Mat heatmap(cv::Size(heatmap_width, heatmap_height), CV_32F, slice_data);
        double confidence;
        cv::Point global_max;
        cv::minMaxLoc(heatmap, NULL, &confidence, NULL, &global_max);   // Only one pose can be detected
        if (confidence > confidence_threshold) {
            global_max = cv::Point2f(global_max);
            cv::Point position = cv::Point2i(global_max.x * heatmap_width_scale, global_max.y * heatmap_height_scale);
            pose.emplace_back((enum BodyParts)i, position, confidence);
        }
    }
    return pose;
}

void PoseEstimation::draw_pose(cv::Mat &image, const PoseEstimation::PosePoints pose) {
    for (const struct PosePoint &point : pose) {
        cv::circle(image, point.location, 4, cv::Scalar(255, 255, 0), -1, cv::FILLED);

        char buffer[10] = {'\0'};
        sprintf(buffer, "%s", bodypart_names.at(point.part).c_str());
        cv::putText(image, buffer, point.location, cv::FONT_HERSHEY_DUPLEX, 1., CV_RGB(255, 128, 128), 2);

        if (bodypart_connections.find(point.part) != bodypart_connections.end()) {
            std::vector<enum BodyParts> connections = bodypart_connections.at(point.part);
            for (enum BodyParts &part : connections) {
                for (const struct PosePoint &_point : pose) {
                    if (_point.part == part)
                        cv::line(image, point.location, _point.location, cv::Scalar(0, 255, 0), 3);
                }
            }
        }
    }
}
