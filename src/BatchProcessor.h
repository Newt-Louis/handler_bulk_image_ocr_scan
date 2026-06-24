#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QThread>
#include <atomic>
#include <condition_variable>
#include <mutex>

class BatchProcessor final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int totalImages READ totalImages NOTIFY totalsChanged)
    Q_PROPERTY(int processedImages READ processedImages NOTIFY totalsChanged)
    Q_PROPERTY(int failedImages READ failedImages NOTIFY totalsChanged)
    Q_PROPERTY(int workerCount READ workerCount NOTIFY workerCountChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit BatchProcessor(QObject *parent = nullptr);
    ~BatchProcessor() override;

    bool running() const;
    bool paused() const;
    int progress() const;
    int totalImages() const;
    int processedImages() const;
    int failedImages() const;
    int workerCount() const;
    QString statusText() const;

    Q_INVOKABLE void start(const QStringList &inputFiles,
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
                           const QString &outputFormat);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();

signals:
    void runningChanged();
    void pausedChanged();
    void progressChanged();
    void totalsChanged();
    void workerCountChanged();
    void statusTextChanged();
    void finished(bool cancelled);
    void failed(const QString &message);

private:
    void setRunning(bool running);
    void setPaused(bool paused);
    void setProgress(int progress);
    void setTotals(int processedImages, int failedImages, int totalImages);
    void setWorkerCount(int workerCount);
    void setStatusText(const QString &statusText);

    std::atomic_bool m_cancelled = false;
    std::atomic_bool m_pausedAtomic = false;
    QPointer<QThread> m_worker;
    bool m_running = false;
    bool m_paused = false;
    int m_progress = 0;
    int m_totalImages = 0;
    int m_processedImages = 0;
    int m_failedImages = 0;
    int m_workerCount = 0;
    QString m_statusText = QStringLiteral("Idle");
    std::mutex m_pauseMutex;
    std::condition_variable m_pauseCondition;
};
