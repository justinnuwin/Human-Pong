#include "PoseEstimation.hpp"

#include <opencv2/highgui.hpp>      // imshow, waitKey

const char *PoseEstimation::bodypart_names[] = {"Nose", "Neck", "RShoulder", "RElbow", "RWrist",
                                                "LShoulder", "LElbow", "LWrist", "RHip", "RKnee",
                                                "RAnkle", "LHip", "LKnee", "LAnkle", "REye",
                                                "LEye", "REar", "LEar", "Background"};

std::ostream &operator<<(std::ostream &out, const struct PoseEstimation::PosePoint &p) {
    out << p.toString();
    return out;
}

std::string PoseEstimation::PosePoint::toString() const {
    static char buff[64] = {'\0'};
    sprintf(buff, "%s [%d, %d] %3.2f", PoseEstimation::bodypart_names[part], location.x, location.y, confidence);
    return std::string(buff);
}

PoseEstimation::PoseEstimation(std::string graph_path, float confidence_threshold, int network_input_width, int network_input_height, float mean_offset) {
    net = cv::dnn::readNetFromTensorflow(graph_path);
    this->confidence_threshold = confidence_threshold;
    this->network_input_width = network_input_width;
    this->network_input_height = network_input_height;
    this->mean_offset = mean_offset;
}

cv::Mat PoseEstimation::run_network(const cv::Mat &image) {
    cv::Mat input_blob = cv::dnn::blobFromImage(image, 1., cv::Size2i(network_input_width, network_input_height), cv::Scalar(mean_offset, mean_offset, mean_offset), true, false);
    net.setInput(input_blob);
    cv::Mat network_output = net.forward();
    return network_output;
}

PoseEstimation::PosePoints PoseEstimation::estimate(const cv::Mat &image) {
    cv::Mat output = run_network(image);

    static int heatmap_width = output.size[2];
    static int heatmap_height = output.size[3];

    static float heatmap_width_scale = (float)image.rows / (float)heatmap_width;
    static float heatmap_height_scale = (float)image.cols / (float)heatmap_height;

    std::vector<struct PosePoint> pose_points;
    for (int i = 0; i < _e_BodyParts_end; ++i) {
        cv::Range heatmap_range[4] = {cv::Range(0, 1), cv::Range(i, i+1),
                                      cv::Range::all(), cv::Range::all()};  // https://stackoverflow.com/a/26938681
        cv::Mat heatmap_slice = output(heatmap_range);
        float *slice_data = (float *)heatmap_slice.data + i * heatmap_width * heatmap_height;
        cv::Mat heatmap(cv::Size(heatmap_width, heatmap_height), CV_32F, slice_data);
        double confidence;
        cv::Point global_max;
        cv::minMaxLoc(heatmap, NULL, &confidence, NULL, &global_max);   // Only one pose can be detected
        global_max = cv::Point2f(global_max);
        cv::Point position = cv::Point2i(global_max.x * heatmap_width_scale, global_max.y * heatmap_height_scale);
        if (confidence > confidence_threshold)
            pose_points.emplace_back((enum BodyParts)i, position, confidence);
    }
    return pose_points;
}
