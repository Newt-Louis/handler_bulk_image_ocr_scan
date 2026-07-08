#include "BatchProcessor.h"

#include "BoundedBuffer.h"
#include "ImageCompressor.h"
#include "ImageOrientation.h"
#include "ImageProcessorInternal.h"
#include "ImageProcessor.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QtGlobal>

#include <stdexcept>
#include <thread>
#include <functional>
#include <algorithm>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(Q_OS_MAC)
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

namespace {
quint64 getAvailableRAM() {
#ifdef Q_OS_WIN
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
#elif defined(Q_OS_MAC)
    int mib[2];
    int64_t physical_memory;
    size_t length = sizeof(int64_t);
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
    return physical_memory;
#else
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return static_cast<quint64>(pages) * static_cast<quint64>(page_size);
#endif
}
} // namespace

namespace {

struct ReadData {
    int index = 0;
    QString sourcePath;
    QString targetPath;
#ifdef AUTOPHOTO_HAS_OPENCV
    cv::Mat image;
#endif
};

struct DetectedData {
    int index = 0;
    QString sourcePath;
    QString targetPath;
#ifdef AUTOPHOTO_HAS_OPENCV
    cv::Mat image;
    std::vector<cv::Rect> faces;
#endif
};

// ── Reader Stage ──────────────────────────────────────────────

void readerStage(const QStringList &inputFiles,
                 const QString &outputFolder,
                 const QString &renamePattern,
                 const ProcessingOptions &options,
                 std::shared_ptr<BoundedBuffer<ReadData>> output,
                 std::atomic_bool &cancelled,
                 std::atomic_bool &paused)
{
    qDebug() << "[Reader] Starting. Files:" << inputFiles.size() << "Output:" << outputFolder;

    const int total = inputFiles.size();

    for (int i = 0; i < total; ++i) {
        if (cancelled.load()) {
            qDebug() << "[Reader] Cancelled at" << i;
            break;
        }

        while (paused.load() && !cancelled.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (cancelled.load()) break;

        const QFileInfo source(inputFiles.at(i));
        const QString baseName = renamePattern.isEmpty() ? source.completeBaseName() : renamePattern;
        const QString number = QString::number(i + 1).rightJustified(4, QLatin1Char('0'));
        const QString ext = (options.compressionEnabled && options.compressionLevel > 0 && options.outputFormat != QLatin1String("jpg"))
            ? options.outputFormat
            : (source.suffix().isEmpty() ? QStringLiteral("jpg") : source.suffix());
        const QString fileName = QStringLiteral("%1_%2.%3").arg(baseName, number, ext);

        ReadData item;
        item.index = i;
        item.sourcePath = source.absoluteFilePath();
        item.targetPath = QDir(outputFolder).filePath(fileName);

#ifdef AUTOPHOTO_HAS_OPENCV
        if (options.rotateEnabled) {
            item.image = readImageRespectingExif(item.sourcePath);
        }
        if (item.image.empty()) {
            item.image = cv::imread(item.sourcePath.toStdString(), cv::IMREAD_COLOR);
        }
        if (item.image.empty()) {
            qDebug() << "[Reader] WARNING: Could not read image:" << item.sourcePath;
        } else {
            qDebug() << "[Reader] Read" << source.fileName() << "(" << item.image.cols << "x" << item.image.rows << ")";
        }
#endif

        output->push(std::move(item));
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    output->close();
    qDebug() << "[Reader] Done. Pushed" << total << "items.";
}

// ── Detector Stage ────────────────────────────────────────────

void detectorStage(std::shared_ptr<BoundedBuffer<ReadData>> input,
                   std::shared_ptr<BoundedBuffer<DetectedData>> output,
                   const ProcessingOptions &options,
                   std::atomic_bool &cancelled,
                   std::atomic_bool &paused,
                   std::atomic_int &detectedCount)
{
    qDebug() << "[Detector] Starting. blurFaces:" << options.blurFaces;

#ifdef AUTOPHOTO_HAS_OPENCV
    const QString yuNetModelPath = findYuNetModelPath();
    qDebug() << "[Detector] YuNet model:" << (yuNetModelPath.isEmpty() ? "NOT FOUND" : yuNetModelPath);

    int itemCount = 0;
    while (true) {
        auto data = input->pop();
        if (!data.has_value()) {
            qDebug() << "[Detector] Buffer closed, exiting.";
            break;
        }

        if (cancelled.load()) {
            qDebug() << "[Detector] Cancelled at item" << itemCount;
            break;
        }

        while (paused.load() && !cancelled.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (cancelled.load()) break;

        DetectedData out;
        out.index = data->index;
        out.sourcePath = std::move(data->sourcePath);
        out.targetPath = std::move(data->targetPath);
        out.image = std::move(data->image);

        if (options.blurFaces && !out.image.empty()) {
            std::vector<cv::Rect> faces;
            std::vector<cv::Rect> yuNetFaces;

            if (!yuNetModelPath.isEmpty()) {
                try {
                    faces = detectWithYuNet(out.image, yuNetModelPath, options.detectionSensitivity, out.sourcePath);
                    yuNetFaces = faces;
                    qDebug() << "[Detector] YuNet found" << faces.size() << "faces in item" << itemCount;
                } catch (const cv::Exception &e) {
                    qDebug() << "[Detector] YuNet exception:" << QString::fromStdString(e.what());
                } catch (const std::exception &e) {
                    qDebug() << "[Detector] std::exception:" << e.what();
                }
            }

            // Cascade fallback
            if (options.detector != QLatin1String("yunet-only")) {
                const QStringList cascadeFiles = {
                    QStringLiteral("haarcascade_frontalface_default.xml"),
                    QStringLiteral("haarcascade_profileface.xml")
                };
                for (const QString &cascadeFile : cascadeFiles) {
                    const QString cascadePath = findCascadePath(cascadeFile);
                    if (cascadePath.isEmpty()) continue;

                    try {
                        std::vector<cv::Rect> cascadeDetections = detectWithCascade(out.image, cascadePath, false);
                        appendMergedFaces(faces, cascadeDetections);

                        if (cascadeFile == QLatin1String("haarcascade_profileface.xml")) {
                            cascadeDetections = detectWithCascade(out.image, cascadePath, true);
                            appendMergedFaces(faces, cascadeDetections);
                        }
                    } catch (const std::exception &e) {
                        qDebug() << "[Detector] Cascade exception:" << e.what();
                    }
                }
            }

            // Apply filters
            if (options.sizeFilterEnabled) {
                faces = filterByBoxSize(faces, out.image.size());
            }
            if (options.cascadeCrossCheckEnabled && yuNetFaces.empty()) {
                faces.clear();
            }
            if (options.skinColorFilterEnabled && !faces.empty()) {
                faces = filterBySkinColor(out.image, faces, options.detectionSensitivity);
                // Aggregate validation
                if (!faces.empty()) {
                    double totalSkinRatio = 0.0;
                    int validRoiCount = 0;
                    for (const cv::Rect &face : faces) {
                        cv::Rect expanded = face;
                        const int padX = static_cast<int>(expanded.width * 0.3);
                        const int padY = static_cast<int>(expanded.height * 0.3);
                        expanded.x = (std::max)(0, expanded.x - padX);
                        expanded.y = (std::max)(0, expanded.y - padY);
                        expanded.width = (std::min)(out.image.cols - expanded.x, expanded.width + padX * 2);
                        expanded.height = (std::min)(out.image.rows - expanded.y, expanded.height + padY * 2);
                        expanded &= cv::Rect(0, 0, out.image.cols, out.image.rows);
                        if (expanded.area() > 0) {
                            cv::Mat roi = out.image(expanded);
                            cv::Mat hsv;
                            cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
                            cv::Mat mask1, mask2, skinMask;
                            cv::inRange(hsv, cv::Scalar(0, 40, 80), cv::Scalar(50, 170, 255), mask1);
                            cv::inRange(hsv, cv::Scalar(170, 40, 80), cv::Scalar(180, 170, 255), mask2);
                            cv::bitwise_or(mask1, mask2, skinMask);
                            const int totalPixels = skinMask.rows * skinMask.cols;
                            if (totalPixels > 0) {
                                totalSkinRatio += static_cast<double>(cv::countNonZero(skinMask)) / totalPixels;
                                ++validRoiCount;
                            }
                        }
                    }
                    if (validRoiCount > 0 && (totalSkinRatio / validRoiCount) < 0.20) {
                        qDebug() << "[Detector] Aggregate skin validation: rejecting ALL faces (avg skin ratio:" << (totalSkinRatio / validRoiCount) << ")";
                        faces.clear();
                    }
                }
            }

            out.faces = std::move(faces);
            detectedCount.fetch_add(static_cast<int>(out.faces.size()));
        }

        qDebug() << "[Detector] Item" << itemCount << ": " << out.faces.size() << "faces";
        output->push(std::move(out));
        ++itemCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
#else
    while (true) {
        auto data = input->pop();
        if (!data.has_value()) break;
        if (cancelled.load()) break;
        DetectedData out;
        out.index = data->index;
        out.sourcePath = std::move(data->sourcePath);
        out.targetPath = std::move(data->targetPath);
        output->push(std::move(out));
    }
#endif

    output->close();
    qDebug() << "[Detector] Done. Processed" << itemCount << "items.";
}

// ── Writer Stage ──────────────────────────────────────────────

void writerStage(std::shared_ptr<BoundedBuffer<DetectedData>> input,
                 const ProcessingOptions &options,
                 std::atomic_bool &cancelled,
                 std::atomic_bool &paused,
                 std::atomic_int &processed,
                 std::atomic_int &failed,
                 std::function<void(int, int)> progressCallback = nullptr)
{
    qDebug() << "[Writer] Starting.";

    int itemCount = 0;
    while (true) {
        auto data = input->pop();
        if (!data.has_value()) {
            qDebug() << "[Writer] Buffer closed, exiting.";
            break;
        }

        if (cancelled.load()) {
            qDebug() << "[Writer] Cancelled at item" << itemCount;
            break;
        }

        while (paused.load() && !cancelled.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (cancelled.load()) break;

        bool success = false;

#ifdef AUTOPHOTO_HAS_OPENCV
        if (!data->image.empty()) {
            // Apply blur
            if (options.blurFaces && !data->faces.empty()) {
                for (const cv::Rect &face : data->faces) {
                    try {
                        applyBlur(data->image, face, options.blurMode, options.strength);
                    } catch (const std::exception &e) {
                        qDebug() << "[Writer] Blur exception:" << e.what();
                    }
                }
                qDebug() << "[Writer] Applied blur to" << data->faces.size() << "faces in item" << itemCount;
            }
            
            try {
                applyTimestamp(data->image, options, data->sourcePath);
            } catch (const std::exception &e) {
                qDebug() << "[Writer] Timestamp exception:" << e.what();
            }

            QDir().mkpath(QFileInfo(data->targetPath).absolutePath());

            if (options.compressionEnabled && options.compressionLevel > 0) {
                const QString tempDir = QDir::tempPath();
                const QString tempName = QStringLiteral("autophoto_compress_%1.jpg")
                    .arg(QCryptographicHash::hash(data->targetPath.toUtf8(), QCryptographicHash::Md5).toHex().left(12));
                const QString tempPath = QDir(tempDir).filePath(tempName);
                success = cv::imwrite(tempPath.toStdString(), data->image);
                if (success) {
                    success = ImageCompressor::compress(tempPath, data->targetPath, options.compressionLevel, options.outputFormat);
                    QFile::remove(tempPath);
                }
            } else {
                success = cv::imwrite(data->targetPath.toStdString(), data->image);
            }

            data->image.release();
        } else {
            qDebug() << "[Writer] WARNING: Empty image for item" << itemCount << "- skipping.";
        }
#else
        QDir().mkpath(QFileInfo(data->targetPath).absolutePath());
        QImage image;
        if (options.rotateEnabled) {
            image = readImageWithResolvedOrientation(data->sourcePath);
        }
        if (image.isNull()) {
            image = QImage(data->sourcePath);
        }
        if (!image.isNull()) {
            if (options.compressionEnabled && options.compressionLevel > 0) {
                const QString tempDir = QDir::tempPath();
                const QString tempName = QStringLiteral("autophoto_compress_%1.jpg")
                    .arg(QCryptographicHash::hash(data->targetPath.toUtf8(), QCryptographicHash::Md5).toHex().left(12));
                const QString tempPath = QDir(tempDir).filePath(tempName);
                success = image.save(tempPath, "JPG", 95);
                if (success) {
                    success = ImageCompressor::compress(tempPath, data->targetPath, options.compressionLevel, options.outputFormat);
                    QFile::remove(tempPath);
                }
            } else {
                success = image.save(data->targetPath, nullptr, 92);
            }
        }
#endif

        if (success) {
            processed.fetch_add(1);
            qDebug() << "[Writer] Wrote" << QFileInfo(data->targetPath).fileName();
        } else {
            failed.fetch_add(1);
            qDebug() << "[Writer] FAILED to write" << data->targetPath;
        }
        
        if (itemCount % 10 == 0) {
            qDebug() << "[Writer] Available RAM:" << getAvailableRAM() / (1024 * 1024) << "MB";
        }
        ++itemCount;
        
        if (progressCallback) {
            progressCallback(processed.load(), failed.load());
        }
    }

    qDebug() << "[Writer] Done. Processed:" << processed.load() << "Failed:" << failed.load();
}

} // namespace

// ── BatchProcessor ────────────────────────────────────────────

BatchProcessor::BatchProcessor(QObject *parent)
    : QObject(parent)
{
}

BatchProcessor::~BatchProcessor()
{
    stop();
}

bool BatchProcessor::running() const { return m_running; }
bool BatchProcessor::paused() const { return m_paused; }
int BatchProcessor::progress() const { return m_progress; }
int BatchProcessor::totalImages() const { return m_totalImages; }
int BatchProcessor::processedImages() const { return m_processedImages; }
int BatchProcessor::failedImages() const { return m_failedImages; }
int BatchProcessor::workerCount() const { return m_workerCount; }
QString BatchProcessor::statusText() const { return m_statusText; }

void BatchProcessor::start(const QStringList &inputFiles,
                           const QString &outputFolder,
                           const QString &renamePattern,
                           bool rotateEnabled,
                           bool blurFaces,
                           const QString &blurMode,
                           int strength,
                           float detectionSensitivity,
                           bool sizeFilterEnabled,
                           bool skinColorFilterEnabled,
                           bool cascadeCrossCheckEnabled,
                           bool compressionEnabled,
                           int compressionLevel,
                           const QString &outputFormat,
                           bool timestampEnabled,
                           const QString &timestampCustomText,
                           const QString &timestampFormat,
                           const QString &timestampPosition,
                           const QString &timestampColor,
                           int timestampSize,
                           int timestampX,
                           int timestampY,
                           const QString &timestampFont)
{
    if (m_running) return;
    if (inputFiles.isEmpty()) { emit failed(tr("No input images selected.")); return; }
    if (outputFolder.isEmpty()) { emit failed(tr("Output folder is empty.")); return; }

    // Make a local copy so we can normalize the path
    QString outDir = outputFolder;

    qDebug() << "=== BatchProcessor::start ===";
    qDebug() << "Input files:" << inputFiles.size();
    qDebug() << "Output folder (raw):" << outDir;

    // Normalize output folder path — strip leading slash before drive letter on Windows
    // (e.g. "/F:/path" → "F:/path") to prevent cv::imwrite failures
#ifdef Q_OS_WIN
    if (outDir.length() >= 3 && outDir.at(0) == QLatin1Char('/')
        && outDir.at(2) == QLatin1Char(':')) {
        outDir = outDir.mid(1);
        qDebug() << "[BatchProcessor] Normalized output path to:" << outDir;
    }
#endif

    QDir().mkpath(outDir);

    qDebug() << "Options: blur=" << blurFaces << "rotate=" << rotateEnabled << "compress=" << compressionEnabled;
    m_cancelled = false;
    m_pausedAtomic = false;
    setProgress(0);
    setTotals(0, 0, inputFiles.size());
    setPaused(false);
    setRunning(true);
    setWorkerCount(3);
    setStatusText(tr("Starting pipeline..."));

    const ProcessingOptions options {
        rotateEnabled,
        blurFaces,
        blurMode == QLatin1String("pixelate") ? QStringLiteral("pixelate") : QStringLiteral("gaussian"),
        qBound(1, strength, 100),
        qBound(0.0f, detectionSensitivity, 1.0f),
        sizeFilterEnabled,
        skinColorFilterEnabled,
        cascadeCrossCheckEnabled,
        compressionEnabled,
        qBound(0, compressionLevel, 100),
        outputFormat,
        QStringLiteral("yunet"),
        timestampEnabled,
        timestampCustomText,
        timestampFormat,
        timestampPosition,
        timestampColor,
        timestampSize,
        timestampX,
        timestampY,
        timestampFont
    };

    quint64 availRam = getAvailableRAM();
    int bufferSize = 2; // Default
    if (availRam > 4ULL * 1024 * 1024 * 1024) {
        bufferSize = 4;
    } else if (availRam < 2ULL * 1024 * 1024 * 1024) {
        bufferSize = 1;
    }
    qDebug() << "[BatchProcessor] Available RAM:" << availRam / (1024 * 1024) << "MB, setting buffer size to:" << bufferSize;

    auto readBuffer = std::make_shared<BoundedBuffer<ReadData>>(bufferSize);
    auto detectBuffer = std::make_shared<BoundedBuffer<DetectedData>>(bufferSize);

    const int total = inputFiles.size();
    BatchProcessor *self = this;

    auto processedCount = std::make_shared<std::atomic_int>(0);
    auto failedCount = std::make_shared<std::atomic_int>(0);
    auto detectedFaces = std::make_shared<std::atomic_int>(0);

    // Launch 3-stage pipeline with error handling
    try {
        m_readerThread = std::make_unique<std::thread>([
            inputFiles, outDir, renamePattern, options,
            readBuffer, &cancelled = m_cancelled, &paused = m_pausedAtomic
        ] {
            try {
                readerStage(inputFiles, outDir, renamePattern, options, readBuffer, cancelled, paused);
            } catch (const std::exception &e) {
                qDebug() << "[Reader] FATAL:" << e.what();
            } catch (...) {
                qDebug() << "[Reader] FATAL: unknown exception";
            }
        });

        m_detectorThread = std::make_unique<std::thread>([
            readBuffer, detectBuffer, options,
            &cancelled = m_cancelled, &paused = m_pausedAtomic, detectedFaces
        ] {
            try {
                detectorStage(readBuffer, detectBuffer, options, cancelled, paused, *detectedFaces);
            } catch (const std::exception &e) {
                qDebug() << "[Detector] FATAL:" << e.what();
            } catch (...) {
                qDebug() << "[Detector] FATAL: unknown exception";
            }
        });

        m_writerThread = std::make_unique<std::thread>([
            self, detectBuffer, options, total,
            &cancelled = m_cancelled, &paused = m_pausedAtomic,
            processedCount, failedCount, detectedFaces
        ] {
            try {
                writerStage(detectBuffer, options, cancelled, paused, *processedCount, *failedCount, [self, total](int p, int f) {
                    QMetaObject::invokeMethod(self, [self, p, f, total]() {
                        self->setTotals(p, f, total);
                        self->setProgress(total > 0 ? ((p + f) * 100 / total) : 0);
                        self->setStatusText(self->tr("Processing: %1/%2").arg(p + f).arg(total));
                    }, Qt::QueuedConnection);
                });
            } catch (const std::exception &e) {
                qDebug() << "[Writer] FATAL:" << e.what();
            } catch (...) {
                qDebug() << "[Writer] FATAL: unknown exception";
            }

            // Pipeline complete — update GUI
            const bool wasCancelled = cancelled.load();
            const int p = processedCount->load();
            const int f = failedCount->load();
            qDebug() << "=== Pipeline complete ===" << "processed:" << p << "failed:" << f << "cancelled:" << wasCancelled;
            QMetaObject::invokeMethod(self, [self, wasCancelled, p, f, total, detectedFaces] {
                self->setRunning(false);
                self->setPaused(false);
                self->setProgress(total > 0 ? ((p + f) * 100 / total) : 0);
                self->setTotals(p, f, total);
                self->setWorkerCount(0);
                self->setStatusText(wasCancelled
                    ? self->tr("Stopped")
                    : self->tr("Completed: %1 exported, %2 failed (%3 faces)")
                        .arg(p).arg(f).arg(detectedFaces->load()));
                if (f > 0) {
                    emit self->failed(self->tr("%1 image(s) failed.").arg(f));
                }
                emit self->finished(wasCancelled);
            }, Qt::QueuedConnection);
        });
    } catch (const std::exception &e) {
        qDebug() << "=== Pipeline launch FAILED ===" << e.what();
        setRunning(false);
        setWorkerCount(0);
        setStatusText(tr("Failed to start pipeline"));
        emit failed(tr("Failed to start pipeline: %1").arg(QString::fromUtf8(e.what())));
        return;
    }

    setStatusText(tr("Pipeline running: 3 stages active..."));
    qDebug() << "=== Pipeline threads launched ===";
}

void BatchProcessor::pause()
{
    if (!m_running || m_paused) return;
    m_pausedAtomic = true;
    setPaused(true);
    setStatusText(tr("Paused"));
}

void BatchProcessor::resume()
{
    if (!m_running || !m_paused) return;
    m_pausedAtomic = false;
    setPaused(false);
    setStatusText(tr("Resuming"));
}

void BatchProcessor::stop()
{
    if (!m_running) return;
    m_cancelled = true;
    m_pausedAtomic = false;

    if (m_readerThread && m_readerThread->joinable()) m_readerThread->join();
    if (m_detectorThread && m_detectorThread->joinable()) m_detectorThread->join();
    if (m_writerThread && m_writerThread->joinable()) m_writerThread->join();

    m_readerThread.reset();
    m_detectorThread.reset();
    m_writerThread.reset();

    setRunning(false);
    setPaused(false);
    setWorkerCount(0);
    setStatusText(tr("Stopped"));
}

void BatchProcessor::setRunning(bool running) { if (m_running == running) return; m_running = running; emit runningChanged(); }
void BatchProcessor::setPaused(bool paused) { if (m_paused == paused) return; m_paused = paused; emit pausedChanged(); }
void BatchProcessor::setProgress(int progress) { const int c = qBound(0, progress, 100); if (m_progress == c) return; m_progress = c; emit progressChanged(); }
void BatchProcessor::setTotals(int p, int f, int t) { if (m_processedImages == p && m_failedImages == f && m_totalImages == t) return; m_processedImages = p; m_failedImages = f; m_totalImages = t; emit totalsChanged(); }
void BatchProcessor::setWorkerCount(int w) { if (m_workerCount == w) return; m_workerCount = w; emit workerCountChanged(); }
void BatchProcessor::setStatusText(const QString &s) { if (m_statusText == s) return; m_statusText = s; emit statusTextChanged(); }
