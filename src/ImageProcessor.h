#pragma once

#include <QString>

struct ProcessingOptions
{
    bool rotateEnabled = true;
    bool blurFaces = true;
    QString blurMode = QStringLiteral("gaussian");
    int strength = 100;
    float detectionSensitivity = 0.50f;
    bool sizeFilterEnabled = true;
    bool skinColorFilterEnabled = true;
    bool cascadeCrossCheckEnabled = true;
    bool compressionEnabled = false;
    int compressionLevel = 0;
    QString outputFormat = QStringLiteral("jpg");
    QString detector = QStringLiteral("yunet");
    bool timestampEnabled = false;
    QString timestampCustomText = QStringLiteral("");
    QString timestampFormat = QStringLiteral("dd/MM/yyyy HH:mm:ss");
    QString timestampPosition = QStringLiteral("BottomRight");
    QString timestampColor = QStringLiteral("#FFFFFF");
    int timestampSize = 24;
    int timestampX = 10;
    int timestampY = 10;
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
