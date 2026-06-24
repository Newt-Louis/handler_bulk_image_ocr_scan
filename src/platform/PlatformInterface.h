#pragma once

#include <QString>
#include <QImage>
#include <QUrl>

class PlatformInterface
{
public:
    virtual ~PlatformInterface() = default;

    virtual QImage loadImage(const QString &path) = 0;
    virtual bool saveImage(const QImage &image, const QString &path, const QString &format, int quality) = 0;
    virtual double screenScale() const = 0;
    virtual QString cacheDir() const = 0;
    virtual QString modelDir() const = 0;
    virtual bool hasOpenCV() const = 0;
};
