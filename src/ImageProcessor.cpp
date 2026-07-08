#include "ImageProcessor.h"

#include "ImageCompressor.h"
#include "ImageOrientation.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QStringList>
#include <QDateTime>
#include <QColor>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QPainterPath>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <QCache>
#include <vector>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#endif

namespace {

struct FaceDetectionCacheKey {
    QString filePath;
    float sensitivity;
    bool operator==(const FaceDetectionCacheKey &other) const {
        return filePath == other.filePath && qFuzzyCompare(sensitivity, other.sensitivity);
    }
};

uint qHash(const FaceDetectionCacheKey &key, uint seed = 0) {
    return ::qHash(key.filePath, seed) ^ ::qHash(static_cast<uint>(key.sensitivity * 100.0f));
}

struct FaceDetectionCacheEntry {
    std::vector<cv::Rect> faces;
    cv::Size imageSize;
};

QCache<FaceDetectionCacheKey, FaceDetectionCacheEntry> s_faceCache(100);

} // namespace

#ifdef AUTOPHOTO_HAS_OPENCV
QString findYuNetModelPath()
{
    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("models/face_detection_yunet_2023mar.onnx")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../Resources/models/face_detection_yunet_2023mar.onnx")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../models/face_detection_yunet_2023mar.onnx")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../models/face_detection_yunet_2023mar.onnx")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("face_detection_yunet_2023mar.onnx")),
        QDir::current().filePath(QStringLiteral("models/face_detection_yunet_2023mar.onnx")),
        QStringLiteral("models/face_detection_yunet_2023mar.onnx"),
    };

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
    return {};
}

QString findCascadePath(const QString &fileName)
{
    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("models/%1").arg(fileName)),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../Resources/models/%1").arg(fileName)),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../models/%1").arg(fileName)),
        QStringLiteral("/usr/share/opencv4/haarcascades/%1").arg(fileName),
        QStringLiteral("/usr/local/share/opencv4/haarcascades/%1").arg(fileName),
        QStringLiteral("/opt/homebrew/share/opencv4/haarcascades/%1").arg(fileName),
        QStringLiteral("/usr/share/opencv/haarcascades/%1").arg(fileName),
    };

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
    return {};
}

cv::Rect privacyRect(const cv::Rect &rect, const cv::Size &bounds)
{
    const int padX = static_cast<int>(rect.width * 0.45);
    const int padTop = static_cast<int>(rect.height * 0.52);
    const int padBottom = static_cast<int>(rect.height * 0.42);
    const int x = std::max(0, rect.x - padX);
    const int y = std::max(0, rect.y - padTop);
    const int right = std::min(bounds.width, rect.x + rect.width + padX);
    const int bottom = std::min(bounds.height, rect.y + rect.height + padBottom);
    return cv::Rect(x, y, std::max(1, right - x), std::max(1, bottom - y));
}

