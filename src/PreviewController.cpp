#include "PreviewController.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

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
#include <QThreadPool>
#include <QRunnable>
#include <QtGlobal>
#include <algorithm>

#ifdef AUTOPHOTO_HAS_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

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
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(150);
    connect(&m_debounceTimer, &QTimer::timeout, this, &PreviewController::regenerate);
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

float PreviewController::detectionSensitivity() const
{
    return m_detectionSensitivity;
}

bool PreviewController::sizeFilterEnabled() const
{
    return m_sizeFilterEnabled;
}

bool PreviewController::skinColorFilterEnabled() const
{
    return m_skinColorFilterEnabled;
}

bool PreviewController::cascadeCrossCheckEnabled() const
{
    return m_cascadeCrossCheckEnabled;
}

int PreviewController::compressionLevel() const
{
    return m_compressionLevel;
}

QString PreviewController::outputFormat() const
{
    return m_outputFormat;
}

bool PreviewController::rotateEnabled() const
{
    return m_rotateEnabled;
}

bool PreviewController::compressionEnabled() const
{
    return m_compressionEnabled;
}

bool PreviewController::timestampEnabled() const { return m_timestampEnabled; }
QString PreviewController::timestampCustomText() const { return m_timestampCustomText; }
QString PreviewController::timestampFormat() const { return m_timestampFormat; }

void PreviewController::setTimestampCustomText(const QString &text) {
    if (m_timestampCustomText == text) return;
    m_timestampCustomText = text;
    emit timestampCustomTextChanged();
    m_debounceTimer.start();
}
void PreviewController::setTimestampFormat(const QString &format) {
    if (m_timestampFormat == format) return;
    m_timestampFormat = format;
    emit timestampFormatChanged();
    m_debounceTimer.start();
}
QString PreviewController::timestampPosition() const { return m_timestampPosition; }
QString PreviewController::timestampColor() const { return m_timestampColor; }
int PreviewController::timestampSize() const { return m_timestampSize; }
int PreviewController::timestampX() const { return m_timestampX; }
int PreviewController::timestampY() const { return m_timestampY; }

void PreviewController::requestPreview(const QString &filePath)
{
    if (m_sourcePath == filePath && !m_previewUrl.isEmpty()) {
        return;
    }

    m_sourcePath = filePath;
    regenerateFast();
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
    m_debounceTimer.start();
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
    m_debounceTimer.start();
}

void PreviewController::setStrength(int strength)
{
    const int clamped = qBound(1, strength, 100);
    if (m_strength == clamped) {
        return;
    }

    m_strength = clamped;
    emit strengthChanged();
    m_debounceTimer.start();
}

void PreviewController::setDetectionSensitivity(float sensitivity)
{
    const float clamped = qBound(0.0f, sensitivity, 1.0f);
    if (qFuzzyCompare(m_detectionSensitivity, clamped)) {
        return;
    }

    m_detectionSensitivity = clamped;
    emit detectionSensitivityChanged();
    m_debounceTimer.start();
}

void PreviewController::setSizeFilterEnabled(bool enabled)
{
    if (m_sizeFilterEnabled == enabled) {
        return;
    }

    m_sizeFilterEnabled = enabled;
    emit sizeFilterEnabledChanged();
    m_debounceTimer.start();
}

void PreviewController::setSkinColorFilterEnabled(bool enabled)
{
    if (m_skinColorFilterEnabled == enabled) {
        return;
    }

    m_skinColorFilterEnabled = enabled;
    emit skinColorFilterEnabledChanged();
    m_debounceTimer.start();
}

void PreviewController::setCascadeCrossCheckEnabled(bool enabled)
{
    if (m_cascadeCrossCheckEnabled == enabled) {
        return;
    }

    m_cascadeCrossCheckEnabled = enabled;
    emit cascadeCrossCheckEnabledChanged();
    m_debounceTimer.start();
}

void PreviewController::setCompressionLevel(int level)
{
    const int clamped = qBound(0, level, 100);
    if (m_compressionLevel == clamped) {
        return;
    }

    m_compressionLevel = clamped;
    emit compressionLevelChanged();
    m_debounceTimer.start();
}

