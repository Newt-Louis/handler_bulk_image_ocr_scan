#include "PreviewController.h"

#include "ImageProcessor.h"
#include "ImageOrientation.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMetaObject>
#include <QPainter>
#include <QPointer>
#include <QStandardPaths>
#include <QThread>
#include <QtGlobal>

namespace {

QImage scaleRegion(const QImage &source, const QRect &region, int divisor, Qt::TransformationMode mode)
{
    if (!region.isValid() || region.isEmpty()) {
        return source;
    }

    QImage result = source;
    const QImage copy = source.copy(region);
    const QSize smallSize(qMax(1, region.width() / divisor), qMax(1, region.height() / divisor));
    const QImage small = copy.scaled(smallSize, Qt::IgnoreAspectRatio, mode);
    const QImage restored = small.scaled(region.size(), Qt::IgnoreAspectRatio, mode);

    QPainter painter(&result);
    painter.drawImage(region.topLeft(), restored);
    return result;
}

QImage applyPreviewEffect(const QImage &image, bool enabled, const QString &mode, int strength)
{
    if (!enabled || image.isNull()) {
        return image;
    }

    const int side = qMin(image.width(), image.height());
    const int regionWidth = qMax(96, static_cast<int>(side * 0.34));
    const int regionHeight = qMax(96, static_cast<int>(side * 0.28));
    const QRect region((image.width() - regionWidth) / 2,
                       (image.height() - regionHeight) / 2,
                       regionWidth,
                       regionHeight);

    const int clampedStrength = qBound(1, strength, 100);
    if (mode == QLatin1String("pixelate")) {
        const int divisor = qBound(4, clampedStrength / 4, 28);
        return scaleRegion(image, region, divisor, Qt::FastTransformation);
    }

    const int divisor = qBound(3, clampedStrength / 6, 18);
    return scaleRegion(image, region, divisor, Qt::SmoothTransformation);
}

} // namespace

PreviewController::PreviewController(QObject *parent)
    : QObject(parent)
{
}

QUrl PreviewController::previewUrl() const
{
    return m_previewUrl;
}

bool PreviewController::busy() const
{
    return m_busy;
}

bool PreviewController::blurFaces() const
{
    return m_blurFaces;
}

QString PreviewController::blurMode() const
{
    return m_blurMode;
}

int PreviewController::strength() const
{
    return m_strength;
}

void PreviewController::requestPreview(const QString &filePath)
{
    if (m_sourcePath == filePath && !m_previewUrl.isEmpty()) {
        return;
    }

    m_sourcePath = filePath;
    regenerate();
}

void PreviewController::clear()
{
    m_sourcePath.clear();
    setPreviewUrl({});
    setBusy(false);
}

void PreviewController::setBlurFaces(bool enabled)
{
    if (m_blurFaces == enabled) {
        return;
    }

    m_blurFaces = enabled;
    emit blurFacesChanged();
    regenerate();
}

void PreviewController::setBlurMode(const QString &mode)
{
    const QString normalized = mode == QLatin1String("pixelate") ? QStringLiteral("pixelate")
                                                                 : QStringLiteral("gaussian");
    if (m_blurMode == normalized) {
        return;
    }

    m_blurMode = normalized;
    emit blurModeChanged();
    regenerate();
}

void PreviewController::setStrength(int strength)
{
    const int clamped = qBound(1, strength, 100);
    if (m_strength == clamped) {
        return;
    }

    m_strength = clamped;
    emit strengthChanged();
    regenerate();
}

void PreviewController::regenerate()
{
    if (m_sourcePath.isEmpty()) {
        return;
    }

    const QString sourcePath = m_sourcePath;
    const QString mode = m_blurMode;
    const bool blurFaces = m_blurFaces;
    const int strength = m_strength;
    const quint64 requestId = ++m_requestId;
    const QString targetPath = cacheFilePath(sourcePath, requestId);
    const QPointer<PreviewController> guard(this);

    setBusy(true);

    QThread *worker = QThread::create([sourcePath, mode, blurFaces, strength, targetPath, guard, requestId] {
#ifdef AUTOPHOTO_HAS_OPENCV
        ImageProcessor processor({blurFaces, mode, strength, QStringLiteral("yunet")});
        const ProcessingResult result = processor.processFile(sourcePath, targetPath);
        if (!guard) {
            return;
        }
        QObject *target = guard.data();
        if (!target) {
            return;
        }
        QMetaObject::invokeMethod(target, [guard, requestId, targetPath, result] {
            PreviewController *self = guard.data();
            if (!self || requestId != self->m_requestId) {
                return;
            }
            self->setBusy(false);
            if (!result.success) {
                emit self->previewFailed(result.error);
                return;
            }
            self->setPreviewUrl(QUrl::fromLocalFile(targetPath));
        }, Qt::QueuedConnection);
#else
        QImage image = readImageWithResolvedOrientation(sourcePath);
        if (!image.isNull()) {
            image = image.scaled(QSize(1800, 1800), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        if (image.isNull()) {
            if (!guard) {
                return;
            }
            QObject *target = guard.data();
            if (!target) {
                return;
            }
            QMetaObject::invokeMethod(target, [guard, requestId, sourcePath] {
                PreviewController *self = guard.data();
                if (!self || requestId != self->m_requestId) {
                    return;
                }
                self->setBusy(false);
                emit self->previewFailed(self->tr("Could not load image: %1").arg(sourcePath));
            }, Qt::QueuedConnection);
            return;
        }

        image = applyPreviewEffect(image, blurFaces, mode, strength);

        QDir().mkpath(QFileInfo(targetPath).absolutePath());
        const bool saved = image.save(targetPath, "JPG", 92);
        if (!guard) {
            return;
        }
        QObject *target = guard.data();
        if (!target) {
            return;
        }
        QMetaObject::invokeMethod(target, [guard, requestId, targetPath, saved] {
            PreviewController *self = guard.data();
            if (!self || requestId != self->m_requestId) {
                return;
            }
            self->setBusy(false);
            if (!saved) {
                emit self->previewFailed(self->tr("Could not save preview."));
                return;
            }
            self->setPreviewUrl(QUrl::fromLocalFile(targetPath));
        }, Qt::QueuedConnection);
#endif
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void PreviewController::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void PreviewController::setPreviewUrl(const QUrl &url)
{
    if (m_previewUrl == url) {
        return;
    }

    m_previewUrl = url;
    emit previewUrlChanged();
}

QString PreviewController::cacheFilePath(const QString &sourcePath, quint64 requestId) const
{
    QString cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheRoot.isEmpty()) {
        cacheRoot = QDir::tempPath() + QStringLiteral("/autophoto");
    }

    const QByteArray hash = QCryptographicHash::hash(
        QStringLiteral("%1:%2:%3:%4:%5")
            .arg(sourcePath, m_blurMode)
            .arg(m_strength)
            .arg(m_blurFaces ? 1 : 0)
            .arg(QStringLiteral("resolved-orient-v3-privacy-mask"))
            .toUtf8(),
        QCryptographicHash::Sha1
    ).toHex();

    return QDir(cacheRoot).filePath(QStringLiteral("preview-%1.jpg").arg(QString::fromLatin1(hash)));
}
