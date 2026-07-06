#include "BatchProcessor.h"

#include "BoundedBuffer.h"
#include "ImageCompressor.h"
#include "ImageOrientation.h"
#include "ImageProcessor.h"
#include "ImageProcessorInternal.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QtGlobal>

#include <thread>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

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
// Reads images from disk, pushes to detector buffer.
// Runs on a dedicated thread. Yields CPU between reads for HDD friendliness.

void readerStage(const QStringList &inputFiles,
                 const QString &outputFolder,
                 const QString &renamePattern,
                 const ProcessingOptions &options,
                 std::shared_ptr<BoundedBuffer<ReadData>> output,
                 std::atomic_bool &cancelled,
                 std::atomic_bool &paused)
{
    const int total = inputFiles.size();

    for (int i = 0; i < total; ++i) {
        if (cancelled.load()) break;

        // Pause spin-wait
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
        // Read image with EXIF orientation if rotate enabled
        if (options.rotateEnabled) {
            item.image = readImageRespectingExif(item.sourcePath);
        }
        if (item.image.empty()) {
            item.image = cv::imread(item.sourcePath.toStdString(), cv::IMREAD_COLOR);
        }
#endif

        output->push(std::move(item));

        // Yield CPU after each read — important for HDD + low-resource machines
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    output->close();
}

// ── Detector Stage ────────────────────────────────────────────
// Pops from reader buffer, detects faces, pushes to writer buffer.
// Heavy CPU work — yields between items.

void detectorStage(std::shared_ptr<BoundedBuffer<ReadData>> input,
                   std::shared_ptr<BoundedBuffer<DetectedData>> output,
                   const ProcessingOptions &options,
                   std::atomic_bool &cancelled,
                   std::atomic_bool &paused,
                   std::atomic_int &detectedCount)
{
#ifdef AUTOPHOTO_HAS_OPENCV
    const QString yuNetModelPath = findYuNetModelPath();

    while (true) {
        auto data = input->pop();
        if (!data.has_value()) break;

        if (cancelled.load()) break;

        // Pause spin-wait
        while (paused.load() && !cancelled.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (cancelled.load()) break;

        DetectedData out;
        out.index = data->index;
        out.sourcePath = std::move(data->sourcePath);
        out.targetPath = std::move(data->targetPath);
        out.image = std::move(data->image);

        // Detect faces if blur is enabled
        if (options.blurFaces && !out.image.empty()) {
            std::vector<cv::Rect> faces;
            std::vector<cv::Rect> yuNetFaces;

            if (!yuNetModelPath.isEmpty()) {
                try {
                    faces = detectWithYuNet(out.image, yuNetModelPath, options.detectionSensitivity, out.sourcePath);
                    yuNetFaces = faces;
                } catch (const cv::Exception &) {
                    // YuNet failed — continue with empty faces
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

                    std::vector<cv::Rect> cascadeDetections = detectWithCascade(out.image, cascadePath, false);
                    appendMergedFaces(faces, cascadeDetections);

                    if (cascadeFile == QLatin1String("haarcascade_profileface.xml")) {
                        cascadeDetections = detectWithCascade(out.image, cascadePath, true);
                        appendMergedFaces(faces, cascadeDetections);
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
            if (options.skinColorFilterEnabled) {
                faces = filterBySkinColor(out.image, faces, options.detectionSensitivity);
                // Aggregate validation
                if (!faces.empty()) {
                    double totalSkinRatio = 0.0;
                    int validRoiCount = 0;
                    for (const cv::Rect &face : faces) {
                        cv::Rect expanded = face;
                        const int padX = static_cast<int>(expanded.width * 0.3);
                        const int padY = static_cast<int>(expanded.height * 0.3);
                        expanded.x = std::max(0, expanded.x - padX);
                        expanded.y = std::max(0, expanded.y - padY);
                        expanded.width = std::min(out.image.cols - expanded.x, expanded.width + padX * 2);
                        expanded.height = std::min(out.image.rows - expanded.y, expanded.height + padY * 2);
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
                        faces.clear();
                    }
                }
            }

            out.faces = std::move(faces);
            detectedCount.fetch_add(static_cast<int>(out.faces.size()));
        }

        output->push(std::move(out));

        // Yield CPU after detection (heavy work)
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
#else
    // No OpenCV — pass through without detection
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
}

// ── Writer Stage ──────────────────────────────────────────────
// Pops from detector buffer, applies blur + compression, writes to disk.

void writerStage(std::shared_ptr<BoundedBuffer<DetectedData>> input,
                 const ProcessingOptions &options,
                 std::atomic_bool &cancelled,
                 std::atomic_bool &paused,
                 std::atomic_int &processed,
                 std::atomic_int &failed)
{
    while (true) {
        auto data = input->pop();
        if (!data.has_value()) break;

        if (cancelled.load()) break;

        // Pause spin-wait
        while (paused.load() && !cancelled.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (cancelled.load()) break;

        bool success = false;

#ifdef AUTOPHOTO_HAS_OPENCV
        if (!data->image.empty()) {
            // Apply blur to detected faces
            if (options.blurFaces) {
                for (const cv::Rect &face : data->faces) {
                    applyBlur(data->image, face, options.blurMode, options.strength);
                }
            }

            // Ensure output directory exists
            QDir().mkpath(QFileInfo(data->targetPath).absolutePath());

            // Write image
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

            // Release memory immediately after writing
            data->image.release();
        }
#else
        // No OpenCV — Qt fallback
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
        } else {
            failed.fetch_add(1);
        }
    }
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
                           const QString &outputFormat)
{
    if (m_running) return;
    if (inputFiles.isEmpty()) { emit failed(tr("No input images selected.")); return; }
    if (outputFolder.isEmpty()) { emit failed(tr("Output folder is empty.")); return; }

    QDir().mkpath(outputFolder);
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
        outputFormat
    };

    // Create bounded buffers (capacity 1 each for minimal RAM usage)
    auto readBuffer = std::make_shared<BoundedBuffer<ReadData>>(1);
    auto detectBuffer = std::make_shared<BoundedBuffer<DetectedData>>(1);

    const int total = inputFiles.size();
    BatchProcessor *self = this;

    // Shared atomic counters
    auto processedCount = std::make_shared<std::atomic_int>(0);
    auto failedCount = std::make_shared<std::atomic_int>(0);
    auto detectedFaces = std::make_shared<std::atomic_int>(0);

    // Status update helper
    auto updateStatus = [self, total, processedCount, failedCount](const QString &msg) {
        const int p = processedCount->load();
        const int f = failedCount->load();
        const int done = p + f;
        const int pct = total > 0 ? (done * 100 / total) : 0;
        QMetaObject::invokeMethod(self, [self, pct, p, f, total, msg] {
            self->setProgress(pct);
            self->setTotals(p, f, total);
            self->setStatusText(msg);
        }, Qt::QueuedConnection);
    };

    // ── Launch 3-stage pipeline ──────────────────────────────

    // Stage 1: Reader (HDD I/O — yields CPU between reads)
    m_readerThread = std::make_unique<std::thread>([
        inputFiles, outputFolder, renamePattern, options,
        readBuffer, &cancelled = m_cancelled, &paused = m_pausedAtomic
    ] {
        readerStage(inputFiles, outputFolder, renamePattern, options, readBuffer, cancelled, paused);
    });

    // Stage 2: Detector (CPU heavy — YuNet + cascade + filters)
    m_detectorThread = std::make_unique<std::thread>([
        readBuffer, detectBuffer, options,
        &cancelled = m_cancelled, &paused = m_pausedAtomic, detectedFaces
    ] {
        detectorStage(readBuffer, detectBuffer, options, cancelled, paused, *detectedFaces);
    });

    // Stage 3: Writer (blur + compress + disk write)
    m_writerThread = std::make_unique<std::thread>([
        self, detectBuffer, options, total,
        &cancelled = m_cancelled, &paused = m_pausedAtomic,
        processedCount, failedCount, detectedFaces, updateStatus
    ] {
        writerStage(detectBuffer, options, cancelled, paused, *processedCount, *failedCount);

        // Pipeline complete — update GUI
        const bool wasCancelled = cancelled.load();
        const int p = processedCount->load();
        const int f = failedCount->load();
        QMetaObject::invokeMethod(self, [self, wasCancelled, p, f, total, detectedFaces] {
            self->setRunning(false);
            self->setPaused(false);
            self->setProgress(total > 0 ? ((p + f) * 100 / total) : 0);
            self->setTotals(p, f, total);
            self->setWorkerCount(0);
            self->setStatusText(wasCancelled
                ? self->tr("Stopped")
                : self->tr("Completed: %1 exported, %2 failed (%3 faces detected)")
                    .arg(p).arg(f).arg(detectedFaces->load()));
            if (f > 0) {
                emit self->failed(self->tr("%1 image(s) failed.").arg(f));
            }
            emit self->finished(wasCancelled);
        }, Qt::QueuedConnection);
    });

    setStatusText(tr("Pipeline running: 3 stages active..."));
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

    // Join all pipeline threads
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
