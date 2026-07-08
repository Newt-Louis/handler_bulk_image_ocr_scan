#pragma once

#include <QString>
#include <vector>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/core.hpp>
#endif

struct ProcessingOptions;

#ifdef AUTOPHOTO_HAS_OPENCV
// Model & cascade paths
QString findYuNetModelPath();
QString findCascadePath(const QString &fileName);

// Detection
std::vector<cv::Rect> detectWithYuNet(const cv::Mat &image, const QString &modelPath, float scoreThreshold, const QString &cacheKey = {});
std::vector<cv::Rect> detectWithCascade(const cv::Mat &image, const QString &cascadePath, bool mirrored);

// Merge & filter
void appendMergedFaces(std::vector<cv::Rect> &faces, const std::vector<cv::Rect> &candidates);
std::vector<cv::Rect> filterByBoxSize(const std::vector<cv::Rect> &faces, const cv::Size &imageSize);
std::vector<cv::Rect> filterBySkinColor(const cv::Mat &image, const std::vector<cv::Rect> &faces, float detectionSensitivity);
std::vector<cv::Rect> filterCascadeCrossCheck(const cv::Mat &image, const std::vector<cv::Rect> &yuNetFaces, const QString &cascadePath);

void applyTimestamp(cv::Mat &image, const ProcessingOptions &options, const QString &sourcePath);

// Blur
void applyBlur(cv::Mat &image, const cv::Rect &face, const QString &mode, int strength);

// Image I/O
cv::Mat readImageRespectingExif(const QString &sourcePath);
#endif