void PreviewController::setOutputFormat(const QString &format)
{
    const QString normalized = (format == QLatin1String("webp") || format == QLatin1String("png"))
        ? format : QStringLiteral("jpg");
    if (m_outputFormat == normalized) {
        return;
    }

    m_outputFormat = normalized;
    emit outputFormatChanged();
    m_debounceTimer.start();
}

void PreviewController::setRotateEnabled(bool enabled)
{
    if (m_rotateEnabled == enabled) {
        return;
    }

    m_rotateEnabled = enabled;
    emit rotateEnabledChanged();
    m_debounceTimer.start();
}

void PreviewController::setCompressionEnabled(bool enabled)
{
    if (m_compressionEnabled == enabled) {
        return;
    }

    m_compressionEnabled = enabled;
    emit compressionEnabledChanged();
    m_debounceTimer.start();
}

void PreviewController::setTimestampEnabled(bool enabled) {
    if (m_timestampEnabled == enabled) return;
    m_timestampEnabled = enabled;
    emit timestampEnabledChanged();
    m_debounceTimer.start();
}
void PreviewController::setTimestampPosition(const QString &position) {
    if (m_timestampPosition == position) return;
    m_timestampPosition = position;
    emit timestampPositionChanged();
    m_debounceTimer.start();
}
void PreviewController::setTimestampColor(const QString &color) {
    if (m_timestampColor == color) return;
    m_timestampColor = color;
    emit timestampColorChanged();
    m_debounceTimer.start();
}
void PreviewController::setTimestampSize(int size) {
    if (m_timestampSize == size) return;
    m_timestampSize = size;
    emit timestampSizeChanged();
    m_debounceTimer.start();
}
void PreviewController::setTimestampX(int x) {
    if (m_timestampX == x) return;
    m_timestampX = x;
    emit timestampXChanged();
    m_debounceTimer.start();
}
void PreviewController::setTimestampY(int y) {
    if (m_timestampY == y) return;
    m_timestampY = y;
    emit timestampYChanged();
    m_debounceTimer.start();
}

QString PreviewController::timestampFont() const { return m_timestampFont; }

void PreviewController::setTimestampFont(const QString &font)
{
    if (m_timestampFont == font) return;
    m_timestampFont = font;
    emit timestampFontChanged();
    m_debounceTimer.start();
}

