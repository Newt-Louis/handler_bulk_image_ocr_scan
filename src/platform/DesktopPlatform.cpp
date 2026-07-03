#include "DesktopPlatform.h"

#include <QCoreApplication>
#include <QDir>
#include <QImageReader>
#include <QStandardPaths>

QImage DesktopPlatform::loadImage(const QString &path)
{
    QImageReader reader(path);
    reader.setAutoTransform(false);
    return reader.read();
}

bool DesktopPlatform::saveImage(const QImage &image, const QString &path, const QString &format, int quality)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    return image.save(path, format.isEmpty() ? nullptr : format.toUtf8().constData(), quality);
}

double DesktopPlatform::screenScale() const
{
    return 1.0;
}

QString DesktopPlatform::cacheDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (dir.isEmpty()) {
        dir = QDir::tempPath() + QStringLiteral("/autophoto");
    }
    return dir;
}

QString DesktopPlatform::modelDir() const
{
    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("models")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../Resources/models")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../models")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../models")),
        QStringLiteral("models"),
    };
    for (const QString &candidate : candidates) {
        if (QDir(candidate).exists()) {
            return candidate;
        }
    }
    return QDir::current().filePath(QStringLiteral("models"));
}

bool DesktopPlatform::hasOpenCV() const
{
#ifdef AUTOPHOTO_HAS_OPENCV
    return true;
#else
    return false;
#endif
}
