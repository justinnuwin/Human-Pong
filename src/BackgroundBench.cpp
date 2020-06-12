#include <iostream>
#include <chrono>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BackgroundSub.hpp"

int main(int argc, char *argv[]) {

   if(argc != 6) {
      std::cerr << "usage: BackgroundBench <model> <background> <input> <output> <iterations>";
      return -1;
   }

   cv::Mat input = cv::imread(argv[3]);
   cv::Mat background = cv::imread(argv[2]);
   cv::resize(background, background, input.size(), 0.0, 0.0, cv::INTER_CUBIC);

   BackgroundSub bSub = BackgroundSub(background, argv[1]);

   int iterations = atoi(argv[5]);
   cv::Mat output;
   auto start = std::chrono::high_resolution_clock::now();
   for (int i = 0; i < iterations; i++) {
      output = bSub.Sub(input);

   }
   auto end = std::chrono::high_resolution_clock::now();
   int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/iterations;
   std::cout << "Avg Substitution Speed: " << duration << "ms" << std::endl;
   cv::imwrite(argv[4], output);
   return 0;
}
