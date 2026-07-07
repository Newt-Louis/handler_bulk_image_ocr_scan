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
    Q_PROPERTY(bool rotateEnabled READ rotateEnabled WRITE setRotateEnabled NOTIFY rotateEnabledChanged)
    Q_PROPERTY(bool compressionEnabled READ compressionEnabled WRITE setCompressionEnabled NOTIFY compressionEnabledChanged)
    Q_PROPERTY(bool timestampEnabled READ timestampEnabled WRITE setTimestampEnabled NOTIFY timestampEnabledChanged)
    Q_PROPERTY(QString timestampFormat READ timestampFormat WRITE setTimestampFormat NOTIFY timestampFormatChanged)
    Q_PROPERTY(QString timestampPosition READ timestampPosition WRITE setTimestampPosition NOTIFY timestampPositionChanged)
    Q_PROPERTY(QString timestampColor READ timestampColor WRITE setTimestampColor NOTIFY timestampColorChanged)
    Q_PROPERTY(int timestampSize READ timestampSize WRITE setTimestampSize NOTIFY timestampSizeChanged)
    Q_PROPERTY(int timestampX READ timestampX WRITE setTimestampX NOTIFY timestampXChanged)
    Q_PROPERTY(int timestampY READ timestampY WRITE setTimestampY NOTIFY timestampYChanged)

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
    bool rotateEnabled() const;
    bool compressionEnabled() const;
    bool timestampEnabled() const;
    QString timestampFormat() const;
    QString timestampPosition() const;
    QString timestampColor() const;
    int timestampSize() const;
    int timestampX() const;
    int timestampY() const;

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
    void setRotateEnabled(bool enabled);
    void setCompressionEnabled(bool enabled);
    void setTimestampEnabled(bool enabled);
    void setTimestampFormat(const QString &format);
    void setTimestampPosition(const QString &position);
    void setTimestampColor(const QString &color);
    void setTimestampSize(int size);
    void setTimestampX(int x);
    void setTimestampY(int y);

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
    void rotateEnabledChanged();
    void compressionEnabledChanged();
    void timestampEnabledChanged();
    void timestampFormatChanged();
    void timestampPositionChanged();
    void timestampColorChanged();
    void timestampSizeChanged();
    void timestampXChanged();
    void timestampYChanged();
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
    float m_detectionSensitivity = 0.50f;
    bool m_sizeFilterEnabled = true;
    bool m_skinColorFilterEnabled = true;
    bool m_cascadeCrossCheckEnabled = true;
    int m_compressionLevel = 0;
    QString m_outputFormat = QStringLiteral("jpg");
    bool m_rotateEnabled = true;
    bool m_compressionEnabled = false;
    bool m_timestampEnabled = false;
    QString m_timestampFormat = QStringLiteral("yyyy-MM-dd HH:mm:ss");
    QString m_timestampPosition = QStringLiteral("BottomRight");
    QString m_timestampColor = QStringLiteral("#FFFFFF");
    int m_timestampSize = 24;
    int m_timestampX = 10;
    int m_timestampY = 10;
    quint64 m_requestId = 0;
    QTimer m_debounceTimer;
};
