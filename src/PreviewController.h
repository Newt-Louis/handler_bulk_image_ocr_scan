#pragma once

#include <QObject>
#include <QUrl>

class PreviewController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl previewUrl READ previewUrl NOTIFY previewUrlChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool blurFaces READ blurFaces WRITE setBlurFaces NOTIFY blurFacesChanged)
    Q_PROPERTY(QString blurMode READ blurMode WRITE setBlurMode NOTIFY blurModeChanged)
    Q_PROPERTY(int strength READ strength WRITE setStrength NOTIFY strengthChanged)

public:
    explicit PreviewController(QObject *parent = nullptr);

    QUrl previewUrl() const;
    bool busy() const;
    bool blurFaces() const;
    QString blurMode() const;
    int strength() const;

    Q_INVOKABLE void requestPreview(const QString &filePath);
    Q_INVOKABLE void clear();

public slots:
    void setBlurFaces(bool enabled);
    void setBlurMode(const QString &mode);
    void setStrength(int strength);

signals:
    void previewUrlChanged();
    void busyChanged();
    void blurFacesChanged();
    void blurModeChanged();
    void strengthChanged();
    void previewFailed(const QString &message);

private:
    void regenerate();
    void setBusy(bool busy);
    void setPreviewUrl(const QUrl &url);
    QString cacheFilePath(const QString &sourcePath, quint64 requestId) const;

    QUrl m_previewUrl;
    QString m_sourcePath;
    QString m_blurMode = QStringLiteral("gaussian");
    bool m_busy = false;
    bool m_blurFaces = true;
    int m_strength = 100;
    quint64 m_requestId = 0;
};
