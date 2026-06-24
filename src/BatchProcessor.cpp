#include "BatchProcessor.h"

#include "ImageProcessor.h"

#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>
#include <QtGlobal>

#include <algorithm>
#include <thread>
#include <vector>

BatchProcessor::BatchProcessor(QObject *parent)
    : QObject(parent)
{
}

BatchProcessor::~BatchProcessor()
{
    stop();
    if (m_worker) {
        m_worker->wait();
    }
}

bool BatchProcessor::running() const
{
    return m_running;
}

bool BatchProcessor::paused() const
{
    return m_paused;
}

int BatchProcessor::progress() const
{
    return m_progress;
}

int BatchProcessor::totalImages() const
{
    return m_totalImages;
}

int BatchProcessor::processedImages() const
{
    return m_processedImages;
}

int BatchProcessor::failedImages() const
{
    return m_failedImages;
}

int BatchProcessor::workerCount() const
{
    return m_workerCount;
}

QString BatchProcessor::statusText() const
{
    return m_statusText;
}

void BatchProcessor::start(const QStringList &inputFiles,
                           const QString &outputFolder,
                           const QString &renamePattern,
                           bool blurFaces,
                           const QString &blurMode,
                           int strength,
                           float detectionSensitivity,
                           bool sizeFilterEnabled,
                           bool skinColorFilterEnabled,
                           bool cascadeCrossCheckEnabled,
                           int compressionLevel,
                           const QString &outputFormat)
{
    if (m_running) {
        return;
    }
    if (inputFiles.isEmpty()) {
        emit failed(tr("No input images selected."));
        return;
    }
    if (outputFolder.isEmpty()) {
        emit failed(tr("Output folder is empty."));
        return;
    }

    QDir().mkpath(outputFolder);
    m_cancelled = false;
    m_pausedAtomic = false;
    setProgress(0);
    setTotals(0, 0, inputFiles.size());
    setPaused(false);
    setRunning(true);
    const unsigned int hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
    const int workerCount = std::max(1, std::min({static_cast<int>(inputFiles.size()), static_cast<int>(hardwareThreads), 2}));
    setWorkerCount(workerCount);
    setStatusText(tr("Starting %1 worker(s)").arg(workerCount));

    const ProcessingOptions options {
        blurFaces,
        blurMode == QLatin1String("pixelate") ? QStringLiteral("pixelate") : QStringLiteral("gaussian"),
        qBound(1, strength, 100),
        qBound(0.0f, detectionSensitivity, 1.0f),
        sizeFilterEnabled,
        skinColorFilterEnabled,
        cascadeCrossCheckEnabled,
        qBound(0, compressionLevel, 100),
        outputFormat
    };

    BatchProcessor *processor = this;
    QThread *worker = QThread::create([processor, inputFiles, outputFolder, renamePattern, options, workerCount] {
        const int total = inputFiles.size();
        std::atomic_int nextIndex = 0;
        std::atomic_int processed = 0;
        std::atomic_int failed = 0;

        auto updateStatus = [processor, total](int processedCount, int failedCount, const QString &message) {
            const int done = processedCount + failedCount;
            const int nextProgress = total > 0 ? static_cast<int>((done * 100.0) / total) : 0;
            QMetaObject::invokeMethod(processor, [processor, nextProgress, processedCount, failedCount, total, message] {
                processor->setProgress(nextProgress);
                processor->setTotals(processedCount, failedCount, total);
                processor->setStatusText(message);
            }, Qt::QueuedConnection);
        };

        std::vector<std::thread> workers;
        workers.reserve(static_cast<size_t>(workerCount));

        for (int workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
            workers.emplace_back([&, workerIndex] {
                ImageProcessor imageProcessor(options);

                while (!processor->m_cancelled.load()) {
                    {
                        std::unique_lock<std::mutex> lock(processor->m_pauseMutex);
                        processor->m_pauseCondition.wait(lock, [processor] {
                            return !processor->m_pausedAtomic.load() || processor->m_cancelled.load();
                        });
                    }

                    if (processor->m_cancelled.load()) {
                        break;
                    }

                    const int i = nextIndex.fetch_add(1);
                    if (i >= total) {
                        break;
                    }

                    const QFileInfo source(inputFiles.at(i));
                    const QString baseName = renamePattern.isEmpty()
                        ? source.completeBaseName()
                        : renamePattern;
                    const QString number = QString::number(i + 1).rightJustified(4, QLatin1Char('0'));
                    const QString ext = (options.compressionLevel > 0 && options.outputFormat != QLatin1String("jpg"))
                        ? options.outputFormat
                        : (source.suffix().isEmpty() ? QStringLiteral("jpg") : source.suffix());
                    const QString fileName = QStringLiteral("%1_%2.%3")
                        .arg(baseName, number, ext);
                    const QString targetPath = QDir(outputFolder).filePath(fileName);

                    const ProcessingResult result = imageProcessor.processFile(source.absoluteFilePath(), targetPath);
                    const int processedCount = result.success ? processed.fetch_add(1) + 1 : processed.load();
                    const int failedCount = result.success ? failed.load() : failed.fetch_add(1) + 1;

                    const QString status = result.success
                        ? QObject::tr("Processed %1 (%2 face(s), worker %3)")
                            .arg(fileName)
                            .arg(result.facesBlurred)
                            .arg(workerIndex + 1)
                        : QObject::tr("Skipped %1: %2")
                            .arg(source.fileName(), result.error);
                    updateStatus(processedCount, failedCount, status);

                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            });
        }

        for (std::thread &thread : workers) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        const bool cancelled = processor->m_cancelled.load();
        const int processedCount = processed.load();
        const int failedCount = failed.load();
        QMetaObject::invokeMethod(processor, [processor, cancelled, processedCount, failedCount, total] {
            processor->setRunning(false);
            processor->setPaused(false);
            processor->setProgress(total > 0 ? static_cast<int>(((processedCount + failedCount) * 100.0) / total) : 0);
            processor->setTotals(processedCount, failedCount, total);
            processor->setWorkerCount(0);
            processor->setStatusText(cancelled
                ? processor->tr("Stopped")
                : processor->tr("Completed: %1 exported, %2 failed").arg(processedCount).arg(failedCount));
            if (failedCount > 0) {
                emit processor->failed(processor->tr("%1 image(s) failed. Successful images were still exported.").arg(failedCount));
            }
            emit processor->finished(cancelled);
        }, Qt::QueuedConnection);
    });

    m_worker = worker;
    connect(worker, &QThread::finished, this, [this] {
        m_worker = nullptr;
        if (!m_running) {
            setWorkerCount(0);
        }
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void BatchProcessor::pause()
{
    if (!m_running || m_paused) {
        return;
    }
    m_pausedAtomic = true;
    setPaused(true);
    setStatusText(tr("Paused"));
}

void BatchProcessor::resume()
{
    if (!m_running || !m_paused) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_pauseMutex);
        m_pausedAtomic = false;
        setPaused(false);
    }
    m_pauseCondition.notify_all();
    setStatusText(tr("Resuming"));
}

void BatchProcessor::stop()
{
    if (!m_running) {
        return;
    }

    m_cancelled = true;
    {
        std::lock_guard<std::mutex> lock(m_pauseMutex);
        m_pausedAtomic = false;
        setPaused(false);
    }
    m_pauseCondition.notify_all();
}

void BatchProcessor::setRunning(bool running)
{
    if (m_running == running) {
        return;
    }
    m_running = running;
    emit runningChanged();
}

void BatchProcessor::setPaused(bool paused)
{
    if (m_paused == paused) {
        return;
    }
    m_paused = paused;
    emit pausedChanged();
}

void BatchProcessor::setProgress(int progress)
{
    const int clamped = qBound(0, progress, 100);
    if (m_progress == clamped) {
        return;
    }
    m_progress = clamped;
    emit progressChanged();
}

void BatchProcessor::setTotals(int processedImages, int failedImages, int totalImages)
{
    if (m_processedImages == processedImages && m_failedImages == failedImages && m_totalImages == totalImages) {
        return;
    }
    m_processedImages = processedImages;
    m_failedImages = failedImages;
    m_totalImages = totalImages;
    emit totalsChanged();
}

void BatchProcessor::setWorkerCount(int workerCount)
{
    if (m_workerCount == workerCount) {
        return;
    }
    m_workerCount = workerCount;
    emit workerCountChanged();
}

void BatchProcessor::setStatusText(const QString &statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}
