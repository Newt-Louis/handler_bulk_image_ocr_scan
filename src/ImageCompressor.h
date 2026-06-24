#pragma once

#include <QString>

struct CompressionParams
{
    int jpegQuality = 95;
    double resizeRatio = 1.0;
    QString outputFormat = QStringLiteral("jpg");
};

class ImageCompressor final
{
public:
    static CompressionParams paramsFromLevel(int level);
    static bool compress(const QString &sourcePath, const QString &targetPath, int compressionLevel, const QString &format = QStringLiteral("jpg"));
};