void applyTimestamp(cv::Mat &image, const ProcessingOptions &options, const QString &sourcePath)
{
    if (!options.timestampEnabled) return;
    
    QFileInfo fi(sourcePath);
    QString fileTimeText = fi.birthTime().isValid() ? fi.birthTime().toString(options.timestampFormat) : fi.lastModified().toString(options.timestampFormat);
    QString userText = options.timestampCustomText;
    
    qDebug() << "[Timestamp] Enabled:" << options.timestampEnabled 
             << "Pos:" << options.timestampPosition 
             << "CustomText:" << userText
             << "TimeText:" << fileTimeText
             << "Format:" << options.timestampFormat;
    
    QImage::Format fmt = QImage::Format_BGR888;
    if (image.channels() == 4) fmt = QImage::Format_ARGB32;
    else if (image.channels() == 1) fmt = QImage::Format_Grayscale8;
    
    QImage qimg(image.data, image.cols, image.rows, static_cast<int>(image.step), fmt);
    QPainter painter(&qimg);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    double baseScale = std::max(1.0, image.rows / 1000.0);
    int pixelSize = static_cast<int>(options.timestampSize * baseScale);
    
    QFont font(options.timestampFont);
    font.setPixelSize(pixelSize);
    painter.setFont(font);
    
    QFontMetrics metrics(font);
    int userTextHeight = userText.isEmpty() ? 0 : metrics.height();
    int timeTextHeight = metrics.height();
    int userTextWidth = userText.isEmpty() ? 0 : metrics.horizontalAdvance(userText);
    int timeTextWidth = metrics.horizontalAdvance(fileTimeText);
    
    int lineSpacing = userText.isEmpty() ? 0 : static_cast<int>(10 * baseScale);
    int maxWidth = std::max(userTextWidth, timeTextWidth);
    
    int x = 0, y1 = 0, y2 = 0;
    int margin = static_cast<int>(20 * baseScale);
    
    if (options.timestampPosition == QStringLiteral("TopLeft")) {
        x = margin;
        y1 = margin + (userText.isEmpty() ? 0 : metrics.ascent());
        y2 = userText.isEmpty() ? margin + metrics.ascent() : margin + userTextHeight + lineSpacing + metrics.ascent();
    } else if (options.timestampPosition == QStringLiteral("TopRight")) {
        x = image.cols - maxWidth - margin;
        y1 = margin + (userText.isEmpty() ? 0 : metrics.ascent());
        y2 = userText.isEmpty() ? margin + metrics.ascent() : margin + userTextHeight + lineSpacing + metrics.ascent();
    } else if (options.timestampPosition == QStringLiteral("BottomLeft")) {
        x = margin;
        y2 = image.rows - margin - metrics.descent();
        y1 = y2 - timeTextHeight - lineSpacing;
    } else if (options.timestampPosition == QStringLiteral("BottomRight")) {
        x = image.cols - maxWidth - margin;
        y2 = image.rows - margin - metrics.descent();
        y1 = y2 - timeTextHeight - lineSpacing;
    } else {
        x = options.timestampX;
        int startY = options.timestampY;
        y1 = startY + (userText.isEmpty() ? 0 : metrics.ascent());
        y2 = userText.isEmpty() ? startY + metrics.ascent() : startY + userTextHeight + lineSpacing + metrics.ascent();
    }
    
    QColor qc(options.timestampColor);
    int outlineWidth = static_cast<int>(std::max(1.0, 2.0 * baseScale));
    
    QPainterPath path;
    if (!userText.isEmpty()) {
        path.addText(x, y1, font, userText);
    }
    path.addText(x, y2, font, fileTimeText);
    
    painter.setPen(QPen(QColor(0, 0, 0, 200), outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(qc);
    painter.drawPath(path);
}

void applyBlur(cv::Mat &image, const cv::Rect &face, const QString &mode, int strength)
{
    const cv::Rect region = privacyRect(face, image.size());
    cv::Mat roi = image(region);
    const int clamped = std::clamp(strength, 1, 100);
    const int minSide = std::max(1, std::min(region.width, region.height));

    cv::Mat mask(region.height, region.width, CV_8UC1, cv::Scalar(0));
    const cv::Point center(region.width / 2, region.height / 2);
    const cv::Size axes(std::max(1, static_cast<int>(region.width * 0.48)),
                        std::max(1, static_cast<int>(region.height * 0.48)));
    cv::ellipse(mask, center, axes, 0.0, 0.0, 360.0, cv::Scalar(255), cv::FILLED, cv::LINE_AA);

    const int featherKernel = std::max(9, ((minSide / 8) | 1));
    cv::GaussianBlur(mask, mask, cv::Size(featherKernel, featherKernel), 0);

    cv::Mat alpha;
    mask.convertTo(alpha, CV_32FC1, 1.0 / 255.0);
    cv::Mat alphaChannels[] = {alpha, alpha, alpha};
    cv::Mat alpha3;
    cv::merge(alphaChannels, 3, alpha3);

    auto blendAnonymized = [&roi, &alpha3](const cv::Mat &anonymized) {
        cv::Mat roiFloat;
        cv::Mat anonymizedFloat;
        roi.convertTo(roiFloat, CV_32FC3);
        anonymized.convertTo(anonymizedFloat, CV_32FC3);
        const cv::Mat blended = anonymizedFloat.mul(alpha3) + roiFloat.mul(cv::Scalar(1.0, 1.0, 1.0) - alpha3);
        blended.convertTo(roi, roi.type());
    };

    if (mode == QLatin1String("pixelate")) {
        const int divisor = std::clamp(clamped / 3, 8, 40);
        const cv::Size smallSize(std::max(1, roi.cols / divisor), std::max(1, roi.rows / divisor));
        cv::Mat small;
        cv::Mat anonymized;
        cv::resize(roi, small, smallSize, 0, 0, cv::INTER_LINEAR);
        cv::resize(small, anonymized, roi.size(), 0, 0, cv::INTER_NEAREST);
        blendAnonymized(anonymized);
        return;
    }

    int kernel = std::clamp((minSide / 3) | 1, 51, 301);
    if (kernel % 2 == 0) {
        ++kernel;
    }

    cv::Mat anonymized;
    cv::GaussianBlur(roi, anonymized, cv::Size(kernel, kernel), 0);
    cv::GaussianBlur(anonymized, anonymized, cv::Size(kernel, kernel), 0);

    const cv::Scalar average = cv::mean(roi, mask);
    cv::Mat flat(roi.size(), roi.type(), average);
    const double flatWeight = std::clamp(0.45 + (clamped / 220.0), 0.45, 0.88);
    cv::addWeighted(anonymized, 1.0 - flatWeight, flat, flatWeight, 0.0, anonymized);
    blendAnonymized(anonymized);
}

double intersectionOverUnion(const cv::Rect &a, const cv::Rect &b)
{
    const cv::Rect intersection = a & b;
    const double intersectionArea = static_cast<double>(intersection.area());
    const double unionArea = static_cast<double>(a.area() + b.area()) - intersectionArea;
    return unionArea > 0.0 ? intersectionArea / unionArea : 0.0;
}

void appendMergedFace(std::vector<cv::Rect> &faces, const cv::Rect &candidate)
{
    if (candidate.area() <= 0) {
        return;
    }

    for (cv::Rect &face : faces) {
        if (intersectionOverUnion(face, candidate) > 0.25
            || static_cast<double>((face & candidate).area()) / std::min(face.area(), candidate.area()) > 0.45) {
            face |= candidate;
            return;
        }
    }

    faces.push_back(candidate);
}

void appendMergedFaces(std::vector<cv::Rect> &faces, const std::vector<cv::Rect> &candidates)
{
    for (const cv::Rect &candidate : candidates) {
        appendMergedFace(faces, candidate);
    }
}

std::vector<cv::Rect> filterByBoxSize(const std::vector<cv::Rect> &faces, const cv::Size &imageSize)
{
    const double imageArea = static_cast<double>(imageSize.width) * imageSize.height;
    std::vector<cv::Rect> result;
    for (const cv::Rect &face : faces) {
        const double faceArea = static_cast<double>(face.area());
        const double areaRatio = faceArea / imageArea;
        if (areaRatio < 0.001 || areaRatio > 0.60) {
            continue;
        }
        const double aspectRatio = static_cast<double>(face.width) / std::max(1, face.height);
        if (aspectRatio < 0.3 || aspectRatio > 3.0) {
            continue;
        }
        result.push_back(face);
    }
    return result;
}

double estimateSkinColorRatio(const cv::Mat &image, const cv::Rect &roi)
{
    cv::Mat roiMat = image(roi);
    cv::Mat hsv;
    cv::cvtColor(roiMat, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask1, mask2, skinMask;
    cv::inRange(hsv, cv::Scalar(0, 40, 80), cv::Scalar(50, 170, 255), mask1);
    cv::inRange(hsv, cv::Scalar(170, 40, 80), cv::Scalar(180, 170, 255), mask2);
    cv::bitwise_or(mask1, mask2, skinMask);

    const int totalPixels = skinMask.rows * skinMask.cols;
    if (totalPixels <= 0) {
        return 0.0;
    }
    const int skinPixels = cv::countNonZero(skinMask);
    return static_cast<double>(skinPixels) / totalPixels;
}

std::vector<cv::Rect> filterBySkinColor(const cv::Mat &image, const std::vector<cv::Rect> &faces, float detectionSensitivity)
{
    const float highConfidenceThreshold = 0.60f;
    std::vector<cv::Rect> result;
    for (const cv::Rect &face : faces) {
        if (detectionSensitivity >= highConfidenceThreshold) {
            result.push_back(face);
            continue;
        }
        cv::Rect expanded = face;
        const int padX = static_cast<int>(expanded.width * 0.3);
        const int padY = static_cast<int>(expanded.height * 0.3);
        expanded.x = std::max(0, expanded.x - padX);
        expanded.y = std::max(0, expanded.y - padY);
        expanded.width = std::min(image.cols - expanded.x, expanded.width + padX * 2);
        expanded.height = std::min(image.rows - expanded.y, expanded.height + padY * 2);
        expanded &= cv::Rect(0, 0, image.cols, image.rows);
        if (expanded.area() <= 0) {
            continue;
        }
        const double skinRatio = estimateSkinColorRatio(image, expanded);
        if (skinRatio >= 0.15) {
            result.push_back(face);
        }
    }
    return result;
}

std::vector<cv::Rect> detectWithYuNet(const cv::Mat &image, const QString &modelPath, float scoreThreshold, const QString &cacheKey = {})
{
    if (!cacheKey.isEmpty()) {
        FaceDetectionCacheKey ck{cacheKey, scoreThreshold};
        FaceDetectionCacheEntry *cached = s_faceCache.object(ck);
        if (cached && cached->imageSize == image.size()) {
            return cached->faces;
        }
    }

    cv::Mat detectImage = image;
    const int maxDetectSide = 640;
    double scale = 1.0;
    const int longestSide = std::max(image.cols, image.rows);
    if (longestSide > maxDetectSide) {
        scale = static_cast<double>(maxDetectSide) / longestSide;
        cv::resize(image, detectImage, cv::Size(), scale, scale, cv::INTER_AREA);
    }

    const std::vector<int> strides = {8, 16, 32};
    const int divisor = 32;
    const int inputWidth = detectImage.cols;
    const int inputHeight = detectImage.rows;
    const int paddedWidth = ((inputWidth - 1) / divisor + 1) * divisor;
    const int paddedHeight = ((inputHeight - 1) / divisor + 1) * divisor;
    const int right = paddedWidth - inputWidth;
    const int bottom = paddedHeight - inputHeight;

    cv::Mat paddedImage;
    cv::copyMakeBorder(detectImage, paddedImage, 0, bottom, 0, right, cv::BORDER_CONSTANT, 0);

    static cv::dnn::Net net;
    static QString loadedModelPath;
    static std::mutex netMutex;
    {
        std::lock_guard<std::mutex> lock(netMutex);
        if (net.empty() || loadedModelPath != modelPath) {
            net = cv::dnn::readNetFromONNX(modelPath.toStdString());
            loadedModelPath = modelPath;
        }
    }
    cv::Mat inputBlob = cv::dnn::blobFromImage(paddedImage);
    net.setInput(inputBlob);

    const std::vector<cv::String> outputNames = {
        "cls_8", "cls_16", "cls_32",
        "obj_8", "obj_16", "obj_32",
        "bbox_8", "bbox_16", "bbox_32",
        "kps_8", "kps_16", "kps_32"
    };
    std::vector<cv::Mat> outputBlobs;
    net.forward(outputBlobs, outputNames);

    const float nmsThreshold = 0.35f;
    const int topK = 5000;
    std::vector<cv::Rect> candidateBoxes;
    std::vector<float> candidateScores;

    for (size_t strideIndex = 0; strideIndex < strides.size(); ++strideIndex) {
        const int stride = strides.at(strideIndex);
        const int cols = paddedWidth / stride;
        const int rows = paddedHeight / stride;
        const cv::Mat &cls = outputBlobs.at(strideIndex);
        const cv::Mat &obj = outputBlobs.at(strideIndex + strides.size());
        const cv::Mat &bbox = outputBlobs.at(strideIndex + strides.size() * 2);

        const float *clsValues = reinterpret_cast<const float *>(cls.data);
        const float *objValues = reinterpret_cast<const float *>(obj.data);
        const float *bboxValues = reinterpret_cast<const float *>(bbox.data);

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const size_t idx = static_cast<size_t>(row * cols + col);
                const float clsScore = std::clamp(clsValues[idx], 0.0f, 1.0f);
                const float objScore = std::clamp(objValues[idx], 0.0f, 1.0f);
                const float score = std::sqrt(clsScore * objScore);
                if (score < scoreThreshold) {
                    continue;
                }

                const float cx = (col + bboxValues[idx * 4 + 0]) * stride;
                const float cy = (row + bboxValues[idx * 4 + 1]) * stride;
                const float width = std::exp(bboxValues[idx * 4 + 2]) * stride;
                const float height = std::exp(bboxValues[idx * 4 + 3]) * stride;
                cv::Rect box(
                    static_cast<int>(cx - width / 2.0f),
                    static_cast<int>(cy - height / 2.0f),
                    static_cast<int>(width),
                    static_cast<int>(height)
                );
                box &= cv::Rect(0, 0, inputWidth, inputHeight);
                if (box.area() > 0) {
                    candidateBoxes.push_back(box);
                    candidateScores.push_back(score);
                }
            }
        }
    }

    std::vector<cv::Rect> result;
    if (candidateBoxes.empty()) {
        if (!cacheKey.isEmpty()) {
            FaceDetectionCacheEntry *entry = new FaceDetectionCacheEntry{{}, image.size()};
            s_faceCache.insert(FaceDetectionCacheKey{cacheKey, scoreThreshold}, entry);
        }
        return result;
    }

    const int minFaceSide = std::max(24, longestSide / 120);
    std::vector<int> keepIndices;
    cv::dnn::NMSBoxes(candidateBoxes, candidateScores, scoreThreshold, nmsThreshold, keepIndices, 1.0f, topK);
    for (int index : keepIndices) {
        cv::Rect face = candidateBoxes.at(static_cast<size_t>(index));
        if (scale != 1.0) {
            face.x = static_cast<int>(face.x / scale);
            face.y = static_cast<int>(face.y / scale);
            face.width = static_cast<int>(face.width / scale);
            face.height = static_cast<int>(face.height / scale);
        }
        face &= cv::Rect(0, 0, image.cols, image.rows);
        if (face.area() > 0 && face.width >= minFaceSide && face.height >= minFaceSide) {
            result.push_back(face);
        }
    }

    if (!cacheKey.isEmpty()) {
        auto *entry = new FaceDetectionCacheEntry{result, image.size()};
        s_faceCache.insert(FaceDetectionCacheKey{cacheKey, scoreThreshold}, entry);
    }
    return result;
}

std::vector<cv::Rect> detectWithCascade(const cv::Mat &image, const QString &cascadePath, bool mirrored)
{
    cv::CascadeClassifier detector;
    if (!detector.load(cascadePath.toStdString())) {
        return {};
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    const int maxDetectSide = 800;
    double scale = 1.0;
    const int longestSide = std::max(gray.cols, gray.rows);
    if (longestSide > maxDetectSide) {
        scale = static_cast<double>(maxDetectSide) / longestSide;
        cv::resize(gray, gray, cv::Size(), scale, scale, cv::INTER_AREA);
    }

    cv::equalizeHist(gray, gray);
    if (mirrored) {
        cv::flip(gray, gray, 1);
    }

    std::vector<cv::Rect> faces;
    detector.detectMultiScale(gray, faces, 1.05, 3, cv::CASCADE_SCALE_IMAGE, cv::Size(22, 22));
    for (cv::Rect &face : faces) {
        if (mirrored) {
            face.x = gray.cols - face.x - face.width;
        }
        if (scale != 1.0) {
            face.x = static_cast<int>(face.x / scale);
            face.y = static_cast<int>(face.y / scale);
            face.width = static_cast<int>(face.width / scale);
            face.height = static_cast<int>(face.height / scale);
        }
        face &= cv::Rect(0, 0, image.cols, image.rows);
    }
    faces.erase(std::remove_if(faces.begin(), faces.end(), [](const cv::Rect &face) {
        return face.area() <= 0;
    }), faces.end());
    return faces;
}

cv::Mat readImageRespectingExif(const QString &sourcePath)
{
    QImage image = readImageWithResolvedOrientation(sourcePath);
    if (image.isNull()) {
        return {};
    }

    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat rgb(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
    return bgr;
}

#endif

ImageProcessor::ImageProcessor(const ProcessingOptions &options)
    : m_options(options)
{
#ifdef AUTOPHOTO_HAS_OPENCV
    cv::setNumThreads(2);
    cv::setUseOptimized(true);
#endif
}

ProcessingResult ImageProcessor::processFile(const QString &sourcePath, const QString &targetPath)
{
#ifdef AUTOPHOTO_HAS_OPENCV
    cv::Mat image;
    if (m_options.rotateEnabled) {
        image = readImageRespectingExif(sourcePath);
    }
    if (image.empty()) {
        image = cv::imread(sourcePath.toStdString(), cv::IMREAD_COLOR);
    }
    if (image.empty()) {
        return {false, 0, {}, QStringLiteral("Could not read image: %1").arg(sourcePath)};
    }

    int facesBlurred = 0;
    QString detectorUsed;
    if (m_options.blurFaces) {
        std::vector<cv::Rect> faces;
        std::vector<cv::Rect> yuNetFaces;

        const QString yuNetModelPath = findYuNetModelPath();
        if (!yuNetModelPath.isEmpty()) {
            try {
                faces = detectWithYuNet(image, yuNetModelPath, m_options.detectionSensitivity, sourcePath);
                yuNetFaces = faces;
                detectorUsed = QStringLiteral("YuNet");
            } catch (const cv::Exception &error) {
                return {
                    false,
                    0,
                    QStringLiteral("YuNet"),
                    QStringLiteral("YuNet detection failed: %1").arg(QString::fromStdString(error.what()))
                };
            }
        } else if (m_options.detector == QLatin1String("yunet")) {
            return {
                false,
                0,
                QStringLiteral("YuNet"),
                QStringLiteral("YuNet model not found. Expected models/face_detection_yunet_2023mar.onnx")
            };
        }

        if (m_options.detector != QLatin1String("yunet-only")) {
            const QStringList cascadeFiles = {
                QStringLiteral("haarcascade_frontalface_default.xml"),
                QStringLiteral("haarcascade_profileface.xml")
            };

            int cascadeFaces = 0;
            for (const QString &cascadeFile : cascadeFiles) {
                const QString cascadePath = findCascadePath(cascadeFile);
                if (cascadePath.isEmpty()) {
                    continue;
                }

                std::vector<cv::Rect> cascadeDetections = detectWithCascade(image, cascadePath, false);
                cascadeFaces += static_cast<int>(cascadeDetections.size());
                appendMergedFaces(faces, cascadeDetections);

                if (cascadeFile == QLatin1String("haarcascade_profileface.xml")) {
                    cascadeDetections = detectWithCascade(image, cascadePath, true);
                    cascadeFaces += static_cast<int>(cascadeDetections.size());
                    appendMergedFaces(faces, cascadeDetections);
                }
            }

            if (cascadeFaces > 0) {
                detectorUsed = detectorUsed.isEmpty()
                    ? QStringLiteral("Haar cascade")
                    : QStringLiteral("%1 + Haar cascade").arg(detectorUsed);
            } else if (detectorUsed.isEmpty()) {
                detectorUsed = QStringLiteral("YuNet");
            }
        }

        if (m_options.sizeFilterEnabled) {
            faces = filterByBoxSize(faces, image.size());
        }
        if (m_options.cascadeCrossCheckEnabled) {
            if (yuNetFaces.empty()) {
                faces.clear();
            } else {
                std::vector<cv::Rect> validated;
                for (const cv::Rect &face : faces) {
                    bool hasYuNetMatch = false;
                    for (const cv::Rect &yn : yuNetFaces) {
                        if (intersectionOverUnion(face, yn) > 0.15) {
                            hasYuNetMatch = true;
                            break;
                        }
                    }
                    if (hasYuNetMatch) {
                        validated.push_back(face);
                    }
                }
                faces = validated;
            }
        }
        if (m_options.skinColorFilterEnabled) {
            const int beforeSkinFilter = static_cast<int>(faces.size());
            faces = filterBySkinColor(image, faces, m_options.detectionSensitivity);
            const int afterSkinFilter = static_cast<int>(faces.size());
            if (beforeSkinFilter > 0 && afterSkinFilter == 0) {
                faces.clear();
            } else if (beforeSkinFilter > 2 && afterSkinFilter < beforeSkinFilter / 4) {
                faces.clear();
            } else if (afterSkinFilter > 0) {
                double totalSkinRatio = 0.0;
                int validRoiCount = 0;
                for (const cv::Rect &face : faces) {
                    cv::Rect expanded = face;
                    const int padX = static_cast<int>(expanded.width * 0.3);
                    const int padY = static_cast<int>(expanded.height * 0.3);
                    expanded.x = std::max(0, expanded.x - padX);
                    expanded.y = std::max(0, expanded.y - padY);
                    expanded.width = std::min(image.cols - expanded.x, expanded.width + padX * 2);
                    expanded.height = std::min(image.rows - expanded.y, expanded.height + padY * 2);
                    expanded &= cv::Rect(0, 0, image.cols, image.rows);
                    if (expanded.area() > 0) {
                        totalSkinRatio += estimateSkinColorRatio(image, expanded);
                        ++validRoiCount;
                    }
                }
                if (validRoiCount > 0) {
                    const double avgSkinRatio = totalSkinRatio / validRoiCount;
                    if (avgSkinRatio < 0.20) {
                        faces.clear();
                    }
                }
            }
        }

        for (const cv::Rect &face : faces) {
            applyBlur(image, face, m_options.blurMode, m_options.strength);
            ++facesBlurred;
        }
    }
    
    applyTimestamp(image, m_options, sourcePath);

    QDir().mkpath(QFileInfo(targetPath).absolutePath());
    bool saved = false;
    if (m_options.compressionEnabled && m_options.compressionLevel > 0) {
        const QString tempDir = QDir::tempPath();
        const QString tempName = QStringLiteral("autophoto_compress_%1.jpg")
            .arg(QCryptographicHash::hash(targetPath.toUtf8(), QCryptographicHash::Md5).toHex().left(12));
        const QString tempPath = QDir(tempDir).filePath(tempName);
        saved = cv::imwrite(tempPath.toStdString(), image);
        if (saved) {
            saved = ImageCompressor::compress(tempPath, targetPath, m_options.compressionLevel, m_options.outputFormat);
            QFile::remove(tempPath);
        }
    } else {
        saved = cv::imwrite(targetPath.toStdString(), image);
    }
    if (!saved) {
        return {false, facesBlurred, detectorUsed, QStringLiteral("Could not write image: %1").arg(targetPath)};
    }
    return {true, facesBlurred, detectorUsed, {}};
#else
    QDir().mkpath(QFileInfo(targetPath).absolutePath());
    QFile::remove(targetPath);
    QImage image;
    if (m_options.rotateEnabled) {
        image = readImageWithResolvedOrientation(sourcePath);
    }
    if (image.isNull()) {
        image = QImage(sourcePath);
    }
    if (image.isNull()) {
        return {false, 0, {}, QStringLiteral("Could not read image: %1").arg(sourcePath)};
    }
    bool saved = false;
    if (m_options.compressionEnabled && m_options.compressionLevel > 0) {
        const QString tempDir = QDir::tempPath();
        const QString tempName = QStringLiteral("autophoto_compress_%1.jpg")
            .arg(QCryptographicHash::hash(targetPath.toUtf8(), QCryptographicHash::Md5).toHex().left(12));
        const QString tempPath = QDir(tempDir).filePath(tempName);
        saved = image.save(tempPath, "JPG", 95);
        if (saved) {
            saved = ImageCompressor::compress(tempPath, targetPath, m_options.compressionLevel, m_options.outputFormat);
            QFile::remove(tempPath);
        }
    } else {
        saved = image.save(targetPath, nullptr, 92);
    }
    if (!saved) {
        return {false, 0, {}, QStringLiteral("Could not write image: %1").arg(targetPath)};
    }
    return {true, 0, QStringLiteral("Qt image fallback"), {}};
#endif
}
