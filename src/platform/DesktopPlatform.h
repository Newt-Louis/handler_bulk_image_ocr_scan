#pragma once

#include "PlatformInterface.h"

class DesktopPlatform final : public PlatformInterface
{
public:
    QImage loadImage(const QString &path) override;
    bool saveImage(const QImage &image, const QString &path, const QString &format, int quality) override;
    double screenScale() const override;
    QString cacheDir() const override;
    QString modelDir() const override;
    bool hasOpenCV() const override;
};
