#pragma once

#include <QString>

struct ProcessingOptions
{
    bool blurFaces = true;
    QString blurMode = QStringLiteral("gaussian");
    int strength = 100;
    QString detector = QStringLiteral("yunet");
};

struct ProcessingResult
{
    bool success = false;
    int facesBlurred = 0;
    QString detectorUsed;
    QString error;
};

class ImageProcessor final
{
public:
    explicit ImageProcessor(const ProcessingOptions &options);

    ProcessingResult processFile(const QString &sourcePath, const QString &targetPath);

private:
    ProcessingOptions m_options;
};