void PreviewController::regenerateFast()
{
    if (m_sourcePath.isEmpty()) {
        return;
    }

    const QString sourcePath = m_sourcePath;
    const QString fastPath = fastCacheFilePath(sourcePath);
    const QPointer<PreviewController> guard(this);

    if (QFileInfo::exists(fastPath)) {
        setPreviewUrl(QUrl::fromLocalFile(fastPath));
        return;
    }

    QThreadPool::globalInstance()->start(QRunnable::create([sourcePath, fastPath, guard] {
        QImage image = readImageWithResolvedOrientation(sourcePath);
        if (image.isNull()) {
            return;
        }
        image = image.scaled(QSize(1200, 1200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QDir().mkpath(QFileInfo(fastPath).absolutePath());
        image.save(fastPath, "JPG", 85);
        if (!guard) {
            return;
        }
        QObject *target = guard.data();
        if (!target) {
            return;
        }
        QMetaObject::invokeMethod(target, [guard, fastPath] {
            PreviewController *self = guard.data();
            if (!self) {
                return;
            }
            self->setPreviewUrl(QUrl::fromLocalFile(fastPath));
        }, Qt::QueuedConnection);
    }));
}

void PreviewController::regenerate()
{
    if (m_sourcePath.isEmpty()) {
        return;
    }

    if (!m_blurFaces && m_compressionLevel <= 0 && !m_timestampEnabled) {
        regenerateFast();
        return;
    }

    const QString sourcePath = m_sourcePath;
    const quint64 requestId = ++m_requestId;
    const QString targetPath = cacheFilePath(sourcePath, requestId);
    const QPointer<PreviewController> guard(this);

    ProcessingOptions options {
        m_rotateEnabled,
        m_blurFaces,
        m_blurMode,
        m_strength,
        m_detectionSensitivity,
        m_sizeFilterEnabled,
        m_skinColorFilterEnabled,
        m_cascadeCrossCheckEnabled,
        m_compressionEnabled,
        m_compressionLevel,
        m_outputFormat,
        QStringLiteral("yunet"),
        m_timestampEnabled,
        m_timestampCustomText,
        m_timestampFormat,
        m_timestampPosition,
        m_timestampColor,
        m_timestampSize,
        m_timestampX,
        m_timestampY,
        m_timestampFont
    };

    setBusy(true);

    QThreadPool::globalInstance()->start(QRunnable::create([sourcePath, targetPath, options, guard, requestId] {
#ifdef AUTOPHOTO_HAS_OPENCV
        ImageProcessor processor(options);
        const ProcessingResult result = processor.processFile(sourcePath, targetPath);
        if (!guard) {
            return;
        }
        QObject *target = guard.data();
        if (!target) {
            return;
        }
        QMetaObject::invokeMethod(target, [guard, result, targetPath, requestId] {
            PreviewController *self = guard.data();
            if (!self || self->m_requestId != requestId) {
                return;
            }

            self->setBusy(false);
            if (result.success) {
                self->setPreviewUrl(QUrl::fromLocalFile(targetPath));
            } else {
                self->setPreviewUrl({});
                emit self->previewFailed(result.error);
            }
        }, Qt::QueuedConnection);
#else
        QImage image = readImageWithResolvedOrientation(sourcePath);
        if (image.isNull()) {
            if (!guard) return;
            QObject *target = guard.data();
            if (!target) return;
            QMetaObject::invokeMethod(target, [guard, requestId, sourcePath] {
                PreviewController *self = guard.data();
                if (!self || requestId != self->m_requestId) return;
                self->setBusy(false);
                emit self->previewFailed(self->tr("Could not load image: %1").arg(sourcePath));
            }, Qt::QueuedConnection);
            return;
        }
        
        image = image.scaled(QSize(1200, 1200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QDir().mkpath(QFileInfo(targetPath).absolutePath());
        bool saved = image.save(targetPath, "JPG", 85);
        if (!guard) {
            return;
        }
        QObject *target = guard.data();
        if (!target) {
            return;
        }
        QMetaObject::invokeMethod(target, [guard, saved, targetPath, requestId] {
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
    }));
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
        QStringLiteral("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13")
            .arg(sourcePath, m_blurMode)
            .arg(m_strength)
            .arg(m_blurFaces ? 1 : 0)
            .arg(m_detectionSensitivity, 0, 'f', 2)
            .arg(m_sizeFilterEnabled ? 1 : 0)
            .arg(m_skinColorFilterEnabled ? 1 : 0)
            .arg(m_cascadeCrossCheckEnabled ? 1 : 0)
            .arg(m_compressionLevel)
            .arg(m_outputFormat)
            .arg(m_rotateEnabled ? 1 : 0)
            .arg(m_compressionEnabled ? 1 : 0)
            .arg(QStringLiteral("resolved-orient-v7-privacy-mask"))
            .toUtf8(),
        QCryptographicHash::Sha1
    ).toHex();

    return QDir(cacheRoot).filePath(QStringLiteral("preview-%1.jpg").arg(QString::fromLatin1(hash)));
}

QString PreviewController::fastCacheFilePath(const QString &sourcePath) const
{
    QString cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheRoot.isEmpty()) {
        cacheRoot = QDir::tempPath() + QStringLiteral("/autophoto");
    }

    const QByteArray hash = QCryptographicHash::hash(
        QStringLiteral("%1:fast-thumbnail-v1").arg(sourcePath).toUtf8(),
        QCryptographicHash::Sha1
    ).toHex();

    return QDir(cacheRoot).filePath(QStringLiteral("fast-%1.jpg").arg(QString::fromLatin1(hash)));
}
