#pragma once

#include <QObject>
#include <QTimer>
#include <QUrl>

class PreviewController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl previewUrl READ previewUrl NOTIFY previewUrlChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool blurFaces READ blurFaces WRITE setBlurFaces NOTIFY blurFacesChanged)
    Q_PROPERTY(QString blurMode READ blurMode WRITE setBlurMode NOTIFY blurModeChanged)
    Q_PROPERTY(int strength READ strength WRITE setStrength NOTIFY strengthChanged)
    Q_PROPERTY(float detectionSensitivity READ detectionSensitivity WRITE setDetectionSensitivity NOTIFY detectionSensitivityChanged)
    Q_PROPERTY(bool sizeFilterEnabled READ sizeFilterEnabled WRITE setSizeFilterEnabled NOTIFY sizeFilterEnabledChanged)
    Q_PROPERTY(bool skinColorFilterEnabled READ skinColorFilterEnabled WRITE setSkinColorFilterEnabled NOTIFY skinColorFilterEnabledChanged)
    Q_PROPERTY(bool cascadeCrossCheckEnabled READ cascadeCrossCheckEnabled WRITE setCascadeCrossCheckEnabled NOTIFY cascadeCrossCheckEnabledChanged)
    Q_PROPERTY(int compressionLevel READ compressionLevel WRITE setCompressionLevel NOTIFY compressionLevelChanged)
    Q_PROPERTY(QString outputFormat READ outputFormat WRITE setOutputFormat NOTIFY outputFormatChanged)

public:
    explicit PreviewController(QObject *parent = nullptr);

    QUrl previewUrl() const;
    bool busy() const;
    bool blurFaces() const;
    QString blurMode() const;
    int strength() const;
    float detectionSensitivity() const;
    bool sizeFilterEnabled() const;
    bool skinColorFilterEnabled() const;
    bool cascadeCrossCheckEnabled() const;
    int compressionLevel() const;
    QString outputFormat() const;

    Q_INVOKABLE void requestPreview(const QString &filePath);
    Q_INVOKABLE void clear();

public slots:
    void setBlurFaces(bool enabled);
    void setBlurMode(const QString &mode);
    void setStrength(int strength);
    void setDetectionSensitivity(float sensitivity);
    void setSizeFilterEnabled(bool enabled);
    void setSkinColorFilterEnabled(bool enabled);
    void setCascadeCrossCheckEnabled(bool enabled);
    void setCompressionLevel(int level);
    void setOutputFormat(const QString &format);

signals:
    void previewUrlChanged();
    void busyChanged();
    void blurFacesChanged();
    void blurModeChanged();
    void strengthChanged();
    void detectionSensitivityChanged();
    void sizeFilterEnabledChanged();
    void skinColorFilterEnabledChanged();
    void cascadeCrossCheckEnabledChanged();
    void compressionLevelChanged();
    void outputFormatChanged();
    void previewFailed(const QString &message);

private:
    void regenerate();
    void regenerateFast();
    void setBusy(bool busy);
    void setPreviewUrl(const QUrl &url);
    QString cacheFilePath(const QString &sourcePath, quint64 requestId) const;
    QString fastCacheFilePath(const QString &sourcePath) const;

    QUrl m_previewUrl;
    QString m_sourcePath;
    QString m_blurMode = QStringLiteral("gaussian");
    bool m_busy = false;
    bool m_blurFaces = true;
    int m_strength = 100;
    float m_detectionSensitivity = 0.35f;
    bool m_sizeFilterEnabled = true;
    bool m_skinColorFilterEnabled = true;
    bool m_cascadeCrossCheckEnabled = true;
    int m_compressionLevel = 0;
    QString m_outputFormat = QStringLiteral("jpg");
    quint64 m_requestId = 0;
    QTimer m_debounceTimer;
};
