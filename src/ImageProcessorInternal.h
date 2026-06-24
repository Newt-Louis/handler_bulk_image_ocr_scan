#pragma once

#include <QString>
#include <vector>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/core.hpp>
#endif

struct ProcessingOptions;

#ifdef AUTOPHOTO_HAS_OPENCV
QString findYuNetModelPath();
std::vector<cv::Rect> detectWithYuNet(const cv::Mat &image, const QString &modelPath, float scoreThreshold, const QString &cacheKey = {});
void applyBlur(cv::Mat &image, const cv::Rect &face, const QString &mode, int strength);
#endif
