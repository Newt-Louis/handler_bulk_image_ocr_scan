#include "ImageCompressor.h"

#include "ImageOrientation.h"

#include <QImage>
#include <QFileInfo>
#include <QDir>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

CompressionParams ImageCompressor::paramsFromLevel(int level)
{
    const int clamped = qBound(0, level, 100);
    CompressionParams params;

    if (clamped <= 0) {
        params.jpegQuality = 95;
        params.resizeRatio = 1.0;
    } else if (clamped <= 25) {
        params.jpegQuality = 95 - (clamped * 0.4);
        params.resizeRatio = 1.0;
    } else if (clamped <= 50) {
        const int t = clamped - 25;
        params.jpegQuality = 85 - (t * 0.6);
        params.resizeRatio = 1.0 - (t * 0.006);
    } else if (clamped <= 75) {
        const int t = clamped - 50;
        params.jpegQuality = 70 - (t * 0.8);
        params.resizeRatio = 0.85 - (t * 0.006);
    } else {
        const int t = clamped - 75;
        params.jpegQuality = 50 - (t * 1.2);
        params.resizeRatio = 0.70 - (t * 0.008);
    }

    params.jpegQuality = qBound(20, params.jpegQuality, 95);
    params.resizeRatio = qBound(0.30, params.resizeRatio, 1.0);

    return params;
}

bool ImageCompressor::compress(const QString &sourcePath, const QString &targetPath, int compressionLevel, const QString &format)
{
    const CompressionParams params = paramsFromLevel(compressionLevel);
    const QString outputFormat = format.isEmpty() ? QStringLiteral("jpg") : format;

#ifdef AUTOPHOTO_HAS_OPENCV
    cv::Mat image = cv::imread(sourcePath.toStdString(), cv::IMREAD_COLOR);
    if (image.empty()) {
        return false;
    }

    if (params.resizeRatio < 1.0 - 1e-6) {
        const int newWidth = std::max(1, static_cast<int>(image.cols * params.resizeRatio));
        const int newHeight = std::max(1, static_cast<int>(image.rows * params.resizeRatio));
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_AREA);
        image = resized;
    }

    QDir().mkpath(QFileInfo(targetPath).absolutePath());

    if (outputFormat == QLatin1String("webp")) {
        const std::vector<int> compressionParams = {cv::IMWRITE_WEBP_QUALITY, params.jpegQuality};
        return cv::imwrite(targetPath.toStdString(), image, compressionParams);
    }

    if (outputFormat == QLatin1String("png")) {
        const int pngCompression = qBound(0, (100 - params.jpegQuality) / 5, 9);
        const std::vector<int> compressionParams = {cv::IMWRITE_PNG_COMPRESSION, pngCompression};
        return cv::imwrite(targetPath.toStdString(), image, compressionParams);
    }

    const std::vector<int> compressionParams = {cv::IMWRITE_JPEG_QUALITY, params.jpegQuality};
    return cv::imwrite(targetPath.toStdString(), image, compressionParams);
#else
    QImage image = readImageWithResolvedOrientation(sourcePath);
    if (image.isNull()) {
        return false;
    }

    if (params.resizeRatio < 1.0 - 1e-6) {
        const int newWidth = std::max(1, static_cast<int>(image.width() * params.resizeRatio));
        const int newHeight = std::max(1, static_cast<int>(image.height() * params.resizeRatio));
        image = image.scaled(newWidth, newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QDir().mkpath(QFileInfo(targetPath).absolutePath());

    if (outputFormat == QLatin1String("webp")) {
        return image.save(targetPath, "WEBP", params.jpegQuality);
    }
    if (outputFormat == QLatin1String("png")) {
        return image.save(targetPath, "PNG");
    }
    return image.save(targetPath, "JPG", params.jpegQuality);
#endif
}
