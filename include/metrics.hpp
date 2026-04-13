#pragma once

#include <opencv2/opencv.hpp>

struct MSE {
    double mseB = 0.0;
    double mseG = 0.0;
    double mseR = 0.0;
};

struct PSNR {
    double psnrB = 0.0;
    double psnrG = 0.0;
    double psnrR = 0.0;
};

class Metrics {
public:
    static MSE computeMSE(const cv::Mat& original, const cv::Mat& compressed);
    static PSNR computePSNR(const MSE& mse);
};