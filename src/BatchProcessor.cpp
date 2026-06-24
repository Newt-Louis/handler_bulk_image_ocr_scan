#include "BatchProcessor.h"

#include "BoundedBuffer.h"
#include "ImageProcessor.h"

#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>
#include <QtGlobal>

#include <thread>

struct PipelineData {
    int index = 0;
    QString sourcePath;
    QString targetPath;
};

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
    setWorkerCount(2);
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

    auto readBuffer = std::make_shared<BoundedBuffer<PipelineData>>(2);
    auto writeBuffer = std::make_shared<BoundedBuffer<PipelineData>>(2);
    auto fileIndex = std::make_shared<std::atomic_int>(0);
    const int total = inputFiles.size();

    BatchProcessor *processor = this;

    QThread *worker = QThread::create([processor, inputFiles, outputFolder, renamePattern, options, readBuffer, writeBuffer, fileIndex, total] {
        std::atomic_int processed{0};
        std::atomic_int failed{0};

        auto updateStatus = [processor, total, &processed, &failed](const QString &msg) {
            const int p = processed.load();
            const int f = failed.load();
            const int done = p + f;
            const int pct = total > 0 ? (done * 100 / total) : 0;
            QMetaObject::invokeMethod(processor, [processor, pct, p, f, total, msg] {
                processor->setProgress(pct);
                processor->setTotals(p, f, total);
                processor->setStatusText(msg);
            }, Qt::QueuedConnection);
        };

        std::thread writerThread([processor, &writeBuffer, &processed, &failed, &options, total, &updateStatus] {
            while (true) {
                auto data = writeBuffer->pop();
                if (!data.has_value()) break;

                {
                    std::unique_lock<std::mutex> lock(processor->m_pauseMutex);
                    processor->m_pauseCondition.wait(lock, [processor] {
                        return !processor->m_pausedAtomic.load() || processor->m_cancelled.load();
                    });
                }
                if (processor->m_cancelled.load()) break;

                PipelineData &item = data.value();
                ImageProcessor processor_instance(options);
                const ProcessingResult result = processor_instance.processFile(item.sourcePath, item.targetPath);

                if (result.success) {
                    processed.fetch_add(1);
                } else {
                    failed.fetch_add(1);
                }

                const QString status = result.success
                    ? QObject::tr("Exported %1 (%2 face(s))")
                        .arg(QFileInfo(item.targetPath).fileName())
                        .arg(result.facesBlurred)
                    : QObject::tr("Failed %1: %2")
                        .arg(QFileInfo(item.sourcePath).fileName(), result.error);
                updateStatus(status);
            }
        });

        for (int i = 0; i < total; ++i) {
            if (processor->m_cancelled.load()) break;

            {
                std::unique_lock<std::mutex> lock(processor->m_pauseMutex);
                processor->m_pauseCondition.wait(lock, [processor] {
                    return !processor->m_pausedAtomic.load() || processor->m_cancelled.load();
                });
            }
            if (processor->m_cancelled.load()) break;

            const QFileInfo source(inputFiles.at(i));
            const QString baseName = renamePattern.isEmpty() ? source.completeBaseName() : renamePattern;
            const QString number = QString::number(i + 1).rightJustified(4, QLatin1Char('0'));
            const QString ext = (options.compressionEnabled && options.compressionLevel > 0 && options.outputFormat != QLatin1String("jpg"))
                ? options.outputFormat
                : (source.suffix().isEmpty() ? QStringLiteral("jpg") : source.suffix());
            const QString fileName = QStringLiteral("%1_%2.%3").arg(baseName, number, ext);

            PipelineData item;
            item.index = i;
            item.sourcePath = source.absoluteFilePath();
            item.targetPath = QDir(outputFolder).filePath(fileName);

            readBuffer->push(item);

            updateStatus(tr("Queued %1...").arg(source.fileName()));
        }

        readBuffer->close();

        writerThread.join();

        const bool cancelled = processor->m_cancelled.load();
        const int processedCount = processed.load();
        const int failedCount = failed.load();
        QMetaObject::invokeMethod(processor, [processor, cancelled, processedCount, failedCount, total] {
            processor->setRunning(false);
            processor->setPaused(false);
            processor->setProgress(total > 0 ? ((processedCount + failedCount) * 100 / total) : 0);
            processor->setTotals(processedCount, failedCount, total);
            processor->setWorkerCount(0);
            processor->setStatusText(cancelled
                ? processor->tr("Stopped")
                : processor->tr("Completed: %1 exported, %2 failed").arg(processedCount).arg(failedCount));
            if (failedCount > 0) {
                emit processor->failed(processor->tr("%1 image(s) failed.").arg(failedCount));
            }
            emit processor->finished(cancelled);
        }, Qt::QueuedConnection);
    });

    m_worker = worker;
    connect(worker, &QThread::finished, this, [this] {
        m_worker = nullptr;
        if (!m_running) setWorkerCount(0);
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
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
    { std::lock_guard<std::mutex> lock(m_pauseMutex); m_pausedAtomic = false; setPaused(false); }
    m_pauseCondition.notify_all();
    setStatusText(tr("Resuming"));
}

void BatchProcessor::stop()
{
    if (!m_running) return;
    m_cancelled = true;
    { std::lock_guard<std::mutex> lock(m_pauseMutex); m_pausedAtomic = false; setPaused(false); }
    m_pauseCondition.notify_all();
}

void BatchProcessor::setRunning(bool running) { if (m_running == running) return; m_running = running; emit runningChanged(); }
void BatchProcessor::setPaused(bool paused) { if (m_paused == paused) return; m_paused = paused; emit pausedChanged(); }
void BatchProcessor::setProgress(int progress) { const int c = qBound(0, progress, 100); if (m_progress == c) return; m_progress = c; emit progressChanged(); }
void BatchProcessor::setTotals(int p, int f, int t) { if (m_processedImages == p && m_failedImages == f && m_totalImages == t) return; m_processedImages = p; m_failedImages = f; m_totalImages = t; emit totalsChanged(); }
void BatchProcessor::setWorkerCount(int w) { if (m_workerCount == w) return; m_workerCount = w; emit workerCountChanged(); }
void BatchProcessor::setStatusText(const QString &s) { if (m_statusText == s) return; m_statusText = s; emit statusTextChanged(); }
