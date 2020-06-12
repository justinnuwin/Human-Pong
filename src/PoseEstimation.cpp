#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>      // imshow, waitKey

#include <opencv2/imgcodecs.hpp>    // imread

enum BodyParts {e_Nose, e_Neck, e_RShoulder, e_RElbow, e_RWrist, e_LShoulder, e_LElbow, 
                e_LWrist, e_RHip, e_RKnee, e_RAnkle, e_LHip, e_LKnee, e_LAnkle, e_REye,
                e_LEye, e_REar, e_LEar, e_Background, _e_BodyParts_end};

std::string bodypart_names[19] = {"Nose", "Neck", "RShoulder", "RElbow", "RWrist",
                                  "LShoulder", "LElbow", "LWrist", "RHip", "RKnee",
                                  "RAnkle", "LHip", "LKnee", "LAnkle", "REye",
                                  "LEye", "REar", "LEar", "Background"};

int main(int argc, char *argv[]) {


    cv::Mat frame;
    
    frame = cv::imread("test.jpg");

    int height = frame.rows;
    int width = frame.cols;
    
    cv::dnn::Net net = cv::dnn::readNetFromTensorflow("../resources/graph_opt.pb");

    int network_input_width = 368;
    int network_input_height = 368;
    cv::Mat input_blob = cv::dnn::blobFromImage(frame, 1., cv::Size2i(network_input_width, network_input_height), cv::Scalar(127.5, 127.5, 127.5), true, false);
    net.setInput(input_blob);


    cv::Mat out = net.forward();

    int heatmap_width = out.size[2];
    int heatmap_height = out.size[3];

    float heatmap_width_scale = (float)width / (float)heatmap_width;
    float heatmap_height_scale = (float)height / (float)heatmap_height;

    for (int i = 0; i < _e_BodyParts_end; ++i) {
        // https://stackoverflow.com/a/26938681
        cv::Range heatmap_range[4] = {cv::Range(0, 1), cv::Range(i, i+1),
                                      cv::Range::all(), cv::Range::all()};
        cv::Mat heatmap_slice = out(heatmap_range);
        float *slice_data = (float *)heatmap_slice.data + i * out.size[2] * out.size[3];
        cv::Mat heatmap(cv::Size(out.size[2], out.size[3]), CV_32F, slice_data);
        double confidence;
        cv::Point global_max;
        cv::minMaxLoc(heatmap, NULL, &confidence, NULL, &global_max);   // Only one pose can be detected
        global_max = cv::Point2f(global_max);
        cv::Point position = cv::Point(global_max.x * heatmap_width_scale, global_max.y * heatmap_height_scale);
        if (confidence > 0.3)
            std::cout << bodypart_names[i] << "\t" << position << "\tConfidence: " << confidence << std::endl;
    }

    return 0;

}
